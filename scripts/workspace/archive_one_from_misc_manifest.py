#!/usr/bin/env python3
"""Single-file SHA-verified archive for approved source-helper custody records.

Only entries explicitly present in the off-Git maintenance manifest are eligible.
No Git, network, firmware or hardware side effects.
"""
import hashlib
import json
import os
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[3]
LEDGER=ROOT/"Reports/Audits/WORKSPACE_MISC_CUSTODY_REORGANIZATION_20261029.json"
def sha(p):
 h=hashlib.sha256()
 with p.open("rb") as f:
  for block in iter(lambda:f.read(131072),b""):h.update(block)
 return h.hexdigest().upper()
def main():
 if len(sys.argv)!=2:raise ValueError("ONE_MANIFEST_SOURCE_PATH_REQUIRED")
 src=sys.argv[1]
 d=json.loads(LEDGER.read_text(encoding="utf-8"))
 hits=[r for r in d["records"] if r["src"]==src and r["state"]=="PLANNED_NOT_MOVED"]
 if len(hits)!=1:raise ValueError("NOT_A_SINGLE_APPROVED_UNMOVED_ENTRY")
 x=hits[0]
 if (x["reason"]!="ONE_TIME_IMPORT_GRAPH_RECOVERY_HELPER"
     or not src.startswith("OS/scripts/workspace/")
     or x["dest"]!="Archive/SourceHistory/WorkspaceReorganization/20261029/"+Path(src).name):
  raise ValueError("INVALID_CUSTODY_CLASS_OR_TARGET")
 a=ROOT/src
 b=ROOT/x["dest"]
 if a.is_symlink() or b.is_symlink() or b.exists() or not a.is_file():raise ValueError("SOURCE_DEST_CONFLICT")
 if a.stat().st_size!=x["bytes"] or sha(a)!=x["sha256"]:raise ValueError("SOURCE_NOT_EXACT")
 b.parent.mkdir(parents=True,exist_ok=True)
 a.rename(b)
 if a.exists() or not b.is_file() or b.stat().st_size!=x["bytes"] or sha(b)!=x["sha256"]:
  raise RuntimeError("POSTMOVE_CUSTODY_FAILED")
 x["state"]="MOVED_VERIFIED"
 tmp=LEDGER.with_suffix(".partial")
 with tmp.open("x",encoding="utf-8") as f:
  json.dump(d,f,indent=2)
  f.write("\n")
  f.flush()
  os.fsync(f.fileno())
 os.replace(tmp,LEDGER)
 print(json.dumps({"result":"MOVED_VERIFIED","source":src,"target":x["dest"],"sha256":x["sha256"]}))
if __name__=="__main__":main()
