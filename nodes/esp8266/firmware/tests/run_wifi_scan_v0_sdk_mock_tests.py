#!/usr/bin/env python3
"""Compile ACTUAL source-only ESP Wi-Fi scan service against SDK v3.4 stubs.
NO Xtensa native claim, no UART/USB/ESP/RF/wiring/Flash interactions.
"""
from pathlib import Path
from hashlib import sha256
from tempfile import TemporaryDirectory
import subprocess
root=Path(__file__).resolve().parents[4]
src=root/"nodes/esp8266/firmware/wifi_lab/wifi_scan_v0.c"
header=root/"nodes/esp8266/firmware/wifi_lab/wifi_scan_v0.h"
test=root/"nodes/esp8266/firmware/tests/wifi_scan_v0_sdk_mock_tests.c"
sdk=root/"nodes/esp8266/firmware/tests/sdk_mock_wifi"
for p in [src,header,test,*sdk.glob("*.h")]:
    print("SOURCE_SHA256",str(p.relative_to(root)),sha256(p.read_bytes()).hexdigest().upper(),flush=True)
with TemporaryDirectory(prefix="deus_esp_wifi_v0_") as temp:
    for name,flags in (
        ("O0",["-O0"]),("Os",["-Os"]),
        ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"]),
    ):
        prog=Path(temp)/name
        cp=subprocess.run(["gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",
                           *flags,"-I",str(sdk),"-I",str(src.parent),
                           str(src),str(test),"-o",str(prog)],
                          capture_output=True,text=True,timeout=20)
        print("ESP_WIFI_BUILD_"+name+"="+str(cp.returncode),flush=True)
        if cp.stderr:print(cp.stderr[-4000:],flush=True)
        assert cp.returncode==0
        run=subprocess.run([str(prog)],capture_output=True,text=True,timeout=15)
        print("ESP_WIFI_TEST_"+name+"="+str(run.returncode),flush=True)
        print(run.stdout[-2000:],flush=True)
        if run.stderr:print(run.stderr[-4000:],flush=True)
        assert run.returncode==0 and "ESP_WIFI_SCAN_SDK_V34_API_MOCK=PASS" in run.stdout
print("ESP_WIFI_SCAN_SOURCE_ONLY=3/3 PASS NO_RADIO_NO_HARDWARE",flush=True)
