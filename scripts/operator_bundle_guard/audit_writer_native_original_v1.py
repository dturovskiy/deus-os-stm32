#!/usr/bin/env python3
"""Independently audit genuine owner-Windows writer selftest ORIGINAL Evidence.

Requires EXACT original Evidence file and whole hash from the owner run.
Never synthesizes or patches original Evidence, never opens hardware.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile

PKG="stm32_os_evidence_writer_native_offline_v1_20261028"
BUNDLE_SHA="3603821E93217EE9617193D70E6868BFE933A82CDD175E2D7A3A314761F9DD93"
SAMPLES={
    "sample_fail.zip":("FAIL","17","ModuleNotFoundError: No module named 'cryptography'"),
    "sample_timeout.zip":("FAIL","-2","TimeoutError: synthetic child exceeded 1200ms and termination requested"),
    "sample_prechild.zip":("FAIL","NOT_STARTED","CHILD_NOT_STARTED=YES"),
    "sample_pass.zip":("PASS","0","SYNTHETIC_SUCCESS=PASS")
}
CASES={
    "FAIL_LONG_TRACEBACK_TERMINAL_CAUSE",
    "TIMEOUT_TREE_STOP_EVIDENCE",
    "PRECHILD_FAIL_EVIDENCE",
    "PASS_EVIDENCE",
    "MISSING_EXCEPTION_REJECTED",
    "BAD_EVIDENCE_ROOT_REJECTED",
    "GIT_STATE_UNCHANGED",
    "FOUR_NATIVE_SAMPLE_ZIPS_INDEXED"
}
ROOT=Path(__file__).resolve().parents[3]
def sha(b): return hashlib.sha256(b).hexdigest().upper()
def need(ok,why):
    if not ok: raise AssertionError(why)
def inspect(blob,expected_members):
    with zipfile.ZipFile(io.BytesIO(blob)) as z:
        names=z.namelist()
        need(len(names)==len(set(names)) and set(names)==expected_members,"MEMBER_SET")
        need(z.testzip() is None,"CRC_FAILED")
        data={n:z.read(n) for n in names}
    index=data["hashes.sha256"].decode("ascii").splitlines()
    need(len(index)==len(data)-1,"SHA_INDEX_COUNT")
    seen=set()
    for line in index:
        m=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",line)
        need(m and m[2] not in seen and m[2] in data and m[2]!="hashes.sha256","INDEX_ROW")
        need(sha(data[m[2]])==m[1],"SHA_INDEX_WRONG")
        seen.add(m[2])
    need(seen==set(data)-{"hashes.sha256"},"INDEX_COVERAGE")
    sizes=data["sizes.txt"].decode("ascii").splitlines()
    need(len(sizes)==len(data)-2,"SIZES_COUNT")
    seen=set()
    for line in sizes:
        m=re.fullmatch(r"([0-9]+)  ([A-Za-z0-9_.-]+)",line)
        need(m and m[2] in data and m[2] not in seen and m[2] not in {"hashes.sha256","sizes.txt"},"SIZE_ROW")
        need(len(data[m[2]])==int(m[1]),"SIZE_WRONG")
        seen.add(m[2])
    need(seen==set(data)-{"hashes.sha256","sizes.txt"},"SIZES_COVERAGE")
    return data
def review(actual_basename,expected_sha):
    need(re.fullmatch(r"[0-9A-F]{64}",expected_sha),"EXPECTED_SHA_FORMAT")
    need(Path(actual_basename).name==actual_basename,"EVIDENCE_BASENAME_ONLY")
    need(actual_basename.startswith(PKG+"_") and actual_basename.endswith(".evidence.zip"),"EVIDENCE_IDENTITY")
    ev=ROOT/"Evidence"/actual_basename
    need(ev.is_file() and not ev.is_symlink(),"ORIGINAL_OWNER_EVIDENCE_NOT_COPIED")
    original=ev.read_bytes()
    need(sha(original)==expected_sha,"ORIGINAL_EVIDENCE_SHA")
    bundle=ROOT/"Bundle"/(PKG+".zip")
    need(sha(bundle.read_bytes())==BUNDLE_SHA,"SOURCE_BUNDLE_SHA")
    required={"outcome.txt","run.log","native_checks.txt","native_test_output.txt",
              "sizes.txt","hashes.sha256"}|set(SAMPLES)
    data=inspect(original,required)
    outcome=json.loads(data["outcome.txt"])
    need(outcome["gate"]==PKG and outcome["result"]=="PASS" and
         outcome["classification"]=="EVIDENCE_WRITER_NATIVE_SCOPED_PASS" and
         outcome["failure_phase"]=="NONE" and outcome["error"]=="NONE",
         "ORIGINAL_NATIVE_TYPED_RESULT")
    need(outcome["git_pre_sha256"]==outcome["git_post_sha256"] and
         re.fullmatch("[0-9A-F]{64}",outcome["git_pre_sha256"]),
         "ORIGINAL_GIT_POSTSTATE")
    need(outcome["device_io"]=="NONE" and outcome["usb"]=="NO" and
         outcome["ssh"]=="NO" and outcome["reset"]=="NO" and
         outcome["flash"]=="NO" and outcome["nonce_ledger_used"]=="NO",
         "UNAUTHORIZED_DEVICE_OR_NONCE")
    checks=data["native_checks.txt"].decode()
    found=[m.group(1) for line in checks.splitlines() if (m:=re.fullmatch(r"PASS ([A-Z_]+)",line)) is not None]
    need(set(found)==CASES and len(found)==len(CASES)==8,"ORIGINAL_EIGHT_CASES")
    native=data["native_test_output.txt"].decode()
    need("WRITER_NATIVE_SELFTEST=PASS" in native and
         "G4_A05_CHILD_EXIT=0" in native,"ORIGINAL_CHILD_SUMMARY_MISSING")
    for name,(result,code,terminal) in SAMPLES.items():
        inner=inspect(data[name],{"outcome.txt","run.log","native_checks.txt",
                                  "native_test_output.txt","sizes.txt","hashes.sha256"})
        obj=json.loads(inner["outcome.txt"])
        need(obj["result"]==result and str(obj["child_exit_code"])==code,
             "NESTED_RESULT_"+name)
        need(obj["gate"]==PKG and obj["device_io"]=="NONE" and
             obj["nonce_ledger_used"]=="NO","NESTED_CUSTODY_"+name)
        text=inner["native_test_output.txt"].decode()
        need("G4_A05_CHILD_EXIT="+code in text and terminal in text,
             "NESTED_TERMINAL_OR_EXIT_LOST_"+name)
        if name=="sample_fail.zip":
            need("MIDDLE_TRUNCATED_TAIL_PRESERVED" in text,"NESTED_LONG_TRACE_TAIL_NOT_TESTED")
            need("CHILD_STDERR=" in text and
                 "TERMINAL_EXCEPTION=ModuleNotFoundError: No module named 'cryptography'" in text,
                 "NESTED_REAL_STDERR_CAUSE_NOT_CAPTURED")
        if name=="sample_timeout.zip":
            need("TERMINAL_EXCEPTION=TimeoutError:" in text,"NESTED_TIMEOUT_EXCEPTION_NOT_CAPTURED")
        if name=="sample_prechild.zip":
            need("CHILD_NOT_STARTED=YES" in text,"NESTED_PRESTART_STATUS_LOST")
    print(json.dumps({
        "result":"ORIGINAL_WINDOWS_WRITER_NATIVE_SCOPED_PASS",
        "evidence_sha256":expected_sha,"outer_members":len(data),
        "outer_crc":"PASS","outer_hashes":len(data)-1,"outer_sizes":len(data)-2,
        "inner_sample_zips":"4/4_NESTED_CRC_AND_HASH_AND_SEMANTIC_PASS",
        "cases":"8/8","no_hardware":"ATTESTED_OFFLINE_ONLY",
        "gate4":"NOT_AUTHORIZED"
    },sort_keys=True))
if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--evidence",required=True)
    p.add_argument("--sha256",required=True)
    args=p.parse_args()
    review(args.evidence,args.sha256)
