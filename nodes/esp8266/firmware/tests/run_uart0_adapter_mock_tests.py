#!/usr/bin/env python3
"""Offline SDK-mock build of production ESP8266 UART0 adapter, no USB/device."""
from __future__ import annotations
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

OS=Path(__file__).resolve().parents[4]
FIRM=OS/"nodes/esp8266/firmware"
FILES=[
 FIRM/"tests/uart0_adapter_mock_tests.c",
 FIRM/"main/uart0_adapter.c",
 FIRM/"main/node_runtime.c",
 OS/"shared/node_link/src/deus_node_link.c"
]
def main():
 if not shutil.which("gcc"):raise RuntimeError("GCC_NOT_INSTALLED")
 for p in FILES:
  if not p.is_file():raise RuntimeError("SOURCE_MISSING:"+str(p))
  print("SOURCE_SHA256 "+str(p.relative_to(OS))+" "+
        hashlib.sha256(p.read_bytes()).hexdigest().upper(),flush=True)
 with tempfile.TemporaryDirectory(prefix="esp01_uart0_adapter_mock_") as td:
  for label,flags in [
   ("O0",["-O0"]),
   ("O2",["-O2"]),
   ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"])
  ]:
   out=Path(td)/("uart0_mock_"+label)
   c=[
    "gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",
    *flags,
    "-I",str(FIRM/"tests/sdk_mock"),
    "-I",str(OS/"shared/node_link/include"),
    "-I",str(FIRM/"main"),
    str(FILES[0]),str(FILES[2]),str(FILES[3]),"-o",str(out)
   ]
   compile=subprocess.run(c,capture_output=True,text=True,timeout=30)
   print("COMPILE_"+label+"="+str(compile.returncode),flush=True)
   if compile.stdout:print(compile.stdout[-2000:],flush=True)
   if compile.stderr:print(compile.stderr[-4000:],flush=True)
   if compile.returncode:raise RuntimeError("COMPILE_FAILED_"+label)
   test=subprocess.run([str(out)],capture_output=True,text=True,timeout=12)
   print("RUN_"+label+"="+str(test.returncode),flush=True)
   print(test.stdout[-5000:],flush=True)
   if test.stderr:print(test.stderr[-4000:],flush=True)
   if test.returncode or "ESP01_UART_ADAPTER_OFFLINE_RESULT=PASS CASES=5 HARDWARE=NO" not in test.stdout:
    raise RuntimeError("TEST_FAILED_"+label)
 print("ESP01_UART0_ADAPTER_V2_HOST_OFFLINE_PASS=15/15 NO_DEVICE_ACCESS=YES",flush=True)
if __name__=="__main__":
 try:main()
 except Exception as ex:
  print("ESP01_UART0_ADAPTER_V2_HOST_OFFLINE=FAIL: "+str(ex),flush=True)
  sys.exit(2)
