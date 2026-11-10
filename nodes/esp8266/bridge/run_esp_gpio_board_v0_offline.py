#!/usr/bin/env python3
"""Source-only ARM peripheral GPIOB PB0/PB1 driver with host MMIO mocks.
NO USB/CH340/STM32/ESP GPIO access. Native ARM size NOT checked.
"""
from pathlib import Path
from hashlib import sha256
from tempfile import TemporaryDirectory
import subprocess
root=Path(__file__).resolve().parents[3]
sources=[root/"nodes/esp8266/bridge/board_lab/esp_gpio_board_v0.c",
         root/"include/drivers/esp_gpio_board_v0.h",
         root/"nodes/esp8266/bridge/test_esp_gpio_board_v0_offline.c"]
for p in sources:
    print("SOURCE_SHA256",p.relative_to(root),sha256(p.read_bytes()).hexdigest().upper(),flush=True)
with TemporaryDirectory(prefix="deus_gpio_pb0_pb1_v0_") as folder:
    for name,flags in [("O0",["-O0"]),("Os",["-Os"]),
                       ("UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all"])]:
        out=Path(folder)/name
        cmd=["gcc","-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",
             "-DDEUS_ESP_GPIO_MMIO_MOCK=1",*flags,
             "-I",str(root/"include"),
             str(sources[0]),str(sources[2]),"-o",str(out)]
        p=subprocess.run(cmd,timeout=20,text=True,capture_output=True)
        print("COMPILE",name,p.returncode,p.stderr[-2000:],flush=True)
        assert p.returncode==0
        q=subprocess.run([str(out)],timeout=12,text=True,capture_output=True)
        print("RUN",name,q.returncode,q.stdout.strip(),q.stderr[-2000:],flush=True)
        assert q.returncode==0 and "ESP_GPIO_BOARD_V0=PASS" in q.stdout
print("ESP_GPIO_BOARD_OFFLINE=3/3 PASS HARDWARE=NOT_TOUCHED",flush=True)
