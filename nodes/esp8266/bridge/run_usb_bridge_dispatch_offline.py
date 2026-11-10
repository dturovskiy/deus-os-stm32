#!/usr/bin/env python3
"""Compile and run ACTUAL bridge USB dispatcher with no device: GCC O0/Os/UBSan.

The original src/kernel/usb_management.c has ARM-only hardware-reset asm.
A temporary copy substitutes *only* exactly two assembly mnemonics with
empty memory barriers so host GCC can assemble it. Rest of source identical.
All peripherals are in-memory callback mocks; no physical buses opened.
"""
from __future__ import annotations
import hashlib
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[3]
USB=ROOT/"src/kernel/usb_management.c"
FRAME=ROOT/"src/kernel/binary_frame.c"
CORE=ROOT/"src/kernel/esp_bridge_core_v0.c"
TEST=ROOT/"nodes/esp8266/bridge/test_usb_bridge_dispatch_offline.c"
for p in (USB,FRAME,CORE,TEST):
    print("SOURCE_SHA256",str(p.relative_to(ROOT)),hashlib.sha256(p.read_bytes()).hexdigest().upper(),flush=True)

raw=USB.read_text()
assert raw.count('__asm volatile("cpsid i" ::: "memory");')==1
assert raw.count('__asm volatile("dsb" ::: "memory");')==3
assert 'return 0;' in raw.split("static int usb_bridge_can_enter(void *context)",1)[1].split("static int usb_bridge_enter",1)[0]
safe=(raw.replace('__asm volatile("cpsid i" ::: "memory");','__asm volatile("" ::: "memory");')
         .replace('__asm volatile("dsb" ::: "memory");','__asm volatile("" ::: "memory");'))
assert safe !=raw and 'cpsid i' not in safe and '__asm volatile("dsb"' not in safe
print("ARM_ONLY_ASM_TEMPORARY_HOST_NEUTRALIZATION=4_INSTRUCTIONS",flush=True)
with tempfile.TemporaryDirectory(prefix="esp_bridge_usb_mock_") as directory:
    sanitized=Path(directory)/"usb_management_syntax_host_only.c"
    sanitized.write_text(safe)
    for key,flags in [
        ("O0",["-O0"]),
        ("OS",["-Os"]),
        ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"])
    ]:
        executable=Path(directory)/("usb_mock_"+key)
        cmd=["gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",*flags,
             "-I",str(ROOT/"include"),str(sanitized),str(FRAME),str(CORE),str(TEST),
             "-o",str(executable)]
        comp=subprocess.run(cmd,capture_output=True,text=True,timeout=20)
        print("HOST_USB_DISPATCH_BUILD_"+key+"="+str(comp.returncode),flush=True)
        if comp.stderr: print(comp.stderr[-4000:],flush=True)
        assert comp.returncode==0,"BUILD_FAILED:"+key
        run=subprocess.run([str(executable)],capture_output=True,text=True,timeout=12)
        print("HOST_USB_DISPATCH_RUN_"+key+"="+str(run.returncode),flush=True)
        if run.stdout:print(run.stdout[-1700:],flush=True)
        if run.stderr:print(run.stderr[-2800:],flush=True)
        assert run.returncode==0 and "USB_BRIDGE_REAL_DISPATCH_OFFLINE=PASS" in run.stdout,"TEST_FAILED:"+key
print("ACTUAL_USB_MANAGEMENT_MOCK_DISPATCH=3/3 PASS NO_DEVICE_ACCESS=YES",flush=True)
