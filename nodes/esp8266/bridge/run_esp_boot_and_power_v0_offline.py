#!/usr/bin/env python3
"""OFFLINE actual Deus OS source boot GPIO0/EN and INA226 current monitor tests.
NO hardware, USB, I2C, ESP or bootloader access. No ARM linker/install.
"""
from __future__ import annotations
from pathlib import Path
from hashlib import sha256
from tempfile import TemporaryDirectory
import subprocess

os=Path(__file__).resolve().parents[3]
paths=[
os/"src/kernel/esp_boot_control_v0.c",
os/"nodes/esp8266/bridge/board_lab/esp_power_monitor_v0.c",
os/"include/kernel/esp_boot_control_v0.h",
os/"include/kernel/esp_power_monitor_v0.h",
os/"nodes/esp8266/bridge/test_esp_boot_and_power_v0.c",
]
for p in paths:
    assert p.exists()
    print("SOURCE_SHA256",p.relative_to(os),sha256(p.read_bytes()).hexdigest().upper(),flush=True)
with TemporaryDirectory(prefix="deus-esp-boot-power-v0-") as tmp:
    for label, flags in (
        ("O0",["-O0"]),
        ("OS",["-Os"]),
        ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"]),
    ):
        program=Path(tmp)/label
        cmd=["gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",
             *flags,"-I",str(os/"include"),
             str(paths[0]),str(paths[1]),str(paths[4]),"-o",str(program)]
        c=subprocess.run(cmd,capture_output=True,text=True,timeout=20)
        print("COMPILE_"+label+"="+str(c.returncode),flush=True)
        if c.stderr:print(c.stderr[-4000:],flush=True)
        assert c.returncode==0
        t=subprocess.run([str(program)],capture_output=True,text=True,timeout=12)
        print("TEST_"+label+"="+str(t.returncode),flush=True)
        if t.stdout:print(t.stdout[-2000:],flush=True)
        if t.stderr:print(t.stderr[-3000:],flush=True)
        assert t.returncode==0 and "ESP_BOOT_AND_POWER_V0_OFFLINE=PASS" in t.stdout
print("BOOT_AND_POWER_OFFLINE_HOST_GCC=3/3 PASS HARDWARE=NOT_TOUCHED",flush=True)
