#!/usr/bin/env python3
"""Fast, read-only developer bench check of EXISTING signed firmware package(s).

This is not a new Gate, release workflow, signer, updater or USB tool.
No global environment, Python wheels, keys, nonce, SSH, reset, SWD or Flash.
Only inspect already-accepted package bytes, headers, image digest and vectors.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import stat
import struct

STORE = Path(__file__).resolve().parents[3] / "ProtectedRecovery"
APP_ORIGIN = 0x08002000
APP_END = 0x0800F000
SRAM_START = 0x20000000
SRAM_END = 0x20005000
PACKAGES = {
    "normal": {
        "name": "deus_os_000e_management_only_tree081aac70_signed_v6.pkg",
        "length": 49072,
        "sha256": "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED",
        "version": 6,
        "image_length": 48992,
        "image_sha256": "1F6966788923496170F47FED9AC24311BEA177B53B3045C2B89CA6305E1B56E1",
    },
    "rescue": {
        "name": "deus_os_rev5_000c_recovery_future_floor7_v1.pkg",
        "length": 52092,
        "sha256": "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A",
        "version": 7,
        "image_length": 52012,
        "image_sha256": "AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210",
    }
}
def sha(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest().upper()
def require(ok: bool, message: str):
    if not ok:
        raise ValueError(message)
def check_blob(raw: bytes, expected: dict) -> dict:
    require(len(raw) == expected["length"], "WHOLE_PACKAGE_LENGTH")
    require(sha(raw) == expected["sha256"], "WHOLE_PACKAGE_SHA256")
    require(len(raw) >= 88, "PACKAGE_TRUNCATED")
    product, fmt, reserved, target, length, version = struct.unpack_from("<IBBHII", raw, 0)
    require((product,fmt,reserved,target)==(0x534F4544,1,0,0x0410), "DEVICE_HEADER")
    require(length==expected["image_length"] and length%4==0 and
            8<=length<=53248 and len(raw)==80+length, "IMAGE_SIZE")
    require(version==expected["version"], "FIRMWARE_VERSION")
    image=raw[80:]
    require(sha(image)==expected["image_sha256"] and
            raw[16:48].hex().upper()==expected["image_sha256"], "PAYLOAD_SHA256")
    sp,pc=struct.unpack_from("<II",image)
    require(SRAM_START<=sp<=SRAM_END and sp%8==0, "INITIAL_MSP_RANGE_ALIGNMENT")
    require(pc&1==1 and APP_ORIGIN<=pc-1<APP_ORIGIN+length and
            (pc-1)<APP_END, "RESET_HANDLER_THUMB_RANGE")
    return {"package":expected["name"],"version":version,
            "package_bytes":len(raw),"package_sha256":sha(raw),
            "application_bytes":length,"application_sha256":sha(image),
            "application_origin":f"0x{APP_ORIGIN:08X}",
            "initial_msp":f"0x{sp:08X}","reset_handler":f"0x{pc:08X}",
            "format_header":"PASS","vector_structure":"PASS",
            "cryptographic_hmac": "ACCEPTED_PRIOR_NOT_REVERIFIED",
            "device": "NOT_TOUCHED"}
def check_file(path: Path, expected: dict) -> dict:
    require(not path.is_symlink(),"PACKAGE_SYMLINK")
    st=path.stat()
    require(stat.S_ISREG(st.st_mode) and st.st_size==expected["length"],"PACKAGE_FILE_TYPE_SIZE")
    return check_blob(path.read_bytes(),expected)
def main():
    p=argparse.ArgumentParser(description="Read-only bench candidate sanity check; no device IO")
    p.add_argument("--package",choices=["normal","rescue","both"],default="both")
    args=p.parse_args()
    try:
        items=["normal","rescue"] if args.package=="both" else [args.package]
        reports=[check_file(STORE/PACKAGES[item]["name"],PACKAGES[item]) for item in items]
    except (OSError,ValueError) as exc:
        print("BENCH_CANDIDATE=FAIL "+str(exc)[:100])
        return 2
    print("BENCH_CANDIDATE=PASS COUNT="+str(len(reports))+
          " READONLY=YES FLASH=NO RESET=NO")
    for r in reports:
        print(json.dumps(r,sort_keys=True,separators=(",",":")))
    print("NEXT=HARDWARE_IDENTITY_AND_UPDATE_PATH_ON_OWNER_BENCH")
    return 0
if __name__=="__main__":
    raise SystemExit(main())
