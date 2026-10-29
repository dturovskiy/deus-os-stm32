#!/usr/bin/env python3
"""Read-only original Windows runner 1.3.2 upgrade + pre-child AST FAIL evidence audit.

This audit does not run PowerShell, touch installed runner, modify evidence, or use USB.
"""
import hashlib
import json
import re
import sys
import zipfile
from pathlib import Path

ROOT=Path.cwd().resolve()
WORK=ROOT.parent
MAIN="stm32_os_deus_runner_preflight_evidence_upgrade_v1_20261028_20261028_181856_fd97e27eec9e42a3a92d373273a1f70c.evidence.zip"
BAD="stm32_os_gate4_a04_offline_stop_native_v2_20261028_runner_ast_20261028_181851_c6203b24477749a68bac990b5475d88a.evidence.zip"
MAIN_SHA="B0E85D896E0B834370625DF75B145327B780A24B9E7B6DA6040343D692F31AFD"
BAD_SHA="6EE35B50133DC00609ADAC09B5CF81C806FE0E6FD113A5A8340E043B904CFF62"
ZIP_SHA="8325465822205CF0BD373267E2D8C8B75EF2D48199E7A7B7D627FADB212974D9"
BAD_BUNDLE_SHA="AFEAC37BCA54BE550B05C1BF347A1175423CA3CC16DB4003A8FC9F095421007E"
OLD_SHA="D4239C914C72782FCE45127D973D5A6D3B56FB2FEE430FE0ECE963D46D0FF906"
NEW_SHA="60C090E8324EA1ECD2ECCF7114E26A6793402AA44B736CA507BB45321DD6EC75"
BAD_APPLY_SHA="51B63E9B4C20669A87CC2ED5A1E3B7255A71D5F950A0B827509CCB2C8C1E2A2B"
MAIN_MEMBERS={"outcome.txt","run.log","native_checks.txt","sizes.txt","hashes.sha256"}
BAD_MEMBERS={"outcome.json","parser_errors.json","run.log","sizes.txt","hashes.sha256"}

class AuditReject(ValueError):pass
def require(value,code):
    if not value:raise AuditReject(code)
def sha(data):return hashlib.sha256(data).hexdigest().upper()
def pairs_unique(pairs):
    out={}
    for k,v in pairs:
        require(k not in out,"JSON_DUPLICATE")
        out[k]=v
    return out
def json_read(b):
    require(len(b)<15000,"JSON_SIZE")
    return json.loads(b.decode("utf-8"),object_pairs_hook=pairs_unique)
def audit_zip(name,digest,expected_members,expected_size):
    f=WORK/"Evidence"/name
    require(f.is_file() and not f.is_symlink(),"EVIDENCE_MISSING_"+name)
    data=f.read_bytes()
    require(len(data)==expected_size and sha(data)==digest,"ZIP_IDENTITY_"+name)
    with zipfile.ZipFile(f) as z:
        infos=z.infolist()
        names=[i.filename for i in infos]
        require(len(names)==5 and len(set(names))==5 and set(names)==expected_members,"MEMBER_SET_"+name)
        require(all(re.fullmatch("[A-Za-z0-9_.-]+",i.filename) and
                    not i.is_dir() and not i.flag_bits&1 and
                    i.file_size<20000 and i.compress_type in (0,8) for i in infos),"MEMBER_SAFETY")
        require(sum(i.file_size for i in infos)<50000,"ZIP_DECOMPRESSION_LIMIT")
        require(z.testzip() is None,"CRC_BAD_"+name)
        files={n:z.read(n) for n in names}
    index=files["hashes.sha256"].decode("ascii").splitlines()
    require(len(index)==4,"HASH_INDEX_COUNT")
    seen=set()
    for line in index:
        m=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",line)
        require(m is not None,"HASH_INDEX_FORMAT")
        key=m[2]
        require(key in files and key!="hashes.sha256" and key not in seen,"HASH_INDEX_MEMBER")
        require(sha(files[key])==m[1],"HASH_INDEX_SHA")
        seen.add(key)
    require(seen==expected_members-{"hashes.sha256"},"HASH_INDEX_COVERAGE")
    sizes=files["sizes.txt"].decode("ascii").splitlines()
    expected_sizes=expected_members-{"hashes.sha256","sizes.txt"}
    require(len(sizes)==len(expected_sizes),"SIZE_INDEX_COUNT")
    sized={}
    for line in sizes:
        m=re.fullmatch(r"([0-9]+)  ([A-Za-z0-9_.-]+)",line)
        require(m is not None and m[2] not in sized,"SIZE_INDEX_ENTRY")
        sized[m[2]]=int(m[1])
    require(set(sized)==expected_sizes,"SIZE_INDEX_COVERAGE")
    require(all(len(files[k])==v for k,v in sized.items()),"SIZE_INDEX_DRIFT")
    return files

