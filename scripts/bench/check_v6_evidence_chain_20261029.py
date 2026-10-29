#!/usr/bin/env python3
"""Offline adjudication of TWO ORIGINAL Windows receipts; never contact USB or SSH.

Do not rewrite, repackage, or retroactively turn an original FAIL into a PASS.
An independently recovered remote command transcript can establish a successful
bench software install, while the native wrapper itself remains FAIL.
"""
from pathlib import Path
import hashlib, json, re, sys, zipfile

ROOT=Path(__file__).resolve().parents[3]
V3=("stm32_os_bench_signed_v6_once_v3_20261029_20261029_195412_bc5f39397688412eb568d5da352ba8c6.evidence.zip",
    "95EC84EFF962B91C41C4C88D7DDD9A6EDE5EE8700987BEB7DF29879B39F85695")
READBACK=("stm32_os_bench_v6_receipt_adjudication_v1_20261029_20261029_201340_dbe8609d154448559c64075d7278c0ef.evidence.zip",
          "A596E9B5DC6ECFC0BEC35B771C79FB979DDBAEB830EBF2E72EA82E2975B7F943")
SIGNED_V6="2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
SOURCE_TREE="081aac70c31909bf681254393e2288917cb9b8b2"

def need(ok, why):
    if not ok: raise AssertionError(why)

def parse_index(data, size):
    rows={}
    for line in data.decode("ascii").splitlines():
        pattern=r"([0-9]+|[0-9A-Fa-f]{64})  ([A-Za-z0-9_.-]+)"
        m=re.fullmatch(pattern,line)
        need(m is not None,"INDEX_FORMAT")
        token,member=m.groups()
        need(member not in rows,"INDEX_DUPLICATE")
        rows[member]=int(token) if size else token.upper()
    return rows

def read_original(name, expected):
    p=ROOT/"Evidence"/name
    need(p.is_file() and not p.is_symlink(),"ORIGINAL_NOT_FOUND")
    raw=p.read_bytes()
    need(hashlib.sha256(raw).hexdigest().upper()==expected,"ORIGINAL_SHA_MISMATCH")
    with zipfile.ZipFile(p) as z:
        names=z.namelist()
        need(z.testzip() is None,"ORIGINAL_CRC_FAIL")
        need(len(names)==len(set(names)),"DUPLICATE_ZIP_NAMES")
        need(all(re.fullmatch(r"[A-Za-z0-9_.-]+",n) for n in names),"UNSAFE_MEMBER")
        data={x.filename:z.read(x.filename) for x in z.infolist()}
    sha=parse_index(data["hashes.sha256"],False)
    sizes=parse_index(data["sizes.txt"],True)
    need(set(sha)==set(data)-{"hashes.sha256"},"SHA_INDEX_COVERAGE")
    need(set(sizes)==set(data)-{"hashes.sha256","sizes.txt"},"SIZE_INDEX_COVERAGE")
    for member,h in sha.items():
        need(hashlib.sha256(data[member]).hexdigest().upper()==h,"INDEX_SHA_"+member)
    for member,length in sizes.items():
        need(len(data[member])==length,"INDEX_SIZE_"+member)
    return data, dict(bytes=len(raw), members=len(names), sha_index=len(sha), size_index=len(sizes), crc="PASS", sha="PASS")

