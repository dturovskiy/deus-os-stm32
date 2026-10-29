#!/usr/bin/env python3
"""Read-only failure-mode regressions for RA07 installed-runner source gap.

No runner install, no child launch, no USB/SSH, no protected trust anchor.
"""
import unittest
from unittest.mock import patch
from pathlib import Path
import tempfile
import audit_ra07_execution_trust_boundary_v1 as audit

def synthetic_runner():
    return "\n".join([
        "$meta=Assert-DeusExtractedBundle $scratch $expectedId",
        "Write-DeusStamped '[OK] BUNDLE_MANIFEST_VERIFIED'",
        "Write-DeusStamped '[OK] PowerShell AST parse of apply.ps1 completed without errors.'",
        "if ($VerifyBundle) {",
        "} else {",
        "Write-DeusStamped 'SECTION=RUN_APPLICATION_SCRIPT'",
        "& $engine -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -File (Join-Path $scratch 'apply.ps1')",
    ])

class RunnerTrustBoundaryTests(unittest.TestCase):
    def test_exact_current_source_audited_not_promoted(self):
        if not audit.RUNNER.is_file() or not (audit.ROOT/"Evidence"/audit.NATIVE_DENY_EVIDENCE).is_file():
            self.skipTest("Owner-only original runner and Evidence absent from clean Git CI")
        result=audit.inspect()
        self.assertEqual(result["result"],"RA07_PINNED_RUNNER_TRUST_BOUNDARY_GAP_SOURCE_AUDIT_PASS")
        self.assertEqual(result["positive_owner_signed_hardware_admission"],"NOT_ATTESTED")
        self.assertEqual(result["gate4"],"NOT_AUTHORIZED")
        self.assertEqual(result["runner_findings"]["hardware_mutating_profile_gate_before_child"],
                         "NOT_FOUND_IN_PINNED_SOURCE")

    def test_runner_unreviewed_source_rejected(self):
        with tempfile.TemporaryDirectory() as dirname:
            p=Path(dirname)/"runner.ps1"
            p.write_bytes(b"unrelated")
            with self.assertRaisesRegex(audit.AuditStop,"SOURCE_CHANGED_REQUIRE_FRESH_REVIEW"):
                audit.source_exact(p,audit.RUNNER_SHA)

    def test_launcher_missing_child_path_fails_closed(self):
        with self.assertRaisesRegex(audit.AuditStop,"RUNNER_LAUNCH_GRAPH_DRIFT"):
            audit.inspect_runner_source("$meta=Assert-DeusExtractedBundle $scratch $expectedId")

    def test_launcher_branch_injected_guard_is_not_misrepresented_as_current(self):
        src=synthetic_runner()
        anchor="if ($VerifyBundle) {"
        modified=src.replace(anchor,
            "if ($meta.execution_profile -eq 'HARDWARE_MUTATING') { throw 'STOP' }\n"
            +anchor,1)
        self.assertNotEqual(modified,src)
        with self.assertRaisesRegex(audit.AuditStop,"PRELAUNCH_ADMISSION_GUARD_CHANGED_REVIEW_REQUIRED"):
            audit.inspect_runner_source(modified)

    def test_signature_word_only_in_comment_does_not_establish_trust(self):
        src=synthetic_runner()
        anchor="if ($VerifyBundle) {"
        modified=src.replace(anchor, "#SSHSIG SOURCE COMMENT IS NOT A GATE\n"+anchor,1)
        with self.assertRaisesRegex(audit.AuditStop,"PRELAUNCH_ADMISSION_GUARD_CHANGED_REVIEW_REQUIRED"):
            audit.inspect_runner_source(modified)
        # This intentionally errs on requiring re-review; the structural
        # audit is not a proof of positive admission on a modified runner.

    def test_builder_without_release_denial_rejected(self):
        src=audit.source_exact(audit.BUILDER,audit.BUILDER_SHA)
        altered=src.replace("MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE","NOT_REALLY_DENIED")
        with self.assertRaisesRegex(audit.AuditStop,"CANONICAL_RELEASE_DENIAL_NOT_FOUND"):
            audit.inspect_builder_source(altered)

    def test_source_controller_without_denied_is_not_admitted(self):
        src=audit.source_exact(audit.CONTROLLER,audit.CONTROLLER_SHA)
        altered=src.replace('"release_admitted":"NO"','"release_admitted":"YES"')
        with self.assertRaisesRegex(audit.AuditStop,"CONTROLLER_POSITIVE_ADMISSION_CHANGE_REVIEW_REQUIRED"):
            audit.inspect_controller_source(altered)

    def test_original_native_evidence_wrong_sha_stops(self):
        if not audit.RUNNER.is_file() or not (audit.ROOT/"Evidence"/audit.NATIVE_DENY_EVIDENCE).is_file():
            self.skipTest("Original owner Evidence absent in GitHub CI")
        with patch.object(audit,"NATIVE_DENY_SHA","0"*64):
            with self.assertRaisesRegex(audit.AuditStop,"ACCEPTED_NATIVE_DENY_ORIGINAL_MISSING_OR_DRIFT"):
                audit.inspect()

if __name__=="__main__":
    unittest.main(verbosity=2)
