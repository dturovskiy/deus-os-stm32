#!/usr/bin/env python3
"""Gate4 A-05 owner-Windows protected admission controller SOURCE CANDIDATE.

NOT installed, NOT a device executor, NOT hardware release. This source
verifies an exact ZIP, externally owner-pinned source/provenance values, live
canonical RA-01 contract, independently pinned real owner Ed25519 SSHSIG, and
accepted original RA-03 Windows qualification before calling the owner-local
PowerShell RA-03 module once. Even durable nonce reservation returns DENIED.
The later independent physical executor admission is deliberately absent.
"""
from __future__ import annotations
import argparse
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import release_approval_contract_v1 as ra01
import verify_release_signature_v1 as ra02
import release_admission_source_composition_v1 as composition

MAX_CONFIG=4096
MAX_SIG=2048
MAX_ZIP=30_000_000
MAX_OUT=4096
HEX64=re.compile(r"[0-9A-F]{64}\Z")
# A pinned identity is mandatory even for the independent adapter; update via
# a reviewed new controller source version when that adapter actually changes.
ADAPTER_SHA256="E4766F4FB5595475B3D886AF36EB9E8BE52A34F503DA0725DB28B7204D17E699"
LEDGER_SHA256="D67EB9820C016902FCDBC7FD3A13A51AE3F1317C33927E60B49D6B5565C351BC"
CONTROLLER_CLASS="G4_A05_NONCE_RECORDED_SOURCE_CONTROLLER_ONLY"

class Stop(Exception):
    pass

def need(ok, code):
    if not ok:
        raise Stop(code)

def sha(b):
    return hashlib.sha256(b).hexdigest().upper()

def no_reparse(path: Path):
    """No symlinks or junctions in any path ancestor, no UNC or ADS."""
    s=str(path)
    need(path.is_absolute() and not s.startswith("\\\\") and
         not s.startswith("//"),"UNSAFE_PATH_ROOT")
    need(not any(x==".." for x in path.parts),"UNSAFE_PATH_TRAVERSAL")
    for part in [path]+list(path.parents):
        if part.exists() or part.is_symlink():
            need(not part.is_symlink(),"UNSAFE_SYMLINK")
            if os.name=="nt":
                need(not(os.stat(part,follow_symlinks=False).st_file_attributes & 0x400),
                     "UNSAFE_REPARSE_POINT")
    return path

def exact_file(path: Path, limit: int):
    no_reparse(path)
    need(path.is_file() and not path.is_symlink(),"FILE_NOT_FOUND_OR_LINK")
    st=path.stat()
    need(0 < st.st_size <= limit,"FILE_BOUNDS")
    data=path.read_bytes()
    need(len(data)==st.st_size,"FILE_CHANGED_DURING_READ")
    return data

def strict_json(data: bytes):
    def pairs(rows):
        result={}
        for key,value in rows:
            need(key not in result,"DUPLICATE_CONFIG_FIELD")
            result[key]=value
        return result
    need(not data.startswith(b"\xef\xbb\xbf") and b"\x00" not in data,
         "CONFIG_ENCODING")
    try:
        obj=json.loads(data.decode("utf-8","strict"),object_pairs_hook=pairs,
                       parse_constant=lambda _ : (_ for _ in ()).throw(Stop("CONFIG_NONFINITE")))
    except (UnicodeError,json.JSONDecodeError,TypeError) as exc:
        raise Stop("INVALID_CONFIG_JSON") from exc
    need(type(obj) is dict,"CONFIG_NOT_OBJECT")
    return obj

@dataclass(frozen=True)
class LocalConfig:
    zip_path: Path
    bundle_root: Path
    ledger_dir: Path
    adapter_path: Path
    module_path: Path
    material: ra01.TrustedMaterial