def main():
    v3,v3_meta=read_original(*V3)
    follow,follow_meta=read_original(*READBACK)
    need(v3_meta["members"]==15 and follow_meta["members"]==7,"SOURCE_MEMBER_COUNT")
    orig=json.loads(v3["outcome.txt"])
    back=json.loads(follow["outcome.txt"])
    need(orig["result"]=="FAIL" and back["result"]=="FAIL","ORIGINAL_RESULTS_RETAINED")
    need(orig["child_status"]["REMOTE"]=="EXIT=0;TIMEOUT=False","HARDWARE_REMOTE_STATUS")
    need(orig["child_status"]["REMOTE_LOG_RETRIEVE"]=="EXIT=1;TIMEOUT=False","ORIGINAL_TRANSPORT_DEFECT")
    need(orig["signed_v6_sha256"]==SIGNED_V6,"ORIGINAL_FIRMWARE_ID")
    need(back["original_v3_sha256"]==V3[1],"CHAIN_SHA_ANCHOR")
    need(back["ssh_exit"]=="EXIT=0;TIMEOUT=False","ORIGINAL_READBACK_STATUS")
    need(back["ssh_operation"]=="EXACT_REMOTE_LOG_CAT_ONLY","READBACK_SCOPE")
    need(all(back[x]=="NO" for x in ("usb_open","enter","flash","reset","sudo","scp")),"NO_FOLLOWUP_DEVICE_ACTION")
    need(back["remote_log_sha256"]==hashlib.sha256(follow["remote_original_log.txt"]).hexdigest().upper(),"RECOVERED_LOG_SHA")
    need(b"usage: scp" in v3["remote_log_retrieve_stderr.txt"],"FAILED_COPY_CAUSE")
    need(b"[REMOTE_SCRIPT_LOG_NOT_AVAILABLE]" in v3["remote_stdout.txt"],"PLACEHOLDER_ON_FAILED_COPY")
    lines=follow["remote_original_log.txt"].decode("utf-8").splitlines()
    need(400 < len(follow["remote_original_log.txt"]) < 12000,"ORIGINAL_TRANSCRIPT_BOUNDS")
    need(all("password:" not in x.lower() for x in lines),"PASSWORD_LEAK_IN_LOG")
    expected_lines={
        "signed_package":"SIGNED_V6_READONLY=PASS SHA256="+SIGNED_V6,
        "old_runtime":"PRESTATE_RUNTIME=000C PORT=usb:001:8",
        "usb_permission":"USB_NODE_ADMIN=AVAILABLE_BEFORE_RESET",
        "single_enter":"ENTER=ACCEPTED ONCE=YES",
        "bootloader":"BOOTLOADER_000D=FOUND PORT=usb:001:8",
        "boot_access":"BOOTLOADER_NODE_ACCESS=PASS",
        "floor":"BOOT_ELIGIBILITY=PASS FLOOR=5 COMMITTED=5 TARGET_V6=6",
        "began_update":"UPDATE_BEGIN=YES PACKAGE_SHA256="+SIGNED_V6,
        "committed":"UPDATE_COMMITTED=Ok/Committed VERSION=6 REBOOT_AND_SMOKE_TEST=REQUIRED",
        "single_update":"SIGNED_V6_COMMITTED=PASS UPDATE_ATTEMPTS=1",
        "new_runtime":"NEW_RUNTIME_000E=FOUND PORT=usb:001:8",
        "protocol":"Protocol=1 Service=3 Registry=36",
        "platform":"OS=DEUS_OS Platform=STM32F103C8 Arch=ARMV7M",
        "source_tree":"SourceTree="+SOURCE_TREE,
        "pong":"PONG",
        "health":"HEALTH TICK=0x000004B8 PC13=0x00000001 WDOG_ACTIVE=0x00000001 WDOG_RELOAD_COUNT=0x0000000C RESET_FLAGS=0x14000000 IWDG_RESET=0x00000000",
        "software_smoke":"SOFTWARE_SMOKE=PASS RUNTIME=000E PING=PASS HEALTH=PASS",
        "no_formal_gate_claim":"PHYSICAL_OLED=NOT_ATTESTED PHYSICAL_UART=NOT_ATTESTED FORMAL_GATE4=NOT_EVALUATED",
        "remote_terminal":"RESULT=BENCH_SIGNED_V6_SOFTWARE_SMOKE_PASS ENTER_COUNT=1 UPDATE_ATTEMPTS=1",
    }
    for name, value in expected_lines.items():
        need(lines.count(value)==1,"MISSING_OR_DUPLICATE_"+name)
    need(lines.count("BOOT_INFO=PASS PORT=usb:001:8 FLOOR=5 COMMITTED=5 STATE=RecoveryIdle")==2,"BOOT_INFO_COUNT")
    need(lines.count("FLASH=NOT_STARTED")==1,"PRE_BEGIN_INFO")
    need(lines.count("ENTER=Ok/Resetting PORT=usb:001:8 STOP=CHECK_BOOTLOADER_INFO_NEXT DO_NOT_AUTORETRY=YES")==1,"ENTER_RESPONSE")
    need(all("STOP_PHASE=" not in s and "UPDATE_OUTCOME_AMBIGUOUS" not in s for s in lines),"REMOTE_STOP_MARKER")
    # This is derived adjudication of original evidence, not a third original runner artifact.
    print(json.dumps({
        "result":"PASS_OFFLINE_DERIVED_ADJUDICATION",
        "scope":"BENCH_SIGNED_V6_INSTALLED_000E_SOFTWARE_SMOKE",
        "original_v3_runner":"FAIL_PRESERVED",
        "original_readback_runner":"FAIL_PRESERVED",
        "root_cause":"HARNESS_READBACK_AND_FIRST_EXACT_LINE_VALIDATION_DEFECT",
        "v3_receipt":v3_meta,
        "readback_receipt":follow_meta,
        "firmware_sha256":SIGNED_V6,
        "remote_original_log_sha256":back["remote_log_sha256"],
        "exact_single_occurrence_markers":len(expected_lines),
        "bootloader_info_observations":2,
        "usb":"NO_ADDITIONAL_IO",
        "ssh":"NO_ADDITIONAL_IO",
        "reset":"NO_ADDITIONAL_IO",
        "flash":"NO_ADDITIONAL_IO",
        "physical_oled":"NOT_ATTESTED",
        "physical_uart":"NOT_ATTESTED",
        "formal_gate4":"NOT_EVALUATED"
    },sort_keys=True))
if __name__=="__main__":
    try: main()
    except Exception as exc:
        print(json.dumps({"result":"FAIL_OFFLINE","reason":str(exc),"type":type(exc).__name__}))
        sys.exit(1)
