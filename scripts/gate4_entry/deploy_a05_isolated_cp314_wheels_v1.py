#!/usr/bin/env python3
"""A-05 Windows-only OFFLINE isolated dependency target, not controller release.

Input: exact original owner Wheelhouse ZIP and original evidence pin.
Writes only a new, unique workspace-sibling HostDependencies directory.
Local pip25.2 --isolated --no-index --no-deps --no-compile --no-cache-dir
--target into a disposable candidate, plus two bounded py.exe -3 imports.
No global site-packages install, network, owner approvals, nonce or device IO.
"""
from __future__ import annotations
import hashlib
import io
import importlib.metadata as md
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import sysconfig
import uuid
import zipfile

from audit_a05_windows_wheelhouse_v1 import verify_wheel

WHEELHOUSE_NAME="a05_python314_regular_wheels_20261028_04ba1366de464d8aad14622aa90ae956.zip"
WHEELHOUSE_SHA="CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C"
OWNER_EVIDENCE_NAME=("stm32_os_gate4_a05_python314_wheel_download_v1_20261028_"
                     "20261028_234401_166a876f51fd4febaa490cec9ae41239.evidence.zip")
OWNER_EVIDENCE_SHA="9D444C61709C9C98AAEB43B2B6D75E2AB0D4F02116D1B3F41B8990ED968617DD"
SELECTION_NAME="a05_windows_python314_regular_wheel_selection_v1.json"
MODULES={
 "release_approval_contract_v1.py":"E954EE53E04148F01710B0017E0D6742704C0DB82963EAE728781E009CB7C3F3",
 "verify_release_signature_v1.py":"33CB33FBB5AEAFE23A329BBC48B7F094AA4430B421F795E70DBF5067DAE73F5C",
 "release_admission_source_composition_v1.py":"6AC3FE87E36A0A79B029CF35D19D2F3DC1C96EBD935840E2AE5AA65E15DAF102",
 "owner_windows_release_controller_v1.py":"05B20216DD7354482C4AB009487DF4CF04E16FDB31482FC2DD67449F183CB2C6"
}
EXPECTED=[
("cryptography","50.0.2","cryptography-50.0.2-cp311-abi3-win_amd64.whl",
 "7AFA5A6602A9F29AF1F3A2965F831BAE7C9D5D597B7CBB716D41AB3B7D89879C","cp311-abi3-win_amd64"),
("cffi","2.1.1","cffi-2.1.1-cp314-cp314-win_amd64.whl",
 "3222BA5D678F80A030E6AFBCC33DC1AE5CB45FACABB61CEE2C7016B8432FDE48","cp314-cp314-win_amd64"),
("pycparser","3.12","pycparser-3.12-py3-none-any.whl",
 "A076F3F489B39B43ED13563A1DAFE42447445BEC2E294154EC5C57FCDDA4C85E","py3-none-any")
]
STEM="a05_cp314_isolated_import_v1_"
PROBE_CODE=r'''
from pathlib import Path
import sys,json,importlib.metadata as md
root=Path(sys.argv[1]).resolve()
sources=Path(sys.argv[2]).resolve()
sys.path.insert(0,str(sources))
import cryptography,cffi,pycparser,_cffi_backend
import release_approval_contract_v1
import verify_release_signature_v1
import release_admission_source_composition_v1
import owner_windows_release_controller_v1
assert sys.version_info[:3]==(3,14,0), "PYTHON_VERSION_CHANGED"
assert Path(cryptography.__file__).resolve().is_relative_to(root), "CRYPTOGRAPHY_OUTSIDE_ISOLATED_TARGET"
assert Path(cffi.__file__).resolve().is_relative_to(root), "CFFI_OUTSIDE_ISOLATED_TARGET"
assert Path(pycparser.__file__).resolve().is_relative_to(root), "PYCPARSER_OUTSIDE_ISOLATED_TARGET"
assert Path(_cffi_backend.__file__).resolve().is_relative_to(root), "CFFI_BINARY_OUTSIDE_TARGET"
for n in ("release_approval_contract_v1","verify_release_signature_v1","release_admission_source_composition_v1","owner_windows_release_controller_v1"):
    m=sys.modules[n]
    assert Path(m.__file__).resolve().parent==sources, "CONTROLLER_SOURCE_PATH_MISMATCH_"+n
for n,v in (("cryptography","50.0.2"),("cffi","2.1.1"),("pycparser","3.12")):
    assert md.version(n)==v,"DEPENDENCY_VERSION_MISMATCH_"+n
print("A05_ISOLATED_CONTROLLER_IMPORT=PASS")
'''
class Stop(Exception):
    pass
