#!/usr/bin/env python3
"""Source-only tests; NEVER claim owner Windows Python inventory or installation."""
from pathlib import Path
import ast
import hashlib
import json
import runpy
import unittest

import audit_a05_python_dependency_inventory_candidate_v1 as candidate
import audit_a05_python_dependency_inventory_zip_v1 as zipaudit

ROOT=Path(__file__).resolve().parents[3]
HERE=ROOT/"OS/scripts/gate4_entry"
def sha(x):return hashlib.sha256(x).hexdigest().upper()
class SourceTests(unittest.TestCase):
    @unittest.skipUnless(candidate.STAGE.is_dir(), "requires historical native operator stage outside public Git")
    def test_candidate_spec_and_source_stage(self):
        candidate.main()
    @unittest.skipUnless((zipaudit.ROOT/'Bundle'/(zipaudit.ID+'.zip')).is_file(), "requires original historical ZIP outside public Git")
    def test_final_exact_bundle_bytes(self):
        zipaudit.audit()
    def test_probe_is_parseable_standard_library_only(self):
        f=HERE/"probe_windows_python_dependency_inventory_v1.py"
        tree=ast.parse(f.read_text(encoding="utf-8"))
        imports=set()
        for n in ast.walk(tree):
            if isinstance(n,ast.Import):imports.update(a.name.split(".")[0] for a in n.names)
            if isinstance(n,ast.ImportFrom) and n.module:
                imports.add(n.module.split(".")[0])
        self.assertFalse(imports & {"subprocess","requests","urllib"})
        self.assertTrue({"sys","sysconfig","importlib","platform","struct","json","os"}<=imports)
        self.assertIn("from pip._vendor.packaging.tags import sys_tags",f.read_text())
        self.assertNotIn("pip._internal",f.read_text())
    def test_no_linux_execution_can_claim_windows_inventory(self):
        import os
        if os.name=="nt":
            self.skipTest("Linux-only negative; owner Windows native must be separate")
        p=HERE/"probe_windows_python_dependency_inventory_v1.py"
        with self.assertRaisesRegex(RuntimeError,"WINDOWS_ONLY_INVENTORY"):
            runpy.run_path(str(p),run_name="__main__")
    def test_official_candidate_hash_metadata_is_not_wheel_bytes(self):
        obj=json.loads((HERE/"a05_windows_python_dependency_candidates_v1.json").read_text())
        self.assertEqual(obj["status"],"UNAPPROVED_SOURCE_METADATA_ONLY_NO_WHEEL_BYTES")
        self.assertEqual(len(obj["wheels"]),5)
        self.assertTrue(all(len(w["sha256"])==64 for w in obj["wheels"]))
        self.assertFalse((ROOT/"Bundle"/"cryptography-50.0.2-cp311-abi3-win_amd64.whl").exists())
    def test_future_original_auditor_ast(self):
        f=HERE/"audit_a05_python_dependency_inventory_original_v1.py"
        ast.parse(f.read_text(encoding="utf-8"))
        source=f.read_text()
        self.assertIn("ORIGINAL_EVIDENCE_NOT_AVAILABLE",source)
        self.assertIn("wheel_bytes_verified",source)
        self.assertIn("installation_performed",source)
if __name__=="__main__":unittest.main(verbosity=2)
