#!/usr/bin/env python3
"""Regression suite for permanent read-only Gate3 signed-owner auditor."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
import unittest
import zipfile
import gate3_auditor as g

ROOT=Path(__file__).resolve().parents[2]
WORKSPACE=ROOT.parent
STAGE=WORKSPACE/"Bundle"/("_stage_"+g.STAGE_ID)
LOCK=WORKSPACE/"Bundle/_stage_stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027/payload/physical_000d_source_lock.json"
CONSENT=WORKSPACE/"Evidence/stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027_20261027_234630_623699d0508b48d1a47ad8441565c32b.evidence.zip"
CONSENT_SHA="DD87116F54B7E3A8C9FB9A68CFC620C5CF58049C5F9B9001CF6E15110CE1E9C0"
TRUST=WORKSPACE/"Evidence/stm32_os_gate3_owner_existing_candidate_readonly_sign_v1_20261027_20261027_210632_df8435a546b44d5993fa34b3f0fb2770.evidence.zip"
AT=datetime(2026,10,28,0,0,0,tzinfo=timezone.utc)

class Gate3PermanentAudit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.owner=g.evidence(CONSENT,CONSENT_SHA)
        cls.trust=g.evidence(TRUST,g.TRUST_EVIDENCE_SHA)
        cls.msg=cls.owner["signing_probe.txt"]
        cls.sig=cls.owner["signing_probe.txt.sig"]
        cls.pub=cls.trust["new_candidate_allowed_signers.txt"]
        cls.lock=LOCK.read_bytes()

    def test_01_real_original_consent_positive(self):
        result=g.audit(STAGE,LOCK,CONSENT,CONSENT_SHA,TRUST,AT)
        self.assertEqual(result["result"],"PASS")
        self.assertEqual(result["source_files"],14)
        self.assertEqual(result["hardware_release"],"DENIED_UNTIL_SEPARATE_NATIVE_GATE")

    def test_02_exact_source_files_positive(self):
        self.assertEqual(g.verify_plan(STAGE,self.lock),14)

    def test_03_wrong_original_evidence_sha_rejected(self):
        with self.assertRaisesRegex(g.Reject,"EVIDENCE_SHA"):
            g.audit(STAGE,LOCK,CONSENT,"A"*64,TRUST,AT)

    def test_04_expired_consent_rejected(self):
        with self.assertRaisesRegex(g.Reject,"CONSENT_EXPIRED"):
            g.audit(STAGE,LOCK,CONSENT,CONSENT_SHA,TRUST,
                    datetime(2026,10,29,tzinfo=timezone.utc))

    def test_05_future_expiry_above_window_rejected(self):
        with self.assertRaisesRegex(g.Reject,"CONSENT_EXPIRED"):
            g.verify_consent(self.msg,g.sha(self.lock),
                             datetime(2026,10,25,tzinfo=timezone.utc))

    def test_06_wrong_reset_count_rejected(self):
        bad=self.msg.replace(b"max_device_reset=1",b"max_device_reset=2")
        with self.assertRaisesRegex(g.Reject,"CONSENT_SCOPE"):
            g.verify_consent(bad,g.sha(self.lock),AT)

    def test_07_tampered_source_sha_binding_rejected(self):
        with self.assertRaisesRegex(g.Reject,"CONSENT_SCOPE"):
            g.verify_consent(self.msg,"F"*64,AT)

    def test_08_bad_sshsig_signature_rejected(self):
        bad=self.sig.replace(b"-----BEGIN SSH SIGNATURE-----",
                             b"-----BEGIN UNKNOWN SIGNATURE-----",1)
        with self.assertRaisesRegex(g.Reject,"SIGNATURE_ARMOR"):
            g.verify_sshsig(self.msg,bad,self.pub)

    def test_09_message_signature_mismatch_rejected(self):
        with self.assertRaisesRegex(g.Reject,"BAD_OWNER_SIGNATURE"):
            g.verify_sshsig(self.msg+b"x",self.sig,self.pub)

    def test_10_bad_public_trust_rejected(self):
        with self.assertRaisesRegex(g.Reject,"SIGNER_NOT_IN_ACCEPTED_TRUST"):
            g.verify_sshsig(self.msg,self.sig,b"deus-os-hardware-owner ssh-ed25519 WRONG")

    def test_11_source_lock_tamper_rejected(self):
        altered=json.loads(self.lock)
        altered["allowed_reset_attempts"]=2
        with self.assertRaisesRegex(g.Reject,"LOCK_SCOPE"):
            g.verify_plan(STAGE,json.dumps(altered).encode())

    def test_12_quarantine_marker_absent_rejected(self):
        with tempfile.TemporaryDirectory(prefix="deus_gate3_audit_") as tmp:
            with self.assertRaisesRegex(g.Reject,"STAGE_QUARANTINE_MISSING"):
                g.verify_plan(Path(tmp),self.lock)

    def test_13_source_file_sha_tamper_rejected(self):
        with tempfile.TemporaryDirectory(prefix="deus_gate3_audit_") as tmp:
            stage=Path(tmp)
            for name in json.loads(self.lock)["source_files"]:
                path=stage/name
                path.parent.mkdir(parents=True,exist_ok=True)
                shutil.copyfile(STAGE/name,path)
            shutil.copyfile(STAGE/"QUARANTINED_NOT_FOR_RUN.txt",stage/"QUARANTINED_NOT_FOR_RUN.txt")
            f=stage/"payload/remote_handoff_v2.py"
            f.write_bytes(f.read_bytes()+b"\n")
            with self.assertRaisesRegex(g.Reject,"SOURCE_DRIFT"):
                g.verify_plan(stage,self.lock)

    def test_14_json_duplicate_field_rejected(self):
        with self.assertRaisesRegex(g.Reject,"DUPLICATE_JSON_KEY"):
            g.parse_json(b'{"a":1,"a":2}')

    def test_15_zip_missing_index_rejected(self):
        with tempfile.TemporaryDirectory(prefix="deus_gate3_audit_") as tmp:
            p=Path(tmp)/"no_index.zip"
            with zipfile.ZipFile(p,"w") as z:z.writestr("outcome.txt","anything")
            with self.assertRaisesRegex(g.Reject,"INDEX_MISSING"):
                g.evidence(p,g.sha(p.read_bytes()))

    def test_16_candidate_without_archive_rejected(self):
        with self.assertRaisesRegex(g.Reject,"CANDIDATE_MISSING"):
            g.inspect_candidate(Path("/nonexistent/deus-gate3.zip"),"0"*64,STAGE,json.loads(self.lock))

    def test_17_unsigned_package_still_quarantined(self):
        self.assertTrue((STAGE/"QUARANTINED_NOT_FOR_RUN.txt").is_file())

    def test_18_no_hardware_invocation_in_module(self):
        source=Path(g.__file__).read_text()
        self.assertNotIn("subprocess.run(",source)
        self.assertNotIn("os.system(",source)
        self.assertNotIn("libusb",source)
        self.assertNotIn("STM32_Programmer_CLI",source)
        self.assertNotIn("os.chown(",source)
        self.assertIn("DENIED_UNTIL_SEPARATE_NATIVE_GATE",source)

    def test_19_permanent_policy_references_current_original_evidence(self):
        policy=g.parse_json((ROOT/"scripts/gate3_operator/policy.json").read_bytes())
        self.assertEqual(policy["schema"],"DEUS_GATE3_OPERATOR_AUDIT_POLICY_V1")
        self.assertEqual(policy["current_owner_consent_evidence_sha256"],CONSENT_SHA)
        self.assertEqual(g.sha(CONSENT.read_bytes()),CONSENT_SHA)

    def test_20_permanent_policy_never_grants_hardware_release(self):
        policy=g.parse_json((ROOT/"scripts/gate3_operator/policy.json").read_bytes())
        self.assertEqual(policy["operational_release"],
                         "DENIED_UNTIL_SEPARATE_NATIVE_HARDWARE_GATE")


if __name__=="__main__":
    unittest.main(verbosity=2)
