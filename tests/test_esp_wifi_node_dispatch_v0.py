"""Native offline C11 tests for ESP8266 Wi-Fi dispatcher + SDK v3.4 mocks.

Compile the real portable source under O0, Os and UBSan, exercising
the older frozen DNL worker, Wi-Fi extension, on-demand scan task service,
replay/correlation and failed scan events. All APs are synthetic.

No device, USB, UART, CH340, SDK firmware build, Wi-Fi radio or credentials.
Run: python3 tests/test_esp_wifi_node_dispatch_v0.py
"""
from pathlib import Path
from hashlib import sha256
import subprocess
import tempfile
import unittest

OS = Path(__file__).resolve().parents[1]
FW = OS / "nodes/esp8266/firmware"
LAB = FW / "wifi_lab"
T = FW / "tests"
SRC = OS / "shared/node_link/src/deus_node_link.c"
DNL_INCLUDE = OS / "shared/node_link/include"

SOURCES = {
    "scan": [LAB / "wifi_scan_v0.c",
             T / "wifi_scan_v0_sdk_mock_tests.c"],
    "dispatcher": [SRC, LAB / "wifi_scan_v0.c",
                   LAB / "wifi_node_extension_v0.c",
                   LAB / "wifi_node_dispatch_v0.c",
                   LAB / "wifi_node_sdk_bridge_v0.c",
                   T / "wifi_node_dispatch_v0_sdk_tests.c"],
}
COMPILATION = {
    "O0": ["-O0"],
    "Os": ["-Os"],
    "UBSAN": ["-O1", "-fsanitize=undefined",
              "-fno-sanitize-recover=all"],
}


class WifiNodeSdkSourceTests(unittest.TestCase):
    def compile_and_execute(self, kind: str, mode: str) -> None:
        paths = SOURCES[kind]
        for path in paths:
            self.assertTrue(path.is_file(), str(path))
            self.assertEqual(len(sha256(path.read_bytes()).hexdigest()), 64)
        with tempfile.TemporaryDirectory(prefix="deus_wifi_sdk_") as temp:
            executable = Path(temp) / (kind + "-" + mode)
            arguments = [
                "gcc", "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                *COMPILATION[mode],
                "-I", str(DNL_INCLUDE),
                "-I", str(LAB),
                "-I", str(T / "sdk_mock_wifi"),
                *(str(path) for path in paths),
                "-o", str(executable),
            ]
            compile_result = subprocess.run(
                arguments, cwd=OS, capture_output=True, text=True,
                timeout=30, check=False,
            )
            self.assertEqual(compile_result.returncode, 0,
                             compile_result.stderr[-4000:])
            result = subprocess.run(
                [str(executable)], cwd=OS, capture_output=True, text=True,
                timeout=15, check=False,
            )
            self.assertEqual(result.returncode, 0, result.stderr[-4000:])
            expected = ("ESP_WIFI_NODE_DISPATCH_SDK_V34_MOCK=PASS"
                        if kind == "dispatcher"
                        else "ESP_WIFI_SCAN_SDK_V34_API_MOCK=PASS")
            self.assertIn(expected, result.stdout)

    def test_six_native_modes(self) -> None:
        for kind in ("scan", "dispatcher"):
            for mode in COMPILATION:
                with self.subTest(kind=kind, mode=mode):
                    self.compile_and_execute(kind, mode)


if __name__ == "__main__":
    unittest.main(verbosity=2)
