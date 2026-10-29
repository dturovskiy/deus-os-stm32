#!/usr/bin/env python3
"""Synthetic-only source integration negatives; no owner signature or hardware."""
from dataclasses import replace
from datetime import timedelta
import hashlib
from pathlib import Path
import tempfile
import unittest

from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
import release_approval_contract_v1 as ra01
import test_release_approval_contract_v1 as ra01_test
import test_verify_release_signature_v1 as ra02_test
import release_admission_source_composition_v1 as c

PROJECT = Path(__file__).resolve().parents[3]
PRIVATE_RA03 = PROJECT / "Evidence" / c.RA03_NAME
PRIVATE_TRUST = PROJECT / "Evidence" / ra02_test.v.TRUST_EVIDENCE_NAME
PRIVATE_ORIGINALS_PRESENT = PRIVATE_RA03.is_file() and PRIVATE_TRUST.is_file()


class CompositionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="deus_g4_composite_source_")
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.bundle = self.root / "Bundle"
        self.bundle.mkdir()
        self.zip = self.bundle / "synthetic_only.zip"
        self.zip.write_bytes(b"NO_REAL_GATE4_HARDWARE_ZIP_SAFETY_NEGATIVES")
        self.key = Ed25519PrivateKey.generate()  # ephemeral RAM-only
        self.pub = ra02_test.ssh_pub(self.key)
        self.material = replace(
            ra01_test.M,
            zip_bytes=self.zip.stat().st_size,
            zip_sha256=ra01.sha(self.zip.read_bytes()),
        )
        self.fields = ra01_test.fixture()
        self.fields["operator_zip_sha256"] = self.material.zip_sha256
        self.fields["operator_zip_bytes"] = self.material.zip_bytes
        self.statement = ra01.canonical_bytes(self.fields)
        self.sig = ra02_test.fake_sign(self.key, msg=self.statement)
        self.now = ra01_test.NOW
        self.flags = {"bundle_root": self.bundle, "workspace_root": PROJECT,
                      "_unit_test_public_key": self.pub}

    def inspect(self, **updates):
        kwargs = dict(statement=self.statement, signature=self.sig,
                      candidate_zip=self.zip, material=self.material,
                      now=self.now, **self.flags)
        kwargs.update(updates)
        return c.inspect_source_composition(**kwargs)

    def stop(self, expected=None, **updates):
        with self.assertRaises(c.ra01.Reject) as cm:
            self.inspect(**updates)
        if expected is not None:
            self.assertEqual(str(cm.exception), expected)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_synthetic_valid_signature_and_native_ra03_still_deny(self):
        result = self.inspect()
        self.assertEqual(result["result"], "PASS")
        self.assertEqual(result["signer"], "ED25519_VERIFIED")
        self.assertEqual(result["ra03_native_cases"], 24)
        self.assertEqual(result["authorization"], "DENIED")
        self.assertEqual(result["nonce_consumed"], "NO")
        self.assertEqual(result["release_admitted"], "NO")
        self.assertEqual(result["trusted_controller"], "NOT_INSTALLED")
        self.assertEqual(result["device_io"], "NONE")
        self.assertEqual(result["gate4"], "NOT_AUTHORIZED")

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_real_owner_public_trust_must_reject_test_key(self):
        self.stop("SSHSIG_SIGNER_NOT_TRUSTED", _unit_test_public_key=None)

    def test_candidate_byte_change(self):
        self.zip.write_bytes(b"ONE_BYTE_DRIFT"+self.zip.read_bytes())
        self.stop("CANDIDATE_SIZE")

    def test_candidate_sha_change_same_size(self):
        raw=self.zip.read_bytes()
        self.zip.write_bytes(bytes([raw[0]^1])+raw[1:])
        self.stop("CANDIDATE_WHOLE_SHA")

    def test_candidate_path_moved(self):
        alternative=self.root/"outside.zip"
        alternative.write_bytes(self.zip.read_bytes())
        self.stop("CANDIDATE_ROOT_OR_LINK",candidate_zip=alternative)

    def test_candidate_symlink(self):
        link=self.bundle/"linked.zip"
        try:
            link.symlink_to(self.zip)
        except (OSError,NotImplementedError):
            self.skipTest("symlinks not permitted by test sandbox")
        self.stop("CANDIDATE_ROOT_OR_LINK",candidate_zip=link)

    def test_bad_zip_size_pin(self):
        self.stop("CANDIDATE_SIZE",material=replace(self.material,zip_bytes=self.material.zip_bytes+1))

    def test_approval_zip_digest_changed(self):
        f=dict(self.fields)
        f["operator_zip_sha256"] = "0"*64
        self.stop("BIND_OPERATOR_ZIP_SHA256",statement=ra01.canonical_bytes(f))

    def test_approval_changed_nonce(self):
        f=dict(self.fields)
        f["nonce"]="0"*32
        self.stop("BIND_NONCE",statement=ra01.canonical_bytes(f))

    def test_approval_expired(self):
        self.stop("APPROVAL_EXPIRED_OR_FUTURE",now=self.now+timedelta(hours=1))

    def test_signature_tampered(self):
        damaged=self.sig.replace(b"-----END SSH SIGNATURE-----", b"-----END OTHER SIGNATURE-----")
        self.stop("SSHSIG_ARMOR",signature=damaged)

    def test_wrong_signing_namespace(self):
        sig=ra02_test.fake_sign(self.key,msg=self.statement,namespace=b"deus-os-gate3-one-reset-v1")
        self.stop("SSHSIG_WRONG_NAMESPACE",signature=sig)

    def test_signature_other_key(self):
        other=Ed25519PrivateKey.generate()
        self.stop("SSHSIG_SIGNER_NOT_TRUSTED",signature=ra02_test.fake_sign(other,msg=self.statement))

    def test_statement_signature_mismatch(self):
        f=dict(self.fields);f["nonce"]="1234567890abcdef1234567890abcdef"
        self.stop("INVALID_OWNER_SIGNATURE",statement=ra01.canonical_bytes(f))

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_explicit_native_ra03_original_evidence_attestation(self):
        result=c.attest_native_ra03(PROJECT)
        self.assertEqual(result["ra03_evidence_sha256"],c.RA03_SHA)
        self.assertEqual(result["native_cases"],24)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_wrong_ra03_whole_digest_rejected(self):
        with self.assertRaisesRegex(c.Reject,"RA03_ORIGINAL_WHOLE_SHA"):
            c.attest_native_ra03(PROJECT,expected_sha="0"*64)

    def test_wrong_ra03_root_rejected(self):
        with self.assertRaisesRegex(c.Reject,"RA03_ORIGINAL_ABSENT"):
            c.attest_native_ra03(self.root)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_no_production_mutations(self):
        before=set(self.bundle.iterdir())
        self.inspect()
        self.assertEqual(before,set(self.bundle.iterdir()))
        self.assertEqual(self.zip.read_bytes(),b"NO_REAL_GATE4_HARDWARE_ZIP_SAFETY_NEGATIVES")

    def handshake(self, writer, **updates):
        kwargs=dict(statement=self.statement, signature=self.sig,
                    candidate_zip=self.zip, material=self.material,
                    now=self.now, source_only_nonce_writer=writer,
                    **self.flags)
        kwargs.update(updates)
        return c.exercise_nonce_sequence_source_only(**kwargs)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_synthetic_nonce_test_sink_only_after_all_source_checks(self):
        calls=[]
        def sink(nonce, statement_sha, zip_sha):
            calls.append((nonce,statement_sha,zip_sha))
            return {"classification":"RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED",
                    "nonce":nonce,"record_sha256":"A"*64,
                    "authorization":"DENIED","launch_authorized":False,
                    "device_io":"NONE"}
        result=self.handshake(sink)
        self.assertEqual(len(calls),1)
        self.assertEqual(calls[0],(self.fields["nonce"],ra01.sha(self.statement),self.material.zip_sha256))
        self.assertEqual(result["nonce_consumed"],"YES_IN_TEST_SINK_ONLY")
        self.assertEqual(result["authorization"],"DENIED")
        self.assertEqual(result["trusted_controller"],"NOT_INSTALLED")
        self.assertEqual(result["release_admitted"],"NO")
        self.assertEqual(result["device_io"],"NONE")

    def test_no_nonce_sink_call_on_wrong_signature(self):
        calls=[]
        def sink(*x): calls.append(x); raise AssertionError("UNAUTHORIZED_WRITER_CALLED")
        sig=ra02_test.fake_sign(Ed25519PrivateKey.generate(),msg=self.statement)
        with self.assertRaisesRegex(c.ra01.Reject,"SSHSIG_SIGNER_NOT_TRUSTED"):
            self.handshake(sink,signature=sig)
        self.assertEqual(calls,[])

    def test_no_nonce_sink_call_on_zip_byte_drift(self):
        calls=[]
        def sink(*x): calls.append(x); raise AssertionError("UNAUTHORIZED_WRITER_CALLED")
        original=self.zip.read_bytes()
        self.zip.write_bytes(bytes([original[0]^1])+original[1:])
        with self.assertRaisesRegex(c.Reject,"CANDIDATE_WHOLE_SHA"):
            self.handshake(sink)
        self.assertEqual(calls,[])

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_nonce_sink_failure_no_retry(self):
        calls=[]
        def sink(*x): calls.append(x); raise ValueError("SIMULATED_DURABILITY_UNKNOWN")
        with self.assertRaisesRegex(c.Reject,"RA03_ONCE_ATTEMPTED_STOP_NO_RETRY"):
            self.handshake(sink)
        self.assertEqual(len(calls),1)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_nonce_sink_invalid_receipt_no_retry(self):
        calls=[]
        def sink(nonce,*x):
            calls.append(nonce)
            return {"classification":"RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED",
                    "nonce":nonce,"record_sha256":"B"*64,
                    "authorization":"ADMITTED","launch_authorized":True,
                    "device_io":"USB"}
        with self.assertRaisesRegex(c.Reject,"RA03_RECEIPT_AFTER_ONCE_UNTRUSTED_STOP_NO_RETRY"):
            self.handshake(sink)
        self.assertEqual(len(calls),1)

    @unittest.skipUnless(PRIVATE_ORIGINALS_PRESENT, "requires SHA-pinned private owner trust and original RA03 Evidence")
    def test_replay_against_test_sink_fails_no_retry(self):
        used=set()
        calls=[]
        def sink(nonce,*x):
            calls.append(nonce)
            if nonce in used: raise ValueError("NONCE_ALREADY_USED")
            used.add(nonce)
            return {"classification":"RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED",
                    "nonce":nonce,"record_sha256":"F"*64,
                    "authorization":"DENIED","launch_authorized":False,
                    "device_io":"NONE"}
        self.assertEqual(self.handshake(sink)["authorization"],"DENIED")
        with self.assertRaisesRegex(c.Reject,"RA03_ONCE_ATTEMPTED_STOP_NO_RETRY"):
            self.handshake(sink)
        self.assertEqual(len(calls),2)
        self.assertEqual(len(used),1)

if __name__=="__main__":
    unittest.main(verbosity=2)
