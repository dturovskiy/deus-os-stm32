#!/usr/bin/env python3
"""Deterministic in-memory tests of fail-closed post-reset receipt policy."""
from __future__ import annotations

from copy import deepcopy
import importlib.util
from pathlib import Path

root = Path("scripts/gate3_operator").resolve()
spec = importlib.util.spec_from_file_location("g3_result_policy", root / "remote_result_guard.py")
assert spec and spec.loader
g = importlib.util.module_from_spec(spec)
import sys
sys.modules[spec.name] = g
spec.loader.exec_module(g)


def good() -> dict:
    return {
        "schema": g.SCHEMA, "result": "PASS", "classification": g.SUCCESS,
        "host": "UBUNTU_MACMINI", "physical_port": "usb:001:8",
        "firmware_flash": "NO", "firmware_erase": "NO",
        "protocol_run_as_root": "NO", "physical_identity_authenticated": "NO",
        "gate3_full": "NOT_ACCEPTED", "gate4": "NOT_AUTHORIZED",
        "reset_attempts": 1, "info_attempts": 1,
        "before": {"pid": "000c", "port": "usb:001:8", "devnum": 90},
        "after": {"pid": "000d", "port": "usb:001:8", "devnum": 91},
        "observed_detach_ordinal": 1, "observed_attach_ordinal": 2,
        "usb_events": [
            {"event": "none", "ordinal": 1, "monotonic": 1.2},
            {"event": "000d:91", "ordinal": 2, "monotonic": 1.5},
        ],
        "permission": {"status": "EPHEMERAL_NODE_OWNER_0600_PASS", "node": "001/091"},
        "enter": {"exit": 0, "timeout": False, "stderr_empty": True,
                  "tokens": [
                      "RUNTIME_NEGOTIATED=PASS",
                      "RUNTIME_TREE=" + g.EXPECTED_RUNTIME_TREE,
                      "RUNTIME_CAPABILITIES=0x00000040",
                      "ENTER_BOOTLOADER_REQUEST_BOUNDARY=ARMED_SINGLE_SEND",
                      "ENTER_RESPONSE=OK_RESETTING",
                  ]},
        "info": {"exit": 0, "timeout": False, "stderr_empty": True,
                 "tokens": [
                     "INFO_STOP=InfoObservedGateClosed",
                     "INFO_OBSERVED=True",
                     "INFO_VERSION_FLOOR=5",
                     "INFO_COMMITTED=5",
                     "PHYSICAL_IDENTITY_AUTHENTICATED=False",
                     "DESTRUCTIVE_OPERATION_AUTHORIZED=False",
                 ]},
    }


cases = 0


def verify(label: str, data: dict | None, exit_status: int | None,
           expected: str | None = None) -> None:
    global cases
    result = g.adjudicate(remote_started=True, process_exit=exit_status,
                          remote_receipt=data)
    assert not result.accepted, (label, result)
    assert result.reset_may_have_been_attempted, (label, result)
    assert not result.retry_authorized and not result.flash_authorized, label
    assert not result.full_gate3_accepted, label
    assert result.classification in (g.STOP_UNCERTAIN, g.STOP_AFTER), label
    if expected:
        assert result.classification == expected, (label, result.classification)
    cases += 1
    print("PASS NEGATIVE", label, result.classification)


def mutate(label: str, fn, *, expected: str | None = None,
           exit_status: int = 0) -> None:
    v = good()
    fn(v)
    verify(label, v, exit_status, expected)


def ok(result: g.Adjudication) -> None:
    global cases
    assert result.accepted and result.classification == g.SUCCESS, result
    assert result.reset_may_have_been_attempted
    assert result.info_may_have_been_attempted
    assert result.retry_authorized is False and result.flash_authorized is False
    assert result.full_gate3_accepted is False
    cases += 1
    print("PASS SCOPED", result.classification)


ok(g.adjudicate(remote_started=True, process_exit=0, remote_receipt=good()))
v = good()
v["permission"]["status"] = "ALREADY_OWNER_0600"
ok(g.adjudicate(remote_started=True, process_exit=0, remote_receipt=v))

before = g.adjudicate(remote_started=False, process_exit=None, remote_receipt=None)
assert before.classification == g.STOP_BEFORE and before.reset_may_have_been_attempted is False
assert not before.accepted and not before.retry_authorized
cases += 1
print("PASS PREBOUNDARY", before.classification)

preflight = {"result": "FAIL", "classification": g.STOP_BEFORE,
             "reset_attempts": 0, "info_attempts": 0}
stop = g.adjudicate(remote_started=True, process_exit=9, remote_receipt=preflight)
assert stop.classification == g.STOP_UNCERTAIN and stop.reset_may_have_been_attempted
assert not stop.accepted and not stop.retry_authorized
cases += 1
print("PASS UNTRUSTED_REMOTE_PRE_RESET_CLAIM", stop.classification)

verify("ssh-timeout", good(), None, g.STOP_UNCERTAIN)
verify("missing-remote-json", None, 9, g.STOP_UNCERTAIN)
verify("corrupt-remote-json", "invalid", 9, g.STOP_UNCERTAIN)
verify("malformed-exit-code", good(), "0", g.STOP_UNCERTAIN)
verify("fail-after-enter", {**good(), "result": "FAIL"}, 9, g.STOP_AFTER)
verify("false-zero-after-boundary", {"result": "PASS", "classification": g.SUCCESS,
                                   "reset_attempts": 0, "info_attempts": 0},
       0, g.STOP_UNCERTAIN)
verify("missing-attempt-counts", {"result": "FAIL", "classification": g.STOP_BEFORE},
       9, g.STOP_UNCERTAIN)
