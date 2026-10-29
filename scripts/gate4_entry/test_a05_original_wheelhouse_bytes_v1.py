#!/usr/bin/env python3
"""Regression: original owner ZIP is optional in clean CI, mandatory when present.

Synthetic negatives run without Python dependency installation or network.
"""
import io
from pathlib import Path
import tempfile
import unittest
import zipfile
import hashlib

import audit_a05_original_wheelhouse_bytes_v1 as audit
ROOT=Path(__file__).resolve().parents[3]

class WheelBytesTests(unittest.TestCase):
    def test_original_owner_zip_if_available(self):
        exact=ROOT/"Wheelhouse"/audit.WHEEL_NAME
        evidence=ROOT/"Evidence"/audit.ORIGINAL_EVIDENCE_NAME
        if not exact.is_file() or not evidence.is_file():
            self.skipTest("Original owner off-Git Wheelhouse+Evidence absent in clean CI")
        result=audit.inspect(audit.WHEEL_NAME,audit.WHEEL_SHA)
        self.assertEqual(result["result"],"ORIGINAL_WHEELHOUSE_BYTES_INDEPENDENT_FULL_PASS")
        self.assertEqual(result["outer_crc"],"4/4")
        self.assertEqual(result["index"],"3/3")
        self.assertEqual(len(result["wheels"]),3)
        self.assertEqual(sum(x["record_members"] for x in result["wheels"]),164)

    def test_wrong_owner_sha_rejected(self):
        with self.assertRaisesRegex(AssertionError,"ORIGINAL_WHEELHOUSE_ID_DRIFT"):
            audit.inspect(audit.WHEEL_NAME,"A"*64)

    def test_unsafe_zip_name_rejected(self):
        self.assertFalse(audit.safe_name("../escape.txt"))
        self.assertFalse(audit.safe_name("/absolute.txt"))
        self.assertFalse(audit.safe_name("dir\\escape"))
        self.assertFalse(audit.safe_name(".hidden"))
        self.assertTrue(audit.safe_name("cryptography-50.0.2.dist-info/RECORD"))

    def test_nested_zip_bad_crc_rejected(self):
        blob=io.BytesIO()
        with zipfile.ZipFile(blob,"w",zipfile.ZIP_STORED) as z:
            z.writestr("item.txt",b"ABC")
        data=bytearray(blob.getvalue())
        with zipfile.ZipFile(io.BytesIO(data)) as z:
            entry=z.getinfo("item.txt")
            offset=entry.header_offset+30+len(entry.filename)+len(entry.extra)
        data[offset]=ord("X")
        with self.assertRaises((zipfile.BadZipFile,AssertionError)):
            audit.zip_data(bytes(data))

    def test_duplicate_inner_entries_rejected(self):
        import warnings
        b=io.BytesIO()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore",UserWarning)
            with zipfile.ZipFile(b,"w",zipfile.ZIP_STORED) as z:
                z.writestr("same.txt",b"A")
                z.writestr("same.txt",b"B")
        with self.assertRaisesRegex(AssertionError,"DUPLICATE_OR_UNBOUNDED_ZIP_MEMBER"):
            audit.zip_data(b.getvalue())

if __name__=="__main__":
    unittest.main(verbosity=2)
