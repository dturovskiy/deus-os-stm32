#!/usr/bin/env python3
"""One scoped native A-05 isolated-controller proof. NO signing, nonce or hardware.

Executes on owner Windows as py.exe -3 with a *child-only* pinned PYTHONPATH.
Checks original HostDependencies target and original Windows Evidence before
importing source-locked RA-01/02/composition/controller Python modules.
Exercises FAIL-closed missing config, never a real approval/production ledger.
"""
from __future__ import annotations
from contextlib import redirect_stdout
import hashlib
import importlib.metadata as metadata
import io
import json
import os
from pathlib import Path, PureWindowsPath
import re
import sys
import sysconfig

TARGET_NAME="a05_cp314_isolated_import_v1_ed2d2b32da064f4a9d8ba141847a9f45"
TREE_PIN="31E08CB9B4AEE5917D4E2F9AF49B4D513370D6E4981331397D70590CF4A21BC4"
RECEIPT_PIN="75152279350DAFD4816F2FAE12D49A0C218F612BEBA6981B70A9EE9EB6AE4A3A"
EVIDENCE_BASENAME=("stm32_os_gate4_a05_cp314_isolated_deploy_v2_20261029_"
                   "20261029_003947_c4a078e0bdab489eb40171cc07fa4399.evidence.zip")
EVIDENCE_PIN="1287E148FDAF78B7B79EF1E27F56E0576FAD94721B08588F2C6D0D4EE303A532"
WHEEL_PIN="CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C"
SOURCES={
 "release_approval_contract_v1.py":"E954EE53E04148F01710B0017E0D6742704C0DB82963EAE728781E009CB7C3F3",
 "verify_release_signature_v1.py":"33CB33FBB5AEAFE23A329BBC48B7F094AA4430B421F795E70DBF5067DAE73F5C",
 "release_admission_source_composition_v1.py":"6AC3FE87E36A0A79B029CF35D19D2F3DC1C96EBD935840E2AE5AA65E15DAF102",
 "owner_windows_release_controller_v1.py":"05B20216DD7354482C4AB009487DF4CF04E16FDB31482FC2DD67449F183CB2C6"
}
VERSIONS={"cryptography":"50.0.2","cffi":"2.1.1","pycparser":"3.12"}
def need(v,code):
    if not v:raise RuntimeError(code)
def sha(data):return hashlib.sha256(data).hexdigest().upper()
def tree_hash(root):
    need(root.is_dir() and not root.is_symlink(),"ISOLATED_TARGET_NOT_DIR")
    files=[]
    for p in root.rglob("*"):
        need(not p.is_symlink(),"ISOLATED_TARGET_SYMLINK")
        if p.is_dir():continue
        need(p.is_file() and p.stat().st_size<=16_000_000,"TARGET_FILE_BOUNDS")
        files.append(p)
    need(0<len(files)<=1200,"TARGET_MEMBERS_LIMIT")
    files.sort(key=lambda p:PureWindowsPath(p.relative_to(root).as_posix()))
    rows=[f"{sha(p.read_bytes())}  {p.stat().st_size}  {p.relative_to(root).as_posix()}"
          for p in files]
    return sha(("\n".join(rows)+"\n").encode()),len(files)
def validate_original(parent,target):
    receipt=target.parent/"ADMISSION.json"
    need(receipt.is_file() and not receipt.is_symlink(),"ORIGINAL_ADMISSION_RECEIPT_MISSING")
    need(sha(receipt.read_bytes())==RECEIPT_PIN,"ORIGINAL_ADMISSION_RECEIPT_SHA")
    data=json.loads(receipt.read_text(encoding="utf-8"))
    digest,count=tree_hash(target)
    need(digest==TREE_PIN==data["target_tree_sha256"] and
         count==174==data["target_member_count"],"INSTALLED_TREE_DRIFT")
    need(data["source_wheelhouse_sha256"]==WHEEL_PIN and
         data["global_python_install"]=="NO" and data["network"]=="NO",
         "ISOLATED_ADMISSION_SCOPE_DRIFT")
    p=parent/"Evidence"/EVIDENCE_BASENAME
    need(p.is_file() and not p.is_symlink() and
         sha(p.read_bytes())==EVIDENCE_PIN,"ORIGINAL_ISOLATED_EVIDENCE_SHA")
    return digest,count
def check_sources(source_dir):
    need(source_dir.is_dir(),"SOURCE_DIR_ABSENT")
    for name,pin in SOURCES.items():
        p=source_dir/name
        need(p.is_file() and not p.is_symlink() and sha(p.read_bytes())==pin,
             "LIVE_CONTROLLER_SOURCE_SHA_"+name)
