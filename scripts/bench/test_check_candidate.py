#!/usr/bin/env python3
"""Fast package checker negative tests, no device or secret access."""
import copy
from pathlib import Path
import unittest
from check_candidate import PACKAGES, STORE, check_blob, check_file

class BenchCandidateTests(unittest.TestCase):
    def test_normal_real_bytes(self):
        normal=PACKAGES["normal"]
        outcome=check_file(STORE/normal["name"],normal)
        self.assertEqual(outcome["version"],6)
        self.assertEqual(outcome["vector_structure"],"PASS")

    def test_rescue_real_bytes(self):
        rescue=PACKAGES["rescue"]
        outcome=check_file(STORE/rescue["name"],rescue)
        self.assertEqual(outcome["version"],7)
        self.assertEqual(outcome["application_origin"],"0x08002000")

    def test_reject_corrupt_payload(self):
        e=PACKAGES["normal"]
        b=bytearray((STORE/e["name"]).read_bytes())
        b[-1]^=0x01
        with self.assertRaisesRegex(ValueError,"WHOLE_PACKAGE_SHA256"):
            check_blob(bytes(b),e)

    def test_reject_wrong_version_or_truncated(self):
        e=copy.deepcopy(PACKAGES["normal"])
        e["version"]=7
        raw=(STORE/e["name"]).read_bytes()
        with self.assertRaisesRegex(ValueError,"FIRMWARE_VERSION"):
            check_blob(raw,e)
        with self.assertRaisesRegex(ValueError,"WHOLE_PACKAGE_LENGTH"):
            check_blob(raw[:-1],PACKAGES["normal"])

    def test_reject_invalid_vector_even_with_matching_payload_digest(self):
        import hashlib
        normal=PACKAGES["normal"]
        b=bytearray((STORE/normal["name"]).read_bytes())
        b[84:88]=b"\x00\x00\x00\x08" # Cortex-M Thumb bit 0 cleared
        image=bytes(b[80:])
        digest=hashlib.sha256(image).digest()
        b[16:48]=digest
        e=copy.deepcopy(normal)
        e["sha256"]=hashlib.sha256(b).hexdigest().upper()
        e["image_sha256"]=hashlib.sha256(image).hexdigest().upper()
        with self.assertRaisesRegex(ValueError,"RESET_HANDLER_THUMB_RANGE"):
            check_blob(bytes(b),e)

if __name__=="__main__":
    unittest.main(verbosity=2)
