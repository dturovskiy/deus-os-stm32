#!/usr/bin/env python3
"""Source-only fail-closed regressions. Real wheel bytes are NOT present."""
import tempfile
from pathlib import Path
import unittest
import json
import audit_a05_windows_wheelhouse_v1 as under_test

class WheelhouseGuardTests(unittest.TestCase):
    def test_no_wheelhouse_is_not_attested(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(AssertionError,"WHEELHOUSE_DIRECTORY_MISSING"):
                under_test.audit(Path(d)/"does_not_exist","cp314")
    def test_empty_wheelhouse_fails(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(AssertionError,"WHEELHOUSE_EXACT_SET_REQUIRED"):
                under_test.audit(Path(d),"cp314")
    def test_tampered_mock_wheels_fail_original_sha_before_record(self):
        cfg=json.loads(under_test.LOCK.read_text())
        names=[
            "cryptography-50.0.2-cp311-abi3-win_amd64.whl",
            "cffi-2.1.1-cp314-cp314-win_amd64.whl",
            "pycparser-3.12-py3-none-any.whl"
        ]
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)
            for n in names:(path/n).write_bytes(b"synthetic-invalid-wheel-not-real"*60)
            with self.assertRaisesRegex(AssertionError,"WHEEL_SHA_MISMATCH_cryptography"):
                under_test.audit(path,"cp314")
    def test_mixed_abi_exact_set_denied(self):
        cfg=json.loads(under_test.LOCK.read_text())
        names=[
            "cryptography-50.0.2-cp314-cp314t-win_amd64.whl",
            "cffi-2.1.1-cp314-cp314-win_amd64.whl",
            "pycparser-3.12-py3-none-any.whl"
        ]
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)
            for n in names:(path/n).write_bytes(b"synthetic-no-wheel-bytes")
            with self.assertRaisesRegex(AssertionError,"WHEELHOUSE_EXACT_SET_REQUIRED"):
                under_test.audit(path,"cp314t")
    def test_metadata_only_sha_never_counts_as_verified_wheel(self):
        cfg=json.loads(under_test.LOCK.read_text())
        self.assertEqual(cfg["status"],"UNAPPROVED_SOURCE_METADATA_ONLY_NO_WHEEL_BYTES")
        self.assertEqual(len(cfg["wheels"]),5)
        self.assertNotEqual(cfg["status"],"SOURCE_PINNED_WHEELHOUSE_BYTES_VERIFIED_NO_INSTALL")
if __name__=="__main__":
    unittest.main(verbosity=2)