def load_owner_config(path: Path):
    """Config must live in protected *owner-local* area, never in Bundle."""
    config= strict_json(exact_file(path,MAX_CONFIG))
    fields={
        "schema","bundle_root","candidate_zip","ledger_directory",
        "adapter_path","module_path","operator_zip_sha256","operator_zip_bytes",
        "source_lock_sha256","source_git_tree","build_provenance_sha256",
        "entrypoint_sha256","readiness_evidence_sha256","device_usb_port",
        "host_identity"
    }
    need(set(config)==fields,"CONFIG_FIELDS")
    need(config["schema"]=="DEUS_OS_GATE4_CONTROLLER_CONFIG_V1",
         "CONFIG_SCHEMA")
    for key in fields-{"operator_zip_bytes"}:
        need(type(config[key]) is str,"CONFIG_TYPE_"+key)
    need(type(config["operator_zip_bytes"]) is int,"CONFIG_ZIP_SIZE_TYPE")
    material=ra01.TrustedMaterial(
        zip_sha256=config["operator_zip_sha256"],
        zip_bytes=config["operator_zip_bytes"],
        source_lock_sha256=config["source_lock_sha256"],
        source_git_tree=config["source_git_tree"],
        build_provenance_sha256=config["build_provenance_sha256"],
        entrypoint_sha256=config["entrypoint_sha256"],
        readiness_evidence_sha256=config["readiness_evidence_sha256"],
        device_usb_port=config["device_usb_port"],
        host_identity=config["host_identity"],
    )
    material.verify()
    paths={name:no_reparse(Path(config[name])) for name in (
        "bundle_root","candidate_zip","ledger_directory","adapter_path","module_path"
    )}
    need(path.parent != paths["bundle_root"] and
         paths["bundle_root"] not in path.parents, "CONFIG_WITHIN_BUNDLE")
    need(paths["candidate_zip"].parent == paths["bundle_root"],
         "CANDIDATE_OUTSIDE_BUNDLE")
    need(paths["candidate_zip"].name == ra01.PKG_ID+".zip",
         "CANDIDATE_WRONG_PACKAGE_ID")
    need(paths["ledger_directory"] != paths["bundle_root"] and
         paths["bundle_root"] not in paths["ledger_directory"].parents and
         paths["ledger_directory"] != paths["candidate_zip"].parent,
         "LEDGER_INSIDE_BUNDLE")
    need(paths["adapter_path"].parent != paths["bundle_root"] and
         paths["module_path"].parent != paths["bundle_root"] and
         paths["bundle_root"] not in paths["adapter_path"].parents and
         paths["bundle_root"] not in paths["module_path"].parents,
         "CONTROLLER_SOURCE_INSIDE_BUNDLE")
    need(paths["ledger_directory"].is_dir(),"LEDGER_NOT_PREPROVISIONED")
    need(path.is_file(),"CONFIG_MISSING")
    return LocalConfig(paths["candidate_zip"],paths["bundle_root"],
                       paths["ledger_directory"],paths["adapter_path"],
                       paths["module_path"],material)

def check_source_files(config: LocalConfig):
    need(HEX64.fullmatch(ADAPTER_SHA256) is not None,"ADAPTER_SOURCE_NOT_FROZEN")
    need(sha(exact_file(config.adapter_path,100_000))==ADAPTER_SHA256,
         "ADAPTER_SHA_MISMATCH")
    need(sha(exact_file(config.module_path,100_000))==LEDGER_SHA256,
         "RA03_MODULE_SHA_MISMATCH")

def inspect_exact_approval(statement: bytes, signature: bytes,
                           cfg: LocalConfig, now: datetime,
                           workspace_root: Path):
    # No test key can be passed on the production path.
    return composition.inspect_source_composition(
        statement, signature, cfg.zip_path,cfg.material,now,
        bundle_root=cfg.bundle_root,workspace_root=workspace_root)

