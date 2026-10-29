#!/usr/bin/env python3
"""Source-only negative regression for future independent ORIGINAL Windows evidence audit."""
import hashlib
import io
import json
import unittest
import zipfile
import zlib

from pathlib import Path
from contextlib import redirect_stdout
from audit_writer_native_original_v1 import inspect, review, ROOT, PKG

def h(blob):return hashlib.sha256(blob).hexdigest().upper()
def payload():
    members={
      "outcome.txt":b'{"result":"FAIL","error":"SIMULATED"}\n',
      "native_checks.txt":b"PASS GENERIC_TEST\n",
      "run.log":b"TEST=YES\n",
      "native_test_output.txt":b"CHILD_EXIT=17\nTERMINAL_EXCEPTION=RuntimeError: synthetic\n"
    }
    members["sizes.txt"]="".join(
        str(len(v))+"  "+k+"\n" for k,v in members.items()
    ).encode()
    members["hashes.sha256"]="".join(
        h(v)+"  "+k+"\n" for k,v in members.items()
    ).encode()
    return members
def pack(m):
    b=io.BytesIO()
    with zipfile.ZipFile(b,"w",zipfile.ZIP_DEFLATED) as z:
        for k,v in m.items():z.writestr(k,v)
    return b.getvalue()
names={"outcome.txt","native_checks.txt","run.log","native_test_output.txt","sizes.txt","hashes.sha256"}
class AuditTests(unittest.TestCase):
    def assertInvalid(self,blob,reason):
        with self.assertRaisesRegex(AssertionError,reason):
            inspect(blob,names)
    def test_exact_complete_zip_passes_structure(self):
        m=payload()
        result=inspect(pack(m),names)
        self.assertEqual(set(result),names)
    def test_index_tamper_rejected(self):
        m=payload()
        m["native_test_output.txt"]+=b"tamper\n"
        self.assertInvalid(pack(m),"SHA_INDEX_WRONG")
    def test_index_member_missing_rejected(self):
        m=payload()
        m.pop("run.log")
        self.assertInvalid(pack(m),"MEMBER_SET")
    def test_size_index_tamper_rejected(self):
        m=payload()
        m["sizes.txt"]=m["sizes.txt"].replace(b"PASS",b"FAIL") if b"PASS" in m["sizes.txt"] else m["sizes.txt"]+b"999  outcome.txt\n"
        # Index changes independently: fail-closed, never falsely pass.
        self.assertInvalid(pack(m),"SHA_INDEX_WRONG|SIZES_COUNT")
    def test_duplicate_member_rejected(self):
        import warnings
        blob=io.BytesIO()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore",UserWarning)
            with zipfile.ZipFile(blob,"w") as z:
                for k,v in payload().items():z.writestr(k,v)
                z.writestr("outcome.txt",b"bad")
        self.assertInvalid(blob.getvalue(),"MEMBER_SET")
    def test_real_owner_native_evidence_full_audit_when_available(self):
        # Original off-Git Windows Evidence is optional in a clean CI checkout.
        # When present, this regression checks exact outer/inner indexed bytes,
        # the actual CRLF eight-case transcript, and final terminal exception.
        basename=(
            "stm32_os_evidence_writer_native_offline_v1_20261028_"
            "20261028_225007_bdf95aa0c2ea4bd3ba4f3afdda259b53.evidence.zip"
        )
        expected="63D9D6567A98979DD4C2D2DE11677AC52039E2117A8B2A980F9BA92CFFE4AB6D"
        original=ROOT/"Evidence"/basename
        if not original.is_file():
            self.skipTest("Original owner Windows Evidence stays off Git")
        self.assertEqual(h(original.read_bytes()),expected)
        output=io.StringIO()
        with redirect_stdout(output):
            review(basename,expected)
        self.assertIn("ORIGINAL_WINDOWS_WRITER_NATIVE_SCOPED_PASS",output.getvalue())
        self.assertIn('"cases": "8/8"',output.getvalue())

    def test_bad_crc_rejected(self):
        raw=io.BytesIO()
        with zipfile.ZipFile(raw,"w",zipfile.ZIP_STORED) as z:
            for k,v in payload().items():z.writestr(k,v)
        blob=bytearray(raw.getvalue())
        with zipfile.ZipFile(io.BytesIO(blob)) as z:
            item=z.getinfo("native_test_output.txt")
            at=item.header_offset+30+len(item.filename.encode("utf-8"))+len(item.extra)
        # Stored entry: definitely changes the uncompressed payload while
        # keeping the original central-directory CRC.
        self.assertEqual(blob[at],ord("C"))
        blob[at]=ord("X")
        with self.assertRaises((zipfile.BadZipFile,AssertionError,zlib.error,EOFError)):
            inspect(bytes(blob),names)
if __name__=="__main__":
    unittest.main(verbosity=2)
