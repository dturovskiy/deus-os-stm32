#!/usr/bin/env python3
"""Independent source-only audit of A-05 Windows Python314 dependency inventory.

NEVER claims Windows-native success, wheel-byte provenance or installation.
"""
from pathlib import Path
import ast
import hashlib
import json
import re
import zipfile

PKG="stm32_os_gate4_a05_python_dependency_native_inventory_v1_20261028"
ROOT=Path(__file__).resolve().parents[3]
STAGE=ROOT/"Bundle"/("_stage_"+PKG)
REPO=ROOT/"OS"/"scripts"/"gate4_entry"
PROBE="probe_windows_python_dependency_inventory_v1.py"
LOCK="a05_windows_python_dependency_candidates_v1.json"
PROBE_SHA="AACC99B0998B831330D543166C9CD72731B1E5D4F9B95E39E32110D8EBBD34B3"
LOCK_SHA="7D683C82DA2A75F7CA7975216BB2EF4E14105671B1A9889EA323A907D37526D1"
EXPECTED_WHEEL={
    "cryptography-50.0.2-cp311-abi3-win_amd64.whl":"7AFA5A6602A9F29AF1F3A2965F831BAE7C9D5D597B7CBB716D41AB3B7D89879C",
    "cryptography-50.0.2-cp314-cp314t-win_amd64.whl":"78198641E5BE9521BEEA5AA782BB551A58068D10E6EB04C9C680C1B69F2E7D45",
    "cffi-2.1.1-cp314-cp314-win_amd64.whl":"3222BA5D678F80A030E6AFBCC33DC1AE5CB45FACABB61CEE2C7016B8432FDE48",
    "cffi-2.1.1-cp314-cp314t-win_amd64.whl":"616F097F2FE415BC92A247F02E11F634E1F9E9A83D327E3C915C15089C87869E",
    "pycparser-3.12-py3-none-any.whl":"A076F3F489B39B43ED13563A1DAFE42447445BEC2E294154EC5C57FCDDA4C85E"
}
def sha(b): return hashlib.sha256(b).hexdigest().upper()
def need(c,msg):
    if not c: raise AssertionError(msg)
def main():
    staged={}
    for n in ("apply.ps1",PROBE,LOCK,"bundle_spec.json","PACKAGE_README.txt"):
        p=STAGE/n
        need(p.is_file() and not p.is_symlink(),"MISSING_STAGE_"+n)
        staged[n]=p.read_bytes()
    need(len(list(STAGE.iterdir()))==5,"STAGE_MEMBERS_DRIFT")
    for n,h in ((PROBE,PROBE_SHA),(LOCK,LOCK_SHA)):
        need(sha(staged[n])==h and sha((REPO/n).read_bytes())==h,"SOURCE_PIN_"+n)
    ast.parse(staged[PROBE].decode("utf-8"))
    cfg=json.loads(staged[LOCK])
    need(cfg["status"]=="UNAPPROVED_SOURCE_METADATA_ONLY_NO_WHEEL_BYTES","SOURCE_CANDIDATE_NOT_LOCKED")
    need({x["filename"]:x["sha256"] for x in cfg["wheels"]}==EXPECTED_WHEEL,"METADATA_WHEEL_HASH_DRIFT")
    need(cfg["selected_versions"]=={"cryptography":"50.0.2","cffi":"2.1.1","pycparser":"3.12"},"VERSIONS")
    spec=json.loads(staged["bundle_spec.json"])
    need(spec["package_id"]==PKG and spec["execution_profile"]=="OFFLINE" and
         spec["runner_result_contract"]=="DEUS_OS_RUNNER_RESULT_V1","SPEC")
    source=staged["apply.ps1"].decode("utf-8")
    probe=staged[PROBE].decode("utf-8")
    need("$PackageId='"+PKG+"'" in source and "Set-StrictMode -Version 3" in source,"APPLY_CONTRACT")
    need("BoundedPy" in source and "ArgumentList.Add" in source and "'-3'" in source and
         "StandardInput.Close()" in source and "Kill($true)" in source and
         "BoundedPy $cmd.Source" in source and "15000" in source,"PYTHON_CHILD_BOUND")
    need("TERMINAL_EXCEPTION=" in source and "MIDDLE_TRUNCATED_TAIL_PRESERVED" in source,
         "FAIL_CAUSE_NOT_PRESERVED")
    need("function Seal(" in source and "CreateFromDirectory(" in source and
         "hashes.sha256" in source and "sizes.txt" in source and
         "FileMode]::CreateNew" in source,"EVIDENCE_SEAL_CONTRACT")
    need("Fingerprint" in source and "GIT_PRE_POST_IDENTICAL" in source,"GIT")
    for bad in ("pip install","pip uninstall","Invoke-WebRequest","Invoke-RestMethod",
                "Start-Process","ProtectedRecovery","nonce_ledger_offline","ST-LINK",
                "stm32cube","Set-MpPreference"):
        need(bad.lower() not in source.lower(),"UNAUTHORIZED_OPERATOR_CODE_"+bad)
    for bad in ("import subprocess","from subprocess","import urllib","from urllib","pip._internal.cli","os.system("):
        need(bad not in probe,"UNAUTHORIZED_PROBE_CODE_"+bad)
    need("cryptography" in probe and "cffi" in probe and "pycparser" in probe,
         "DEPENDENCY_GRAPH_MISSING")
    need("gil_disabled" in probe and "architecture_bits" in probe and "pip_tags_first_30" in probe,
         "RUNTIME_ABI_NOT_CAPTURED")
    need("python_executable_path" in probe and "REDACTED" in probe,"HOST_PATH_REDACTION")
    # Additional independent direct static check of known unsafe evidence formatting.
    from importlib.machinery import SourceFileLoader
    guard=SourceFileLoader("bundle_guard",(ROOT/"OS/scripts/operator_bundle_guard/bundle_guard.py").as_posix()).load_module()
    guard.check_apply(staged["apply.ps1"])
    output=ROOT/"Bundle"/(PKG+".zip")
    zresult="NOT_BUILT"
    if output.exists():
        result=guard.validate_file(output)
        need(result["profile"]=="OFFLINE" and
             result["qualification"]=="STATIC_ONLY_NOT_NATIVE_OR_HARDWARE",
             "CANONICAL_VALIDATE")
        with zipfile.ZipFile(output) as z:
            need(z.testzip() is None,"ZIP_CRC")
            for n in (PROBE,LOCK,"apply.ps1","PACKAGE_README.txt"):
                need(z.read(n)==staged[n],"ZIP_STAGE_BYTES_"+n)
        zresult="CANONICAL_STATIC_VALIDATED"
    print(json.dumps({
      "result":"A05_PYTHON_DEPENDENCY_INVENTORY_SOURCE_ONLY_PASS",
      "package_id":PKG,
      "probe_sha256":PROBE_SHA,
      "candidate_metadata_sha256":LOCK_SHA,
      "candidate_wheel_hashes":"5_PYPI_METADATA_ONLY_NOT_DOWNLOADED",
      "windows_native":"NOT_RUN",
      "installation":"NOT_PERFORMED",
      "zip":zresult
    },sort_keys=True))
if __name__=="__main__":main()