def need(ok,code):
    if not ok:raise Stop(code)
def sha(x):
    return hashlib.sha256(x).hexdigest().upper()
def report(event,detail=""):
    print("A05_DEPLOY="+event+(" DETAIL="+detail if detail else ""),flush=True)
def sanitized(text,limit=900):
    s=re.sub(r"(?i)[a-z]:\\[^\r\n \t\"']+","[LOCAL_PATH]",str(text))
    s=re.sub(r"[\x00-\x08\x0b-\x1f\x7f]","?",s)
    if len(s)>limit:
        s=s[:100]+" [MIDDLE_TRUNCATED_TAIL_PRESERVED] "+s[-(limit-160):]
    return s
def bounded(argv,env,timeout,stage):
    report("CHILD_START",stage)
    try:
        child=subprocess.run(argv,env=env,capture_output=True,text=True,
                             encoding="utf-8",errors="replace",timeout=timeout,
                             stdin=subprocess.DEVNULL,check=False)
    except subprocess.TimeoutExpired as exc:
        report("CHILD_TIMEOUT",stage+" "+str(timeout)+"sec")
        raise Stop("TimeoutError: "+stage+"_EXCEEDED_"+str(timeout)+"SECONDS") from exc
    except OSError as exc:
        raise Stop("ChildLaunchError: "+stage+"_"+type(exc).__name__) from exc
    report("CHILD_EXIT",stage+" CODE="+str(child.returncode))
    # Keep both streams, final exception line and bounded transcript tail.
    for label,payload in (("STDOUT",child.stdout),("STDERR",child.stderr)):
        if payload:
            lines=payload.splitlines()
            if len(lines)>55:
                lines=lines[:10]+["[MIDDLE_TRUNCATED_TAIL_PRESERVED]"]+lines[-45:]
            for line in lines:
                report("CHILD_"+label,sanitized(stage+": "+line))
    if child.returncode!=0:
        last=next((x.strip() for x in reversed(child.stderr.splitlines()) if x.strip()),"NO_CHILD_STDERR")
        raise Stop("ChildExitError: "+stage+"_EXIT_"+str(child.returncode)+
                   " TERMINAL="+sanitized(last,500))
    return child
def safe_parent(p):
    need(p.is_absolute(),"ABSOLUTE_PATH_REQUIRED")
    need(not str(p).startswith("\\\\"),"UNC_DENIED")
    for anc in (p,*p.parents):
        if anc.exists():
            need(not anc.is_symlink(),"SYMLINK_DENIED")
    return p
def compute_tree(root):
    lines=[]
    for f in sorted(root.rglob("*")):
        if f.is_dir():
            need(not f.is_symlink(),"DEPLOY_DIR_SYMLINK")
            continue
        need(f.is_file() and not f.is_symlink(),"DEPLOY_NONFILE")
        rel=f.relative_to(root).as_posix()
        need(len(rel)<=250 and f.stat().st_size<=16_000_000,"DEPLOY_FILE_BOUNDS")
        lines.append(f"{sha(f.read_bytes())}  {f.stat().st_size}  {rel}")
    need(len(lines)<=1200,"DEPLOY_FILES_EXCEEDED")
    return sha(("\n".join(lines)+"\n").encode()),len(lines)
