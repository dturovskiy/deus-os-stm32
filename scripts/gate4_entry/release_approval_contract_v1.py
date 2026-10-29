#!/usr/bin/env python3
"""A-05 / RA-01 source-only canonical approval statement validator.

NO signature verification, nonce consumption, admission, USB, SSH, reset, Flash,
operator ZIP construction or hardware execution. PASS means grammar/binding ONLY.
"""
from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
import hashlib
import json
import re
from typing import Any

SCHEMA = "DEUS_OS_GATE4_HARDWARE_RELEASE_APPROVAL_V1"
NAMESPACE = "deus-os-gate4-hardware-release-v1"
PROFILE = "HARDWARE_MUTATING"
BUNDLE_SCHEMA = "DEUS_OS_OPERATOR_BUNDLE_V2"
WORKFLOW = "gate4_enter_info_once"
PKG_ID = "stm32_os_gate4_enter_info_once_v1"
OWNER = "deus-os-hardware-owner"
SIGNER_FINGERPRINT = "SHA256:4Ze7JMXNBsGEmhG/j+VRm9+X+a/C3dVlLSk59MU440M"
NORMAL_SHA = "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
RESCUE_SHA = "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A"
INSTALLED_TREE = "ccbd50200ec7f0ec53da6b1048d7377271dfb7bd"
FUTURE_TREE = "081aac70c31909bf681254393e2288917cb9b8b2"
HEX64 = re.compile(r"[A-F0-9]{64}\Z")
HEX40 = re.compile(r"[a-f0-9]{40}\Z")
HEX32 = re.compile(r"[a-f0-9]{32}\Z")
PORT = re.compile(r"usb:[0-9]{3}:[0-9]{1,3}\Z")
REQUIRED = frozenset({
    "schema", "workflow_id", "package_id", "bundle_schema", "operator_profile",
    "operator_zip_sha256", "operator_zip_bytes", "source_lock_sha256",
    "source_git_tree", "build_provenance_sha256", "entrypoint_sha256",
    "owner_identity", "owner_signer_fingerprint", "signing_namespace",
    "valid_from_utc", "valid_until_utc", "nonce", "host_identity",
    "device_usb_port", "expected_old_vid_pid", "expected_old_source_tree",
    "expected_bootloader_vid_pid", "signed_normal_v6_sha256",
    "signed_rescue_v7_sha256", "allowed_enter_count", "allowed_info_count",
    "allowed_usb_node_fix_count", "flash_allowed", "erase_allowed",
    "retry_allowed", "automatic_return_allowed", "readiness_evidence_sha256",
})

class Reject(ValueError):
    pass

def need(ok: bool, reason: str) -> None:
    if not ok:
        raise Reject(reason)

