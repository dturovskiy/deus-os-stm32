#!/usr/bin/env python3
"""No mutation source tests for owner-local CPython3.14 isolated target operator."""
import ast
from pathlib import Path
import hashlib
import os
import re
import tempfile
import unittest
from unittest.mock import patch

import deploy_a05_isolated_cp314_wheels_v1 as dep

HERE=Path(__file__).resolve().parent
class IsolatedSourceTests(unittest.TestCase):
    def test_no_linux_install_allowed(self):
        if os.name=="nt":
            self.skipTest("Linux-only fail-closed Windows guard")
        with patch.object(dep.sys,"argv",["script","a","b","c","d","e"]):
            self.assertEqual(dep.main(),2)

    def test_probe_compiles_but_does_not_execute(self):
        ast.parse(dep.PROBE_CODE)
        self.assertIn("A05_ISOLATED_CONTROLLER_IMPORT=PASS",dep.PROBE_CODE)
        self.assertIn("CONTROLLER_SOURCE_PATH_MISMATCH_",dep.PROBE_CODE)
        self.assertIn("CRYPTOGRAPHY_OUTSIDE_ISOLATED_TARGET",dep.PROBE_CODE)
        self.assertIn("CFFI_BINARY_OUTSIDE_TARGET",dep.PROBE_CODE)
        self.assertIn("sys.path.insert(0,str(sources))",dep.PROBE_CODE)

    def test_exact_accepted_source_and_wheel_pins(self):
        self.assertEqual(len(dep.EXPECTED),3)
        self.assertEqual({x[0] for x in dep.EXPECTED},
                         {"cryptography","cffi","pycparser"})
        self.assertEqual(len(dep.MODULES),4)
        for name,pin in dep.MODULES.items():
            self.assertEqual(hashlib.sha256((HERE/name).read_bytes()).hexdigest().upper(),pin)
        self.assertEqual(dep.WHEELHOUSE_SHA,
             "CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C")

    def test_sanitizer_preserves_terminal_exception_tail(self):
        tail="ModuleNotFoundError: No module named 'cryptography'"
        blob=("X"*2500)+tail
        out=dep.sanitized(blob,420)
        self.assertIn(tail,out)
        self.assertIn("[MIDDLE_TRUNCATED_TAIL_PRESERVED]",out)
        self.assertLessEqual(len(out),420)

    def test_child_launch_failure_has_specific_stop(self):
        with self.assertRaisesRegex(dep.Stop,"ChildLaunchError: MOCK_"):
            dep.bounded(["/definitely_missing_py3_bin"],os.environ.copy(),1,"MOCK")

    def test_target_tree_deterministic_and_no_symlink(self):
        with tempfile.TemporaryDirectory() as temp:
            d=Path(temp)
            f=d/"one.txt"
            f.write_bytes(b"fixture")
            h,n=dep.compute_tree(d)
            self.assertEqual(n,1)
            self.assertRegex(h,r"^[A-F0-9]{64}$")
            self.assertEqual((h,n),dep.compute_tree(d))

    def test_no_download_or_global_python_mutating_api(self):
        body=(HERE/"deploy_a05_isolated_cp314_wheels_v1.py").read_text()
        for banned in ("urllib.request","requests.get","pip install -U",
                       "pip install --user","Set-MpPreference","ssh ","st-flash"):
            self.assertNotIn(banned,body)
        for required in ("--no-index","--no-deps","--no-compile",
                         "--no-cache-dir","--target","--isolated",
                         "shutil.rmtree","PYTHONPATH","HostDependencies",
                         "CHILD_PY_FINAL_TARGET_IMPORT"):
            if required=="HostDependencies":
                # Host path role owned by PowerShell wrapper, not worker.
                continue
            self.assertIn(required,body)
        self.assertIn('host_root.name=="HostDependencies"',body)
if __name__=="__main__":
    unittest.main(verbosity=2)