def preflight(wheel_dir,stage,repo):
    need(sys.platform=="win32" and platform.python_implementation()=="CPython"
         and sys.version_info[:3]==(3,14,0) and sysconfig.get_platform()=="win-amd64"
         and not bool(sysconfig.get_config_var("Py_GIL_DISABLED")),
         "PYTHON314_WIN64_REGULAR_REQUIRED")
    need(md.version("pip")=="25.2","PINNED_PIP25_2_REQUIRED")
    need(wheel_dir.name=="Wheelhouse" and repo.name=="OS","OWNER_WORKSPACE_SHAPE")
    need(repo.parent==wheel_dir.parent,"WORKSPACE_PARENT_MISMATCH")
    need(safe_parent(wheel_dir).is_dir() and safe_parent(repo).is_dir(),
         "OWNER_DIR_MISSING")
    src_dir=repo/"scripts"/"gate4_entry"
    for name,pin in MODULES.items():
        path=src_dir/name
        need(path.is_file() and sha(path.read_bytes())==pin,"CONTROLLER_SOURCE_PIN_"+name)
    report("CONTROLLER_SOURCES_PINNED",str(len(MODULES)))
    selection=json.loads((stage/SELECTION_NAME).read_text("utf-8"))
    need(selection["owner_windows_inventory"]["gil_disabled"] is False
         and selection["owner_windows_inventory"]["target_wheel_abi"]=="cp314",
         "ABI_SELECTION_DRIFT")
    locked=[(w["name"],w["version"],w["filename"],w["sha256"])
            for w in selection["selected_wheels"]]
    need(locked==[(a,b,c,d) for a,b,c,d,_ in EXPECTED],"SOURCE_LOCK_DRIFT")
    original_evidence=repo.parent/"Evidence"/OWNER_EVIDENCE_NAME
    need(original_evidence.is_file() and
         sha(original_evidence.read_bytes())==OWNER_EVIDENCE_SHA,"ORIGINAL_EVIDENCE_SHA")
    zpath=wheel_dir/WHEELHOUSE_NAME
    need(safe_parent(zpath).is_file() and
         zpath.stat().st_size==4076132 and
         sha(zpath.read_bytes())==WHEELHOUSE_SHA,
         "ORIGINAL_WHEELHOUSE_SHA")
    report("OWNER_WHEELHOUSE_PINNED","3_WHEELS_ORIGINAL_SHA")
    with zipfile.ZipFile(zpath) as z:
        members=z.namelist()
        need(len(members)==4 and len(set(members))==4 and
             set(members)=={"SHA256SUMS.txt"}|{x[2] for x in EXPECTED},
             "WHEELHOUSE_MEMBER_DRIFT")
        need(z.testzip() is None,"WHEELHOUSE_CRC_FAIL")
        manifest=z.read("SHA256SUMS.txt").decode("ascii").splitlines()
        need(set(manifest)=={d+"  "+c for _,_,c,d,_ in EXPECTED},
             "WHEELHOUSE_SHA_INDEX")
        return {fn:z.read(fn) for _,_,fn,_,_ in EXPECTED},src_dir
