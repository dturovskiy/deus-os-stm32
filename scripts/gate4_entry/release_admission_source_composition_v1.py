#!/usr/bin/env python3
"""Gate4 A-05 source-only composition of independently accepted RA-01/02/03.

Verifies a hypothetical exact ZIP-bound approval, detached SSHSIG using an
independently pinned owner trust anchor, and immutable *original* Windows RA-03
acceptance evidence. Never writes a nonce, grants release, launches another
process, accesses an owner private key, or interacts with a device.
"""
from __future__ import annotations

from datetime import datetime
import hashlib
import json
from pathlib import Path
import re
import zipfile

import release_approval_contract_v1 as ra01
import verify_release_signature_v1 as ra02

RA03_NAME = (
    "stm32_os_gate4_a05_ra03_nonce_ledger_native_offline_v3_20261028_"
    "20261028_193409_3612d1a821494539bb92d6dd60dff1c0.evidence.zip"
)
RA03_SHA = "80F3521AF3B738F2EBA58AD8460B05709609CEE218459147C5E5965366D0A9D5"
RA03_MEMBERS = frozenset({
    "outcome.txt", "native_test_output.txt", "native_checks.txt",
    "run.log", "sizes.txt", "hashes.sha256",
})
RA03_PINS = {
    "ra03_module_sha256": "D67EB9820C016902FCDBC7FD3A13A51AE3F1317C33927E60B49D6B5565C351BC",
    "ra03_tests_sha256": "049E81F4078AA53DC4C7405A0A5253108D8C3C63DD9FF33746577FAA1A084965",
    "ra03_child_sha256": "DF9B7AFA3FDDA926EA669A7FE40EB52E24F9F0F1259E0A56331259E7126F9719",
}

class Reject(ra01.Reject):
    pass

def need(ok, reason):
    if not ok:
        raise Reject(reason)