def validate_runtime(target,source_dir):
    need(sys.platform=="win32" and sys.version_info[:3]==(3,14,0) and
         sysconfig.get_platform()=="win-amd64" and
         not bool(sysconfig.get_config_var("Py_GIL_DISABLED")),
         "NOT_WINDOWS_CP314_REGULAR")
    need(os.environ.get("PYTHONPATH") == str(target),"PYTHONPATH_NOT_EXACT_ISOLATED_TARGET")
    need(os.environ.get("PYTHONDONTWRITEBYTECODE")=="1" and
         os.environ.get("PYTHONNOUSERSITE")=="1","PYTHON_CHILD_ISOLATION_FLAGS")
    sys.path.insert(0,str(source_dir))
    import cryptography
    import cffi
    import pycparser
    import _cffi_backend
    import release_approval_contract_v1 as ra01
    import verify_release_signature_v1 as ra02
    import release_admission_source_composition_v1 as composition
    import owner_windows_release_controller_v1 as controller
    for module in (cryptography,cffi,pycparser,_cffi_backend):
        need(Path(module.__file__).resolve().is_relative_to(target.resolve()),
             "DEPENDENCY_IMPORT_OUTSIDE_TARGET_"+module.__name__)
    for module in (ra01,ra02,composition,controller):
        need(Path(module.__file__).resolve().parent==source_dir.resolve(),
             "CONTROLLER_SOURCE_IMPORT_DRIFT_"+module.__name__)
    for package,version in VERSIONS.items():
        need(metadata.version(package)==version,"PACKAGE_VERSION_DRIFT_"+package)
    need(controller.CONTROLLER_CLASS=="G4_A05_NONCE_RECORDED_SOURCE_CONTROLLER_ONLY",
         "UNEXPECTED_CONTROLLER_RELEASE_BRANCH")
    return controller
def exercise_denied(controller,missing):
    need(not missing.exists(),"NEGATIVE_TEST_PATH_ALREADY_EXISTS")
    stream=io.StringIO()
    # Public CLI parses a missing config then must reject before any
    # owner identity, signer, protected ledger, nonce or subprocess is reached.
    args=["--config",str(missing),"--statement",str(missing),
          "--signature",str(missing),"--workspace-root",str(missing.parent),
          "--pwsh",str(missing)]
    with redirect_stdout(stream):
        rc=controller.main(args)
    lines=stream.getvalue().splitlines()
    need(rc==1 and len(lines)==1,"DENIAL_EXIT_OR_TRANSCRIPT")
    result=json.loads(lines[0])
    need(result.get("result")=="FAIL" and
         result.get("authorization")=="DENIED" and
         result.get("release_admitted")=="NO" and
         result.get("device_io")=="NONE" and
         result.get("gate4")=="NOT_AUTHORIZED" and
         result.get("error")=="FILE_NOT_FOUND_OR_LINK",
         "DENIAL_SEMANTICS_INVALID")
def execute(root,source_dir):
    need(root.name=="OS" and
         source_dir.resolve()==(root/"scripts"/"gate4_entry").resolve(),
         "REPO_ROLE_MISMATCH")
    parent=root.parent
    target=parent/"HostDependencies"/TARGET_NAME/"python_target"
    need(not target.is_symlink(),"TARGET_REPARSE")
    digest,count=validate_original(parent,target)
    print("A05_CONTROLLER_CASE=PASS ORIGINAL_HOST_TARGET_SHA_AND_COUNT",flush=True)
    check_sources(source_dir)
    print("A05_CONTROLLER_CASE=PASS FOUR_SOURCE_PINS",flush=True)
    ctrl=validate_runtime(target,source_dir)
    print("A05_CONTROLLER_CASE=PASS NATIVE_CP314_TARGET_DEPENDENCIES_AND_MODULE_IMPORT",flush=True)
    exercise_denied(ctrl,parent/"HostDependencies"/"MISSING_A05_OWNER_CONFIG_DO_NOT_CREATE")
    print("A05_CONTROLLER_CASE=PASS NATIVE_CONTROLLER_DENIES_WITHOUT_OWNER_CONFIG",flush=True)
    print("DEUS_A05_CONTROLLER_NATIVE_JSON="+json.dumps({
      "schema":"DEUS_A05_ISOLATED_CONTROLLER_NATIVE_PROBE_V1","result":"PASS",
      "classification":"A05_ISOLATED_CONTROLLER_NATIVE_DENY_ONLY_SCOPED_PASS",
      "case_count":4,"controller_import":"PASS","authorization":"DENIED",
      "release_admitted":"NO","nonce":"NOT_USED","executor_started":"NO",
      "device_io":"NONE","original_tree_sha256":digest,
      "original_tree_members":count,"original_evidence_sha256":EVIDENCE_PIN,
      "global_python_install":"NO","gate4":"NOT_AUTHORIZED"
    },sort_keys=True,separators=(",",":")),flush=True)
def main():
    try:
        need(len(sys.argv)==3,"ARGUMENT_VECTOR_REQUIRED")
        root=Path(sys.argv[1]).resolve()
        source_dir=Path(sys.argv[2]).resolve()
        execute(root,source_dir)
        return 0
    except BaseException as exc:
        code=str(exc)
        if len(code)>440:
            code=code[:90]+" [MIDDLE_TRUNCATED_TAIL_PRESERVED] "+code[-290:]
        print("TERMINAL_EXCEPTION="+type(exc).__name__+": "+code,flush=True)
        print("DEUS_A05_CONTROLLER_NATIVE_JSON="+json.dumps({
          "schema":"DEUS_A05_ISOLATED_CONTROLLER_NATIVE_PROBE_V1",
          "result":"FAIL","classification":"A05_ISOLATED_CONTROLLER_NATIVE_STOP",
          "exception_type":type(exc).__name__,"terminal_message":code,
          "authorization":"DENIED","release_admitted":"NO","nonce":"NOT_USED",
          "executor_started":"NO","device_io":"NONE","gate4":"NOT_AUTHORIZED"
        },sort_keys=True,separators=(",",":")),flush=True)
        return 2
if __name__=="__main__":
    raise SystemExit(main())
