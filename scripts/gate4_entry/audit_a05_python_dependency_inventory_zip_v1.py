#!/usr/bin/env python3
"""Independent original-input ZIP verifier of A05 Python inventory canonical bundle v2."""
import hashlib
import json
import re
from pathlib import Path
import zipfile

ID="stm32_os_gate4_a05_python_dependency_native_inventory_v1_20261028"
SHA="65CC3423E3BC0C96CBBF8D2887C6AC3CC18FFD1A845A5D66ADF37751EB8C519C"
ROOT=Path(__file__).resolve().parents[3]
def hashit(x): return hashlib.sha256(x).hexdigest().upper()
def need(b,msg):
    if not b:raise AssertionError(msg)
def nodup(ps):
    d={}
    for k,v in ps:
        need(k not in d,"MANIFEST_DUPLICATE_KEY")
        d[k]=v
    return d
def audit():
    p=ROOT/"Bundle"/(ID+".zip")
    need(p.is_file() and not p.is_symlink(),"MISSING_FINAL_ZIP")
    b=p.read_bytes()
    need(hashit(b)==SHA,"BUNDLE_WHOLE_SHA")
    exact={"apply.ps1","probe_windows_python_dependency_inventory_v1.py",
           "a05_windows_python_dependency_candidates_v1.json",
           "PACKAGE_README.txt","hashes.sha256","manifest.json"}
    with zipfile.ZipFile(p) as z:
        names=z.namelist()
        need(set(names)==exact and len(names)==len(exact),"ZIP_MEMBER_SET")
        need(z.testzip() is None,"ZIP_MEMBER_CRC")
        data={n:z.read(n) for n in names}
    hashes=data["hashes.sha256"].decode().splitlines()
    seen=set()
    for line in hashes:
        m=re.fullmatch(r"([0-9A-F]{64})  ([A-Za-z0-9_./-]+)",line)
        need(m is not None and m[2] in data and m[2]!="hashes.sha256" and m[2] not in seen,
             "INDEX_ROW_OR_DUPLICATE")
        need(hashit(data[m[2]])==m[1],"INDEX_SHA")
        seen.add(m[2])
    need(seen==exact-{"hashes.sha256"} and len(seen)==5,"INDEX_COVERAGE")
    man=json.loads(data["manifest.json"],object_pairs_hook=nodup)
    need(man["schema"]=="DEUS_OS_OPERATOR_BUNDLE_V2" and man["package_id"]==ID and
         man["execution_profile"]=="OFFLINE" and
         man["runner_result_contract"]=="DEUS_OS_RUNNER_RESULT_V1",
         "BUNDLE_MANIFEST")
    need(man["apply_sha256"]==hashit(data["apply.ps1"]),"APPLY_MANIFEST_DRIFT")
    stage=ROOT/"Bundle"/("_stage_"+ID)
    for n in exact-{"hashes.sha256","manifest.json"}:
        need(data[n]==(stage/n).read_bytes(),"FINAL_STAGE_SOURCE_"+n)
    for n in ("probe_windows_python_dependency_inventory_v1.py",
              "a05_windows_python_dependency_candidates_v1.json"):
        need(data[n]==(ROOT/"OS/scripts/gate4_entry"/n).read_bytes(),
             "LIVE_REPO_PIN_"+n)
    print(json.dumps({"result":"A05_INVENTORY_FINAL_ZIP_INDEPENDENT_STATIC_PASS",
       "whole_sha256":SHA,"members":6,"crc":"6/6","index":"5/5",
       "apply_sha256":man["apply_sha256"],
       "source_pins":"2/2_ZIP_AND_REPO",
       "windows_native":"PENDING",
       "installation":"NONE"},sort_keys=True))
if __name__=="__main__":audit()
