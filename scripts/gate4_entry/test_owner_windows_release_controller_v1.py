#!/usr/bin/env python3
"""RA-05 source controller unit tests; no production key/nonce/device action."""
import json
import os
from pathlib import Path
import tempfile
import unittest
from types import SimpleNamespace
from unittest.mock import patch

from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
import release_approval_contract_v1 as ra01
import test_release_approval_contract_v1 as ra01_test
import test_verify_release_signature_v1 as ssh_test
import owner_windows_release_controller_v1 as ctrl

ROOT=Path(__file__).resolve().parents[3]
HERE=Path(__file__).resolve().parent

class SourceTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(prefix="deus_owner_controller_")
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.bundles=self.root/"Bundle"
        self.bundles.mkdir()
        self.ledger=self.root/"PrivateLedger"
        self.ledger.mkdir()
        self.owned=self.root/"PrivateController"
        self.owned.mkdir()
        self.configpath=self.owned/"config.json"
        self.letter=self.root/"Approval"
        self.letter.mkdir()
        self.candidate=self.bundles/(ra01.PKG_ID+".zip")
        self.candidate.write_bytes(b"NONEXECUTABLE_SYNTHETIC_CANONICAL_CANDIDATE")
        self.adapter=HERE/"owner_windows_nonce_adapter_v1.ps1"
        self.module=HERE/"nonce_ledger_offline.psm1"
        self.conf={
           "schema":"DEUS_OS_GATE4_CONTROLLER_CONFIG_V1",
           "bundle_root":str(self.bundles),
           "candidate_zip":str(self.candidate),
           "ledger_directory":str(self.ledger),
           "adapter_path":str(self.adapter),
           "module_path":str(self.module),
           "operator_zip_sha256":ra01.sha(self.candidate.read_bytes()),
           "operator_zip_bytes":self.candidate.stat().st_size,
           "source_lock_sha256":"A"*64,
           "source_git_tree":ra01.FUTURE_TREE,
           "build_provenance_sha256":"B"*64,
           "entrypoint_sha256":"C"*64,
           "readiness_evidence_sha256":"D"*64,
           "device_usb_port":"usb:001:8",
           "host_identity":"macmini",
        }
        self.write_config()
        self.own=Ed25519PrivateKey.generate()
        self.statement=ra01_test.fixture()
        self.statement["operator_zip_sha256"]=self.conf["operator_zip_sha256"]
        self.statement["operator_zip_bytes"]=self.conf["operator_zip_bytes"]
        self.message=ra01.canonical_bytes(self.statement)
        self.sig=ssh_test.fake_sign(self.own,msg=self.message)
        self.signfile=self.letter/"statement.sig"
        self.msgfile=self.letter/"statement.json"
        self.msgfile.write_bytes(self.message)
        self.signfile.write_bytes(self.sig)
        self.fake_pwsh=self.owned/"pwsh.exe"
        self.fake_pwsh.write_bytes(b"ONLY_A_TEST_PATH")
    def write_config(self):
        self.configpath.write_text(json.dumps(self.conf,sort_keys=True)+"\n")

    def test_source_sha_pins_real_adapter_and_ledger(self):
        cfg=ctrl.load_owner_config(self.configpath)
        self.assertEqual(cfg.zip_path,self.candidate)
        ctrl.check_source_files(cfg)
        self.assertEqual(ctrl.sha(self.adapter.read_bytes()),ctrl.ADAPTER_SHA256)
        self.assertEqual(ctrl.sha(self.module.read_bytes()),ctrl.LEDGER_SHA256)

    def test_missing_nonce_ledger(self):
        self.ledger.rmdir()
        with self.assertRaisesRegex(ctrl.Stop,"LEDGER_NOT_PREPROVISIONED"):
            ctrl.load_owner_config(self.configpath)
    def test_candidate_wrong_name(self):
        q=self.bundles/"other.zip"
        q.write_bytes(self.candidate.read_bytes())
        self.conf["candidate_zip"]=str(q);self.write_config()
        with self.assertRaisesRegex(ctrl.Stop,"CANDIDATE_WRONG_PACKAGE_ID"):
            ctrl.load_owner_config(self.configpath)
    def test_config_inside_bundle(self):
        inside=self.bundles/"config.json"
        inside.write_bytes(self.configpath.read_bytes())
        with self.assertRaisesRegex(ctrl.Stop,"CONFIG_WITHIN_BUNDLE"):
            ctrl.load_owner_config(inside)
    def test_ledger_under_bundle(self):
        new=self.bundles/"private"
        new.mkdir()
        self.conf["ledger_directory"]=str(new);self.write_config()
        with self.assertRaisesRegex(ctrl.Stop,"LEDGER_INSIDE_BUNDLE"):
            ctrl.load_owner_config(self.configpath)
    def test_wrong_zip_sha_format(self):
        self.conf["operator_zip_sha256"]="A"*63;self.write_config()
        with self.assertRaisesRegex(ra01.Reject,"TRUST_ZIP_SHA256"):
            ctrl.load_owner_config(self.configpath)
    def test_wrong_zip_size_bool(self):
        self.conf["operator_zip_bytes"]=True;self.write_config()
        with self.assertRaisesRegex(ctrl.Stop,"CONFIG_ZIP_SIZE_TYPE"):
            ctrl.load_owner_config(self.configpath)
    def test_config_duplicate_json_field(self):
        raw=self.configpath.read_text().rstrip()
        self.configpath.write_text(raw[:-1]+',"host_identity":"evil"}\n')
        with self.assertRaisesRegex(ctrl.Stop,"DUPLICATE_CONFIG_FIELD"):
            ctrl.load_owner_config(self.configpath)
    def test_config_unknown_field(self):
        self.conf["extra"]="not-trusted";self.write_config()
        with self.assertRaisesRegex(ctrl.Stop,"CONFIG_FIELDS"):
            ctrl.load_owner_config(self.configpath)
    def test_config_no_symlink(self):
        link=self.owned/"config_link"
        link.symlink_to(self.configpath)
        with self.assertRaisesRegex(ctrl.Stop,"UNSAFE_SYMLINK"):
            ctrl.load_owner_config(link)
    def test_candidate_zip_drift_is_rejected_before_signed_verification(self):
        cfg=ctrl.load_owner_config(self.configpath)
        self.candidate.write_bytes(self.candidate.read_bytes()+b"!")
        with self.assertRaisesRegex(ra01.Reject,"CANDIDATE_SIZE"):
            ctrl.inspect_exact_approval(self.message,self.sig,cfg,ra01_test.NOW,ROOT)
    @unittest.skipUnless((ROOT/"Evidence"/ssh_test.v.TRUST_EVIDENCE_NAME).is_file(), "requires pinned private owner trust Evidence")
    def test_real_owner_key_never_accepts_ephemeral_test_key(self):
        cfg=ctrl.load_owner_config(self.configpath)
        with self.assertRaisesRegex(ra01.Reject,"SSHSIG_SIGNER_NOT_TRUSTED"):
            ctrl.inspect_exact_approval(self.message,self.sig,cfg,ra01_test.NOW,ROOT)
    def test_source_controller_no_windows_platform(self):
        with patch.object(ctrl.sys,"platform","linux"):
            with self.assertRaisesRegex(ctrl.Stop,"WINDOWS_ONLY"):
                ctrl.execute_source_controller(self.configpath,self.msgfile,self.signfile,
                                              ROOT,self.fake_pwsh)
    def test_source_reserve_once_never_retries_after_timeout(self):
        calls=[]
        def fake_run(command,**kwargs):
            calls.append(command)
            raise TimeoutError("simulated")
        with patch.object(ctrl.sys,"platform","win32"):
            cfg=ctrl.load_owner_config(self.configpath)
            with self.assertRaisesRegex(ctrl.Stop,"NONCE_POSSIBLY_CONSUMED_NO_RETRY"):
                ctrl.reserve_once(cfg=cfg,nonce=self.statement["nonce"],
                     statement_sha256="A"*64,operator_sha256="B"*64,
                     executable=self.fake_pwsh,runner=fake_run)
        self.assertEqual(len(calls),1)
    def test_source_reserve_once_refuses_bad_receipt_without_retry(self):
        calls=[]
        def fake_run(command,**kwargs):
            calls.append(command)
            return SimpleNamespace(returncode=0,stderr=b"",
                stdout=b'{"authorization":"ADMITTED"}\n')
        with patch.object(ctrl.sys,"platform","win32"):
            cfg=ctrl.load_owner_config(self.configpath)
            with self.assertRaisesRegex(ctrl.Stop,"NONCE_RECEIPT_UNTRUSTED_NO_RETRY"):
                ctrl.reserve_once(cfg=cfg,nonce=self.statement["nonce"],
                     statement_sha256="A"*64,operator_sha256="B"*64,
                     executable=self.fake_pwsh,runner=fake_run)
        self.assertEqual(len(calls),1)
    def test_source_reserve_once_valid_denial_receipt(self):
        calls=[]
        nonce=self.statement["nonce"]
        receipt={"classification":"RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED",
                 "nonce":nonce,"record_sha256":"C"*64,"authorization":"DENIED",
                 "launch_authorized":False,"device_io":"NONE"}
        def fake_run(command,**kwargs):
            calls.append(command)
            return SimpleNamespace(returncode=0,stderr=b"",
                                   stdout=json.dumps(receipt).encode()+b"\n")
        with patch.object(ctrl.sys,"platform","win32"):
            cfg=ctrl.load_owner_config(self.configpath)
            result=ctrl.reserve_once(cfg=cfg,nonce=nonce,
                statement_sha256="A"*64,operator_sha256="B"*64,
                executable=self.fake_pwsh,runner=fake_run)
        self.assertEqual(result["authorization"],"DENIED")
        self.assertEqual(len(calls),1)
        self.assertEqual(calls[0][0],str(self.fake_pwsh))
        self.assertIn("-File",calls[0])
        self.assertIn(str(self.ledger),calls[0])
    def test_noncanonical_lowercase_digest_rejected(self):
        cfg=ctrl.load_owner_config(self.configpath)
        with patch.object(ctrl.sys,"platform","win32"):
            with self.assertRaisesRegex(ctrl.Stop,"SHA_INVALID"):
                ctrl.reserve_once(cfg=cfg,nonce=self.statement["nonce"],
                    statement_sha256="a"*64,operator_sha256="B"*64,
                    executable=self.fake_pwsh)
    def test_real_path_module_drift_rejected(self):
        changed=self.owned/"nonce_ledger.psm1"
        changed.write_bytes(self.module.read_bytes()+b"\n#change")
        self.conf["module_path"]=str(changed);self.write_config()
        cfg=ctrl.load_owner_config(self.configpath)
        with self.assertRaisesRegex(ctrl.Stop,"RA03_MODULE_SHA_MISMATCH"):
            ctrl.check_source_files(cfg)
    def test_real_path_adapter_drift_rejected(self):
        changed=self.owned/"adapter.ps1"
        changed.write_bytes(self.adapter.read_bytes()+b"\n#change")
        self.conf["adapter_path"]=str(changed);self.write_config()
        cfg=ctrl.load_owner_config(self.configpath)
        with self.assertRaisesRegex(ctrl.Stop,"ADAPTER_SHA_MISMATCH"):
            ctrl.check_source_files(cfg)
    def test_nonce_replay_sink_never_implicitly_admits(self):
        # Not a real Windows native test. Proof that a successful receipt
        # from the offline module can NEVER become physical release.
        self.assertEqual(ctrl.CONTROLLER_CLASS,
                         "G4_A05_NONCE_RECORDED_SOURCE_CONTROLLER_ONLY")
        self.assertEqual(ctrl.main(["--config",str(self.configpath),
                                    "--statement",str(self.msgfile),
                                    "--signature",str(self.signfile),
                                    "--workspace-root",str(ROOT),
                                    "--pwsh",str(self.fake_pwsh)]),1)

if __name__=="__main__":
    unittest.main(verbosity=2)
