#!/usr/bin/env python3
"""Original Windows A05 v2 Evidence and cross-platform target-tree regressions.

Offline, read-only for original artifacts; synthetic tests use temporary files.
"""
from pathlib import Path, PurePosixPath, PureWindowsPath
from unittest import TestCase, main
import hashlib
import tempfile
import unittest

import audit_a05_isolated_cp314_original_v2 as audit

NAME=("stm32_os_gate4_a05_cp314_isolated_deploy_v2_20261029_"
      "20261029_003947_c4a078e0bdab489eb40171cc07fa4399.evidence.zip")
SHA="1287E148FDAF78B7B79EF1E27F56E0576FAD94721B08588F2C6D0D4EE303A532"
TARGET="a05_cp314_isolated_import_v1_ed2d2b32da064f4a9d8ba141847a9f45"
TREE="31E08CB9B4AEE5917D4E2F9AF49B4D513370D6E4981331397D70590CF4A21BC4"
class A05OriginalTests(TestCase):
    def test_exact_original_if_available(self):
        if not (audit.ROOT/"Evidence"/NAME).exists() or not (
            audit.ROOT/"HostDependencies"/TARGET/"ADMISSION.json").exists():
            self.skipTest("Owner-original off-Git files are absent in clean CI")
        result=audit.review(NAME,SHA)
        self.assertEqual(result["result"],"A05_OWNER_ISOLATED_CP314_IMPORT_ORIGINAL_SCOPED_PASS")
        self.assertEqual(result["evidence_crc"],"6/6")
        self.assertEqual(result["sha_index"],"5/5")
        self.assertEqual(result["size_index"],"4/4")
        self.assertEqual(result["target_members"],174)
        self.assertEqual(result["target_tree_sha256"],TREE)
        self.assertEqual(result["verified_target_basename"],TARGET)
        self.assertEqual(result["controller_import"],"ORIGINAL_WIN_NATIVE_PASS")

    def test_unknown_evidence_cannot_pass(self):
        with self.assertRaisesRegex(AssertionError,"WRONG_PACKAGE"):
            audit.read_original("unknown.evidence.zip",SHA)

    def test_wrong_sha_length_rejected(self):
        with self.assertRaisesRegex(AssertionError,"EVIDENCE_HASH_FORMAT"):
            audit.read_original(NAME,"0"*32)

    def test_windows_directory_component_sort_not_posix(self):
        # WindowsPath compares path *components* after case normalisation.
        # Do not use POSIX sorted() or whole-string casefold key.
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            (root/"Z_folder").mkdir()
            (root/"Z_folder"/"a.txt").write_bytes(b"child")
            (root/"a_folder-1.txt").write_bytes(b"sibling")
            f1,n=audit.tree_fingerprint(root)
            self.assertEqual(n,2)
            self.assertEqual(len(f1),64)
            entries=[f for f in root.rglob("*") if f.is_file()]
            # Native Path ordering is Windows-like on Windows runners; force
            # both reference orderings explicitly for a cross-OS regression.
            posix_entries=sorted(entries,key=lambda f:PurePosixPath(f.relative_to(root).as_posix()))
            windows_entries=sorted(entries,key=lambda f:PureWindowsPath(f.relative_to(root).as_posix()))
            self.assertNotEqual([f.relative_to(root).as_posix() for f in posix_entries],
                                [f.relative_to(root).as_posix() for f in windows_entries])
            payload="\n".join(
                f"{hashlib.sha256(p.read_bytes()).hexdigest().upper()}  {p.stat().st_size}  {p.relative_to(root).as_posix()}"
                for p in windows_entries)+"\n"
            self.assertEqual(f1,hashlib.sha256(payload.encode()).hexdigest().upper())

    def test_tree_sha_tamper_detected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            f=root/"one.txt"
            f.write_bytes(b"ok")
            first=audit.tree_fingerprint(root)
            f.write_bytes(b"changed")
            second=audit.tree_fingerprint(root)
            self.assertEqual(first[1],second[1])
            self.assertNotEqual(first[0],second[0])

    def test_missing_target_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            with self.assertRaisesRegex(AssertionError,"ACTUAL_TARGET_MISSING"):
                audit.tree_fingerprint(Path(tmp)/"not-there")

    def test_no_global_install_or_physical_admission_claim(self):
        self.assertEqual(audit.INPUT_SHA,
            "FF3B4FE1164BE30A41A1554A7FE11EF5A62084F5E3FBB9A366AF827E5C4816AE")
        content=Path(audit.__file__).read_text(encoding="utf-8")
        self.assertIn('"gate4":"NOT_AUTHORIZED"',content)
        self.assertIn('global_python_install',content)

if __name__=="__main__":
    main(verbosity=2)
