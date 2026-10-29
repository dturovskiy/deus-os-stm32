#!/usr/bin/env python3
"""Independent A-05 OFFLINE controller DENY-only operator and original Windows evidence audit.

Static ZIP check uses exact whole SHA and exact payload bytes. Native check
requires owner ORIGINAL ZIP and original target receipt, not synthetic logs.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import zipfile
ID="stm32_os_gate4_a05_controller_isolated_native_deny_v1_20261029"
WHOLE="F01EA8860A188A4CE1DF6CBBAA67A7637AFD58F829460F7FF4ACDFC32075E63B"
WORKER="CAB0E157E204CF119B1FDE0CD5E642BD2D25ACEB9715D2D60B57FB5F97CDCD7B"
DEPLOY="1287E148FDAF78B7B79EF1E27F56E0576FAD94721B08588F2C6D0D4EE303A532"
TREE="31E08CB9B4AEE5917D4E2F9AF49B4D513370D6E4981331397D70590CF4A21BC4"
RECEIPT="75152279350DAFD4816F2FAE12D49A0C218F612BEBA6981B70A9EE9EB6AE4A3A"
TARGET="a05_cp314_isolated_import_v1_ed2d2b32da064f4a9d8ba141847a9f45"
ROOT=Path(__file__).resolve().parents[3]
def sha(x):return hashlib.sha256(x).hexdigest().upper()
def need(ok,label):
    if not ok:raise AssertionError(label)
def pairs(rows):
    out={}
    for k,v in rows:
        need(k not in out,"DUPLICATE_JSON_FIELD")
        out[k]=v
    return out
def indexed_zip(path,expected_members):
    with zipfile.ZipFile(path) as z:
        names=z.namelist()
        need(len(names)==len(set(names))==len(expected_members) and
             set(names)==expected_members,"ZIP_EXACT_MEMBERS")
        need(z.testzip() is None,"ZIP_BAD_CRC")
        return {n:z.read(n) for n in names}
def test_index(data,sha_name,size_name=None):
    rows=data[sha_name].decode("ascii").splitlines()
    seen=set()
    for row in rows:
        m=re.fullmatch(r"([0-9A-F]{64})  ([A-Za-z0-9_./-]+)",row)
        need(m and m[2] in data and m[2] not in seen and m[2]!=sha_name,
             "HASH_INDEX_ROW")
        need(sha(data[m[2]])==m[1],"HASH_INDEX_VALUE")
        seen.add(m[2])
    need(seen==set(data)-{sha_name},"SHA_INDEX_COVERAGE")
    if size_name is not None:
        seen=set()
        for row in data[size_name].decode("ascii").splitlines():
            m=re.fullmatch(r"(\d+)  ([A-Za-z0-9_.-]+)",row)
            need(m and m[2] not in seen and
                 m[2] in data and m[2] not in {sha_name,size_name},
                 "SIZE_INDEX_ROW")
            need(len(data[m[2]])==int(m[1]),"SIZE_INDEX_VALUE")
            seen.add(m[2])
        need(seen==set(data)-{sha_name,size_name},"SIZE_INDEX_COVERAGE")
    return len(rows)
def bundle():
    p=ROOT/"Bundle"/(ID+".zip")
    b=p.read_bytes()
    need(sha(b)==WHOLE,"OPERATOR_WHOLE_SHA")
    members={"apply.ps1","PACKAGE_README.txt","manifest.json","hashes.sha256",
             "native_a05_isolated_controller_probe_v1.py"}
    data=indexed_zip(p,members)
    need(test_index(data,"hashes.sha256")==4,"INDEX_COUNT")
    manifest=json.loads(data["manifest.json"],object_pairs_hook=pairs)
    need(manifest["schema"]=="DEUS_OS_OPERATOR_BUNDLE_V2" and
         manifest["package_id"]==ID and
         manifest["execution_profile"]=="OFFLINE" and
         manifest["workflow_id"]=="gate4_a05_isolated_controller_native_deny_probe" and
         manifest["host_mutation"]=="EVIDENCE_ONLY_NO_DEPLOY_NO_NONCE" and
         manifest["network_scope"]=="NO_NETWORK" and
         manifest["runner_result_contract"]=="DEUS_OS_RUNNER_RESULT_V1" and
         manifest["apply_sha256"]==sha(data["apply.ps1"]),
         "MANIFEST_SCOPE_DRIFT")
    stage=ROOT/"Bundle"/("_stage_"+ID)
    for name in members-{"manifest.json","hashes.sha256"}:
        need(data[name]==(stage/name).read_bytes(),"STAGE_BYTE_DRIFT_"+name)
    for name in ("native_a05_isolated_controller_probe_v1.py",):
        need(sha(data[name])==WORKER and
             sha((ROOT/"OS/scripts/gate4_entry"/name).read_bytes())==WORKER,
             "LIVE_SOURCE_PIN")
    # Native AST is owned by the Windows runner and separately configured CI.
    # This source-only guard is NOT a PowerShell compiler.
    ps=data["apply.ps1"].decode("utf-8","strict")
    for required in (
         "DEUS_RUN_RESULT_PATH","MIDDLE_TRUNCATED_TAIL_PRESERVED",
         "TERMINAL_EXCEPTION=","CHILD_STDERR=","ProcessStartInfo",
         "Kill($true)","30000","PYTHONPATH","PYTHONDONTWRITEBYTECODE",
         "PYTHONNOUSERSITE","nonce_consumed='NO'",
         "controller_release='DENIED'","GitFingerprint",
         "GIT_PRE_POST_IDENTICAL","CreateFromDirectory("):
        need(required in ps,"SOURCE_GUARD_MISSING_"+required)
    need(ps.count("WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))")==1,
         "DUPLICATE_OR_MISSING_RESULT_WRITER")
    from audit_a05_cp314_pwsh_delimiters_v1 import delimiters
    need(delimiters(ps)["balanced"],"POWERSHELL_LEXICAL_DELIMITERS_UNBALANCED")
    for forbidden in ("Invoke-WebRequest","Invoke-RestMethod","Start-Process",
                      "ssh.exe","st-flash","STM32CubeProgrammer","ProtectedRecovery",
                      "Write-G4ConsumedNonceSourceOnly"):
        need(forbidden.lower() not in ps.lower(),
             "UNAUTHORIZED_OPERATOR_ACTION_"+forbidden)
    # Authoring-source native AST CI gate must validate these exact bytes.
    need(data["apply.ps1"]==(ROOT/"OS/scripts/gate4_entry"/
         "apply_a05_controller_isolated_deny_v1_20261029.ps1").read_bytes(),
         "AST_REVIEW_SOURCE_DIFFERS_FROM_CANONICAL_ZIP")
    return {"result":"A05_CONTROLLER_ISOLATED_DENY_ZIP_STATIC_PASS",
            "whole_sha256":WHOLE,"bytes":len(b),
            "crc":"5/5","sha_index":"4/4","source_pin":"1/1",
            "native_windows":"PENDING","gate4":"NOT_AUTHORIZED"}
def evidence(name,expect):
    need(name==Path(name).name and
         name.startswith(ID+"_") and name.endswith(".evidence.zip") and
         re.fullmatch(r"[A-Za-z0-9_.-]{30,180}",name),
         "ORIGINAL_EVIDENCE_BASENAME")
    need(re.fullmatch(r"[0-9A-F]{64}",expect),"ORIGINAL_EVIDENCE_SHA_FORMAT")
    p=ROOT/"Evidence"/name
    need(p.is_file() and not p.is_symlink(),"ORIGINAL_EVIDENCE_NOT_AVAILABLE")
    need(sha(p.read_bytes())==expect,"ORIGINAL_EVIDENCE_WHOLE_SHA")
    names={"outcome.txt","native_checks.txt","native_test_output.txt",
           "run.log","sizes.txt","hashes.sha256"}
    data=indexed_zip(p,names)
    need(test_index(data,"hashes.sha256","sizes.txt")==5,"ORIGINAL_EVIDENCE_INDEX_COUNTS")
    o=json.loads(data["outcome.txt"],object_pairs_hook=pairs)
    need(o["gate"]==ID and
         o["schema"]=="DEUS_A05_CONTROLLER_ISOLATED_NATIVE_DENY_EVIDENCE_V1" and
         o["worker_sha256"]==WORKER and o["original_host_tree_sha256"]==TREE and
         o["original_deploy_evidence_sha256"]==DEPLOY and
         o["receipt_sha256"]==RECEIPT and
         o["controller_release"]=="DENIED" and
         o["nonce_consumed"]=="NO" and o["device_io"]=="NONE" and
         o["installation"]=="NONE" and o["network"]=="NO" and
         o["gate4"]=="NOT_AUTHORIZED",
         "ORIGINAL_TYPED_SCOPE")
    transcript=data["native_test_output.txt"].decode("utf-8","strict")
    checks=data["native_checks.txt"].decode("utf-8","strict").splitlines()
    need("G4_A05_CHILD_EXIT=" in transcript,"CHILD_EXIT_MISSING")
    if o["result"]=="FAIL":
        need(o["failure_phase"]!="NONE" and
             "TERMINAL_EXCEPTION=" in transcript and
             o["terminal_exception"]!="NOT_ATTESTED",
             "FAIL_DIAGNOSTICS_INCOMPLETE")
        return {"result":"A05_NATIVE_ORIGINAL_CLASSIFIED_FAIL",
                "original_evidence_sha256":expect,
                "classification":o["classification"],
                "phase":o["failure_phase"],"gate4":"NOT_AUTHORIZED"}
    need(o["result"]=="PASS" and
         o["classification"]=="A05_CONTROLLER_ISOLATED_NATIVE_DENY_ONLY_SCOPED_PASS" and
         o["child_exit_code"]=="0" and o["failure_phase"]=="NONE" and
         o["error"]=="NONE" and
         o["git_pre_sha256"]==o["git_post_sha256"] and
         re.fullmatch(r"[0-9A-F]{64}",o["git_pre_sha256"]),
         "NATIVE_PASS_TYPED_OR_GIT_DRIFT")
    expected=[
      "ORIGINAL_HOST_TARGET_SHA_AND_COUNT",
      "FOUR_SOURCE_PINS",
      "NATIVE_CP314_TARGET_DEPENDENCIES_AND_MODULE_IMPORT",
      "NATIVE_CONTROLLER_DENIES_WITHOUT_OWNER_CONFIG",
    ]
    for x in expected:
        need("CHILD_STDOUT=A05_CONTROLLER_CASE=PASS "+x in transcript,
             "MISSING_ACTUAL_NATIVE_CASE_"+x)
    need(len(checks)>=10 and all(x.startswith("PASS ") for x in checks)
         and transcript.count("CHILD_STDOUT=A05_CONTROLLER_CASE=PASS ")==4,
         "NATIVE_CHECK_COVERAGE")
    need(sha((ROOT/"HostDependencies"/TARGET/"ADMISSION.json").read_bytes())==RECEIPT,
         "ACCEPTED_TARGET_RECEIPT_CHANGED")
    return {"result":"A05_ISOLATED_CONTROLLER_NATIVE_DENY_ORIGINAL_SCOPED_PASS",
            "original_evidence_sha256":expect,"crc":"6/6",
            "sha_index":"5/5","size_index":"4/4",
            "actual_native_cases":"4/4","authorization":"DENIED",
            "hardware":"NONE","gate4":"NOT_AUTHORIZED"}
if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--evidence")
    p.add_argument("--sha256")
    args=p.parse_args()
    if args.evidence:
        need(args.sha256 is not None,"MISSING_ORIGINAL_SHA")
        result=evidence(args.evidence,args.sha256)
    else:
        result=bundle()
    print(json.dumps(result,sort_keys=True))
