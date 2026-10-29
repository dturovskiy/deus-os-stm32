#!/usr/bin/env python3
"""Optional original owner Windows A05 controller v2 Evidence regression.

In clean CI original off-Git Windows Evidence is absent and this sole
original-only test skips; all synthetic fail-closed tests run separately.
Never replays native controller, imports the .pyd, or mutates source/evidence.
"""
from pathlib import Path
import unittest
import audit_a05_controller_isolated_deny_v2 as audit

NAME=("stm32_os_gate4_a05_controller_isolated_native_deny_v2_20261029_"
      "20261029_011416_85a5af83ed814b6baabe2821b724efd1.evidence.zip")
SHA="8297218F3C964E47FA35BD7E22B829DDD6BC81103E9C86416F1103F9C65A1A47"

class GenuineOriginalReadOnlyTests(unittest.TestCase):
    def test_original_evidence_if_present(self):
        original=audit.ROOT/"Evidence"/NAME
        receipt=(audit.ROOT/"HostDependencies"/audit.TARGET/"ADMISSION.json")
        if not original.is_file() or not receipt.is_file():
            self.skipTest("Original owner Windows Evidence and target are off Git")
        result=audit.evidence(NAME,SHA)
        self.assertEqual(result["result"],"A05_ISOLATED_CONTROLLER_NATIVE_DENY_ORIGINAL_SCOPED_PASS")
        self.assertEqual(result["original_evidence_sha256"],SHA)
        self.assertEqual(result["actual_native_cases"],"4/4")
        self.assertEqual(result["crc"],"6/6")
        self.assertEqual(result["sha_index"],"5/5")
        self.assertEqual(result["size_index"],"4/4")
        self.assertEqual(result["authorization"],"DENIED")
        self.assertEqual(result["hardware"],"NONE")
        self.assertEqual(result["gate4"],"NOT_AUTHORIZED")

    def test_wrong_original_digest_fails_closed(self):
        if not (audit.ROOT/"Evidence"/NAME).is_file():
            self.skipTest("Off-Git original Windows Evidence absent")
        with self.assertRaisesRegex(AssertionError,"ORIGINAL_EVIDENCE_WHOLE_SHA"):
            audit.evidence(NAME,"A"*64)

if __name__=="__main__":
    unittest.main(verbosity=2)