def digest(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest().upper()

def attest_native_ra03(root: Path | None = None, expected_sha: str = RA03_SHA) -> dict[str, object]:
    """Read-only accepted owner Windows receipt verification; no native rerun."""
    if root is None:
        root = Path(__file__).resolve().parents[3]
    p = Path(root) / "Evidence" / RA03_NAME
    need(p.is_file() and not p.is_symlink(), "RA03_ORIGINAL_ABSENT")
    blob = p.read_bytes()
    need(len(blob) == 2292 and digest(blob) == expected_sha == RA03_SHA,
         "RA03_ORIGINAL_WHOLE_SHA")
    with zipfile.ZipFile(p) as z:
        infos = z.infolist()
        names = [v.filename for v in infos]
        need(len(names) == 6 and len(set(names)) == 6 and
             set(names) == RA03_MEMBERS and all(not v.is_dir() and
             v.file_size < 100_000 and not(v.flag_bits & 1) for v in infos),
             "RA03_MEMBERS")
        need(z.testzip() is None, "RA03_CRC")
        data = {n: z.read(n) for n in names}
    sha_rows = data["hashes.sha256"].decode("ascii").splitlines()
    size_rows = data["sizes.txt"].decode("ascii").splitlines()
    need(len(sha_rows) == 5 and len(size_rows) == 4, "RA03_INDEX_COUNT")
    seen = set()
    for row in sha_rows:
        m = re.fullmatch(r"([0-9A-Fa-f]{64})  ([a-z0-9_.-]+)", row)
        need(m and m[2] in RA03_MEMBERS - {"hashes.sha256"} and
             m[2] not in seen, "RA03_INDEX_ROW")
        need(digest(data[m[2]]) == m[1].upper(), "RA03_INDEX_SHA")
        seen.add(m[2])
    need(seen == RA03_MEMBERS - {"hashes.sha256"}, "RA03_INDEX_COVERAGE")
    seen.clear()
    for row in size_rows:
        m = re.fullmatch(r"(\d+)  ([a-z0-9_.-]+)", row)
        need(m and m[2] in RA03_MEMBERS - {"hashes.sha256", "sizes.txt"} and
             m[2] not in seen, "RA03_SIZE_ROW")
        need(len(data[m[2]]) == int(m[1]), "RA03_SIZE_VALUE")
        seen.add(m[2])
    need(seen == RA03_MEMBERS - {"hashes.sha256", "sizes.txt"},
         "RA03_SIZE_COVERAGE")
    obj = json.loads(data["outcome.txt"].decode("utf-8", "strict"))
    for key, val in {
        "result": "PASS",
        "gate": "stm32_os_gate4_a05_ra03_nonce_ledger_native_offline_v3_20261028",
        "classification": "G4_RA03_WINDOWS_NATIVE_NONCE_LEDGER_SCOPED_PASS",
        "child_exit_code": "0",
        "source_checks": 13,
        "nonce_ledger_installed": "NO",
        "controller_installed": "NO",
        "physical_release": "DENIED",
        "gate4": "NOT_AUTHORIZED",
    }.items():
        need(obj.get(key) == val, "RA03_TYPED_" + key.upper())
    for key, val in RA03_PINS.items():
        need(obj.get(key) == val, "RA03_SOURCE_PIN_" + key.upper())
    need(obj.get("git_pre_sha256") == obj.get("git_post_sha256") and
         re.fullmatch(r"[0-9A-F]{64}",obj.get("git_pre_sha256","")) is not None,
         "RA03_GIT_DRIFT")
    transcript = data["native_test_output.txt"].decode("utf-8", "strict").splitlines()
    markers = [x for x in transcript if x.startswith("RA03_NATIVE_WINDOWS_CASE=PASS ")]
    need(len(markers) == len(set(markers)) == 24 and
         "RA03_NATIVE_WINDOWS_CASE=PASS TWO_PROCESS_CREATE_NEW_ONE_WINNER_ONE_STOP" in markers and
         "RA03_NATIVE_WINDOWS_CASE=PASS UNEXPECTED_EVERYONE_ACE" in markers and
         "RA03_NATIVE_WINDOWS_CASE=PASS REPARSE_JUNCTION_REJECTED" in markers and
         "RA03_OFFLINE_WINDOWS=PASS COUNT=24 USB=NO SSH=NO RESET=NO FLASH=NO INSTALL=NO AUTHORIZATION=DENIED" in transcript,
         "RA03_NATIVE_TESTS_NOT_24")
    return {"result": "PASS", "ra03_evidence_sha256": RA03_SHA,
            "native_cases": 24, "nonce_ledger_installed": "NO"}

def inspect_source_composition(
    statement: bytes,
    signature: bytes,
    candidate_zip: Path,
    material: ra01.TrustedMaterial,
    now: datetime,
    *,
    bundle_root: Path,
    workspace_root: Path | None = None,
    _unit_test_public_key: bytes | None = None,
) -> dict[str, object]:
    """No release/nonce-authorizing branch, even for fully verified inputs.

    _unit_test_public_key exists only to exercise Ed25519 with an ephemeral
    in-RAM unit-test key. The normal path *always* uses the pinned real owner
    public key from original independently authenticated Evidence.
    """
    material.verify()
    root = Path(bundle_root)
    candidate = Path(candidate_zip)
    need(root.is_dir() and not root.is_symlink(), "BUNDLE_ROOT")
    need(candidate.parent == root and candidate.is_file() and
         not candidate.is_symlink(), "CANDIDATE_ROOT_OR_LINK")
    need(candidate.stat().st_size == material.zip_bytes, "CANDIDATE_SIZE")
    need(digest(candidate.read_bytes()) == material.zip_sha256, "CANDIDATE_WHOLE_SHA")
    contract_result = ra01.inspect_source_only(statement, material, now)
    trusted_key = (_unit_test_public_key if _unit_test_public_key is not None
                   else ra02.pinned_trust_from_original_evidence(workspace_root))
    ra02.verify_crypto(statement, signature, trusted_key)
    ra03_result = attest_native_ra03(workspace_root)
    need(ra03_result["nonce_ledger_installed"] == "NO", "UNEXPECTED_RA03_DEPLOYMENT")
    return {
        "result": "PASS",
        "classification": "G4_A05_RA01_RA02_RA03_SOURCE_COMPOSITION_ONLY",
        "statement_sha256": contract_result["statement_sha256"],
        "candidate_zip_sha256": material.zip_sha256,
        "signer": "ED25519_VERIFIED",
        "ra03_original_evidence_sha256": RA03_SHA,
        "ra03_native_cases": 24,
        "authorization": "DENIED",
        "nonce_consumed": "NO",
        "trusted_controller": "NOT_INSTALLED",
        "release_admitted": "NO",
        "device_io": "NONE",
        "gate4": "NOT_AUTHORIZED",
    }

def exercise_nonce_sequence_source_only(
    statement: bytes,
    signature: bytes,
    candidate_zip: Path,
    material: ra01.TrustedMaterial,
    now: datetime,
    *,
    bundle_root: Path,
    source_only_nonce_writer,
    workspace_root: Path | None = None,
    _unit_test_public_key: bytes | None = None,
) -> dict[str, object]:
    """Test *ordering* against an injected sandbox-only RA03-like sink.

    THIS FUNCTION IS NOT AN OWNER RELEASE CONTROLLER. The injected sink is not
    authenticated/installed here and a synthetic key may be accepted in unit
    tests. No operator launch, signature issuance or hardware authorization.
    There is exactly ONE sink call after verification, never a retry.
    """
    verified = inspect_source_composition(
        statement, signature, candidate_zip, material, now,
        bundle_root=bundle_root, workspace_root=workspace_root,
        _unit_test_public_key=_unit_test_public_key,
    )
    nonce = ra01.parse_canonical(statement)["nonce"]
    try:
        recorded = source_only_nonce_writer(
            nonce, verified["statement_sha256"], verified["candidate_zip_sha256"]
        )
    except Exception as ex:
        raise Reject("RA03_ONCE_ATTEMPTED_STOP_NO_RETRY") from ex
    need(type(recorded) is dict and
         recorded.get("classification") == "RA03_DURABLE_SOURCE_ONLY_NONCE_CONSUMED" and
         recorded.get("nonce") == nonce and
         recorded.get("authorization") == "DENIED" and
         recorded.get("launch_authorized") is False and
         recorded.get("device_io") == "NONE" and
         re.fullmatch(r"[0-9A-F]{64}", recorded.get("record_sha256","")) is not None,
         "RA03_RECEIPT_AFTER_ONCE_UNTRUSTED_STOP_NO_RETRY")
    return {
        **verified,
        "classification": "G4_A05_SOURCE_NONCE_ORDER_TEST_ONLY",
        "nonce_consumed": "YES_IN_TEST_SINK_ONLY",
        "record_sha256": recorded["record_sha256"],
        "authorization": "DENIED",
        "release_admitted": "NO",
        "trusted_controller": "NOT_INSTALLED",
        "device_io": "NONE",
        "gate4": "NOT_AUTHORIZED",
    }
