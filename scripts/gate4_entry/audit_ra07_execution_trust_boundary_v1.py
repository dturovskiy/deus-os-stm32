#!/usr/bin/env python3
"""READ-ONLY source/custody audit of the currently accepted runner admission boundary.

This script is NOT an executable approval controller, authorizer, profile gate,
signer, USB/SSH operator or runner patch. It detects a specific source-level
gap and *fails closed* if the inspected source is no longer the exact known
v1.3.2 byte identity. It never creates release permission or consumes nonce.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re

REPO = Path(__file__).resolve().parents[2]
ROOT = REPO.parent  # Off-Git owner workspace (optional in public CI checkout).
RUNNER = (ROOT / "Bundle" / "_stage_deus_run_preflight_evidence_v132_source_20261028"
          / "deus-run-next.ps1")
RUNNER_SHA = "60C090E8324EA1ECD2ECCF7114E26A6793402AA44B736CA507BB45321DD6EC75"
BUILDER = REPO / "scripts/operator_bundle_guard/bundle_guard.py"
BUILDER_SHA = "F706E51F231F663138E41533E288A7C856CDF7240ABA3675D94F32B6556427E0"
CONTROLLER = REPO / "scripts/gate4_entry/owner_windows_release_controller_v1.py"
CONTROLLER_SHA = "05B20216DD7354482C4AB009487DF4CF04E16FDB31482FC2DD67449F183CB2C6"
NATIVE_DENY_EVIDENCE = (
    "stm32_os_gate4_a05_controller_isolated_native_deny_v2_20261029_"
    "20261029_011416_85a5af83ed814b6baabe2821b724efd1.evidence.zip"
)
NATIVE_DENY_SHA = "8297218F3C964E47FA35BD7E22B829DDD6BC81103E9C86416F1103F9C65A1A47"
APPROVAL_SOURCE = REPO / "scripts/gate4_entry/release_approval_contract_v1.py"
APPROVAL_SHA = "E954EE53E04148F01710B0017E0D6742704C0DB82963EAE728781E009CB7C3F3"
SIGNATURE_SOURCE = REPO / "scripts/gate4_entry/verify_release_signature_v1.py"
SIGNATURE_SHA = "33CB33FBB5AEAFE23A329BBC48B7F094AA4430B421F795E70DBF5067DAE73F5C"

class AuditStop(Exception):
    pass

def need(ok: bool, why: str):
    if not ok:
        raise AuditStop(why)

def sha(raw: bytes) -> str:
    return hashlib.sha256(raw).hexdigest().upper()

def source_exact(path: Path, digest: str) -> str:
    need(path.is_file() and not path.is_symlink(), "SOURCE_MISSING_OR_LINK")
    raw = path.read_bytes()
    need(sha(raw) == digest, "SOURCE_CHANGED_REQUIRE_FRESH_REVIEW:" + path.name)
    return raw.decode("utf-8", "strict")

def inspect_runner_source(text: str) -> dict:
    """Detect the absence of admission checks in known *exact* source control flow.

    This is a structural audit, not a formal proof that Windows users cannot
    execute device commands by some other channel.
    """
    manifest = "$meta=Assert-DeusExtractedBundle $scratch $expectedId"
    branch = "if ($VerifyBundle) {"
    launch = "& $engine -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File (Join-Path $scratch 'apply.ps1')"
    i = text.find(manifest)
    j = text.find(branch, i)
    k = text.find(launch, j)
    need(0 <= i < j < k and text.count(launch) == 1, "RUNNER_LAUNCH_GRAPH_DRIFT")
    control_flow = text[i:k]
    need("SECTION=RUN_APPLICATION_SCRIPT" in control_flow and
         "PowerShell AST parse of apply.ps1 completed without errors" in control_flow,
         "RUNNER_NATIVE_AST_PATH_DRIFT")
    need("BUNDLE_MANIFEST_VERIFIED" in control_flow, "MANIFEST_CHECK_PATH_DRIFT")
    # Never silently declare the issue fixed when a new check is inserted;
    # the accepted v1.3.2 source is deliberately pinned. Re-review then.
    observed_checks = (
      "execution_profile", "HARDWARE_MUTATING", "REMOTE_PUBLIC_TRUST_MUTATING",
      "RELEASE_ADMITTED", "signed_owner_approval", "allowed_signers",
      "SSHSIG", "trusted_release", "nonce_ledger"
    )
    actually = [token for token in observed_checks if token.lower() in control_flow.lower()]
    need(not actually, "PRELAUNCH_ADMISSION_GUARD_CHANGED_REVIEW_REQUIRED")
    return {
        "manifest_sha_and_ast": "SOURCE_PATH_PRESENT",
        "child_process_start": "SOURCE_PATH_PRESENT",
        "independent_owner_signature_gate_before_child": "NOT_FOUND_IN_PINNED_SOURCE",
        "hardware_mutating_profile_gate_before_child": "NOT_FOUND_IN_PINNED_SOURCE",
        "scope": "EXACT_SOURCE_ONLY_NOT_AN_EXECUTED_BYPASS_TEST",
    }

def inspect_builder_source(text: str):
    need("MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE" in text and
         'res["profile"] not in ("HARDWARE_MUTATING", "REMOTE_PUBLIC_TRUST_MUTATING")' in text,
         "CANONICAL_RELEASE_DENIAL_NOT_FOUND")
    need("def validate_blob(" in text and "def release(" in text,
         "RELEASE_VS_VALIDATE_BOUNDARY_DRIFT")
    return "MUTATING_RELEASE_DENIED_SEPARATE_FROM_RUNNER"

def inspect_controller_source(text: str):
    need('CONTROLLER_CLASS="G4_A05_NONCE_RECORDED_SOURCE_CONTROLLER_ONLY"' in text and
         '"authorization":"DENIED"' in text and
         '"release_admitted":"NO"' in text and
         '"executor_started":"NO"' in text and
         '"controller_installed":"NOT_ATTESTED"' in text,
         "CONTROLLER_POSITIVE_ADMISSION_CHANGE_REVIEW_REQUIRED")
    return "SOURCE_ONLY_DENIED_NOT_TRUSTED_EXECUTOR"

def inspect():
    runner = source_exact(RUNNER, RUNNER_SHA)
    builder = source_exact(BUILDER, BUILDER_SHA)
    controller = source_exact(CONTROLLER, CONTROLLER_SHA)
    source_exact(APPROVAL_SOURCE, APPROVAL_SHA)
    source_exact(SIGNATURE_SOURCE, SIGNATURE_SHA)
    evidence = ROOT / "Evidence" / NATIVE_DENY_EVIDENCE
    need(evidence.is_file() and not evidence.is_symlink() and
         sha(evidence.read_bytes()) == NATIVE_DENY_SHA,
         "ACCEPTED_NATIVE_DENY_ORIGINAL_MISSING_OR_DRIFT")
    return {
        "result": "RA07_PINNED_RUNNER_TRUST_BOUNDARY_GAP_SOURCE_AUDIT_PASS",
        "runner_source_sha256": RUNNER_SHA,
        "runner_path": "OWNER_PREVIOUSLY_ACCEPTED_V132_SOURCE_CANDIDATE",
        "runner_findings": inspect_runner_source(runner),
        "bundle_guard": inspect_builder_source(builder),
        "controller": inspect_controller_source(controller),
        "accepted_native_deny_evidence_sha256": NATIVE_DENY_SHA,
        "positive_owner_signed_hardware_admission": "NOT_ATTESTED",
        "separate_owner_installed_execution_gate": "NOT_ATTESTED",
        "physical_usb_ssh_execution": "NOT_PERFORMED",
        "production_nonce": "NOT_CONSUMED",
        "gate4": "NOT_AUTHORIZED",
        "candidate_hardware_mutating_bundle": "NOT_AUTHORED",
        "windows_native_test_of_new_admission": "NOT_RUN",
    }

if __name__ == "__main__":
    print(json.dumps(inspect(), sort_keys=True))