def execute(wheel_dir,host_root,repo,stage,launcher):
    archive,src=preflight(wheel_dir,stage,repo)
    host_root=safe_parent(host_root)
    need(host_root.name=="HostDependencies" and host_root.parent==repo.parent,
         "TARGET_MUST_BE_OFF_GIT_SIBLING")
    need(not host_root.exists() or host_root.is_dir(),"TARGET_ROOT_NOT_DIR")
    host_root.mkdir(exist_ok=True)
    runid=uuid.uuid4().hex
    scratch=host_root/(".pending_"+STEM+runid)
    final=host_root/(STEM+runid)
    need(not scratch.exists() and not final.exists(),"TARGET_COLLISION")
    scratch.mkdir()
    published=False
    try:
        wh=scratch/"wheel_inputs"
        wh.mkdir()
        target=scratch/"python_target"
        target.mkdir()
        for package,version,fn,pin,tag in EXPECTED:
            wheel=wh/fn
            with wheel.open("xb") as file:
                file.write(archive[fn])
                file.flush()
                os.fsync(file.fileno())
            verify_wheel(wheel,pin,package,version,tag)
            report("WHEEL_SHA_CRC_RECORD","PASS "+package)
        env=os.environ.copy()
        env["PIP_NO_INDEX"]="1"
        env["PIP_DISABLE_PIP_VERSION_CHECK"]="1"
        env["PYTHONDONTWRITEBYTECODE"]="1"
        env["PYTHONNOUSERSITE"]="1"
        env.pop("PYTHONPATH",None)
        env.pop("PIP_INDEX_URL",None)
        # --isolated disregards host pip user configuration/env; explicit
        # --no-index, --no-deps and three exact filename paths ban network.
        args=[sys.executable,"-m","pip","--isolated","--disable-pip-version-check",
              "install","--no-index","--no-deps","--no-compile",
              "--no-cache-dir","--no-input","--target",str(target),
              *[str(wh/fn) for _,_,fn,_,_ in EXPECTED]]
        bounded(args,env,100,"OFFLINE_PIP_EXACT_TARGET")
        report("OFFLINE_TARGET_INSTALLED","3")
        env["PYTHONPATH"]=str(target)
        probe=[launcher,"-3","-c",PROBE_CODE,str(target),str(src)]
        bounded(probe,env,25,"CHILD_PY_EXPLICIT_TARGET_IMPORT")
        report("ISOLATED_CONTROLLER_IMPORT","PASS_TEMP")
        # remove original wheel inputs from staging; not required at runtime
        shutil.rmtree(wh)
        tree,count=compute_tree(target)
        receipt={
          "schema":"DEUS_A05_ISOLATED_CP314_DEPLOY_RECEIPT_V1",
          "source_wheelhouse_sha256":WHEELHOUSE_SHA,
          "source_download_evidence_sha256":OWNER_EVIDENCE_SHA,
          "python_version":"3.14.0",
          "pip_version":"25.2",
          "installed_only_to":"HOST_DEPENDENCIES_ISOLATED_TARGET",
          "target_tree_sha256":tree,
          "target_member_count":count,
          "controller_import_verified":"TEMP_TARGET",
          "nonce":"NOT_USED","device_io":"NONE","release":"DENIED",
          "global_python_install":"NO","network":"NO"
        }
        receiptbytes=(json.dumps(receipt,sort_keys=True,indent=2)+"\n").encode()
        (scratch/"ADMISSION.json").write_bytes(receiptbytes)
        # Promote only after all source pins, real wheel bytes, actual offline
        # installation, Python module imports and staged tree hash succeeded.
        scratch.rename(final)
        published=True
        report("TARGET_ATOMIC_PROMOTION","PASS BASENAME="+final.name)
        # Verify the imported environment again after final rename.
        env["PYTHONPATH"]=str(final/"python_target")
        check=[launcher,"-3","-c",PROBE_CODE,str(final/"python_target"),str(src)]
        bounded(check,env,25,"CHILD_PY_FINAL_TARGET_IMPORT")
        report("ISOLATED_CONTROLLER_IMPORT","PASS_FINAL")
        return {
          "schema":"DEUS_A05_CP314_ISOLATED_DEPLOY_V1","result":"PASS",
          "classification":"A05_ISOLATED_HOST_DEPENDENCIES_AND_CONTROLLER_IMPORT_SCOPED_PASS",
          "deployment_basename":final.name,
          "target_tree_sha256":tree,
          "target_member_count":count,
          "receipt_sha256":sha(receiptbytes),
          "python_env_global_modified":"NO",
          "python_import":"PASS_FINAL",
          "wheelhouse_original_sha256":WHEELHOUSE_SHA,
          "no_network":"YES","no_device":"YES","no_nonce":"YES"
        }
    finally:
        if not published and scratch.exists():
            shutil.rmtree(scratch,ignore_errors=True)
        # If an error occurs after promotion, leave the already published
        # target for separate custody inspection, not an unlogged deletion.
def main():
    try:
        need(os.name=="nt","WINDOWS_ONLY")
        need(len(sys.argv)==6,"EXPECTED_EXACT_ARGV")
        wh,host,repo,stage,launcher=map(Path,sys.argv[1:])
        need(launcher.name.lower()=="py.exe" and launcher.is_file(),
             "PY_LAUNCHER_REQUIRED")
        result=execute(wh,host,repo,stage,str(launcher))
        print("DEUS_ISOLATED_DEPLOY_JSON="+json.dumps(result,sort_keys=True,separators=(",",":")),flush=True)
        return 0
    except BaseException as e:
        code=sanitized(str(e),420)
        print("TERMINAL_EXCEPTION="+type(e).__name__+": "+code,flush=True)
        print("DEUS_ISOLATED_DEPLOY_JSON="+json.dumps({
            "result":"FAIL","classification":"A05_ISOLATED_DEPLOY_STOP",
            "exception_type":type(e).__name__,"terminal_message":code,
            "global_python_install":"NO","device_io":"NONE",
            "network":"NO","nonce":"NOT_USED"
        },sort_keys=True,separators=(",",":")),flush=True)
        return 2
if __name__=="__main__":
    raise SystemExit(main())
