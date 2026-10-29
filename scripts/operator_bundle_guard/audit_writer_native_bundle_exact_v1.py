#!/usr/bin/env python3
"""Independent byte-level audit of exact canonical OFFLINE evidence writer ZIP."""
import hashlib
import json
import re
from pathlib import Path
import zipfile

PKG="stm32_os_evidence_writer_native_offline_v1_20261028"
EXPECTED="3603821E93217EE9617193D70E6868BFE933A82CDD175E2D7A3A314761F9DD93"
ROOT=Path(__file__).resolve().parents[3]
def hashblob(b):return hashlib.sha256(b).hexdigest().upper()
def need(b,reason):
    if not b:raise AssertionError(reason)
def main():
    source=ROOT/"Bundle"/("_stage_"+PKG)
    target=ROOT/"Bundle"/(PKG+".zip")
    b=target.read_bytes()
    need(len(b)==8192,"ZIP_BYTE_SIZE")
    need(hashblob(b)==EXPECTED,"WHOLE_ZIP_SHA")
    with zipfile.ZipFile(target) as z:
        names=z.namelist()
        exact={"manifest.json","hashes.sha256","apply.ps1","PACKAGE_README.txt"}
        need(len(names)==4 and len(set(names))==4 and set(names)==exact,"MEMBER_NAMES")
        need(z.testzip() is None,"ZIP_CRC")
        items={n:z.read(n) for n in names}
        need(all(not x.is_dir() and x.flag_bits & 1==0 for x in z.infolist()),
             "UNSAFE_ENTRY")
    rows=items["hashes.sha256"].decode("ascii").splitlines()
    need(len(rows)==3,"INDEX_COUNT")
    checked={}
    for row in rows:
        match=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",row)
        need(match is not None,"INDEX_FORMAT")
        name=match[2]
        need(name in items and name!="hashes.sha256" and name not in checked,
             "INDEX_ENTRY")
        need(hashblob(items[name])==match[1],"INDEX_SHA_DRIFT")
        checked[name]=match[1]
    need(set(checked)==set(items)-{"hashes.sha256"},"INDEX_COVERAGE")
    manifest=json.loads(items["manifest.json"],object_pairs_hook=_strict_pairs)
    spec=json.loads((source/"bundle_spec.json").read_bytes())
    need(manifest["schema"]=="DEUS_OS_OPERATOR_BUNDLE_V2" and
         manifest["package_id"]==PKG and
         manifest["runner_result_contract"]=="DEUS_OS_RUNNER_RESULT_V1" and
         manifest["execution_profile"]=="OFFLINE","EXACT_MANIFEST_CONTRACT")
    need(manifest["apply_sha256"]==hashblob(items["apply.ps1"]),"APPLY_MANIFEST_SHA")
    need(all(manifest[k]==v for k,v in spec.items()),"MANIFEST_SPEC_DRIFT")
    need(items["apply.ps1"]==(source/"apply.ps1").read_bytes(),"LIVE_APPLY_SOURCE_DRIFT")
    need(items["PACKAGE_README.txt"]==(source/"PACKAGE_README.txt").read_bytes(),
         "README_BYTE_DRIFT")
    need("Set-StrictMode -Version 3" in items["apply.ps1"].decode(),"STRICT")
    print(json.dumps({"result":"INDEPENDENT_CANONICAL_BUNDLE_V2_STATIC_PASS",
        "package_id":PKG,"zip_bytes":len(b),"whole_sha256":EXPECTED,
        "crc":"4/4","indexed":"3/3","apply_sha256":manifest["apply_sha256"],
        "source_matches":True,"execution_profile":"OFFLINE",
        "windows_native":"NOT_RUN"},sort_keys=True))
def _strict_pairs(pairs):
    out={}
    for k,v in pairs:
        need(k not in out,"DUPLICATE_JSON_KEY")
        out[k]=v
    return out
if __name__=="__main__":main()
