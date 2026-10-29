#!/usr/bin/env python3
"""Independently verify the original Windows-native Gate3 G3-4b evidence.

Read-only, no target/network I/O or Git mutation. Validates original bytes,
ZIP CRC/index/sizes/strict outcome semantics, and binds the canonical operator.
Not a source or physical-hardware Gate3 acceptance grant.
"""
from __future__ import annotations

import hashlib
import json
import re
import sys
import zipfile
from pathlib import Path

OS = Path(__file__).resolve().parents[2]
PARENT = OS.parent
BUNDLE_NAME = "stm32_os_gate3_g34b_native_readiness_evidence_v1_20261028.zip"
BUNDLE_SHA = "4D7ECF88284B1EA283E40C5D7A94F81B46FF26A01265FF09C1A2F17864BB7DDB"
APPLY_SHA = "D0B231935F3CD9D8E09688B080340A2942EA4311A985B9105C9360C6AF946997"
EVIDENCE_NAME = "stm32_os_gate3_g34b_native_readiness_evidence_v1_20261028_20261028_073503_231d274c368a49f4a97846d5035962d7.evidence.zip"
EVIDENCE_SHA = "038A9D74847B53DC67D93AB72AF494DC373F76CDD167A37151F8AB393F27BBDE"
V6 = "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
V7 = "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A"
NAMES = {"outcome.txt","run.log","native_checks.txt","sizes.txt","hashes.sha256"}
EXPECTED_PACKAGE = "stm32_os_gate3_g34b_native_readiness_evidence_v1_20261028"

class Reject(Exception): pass
def require(ok, code):
    if not ok: raise Reject(code)
def sha(blob):
    return hashlib.sha256(blob).hexdigest().upper()
def exact_read(path, expected_sha, expected_size):
    require(path.is_file() and not path.is_symlink(), "MISSING_OR_LINK:" + path.name)
    b = path.read_bytes()
    require(len(b) == expected_size, "BYTE_SIZE_DRIFT:" + path.name)
    require(sha(b) == expected_sha, "SHA256_DRIFT:" + path.name)
    return b
def text_data(blobs, name):
    try:
        raw = blobs[name]
        require(not raw.startswith(b"\xef\xbb\xbf"), "BOM_FORBIDDEN:" + name)
        return raw.decode("utf-8","strict")
    except UnicodeError as e: raise Reject("INVALID_UTF8:" + name) from e
def unique_pairs(items):
    d = {}
    for key, value in items:
        require(key not in d,"DUPLICATE_JSON_KEY:"+key)
        d[key] = value
    return d

