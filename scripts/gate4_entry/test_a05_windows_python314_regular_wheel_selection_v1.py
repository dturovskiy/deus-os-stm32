#!/usr/bin/env python3
"""Regression of genuine native CPython 3.14 regular-wheel choice; no wheel binary install."""
import hashlib
import json
from pathlib import Path
import unittest

import audit_a05_windows_wheelhouse_v1 as guard

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
SELECTION=HERE/"a05_windows_python314_regular_wheel_selection_v1.json"
METADATA=HERE/"a05_windows_python_dependency_candidates_v1.json"
EVIDENCE=ROOT/"Evidence"/("stm32_os_gate4_a05_python_dependency_native_inventory_v1_20261028_"
                         "20261028_231409_35ab14b1808d416b82bb243bdfe3a4a3.evidence.zip")
ORIGINAL="7EFBC88F2F8678E5E1F6BC1DCCD3B7558A4A4789076266D12BDF69810AB24CF2"
def sha(blob):return hashlib.sha256(blob).hexdigest().upper()
class SelectionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.s=json.loads(SELECTION.read_text(encoding="utf-8"))
        cls.original=json.loads(METADATA.read_text(encoding="utf-8"))

    def test_original_owner_windows_evidence_unchanged_when_available(self):
        if not EVIDENCE.is_file():
            self.skipTest("Original owner Evidence off Git; source SHA gate stays")
        self.assertEqual(sha(EVIDENCE.read_bytes()),ORIGINAL)
        self.assertEqual(self.s["owner_windows_inventory"]["original_evidence_sha256"],ORIGINAL)

    def test_accepted_regular_cpython314_abi(self):
        x=self.s["owner_windows_inventory"]
        self.assertEqual(x["original_auditor_result"],"A05_WINDOWS_PYTHON_INVENTORY_ORIGINAL_SCOPED_PASS")
        self.assertEqual((x["python_version"],x["implementation"],x["architecture_bits"],x["gil_disabled"]),
                         ("3.14.0","CPython",64,False))
        self.assertEqual(x["target_wheel_abi"],"cp314")
        self.assertEqual(x["pip_version"],"25.2")
        self.assertEqual(x["installed_packages"],{"cryptography":None,"cffi":None,"pycparser":None})

    def test_selected_exactly_three_binary_candidates(self):
        names=[
          "cryptography-50.0.2-cp311-abi3-win_amd64.whl",
          "cffi-2.1.1-cp314-cp314-win_amd64.whl",
          "pycparser-3.12-py3-none-any.whl"
        ]
        self.assertEqual([w["filename"] for w in self.s["selected_wheels"]],names)
        self.assertTrue(all("cp314t" not in n for n in names))

    def test_exact_published_metadata_sha_matches_original_lock(self):
        self.assertEqual(sha(METADATA.read_bytes()),self.s["authority_source"]["candidate_metadata_sha256"])
        pin={x["filename"]:x["sha256"] for x in self.original["wheels"]}
        self.assertEqual({x["filename"]:x["sha256"] for x in self.s["selected_wheels"]},
                         {n:pin[n] for n in pin if n in {z["filename"] for z in self.s["selected_wheels"]}})

    def test_no_wheel_bytes_or_install_are_claimed(self):
        self.assertEqual(self.s["qualification"],
                         "ORIGINAL_WINDOWS_NATIVE_ABI_CONFIRMED_METADATA_WHEEL_SELECTION_ONLY")
        state=self.s["status"]
        for k in ("all_3_exact_wheel_files_present","binary_sha_verified",
                  "zip_crc_verified","wheel_record_verified",
                  "owner_host_installation_authorized","owner_host_installation_performed",
                  "controller_import_pass"):
            self.assertIs(state[k],False)

    def test_real_wheel_guard_only_with_actual_three_files(self):
        self.assertEqual(guard.LOCK.resolve(),METADATA.resolve())
        self.assertEqual(self.s["owner_windows_inventory"]["target_wheel_abi"],"cp314")
if __name__=="__main__":
    unittest.main(verbosity=2)