verify("bool-attempt-counts", {**good(), "reset_attempts": True}, 0, g.STOP_UNCERTAIN)
verify("multiple-resets", {**good(), "reset_attempts": 2}, 0, g.STOP_UNCERTAIN)
verify("multiple-info", {**good(), "info_attempts": 2}, 0, g.STOP_UNCERTAIN)

mutate("false-pass", lambda x: x.update(result="FAIL"))
mutate("unknown-scope", lambda x: x.update(schema="UNKNOWN"))
mutate("wrong-port", lambda x: x["after"].update(port="usb:002:8"))
mutate("wrong-runtime-pid", lambda x: x["before"].update(pid="000e"))
mutate("same-devnum", lambda x: x["after"].update(devnum=90))
mutate("second-detach", lambda x: x["usb_events"].append({
    "event": "none", "ordinal": 3, "monotonic": 1.6}))
mutate("missing-detach", lambda x: x["usb_events"].pop(0))
mutate("wrong-event-order", lambda x: x.update(observed_detach_ordinal=2))
mutate("bad-event-time", lambda x: x["usb_events"][1].update(monotonic=1.0))
mutate("wrong-node-owner", lambda x: x["permission"].update(status="NOT_RUN"))
mutate("wrong-node-number", lambda x: x["permission"].update(node="001/090"))
mutate("enter-rc-nonzero", lambda x: x["enter"].update(exit=23))
mutate("enter-timeout", lambda x: x["enter"].update(timeout=True))
mutate("enter-stderr", lambda x: x["enter"].update(stderr_empty=False))
mutate("no-enter-response", lambda x: x["enter"]["tokens"].pop())
mutate("duplicate-enter-response", lambda x: x["enter"]["tokens"].append(
    "ENTER_RESPONSE=OK_RESETTING"))
mutate("enter-tree-not-pinned", lambda x: x["enter"]["tokens"].__setitem__(
    1, "RUNTIME_TREE=081aac70c31909bf681254393e2288917cb9b8b2"))
mutate("no-enter-send-boundary", lambda x: x["enter"]["tokens"].pop(3))
mutate("info-stderr", lambda x: x["info"].update(stderr_empty=False))
mutate("info-timeout", lambda x: x["info"].update(timeout=True))
mutate("info-missing-stop", lambda x: x["info"]["tokens"].pop(0))
mutate("info-duplicate-stop", lambda x: x["info"]["tokens"].append(
    "INFO_STOP=InfoObservedGateClosed"))
mutate("info-claims-flash", lambda x: x["info"]["tokens"].__setitem__(
    5, "DESTRUCTIVE_OPERATION_AUTHORIZED=True"))
mutate("info-claims-physical-auth", lambda x: x["info"]["tokens"].__setitem__(
    4, "PHYSICAL_IDENTITY_AUTHENTICATED=True"))
mutate("info-wrong-floor", lambda x: x["info"]["tokens"].__setitem__(
    2, "INFO_VERSION_FLOOR=8"))
mutate("info-floor-below-committed", lambda x: x["info"]["tokens"].__setitem__(
    2, "INFO_VERSION_FLOOR=4"))
mutate("info-version-not-decimal", lambda x: x["info"]["tokens"].__setitem__(
    2, "INFO_VERSION_FLOOR=5q"))
mutate("info-no-flash-field", lambda x: x.update(firmware_flash="YES"))
mutate("info-no-root-field", lambda x: x.update(protocol_run_as_root="YES"))
mutate("true-as-devnum", lambda x: x["after"].update(devnum=True))
mutate("absent-gate-status", lambda x: x.pop("gate3_full"))
mutate("info-token-line-injection", lambda x: x["info"]["tokens"].__setitem__(
    0, "INFO_STOP=InfoObservedGateClosed\nFAKE=TRUE"))
mutate("null-remote-json", lambda x: x.update(before=None))

mutate("enter-no-firmware-update-bit", lambda x: x["enter"]["tokens"].__setitem__(
    2, "RUNTIME_CAPABILITIES=0x00000001"))
mutate("enter-capability-not-hex", lambda x: x["enter"]["tokens"].__setitem__(
    2, "RUNTIME_CAPABILITIES=0x0000ZZ40"))
mutate("enter-capability-lowercase", lambda x: x["enter"]["tokens"].__setitem__(
    2, "RUNTIME_CAPABILITIES=0x0000004f"))
mutate("enter-unexpected-extra-field", lambda x: x["enter"]["tokens"].append(
    "UNREVIEWED_FIELD=1"))
mutate("info-unexpected-extra-field", lambda x: x["info"]["tokens"].append(
    "UNREVIEWED_FIELD=1"))
mutate("nan-usb-event-time", lambda x: x["usb_events"][0].update(
    monotonic=float("nan")))
mutate("inf-usb-event-time", lambda x: x["usb_events"][1].update(
    monotonic=float("inf")))
mutate("bool-ordinal", lambda x: x.update(observed_detach_ordinal=True))
mutate("zero-entire-runtime-capability", lambda x: x["enter"]["tokens"].__setitem__(
    2, "RUNTIME_CAPABILITIES=0x00000000"))
mutate("info-success-empty-version", lambda x: x["info"]["tokens"].__setitem__(
    2, "INFO_VERSION_FLOOR="))
mutate("enter-duplicate-negotiated", lambda x: x["enter"]["tokens"].insert(
    0, "RUNTIME_NEGOTIATED=PASS"))
mutate("info-other-result-field", lambda x: x["info"]["tokens"].__setitem__(
    0, "INFO_STOP=InfoObservedGateClosedExtra"))

print(f"GATE3_REMOTE_RESULT_GUARD=PASS COUNT={cases} LIVE_USB=NO SUDO=NO RESET=NO FLASH=NO")