def audit():
    checks = []
    def checked(condition, label):
        require(condition, label)
        checks.append(label)

    bundle = exact_read(PARENT/"Bundle"/BUNDLE_NAME,BUNDLE_SHA,6045)
    with zipfile.ZipFile(PARENT/"Bundle"/BUNDLE_NAME) as z:
        checked(z.testzip() is None,"BUNDLE_ALL_CRC")
        checked(len(z.infolist())==4,"BUNDLE_COUNT4")
        names = [i.filename for i in z.infolist()]
        checked(len(set(names))==4 and set(names)==
                {"manifest.json","hashes.sha256","apply.ps1","PACKAGE_README.txt"},"BUNDLE_MEMBERS")
        manifest=json.loads(z.read("manifest.json"),object_pairs_hook=unique_pairs)
        checked(manifest.get("package_id")==EXPECTED_PACKAGE and
                manifest.get("execution_profile")=="OFFLINE" and
                manifest.get("runner_result_contract")=="DEUS_OS_RUNNER_RESULT_V1",
                "BUNDLE_SOURCE_PROFILE")
        checked(sha(z.read("apply.ps1"))==APPLY_SHA==manifest.get("apply_sha256"),"APPLY_EXACT_SHA")
        checked(b"EnterBootloaderAsync" not in z.read("apply.ps1") and
                b"scp " not in z.read("apply.ps1").lower(), "BUNDLE_NO_PHYSICAL_COMMAND")

    original = exact_read(PARENT/"Evidence"/EVIDENCE_NAME,EVIDENCE_SHA,1875)
    with zipfile.ZipFile(PARENT/"Evidence"/EVIDENCE_NAME) as z:
        checked(len(z.infolist()) == 5,"ORIGINAL_MEMBER_COUNT5")
        entries = z.infolist()
        names=[i.filename for i in entries]
        checked(set(names)==NAMES and len(set(names))==5 and
                len(set(x.casefold() for x in names))==5,"ORIGINAL_MEMBER_SET_UNIQUE")
        checked(all(not n.startswith("/") and ".." not in n for n in names),"ORIGINAL_UNSAFE_NAMES_NONE")
        checked(z.testzip() is None,"ORIGINAL_ALL_CRC_PASS")
        blobs={n:z.read(n) for n in names}
        checked(all(0<len(blob)<80000 for blob in blobs.values()),"ORIGINAL_MEMBERS_BOUNDED")
        index=text_data(blobs,"hashes.sha256").splitlines()
        checked(len(index)==4,"ORIGINAL_INDEX_FOUR_LINES")
        hashes={}
        for row in index:
            m=re.fullmatch(r"([A-F0-9]{64})  ([A-Za-z0-9_.-]+)",row)
            require(m is not None,"INDEX_LINE_MALFORMED")
            h,n=m.groups()
            require(n not in hashes,"INDEX_DUPLICATE")
            hashes[n]=h
        checked(set(hashes)==NAMES-{"hashes.sha256"},"INDEX_EXACT_SET")
        for n in sorted(hashes):
            checked(hashes[n] == sha(blobs[n]),"INDEX_SHA_"+n)
        sizes=text_data(blobs,"sizes.txt").splitlines()
        checked(len(sizes)==3,"ORIGINAL_SIZES_THREE_LINES")
        records={}
        for row in sizes:
            m=re.fullmatch(r"([0-9]+)  ([A-Za-z0-9_.-]+)",row)
            require(m is not None,"SIZE_LINE_MALFORMED")
            size,n=m.groups()
            require(n not in records,"SIZE_DUPLICATE")
            records[n]=int(size)
        checked(set(records)==NAMES-{"sizes.txt","hashes.sha256"},"SIZES_EXACT_SET")
        for n in sorted(records):
            checked(records[n] == len(blobs[n]),"SIZE_MATCH_"+n)
    outcome=json.loads(text_data(blobs,"outcome.txt"),object_pairs_hook=unique_pairs,
                       parse_constant=lambda _: (_ for _ in ()).throw(Reject("NONFINITE_JSON")))
    checked(outcome.get("schema")=="DEUS_OS_G3_4B_NATIVE_OFFLINE_EVIDENCE_V1","OUTCOME_SCHEMA")
    checked(outcome.get("gate")==EXPECTED_PACKAGE,"OUTCOME_PACKAGE_ID")
    checked(outcome.get("result")=="PASS","ORIGINAL_RESULT_PASS")
    checked(outcome.get("classification")=="G34B_WINDOWS_NATIVE_OFFLINE_RECOVERY_ROUTE_SCOPED_PASS",
            "ORIGINAL_CLASSIFICATION_SCOPED")
    checked(outcome.get("failure_phase")=="NONE" and outcome.get("error")=="NONE",
            "ORIGINAL_NO_FAIL")
    checked(type(outcome.get("source_checks"))==int and outcome["source_checks"]==24,
            "CHECK_COUNT_24")
    checked(outcome.get("git_pre_sha256")==outcome.get("git_post_sha256") and
            bool(re.fullmatch("[A-F0-9]{64}",outcome.get("git_pre_sha256") or "")),
            "GIT_POST_EQUAL_PRE")
    checked(outcome.get("signed_v6_sha256")==V6 and outcome.get("rescue_v7_sha256")==V7,
            "SIGNED_PAIR_MATCH")
    checked(outcome.get("full_gate3")=="NOT_ACCEPTED" and outcome.get("gate4")=="NOT_AUTHORIZED",
            "GATE_BOUNDARY_NOT_PROMOTED")
    checked(all(outcome.get(k)==v for k,v in
                {"device_io":"NONE","usb":"NO","ssh":"NO","reset":"NO","flash":"NO",
                 "future_000e_installed":"NOT_ATTESTED","actual_000d_info":"NOT_ATTESTED"}.items()),
            "NO_PHYSICAL_ATTESTATION")
    native_checks=text_data(blobs,"native_checks.txt").splitlines()
    checked(len(native_checks)==24 and len(set(native_checks))==24 and
            all(x.startswith("PASS ") for x in native_checks),"CHECKS_24_PASS_UNIQUE")
    for token in ["PASS VALID_V6_ROUTE_SOURCE_ONLY","PASS NO_VALID_APP_ROUTE_SOURCE_ONLY",
                  "PASS V6_1052_CHUNKS_48_PAGES","PASS V7_1117_CHUNKS_51_PAGES",
                  "PASS GIT_PRE_POST_IDENTICAL"]:
        checked(token in native_checks,"REQUIRED_CHECK_MISSING_"+token.split(" ")[1])
    checked("USB=NO SSH=NO RESET=NO FLASH=NO" in text_data(blobs,"run.log"),"RAW_LOG_NO_DEVICE")
    checked("CLASSIFICATION=G34B_WINDOWS_NATIVE_OFFLINE_RECOVERY_ROUTE_SCOPED_PASS"
            in text_data(blobs,"run.log"),"LOG_CLASSIFICATION_MATCH")
    return checks,outcome,native_checks

try:
    result,report,check_names=audit()
    for c in result: print("G34B_ORIGINAL_EVIDENCE_CASE=PASS CHECK="+c)
    print("G34B_ORIGINAL_EVIDENCE=PASS CHECKS="+str(len(result))+
          " NATIVE_CHECKS="+str(report['source_checks'])+
          " BUNDLE_SHA="+BUNDLE_SHA+
          " EVIDENCE_SHA="+EVIDENCE_SHA+
          " PHYSICAL_IO=NO FULL_GATE3=NOT_ACCEPTED")
except (Reject,KeyError,ValueError,TypeError,zipfile.BadZipFile,OSError) as e:
    print("G34B_ORIGINAL_EVIDENCE=FAIL "+str(e))
    sys.exit(2)
