#!/usr/bin/env python3
"""Independently verify ORIGINAL owner Windows A05 scoped isolated dependency Evidence.

Exact off-Git original ZIP; independent indexed CRC/SHA/size evidence and
isolated local target tree/receipt. Never loads a .pyd, runs pip or USB.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import zipfile

ROOT=Path(__file__).resolve().parents[3]
ID="stm32_os_gate4_a05_cp314_isolated_deploy_v1_20261028"
INPUT_SHA="EB8B38354EDD866F4A6970CFACD156DCBE8744081E1F3CB290B37DE7A1E2F2A8"
WHEEL_SHA="CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C"
SOURCE_PINS={
 "worker_sha256":"492E95B7F1FCD221EEAFF653BF4AEDAF61ED0FB5488C78A3AD518DA26E0F8445",
 "wheel_verifier_sha256":"880923C0DA9A81BB81105584679F32AE9B5AC3949A8300357F66F6756B2ECB2C",
 "selection_sha256":"28B6FFE7AD8B39DC81D9EA0E8B5AA7FDDAF8E13FA5018C8EA26AB918A1B0CBFC",
}
def sha(b):return hashlib.sha256(b).hexdigest().upper()
def need(ok,why):
    if not ok:raise AssertionError(why)
def nodup(pairs):
    d={}
    for k,v in pairs:
        need(k not in d,"DUPLICATE_JSON_KEY")
        d[k]=v
    return d
def read_original(name,hash_expected):
    need(name==Path(name).name and
         re.fullmatch(r"[a-zA-Z0-9_.-]{8,160}",name),"OWNER_EVIDENCE_BASENAME")
    need(name.startswith(ID+"_") and name.endswith(".evidence.zip"),"WRONG_PACKAGE")
    need(re.fullmatch(r"[0-9A-Fa-f]{64}",hash_expected),"EVIDENCE_HASH_FORMAT")
    path=ROOT/"Evidence"/name
    need(path.is_file() and not path.is_symlink(),"ORIGINAL_EVIDENCE_NOT_ACCESSIBLE")
    data=path.read_bytes()
    need(sha(data)==hash_expected.upper(),"ORIGINAL_EVIDENCE_SHA_MISMATCH")
    with zipfile.ZipFile(path) as z:
        names=z.namelist()
        exact={"outcome.txt","run.log","native_checks.txt",
               "native_test_output.txt","sizes.txt","hashes.sha256"}
        need(len(names)==6 and len(set(names))==6 and set(names)==exact,"EXACT_EVIDENCE_MEMBERS")
        need(z.testzip() is None,"ORIGINAL_EVIDENCE_CRC")
        contents={n:z.read(n) for n in names}
    hi=contents["hashes.sha256"].decode("ascii").splitlines()
    indexed=set()
    for row in hi:
        m=re.fullmatch(r"([0-9A-Fa-f]{64})  ([A-Za-z0-9_.-]+)",row)
        need(m and m[2] not in indexed and m[2] in contents and m[2]!="hashes.sha256",
             "EVIDENCE_SHA_INDEX_FORMAT")
        need(sha(contents[m[2]])==m[1].upper(),"EVIDENCE_MEMBER_SHA")
        indexed.add(m[2])
    need(len(indexed)==5 and indexed==set(contents)-{"hashes.sha256"},
         "EVIDENCE_INDEX_COVERAGE")
    indexed=set()
    for row in contents["sizes.txt"].decode("ascii").splitlines():
        m=re.fullmatch(r"(\d+)  ([A-Za-z0-9_.-]+)",row)
        need(m and m[2] not in indexed and m[2] in contents and
             m[2] not in ("sizes.txt","hashes.sha256"),"EVIDENCE_SIZE_INDEX_FORMAT")
        need(len(contents[m[2]])==int(m[1]),"EVIDENCE_SIZE_MISMATCH")
        indexed.add(m[2])
    need(len(indexed)==4 and indexed==set(contents)-{"sizes.txt","hashes.sha256"},
         "EVIDENCE_SIZE_COVERAGE")
    return contents
def tree_fingerprint(folder):
    need(folder.is_dir() and not folder.is_symlink(),"ACTUAL_TARGET_MISSING")
    lines=[]
    for p in sorted(folder.rglob("*")):
        need(not p.is_symlink(),"TARGET_SYMLINK_FORBIDDEN")
        if p.is_dir():continue
        need(p.is_file() and p.stat().st_size<=16_000_000,"TARGET_NONFILE_OR_OVERLONG")
        rel=p.relative_to(folder).as_posix()
        lines.append(f"{sha(p.read_bytes())}  {p.stat().st_size}  {rel}")
    need(0<len(lines)<=1200,"TARGET_FILE_COUNT")
    return sha(("\n".join(lines)+"\n").encode()),len(lines)
def review(name,whole):
    contents=read_original(name,whole)
    need(sha((ROOT/"Bundle"/(ID+".zip")).read_bytes())==INPUT_SHA,"INPUT_BUNDLE_SHA")
    o=json.loads(contents["outcome.txt"],object_pairs_hook=nodup)
    need(o["gate"]==ID and o["schema"]=="DEUS_A05_CP314_HOST_ISOLATED_DEPLOY_EVIDENCE_V1",
         "EVIDENCE_TYPE")
    for key,pin in SOURCE_PINS.items():
        need(o[key]==pin,"SOURCE_PIN_"+key)
    need(o["original_wheelhouse_sha256"]==WHEEL_SHA and
         o["original_download_evidence_sha256"]==
         "9D444C61709C9C98AAEB43B2B6D75E2AB0D4F02116D1B3F41B8990ED968617DD",
         "ORIGINAL_PARENT_SOURCE_HASH")
    need(o["device_io"]=="NONE" and o["usb"]=="NO" and o["ssh"]=="NO" and
         o["reset"]=="NO" and o["flash"]=="NO" and o["nonce_ledger"]=="NOT_USED" and
         o["global_python_changed"]=="NO" and o["network"]=="NO",
         "HARDWARE_OR_GLOBAL_SCOPE_VIOLATION")
    transcript=contents["native_test_output.txt"].decode("utf-8")
    need("G4_A05_CHILD_EXIT=" in transcript,"CHILD_EXIT_MISSING")
    if o["result"]!="PASS":
        need(o["result"]=="FAIL" and o["failure_phase"]!="NONE" and
             o["error"]!="NONE" and
             "TERMINAL_EXCEPTION=" in transcript,"INCOMPLETE_FAIL_EVIDENCE")
        return {"result":"A05_ISOLATED_ORIGINAL_CLASSIFIED_FAIL_SEALED",
                "classification":o["classification"],"failure_phase":o["failure_phase"],
                "error":o["error"],"evidence_sha256":whole.upper(),
                "host_import":"NOT_ACCEPTED","gate4":"NOT_AUTHORIZED"}
    need(o["classification"]=="A05_ISOLATED_CP314_DEPENDENCIES_NATIVE_IMPORT_SCOPED_PASS" and
         o["failure_phase"]=="NONE" and o["error"]=="NONE" and
         str(o["child_exit_code"])=="0" and
         o["isolated_status"]=="ISOLATED_CP314_CONTROLLER_IMPORT_PASS",
         "NATIVE_TYPED_PASS")
    need(re.fullmatch(r"[0-9A-F]{64}",o["git_pre_sha256"]) and
         o["git_pre_sha256"]==o["git_post_sha256"],"REPO_GIT_POSTSTATE")
    required={"CHILD_PY_EXPLICIT_TARGET_IMPORT","CHILD_PY_FINAL_TARGET_IMPORT",
              "WHEEL_SHA_CRC_RECORD","TARGET_ATOMIC_PROMOTION","OFFLINE_TARGET_INSTALLED",
              "ISOLATED_CONTROLLER_IMPORT"}
    need(all(("A05_DEPLOY="+x) in transcript for x in required) and
         "A05_ISOLATED_CONTROLLER_IMPORT=PASS" in transcript and
         "CHILD_STDOUT=" in transcript,"NATIVE_ORIGINAL_IMPORT_TRANSCRIPT")
    checks=contents["native_checks.txt"].decode("utf-8").splitlines()
    need(len(checks)>=12 and all(s.startswith("PASS ") for s in checks),
         "SOURCE_AND_POSTSTATE_CHECKS")
    basename=o["isolated_target_basename"]
    need(re.fullmatch(r"a05_cp314_isolated_import_v1_[a-f0-9]{32}",basename),
         "HOST_DEPENDENCY_TARGET_NAME")
    target=ROOT/"HostDependencies"/basename
    need(target.is_dir() and not target.is_symlink(),"ORIGINAL_TARGET_NOT_SYNCED")
    receipt=target/"ADMISSION.json"
    need(receipt.is_file() and sha(receipt.read_bytes())==o["receipt_sha256"],
         "TARGET_RECEIPT_SHA_MISMATCH")
    rec=json.loads(receipt.read_bytes(),object_pairs_hook=nodup)
    need(rec["schema"]=="DEUS_A05_ISOLATED_CP314_DEPLOY_RECEIPT_V1" and
         rec["source_wheelhouse_sha256"]==WHEEL_SHA and
         rec["python_version"]=="3.14.0" and rec["pip_version"]=="25.2" and
         rec["global_python_install"]=="NO" and rec["network"]=="NO",
         "ORIGINAL_TARGET_RECEIPT_SCOPE")
    digest,count=tree_fingerprint(target/"python_target")
    need(rec["target_tree_sha256"]==digest==o["target_tree_sha256"] and
         rec["target_member_count"]==count,"TARGET_TREE_SHA_SIZE_MISMATCH")
    # Imported binary extensions only on owner Windows; on Linux inspect
    # actual immutable installed structure, do not attempt to load .pyd.
    expected=("cryptography-50.0.2.dist-info","cffi-2.1.1.dist-info",
              "pycparser-3.12.dist-info")
    for dirname in expected:
        need((target/"python_target"/dirname/"METADATA").is_file(),
             "INSTALLED_METADATA_MISSING_"+dirname)
    return {"result":"A05_OWNER_ISOLATED_CP314_IMPORT_ORIGINAL_SCOPED_PASS",
            "original_evidence_sha256":whole.upper(),
            "evidence_crc":"6/6","sha_index":"5/5","size_index":"4/4",
            "verified_target_basename":basename,
            "target_tree_sha256":digest,"target_members":count,
            "controller_import":"ORIGINAL_WIN_NATIVE_PASS",
            "global_python_install":"NO",
            "gate4":"NOT_AUTHORIZED"}
if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--evidence",required=True)
    parser.add_argument("--sha256",required=True)
    a=parser.parse_args()
    print(json.dumps(review(a.evidence,a.sha256),sort_keys=True))
