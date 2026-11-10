#!/usr/bin/env python3
"""OFFLINE x86 validation of proposed targetable ESP Bridge v0 C-only core."""
from __future__ import annotations
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[3]
PATH=ROOT/"nodes/esp8266/bridge"
FILES=[ROOT/"src/kernel/esp_bridge_core_v0.c",ROOT/"include/kernel/esp_bridge_core_v0.h",PATH/"test_esp_bridge_core_v0.c"]
def main():
    assert shutil.which("gcc"),"GCC_NOT_FOUND"
    for f in FILES:
        assert f.is_file(),"SOURCE_MISSING:"+str(f)
        print("SOURCE_SHA256 "+str(f.relative_to(ROOT))+" "+hashlib.sha256(f.read_bytes()).hexdigest().upper(),flush=True)
    with tempfile.TemporaryDirectory(prefix="deus_esp_bridge_v0_offline_") as tmp:
        for name, flags in [
            ("O0",["-O0"]),
            ("OS",["-Os"]),
            ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"])
        ]:
            binary=Path(tmp)/name
            cmd=["gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",
                 *flags,"-I",str(ROOT/"include"),"-I",str(PATH),str(FILES[0]),str(FILES[2]),"-o",str(binary)]
            compile=subprocess.run(cmd,capture_output=True,text=True,timeout=25)
            print(f"BUILD_{name}={compile.returncode}",flush=True)
            if compile.stderr:print(compile.stderr[-3000:],flush=True)
            assert compile.returncode==0,"BUILD_FAILED:"+name
            check=subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
            print(f"TEST_{name}={check.returncode}",flush=True)
            print(check.stdout[-1200:],flush=True)
            if check.stderr:print(check.stderr[-3000:],flush=True)
            assert check.returncode==0 and "ESP_BRIDGE_CORE_V0_OFFLINE=PASS" in check.stdout,"OFFLINE_TEST_FAILED:"+name
    print("ESP_BRIDGE_CORE_V0_OFFLINE_MATRIX=3/3 PASS HARDWARE=NOT_TOUCHED",flush=True)

if __name__=="__main__":
    try:main()
    except Exception as e:
        print("ESP_BRIDGE_CORE_V0_OFFLINE_MATRIX=FAIL "+str(e),flush=True)
        sys.exit(2)
