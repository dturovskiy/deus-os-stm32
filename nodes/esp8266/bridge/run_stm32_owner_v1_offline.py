#!/usr/bin/env python3
"""Compile product STM32 ESP controller with mock MMIO, two build variants."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
root=Path.cwd()
sources=[
root/"src/kernel/esp_stm32_owner_v1.c",
root/"src/kernel/esp_boot_control_v0.c",
root/"src/drivers/esp_gpio_board_v0.c",
root/"nodes/esp8266/bridge/test_stm32_owner_v1_offline.c",
]
with TemporaryDirectory(prefix="deus_esp_stm32_owner_v1_") as td:
    for label,opts in [
        ("DEFAULT_DENY",["-O0"]),
        ("QUALIFIED_MOCK",["-Os","-DDEUS_ESP_STM32_BOARD_QUALIFIED=1","-DDEUS_ESP_GPIO_MMIO_MOCK"]),
        ("QUALIFIED_UBSAN",["-O1","-fsanitize=undefined","-fno-sanitize-recover=all",
                            "-DDEUS_ESP_STM32_BOARD_QUALIFIED=1","-DDEUS_ESP_GPIO_MMIO_MOCK"])
    ]:
        obj=Path(td)/label
        # Both compiled variants use in-memory GPIO registers; default deny
        # must produce no register writes at all.
        cmd=["gcc","-std=c11","-Wall","-Wextra","-Werror","-Wpedantic",
             "-DDEUS_ESP_GPIO_MMIO_MOCK",*opts,"-I",str(root/"include"),
             "-I",str(root/"nodes/esp8266/firmware/wifi_lab"),
             *(str(x) for x in sources),"-o",str(obj)]
        build=subprocess.run(cmd,capture_output=True,text=True,timeout=22)
        if build.returncode:
            print(label,build.stderr,flush=True)
            raise SystemExit(3)
        test=subprocess.run([str(obj)],capture_output=True,text=True,timeout=8)
        print(label,"BUILD=PASS RUN="+str(test.returncode),test.stdout.strip(),test.stderr.strip(),flush=True)
        if test.returncode:raise SystemExit(4)
print("ESP_STM32_OWNED_BOARD_SOURCE=3/3 PASS NO_DEVICE",flush=True)
