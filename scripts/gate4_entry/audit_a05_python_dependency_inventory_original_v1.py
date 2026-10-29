#!/usr/bin/env python3
"""Read-only independent original Windows A-05 Python314 inventory Evidence audit."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import zipfile

ROOT=Path(__file__).resolve().parents[3]
ID="stm32_os_gate4_a05_python_dependency_native_inventory_v1_20261028"
BUNDLE_SHA="65CC3423E3BC0C96CBBF8D2887C6AC3CC18FFD1A845A5D66ADF37751EB8C519C"
PROBE_SHA="AACC99B0998B831330D543166C9CD72731B1E5D4F9B95E39E32110D8EBBD34B3"
LOCK_SHA="7D683C82DA2A75F7CA7975216BB2EF4E14105671B1A9889EA323A907D37526D1"
def sha(data):return hashlib.sha256(data).hexdigest().upper()
def need(ok,why):
    if not ok:raise AssertionError(why)
def strictly_unique(pairs):
    d={}
    for k,v in pairs:
        need(k not in d,"DUPLICATE_JSON_KEY")
        d[k]=v
    return d
def run(name,expected):
    need(name==Path(name).name and re.fullmatch(r"[A-Za-z0-9_.-]+",name),"BASENAME_ONLY")
    need(name.startswith(ID+"_") and name.endswith(".evidence.zip"),"SOURCE_ID")
    need(re.fullmatch(r"[0-9A-F]{64}",expected),"PIN_FORMAT")
    original=ROOT/"Evidence"/name
    need(original.is_file() and not original.is_symlink(),"ORIGINAL_EVIDENCE_NOT_AVAILABLE")
    blob=original.read_bytes()
    need(sha(blob)==expected,"EVIDENCE_SHA")
    need(sha((ROOT/"Bundle"/(ID+".zip")).read_bytes())==BUNDLE_SHA,"INPUT_ZIP_SHA")
    with zipfile.ZipFile(original) as z:
        ns=z.namelist()
        need(len(ns)==6 and len(ns)==len(set(ns)) and
             set(ns)=={"outcome.txt","native_checks.txt","run.log","native_test_output.txt",
                       "sizes.txt","hashes.sha256"},"MEMBER_SET")
        need(z.testzip() is None,"CRC")
        items={n:z.read(n) for n in ns}
    checks=items["hashes.sha256"].decode("ascii").splitlines()
    seen=set()
    for check in checks:
        m=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",check)
        need(m and m[2] in items and m[2]!="hashes.sha256" and m[2] not in seen,
             "INDEX_ROW")
        need(sha(items[m[2]])==m[1],"INDEX_SHA")
        seen.add(m[2])
    need(seen==set(items)-{"hashes.sha256"} and len(seen)==5,"INDEX_COVERAGE")
    sizeLines=items["sizes.txt"].decode("ascii").splitlines()
    sizeEntries=set()
    for line in sizeLines:
        m=re.fullmatch(r"([0-9]+)  ([A-Za-z0-9_.-]+)",line)
        need(m and m[2] in items and m[2] not in {"hashes.sha256","sizes.txt"}
             and m[2] not in sizeEntries,"SIZE_ROW")
        need(len(items[m[2]])==int(m[1]),"SIZE_MISMATCH")
        sizeEntries.add(m[2])
    need(sizeEntries==set(items)-{"hashes.sha256","sizes.txt"} and len(sizeEntries)==4,
         "SIZE_COVERAGE")
    outcome=json.loads(items["outcome.txt"].decode("utf-8"),object_pairs_hook=strictly_unique)
    need(outcome["gate"]==ID and outcome["result"]=="PASS" and
         outcome["classification"]=="G4_A05_WINDOWS_PYTHON_DEPENDENCY_INVENTORY_SCOPED_PASS" and
         outcome["failure_phase"]=="NONE" and outcome["error"]=="NONE","TYPED_RESULT")
    need(outcome["probe_sha256"]==PROBE_SHA and outcome["candidates_sha256"]==LOCK_SHA,
         "SOURCE_SHA")
    need(outcome["git_pre_sha256"]==outcome["git_post_sha256"] and
         re.fullmatch(r"[A-F0-9]{64}",outcome["git_pre_sha256"]),"GIT_POSTSTATE")
    need(outcome["installation"]=="NONE" and outcome["python_env_changed"]=="NO" and
         outcome["wheel_bytes_present"]=="NOT_ATTESTED" and
         outcome["device_io"]=="NONE" and outcome["nonce_ledger"]=="NOT_USED" and
         outcome["gate4"]=="NOT_AUTHORIZED","SCOPE")
    native=items["native_test_output.txt"].decode("utf-8")
    need("G4_A05_CHILD_EXIT=0" in native,"CHILD_EXIT")
    recordLines=[x[len("CHILD_STDOUT=G4_A05_PYTHON_INVENTORY_JSON="):]
                 for x in native.splitlines() if x.startswith("CHILD_STDOUT=G4_A05_PYTHON_INVENTORY_JSON=")]
    need(len(recordLines)==1,"ORIGINAL_INVENTORY_JSON_COUNT")
    info=json.loads(recordLines[0],object_pairs_hook=strictly_unique)
    need(info["schema"]=="DEUS_A05_PYTHON314_WINDOWS_DEP_INVENTORY_V1" and
         info["source_scope"]=="READ_ONLY_HOST_INVENTORY" and
         info["python314_win_amd64_candidate_match"] is True and
         info["platform"]=="win32" and info["implementation"]=="CPython" and
         info["version"].startswith("3.14.") and info["architecture_bits"]==64,
         "INTERPRETER_ABI_MISMATCH")
    need(info["dependency_install_performed"]=="NO" and info["network_access"]=="NO" and
         info["usb"]=="NO" and info["flash"]=="NO" and
         info["python_executable_path"]=="REDACTED","NO_MUTATION_OR_IDENTITY_LEAK")
    need(set(info["package_versions"])=={"cryptography","cffi","pycparser","pip"} and
         set(info["module_imports"])=={"cryptography","cffi","pycparser"},
         "PACKAGE_INVENTORY_MISSING")
    need(type(info["gil_disabled"]) is bool and
         type(info["recommended_wheel_filenames_METADATA_ONLY"]) is list,
         "ABI_PROOF")
    expectedNames=[
      ("cryptography-50.0.2-cp314-cp314t-win_amd64.whl",
       "cffi-2.1.1-cp314-cp314t-win_amd64.whl")
      if info["gil_disabled"] else
      ("cryptography-50.0.2-cp311-abi3-win_amd64.whl",
       "cffi-2.1.1-cp314-cp314-win_amd64.whl")
    ]
    need(info["recommended_wheel_filenames_METADATA_ONLY"] ==
         list(expectedNames[0])+["pycparser-3.12-py3-none-any.whl"],"WHEEL_TAG_BRANCH")
    tests=items["native_checks.txt"].decode("utf-8").splitlines()
    need(all(x.startswith("PASS ") for x in tests) and len(tests)>=8,"SCOPE_TESTS")
    missing=info["package_versions"]["cryptography"] is None
    print(json.dumps({
       "result":"A05_WINDOWS_PYTHON_INVENTORY_ORIGINAL_SCOPED_PASS",
       "original_evidence_sha256":expected,
       "crc":"6/6","sha_index":"5/5","size_index":"4/4",
       "python_version":info["version"],
       "gil_disabled":info["gil_disabled"],
       "cryptography_status":"MISSING" if missing else "PRESENT",
       "cffi_version":info["package_versions"]["cffi"],
       "pycparser_version":info["package_versions"]["pycparser"],
       "pip_version":info["package_versions"]["pip"],
       "wheel_bytes_verified":False,
       "installation_performed":False,
       "gate4":"NOT_AUTHORIZED"
    },sort_keys=True))
if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--evidence",required=True)
    parser.add_argument("--sha256",required=True)
    args=parser.parse_args()
    run(args.evidence,args.sha256)
