#!/usr/bin/env python3
"""Independent no-extraction audit of ORIGINAL owner Windows A05 wheelhouse bytes.

Requires exact original workspace-sibling Wheelhouse basename and SHA. Validates
outer CRC/index and all original wheel bytes, inner ZIP CRC/metadata/RECORD.
Does not execute, import, extract or install wheel code; no network/device I/O.
"""
from __future__ import annotations

import argparse
import base64
import csv
from email.parser import Parser
import hashlib
import io
import json
from pathlib import Path
import re
import stat
import zipfile

ROOT=Path(__file__).resolve().parents[3]
WHEEL_NAME="a05_python314_regular_wheels_20261028_04ba1366de464d8aad14622aa90ae956.zip"
WHEEL_SHA="CD45CB76A47C893DE0032BF53E3EAB1304F0822D56D29254CB10A0888702198C"
ORIGINAL_EVIDENCE_NAME=("stm32_os_gate4_a05_python314_wheel_download_v1_20261028_"
                        "20261028_234401_166a876f51fd4febaa490cec9ae41239.evidence.zip")
ORIGINAL_EVIDENCE_SHA="9D444C61709C9C98AAEB43B2B6D75E2AB0D4F02116D1B3F41B8990ED968617DD"
SOURCE_SELECTION=ROOT/"OS/scripts/gate4_entry/a05_windows_python314_regular_wheel_selection_v1.json"

def need(test,msg):
    if not test:
        raise AssertionError(msg)

def sha(blob):
    return hashlib.sha256(blob).hexdigest().upper()

def safe_name(n):
    return (isinstance(n,str) and len(n)<=190 and
        re.fullmatch(r"[a-zA-Z0-9_./-]+",n) is not None and
        not n.startswith(("/",".","\\")) and
        all(part not in ("",".","..") for part in n.split("/")))

def zip_data(blob,exact=None,allowed_max=1000):
    with zipfile.ZipFile(io.BytesIO(blob)) as zin:
        members=zin.infolist()
        names=[x.filename for x in members]
        need(0<len(members)<=allowed_max and len(names)==len(set(names)),
             "DUPLICATE_OR_UNBOUNDED_ZIP_MEMBER")
        need(all(safe_name(x) for x in names),"UNSAFE_ZIP_MEMBER")
        if exact is not None:
            need(set(names)==exact and len(names)==len(exact),"EXACT_ARCHIVE_MEMBERS")
        need(all(not x.is_dir() and x.flag_bits & 1 == 0 and
                 not stat.S_ISLNK(x.external_attr >> 16) and
                 x.file_size<=15_000_000 for x in members),"UNSAFE_MEMBER_CONTENT")
        need(sum(x.file_size for x in members)<=80_000_000,"ZIP_EXPANSION_LIMIT")
        need(zin.testzip() is None,"ORIGINAL_ARCHIVE_CRC_FAIL")
        data={n:zin.read(n) for n in names}
        return data

def verify_inner_wheel(blob,package,version,filename,tag):
    need(filename.endswith(".whl"),"WHEEL_NAME")
    need(1000<=len(blob)<=12_000_000,"WHEEL_SIZE")
    data=zip_data(blob,allowed_max=800)
    dist=package.replace("-","_")+"-"+version+".dist-info/"
    required={dist+"METADATA",dist+"WHEEL",dist+"RECORD"}
    need(required<=data.keys(),"DIST_INFO_REQUIRED")
    meta=Parser().parsestr(data[dist+"METADATA"].decode("utf-8","strict"))
    wheel=Parser().parsestr(data[dist+"WHEEL"].decode("utf-8","strict"))
    need(meta.get("Name","").lower().replace("_","-")==package and
         meta.get("Version")==version,"WHEEL_METADATA_IDENTITY")
    tags=wheel.get_all("Tag",[])
    need(tag in tags,"WHEEL_TAG_INCOMPATIBLE")
    dependencies=[x.strip().lower() for x in meta.get_all("Requires-Dist",[])]
    if package=="cryptography":
        need(any(x.startswith("cffi") for x in dependencies),"CRYPTOGRAPHY_CFFI_MISSING")
    elif package=="cffi":
        need(any(x.startswith("pycparser") for x in dependencies),"CFFI_PYCPARSER_MISSING")
    elif package=="pycparser":
        need(not dependencies,"PYCPARSER_UNEXPECTED_REQUIRES_DIST")
    got=set()
    csvtext=data[dist+"RECORD"].decode("utf-8","strict")
    for record in csv.reader(io.StringIO(csvtext)):
        need(len(record)==3 and record[0] in data and
             record[0] not in got,"RECORD_INVALID_MEMBER")
        name,pin,declared_size=record
        got.add(name)
        if name==dist+"RECORD":
            need(pin=="" and declared_size=="","SELF_RECORD_MUST_BE_EMPTY")
        else:
            need(pin.startswith("sha256="),"RECORD_UNPINNED_MEMBER")
            dig=base64.urlsafe_b64decode(pin[7:]+"===")
            need(len(dig)==32 and dig==hashlib.sha256(data[name]).digest(),
                 "RECORD_SHA256_BAD")
            need(declared_size.isdecimal() and
                 int(declared_size)==len(data[name]),"RECORD_SIZE_BAD")
    need(got==data.keys(),"RECORD_INCOMPLETE")
    return {"filename":filename,"whole_sha256":sha(blob),
            "bytes":len(blob),"zip_crc":"PASS",
            "metadata":package+"=="+version,
            "wheel_tag":tag,"record_members":len(got)}

