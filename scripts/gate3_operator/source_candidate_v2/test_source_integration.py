#!/usr/bin/env python3
"""Targeted candidate-v2 integration proof. No hardware, sudo or SSH."""
from __future__ import annotations
from pathlib import Path
import ast
import hashlib
import json
import re
import subprocess
import sys
import tempfile

BASE = Path(__file__).resolve().parent
FILES = ("Program.cs", "remote_handoff_v3.py", "remote_result_guard.py",
         "remote_receipt_bridge.py", "WindowsResultOwner.psm1")
COUNT = 0


def require(condition, detail):
    if not condition:
        raise AssertionError(detail)


def good():
    return {
        "schema": "DEUS_G3_000D_ONE_RESET_INFO_PHYSICAL_V1",
        "result": "PASS", "classification": "SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH",
        "host": "UBUNTU_MACMINI", "physical_port": "usb:001:8",
        "firmware_flash": "NO", "firmware_erase": "NO",
        "protocol_run_as_root": "NO", "physical_identity_authenticated": "NO",
        "gate3_full": "NOT_ACCEPTED", "gate4": "NOT_AUTHORIZED",
        "reset_attempts": 1, "info_attempts": 1,
        "before": {"pid": "000c", "port": "usb:001:8", "devnum": 18},
        "after": {"pid": "000d", "port": "usb:001:8", "devnum": 19},
        "observed_detach_ordinal": 1, "observed_attach_ordinal": 2,
        "usb_events": [
            {"event": "none", "ordinal": 1, "monotonic": 2.0},
            {"event": "000d:19", "ordinal": 2, "monotonic": 2.3},
        ],
        "permission": {"status": "ALREADY_OWNER_0600", "node": "001/019"},
        "enter": {"exit": 0, "timeout": False, "stderr_empty": True,
                  "tokens": [
                      "RUNTIME_NEGOTIATED=PASS",
                      "RUNTIME_TREE=ccbd50200ec7f0ec53da6b1048d7377271dfb7bd",
                      "RUNTIME_CAPABILITIES=0x00000040",
                      "ENTER_BOOTLOADER_REQUEST_BOUNDARY=ARMED_SINGLE_SEND",
                      "ENTER_RESPONSE=OK_RESETTING"]},
        "info": {"exit": 0, "timeout": False, "stderr_empty": True,
                 "tokens": [
                     "INFO_STOP=InfoObservedGateClosed",
                     "INFO_OBSERVED=True",
                     "INFO_VERSION_FLOOR=5",
                     "INFO_COMMITTED=5",
                     "PHYSICAL_IDENTITY_AUTHENTICATED=False",
                     "DESTRUCTIVE_OPERATION_AUTHORIZED=False"]},
    }


def run_case(label, started, exitcode, text=None, expect="FAIL", classification=None):
    global COUNT
    with tempfile.TemporaryDirectory(prefix="deus_g3_src_v2_") as d:
        path = Path(d) / "remote.json"
        if text is not None:
            path.write_text(text, encoding="utf-8")
        args = [sys.executable, str(BASE / "remote_receipt_bridge.py"),
                started, exitcode, str(path) if text is not None else "MISSING"]
        result = subprocess.run(args, input="", capture_output=True, text=True,
                                timeout=10, cwd=str(BASE))
        require(result.returncode == 0, (label, result.stderr))
        decision = json.loads(result.stdout)
        require(result.stderr == "", (label, result.stderr))
        require(decision["schema"] == "DEUS_G3_REMOTE_RESULT_DECISION_V1", label)
        require(decision["result"] == expect, label)
        if classification is not None:
            require(decision["classification"] == classification, label)
        require(decision["retry_authorized"] is False and
                decision["flash_authorized"] is False and
                decision["full_gate3_accepted"] is False, label)
        COUNT += 1
        print("SOURCE_V2_OFFLINE=" + label + "=PASS")


