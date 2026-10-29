#!/usr/bin/env python3
"""Read-only, no-network auditor of future exact Python314 Windows wheelhouse.

No download, install, import of wheel code, environment or device mutation.
Published PyPI SHA list is a SOURCE METADATA pin until wheel bytes are present.
"""
import argparse
import base64
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import zipfile
from email.parser import Parser

REPO=Path(__file__).resolve().parents[2]
LOCK=REPO/"scripts/gate4_entry/a05_windows_python_dependency_candidates_v1.json"
def need(ok,code):
    if not ok:raise AssertionError(code)
def sha(b):return hashlib.sha256(b).hexdigest().upper()
def safe_name(n):
    return (bool(re.fullmatch(r"[A-Za-z0-9_.\/-]{1,190}",n)) and
            not n.startswith(("/",".","\\")) and
            all(s not in ("",".","..") for s in n.split("/")))
def verify_wheel(path,expected_sha,package,version,tag):
    need(path.is_file() and not path.is_symlink(),"WHEEL_NOT_REGULAR")
    size=path.stat().st_size
    need(1000<=size<=12_000_000,"WHEEL_SIZE_BOUND")
    original=path.read_bytes()
    need(sha(original)==expected_sha.upper(),"WHEEL_SHA_MISMATCH_"+package)
    with zipfile.ZipFile(io.BytesIO(original)) as z:
        infos=z.infolist()
        ns=[i.filename for i in infos]
        need(0<len(ns)<=500 and len(ns)==len(set(ns)) and all(safe_name(n) for n in ns),
             "WHEEL_MEMBER_UNSAFE")
        need(z.testzip() is None,"WHEEL_CRC")
        data={i.filename:z.read(i) for i in infos}
        need(sum(i.file_size for i in infos)<=60_000_000,"WHEEL_EXPANDED_TOO_LARGE")
        need(all(i.flag_bits&1==0 and i.file_size<=12_000_000 for i in infos),
             "WHEEL_ENCRYPTED_OR_LARGE")
    dist_prefix=package.replace("-","_")+"-"+version+".dist-info/"
    need(dist_prefix+"METADATA" in data and dist_prefix+"WHEEL" in data and
         dist_prefix+"RECORD" in data,"WHEEL_DISTINFO_MISSING")
    metadata=Parser().parsestr(data[dist_prefix+"METADATA"].decode("utf-8","strict"))
    wh=Parser().parsestr(data[dist_prefix+"WHEEL"].decode("utf-8","strict"))
    need(metadata.get("Name","").lower().replace("_","-")==package and
         metadata.get("Version")==version,"WHEEL_IDENTITY")
    tags=wh.get_all("Tag",[])
    need(tag in tags,"WHEEL_TAG_MISMATCH")
    requirements=metadata.get_all("Requires-Dist",[])
    if package=="cryptography":
        need(any("cffi" in r.lower() for r in requirements),"CRYPTOGRAPHY_CFFI_DEP_MISSING")
    if package=="cffi":
        need(any("pycparser" in r.lower() for r in requirements),"CFFI_PYCPARSER_DEP_MISSING")
    if package=="pycparser":
        need(not requirements,"PYCPARSER_UNEXPECTED_DEPENDENCY")
    seen=set()
    for row in csv.reader(io.StringIO(data[dist_prefix+"RECORD"].decode("utf-8","strict"))):
        need(len(row)==3 and row[0] in data and row[0] not in seen,
             "WHEEL_RECORD_ENTRY")
        name,hash_spec,sz=row
        seen.add(name)
        if name==dist_prefix+"RECORD":
            need(not hash_spec and not sz,"WHEEL_RECORD_SELF_HASHED")
            continue
        need(hash_spec.startswith("sha256="),"WHEEL_RECORD_SHA_MISSING")
        raw=base64.urlsafe_b64decode(hash_spec[7:]+"===")
        need(len(raw)==32 and raw==hashlib.sha256(data[name]).digest(),
             "WHEEL_RECORD_SHA_BAD")
        need(sz.isdigit() and int(sz)==len(data[name]),"WHEEL_RECORD_SIZE_BAD")
    need(seen==set(data),"WHEEL_RECORD_COVERAGE")
    return {"filename":path.name,"sha256":expected_sha.upper(),
            "bytes":size,"dist_name":package,"version":version,
            "wheel_tag":tag,"records":len(seen)}
def audit(directory,abi):
    need(abi in ("cp314","cp314t"),"BAD_ABI")
    need(directory.is_dir() and not directory.is_symlink(),"WHEELHOUSE_DIRECTORY_MISSING")
    cfg=json.loads(LOCK.read_text(encoding="utf-8"))
    need(cfg["status"]=="UNAPPROVED_SOURCE_METADATA_ONLY_NO_WHEEL_BYTES",
         "LOCK_STATUS_DRIFT")
    choices={e["filename"]:e for e in cfg["wheels"]}
    if abi=="cp314":
        expected=[
          ("cryptography-50.0.2-cp311-abi3-win_amd64.whl","cryptography","50.0.2","cp311-abi3-win_amd64"),
          ("cffi-2.1.1-cp314-cp314-win_amd64.whl","cffi","2.1.1","cp314-cp314-win_amd64")]
    else:
        expected=[
          ("cryptography-50.0.2-cp314-cp314t-win_amd64.whl","cryptography","50.0.2","cp314-cp314t-win_amd64"),
          ("cffi-2.1.1-cp314-cp314t-win_amd64.whl","cffi","2.1.1","cp314-cp314t-win_amd64")]
    expected.append(("pycparser-3.12-py3-none-any.whl","pycparser","3.12","py3-none-any"))
    need({p.name for p in directory.iterdir()}=={x[0] for x in expected},
         "WHEELHOUSE_EXACT_SET_REQUIRED")
    results=[]
    for filename,name,version,tag in expected:
        results.append(verify_wheel(directory/filename,
                         choices[filename]["sha256"],name,version,tag))
    print(json.dumps({"result":"SOURCE_PINNED_WHEELHOUSE_BYTES_VERIFIED_NO_INSTALL",
       "abi":abi,"wheels":results,"host_python_modified":False,
       "network":False,"gate4":"NOT_AUTHORIZED"},sort_keys=True))
if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--directory",type=Path,required=True)
    parser.add_argument("--abi",choices=["cp314","cp314t"],required=True)
    args=parser.parse_args()
    audit(args.directory,args.abi)
