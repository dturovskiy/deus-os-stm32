#!/usr/bin/env python3
"""Pure offline Gate3 physical remote-result adjudication.

NEVER executes an operator, imports a transport, sends a command, or authorizes
hardware release. This code is a safety component for a future, separately
reviewed Windows result owner; not an admission mechanism.
"""
from __future__ import annotations

from dataclasses import dataclass
import math
import re
from typing import Any, Mapping

SCHEMA = "DEUS_G3_000D_ONE_RESET_INFO_PHYSICAL_V1"
PORT = "usb:001:8"
EXPECTED_RUNTIME_TREE = "ccbd50200ec7f0ec53da6b1048d7377271dfb7bd"
SUCCESS = "SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH"
STOP_UNCERTAIN = "RESET_POSSIBLY_ATTEMPTED"
STOP_BEFORE = "STOP_BEFORE_RESET"
STOP_AFTER = "STOP_AFTER_ONE_ENTER_NO_RETRY"
HEX40 = re.compile(r"[0-9a-f]{40}\Z")


@dataclass(frozen=True)
class Adjudication:
    classification: str
    accepted: bool
    reset_may_have_been_attempted: bool
    info_may_have_been_attempted: bool
    retry_authorized: bool = False
    flash_authorized: bool = False
    full_gate3_accepted: bool = False
    reason: str = "NOT_ATTESTED"


class Reject(ValueError):
    pass


def need(condition: bool, reason: str) -> None:
    if not condition:
        raise Reject(reason)


def fields(obj: Any, required: set[str]) -> Mapping[str, Any]:
    need(type(obj) is dict and required <= obj.keys(), "REMOTE_RECEIPT_STRUCTURE_INVALID")
    return obj


def integer(value: Any, minimum: int, maximum: int, why: str) -> int:
    need(type(value) is int and minimum <= value <= maximum, why)
    return value


def exact_lines(value: Any, required: Mapping[str, str]) -> dict[str, str]:
    need(type(value) is list and 0 < len(value) <= 16, "MISSING_OR_OVERSIZE_TOKENS")
    parsed: dict[str, str] = {}
    for item in value:
        need(type(item) is str and 0 < len(item) <= 160 and
             "\r" not in item and "\n" not in item, "UNSAFE_TOKEN")
        key, sep, entry = item.partition("=")
        need(sep == "=" and re.fullmatch(r"[A-Z_]+", key) is not None and
             key not in parsed and 0 < len(entry) <= 100, "MALFORMED_OR_DUPLICATE_TOKEN")
        parsed[key] = entry
    for key, expected in required.items():
        need(parsed.get(key) == expected, "EXPECTED_TOKEN_MISSING_" + key)
    permitted = ({"RUNTIME_NEGOTIATED", "RUNTIME_TREE", "RUNTIME_CAPABILITIES",
                  "ENTER_BOOTLOADER_REQUEST_BOUNDARY", "ENTER_RESPONSE"}
                 if "ENTER_RESPONSE" in required else
                 {"INFO_STOP", "INFO_OBSERVED", "INFO_VERSION_FLOOR",
                  "INFO_COMMITTED", "PHYSICAL_IDENTITY_AUTHENTICATED",
                  "DESTRUCTIVE_OPERATION_AUTHORIZED"})
    need(set(parsed) == permitted, "UNEXPECTED_OR_MISSING_TOKEN_FIELDS")
    return parsed


def stage_receipt(value: Any, phase: str) -> dict[str, str]:
    obj = fields(value, {"exit", "timeout", "stderr_empty", "tokens"})
    need(type(obj["exit"]) is int and obj["exit"] == 0 and
         obj["timeout"] is False and obj["stderr_empty"] is True,
         phase + "_PROCESS_OR_STDERR_FAILURE")
    return exact_lines(obj["tokens"], {
        "ENTER_RESPONSE": "OK_RESETTING"
    } if phase == "ENTER" else {
        "INFO_STOP": "InfoObservedGateClosed",
        "INFO_OBSERVED": "True",
        "PHYSICAL_IDENTITY_AUTHENTICATED": "False",
        "DESTRUCTIVE_OPERATION_AUTHORIZED": "False",
    })