def check_source():
    global COUNT
    csharp = (BASE / "Program.cs").read_text(encoding="utf-8")
    remote = (BASE / "remote_handoff_v3.py").read_text(encoding="utf-8")
    win = (BASE / "WindowsResultOwner.psm1").read_text(encoding="utf-8")
    guard = (BASE / "remote_result_guard.py").read_bytes()
    canonical = (BASE.parent / "remote_result_guard.py").read_bytes()
    require(guard == canonical and
            hashlib.sha256(guard).hexdigest().upper() ==
            "D2E047BEC44F580946248A0A6A8330F85E17AD1D78E894D69D5CDA7A9C4A54CD",
            "PINNED_GUARD_DRIFT")
    ast.parse(remote, filename="remote_handoff_v3.py")
    ast.parse((BASE / "remote_receipt_bridge.py").read_text(encoding="utf-8"))
    require(csharp.count("EnterBootloaderAsync(") == 1, "MORE_THAN_ONE_ENTER")
    require(csharp.count("ProbeAsync(") == 1, "MORE_THAN_ONE_INFO")
    require("private static readonly bool HardwareExecutionAdmitted = false;" in csharp and
            'return Fail("SOURCE_CANDIDATE_HARDWARE_NOT_RELEASED");' in csharp,
            "C_SHARP_HARDWARE_ENTRY_NOT_BLOCKED")
    require("StringComparer.Ordinal.Equals(info.SourceTree, ExpectedInstalledRuntimeTree)" in csharp,
            "SOURCE_TREE_PRE_ENTER_PIN_ABSENT")
    require(csharp.index("INSTALLED_RUNTIME_TREE_PIN_MISMATCH") <
            csharp.index("ENTER_BOOTLOADER_REQUEST_BOUNDARY=ARMED_SINGLE_SEND") <
            csharp.index("EnterBootloaderAsync("), "PIN_ORDER_INVALID")
    require(remote.count('helper("enter",') == 1 and
            remote.count('helper("info",') == 1, "REMOTE_MORE_THAN_ONE_OPERATION")
    require('result_guard.stage_receipt(R["enter"],"ENTER")' in remote and
            'result_guard.stage_receipt(R["info"],"INFO")' in remote and
            'result_guard.success_is_exact(R)' in remote, "STRICT_RECEIPT_NOT_WIRED")
    require('SOURCE_ONLY_NOT_FOR_HARDWARE_EXECUTION' in remote and
            'sys.exit(main())' in remote.split('# Admission must be separate;')[1],
            "REMOTE_ENTRY_NOT_BLOCKED")
    require("Complete-G3CapturedResult" in win and
            "Get-G3NativeRemoteDecision" in win and
            "WindowsNativeJsonGuard.psm1" in win and
            "Get-Command python" not in win and
            "result='FAIL'" in win, "WINDOWS_ADJUDICATION_NOT_CONNECTED")
    require("ssh.exe" not in win and "scp.exe" not in win and
            "EnterBootloaderAsync" not in win, "WINDOWS_OWNER_CAN_TOUCH_HARDWARE")
    COUNT += 1
    print("SOURCE_V2_OFFLINE=STATIC_SCOPE_PASS")


check_source()
positive = json.dumps(good(), separators=(",", ":"))
run_case("VALID_RECEIPT_SCOPED_ONLY", "started", "0", positive,
         "SCOPED_PASS", "SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH")
run_case("SSH_TIMEOUT", "started", "UNAVAILABLE", None,
         classification="RESET_POSSIBLY_ATTEMPTED")
run_case("JSON_MISSING", "started", "0", None,
         classification="RESET_POSSIBLY_ATTEMPTED")
run_case("JSON_BAD", "started", "0", "{",
         classification="RESET_POSSIBLY_ATTEMPTED")
run_case("JSON_DUPLICATE_KEY", "started", "0",
         '{"reset_attempts":1,"reset_attempts":0}',
         classification="RESET_POSSIBLY_ATTEMPTED")
run_case("PRE_CHILD_STOP", "not-started", "UNAVAILABLE", None,
         classification="STOP_BEFORE_RESET")
run_case("REMOTE_ZERO_ATTEMPT_UNTRUSTWORTHY", "started", "9",
         '{"result":"FAIL","classification":"STOP_BEFORE_RESET","reset_attempts":0,"info_attempts":0}',
         classification="RESET_POSSIBLY_ATTEMPTED")
bad = good()
bad["enter"]["tokens"][1] = "RUNTIME_TREE=081aac70c31909bf681254393e2288917cb9b8b2"
run_case("WRONG_REV5_TREE", "started", "0", json.dumps(bad),
         classification="RESET_POSSIBLY_ATTEMPTED")
bad = good()
bad["info"]["tokens"].append("INFO_STOP=InfoObservedGateClosed")
run_case("DUPLICATE_INFO_FIELD", "started", "0", json.dumps(bad),
         classification="RESET_POSSIBLY_ATTEMPTED")
bad = good()
bad["enter"]["tokens"][2] = "RUNTIME_CAPABILITIES=0x00000000"
run_case("MISSING_FW_CAPABILITY", "started", "0", json.dumps(bad),
         classification="RESET_POSSIBLY_ATTEMPTED")

# Regression: if any late strict-validation error occurs after an internal
# provisional PASS claim, the persisted remote result MUST be reset to FAIL.
import contextlib
import importlib.util
import io
spec = importlib.util.spec_from_file_location("g3_v2_offline_remote", BASE / "remote_handoff_v3.py")
assert spec and spec.loader
remote = importlib.util.module_from_spec(spec)
spec.loader.exec_module(remote)
remote.persist = lambda: None
remote.execute = lambda: (_ for _ in ()).throw(RuntimeError("LATE_RECEIPT_REJECTED"))
remote.R["result"] = "PASS"
remote.R["reset_attempts"] = 1
with contextlib.redirect_stdout(io.StringIO()):
    rc = remote.main()
require(rc == 9 and remote.R["result"] == "FAIL" and
        remote.R["classification"] == "STOP_AFTER_ONE_ENTER_NO_RETRY",
        "PROVISIONAL_PASS_SURVIVED_FAILURE")
COUNT += 1
print("SOURCE_V2_OFFLINE=PROVISIONAL_PASS_RESET_TO_FAIL=PASS")

print(f"GATE3_SOURCE_V2_INTEGRATION_OFFLINE=PASS COUNT={COUNT} SSH=NO USB=NO RESET=NO FLASH=NO")
