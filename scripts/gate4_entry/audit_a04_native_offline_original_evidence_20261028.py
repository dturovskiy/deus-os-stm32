#!/usr/bin/env python3
"""Independently audit original Windows Gate4 A-04 v3 OFFLINE evidence.

No device I/O, no modifying original files, no PowerShell execution.
"""
import hashlib
import json
import re
import sys
import zipfile
from pathlib import Path

ROOT=Path.cwd().resolve()
WORKSPACE=ROOT.parent
ID="stm32_os_gate4_a04_offline_stop_native_v3_20261028"
ORIGINAL=(ID+"_20261028_182548_98da3e00afc04cb79f84fe6cc4575247.evidence.zip")
ZIP_SHA="2AE07EED5E8B95154B56B365EF8EA68A5D33E83D0A3B662573E0F43B2E80D322"
BUNDLE_SHA="6E40B9757109614CA5F1A47DA2976E819EAFCAF0A74DA35D47E60A678BD39D31"
APPLY_SHA="4AD7953AE838C467D0F4ACA8D62F74C1180E73941CC340E670267F8BB3074D9D"
V6_SHA="2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
V7_SHA="C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A"
MEMBERS={"outcome.txt","native_checks.txt","run.log","sizes.txt","hashes.sha256"}

class Reject(ValueError): pass
def need(value,code):
    if not value: raise Reject(code)
def sha(data): return hashlib.sha256(data).hexdigest().upper()
def unique(pairs):
    out={}
    for k,v in pairs:
        need(k not in out,"DUPLICATE_JSON_KEY")
        out[k]=v
    return out
def index_check(raw,data,target):
    lines=raw.decode("ascii").splitlines()
    need(len(lines)==len(target),"INDEX_COUNT")
    got=set()
    for line in lines:
        m=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",line)
        need(m is not None,"INDEX_SYNTAX")
        hash_expected,name=m.groups()
        need(name in target and name not in got,"INDEX_MEMBER")
        need(sha(data[name])==hash_expected,"INDEX_CONTENT_SHA_"+name)
        got.add(name)
    need(got==target,"INDEX_COVERAGE")
def size_check(raw,data,target):
    lines=raw.decode("ascii").splitlines()
    need(len(lines)==len(target),"SIZE_INDEX_COUNT")
    got=set()
    for line in lines:
        m=re.fullmatch(r"([0-9]+)  ([A-Za-z0-9_.-]+)",line)
        need(m is not None,"SIZE_INDEX_SYNTAX")
        length,name=m.groups()
        need(name in target and name not in got,"SIZE_INDEX_MEMBER")
        need(int(length)==len(data[name]),"SIZE_INDEX_MISMATCH_"+name)
        got.add(name)
    need(got==target,"SIZE_INDEX_COVERAGE")

