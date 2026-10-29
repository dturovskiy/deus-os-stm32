#!/usr/bin/env python3
"""Gate3 metadata-only, source-bound candidate inspection.

Reads exact source text and protected package headers/digests. Does NOT validate
production HMAC, invoke a signer, open USB, reset a target, or release an operator.
"""
from __future__ import annotations

import hashlib
import json
import stat
import struct
from pathlib import Path

OS = Path(__file__).resolve().parents[2]
STORE = OS.parent / "ProtectedRecovery"
SOURCES = {
    "host/src/DeusOs.Control.Recovery.ReadOnly/ReadOnlyRecoveryInfoProbe.cs":
        "528B5DD97BC35051838A842DE4A4FFFC2E86D2F2F7D38BF5DCAE59CC2D9F2E50",
    "host/src/DeusOs.Control.Recovery.ReadOnly/RecoveryRouteReadiness.cs":
        "26B944FC9CA57D8B9F64F593C325583F124B0AAE24B731B7BC48B87881A70B10",
    "host/src/DeusOs.Control.Recovery.ReadOnly/RecoveryRouteCoordinator.cs":
        "3F4E5D326AB150B80858F9D85194EA317C340C63CF0BE21369159BA2796681AA",
    "host/tests/DeusOs.Control.Recovery.Acceptance/Program.cs":
        "3398D1C24526AEF31019BA8CB15E7BFAC56C2C2E298D2A22DF97669ABCD70D67",
    "host/tests/DeusOs.Control.Recovery.Acceptance/CoordinatorTests.cs":
        "83CB90CE84C33C71BDDF34AF922B502777ADAF9832D81624583E51BCB8BEE4A3",
}
PACKAGES = {
    "deus_os_000e_management_only_tree081aac70_signed_v6.pkg":
        (49072, "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED",
         6, 48992, "1F6966788923496170F47FED9AC24311BEA177B53B3045C2B89CA6305E1B56E1"),
    "deus_os_rev5_000c_recovery_future_floor7_v1.pkg":
        (52092, "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A",
         7, 52012, "AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210"),
}


def require(condition: bool, code: str) -> None:
    if not condition:
        raise ValueError(code)


def regular_bytes(path: Path, maximum: int) -> bytes:
    require(not path.is_symlink(), "SOURCE_SYMLINK_NOT_ALLOWED")
    meta = path.lstat()
    require(stat.S_ISREG(meta.st_mode) and 0 < meta.st_size <= maximum, "FILE_TYPE_OR_SIZE")
    return path.read_bytes()


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest().upper()


def check_package(data: bytes, *, total: int, digest: str,
                  version: int, image_bytes: int, image_sha: str) -> None:
    require(len(data) == total and sha256(data) == digest, "WHOLE_PACKAGE_MISMATCH")
    require(total == 80 + image_bytes, "PACKAGE_SIZE_MISMATCH")
    require(len(data) >= 88, "PACKAGE_TRUNCATED")
    product, fmt, flags, target, length, revision = struct.unpack_from("<IBBHII", data, 0)
    require((product, fmt, flags, target, length, revision)
            == (0x534F4544, 1, 0, 0x0410, image_bytes, version),
            "PACKAGE_HEADER_MISMATCH")
    require(8 <= length <= 53248 and length % 4 == 0, "BAD_IMAGE_LENGTH")
    payload_digest = sha256(data[80:])
    require(payload_digest == image_sha and
            data[16:48].hex().upper() == image_sha, "IMAGE_DIGEST_MISMATCH")


