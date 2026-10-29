#!/usr/bin/env python3
"""Independently attest genuine Windows RA-03 v3 OFFLINE Evidence (read-only)."""
from pathlib import Path
import hashlib
import json
import re
import sys
import zipfile

PKG="stm32_os_gate4_a05_ra03_nonce_ledger_native_offline_v3_20261028"
ZIP_SHA="40F5751568789955DD5CBE33F558F107EA8A9C823C20E36DC7FF572A77AD806A"
EVIDENCE_NAME=PKG+"_20261028_193409_3612d1a821494539bb92d6dd60dff1c0.evidence.zip"
EVIDENCE_SHA="80F3521AF3B738F2EBA58AD8460B05709609CEE218459147C5E5965366D0A9D5"
MEMBERS={"hashes.sha256","sizes.txt","outcome.txt","run.log","native_checks.txt","native_test_output.txt"}
PINS={"ra03_module_sha256":"D67EB9820C016902FCDBC7FD3A13A51AE3F1317C33927E60B49D6B5565C351BC",
      "ra03_tests_sha256":"049E81F4078AA53DC4C7405A0A5253108D8C3C63DD9FF33746577FAA1A084965",
      "ra03_child_sha256":"DF9B7AFA3FDDA926EA669A7FE40EB52E24F9F0F1259E0A56331259E7126F9719"}
MODULES={"ra03_module_sha256":"nonce_ledger_offline.psm1",
         "ra03_tests_sha256":"test_nonce_ledger_offline.ps1",
         "ra03_child_sha256":"nonce_ledger_child_offline.ps1"}
def require(b,msg):
    if not b: raise RuntimeError(msg)
def digest(b): return hashlib.sha256(b).hexdigest().upper()

