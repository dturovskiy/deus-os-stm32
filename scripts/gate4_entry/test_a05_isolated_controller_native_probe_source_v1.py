#!/usr/bin/env python3
"""No Windows launch, network, nonce or host mutation: native probe source tests."""
import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import native_a05_isolated_controller_probe_v1 as m
ROOT=Path(__file__).resolve().parents[3]
class CandidateTests(unittest.TestCase):
 def test_accepted_original_read_only(self):
  parent=ROOT
  t=parent/"HostDependencies"/m.TARGET_NAME/"python_target"
  e=parent/"Evidence"/m.EVIDENCE_BASENAME
  if not t.is_dir() or not e.is_file():
   self.skipTest("Owner off-Git original not in clean CI")
  digest,count=m.validate_original(parent,t)
  self.assertEqual(digest,m.TREE_PIN)
  self.assertEqual(count,174)
 def test_missing_original_rejected(self):
  with tempfile.TemporaryDirectory() as d:
   with self.assertRaisesRegex(RuntimeError,"ORIGINAL_ADMISSION_RECEIPT_MISSING"):
    m.validate_original(Path(d),Path(d)/"missing"/"python_target")
 def test_tampered_receipt_rejected(self):
  with tempfile.TemporaryDirectory() as d:
   parent=Path(d)
   target=parent/"HostDependencies"/m.TARGET_NAME/"python_target"
   target.mkdir(parents=True)
   (target.parent/"ADMISSION.json").write_bytes(b"changed")
   with self.assertRaisesRegex(RuntimeError,"ORIGINAL_ADMISSION_RECEIPT_SHA"):
    m.validate_original(parent,target)
 def test_source_sha_pins_live(self):
  src=ROOT/"OS/scripts/gate4_entry"
  m.check_sources(src)
  self.assertEqual(len(m.SOURCES),4)
 def test_source_drift_rejected(self):
  with tempfile.TemporaryDirectory() as d:
   for name in m.SOURCES:
    (Path(d)/name).write_bytes(b"WRONG")
   with self.assertRaisesRegex(RuntimeError,"LIVE_CONTROLLER_SOURCE_SHA"):
    m.check_sources(Path(d))
 def test_no_windows_runtime_pass_from_linux(self):
  if m.sys.platform=="win32":self.skipTest("Windows-only source negative")
  with self.assertRaisesRegex(RuntimeError,"NOT_WINDOWS_CP314_REGULAR"):
   m.validate_runtime(Path("/tmp/no-target"),Path("/tmp/no-source"))
 def test_missing_config_denial_with_real_controller_on_linux(self):
  import owner_windows_release_controller_v1 as c
  with tempfile.TemporaryDirectory() as d:
   # Platform guard is unrelated to missing-config denial; model Windows
   # solely in-memory without invoking any process or protected state.
   with patch.object(c.sys,"platform","win32"):
    m.exercise_denied(c,Path(d)/"missing.json")
 def test_terminal_errors_are_preserved(self):
  with patch.object(m.sys,"argv",["test_script"]):
   stream=io.StringIO()
   with patch("sys.stdout",stream):
    self.assertEqual(m.main(),2)
   text=stream.getvalue()
   self.assertIn("TERMINAL_EXCEPTION=RuntimeError: ARGUMENT_VECTOR_REQUIRED",text)
   self.assertIn("DEUS_A05_CONTROLLER_NATIVE_JSON=",text)
   self.assertIn('"authorization":"DENIED"',text)
 def test_no_released_hardware_or_nonce_operation(self):
  body=Path(m.__file__).read_text()
  for phrase in ("flash(","subprocess.run(","urllib.request","socket.connect(","Write-G4ConsumedNonce",
                 "begin_update(","execute_hardware("):
   self.assertNotIn(phrase,body)
  self.assertIn('"release_admitted":"NO"',body)
  self.assertIn('"gate4":"NOT_AUTHORIZED"',body)
 def test_windows_order_matches_original(self):
  if not (ROOT/"HostDependencies"/m.TARGET_NAME/"python_target").is_dir():
   self.skipTest("Original target absent")
  digest,count=m.tree_hash(ROOT/"HostDependencies"/m.TARGET_NAME/"python_target")
  self.assertEqual((digest,count),(m.TREE_PIN,174))
if __name__=="__main__":unittest.main(verbosity=2)
