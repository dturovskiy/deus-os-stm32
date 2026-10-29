#!/usr/bin/env python3
"""Standard-library-only source regression for operator Evidence semantics.

This checks the *independent adjudicator*, not native Windows PowerShell
formatter execution. Original owner v4 Evidence is immutable and must reject.
"""
from __future__ import annotations

import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

from evidence_diagnostic_guard_v1 import diagnose_original_evidence, RejectedEvidence

BASE = Path(__file__).resolve().parents[3]
V4_NAME = (
    "stm32_os_gate4_a05_controller_adapter_native_offline_v4_20261028_"
    "20261028_211601_84e61bbdc92345d781a6023698b84f2d.evidence.zip"
)
V4_SHA = "471563D0FC49AD7DFE091FF1158C1F1B1A8E8173014B32C8348D0AD117AD5972"


def digest(blob):
    return hashlib.sha256(blob).hexdigest().upper()


def fixture(terminal=True, result="FAIL", text=None):
    if text is None:
        text = (
            "G4_A05_CHILD_EXIT=1\n"
            "G4_A05_NATIVE_CASE=PASS WINDOWS_POWERSHELL7\n"
            "Traceback (most recent call last):\n"
            '  File "<string>", line 1, in <module>\n'
        )
        if terminal:
            text += (
                "G4_A05_PYTHON_EXCEPTION=py.exe EXIT=1 "
                "MESSAGE=ModuleNotFoundError: No module named 'cryptography'\n"
            )
    outcome = {
        "gate": "synthetic_evidence_test_only",
        "result": result,
        "classification": "SYNTHETIC_SOURCE_TEST_ONLY",
        "failure_phase": "WINDOWS_NATIVE_CONTROLLER_OFFLINE" if result != "PASS" else "NONE",
        "error": "PYTHON_IMPORT_FAILED" if result != "PASS" else "NONE",
        "child_exit_code": "1" if result != "PASS" else "0",
    }
    data = {
        "outcome.txt": (json.dumps(outcome, sort_keys=True) + "\n").encode(),
        "run.log": b"OFFLINE=YES\nUSB=NO\n",
        "native_checks.txt": b"PASS SOURCE_PIN\n",
        "native_test_output.txt": text.encode("utf-8"),
    }
    data["sizes.txt"] = "".join(
        f"{len(v)}  {n}\n" for n, v in data.items()
    ).encode()
    data["hashes.sha256"] = "".join(
        f"{digest(v)}  {n}\n" for n, v in data.items()
    ).encode()
    return data


def zip_bytes(members):
    f = io.BytesIO()
    with zipfile.ZipFile(f, mode="w", compression=zipfile.ZIP_DEFLATED) as z:
        for name, data in members.items():
            z.writestr(name, data)
    return f.getvalue()


class SemanticTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="deus_evidence_semantics_")
        self.addCleanup(self.tmp.cleanup)
        self.target = Path(self.tmp.name) / "synthetic.evidence.zip"

    def inspect(self, members=None, pin=None):
        blob = zip_bytes(fixture() if members is None else members)
        self.target.write_bytes(blob)
        return diagnose_original_evidence(self.target, digest(blob) if pin is None else pin)

    def stop(self, reason, members=None, pin=None):
        with self.assertRaises(RejectedEvidence) as cm:
            self.inspect(members, pin)
        self.assertEqual(cm.exception.reason, reason)

    def test_real_owner_v4_original_integrity_but_diagnostics_rejected(self):
        p = BASE / "Evidence" / V4_NAME
        if not p.is_file():
            self.skipTest("Original owner Windows Evidence stays off Git; synthetic regression still runs")
        self.assertEqual(digest(p.read_bytes()), V4_SHA)
        with self.assertRaises(RejectedEvidence) as cm:
            diagnose_original_evidence(p, V4_SHA)
        self.assertEqual(cm.exception.reason, "DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY")

    def test_complete_synthetic_fail_retains_final_module_exception(self):
        got = self.inspect()
        self.assertEqual(got["result"], "EVIDENCE_DIAGNOSTIC_COMPLETE")
        self.assertEqual(got["original_result"], "FAIL")

    def test_pass_synthetic_has_no_exception_requirement(self):
        x = fixture(result="PASS", text="G4_A05_CHILD_EXIT=0\nG4_A05_NATIVE_CASE=PASS TEST_CASE\n")
        self.assertEqual(self.inspect(x)["original_result"], "PASS")

    def test_fail_without_exception_is_denied(self):
        self.stop("DIAGNOSTIC_INCOMPLETE_TERMINAL_CAUSE_MISSING", fixture(terminal=False))

    def test_traceback_with_prefix_truncation_denied_even_with_cause(self):
        x = fixture()
        x["native_test_output.txt"] += b" [LINE_TRUNCATED]\n"
        self.stop("DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY", reindex(x))

    def test_terminal_cause_without_exception_class_denied(self):
        x = fixture()
        x["native_test_output.txt"] = x["native_test_output.txt"].replace(
            b"ModuleNotFoundError: No module named 'cryptography'", b"Something went wrong"
        )
        # Recompute indexes to isolate the semantic failure.
        x = fixture(text=x["native_test_output.txt"].decode())
        self.stop("DIAGNOSTIC_INCOMPLETE_EXCEPTION_TYPE_MISSING", x)

    def test_multiline_long_context_with_cause_in_tail(self):
        text = (
            "G4_A05_CHILD_EXIT=1\n"
            "Traceback (most recent call last):\n"
            + "frame: " + "x" * 850 +
            " [MIDDLE_TRUNCATED_TAIL_PRESERVED] "
            "ModuleNotFoundError: No module named 'cryptography'\n"
            "G4_A05_PYTHON_EXCEPTION=py.exe EXIT=1 "
            "MESSAGE=ModuleNotFoundError: No module named 'cryptography'\n"
        )
        self.assertEqual(self.inspect(fixture(text=text))["result"],
                         "EVIDENCE_DIAGNOSTIC_COMPLETE")

    def test_timeout_terminal_cause_preserved(self):
        text = ("G4_A05_CHILD_EXIT=-2\n"
                "TERMINAL_EXCEPTION=TimeoutError: native child exceeded 120 seconds\n")
        self.assertEqual(self.inspect(fixture(text=text))["result"],
                         "EVIDENCE_DIAGNOSTIC_COMPLETE")

    def test_changed_pinned_sha_refused(self):
        self.stop("EVIDENCE_SHA_MISMATCH", pin="0" * 64)

    def test_changed_member_with_unchanged_hash_index_refused(self):
        x = fixture()
        x["native_test_output.txt"] += b"TAMPER\n"
        self.stop("HASH_INDEX_DRIFT", x)

    def test_deleted_member_index_coverage_refused(self):
        x = fixture()
        del x["run.log"]
        self.stop("HASH_INDEX_MALFORMED", x)

    def test_duplicate_zip_member_refused(self):
        b = io.BytesIO()
        import warnings
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            with zipfile.ZipFile(b, "w") as z:
                z.writestr("outcome.txt", b"{}")
                z.writestr("outcome.txt", b"{}")
        blob = b.getvalue()
        self.target.write_bytes(blob)
        with self.assertRaises(RejectedEvidence) as cm:
            diagnose_original_evidence(self.target, digest(blob))
        self.assertEqual(cm.exception.reason, "DUPLICATE_MEMBER")

    def test_path_traversal_member_refused(self):
        x = fixture()
        x["../escape"] = b"unsafe"
        self.stop("UNSAFE_MEMBER", x)

    def test_missing_failure_phase_refused(self):
        x = fixture()
        o = json.loads(x["outcome.txt"])
        o["failure_phase"] = "NONE"
        x = fixture(text=x["native_test_output.txt"].decode())
        x["outcome.txt"] = json.dumps(o).encode()
        # Renew indices only for this semantic negative.
        x = reindex(x)
        self.stop("FAIL_WITHOUT_PHASE_OR_ERROR", x)

    def test_non_ascii_sha_index_fails_typed_not_traceback(self):
        x = fixture()
        x["hashes.sha256"] = b"\xff\x00\x80"
        self.stop("EVIDENCE_INDEX_UNREADABLE", x)

    def test_malformed_utf8_child_transcript_fails_typed_not_traceback(self):
        x = fixture()
        x["native_test_output.txt"] = b"\xff\x00\x80"
        self.stop("TRANSCRIPT_NOT_UTF8", reindex(x))

    def test_original_v4_unchanged_by_readonly_auditor(self):
        p = BASE / "Evidence" / V4_NAME
        if not p.is_file():
            self.skipTest("Original owner Windows Evidence stays off Git; synthetic regression still runs")
        before = digest(p.read_bytes())
        with self.assertRaises(RejectedEvidence):
            diagnose_original_evidence(p, V4_SHA)
        self.assertEqual(digest(p.read_bytes()), before)


def reindex(data):
    content = {k: v for k, v in data.items()
               if k not in {"hashes.sha256", "sizes.txt"}}
    content["sizes.txt"] = "".join(
        f"{len(v)}  {n}\n" for n, v in content.items()
    ).encode()
    content["hashes.sha256"] = "".join(
        f"{digest(v)}  {n}\n" for n, v in content.items()
    ).encode()
    return content


if __name__ == "__main__":
    unittest.main(verbosity=2)