def success_is_exact(r: Any) -> None:
    obj = fields(r, {"schema", "result", "classification", "host", "physical_port",
                     "firmware_flash", "firmware_erase", "protocol_run_as_root",
                     "physical_identity_authenticated", "gate3_full", "gate4",
                     "reset_attempts", "info_attempts", "before", "after",
                     "observed_detach_ordinal", "observed_attach_ordinal",
                     "usb_events", "enter", "info", "permission"})
    exact = {
        "schema": SCHEMA,
        "result": "PASS",
        "classification": SUCCESS,
        "host": "UBUNTU_MACMINI",
        "physical_port": PORT,
        "firmware_flash": "NO",
        "firmware_erase": "NO",
        "protocol_run_as_root": "NO",
        "physical_identity_authenticated": "NO",
        "gate3_full": "NOT_ACCEPTED",
        "gate4": "NOT_AUTHORIZED",
    }
    for name, expected in exact.items():
        need(obj[name] == expected, "REMOTE_SCOPE_DRIFT_" + name)
    integer(obj["reset_attempts"], 1, 1, "RESET_COUNT_NOT_ONE")
    integer(obj["info_attempts"], 1, 1, "INFO_COUNT_NOT_ONE")

    before = fields(obj["before"], {"pid", "port", "devnum"})
    after = fields(obj["after"], {"pid", "port", "devnum"})
    need(before["pid"] == "000c" and after["pid"] == "000d" and
         before["port"] == PORT and after["port"] == PORT, "USB_IDENTITY_DRIFT")
    bdev = integer(before["devnum"], 1, 127, "PRE_DEVNUM_INVALID")
    adev = integer(after["devnum"], 1, 127, "POST_DEVNUM_INVALID")
    need(bdev != adev, "USB_SESSION_NOT_DISTINCT")
    det = integer(obj["observed_detach_ordinal"], 1, 8, "DETACH_ORDINAL_INVALID")
    att = integer(obj["observed_attach_ordinal"], 2, 9, "ATTACH_ORDINAL_INVALID")
    need(det < att, "EVENT_ORDER_INVALID")
    events = obj["usb_events"]
    need(type(events) is list and len(events) == 2, "USB_EVENT_COUNT_NOT_TWO")
    for index, (event, want) in enumerate(zip(events, ("none", f"000d:{adev}")), 1):
        entry = fields(event, {"event", "ordinal", "monotonic"})
        need(entry["event"] == want and entry["ordinal"] == index and
             type(entry["monotonic"]) in (int, float) and
             type(entry["monotonic"]) is not bool and
             math.isfinite(entry["monotonic"]) and
             entry["monotonic"] >= 0, "USB_EVENT_MISMATCH")
    need(det == 1 and att == 2 and
         events[0]["monotonic"] < events[1]["monotonic"],
         "USB_EVENT_TIMELINE_NOT_ESTABLISHED")

    permission = fields(obj["permission"], {"status", "node"})
    need(permission["status"] in ("ALREADY_OWNER_0600", "EPHEMERAL_NODE_OWNER_0600_PASS")
         and permission["node"] == f"001/{adev:03d}", "000D_NODE_PERMISSION_NOT_PROVEN")

    enter = stage_receipt(obj["enter"], "ENTER")
    need(enter.get("RUNTIME_NEGOTIATED") == "PASS" and
         enter.get("ENTER_BOOTLOADER_REQUEST_BOUNDARY") == "ARMED_SINGLE_SEND" and
         enter.get("RUNTIME_TREE") == EXPECTED_RUNTIME_TREE and
         HEX40.fullmatch(enter["RUNTIME_TREE"]) is not None,
         "ENTER_SOURCE_OR_BOUNDARY_NOT_PROVEN")
    cap_text = enter["RUNTIME_CAPABILITIES"]
    need(re.fullmatch(r"0x[0-9A-F]{8}", cap_text) is not None and
         (int(cap_text[2:], 16) & (1 << 6)) != 0,
         "ENTER_FIRMWARE_UPDATE_CAPABILITY_NOT_PROVEN")
    info = stage_receipt(obj["info"], "INFO")
    need(re.fullmatch(r"[0-9]+", info.get("INFO_VERSION_FLOOR", "")) is not None and
         re.fullmatch(r"[0-9]+", info.get("INFO_COMMITTED", "")) is not None,
         "INFO_FLOOR_OR_COMMITTED_MISSING")
    floor = int(info["INFO_VERSION_FLOOR"])
    committed = int(info["INFO_COMMITTED"])
    need(0 < committed <= floor < 7 and committed in (5, 6),
         "FLOOR_POLICY_BLOCKED")
    # Only an observation; never an authenticated physical identity or flash permission.


def adjudicate(*, remote_started: bool, process_exit: int | None,
              remote_receipt: Any) -> Adjudication:
    """Strict success or conservative STOP; never raises to caller.

    Once the remote execution boundary may have been entered, any missing,
    malformed, conflicting or untransported result conservatively means that
    a reset might already have been sent. There is no retry path.
    """
    if remote_started is not True:
        return Adjudication(STOP_BEFORE, False, False, False,
                            reason="REMOTE_NOT_STARTED")
    if process_exit is None or type(process_exit) is not int:
        return Adjudication(STOP_UNCERTAIN, False, True, True,
                            reason="REMOTE_EXIT_UNAVAILABLE")
    if type(remote_receipt) is not dict:
        return Adjudication(STOP_UNCERTAIN, False, True, True,
                            reason="REMOTE_RECEIPT_MISSING_OR_INVALID")
    reset = remote_receipt.get("reset_attempts")
    info = remote_receipt.get("info_attempts")
    if type(reset) is not int or reset not in (0, 1) or \
       type(info) is not int or info not in (0, 1) or info > reset:
        return Adjudication(STOP_UNCERTAIN, False, True, True,
                            reason="ATTEMPT_COUNT_UNTRUSTWORTHY")
    if reset == 0:
        # A remote JSON claim alone is insufficient to establish that the
        # possibly mutating child never sent ENTER. STOP_BEFORE_RESET is
        # allowed only when the local result owner did not start the child.
        return Adjudication(STOP_UNCERTAIN, False, True, True,
                            reason="REMOTE_ZERO_ENTER_CLAIM_UNATTESTED")
    if process_exit != 0:
        return Adjudication(STOP_AFTER, False, True, bool(info),
                            reason="REMOTE_FAILURE_AFTER_ENTER_BOUNDARY")
    try:
        success_is_exact(remote_receipt)
    except (Reject, KeyError, TypeError, ValueError, OverflowError) as exc:
        return Adjudication(STOP_UNCERTAIN, False, True, bool(info),
                            reason="MALFORMED_SUCCESS_RECEIPT_" + type(exc).__name__)
    return Adjudication(SUCCESS, True, True, True, reason="SCOPED_READONLY_INFO_ONLY")


if __name__ == "__main__":
    raise SystemExit("Offline library only: import and test; never execute a device operator.")
