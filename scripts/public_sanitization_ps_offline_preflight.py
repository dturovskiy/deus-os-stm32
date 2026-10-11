#!/usr/bin/env python3
"""Static non-hardware checks for the Gate-1B PowerShell tool resolver change."""
from __future__ import annotations
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
SCRIPTS = ROOT / "scripts"
SOURCE = (
    "build_bootloader.ps1", "build_firmware.ps1",
    "create_asset_recovery_bundle.ps1", "create_bootloader_recovery_bundle.ps1",
    "stm32_asset_recovery.ps1", "stm32_bootloader_recovery.ps1",
    "stm32_readonly_flash_preflight.ps1",
)
recovery = SOURCE[4:]
build = SOURCE[:2]
generator = SOURCE[2:4]
data = {name: (SCRIPTS / name).read_text(encoding="utf-8") for name in SOURCE}
def ensure(condition: bool, why: str) -> None:
    if not condition:
        raise AssertionError(why)

for name, source in data.items():
    ensure(r"D:\Projects\STM32\Tools" not in source, f"hardcoded tool root: {name}")
    ensure(source.strip().startswith("param("), f"unexpected script header: {name}")
    ensure(source.count("Set-StrictMode") >= 1, f"strict-mode missing: {name}")

for name in build:
    source = data[name]
    ensure("[string]$ToolchainRoot = $env:DEUS_ARM_TOOLCHAIN_ROOT" in source, f"toolchain default: {name}")
    ensure("Get-Command -Name 'arm-none-eabi-gcc.exe' -CommandType Application -All" in source, f"unambiguous gcc discovery: {name}")

for name in recovery:
    source = data[name]
    ensure("[string]$ProgrammerCli = $env:DEUS_STM32_PROGRAMMER_CLI" in source, f"programmer default: {name}")
    ensure("Get-Command -Name 'STM32_Programmer_CLI.exe' -CommandType Application -All" in source, f"unambiguous programmer discovery: {name}")
    ensure("GetFileName($ProgrammerCli)" in source, f"programmer log path exposure: {name}")
    ensure(source.count("not uniquely configured") == 1, f"fail-closed condition count: {name}")

for name in generator:
    source = data[name]
    ensure(source.count("[string]$ProgrammerCli = $env:DEUS_STM32_PROGRAMMER_CLI") == 2,
           f"wrapper environment resolver count: {name}")
    ensure(source.count("-ProgrammerCli $ProgrammerCli") >= 2, f"wrapper forwarding: {name}")
    for wrap in ("PreserveWrapper", "CleanWrapper"):
        pattern = r"\$" + wrap + r"\s*=\s*@'\r?\n(?P<body>.*?)\r?\n'@"
        match = re.search(pattern, source, flags=re.DOTALL)
        ensure(match is not None, f"missing wrapper {name}:{wrap}")
        body = match.group("body")
        ensure(body.count("$env:DEUS_STM32_PROGRAMMER_CLI") == 1, f"wrapper default {name}:{wrap}")
        ensure(body.count("-ProgrammerCli $ProgrammerCli") == 1, f"wrapper forwarding {name}:{wrap}")

boot = data["stm32_bootloader_recovery.ps1"]
ensure("VALIDATE_ONLY" in boot and "PRESERVE_PERSISTENCE" in boot and "CLEAN_STATE" in boot,
       "bootloader mode set changed")
ensure("0xF800" in boot and "0xF400" in boot and "65536" in boot,
       "bootloader/persistence geometry changed")
for name in SOURCE:
    ensure(not re.search(r"['\"][A-Za-z]:\\Projects\\STM32\\Tools", data[name]),
           f"Windows operator fallback: {name}")

wf = (ROOT / ".github/workflows/powershell-portability.yml").read_text(encoding="utf-8")
ensure("runs-on: windows-2022" in wf and "shell: pwsh" in wf, "Windows runtime test absent")
ensure("public_sanitization_power_shell_offline.ps1" in wf, "Windows offline test not wired")
ensure("actions/checkout@" in wf and "persist-credentials: false" in wf,
       "CI checkout trust policy missing")
result = {"result": "PASS", "files": len(SOURCE), "generator_wrappers": 4,
          "recovery_modes_preserved": True,
          "recovery_script_executed": False, "target_access": False,
          "windows_runtime_proof": "REQUIRED_FROM_HOSTED_JOB"}
print(json.dumps(result, sort_keys=True))
