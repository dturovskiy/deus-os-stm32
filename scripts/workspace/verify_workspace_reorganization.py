#!/usr/bin/env python3
"""Independently verify exact-byte STM32 workspace custody and the Git boundary."""
from pathlib import Path
import hashlib
import json
import subprocess
import zipfile
ROOT=Path(__file__).resolve().parents[3]
OS=ROOT/"OS"
REPORTS=ROOT/"Reports/Audits"
def file_sha(p):
 h=hashlib.sha256()
 with p.open("rb") as f:
  for blob in iter(lambda:f.read(131072),b""):h.update(blob)
 return h.hexdigest().upper()
def require(ok,msg):
 if not ok:raise RuntimeError(msg)
def verify_entry(src,dst,expected_size,expected_hash,moved,check_crc=False):
 a=ROOT/src
 b=ROOT/dst
 require(not a.is_symlink() and not b.is_symlink(),"SYMLINK_FOUND "+src)
 if moved:
  require(not a.exists(),"SOURCE_STILL_PRESENT "+src)
  require(b.is_file(),"ARCHIVED_TARGET_MISSING "+dst)
  f=b
 else:
  require(a.is_file(),"RETAINED_SOURCE_MISSING "+src)
  require(dst==src,"RETAINED_PATH_MISMATCH "+src)
  f=a
 require(f.stat().st_size==expected_size,"SIZE_MISMATCH "+src)
 require(file_sha(f)==expected_hash,"SHA_MISMATCH "+src)
 if check_crc:
  with zipfile.ZipFile(f) as z:
   require(z.testzip() is None,"ORIGINAL_ZIP_CRC "+dst)
def main():
 a=json.loads((REPORTS/"WORKSPACE_REORGANIZATION_MANIFEST_20261029.json").read_text())
 b=json.loads((REPORTS/"WORKSPACE_MISC_CUSTODY_REORGANIZATION_20261029.json").read_text())
 c=json.loads((REPORTS/"WORKSPACE_BUNDLE_HISTORICAL_CUSTODY_20261029.json").read_text())
 states={}
 for r in a["records"]:
  require(r["state"] in ("MOVED_VERIFIED","RETAINED_VERIFIED"),"SOURCE_UNCLOSED")
  moved=r["state"]=="MOVED_VERIFIED"
  verify_entry(r["source"],r["target"],r["bytes"],r["sha256"],moved)
  states[r["state"]]=states.get(r["state"],0)+1
 require(len(a["records"])==132 and states.get("MOVED_VERIFIED")==30 and states.get("RETAINED_VERIFIED")==102,"SOURCE_COUNTS_DRIFT")
 for r in b["records"]:
  require(r["state"]=="MOVED_VERIFIED","EXTERNAL_NOT_CLOSED")
  verify_entry(r["src"],r["dest"],r["bytes"],r["sha256"],True)
 require(len(b["records"])==14,"EXTERNAL_COUNT_DRIFT")
 for r in c["records"]:
  require(r["state"]=="MOVED_VERIFIED","BUNDLE_NOT_CLOSED")
  verify_entry(r["source"],r["dest"],r["bytes"],r["sha256"],True,True)
 require(len(c["records"])==93,"BUNDLE_COUNT_DRIFT")
 for p in ROOT.iterdir():
  require(p.is_dir() or p.name=="README.md","EXTRANEOUS_WORKSPACE_ROOT_FILE "+p.name)
 require(sorted(p.name for p in (ROOT/"Logs").iterdir())==["log-1.log"],"LOGS_NOT_ONLY_LOG")
 expected_sensitive={
  "ProtectedRecovery/deus_os_000e_management_only_tree081aac70_signed_v6.pkg":"2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED",
  "ProtectedRecovery/deus_os_rev5_000c_recovery_future_floor7_v1.pkg":"C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A",
  "Evidence/stm32_os_bench_signed_v6_once_v3_20261029_20261029_195412_bc5f39397688412eb568d5da352ba8c6.evidence.zip":"95EC84EFF962B91C41C4C88D7DDD9A6EDE5EE8700987BEB7DF29879B39F85695",
  "Evidence/stm32_os_bench_v6_receipt_adjudication_v1_20261029_20261029_201340_dbe8609d154448559c64075d7278c0ef.evidence.zip":"A596E9B5DC6ECFC0BEC35B771C79FB979DDBAEB830EBF2E72EA82E2975B7F943",
 }
 for name,sha in expected_sensitive.items():
  p=ROOT/name
  require(p.is_file() and file_sha(p)==sha,"PRESERVED_SOURCE_DRIFT "+name)
 git=subprocess.run(["git","diff","--check"],cwd=OS,capture_output=True,text=True,timeout=12)
 require(git.returncode==0,"GIT_DIFF_CHECK_FAIL")
 gitstat=subprocess.run(["git","status","--short","--untracked-files=all"],cwd=OS,capture_output=True,text=True,timeout=15)
 require(gitstat.returncode==0,"GIT_STATUS_FAIL")
 print(json.dumps({"result":"PASS_WORKSPACE_CUSTODY_VERIFIED","source_original_total":len(a["records"]),"source_archived":states["MOVED_VERIFIED"],"source_retained":states["RETAINED_VERIFIED"],"external_moved":len(b["records"]),"bundle_zip_archived_crc_pass":len(c["records"]),"cross_boundary_preserved_packages_evidence":len(expected_sensitive),"workspace_root":"DIRECTORIES_AND_README_ONLY","logs":"ONLY_LOG_FILES","git_diff_check":"PASS","git_dirty_entries":len([x for x in gitstat.stdout.splitlines() if x]),"no_git_commit_push":"NO_ACTION"}))
if __name__=="__main__":main()
