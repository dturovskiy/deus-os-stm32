#!/usr/bin/env python3
"""OFFLINE source-actual syntax and Node maintenance exclusion tests; NO PORTS."""
from __future__ import annotations
import hashlib
import subprocess
import tempfile
from pathlib import Path

OS=Path(__file__).resolve().parents[3]
INC=OS/"include"
CORE=OS/"src/kernel/esp_bridge_core_v0.c"
NODE=OS/"src/kernel/node_transport.c"
WIFI_CLIENT=OS/"src/kernel/node_wifi_client_v0.c"
WIFI_EXTENSION=OS/"nodes/esp8266/firmware/wifi_lab/wifi_node_extension_v0.c"
USB=OS/"src/kernel/usb_management.c"
FRAME=OS/"src/kernel/binary_frame.c"
DNL=OS/"src/drivers/deus_node_link.c"
TEST=OS/"nodes/esp8266/bridge/test_node_maintenance_source_offline.c"
SOURCES=[CORE,NODE,USB,FRAME,DNL,TEST,OS/"scripts/build_firmware.ps1"]
for p in SOURCES:
    assert p.exists(), f"SOURCE_NOT_FOUND {p}"
    print("SOURCE_SHA256",p.relative_to(OS),hashlib.sha256(p.read_bytes()).hexdigest().upper(),flush=True)

for path in [CORE,NODE,USB,FRAME]:
    p=subprocess.run(["gcc","-std=c11","-Wall","-Wextra","-Werror",
        "-fsyntax-only","-I",str(INC),
        "-I",str(OS/"nodes/esp8266/firmware/wifi_lab"),str(path)],capture_output=True,text=True,timeout=20)
    print(f"SYNTAX_{path.name}={p.returncode}",flush=True)
    if p.stderr:print(p.stderr[-2800:],flush=True)
    assert p.returncode==0, "SYNTAX_ERROR:"+path.name

with tempfile.TemporaryDirectory(prefix="deus_node_mode_source_") as td:
    for label,flags in [("O0",["-O0"]),("OS",["-Os"]),
                        ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"])]:
        target=Path(td)/label
        cmd=["gcc","-std=c11","-Wall","-Wextra","-Werror",*flags,
             "-I",str(INC),"-I",str(OS/"nodes/esp8266/firmware/wifi_lab"),
             str(NODE),str(WIFI_CLIENT),str(WIFI_EXTENSION),str(DNL),str(TEST),"-o",str(target)]
        b=subprocess.run(cmd,capture_output=True,text=True,timeout=20)
        print(f"NODE_BUILD_{label}={b.returncode}",flush=True)
        if b.stderr: print(b.stderr[-2800:],flush=True)
        assert b.returncode==0, "NODE_BUILD_FAIL:"+label
        t=subprocess.run([str(target)],capture_output=True,text=True,timeout=10)
        print(f"NODE_TEST_{label}={t.returncode}",flush=True)
        if t.stdout:print(t.stdout[-1500:],flush=True)
        if t.stderr:print(t.stderr[-2800:],flush=True)
        assert t.returncode==0 and "NODE_MAINTENANCE_SOURCE_EXCLUSIVE_OFFLINE=PASS" in t.stdout, "NODE_TEST_FAIL:"+label

s=USB.read_text()
admission=s.split("static int usb_bridge_can_enter(void *context)",1)[1].split("static int usb_bridge_enter",1)[0]
assert "#if DEUS_ESP_SPLIT_BOOT_ADAPTER" in admission, "SOURCE_LOCKED_SPLIT_PROFILE_MISSING"
assert "return firmware_update_reset_pending == 0u;" in admission, "SPLIT_UPDATE_EXCLUSION_MISSING"
assert "esp_stm32_owner_v1_can_enter()" in admission, "STM32_OWNED_BOARD_QUALIFICATION_MISSING"
assert "BINARY_FRAME_TYPE_ESP_BRIDGE_REQUEST" in s
assert "esp_bridge_core_v0_abort" in s
assert "usb_management_esp_bridge.active" in s
assert "BINARY_FRAME_TYPE_FIRMWARE_UPDATE_REQUEST" in s
assert "BINARY_FRAME_TYPE_ASSET_TRANSFER_REQUEST" in s
assert "BINARY_FRAME_TYPE_RPC_REQUEST" in s
print("USB_DISPATCH_HARDWARE_DENY_SOURCE_INVARIANTS=PASS",flush=True)
print("SOURCE_INTEGRATION_OFFLINE=PASS SYNTAX_4/4 NODE_MODE_3/3 HARDWARE=NO",flush=True)