def check():
    need(ROOT.name=="OS" and ROOT.joinpath(".git").exists(),"OS_REPO_ROOT")
    original=WORKSPACE/"Evidence"/ORIGINAL
    bundle=WORKSPACE/"Bundle"/(ID+".zip")
    source=WORKSPACE/"Bundle"/("_stage_"+ID)/"apply.ps1"
    for path in (original,bundle,source):
        need(path.is_file() and not path.is_symlink(),"INPUT_MISSING_"+path.name)
    raw=original.read_bytes()
    need(len(raw)==2172,"ORIGINAL_EVIDENCE_BYTES")
    need(sha(raw)==ZIP_SHA,"ORIGINAL_EVIDENCE_SHA")
    blob=bundle.read_bytes()
    need(len(blob)==6985 and sha(blob)==BUNDLE_SHA,"OPERATOR_INPUT_ZIP_SHA")
    source_raw=source.read_bytes()
    need(sha(source_raw)==APPLY_SHA,"OPERATOR_APPLY_SHA")
    with zipfile.ZipFile(bundle) as z:
        need(z.testzip() is None,"BUNDLE_CRC")
        need(z.read("apply.ps1")==source_raw,"BUNDLE_APPLY_BYTES")
    with zipfile.ZipFile(original) as z:
        entries=z.infolist()
        names=[info.filename for info in entries]
        need(len(names)==5 and set(names)==MEMBERS and len(set(names))==5,"ORIGINAL_MEMBERS")
        need(all(not info.is_dir() and not(info.flag_bits&1) and
                 re.fullmatch(r"[A-Za-z0-9_.-]+",info.filename) and
                 info.file_size<30000 and info.compress_type in (0,8)
                 for info in entries),"ORIGINAL_MEMBER_SAFETY")
        need(sum(info.file_size for info in entries)<45000,"ORIGINAL_TOTAL_BOUNDS")
        need(z.testzip() is None,"ORIGINAL_CRC")
        files={name:z.read(name) for name in names}
    index_check(files["hashes.sha256"],files,MEMBERS-{"hashes.sha256"})
    size_check(files["sizes.txt"],files,MEMBERS-{"hashes.sha256","sizes.txt"})
    outcome=json.loads(files["outcome.txt"].decode("utf-8"),object_pairs_hook=unique)
    need(type(outcome) is dict,"OUTCOME_SHAPE")
    need(outcome.get("schema")=="DEUS_OS_G4_A04_NATIVE_OFFLINE_STOP_EVIDENCE_V1","OUTCOME_SCHEMA")
    need(outcome.get("result")=="PASS" and
         outcome.get("classification")=="G4_A04_WINDOWS_NATIVE_OFFLINE_STOP_POLICY_SCOPED_PASS"
         and outcome.get("failure_phase")=="NONE" and
         outcome.get("error")=="NONE","OUTCOME_STATUS")
    need(outcome.get("gate")==ID and
         type(outcome.get("source_checks")) is int and
         outcome.get("source_checks")==48,"OUTCOME_SOURCE_IDENTITY")
    need(outcome.get("signed_v6_sha256")==V6_SHA and
         outcome.get("rescue_v7_sha256")==V7_SHA,"PROTECTED_SOURCE_SHA")
    need(outcome.get("native_scope")=="OFFLINE_STOP_SCENARIOS_ONLY" and
         outcome.get("gate4")=="NOT_AUTHORIZED" and
         outcome.get("device_io")=="NONE","NO_PHYSICAL_PROMOTION")
    need(all(outcome.get(k)=="NO" for k in ("usb","ssh","reset","flash")),
         "DEVICE_IO_CLAIM")
    need(outcome.get("future_000e_installed")=="NOT_ATTESTED" and
         outcome.get("actual_000d_info")=="NOT_ATTESTED","DEVICE_UNATTESTED")
    before=outcome.get("git_pre_sha256")
    need(isinstance(before,str) and re.fullmatch("[A-F0-9]{64}",before) and
         before==outcome.get("git_post_sha256"),"GIT_PRE_POST")
    text=source_raw.decode("utf-8")
    source_paths=re.findall(r"(?m)^\s*'([^']+)'='([A-F0-9]{64})'\s*$",text)
    need(len(source_paths)==8,"SOURCE_PINS_COUNT")
    source_names={"PIN_"+re.sub(r"[^A-Za-z0-9]","_",p) for p,h in source_paths}
    need(len(source_names)==8,"SOURCE_PINS_UNIQUE")
    source_marker_names=re.findall(r"(?m)^\s*Check-G4Scenario '([A-Z0-9_]+)'",text)
    need(len(source_marker_names)==34 and len(set(source_marker_names))==34,"STOP_SCENARIO_SOURCE_COUNT")
    protected_checks={
      "SIZE_deus_os_000e_management_only_tree081aac70_signed_v6.pkg",
      "SHA_deus_os_000e_management_only_tree081aac70_signed_v6.pkg",
      "SIZE_deus_os_rev5_000c_recovery_future_floor7_v1.pkg",
      "SHA_deus_os_rev5_000c_recovery_future_floor7_v1.pkg",
    }
    expected=source_names|set(source_marker_names)|protected_checks|{"HOST_DATA_RETRY_ONLY_NOT_SESSION","GIT_PRE_POST_IDENTICAL"}
    need(len(expected)==48,"EXPECTED_CHECK_SET_SIZE")
    checks=files["native_checks.txt"].decode("utf-8").splitlines()
    need(len(checks)==48 and len(set(checks))==48,"NATIVE_CHECK_COUNT_UNIQUE")
    need(set(checks)=={"PASS "+x for x in expected},"ORIGINAL_NATIVE_CHECK_SET")
    log=files["run.log"].decode("utf-8")
    need("G4_A04_WINDOWS_NATIVE_OFFLINE=YES" in log and
         "CLASSIFICATION=G4_A04_WINDOWS_NATIVE_OFFLINE_STOP_POLICY_SCOPED_PASS" in log and
         "USB=NO SSH=NO RESET=NO FLASH=NO" in log,"ORIGINAL_LOG_CLASS")
    # Also require the exact production source pin artifacts, not just check labels.
    for name,digest in source_paths:
        target=ROOT/name
        need(target.is_file() and sha(target.read_bytes())==digest,
             "LIVE_SOURCE_SHA_"+name)
    for package,digest,size in [
        ("deus_os_000e_management_only_tree081aac70_signed_v6.pkg",V6_SHA,49072),
        ("deus_os_rev5_000c_recovery_future_floor7_v1.pkg",V7_SHA,52092)]:
        p=WORKSPACE/"ProtectedRecovery"/package
        need(p.is_file() and p.stat().st_size==size and sha(p.read_bytes())==digest,
             "PROTECTED_PACKAGE_"+package)
    return {
      "result":"PASS","classification":"GATE4_A04_WINDOWS_NATIVE_OFFLINE_STOP_ORIGINAL_EVIDENCE_ACCEPTED_SCOPED",
      "original_evidence_sha256":ZIP_SHA,"original_evidence_bytes":len(raw),
      "original_crc":"5/5","original_sha_index":"4/4","original_size_index":"3/3",
      "source_pins":"8/8","signed_package_size_sha":"4/4",
      "synthetic_stop_scenarios":"34/34","host_data_retry":"PASS",
      "git_pre_post":"PASS","native_checks":"48/48",
      "original_bundle_sha256":BUNDLE_SHA,"device_io":"NONE",
      "A04":"ACCEPTED_SCOPED_OFFLINE_ONLY","A05":"NOT_AUTHORIZED",
      "GATE4":"NOT_AUTHORIZED","GATE4_EIGHT_ORIGINAL":"0/8"
    }
if __name__=="__main__":
    try:
        print(json.dumps(check(),sort_keys=True))
        sys.exit(0)
    except Exception as e:
        print(json.dumps({"result":"FAIL","classification":str(e)[:180]}))
        sys.exit(2)