def sha(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest().upper()

def unique(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    out: dict[str, Any] = {}
    for k, v in pairs:
        need(k not in out, "DUPLICATE_JSON_FIELD")
        out[k] = v
    return out

def canonical_bytes(obj: dict[str, Any]) -> bytes:
    return (json.dumps(obj, sort_keys=True, separators=(",", ":"),
                       ensure_ascii=True, allow_nan=False) + "\n").encode("ascii")

def parse_canonical(blob: bytes) -> dict[str, Any]:
    need(type(blob) is bytes and 0 < len(blob) <= 4096, "APPROVAL_SIZE")
    need(b"\x00" not in blob and not blob.startswith(b"\xef\xbb\xbf"), "APPROVAL_ENCODING")
    try:
        obj = json.loads(blob.decode("utf-8", "strict"), object_pairs_hook=unique,
                         parse_constant=lambda _: (_ for _ in ()).throw(Reject("NONFINITE_JSON")))
    except (UnicodeError, json.JSONDecodeError, TypeError) as ex:
        raise Reject("APPROVAL_JSON_INVALID") from ex
    need(type(obj) is dict and set(obj) == REQUIRED, "APPROVAL_FIELDS")
    need(blob == canonical_bytes(obj), "APPROVAL_NOT_CANONICAL")
    return obj

def utc_time(value: Any, code: str) -> datetime:
    need(type(value) is str and re.fullmatch(
        r"20[0-9]{2}-[01][0-9]-[0-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9]Z", value)
        is not None, code + "_FORMAT")
    try:
        return datetime.strptime(value, "%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
    except ValueError as ex:
        raise Reject(code + "_INVALID") from ex

@dataclass(frozen=True)
class TrustedMaterial:
    """Externally verified pins; never self-derived from the unsigned statement."""
    zip_sha256: str
    zip_bytes: int
    source_lock_sha256: str
    source_git_tree: str
    build_provenance_sha256: str
    entrypoint_sha256: str
    readiness_evidence_sha256: str
    device_usb_port: str
    host_identity: str = "macmini"

    def verify(self) -> None:
        for f in ("zip_sha256", "source_lock_sha256", "build_provenance_sha256",
                  "entrypoint_sha256", "readiness_evidence_sha256"):
            val = getattr(self, f)
            need(type(val) is str and HEX64.fullmatch(val) is not None,
                 "TRUST_" + f.upper())
        need(type(self.zip_bytes) is int and 0 < self.zip_bytes <= 30_000_000,
             "TRUST_ZIP_SIZE")
        need(type(self.source_git_tree) is str and HEX40.fullmatch(self.source_git_tree)
             is not None, "TRUST_TREE")
        need(type(self.device_usb_port) is str and PORT.fullmatch(self.device_usb_port)
             is not None, "TRUST_PORT")
        need(self.host_identity == "macmini", "TRUST_HOST")

def inspect_source_only(blob: bytes, expected: TrustedMaterial,
                        now: datetime) -> dict[str, str]:
    """PASS only means byte-format validation; NEVER grants hardware release."""
    expected.verify()
    need(isinstance(now, datetime) and now.tzinfo is not None and
         now.utcoffset() == timedelta(0), "UTC_CLOCK_REQUIRED")
    x = parse_canonical(blob)
    constants = {
        "schema": SCHEMA, "workflow_id": WORKFLOW, "package_id": PKG_ID,
        "bundle_schema": BUNDLE_SCHEMA, "operator_profile": PROFILE,
        "owner_identity": OWNER, "owner_signer_fingerprint": SIGNER_FINGERPRINT,
        "signing_namespace": NAMESPACE, "host_identity": expected.host_identity,
        "device_usb_port": expected.device_usb_port,
        "expected_old_vid_pid": "1209:000C",
        "expected_old_source_tree": INSTALLED_TREE,
        "expected_bootloader_vid_pid": "1209:000D",
        "source_git_tree": expected.source_git_tree,
        "operator_zip_sha256": expected.zip_sha256,
        "source_lock_sha256": expected.source_lock_sha256,
        "build_provenance_sha256": expected.build_provenance_sha256,
        "entrypoint_sha256": expected.entrypoint_sha256,
        "readiness_evidence_sha256": expected.readiness_evidence_sha256,
        "signed_normal_v6_sha256": NORMAL_SHA,
        "signed_rescue_v7_sha256": RESCUE_SHA,
    }
    for key, value in constants.items():
        need(type(x[key]) is str and x[key] == value, "BIND_" + key.upper())
    need(expected.source_git_tree == FUTURE_TREE, "SOURCE_TREE_NOT_APPROVED")
    need(type(x["operator_zip_bytes"]) is int and
         x["operator_zip_bytes"] == expected.zip_bytes, "BIND_ZIP_SIZE")
    need(type(x["nonce"]) is str and HEX32.fullmatch(x["nonce"]) is not None and
         x["nonce"] != "0" * 32, "BIND_NONCE")
    for key in ("allowed_enter_count", "allowed_info_count", "allowed_usb_node_fix_count"):
        need(type(x[key]) is int and x[key] == 1, "LIMIT_" + key.upper())
    for key in ("flash_allowed", "erase_allowed", "retry_allowed", "automatic_return_allowed"):
        need(x[key] is False, "FORBIDDEN_" + key.upper())
    start = utc_time(x["valid_from_utc"], "START")
    end = utc_time(x["valid_until_utc"], "END")
    need(timedelta(0) < end-start <= timedelta(minutes=20), "INVALID_DURATION")
    need(start <= now <= end and end <= now+timedelta(minutes=20),
         "APPROVAL_EXPIRED_OR_FUTURE")
    return {"result": "PASS", "classification": "RA01_CANONICAL_SOURCE_CONTRACT_ONLY",
            "statement_sha256": sha(blob), "zip_sha256": expected.zip_sha256,
            "authorization": "DENIED", "signature": "NOT_VERIFIED",
            "nonce_consumed": "NO", "physical_executor": "NOT_ADMITTED",
            "device_io": "NONE"}
