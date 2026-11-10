"""Dependency-free Python driver for real C11 Wi-Fi Node Link extension tests.

The frozen Node Link REV2 C source is compiled together with the proposed
pure Wi-Fi extension. Three native GCC variants: O0, Os and UBSan.
NO device, UART, CH340, Wi-Fi radio, SDK, Flash or credentials.
Run: python3 tests/test_esp_wifi_node_extension_v0.py
"""
from hashlib import sha256
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "nodes/esp8266/firmware/wifi_lab/wifi_node_extension_v0.c"
HDR = SRC.with_suffix(".h")
SCAN = SRC.parent / "wifi_scan_v0.h"
REV2_HDR = ROOT / "include/deus_node_link.h"
REV2_SRC = ROOT / "src/drivers/deus_node_link.c"
TEST = ROOT / "nodes/esp8266/firmware/tests/wifi_node_extension_v0_tests.c"


class WifiNodeExtensionNativeCTest(unittest.TestCase):
    def _build_and_test(self, mode: str, flags: list[str]) -> None:
        for path in (SRC, HDR, SCAN, REV2_SRC, REV2_HDR, TEST):
            self.assertTrue(path.is_file(), str(path))
            digest = sha256(path.read_bytes()).hexdigest()
            self.assertEqual(len(digest), 64)
        with tempfile.TemporaryDirectory(prefix="deus_wifi_node_ext_") as td:
            exe = Path(td) / mode
            build = subprocess.run(
                [
                    "gcc", "-std=c11", "-Wall", "-Wextra", "-Wpedantic",
                    "-Werror", *flags, "-I", str(ROOT / "include"),
                    "-I", str(SRC.parent),
                    str(REV2_SRC), str(SRC), str(TEST), "-o", str(exe),
                ],
                text=True, capture_output=True, timeout=30, check=False,
                cwd=ROOT,
            )
            self.assertEqual(
                build.returncode, 0,
                f"{mode} compile failed:\n{build.stdout}\n{build.stderr}",
            )
            run = subprocess.run(
                [str(exe)], text=True, capture_output=True, timeout=15,
                check=False, cwd=ROOT,
            )
            self.assertEqual(
                run.returncode, 0,
                f"{mode} test failed:\n{run.stdout}\n{run.stderr}",
            )
            self.assertIn("ESP_WIFI_NODE_EXTENSION_V0_SOURCE_ONLY=PASS",
                          run.stdout)

    def test_O0(self) -> None:
        self._build_and_test("O0", ["-O0"])

    def test_Os(self) -> None:
        self._build_and_test("Os", ["-Os"])

    def test_UBSAN(self) -> None:
        self._build_and_test(
            "UBSAN", ["-O1", "-fsanitize=undefined",
                      "-fno-sanitize-recover=all"],
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