def check_host_integration() -> None:
    """Source-shape guard only; it is not Windows AST/.NET execution evidence."""
    solution = regular_bytes(OS / "host/DeusOs.Control.sln", 20000).decode("utf-8")
    cli_project = regular_bytes(
        OS / "host/src/DeusOs.Control.Cli/DeusOs.Control.Cli.csproj", 5000
    ).decode("utf-8")
    cli_source = regular_bytes(
        OS / "host/src/DeusOs.Control.Cli/Program.cs", 30000
    ).decode("utf-8")
    paths = (
        "DeusOs.Control.Recovery.ReadOnly\\DeusOs.Control.Recovery.ReadOnly.csproj",
        "DeusOs.Control.Recovery.Acceptance\\DeusOs.Control.Recovery.Acceptance.csproj",
    )
    for item in paths:
        require(solution.count(item) == 1, "SOLUTION_PROJECT_NOT_UNIQUE")
    for guid in ("AE35C55B-5385-42A0-8CE3-AFBBD6081014",
                 "284886F9-21BA-42E7-A06D-8E90229ACAA2"):
        require(solution.count(guid) == 5, "SOLUTION_CONFIGURATION_DRIFT")
    require(cli_project.count(
        '../DeusOs.Control.Recovery.ReadOnly/DeusOs.Control.Recovery.ReadOnly.csproj'
    ) == 1, "CLI_PROJECT_REFERENCE_MISSING")
    parse_at = cli_source.index("var parsed = CliArguments.Parse(args);")
    contract_at = cli_source.index('if (parsed.Command == "recovery-contract")')
    discovery_at = cli_source.index("var discovery = CreateDiscovery();")
    require(parse_at < contract_at < discovery_at, "OFFLINE_USB_BYPASS_ORDER_INVALID")
    require('if (remaining[0] == "recovery")' in cli_source
            and "remaining.Count != 2" in cli_source
            and 'remaining[1], "contract"' in cli_source
            and "device is not null" in cli_source,
            "OFFLINE_ARGUMENTS_NOT_FAIL_CLOSED")
    body = cli_source.split("private static void PrintRecoveryContract()", 1)[1].split(
        "private static IDeviceDiscovery CreateDiscovery()", 1
    )[0]
    for forbidden in ("DiscoverAsync(", "OpenAsync(", "EnterBootloaderAsync(",
                      "UpdateAsync(", "InfoAsync(", "File.", "Directory.",
                      "Process.", "sudo", "ssh"):
        require(forbidden not in body, "OFFLINE_COMMAND_CONTAINS_IO")
    for token in ("RECOVERY_CONTRACT=SOURCE_PINS_ONLY",
                  "LIVE_USB=NOT_QUERIED",
                  "DESTRUCTIVE_OPERATION_AUTHORIZED=NO",
                  "GATE3_AUTO_ACCEPTANCE=NO"):
        require(token in body, "OFFLINE_CONTRACT_OUTPUT_DRIFT")


def main() -> None:
    check_host_integration()
    for relative, wanted in SOURCES.items():
        data = regular_bytes(OS / relative, 50000)
        require(sha256(data) == wanted, "SOURCE_DRIFT_" + Path(relative).name)
    library = OS / "host/src/DeusOs.Control.Recovery.ReadOnly"
    code = b"\n".join((library / part).read_bytes()
                      for part in ("ReadOnlyRecoveryInfoProbe.cs",
                                   "RecoveryRouteReadiness.cs",
                                   "RecoveryRouteCoordinator.cs"))
    for forbidden in (b"EnterBootloaderAsync(", b"UpdateAsync(",
                      b"OpcodeAuthorizeHeader", b"OpcodeData", b"OpcodeEnd"):
        require(forbidden not in code, "DESTRUCTIVE_CALL_SURFACE_DETECTED")
    for name, (length, digest, rev, payload_length, payload_sha) in PACKAGES.items():
        raw = regular_bytes(STORE / name, 60000)
        check_package(raw, total=length, digest=digest, version=rev,
                      image_bytes=payload_length, image_sha=payload_sha)
    print(json.dumps({
        "result": "PASS",
        "classification": "GATE3_READONLY_HOST_SOURCE_AND_SIGNED_PAIR_METADATA_STATIC_ONLY",
        "source_sha256": "5/5",
        "solution_and_cli_contract_static": "PASS",
        "protected_signed_pair_whole_sha256": "2/2",
        "protected_signed_pair_header_and_payload_sha256": "2/2",
        "production_hmac_reverified": False,
        "dotnet_compiled_in_this_check": False,
        "live_usb_info": False,
        "device_reset": False,
        "flash_or_erase": False,
        "source_candidate_released": False,
        "full_gate3": "NOT_ACCEPTED",
        "gate4": "NOT_AUTHORIZED",
    }, sort_keys=True))


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as exc:
        print(json.dumps({"result": "FAIL", "classification": "STATIC_ADMISSION_STOP",
                          "error": str(exc)[:96]}))
        raise SystemExit(2)
