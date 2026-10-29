#!/usr/bin/env python3
"""Source-only static prepublication audit of one dedicated Windows Evidence writer.

Does NOT run PowerShell, assert native PASS, install dependencies or use devices.
"""
from pathlib import Path
import hashlib, json, re, zipfile, sys

PKG="stm32_os_evidence_writer_native_offline_v1_20261028"
ROOT=Path(__file__).resolve().parents[3]
STAGE=ROOT/"Bundle"/("_stage_"+PKG)
PACKAGE=ROOT/"Bundle"/(PKG+".zip")
EXPECTED_CASES={
"FAIL_LONG_TRACEBACK_TERMINAL_CAUSE",
"TIMEOUT_TREE_STOP_EVIDENCE",
"PRECHILD_FAIL_EVIDENCE",
"PASS_EVIDENCE",
"MISSING_EXCEPTION_REJECTED",
"BAD_EVIDENCE_ROOT_REJECTED",
"GIT_STATE_UNCHANGED",
"FOUR_NATIVE_SAMPLE_ZIPS_INDEXED",
}
def sha(b):return hashlib.sha256(b).hexdigest().upper()
def need(x,msg):
    if not x: raise AssertionError(msg)
def check():
    files={p.name:p.read_bytes() for p in STAGE.iterdir() if p.is_file()}
    need(set(files)=={"apply.ps1","bundle_spec.json","PACKAGE_README.txt"},"STAGE_FILES")
    spec=json.loads(files["bundle_spec.json"].decode())
    need(spec["package_id"]==PKG and spec["execution_profile"]=="OFFLINE" and
         spec["runner_result_contract"]=="DEUS_OS_RUNNER_RESULT_V1","SPEC")
    apply=files["apply.ps1"].decode("utf-8")
    need("Set-StrictMode -Version 3" in apply,"STRICT")
    need("$PackageId='"+PKG+"'" in apply,"ID")
    need("DEUS_RUN_RESULT_PATH" in apply,"RESULT_CONTRACT")
    need("function SealEvidence(" in apply and "CreateFromDirectory(" in apply
         and "[IO.File]::Move($temp,$target,$false)" in apply,"REAL_NATIVE_WRITER")
    need("EVIDENCE_OUTPUT_ROOT_UNAVAILABLE" in apply and
         "DIAGNOSTIC_INCOMPLETE_TERMINAL_CAUSE" in apply,"FAIL_CLOSE_WRITER")
    need("MIDDLE_TRUNCATED_TAIL_PRESERVED" in apply and
         ".Substring($one.Length-430)" in apply,"TRACE_TAIL")
    need("Substring(0,600)" not in apply,"KNOWN_BAD_PREFIX_ONLY")
    need("CHILD_STDERR=" in apply and "CHILD_STDOUT=" in apply and
         "TERMINAL_EXCEPTION=" in apply,"STREAM_AND_CAUSE")
    need("Kill($true)" in apply and "TimeoutMilliseconds 1200" in apply,
         "CHILD_BOUND_AND_TREE_KILL")
    need("FileMode]::CreateNew" in apply and ".Flush($true)" in apply,"NO_OVERWRITE_EVIDENCE")
    need("FingerprintGit" in apply and "GIT_POSTSTATE_CHANGED" in apply,
         "READONLY_REPO")
    need("GetFullPath($env:DEUS_RUN_RESULT_PATH)" in apply and
         apply.count("WriteNew ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))")==1,
         "SINGLE_RUNNER_RESULT_WRITER")
    need("nonce_ledger_offline" not in apply and
         "ProtectedRecovery" not in apply and
         "owner_windows_nonce_adapter" not in apply,"FORBIDDEN_CONTROLLER_TOUCH")
    need("pip install" not in apply and "Invoke-RestMethod" not in apply
         and "Invoke-WebRequest" not in apply and "Start-Process" not in apply,
         "NO_DEPENDENCY_INSTALL_OR_NETWORK")
    cases=re.findall(r"TestMark '([A-Z0-9_]+)'",apply)
    need(set(cases)==EXPECTED_CASES and len(cases)==8,"EXACT_NATIVE_CASE_MARKERS")
    for name in ("sample_fail.zip","sample_timeout.zip","sample_prechild.zip","sample_pass.zip"):
        need(("'"+name+"'") in apply,"NATIVE_SAMPLE_MISSING_"+name)
    need("exit 17" in apply and "Start-Sleep -Seconds 30" in apply,"REAL_SYNTHETIC_CHILD")
    # Check all actual nested native ZIPs will be copied into original outer
    # Evidence and included in the writer-generated SHA/size indexes.
    need("foreach($name in $Attach.Keys)" in apply and
         "[IO.File]::Copy($source" in apply and
         "foreach($name in $names)" in apply,"NESTED_NATIVE_EVIDENCE_NOT_INDEXED")
    from bundle_guard import check_apply
    check_apply(files["apply.ps1"])
    if PACKAGE.exists():
        from bundle_guard import validate_file
        reviewed=validate_file(PACKAGE)
        need(reviewed["profile"]=="OFFLINE" and
             reviewed["qualification"]=="STATIC_ONLY_NOT_NATIVE_OR_HARDWARE",
             "FINAL_ZIP_NOT_STATIC")
        with zipfile.ZipFile(PACKAGE) as z:
            need(z.testzip() is None,"CRC")
            need(z.read("apply.ps1")==files["apply.ps1"],"APPLY_STAGE_BYTE_DRIFT")
    print(json.dumps({
        "result":"PASS_SOURCE_ONLY_NOT_WINDOWS_NATIVE",
        "package_id":PKG,"apply_sha256":sha(files["apply.ps1"]),
        "8_case_markers":"PASS","four_native_samples":"SOURCE_PRESENT",
        "child_timeout":"SOURCE_PRESENT","terminal_cause":"SOURCE_PRESENT",
        "nested_zip_index":"SOURCE_PRESENT","final_zip":"STATIC_VALIDATED" if PACKAGE.exists() else "NOT_BUILT"
    },sort_keys=True))
if __name__=="__main__":
    check()