def main():
    root=Path.cwd().resolve()
    require(root.name=="OS" and root.parent.name=="deus-os-stm32","CWD_IDENTITY")
    ev=root.parent/"Evidence"/EVIDENCE_NAME
    bundle=root.parent/"Bundle"/(PKG+".zip")
    require(ev.is_file() and not ev.is_symlink(),"ORIGINAL_EVIDENCE_ABSENT_OR_LINK")
    raw=ev.read_bytes()
    require(len(raw)==2292 and digest(raw)==EVIDENCE_SHA,"EVIDENCE_SHA_SIZE_MISMATCH")
    require(bundle.is_file() and digest(bundle.read_bytes())==ZIP_SHA,"OPERATOR_ZIP_SHA_MISMATCH")
    with zipfile.ZipFile(ev) as z:
        files=z.infolist()
        names=[x.filename for x in files]
        require(len(names)==6 and len(set(names))==6 and set(names)==MEMBERS,"UNEXPECTED_EVIDENCE_MEMBERS")
        require(z.testzip() is None,"ZIP_CRC_FAILED")
        contents={n:z.read(n) for n in names}
        for entry in files:
            require(entry.file_size==len(contents[entry.filename]),"ZIP_DECLARED_SIZE")
            require(entry.file_size<=100000 and not entry.is_dir(),"ZIP_MEMBER_BOUNDS")
    sha_rows=contents["hashes.sha256"].decode("ascii").splitlines()
    size_rows=contents["sizes.txt"].decode("ascii").splitlines()
    require(len(sha_rows)==5 and len(size_rows)==4,"INDEX_ROW_COUNTS")
    found_sha=set()
    for row in sha_rows:
        m=re.fullmatch(r"([a-fA-F0-9]{64})  ([a-z0-9_.-]+)",row)
        require(m and m[2] in MEMBERS-{"hashes.sha256"} and m[2] not in found_sha, "INVALID_SHA_ROW")
        found_sha.add(m[2])
        require(digest(contents[m[2]])==m[1].upper(),"INDEX_SHA_FAILURE")
    require(found_sha==MEMBERS-{"hashes.sha256"},"MISSING_SHA_INDEX_ENTRY")
    found_size=set()
    for row in size_rows:
        m=re.fullmatch(r"([0-9]+)  ([a-z0-9_.-]+)",row)
        require(m and m[2] in MEMBERS-{"hashes.sha256","sizes.txt"} and m[2] not in found_size,"INVALID_SIZE_ROW")
        found_size.add(m[2])
        require(len(contents[m[2]])==int(m[1]),"INDEX_SIZE_FAILURE")
    require(found_size==MEMBERS-{"hashes.sha256","sizes.txt"},"MISSING_SIZE_INDEX_ENTRY")
    receipt=json.loads(contents["outcome.txt"].decode("utf-8"))
    for key,val in [("schema","DEUS_OS_G4_RA03_NATIVE_NONCE_OFFLINE_EVIDENCE_V3"),
                    ("gate",PKG),("result","PASS"),
                    ("classification","G4_RA03_WINDOWS_NATIVE_NONCE_LEDGER_SCOPED_PASS"),
                    ("failure_phase","NONE"),("error","NONE"),("child_exit_code","0"),
                    ("child_diagnostics_in_evidence",True),
                    ("nonce_ledger_installed","NO"),("controller_installed","NO"),
                    ("physical_release","DENIED"),("gate4","NOT_AUTHORIZED"),
                    ("device_io","NONE"),("usb","NO"),("ssh","NO"),("reset","NO"),("flash","NO")]:
        require(receipt.get(key)==val,"WRONG_RECEIPT_"+key+":"+repr(receipt.get(key)))
    require(receipt.get("source_checks")==13,"SOURCE_CHECK_COUNT")
    before=receipt.get("git_pre_sha256")
    after=receipt.get("git_post_sha256")
    require(isinstance(before,str) and re.fullmatch(r"[0-9A-F]{64}",before) and after==before,"GIT_POSTSTATE_NOT_MATCH")
    for key,sha in PINS.items():
        require(receipt.get(key)==sha,"RECEIPT_PIN_"+key)
        src=root/"scripts"/"gate4_entry"/MODULES[key]
        require(digest(src.read_bytes())==sha,"LIVE_SOURCE_DRIFT_"+key)
    checks=contents["native_checks.txt"].decode("utf-8").splitlines()
    require(len(checks)==13 and len(set(checks))==13 and all(x.startswith("PASS ") for x in checks),"NATIVE_CHECKS_NOT_13_UNIQUE_PASS")
    for term in ("RA03_NATIVE_TEST_EXIT0","RA03_NATIVE_24_EXACT_CASE_MARKERS","RA03_NATIVE_NO_DEVICE_NO_RELEASE","GIT_PRE_POST_IDENTICAL"):
        require("PASS "+term in checks,"CHECK_MISSING_"+term)
    transcript=contents["native_test_output.txt"].decode("utf-8")
    lines=transcript.splitlines()
    require(lines[0]=="RA03_CHILD_EXIT=0","CHILD_EXIT_MARKER")
    m=re.fullmatch(r"RA03_CHILD_RECORDED_LINES=(\d+)",lines[1])
    require(m,"CHILD_COUNT_MARKER")
    # Captured original child stdout should be exactly count lines, after the two metadata lines.
    require(int(m[1])==len(lines)-2,"CHILD_LINE_COUNT_DRIFT")
    payload=lines[2:]
    markers=[s[len("RA03_NATIVE_WINDOWS_CASE=PASS "):] for s in payload if s.startswith("RA03_NATIVE_WINDOWS_CASE=PASS ")]
    require(len(markers)==24 and len(set(markers))==24,"NOT_24_UNIQUE_NATIVE_CASES")
    for item in ("UNEXPECTED_EVERYONE_ACE","TWO_PROCESS_CREATE_NEW_ONE_WINNER_ONE_STOP","REPARSE_JUNCTION_REJECTED"):
        require(item in markers,"CASE_MISSING_"+item)
    require(payload[-1]=="RA03_OFFLINE_WINDOWS=PASS COUNT=24 USB=NO SSH=NO RESET=NO FLASH=NO INSTALL=NO AUTHORIZATION=DENIED","NATIVE_TERMINAL_LINE")
    require(len(payload)==25,"NATIVE_EXTRA_OUTPUT")
    log=contents["run.log"].decode("utf-8")
    require("CLASSIFICATION=G4_RA03_WINDOWS_NATIVE_NONCE_LEDGER_SCOPED_PASS" in log and "CHILD_EXIT=0" in log,"LOG_MISMATCH")
    print(json.dumps({"result":"G4_RA03_V3_ORIGINAL_WINDOWS_EVIDENCE_ACCEPTED_SCOPED",
      "evidence_bytes":len(raw),"original_sha256":EVIDENCE_SHA,"operator_sha256":ZIP_SHA,
      "crc":"6/6 PASS","sha_index":"5/5 PASS","size_index":"4/4 PASS",
      "preflight":"13/13 PASS","native_distinct_cases":"24/24 PASS",
      "child_exit":receipt["child_exit_code"],"git_pre_post":"MATCH",
      "release":"DENIED","gate4":"NOT_AUTHORIZED"},sort_keys=True))
    print("NATIVE_CASES="+", ".join(markers))
if __name__=="__main__":
    try: main()
    except Exception as exc:
        print("G4_RA03_V3_ORIGINAL_EVIDENCE_AUDIT_FAIL="+str(exc))
        sys.exit(2)