def inspect(archive,whole_sha):
    need(Path(archive).name==archive and
         re.fullmatch(r"[a-zA-Z0-9_.-]+",archive),"WHEELHOUSE_BASENAME_ONLY")
    need(archive==WHEEL_NAME and whole_sha.upper()==WHEEL_SHA,
         "ORIGINAL_WHEELHOUSE_ID_DRIFT")
    original=ROOT/"Wheelhouse"/archive
    need(original.is_file() and not original.is_symlink(),"ORIGINAL_WHEELHOUSE_NOT_AVAILABLE")
    blob=original.read_bytes()
    need(sha(blob)==whole_sha.upper(),"ORIGINAL_WHEELHOUSE_SHA")
    need(len(blob)==4_076_132,"ORIGINAL_WHEELHOUSE_BYTE_LENGTH")
    originalEvidence=ROOT/"Evidence"/ORIGINAL_EVIDENCE_NAME
    need(originalEvidence.is_file() and not originalEvidence.is_symlink() and
         sha(originalEvidence.read_bytes())==ORIGINAL_EVIDENCE_SHA,
         "ORIGINAL_WIN_EVIDENCE_CUSTODY_MISMATCH")
    selection=json.loads(SOURCE_SELECTION.read_bytes())
    need(selection["owner_windows_inventory"]["target_wheel_abi"]=="cp314" and
         selection["owner_windows_inventory"]["gil_disabled"] is False,
         "ORIGINAL_HOST_ABI_MISMATCH")
    expected={w["filename"] for w in selection["selected_wheels"]}
    need(len(expected)==3,"EXPECTED_THREE_WHEELS")
    outer=zip_data(blob,exact=expected|{"SHA256SUMS.txt"},allowed_max=4)
    with zipfile.ZipFile(io.BytesIO(blob)) as zipx:
        need(all(m.compress_type==zipfile.ZIP_STORED for m in zipx.infolist()),
             "ARCHIVE_SHOULD_KEEP_ORIGINAL_WHEEL_BYTES")
    index=outer["SHA256SUMS.txt"].decode("ascii").splitlines()
    expected_index={w["sha256"]+"  "+w["filename"]
                    for w in selection["selected_wheels"]}
    need(len(index)==3 and set(index)==expected_index and
         len(set(index))==3,"ORIGINAL_WHEELHOUSE_INDEX")
    wanted_tags={
        "cryptography":"cp311-abi3-win_amd64",
        "cffi":"cp314-cp314-win_amd64",
        "pycparser":"py3-none-any",
    }
    results=[]
    for expected_wheel in selection["selected_wheels"]:
        filename=expected_wheel["filename"]
        wheelbytes=outer[filename]
        need(sha(wheelbytes)==expected_wheel["sha256"],
             "WHEEL_SHA_MISMATCH_"+expected_wheel["name"])
        results.append(verify_inner_wheel(
            wheelbytes,expected_wheel["name"],expected_wheel["version"],
            filename,wanted_tags[expected_wheel["name"]]))
    return {"result":"ORIGINAL_WHEELHOUSE_BYTES_INDEPENDENT_FULL_PASS",
            "original_wheelhouse_sha256":sha(blob),
            "original_wheelhouse_bytes":len(blob),
            "original_evidence_sha256":ORIGINAL_EVIDENCE_SHA,
            "outer_members":"4/4","outer_crc":"4/4",
            "index":"3/3","wheels":results,
            "installation":"NOT_PERFORMED",
            "controller_import":"NOT_TESTED",
            "gate4":"NOT_AUTHORIZED"}

if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("--archive",required=True)
    parser.add_argument("--sha256",required=True)
    opts=parser.parse_args()
    print(json.dumps(inspect(opts.archive,opts.sha256),sort_keys=True))
