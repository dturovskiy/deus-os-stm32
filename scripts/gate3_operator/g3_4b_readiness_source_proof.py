#!/usr/bin/env python3
"""Purely offline source/evidence-bound G3-4b recovery-route pre-adjudication.

No hardware, USB, SSH, signing, firmware package extraction, Git writes or
mutation. This is NOT original Windows native evidence or FULL_GATE3 approval.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from pathlib import Path
from typing import Any

OS = Path(__file__).resolve().parents[2]
PROJECT = OS.parent
TREE = "081aac70c31909bf681254393e2288917cb9b8b2"
V6 = "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
V7 = "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A"
REV5_TREE = "ccbd50200ec7f0ec53da6b1048d7377271dfb7bd"

SOURCE = {
    "OS/bootloader/bootloader.c": "A7A52D4962F48D7C95506FB97613BC6A5F7DA661BC07CF3CE850D86247EE25E3",
    "OS/host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs": "CFBFF53BDE5D0C49303F0733448AEF61AC63172CB2478A558E92062A79E8256B",
    "OS/host/src/DeusOs.Control.Core/DeusDeviceClient.cs": "CEDB67F7314977BB8F8D6FB6CDEC45A15E88AFA3923C7807DA238FCCD0906CA8",
    "OS/host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs": "6453EDCE4EDEC8B827E87B771D9E8081004D05051F9A24662940E0217DCEED82",
    "OS/host/src/DeusOs.Control.Transport.Windows/WindowsBootloaderWinUsbDiscovery.cs": "A4E6481CD5D7FB16E05ECFC29A2530911D641005E4C78742584A393BF59B59CD",
    "OS/docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md": "1777AF7572F3A27091C0F53726329166B5908FF9E64D02F6A9EF68EF64CC1A73",
}
RECORDS = {
    "gate2_tree": ("Bundle/_recovered_gate2_host165_exact_20261026/GATE2_EXACT_SOURCE_TREE_RECONSTRUCTION_PROOF.json",
                   "538CEA9E913C410DFF7348281B0DE712523462432D938495C287F5F00B1992D9"),
    "oracle": ("Reports/ManagementOnlyRuntimeMigration/Gate3/Acceptance/GATE3_RECOVERY_ROUTE_SOURCE_ORACLE_20261026.json",
               "1E9D52CAC23A8AD25CB6746FCB5598AA072F4A0676499A422589BE687293C963"),
    "v6_native": ("Reports/ManagementOnlyRuntimeMigration/Gate3/Acceptance/GATE3_000E_V6_NATIVE_SIGNED_ACCEPTANCE_20261026.md",
                   "509D17DB79796BFD98746B3C6AFEAFEAF398FA5CD2E44073CBA0065137D0E6A5"),
    "pair_native": ("Reports/ManagementOnlyRuntimeMigration/Gate3/Acceptance/GATE3_000D_SIGNED_PAIR_OFFLINE_ROUTE_V1_NATIVE_ACCEPTANCE_20261026.md",
                     "75468F352396E1D9EEE6C2F9B3556CCE182D6149F07FB37D2B4807A1C7DEC236"),
}
SIGNED_V6_BYTES, SIGNED_V7_BYTES = 49072, 52092
IMAGE_V6_BYTES, IMAGE_V7_BYTES = 48992, 52012


class ProofReject(ValueError):
    pass


def need(cond: Any, code: str) -> None:
    if not cond:
        raise ProofReject(code)


def read_verified(relative: str, expected_sha: str) -> bytes:
    p = PROJECT / relative
    need(p.is_file() and not p.is_symlink(), "INPUT_FILE_MISSING_OR_SYMLINK:" + relative)
    data = p.read_bytes()
    need(len(data) <= 200000 and hashlib.sha256(data).hexdigest().upper() == expected_sha,
         "PINNED_FILE_HASH_DRIFT:" + relative)
    return data


def inputs() -> dict[str, Any]:
    data: dict[str, Any] = {"sources": {}, "records": {}, "shas": {}}
    for rel, digest in SOURCE.items():
        raw = read_verified(rel, digest)
        data["sources"][rel] = raw.decode("utf-8")
        data["shas"][rel] = digest
    for key, (rel, digest) in RECORDS.items():
        raw = read_verified(rel, digest)
        data["records"][key] = json.loads(raw) if rel.endswith(".json") else raw.decode("utf-8")
        data["shas"][rel] = digest
    return data


def data_chunks(size: int) -> tuple[int, int]:
    need(type(size) is int and 8 <= size <= 53248 and size % 4 == 0,
         "IMAGE_SIZE_NOT_POSSIBLE")
    chunks = 0
    pages = set()
    offset = 0
    while offset < size:
        remaining_page = 1024 - (offset & 1023)
        count = min(48, remaining_page, size - offset)
        need(count >= 2 and (count & 1) == 0 and
             ((offset & 1023) + count) <= 1024, "DATA_CHUNK_POLICY")
        pages.add(offset // 1024)
        chunks += 1
        offset += count
    return chunks, len(pages)


def version_ok(candidate: int, floor: int, valid_app: bool) -> bool:
    need(type(candidate) is int and type(floor) is int and type(valid_app) is bool and
         0 < candidate <= 0xFFFFFFFF and 0 <= floor <= 0xFFFFFFFF,
         "INVALID_VERSION_INPUT")
    return candidate > floor if valid_app else candidate >= floor



def plan_route(kind: str, floor: int, committed: int, *,
               runtime_pid: str | None, bootloader_pid: str,
               usb_host: str, candidate_tree: str,
               rescue_sha: str, physical_go: bool = False) -> dict[str, Any]:
    """Hypothetical future-state contract, never a transport implementation.

    The function intentionally rejects a physical GO and never returns an
    executable, a device locator or a permission to mutate a target.
    """
    need(physical_go is False, "PHYSICAL_ACTION_CANNOT_BE_ADMITTED_BY_SOURCE_MODEL")
    need(kind in ("VALID_V6", "NO_VALID_APP"), "UNKNOWN_RECOVERY_BRANCH")
    need(usb_host == "UBUNTU_MACMINI", "USB_OWNER_UNKNOWN_OR_WRONG")
    need(bootloader_pid == "1209:000D", "BOOTLOADER_PID_UNKNOWN")
    need(candidate_tree == TREE, "FUTURE_SOURCE_TREE_UNBOUND")
    need(rescue_sha == V7, "PROTECTED_RESCUE_MISMATCH")
    need(type(floor) is int and type(committed) is int and
         0 <= floor <= 0xFFFFFFFF and 0 <= committed <= 0xFFFFFFFF,
         "FLOOR_INFO_UNKNOWN_OR_INVALID")
    if kind == "VALID_V6":
        need(runtime_pid == "1209:000E", "FUTURE_RUNTIME_IDENTITY_NOT_ESTABLISHED")
        need(committed == 6 and floor == 6, "VALID_V6_METADATA_UNEXPECTED")
        need(version_ok(7, floor, True), "VALID_APP_RESCUE_NOT_STRICTLY_NEWER")
        first = "GATE4_ONLY_RUNTIME_ENTER_AFTER_EXACT_000E_IDENTITY_AND_OWNER_GO"
    else:
        need(runtime_pid is None, "NO_APP_MUST_NOT_USE_RUNTIME_ENTER")
        need(committed == 0, "NO_APP_STATE_CONTRADICTED")
        need(version_ok(7, floor, False), "NO_APP_RESCUE_BELOW_FLOOR")
        first = "GATE4_ONLY_OWNER_RESET_OR_ALREADY_000D; NEVER_RUNTIME_ENTER"
    return {
        "kind": kind,
        "hypothetical_floor": floor,
        "hypothetical_committed": committed,
        "status": "SOURCE_PLAN_ONLY_NOT_PHYSICALLY_OBSERVED",
        "target_000d": "1209:000D/IF0/EP_OUT_01/EP_IN_81",
        "host": "UBUNTU_MACMINI",
        "steps": [
            first,
            "MUST_OBSERVE_EXACT_000D_SAME_PHYSICAL_OWNER; GUID_ALONE_NOT_PROOF",
            "READONLY_INFO_FLAGS_ZERO; HMAC_FLOOR_CALCULATED_IN_BOOTLOADER",
            "STOP_IF_INFO_UNKNOWN_OR_FLOOR_DIFFERS; NO_AUTOMATIC_RETRY",
            "GATE4_ONLY_AFTER_FRESH_OWNER_GO: BEGIN_HEADER_THEN_AUTHORIZE_TAG",
            "GATE4_ONLY: DATA_EVEN_PAGE_SAFE_MAX48; END_SHA_VECTOR_COMMIT_LAST",
            "GATE4_ONLY: POST_RECOVERY_000C_REV5_CONTENT_HEADER_VERSION7; NEW_000E_REQUIRES_VERSION8",
        ],
        "mutation_permitted_now": False,
        "live_info_attested": False,
        "physical_reset_executed": False,
    }


def inspect(data: dict[str, Any]) -> dict[str, Any]:
    checks = []

    def verify(code: str, condition: bool) -> None:
        need(condition is True, "G3_4B_SOURCE_REJECT:" + code)
        checks.append(code)

    src = data["sources"]
    records = data["records"]
    for path, digest in SOURCE.items():
        verify("SOURCE_PIN_" + Path(path).name,
               type(src.get(path)) is str and
               hashlib.sha256(src[path].encode("utf-8")).hexdigest().upper() == digest)

    boot = src["OS/bootloader/bootloader.c"]
    fw = src["OS/host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs"]
    client = src["OS/host/src/DeusOs.Control.Core/DeusDeviceClient.cs"]
    linux = src["OS/host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs"]
    win = src["OS/host/src/DeusOs.Control.Transport.Windows/WindowsBootloaderWinUsbDiscovery.cs"]
    protocol = src["OS/docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md"]
    verify("BOOT_NO_VALID_APP_STAYS_IN_RECOVERY",
           "if(!explicit_update && committed_version!=0u)handoff_app();" in boot and
           "explicit_update=consume_update_token();" in boot)
    verify("BOOT_INFO_READ_ONLY_FLAG_ZERO",
           "if(opcode==OP_INFO)" in boot and
           "if(flags!=0u||payload!=1u)" in boot and
           "scan_metadata();" in boot)
    verify("BOOT_HMAC_BEFORE_FLOOR",
           "hmac_header(staged_header,calc);" in boot and
           "if(!bytes_equal(calc,staged_tag,32u))return ST_AUTH_FAILED;" in boot and
           "if(cand<=version_floor)return ST_VERSION_REJECTED;" in boot and
           "if(cand<version_floor)return ST_VERSION_REJECTED;" in boot)
    verify("BOOT_DESTRUCTIVE_BOUNDARY",
           "else if(opcode==OP_AUTHORIZE)status=handle_authorize" in boot and
           "if(flags!=FLAG_DESTRUCTIVE)status=ST_BAD_FLAGS;" in boot and
           "flash_erase_page(" in boot)
    verify("HOST_READONLY_INFO_DISTINCT_FROM_UPDATE",
           "public async Task<FirmwareUpdateResponse> InfoAsync(" in fw and
           "OpcodeInfo,\n            0," in fw and
           "public async Task<FirmwareUpdateResponse> UpdateAsync(" in fw and
           "OpcodeAuthorizeHeader,\n                DestructiveFlag" in fw)
    verify("HOST_EXACT_SHA_HEADER_AND_SIZE",
           "CryptographicOperations.FixedTimeEquals(" in fw and
           "digest,\n                header.Slice(16, 32)" in fw and
           "public const uint ProductId = 0x534F4544;" in fw and
           "public const ushort TargetDeviceId = 0x0410;" in fw)
    verify("HOST_ABORT_ON_RESPONSE_MISMATCH",
           "data.ExpectedOffset != expectedOffset" in fw and
           "if (response.Status != FirmwareUpdateStatus.Ok)" in fw and
           "if (response.State != expectedState)" in fw)
    verify("HOST_TRANSFER_RETRY_SCOPE",
           "SendDataWithSingleTimeoutRetryAsync" in fw and
           "catch (DeusHostException exception)" in fw and
           "when (exception.Kind == HostErrorKind.Timeout)" in fw)
    verify("RUNTIME_ENTRY_BOUND_TO_PUBLISHED_CAPABILITY",
           "public async Task<FirmwareUpdateResponse> EnterBootloaderAsync(" in client and
           "FirmwareUpdateAccessPolicy.PublishedOnly" in client and
           "SystemCapability.FirmwareUpdate" in client)
    verify("LINUX_000D_OWNERSHIP_INTERFACE_ENDPOINTS",
           "public const ushort ProductId = 0x000D;" in linux and
           "public const int InterfaceNumber = 0;" in linux and
           "public const byte OutEndpoint = 0x01;" in linux and
           "public const byte InEndpoint = 0x81;" in linux)
    verify("WINDOWS_GUID_NOT_PHYSICAL_ATTESTATION",
           "BootloaderInterfaceGuid" in win and
           "WindowsWinUsbDiscovery" in win and
           "return ValueTask.FromResult(transport);" in win)
    verify("PROTOCOL_NO_PERSISTENT_PARTIAL_RESUME",
           "INFO -> BEGIN -> AUTHORIZE_HEADER" in protocol and
           "Persistent partial-resume semantics are explicitly absent" in protocol)

    proof = records["gate2_tree"]
    verify("EXACT_CLOSED_WORLD_TREE",
           proof.get("schema") == "DEUS_OS_GATE2_TREE_OFFLINE_RECONSTRUCTION_V1" and
           proof.get("expected", {}).get("gate2") == TREE and
           proof.get("derived", {}).get("gate2") == TREE and
           proof.get("pass", {}).get("gate2") is True and
           proof.get("pass", {}).get("gate1") is True and
           proof.get("pass", {}).get("base") is True)
    oracle = records["oracle"]
    verify("ACCEPTED_31_SOURCE_POLICY_ORACLE",
           oracle.get("schema") == "DEUS_OS_GATE3_RECOVERY_ROUTE_OFFLINE_REVIEW_V1" and
           oracle.get("tests_count") == oracle.get("tests_pass") == 31 and
           len(oracle.get("tests", [])) == 31 and
           all(x.get("pass") is True for x in oracle["tests"]) and
           not oracle.get("tests_fail") and
           oracle.get("gate4_authorized") is False)
    route = oracle["route"]
    verify("EXACT_HEADER_VERSIONS", route.get("planned_000e_signed_version") == 6 and
           route.get("signed_rescue_version") == 7 and
           route.get("post_rescue_reinstall_min_version") == 8)
    v6report = records["v6_native"]
    verify("REAL_NATIVE_V6_PACKAGE_ACCEPTED",
           "SIGNED_NORMAL_000E_V6=ACCEPTED_SCOPED" in v6report and
           "49072 B" in v6report and V6 in v6report and
           "E138DF3C564A1D38171EB0166353D445D752187621A69F483265C70BA5A378C9" in v6report)
    verify("V7_CUSTODY_INDEPENDENTLY_ACCEPTED",
           "52092 B" in v6report and V7 in v6report and
           "RESCUE_V7=PASS_UNCHANGED" in v6report)
    pair = records["pair_native"]
    verify("SIGNED_PAIR_NATIVE_OFFLINE_CHECKS",
           "8 affirmative checks PASS" in pair and V6 in pair and V7 in pair and
           "000D_REAL_DEVICE_OWNER_HANDOFF=NOT_ATTESTED" in pair)
    verify("NO_PHYSICAL_PROOF_IN_ACCEPTED_PAIR",
           "ACTUAL_000D_OPERATOR_AND_MODE_HANDOFF_NOT_ATTESTED" in pair)

    verify("V6_TRANSFER_PLAN",data_chunks(IMAGE_V6_BYTES) == (1052, 48) and
           SIGNED_V6_BYTES == IMAGE_V6_BYTES + 80)
    verify("V7_TRANSFER_PLAN",data_chunks(IMAGE_V7_BYTES) == (1117, 51) and
           SIGNED_V7_BYTES == IMAGE_V7_BYTES + 80)

    scenarios = [
        ("NORMAL_V6_FROM_FLOOR5", 6, 5, True, True),
        ("RESCUE_V7_FROM_VALID_V6", 7, 6, True, True),
        ("V6_AFTER_V7_REJECT", 6, 7, True, False),
        ("V7_EQUAL_FLOOR_VALID_REJECT", 7, 7, True, False),
        ("NO_APP_V7_EQUAL_FLOOR_ACCEPT", 7, 7, False, True),
        ("NO_APP_V7_BELOW_FLOOR8_REJECT", 7, 8, False, False),
        ("NO_APP_V7_FROM_FLOOR5_ACCEPT", 7, 5, False, True),
        ("NEXT_NORMAL_AFTER_V7_MIN8", 8, 7, True, True),
    ]
    for name, candidate, floor, valid_app, expected in scenarios:
        verify("VERSION_" + name, version_ok(candidate, floor, valid_app) is expected)

    return {
        "schema": "DEUS_OS_GATE3_G3_4B_SOURCE_READINESS_PRE_ADJUDICATION_V1",
        "result": "PASS_SOURCE_AND_PREVIOUS_EVIDENCE_RECONCILIATION_ONLY",
        "gate3_compound_G3_4b": "OPEN_PENDING_INDEPENDENT_NATIVE_ROUTE_ATTESTATION",
        "full_gate3": "NOT_ACCEPTED",
        "gate4": "NOT_AUTHORIZED",
        "source_and_evidence_checks_passed": len(checks),
        "checks": checks,
        "candidate_tree": TREE,
        "signed_normal_v6": {"bytes": SIGNED_V6_BYTES, "sha256": V6,
                             "custody": "PREVIOUS_OWNER_WINDOWS_NATIVE_ACCEPTED_NOT_REREAD"},
        "signed_rescue_v7": {"bytes": SIGNED_V7_BYTES, "sha256": V7,
                             "custody": "PREVIOUS_OWNER_WINDOWS_NATIVE_ACCEPTED_NOT_REREAD"},
        "route_a": plan_route("VALID_V6", 6, 6, runtime_pid="1209:000E",
                              bootloader_pid="1209:000D", usb_host="UBUNTU_MACMINI",
                              candidate_tree=TREE, rescue_sha=V7),
        "route_b": plan_route("NO_VALID_APP", 7, 0, runtime_pid=None,
                              bootloader_pid="1209:000D", usb_host="UBUNTU_MACMINI",
                              candidate_tree=TREE, rescue_sha=V7),
        "usb_owner": "UBUNTU_MACMINI_EXPECTED; NOT_OBSERVED_IN_THIS_RUN",
        "bootloader_info": "READ_ONLY_POLICY_PROVEN_BY_SOURCE; NOT_OBSERVED_IN_THIS_RUN",
        "transport_update": "DESTRUCTIVE_BEGIN_AUTHORIZE_DATA_END_SOURCE_ONLY; NOT_EXECUTED",
        "unresolved": [
            "INDEPENDENT_WINDOWS_NATIVE_ORIGINAL_EVIDENCE_FOR_ONE_INTEGRATED_NONDESTRUCTIVE_ROUTE_OWNER",
            "REVIEWED_OPERATOR_HANDOFF_FOR_BOTH_VALID_AND_INVALID_APPLICATION_PATHS",
            "PHYSICAL_DEVICE_IDENTITY_NOT_ATTESTED_AND_NOT_REQUIRED_AS_LIVE_G3_RECOVERY_DRILL",
        ],
        "device_io": False, "ssh": False, "reset": False, "flash": False,
    }


def selftest(data: dict[str, Any]) -> int:
    count = 0
    base = inspect(data)
    need(base["source_and_evidence_checks_passed"] >= 28, "BASE_COUNT_TOO_SMALL")
    print("G3_4B_TEST=PASS CASE=EXACT_SOURCE_AND_EVIDENCE")
    count += 1

    for p in SOURCE:
        broken = copy.deepcopy(data)
        broken["sources"][p] += "\n#TAMPER\n"
        try: inspect(broken)
        except ProofReject: pass
        else: raise AssertionError("SOURCE_TAMPER_ACCEPTED:" + p)
        count += 1
        print("G3_4B_TEST=PASS CASE=REJECT_TAMPER_" + Path(p).stem)

    for name, change in (
        ("WRONG_TREE", lambda v: v["records"]["gate2_tree"]["derived"].update(gate2="0"*40)),
        ("ORACLE_TEST_FAIL", lambda v: v["records"]["oracle"]["tests"][0].update({"pass": False})),
        ("ORACLE_COUNT_DRIFT", lambda v: v["records"]["oracle"].update(tests_pass=30)),
        ("NORMAL_VERSION_DRIFT", lambda v: v["records"]["oracle"]["route"].update(planned_000e_signed_version=5)),
        ("RESCUE_VERSION_DRIFT", lambda v: v["records"]["oracle"]["route"].update(signed_rescue_version=6)),
        ("V6_SHA_PROOF_MISSING", lambda v: v["records"].update(v6_native=v["records"]["v6_native"].replace(V6,"0"*64))),
        ("V7_SHA_PROOF_MISSING", lambda v: v["records"].update(pair_native=v["records"]["pair_native"].replace(V7,"0"*64))),
        ("PAIR_NOT_ATTESTED_BOUNDARY_MISSING", lambda v: v["records"].update(pair_native=v["records"]["pair_native"].replace("000D_REAL_DEVICE_OWNER_HANDOFF=NOT_ATTESTED","000D_REAL_DEVICE_OWNER_HANDOFF=PASS"))),
    ):
        modified=copy.deepcopy(data)
        change(modified)
        try: inspect(modified)
        except ProofReject: pass
        else: raise AssertionError("REJECT_CASE_ACCEPTED:"+name)
        count += 1
        print("G3_4B_TEST=PASS CASE="+name)

    for candidate, floor, valid in ((6,7,True),(7,7,True),(7,8,False),(6,5,True)):
        count += 1
        expected=(candidate==6 and floor==5 and valid)
        need(version_ok(candidate,floor,valid) == expected,"FLOOR_MATRIX_DRIFT")
        print("G3_4B_TEST=PASS CASE=VERSION_FLOOR_MATRIX_"+str(count))
    for n in (1,2,47,49,53250):
        try: data_chunks(n)
        except ProofReject: pass
        else: raise AssertionError("INVALID_IMAGE_LENGTH_ACCEPTED:"+str(n))
        count += 1
        print("G3_4B_TEST=PASS CASE=CHUNK_INVALID_"+str(n))
    good = dict(bootloader_pid="1209:000D", usb_host="UBUNTU_MACMINI",
                candidate_tree=TREE, rescue_sha=V7)
    need(plan_route("VALID_V6",6,6,runtime_pid="1209:000E",**good)["mutation_permitted_now"] is False,
         "VALID_ROUTE_MUTATION_LEAK")
    count += 1
    print("G3_4B_TEST=PASS CASE=VALID_V6_ROUTE_SOURCE_ONLY")
    need(plan_route("NO_VALID_APP",7,0,runtime_pid=None,**good)["live_info_attested"] is False,
         "NO_APP_ROUTE_LIVE_CLAIM")
    count += 1
    print("G3_4B_TEST=PASS CASE=NO_APP_ROUTE_SOURCE_ONLY")
    for name, kind, floor, committed, extra in (
        ("WRONG_USB_HOST","VALID_V6",6,6,{"usb_host":"WINDOWS"}),
        ("WRONG_BOOTLOADER_PID","VALID_V6",6,6,{"bootloader_pid":"1209:000C"}),
        ("WRONG_CANDIDATE_TREE","VALID_V6",6,6,{"candidate_tree":"0"*40}),
        ("WRONG_RESCUE_SHA","VALID_V6",6,6,{"rescue_sha":"0"*64}),
        ("UNAUTHORIZED_PHYSICAL_GO","VALID_V6",6,6,{"physical_go":True}),
        ("FLOOR_UNKNOWN","VALID_V6",-1,6,{}),
        ("VALID_V6_WRONG_RUNTIME","VALID_V6",6,6,{"runtime_pid":"1209:000C"}),
        ("VALID_V6_WRONG_COMMITTED","VALID_V6",6,5,{}),
        ("VALID_V6_FLOOR7","VALID_V6",7,6,{}),
        ("NO_APP_USING_RUNTIME_ENTER","NO_VALID_APP",7,0,{"runtime_pid":"1209:000E"}),
        ("NO_APP_HAS_COMMITTED_APP","NO_VALID_APP",7,6,{}),
        ("NO_APP_VERSION8_FLOOR","NO_VALID_APP",8,0,{}),
        ("UNKNOWN_MODE","UNKNOWN",7,0,{})
    ):
        attrs=good.copy()
        runtime=extra.pop("runtime_pid","1209:000E" if kind=="VALID_V6" else None)
        attrs.update(extra)
        try: plan_route(kind,floor,committed,runtime_pid=runtime,**attrs)
        except ProofReject: pass
        else: raise AssertionError("ROUTE_STOP_BYPASS:"+name)
        count += 1
        print("G3_4B_TEST=PASS CASE=REJECT_ROUTE_"+name)
    print(f"G3_4B_SOURCE_PRE_ADJUDICATION_SELFTEST=PASS COUNT={count} USB=NO SSH=NO RESET=NO FLASH=NO")
    return count


def main() -> int:
    parser=argparse.ArgumentParser()
    parser.add_argument("--selftest",action="store_true")
    args=parser.parse_args()
    try:
        data=inputs()
        if args.selftest:
            selftest(data)
        else:
            print(json.dumps(inspect(data),sort_keys=True,indent=2))
        return 0
    except (ProofReject, AssertionError, KeyError, TypeError, ValueError) as exc:
        print("G3_4B_SOURCE_PRE_ADJUDICATION=FAIL " + type(exc).__name__ + " " + str(exc))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
