#!/usr/bin/env python3
"""Independent auditor negative tests; synthetic in-memory ZIPs only, not native proof."""
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile
import audit_a05_controller_isolated_deny_v2 as m
import audit_a05_controller_ps_hashtable_keys_v1 as key_audit

def digest(x):return hashlib.sha256(x).hexdigest().upper()
def fixture(terminal=True):
    outcome={
     "schema":"DEUS_A05_CONTROLLER_ISOLATED_NATIVE_DENY_EVIDENCE_V1",
     "gate":m.ID,"result":"FAIL",
     "failure_phase":"NATIVE_ISOLATED_CONTROLLER_DENY_PROBE",
     "classification":"A05_CONTROLLER_ISOLATED_NATIVE_STOP",
     "terminal_exception":"RuntimeError: ModuleNotFoundError: No module named 'cryptography'",
     "worker_sha256":m.WORKER,"original_host_tree_sha256":m.TREE,
     "original_deploy_evidence_sha256":m.DEPLOY,"expected_receipt_sha256":m.RECEIPT,"receipt_sha256":"NONE",
     "controller_release":"DENIED","nonce_consumed":"NO","device_io":"NONE",
     "installation":"NONE","network":"NO","gate4":"NOT_AUTHORIZED"}
    transcript="G4_A05_CHILD_EXIT=2\n"
    if terminal:
        transcript+=("CHILD_STDOUT=TERMINAL_EXCEPTION=ModuleNotFoundError: "
                     "No module named 'cryptography'\n"
                     "TERMINAL_EXCEPTION=ModuleNotFoundError: No module named 'cryptography'\n")
    data={
      "outcome.txt":(json.dumps(outcome)+"\n").encode(),
      "native_checks.txt":b"PASS PRESTATE\n",
      "native_test_output.txt":transcript.encode(),
      "run.log":b"RESULT=FAIL\n"}
    data["sizes.txt"]=("\n".join(f"{len(v)}  {k}" for k,v in data.items())+"\n").encode()
    data["hashes.sha256"]=("\n".join(f"{digest(v)}  {k}" for k,v in data.items())+"\n").encode()
    f=io.BytesIO()
    with zipfile.ZipFile(f,"w",compression=zipfile.ZIP_DEFLATED) as z:
        for k,v in data.items():z.writestr(k,v)
    return f.getvalue()
class OriginalAuditorTests(unittest.TestCase):
 def test_v2_keys_pass_in_clean_checkout_without_historical_v1(self):
  with tempfile.TemporaryDirectory() as d:
   capture=io.StringIO()
   with patch.object(key_audit,"OLD",Path(d)/"missing_historical_v1.ps1"),patch("sys.stdout",capture):
    key_audit.audit()
   result=json.loads(capture.getvalue())
   self.assertEqual(result["result"],"A05_CONTROLLER_V2_HASH_KEYS_STATIC_PASS")
   self.assertEqual(result["historical_v1_source"],
                    "HISTORICAL_V1_OFF_GIT_NOT_ATTESTED_SYNTHETIC_NEGATIVE_ONLY")
   self.assertEqual(result["historical_v1_duplicate_keys"],["receipt_sha256"])
   self.assertEqual((result["v2_typed_literal_keys"],result["v2_record_literal_keys"]),(32,19))
 def test_present_tampered_historical_v1_not_silently_accepted(self):
  with tempfile.TemporaryDirectory() as d:
   fake=Path(d)/"historical_v1.ps1"
   fake.write_text("NOT_THE_PINNED_ORIGINAL",encoding="utf-8")
   with patch.object(key_audit,"OLD",fake):
    with self.assertRaisesRegex(AssertionError,"HISTORICAL_V1_ORIGINAL_DRIFT"):
     key_audit.audit()
 def test_missing_original_rejected(self):
  with self.assertRaisesRegex(AssertionError,"ORIGINAL_EVIDENCE_NOT_AVAILABLE"):
   m.evidence(m.ID+"_20990101_000000_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.evidence.zip","A"*64)
 def test_exact_hash_required(self):
  with self.assertRaisesRegex(AssertionError,"ORIGINAL_EVIDENCE_SHA_FORMAT"):
   m.evidence(m.ID+"_20990101_000000_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.evidence.zip","wrong")
 def test_index_integrity_positive(self):
  with zipfile.ZipFile(io.BytesIO(fixture())) as z:
   data={n:z.read(n) for n in z.namelist()}
  self.assertEqual(m.test_index(data,"hashes.sha256","sizes.txt"),5)
 def test_index_tamper_denied(self):
  with zipfile.ZipFile(io.BytesIO(fixture())) as z:
   data={n:z.read(n) for n in z.namelist()}
  data["run.log"]=b"TAMPER"
  with self.assertRaisesRegex(AssertionError,"HASH_INDEX_VALUE"):
   m.test_index(data,"hashes.sha256","sizes.txt")
 def test_real_style_fail_preserves_terminal_exception(self):
  name=m.ID+"_20261029_123456_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.evidence.zip"
  with tempfile.TemporaryDirectory() as temp:
   base=Path(temp)
   (base/"Evidence").mkdir()
   original=base/"Evidence"/name
   blob=fixture(terminal=True)
   original.write_bytes(blob)
   with patch.object(m,"ROOT",base):
    r=m.evidence(name,digest(blob))
   self.assertEqual(r["result"],"A05_NATIVE_ORIGINAL_CLASSIFIED_FAIL")
   self.assertEqual(r["phase"],"NATIVE_ISOLATED_CONTROLLER_DENY_PROBE")
 def test_incomplete_fail_diagnostic_cannot_pass(self):
  name=m.ID+"_20261029_123456_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.evidence.zip"
  with tempfile.TemporaryDirectory() as temp:
   base=Path(temp)
   (base/"Evidence").mkdir()
   original=base/"Evidence"/name
   blob=fixture(terminal=False)
   original.write_bytes(blob)
   with patch.object(m,"ROOT",base):
    with self.assertRaisesRegex(AssertionError,"FAIL_DIAGNOSTICS_INCOMPLETE"):
     m.evidence(name,digest(blob))
if __name__=="__main__":
 unittest.main(verbosity=2)
