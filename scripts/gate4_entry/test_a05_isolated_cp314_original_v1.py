#!/usr/bin/env python3
"""Negative source-only regression for future immutable owner Evidence acceptance."""
import hashlib
import tempfile
from pathlib import Path
import unittest

import audit_a05_isolated_cp314_original_v1 as audit

class OriginalAcceptanceTests(unittest.TestCase):
    def test_nonexistent_original_fail_closed(self):
        name=audit.ID+"_20990101_000000_00000000000000000000000000000000.evidence.zip"
        with self.assertRaisesRegex(AssertionError,"ORIGINAL_EVIDENCE_NOT_ACCESSIBLE"):
            audit.read_original(name,"A"*64)

    def test_foreign_evidence_name_rejected(self):
        with self.assertRaisesRegex(AssertionError,"WRONG_PACKAGE"):
            audit.read_original("unrelated_bundle_0.evidence.zip","A"*64)

    def test_sha_format_rejected(self):
        name=audit.ID+"_20261028_000000_00000000000000000000000000000000.evidence.zip"
        with self.assertRaisesRegex(AssertionError,"EVIDENCE_HASH_FORMAT"):
            audit.read_original(name,"bad")

    def test_tree_digest_exact_and_changes_on_tamper(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d)
            f=root/"a.txt"
            f.write_bytes(b"payload")
            a= audit.tree_fingerprint(root)
            self.assertEqual(a[1],1)
            self.assertEqual(len(a[0]),64)
            self.assertEqual(a,audit.tree_fingerprint(root))
            f.write_bytes(b"tamper")
            self.assertNotEqual(a[0],audit.tree_fingerprint(root)[0])

    def test_missing_target_never_admitted(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaisesRegex(AssertionError,"ACTUAL_TARGET_MISSING"):
                audit.tree_fingerprint(Path(d)/"does-not-exist")

    def test_isolated_receipt_not_physical_release(self):
        self.assertIn("NOT_AUTHORIZED",Path(audit.__file__).read_text())
        self.assertIn("global_python_install",Path(audit.__file__).read_text())
        self.assertEqual(audit.INPUT_SHA,
            "EB8B38354EDD866F4A6970CFACD156DCBE8744081E1F3CB290B37DE7A1E2F2A8")
if __name__=="__main__":
    unittest.main(verbosity=2)