def reserve_once(*,cfg:LocalConfig, nonce:str, statement_sha256:str,
                 operator_sha256:str, executable:Path, runner=None):
    """One Windows adapter invocation, no retry after ambiguous launch."""
    need(sys.platform=="win32","WINDOWS_ONLY")
    need(re.fullmatch(r"[a-f0-9]{32}",nonce) is not None and
         nonce!="0"*32,"NONCE_INVALID")
    for value in (statement_sha256,operator_sha256):
        need(HEX64.fullmatch(value) is not None,"SHA_INVALID")
    check_source_files(cfg)
    need(executable.is_file() and executable.name.lower()=="pwsh.exe",
         "POWERSHELL_7_MISSING")
    command=[str(executable),"-NoLogo","-NoProfile","-NonInteractive",
             "-File",str(cfg.adapter_path),"-LedgerDirectory",str(cfg.ledger_dir),
             "-LedgerModule",str(cfg.module_path),"-Nonce",nonce,
             "-StatementSha256",statement_sha256,"-OperatorZipSha256",operator_sha256]
    run=runner if runner is not None else subprocess.run
    try:
        result=run(command,capture_output=True,timeout=30,check=False)
    except Exception as exc:
        raise Stop("NONCE_POSSIBLY_CONSUMED_NO_RETRY") from exc
    need(result.returncode==0,"NONCE_POSSIBLY_CONSUMED_NO_RETRY")
    need(type(result.stdout) is bytes and len(result.stdout)<=MAX_OUT and
         not result.stderr,"NONCE_RECEIPT_UNTRUSTED_NO_RETRY")
    reply=strict_json(result.stdout)
    required={"classification","nonce","record_sha256","authorization",
              "launch_authorized","device_io"}
    need(set(reply)==required and
         reply["classification"]=="RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED" and
         reply["nonce"]==nonce and
         type(reply["record_sha256"]) is str and HEX64.fullmatch(reply["record_sha256"]) and
         reply["authorization"]=="DENIED" and
         reply["launch_authorized"] is False and reply["device_io"]=="NONE",
         "NONCE_RECEIPT_UNTRUSTED_NO_RETRY")
    return reply

def execute_source_controller(config_path:Path,statement_path:Path,
                              signature_path:Path,workspace_root:Path,
                              pwsh_path:Path,now:datetime|None=None):
    """Real signature accepted => at most one nonce reservation, no launch."""
    need(sys.platform=="win32","WINDOWS_ONLY")
    need(now is None or (now.tzinfo is not None),"CLOCK_REQUIRED")
    config=load_owner_config(config_path)
    check_source_files(config)
    statement=exact_file(statement_path,4096)
    signature=exact_file(signature_path,MAX_SIG)
    need(config.bundle_root not in statement_path.parents and
         config.bundle_root not in signature_path.parents,"APPROVAL_WITHIN_BUNDLE")
    observed=datetime.now(timezone.utc) if now is None else now
    verified=inspect_exact_approval(statement,signature,config,observed,workspace_root)
    nonce=ra01.parse_canonical(statement)["nonce"]
    result=reserve_once(cfg=config,nonce=nonce,
                        statement_sha256=verified["statement_sha256"],
                        operator_sha256=verified["candidate_zip_sha256"],
                        executable=pwsh_path)
    return {"result":"PASS","classification":CONTROLLER_CLASS,
            "statement_sha256":verified["statement_sha256"],
            "candidate_zip_sha256":verified["candidate_zip_sha256"],
            "nonce_record_sha256":result["record_sha256"],
            "nonce_consumed":"YES","authorization":"DENIED",
            "release_admitted":"NO","executor_started":"NO",
            "controller_installed":"NOT_ATTESTED",
            "device_io":"NONE","gate4":"NOT_AUTHORIZED"}

def main(argv=None):
    parser=argparse.ArgumentParser(description="Gate4 offline source-only Windows controller; NO hardware")
    parser.add_argument("--config",required=True,type=Path)
    parser.add_argument("--statement",required=True,type=Path)
    parser.add_argument("--signature",required=True,type=Path)
    parser.add_argument("--workspace-root",required=True,type=Path)
    parser.add_argument("--pwsh",required=True,type=Path)
    args=parser.parse_args(argv)
    try:
        r=execute_source_controller(args.config,args.statement,args.signature,
                                    args.workspace_root,args.pwsh)
        print(json.dumps(r,sort_keys=True))
        return 0
    except Exception as ex:
        # Never leak original byte material, owner account paths or keys.
        code=str(ex) if isinstance(ex,(Stop,ra01.Reject)) else "CONTROLLER_UNEXPECTED_STOP"
        print(json.dumps({"result":"FAIL","classification":"G4_A05_CONTROLLER_STOP",
                          "error":code,"authorization":"DENIED",
                          "release_admitted":"NO","device_io":"NONE",
                          "gate4":"NOT_AUTHORIZED"},sort_keys=True))
        return 1

if __name__=="__main__":
    raise SystemExit(main())