def main():
    require(ROOT.name=="OS" and (ROOT/".git").exists(),"OS_ROOT")
    bundle=WORK/"Bundle"/"stm32_os_deus_runner_preflight_evidence_upgrade_v1_20261028.zip"
    require(bundle.is_file() and not bundle.is_symlink() and sha(bundle.read_bytes())==ZIP_SHA,"UPGRADE_BUNDLE_SHA")
    badBundle=WORK/"Bundle"/"stm32_os_gate4_a04_offline_stop_native_v2_20261028.zip"
    require(badBundle.is_file() and sha(badBundle.read_bytes())==BAD_BUNDLE_SHA,"HISTORICAL_BUNDLE_SHA")
    m=audit_zip(MAIN,MAIN_SHA,MAIN_MEMBERS,1784)
    f=audit_zip(BAD,BAD_SHA,BAD_MEMBERS,1733)
    outcome=json_read(m["outcome.txt"])
    require(outcome.get("schema")=="DEUS_RUNNER_PREFLIGHT_EVIDENCE_UPGRADE_EVIDENCE_V1","UPGRADE_SCHEMA")
    require(outcome.get("result")=="PASS" and
            outcome.get("classification")=="RUNNER_V132_PREFLIGHT_EVIDENCE_INSTALL_NATIVE_PASS" and
            outcome.get("failure_phase")=="NONE" and
            outcome.get("error")=="NONE","UPGRADE_OUTCOME")
    require(outcome.get("package_id")=="stm32_os_deus_runner_preflight_evidence_upgrade_v1_20261028","UPGRADE_PACKAGE_ID")
    require(outcome.get("old_sha256")==OLD_SHA and outcome.get("candidate_sha256")==NEW_SHA and
            outcome.get("installed_sha256")==NEW_SHA and outcome.get("operator_zip_sha256")==ZIP_SHA,"UPGRADE_SHA_PINS")
    require(outcome.get("rollback_backup_prepared") is True and
            outcome.get("firmware_io")=="NONE" and outcome.get("git_writes")=="NONE","UPGRADE_SCOPE")
    checks=m["native_checks.txt"].decode("utf-8").splitlines()
    names=["BASELINE_SELFTEST","PATCHED_SELFTEST","BUNDLE_SEARCH_POSITIVE","BUNDLE_SEARCH_NEGATIVE",
           "RUNNER_132_ACTUAL_V2_AST_FAIL_ORIGINAL_EVIDENCE","INSTALL","INSTALLED_BUNDLE_SEARCH"]
    for name in names:
        matched=[x for x in checks if x.startswith(name+" ") or x.startswith(name+"=")]
        require(len(matched)==1,"MISSING_OR_DUPLICATE_CHECK_"+name)
        if name=="RUNNER_132_ACTUAL_V2_AST_FAIL_ORIGINAL_EVIDENCE":
            require("PASS LINE=92 COL=73" in matched[0],"ORIGINAL_AST_CHECK_FAIL")
        else:
            require("MARKER_PRESENT=True" in matched[0] and
                    (("EXIT=1 EXPECT=1" in matched[0]) if name=="BUNDLE_SEARCH_NEGATIVE" else
                    ("EXIT=0 EXPECT=0" in matched[0])),"NATIVE_CHECK_FAIL_"+name)
    require(len(checks)==len(names),"NATIVE_CHECKS_COUNT")
    badOutcome=json_read(f["outcome.json"])
    require(badOutcome.get("schema")=="DEUS_OS_RUNNER_PREFLIGHT_EVIDENCE_V1","BAD_AST_SCHEMA")
    require(badOutcome.get("result")=="FAIL" and
            badOutcome.get("classification")=="HARNESS_RUNNER_OR_RESULT_CONTRACT" and
            badOutcome.get("failure_phase")=="LAUNCHER_AST_PREFLIGHT" and
            badOutcome.get("error")=="BUNDLE_APPLY_POWERSHELL_AST_REJECTED","BAD_AST_FAIL")
    require(badOutcome.get("bundle_sha256")==BAD_BUNDLE_SHA and
            badOutcome.get("apply_sha256")==BAD_APPLY_SHA,"BAD_AST_INPUT_SHA")
    require(badOutcome.get("child_started") is False and
            badOutcome.get("device_io")=="NO_CHILD_EXECUTED","BAD_AST_NO_CHILD")
    errors=json_read(f["parser_errors.json"])
    require(type(errors) is list and 1<=len(errors)<=32,"AST_ERROR_LIST")
    require(any(x.get("line")==92 and x.get("column")==73 and
                "Missing closing" in x.get("message","") for x in errors),"ACTUAL_AST_92_73")
    log=m["run.log"].decode("utf-8")
    require("Runner-owned AST pre-child FAIL evidence SHA="+BAD_SHA in log,"FAIL_EVIDENCE_CUSTODY")
    return {
        "classification":"RUNNER_132_UPGRADE_AND_PRECHILD_FAIL_EVIDENCE_INDEPENDENT_ACCEPTED_SCOPED",
        "result":"PASS","upgrade_zip_sha256":ZIP_SHA,"upgrade_evidence_sha256":MAIN_SHA,
        "original_ast_fail_evidence_sha256":BAD_SHA,
        "original_zip_crc":"5/5 + 5/5","hash_index":"4/4 + 4/4","size_index":"3/3 + 3/3",
        "upgrade_native_checks":len(checks),"parser_error_count":len(errors),
        "installed_runner_sha256_attested":NEW_SHA,
        "prior_runner_backup":True,"ast_error":"line 92, column 73",
        "STM32":"NOT_TOUCHED","A04_NATIVE":"OPEN","Gate4":"NOT_AUTHORIZED",
    }

if __name__=="__main__":
    try:
        print(json.dumps(main(),sort_keys=True))
        sys.exit(0)
    except (Exception,) as ex:
        print(json.dumps({"result":"FAIL","reason":str(ex)[:150]}))
        sys.exit(2)
