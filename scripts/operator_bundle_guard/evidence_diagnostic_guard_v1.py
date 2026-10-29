#!/usr/bin/env python3
"""Independent original operator Evidence ZIP semantic diagnostic guard (v1).

This is a *read-only* source-level acceptance helper, not the ZIP builder and
not a Windows-native test of apply.ps1. It cannot retroactively fix Evidence.
Normative: docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md, mandatory evidence section.

A ZIP that is CRC/index-correct but has a flattened prefix-truncated traceback
MUST be classified DIAGNOSTIC_INCOMPLETE. No raw private data is echoed.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import zipfile

NAME_RE = re.compile(r"[A-Za-z0-9_.-]+\Z")
SHA_RE = re.compile(r"[0-9A-Fa-f]{64}\Z")
SIZE_RE = re.compile(r"([0-9]+)  ([A-Za-z0-9_.-]+)\Z")
INDEX_RE = re.compile(r"([0-9A-Fa-f]{64})  ([A-Za-z0-9_.-]+)\Z")
CAUSE_RE = re.compile(
    r"(?:^|\n)(?:G4_A05_PYTHON_EXCEPTION|TERMINAL_EXCEPTION|"
    r"CHILD_EXCEPTION|NATIVE_TERMINAL_EXCEPTION)="
    r"[^\r\n]{1,900}"
)
TRACE_RE = re.compile(
    r"Traceback \(most recent call last\)|"
    r"(?:ModuleNotFoundError|ImportError|TypeError|ValueError|Exception):"
)
ERROR_LINE_RE = re.compile(
    r"(?:ModuleNotFoundError|ImportError|TypeError|ValueError|"
    r"RuntimeError|OSError|PermissionError|TimeoutError|"
    r"AssertionError|FileNotFoundError|Exception):\s*\S[^\r\n]{0,700}"
)


class RejectedEvidence(Exception):
    def __init__(self, reason: str):
        super().__init__(reason)
        self.reason = reason


def require(ok: bool, reason: str):
    if not ok:
        raise RejectedEvidence(reason)


def sha256(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest().upper()


def _pairs_no_duplicate(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "DUPLICATE_TYPED_FIELD")
        result[key] = value
    return result


def _unique_files(z: zipfile.ZipFile) -> dict[str, bytes]:
    infos = z.infolist()
    require(0 < len(infos) <= 48, "UNBOUNDED_MEMBER_COUNT")
    names = [info.filename for info in infos]
    require(len(names) == len(set(names)), "DUPLICATE_MEMBER")
    for info in infos:
        name = info.filename
        require(NAME_RE.fullmatch(name) is not None and
                name not in (".", "..") and
                PurePosixPath(name).name == name and
                not info.is_dir() and
                info.file_size <= 400_000 and
                not(info.flag_bits & 1), "UNSAFE_MEMBER")
    require(sum(x.file_size for x in infos) <= 800_000, "UNBOUNDED_EVIDENCE_BYTES")
    require(z.testzip() is None, "ZIP_CRC")
    return {name: z.read(name) for name in names}


def _verify_indexes(data: dict[str, bytes]):
    require("hashes.sha256" in data and "sizes.txt" in data and
            "outcome.txt" in data, "MISSING_REQUIRED_MEMBERS")
    rows = data["hashes.sha256"].decode("ascii", "strict").splitlines()
    seen = set()
    for row in rows:
        m = INDEX_RE.fullmatch(row)
        require(m is not None and m[2] != "hashes.sha256" and
                m[2] in data and m[2] not in seen, "HASH_INDEX_MALFORMED")
        require(sha256(data[m[2]]) == m[1].upper(), "HASH_INDEX_DRIFT")
        seen.add(m[2])
    require(seen == set(data) - {"hashes.sha256"}, "HASH_INDEX_COVERAGE")
    rows = data["sizes.txt"].decode("ascii", "strict").splitlines()
    seen = set()
    for row in rows:
        m = SIZE_RE.fullmatch(row)
        require(m is not None and m[2] not in {"hashes.sha256", "sizes.txt"} and
                m[2] in data and m[2] not in seen, "SIZE_INDEX_MALFORMED")
        require(len(data[m[2]]) == int(m[1]), "SIZE_INDEX_DRIFT")
        seen.add(m[2])
    require(seen == set(data) - {"hashes.sha256", "sizes.txt"},
            "SIZE_INDEX_COVERAGE")


def diagnose_original_evidence(path: Path, pinned_sha256: str) -> dict[str, object]:
    """Fail closed on exact-byte mismatch, malformed index or lost terminal cause.

    Supported v1 semantic proof scope: canonical six-member source/host gate
    Evidence with outcome.txt, native_test_output.txt and optional run.log.
    Other evidence schemas require a separately reviewed adapter, not guesses.
    """
    require(SHA_RE.fullmatch(pinned_sha256) is not None, "UNPINNED_EVIDENCE")
    p = Path(path)
    require(p.is_file() and not p.is_symlink(), "EVIDENCE_NOT_FILE")
    original = p.read_bytes()
    require(0 < len(original) <= 1_000_000 and
            sha256(original) == pinned_sha256.upper(), "EVIDENCE_SHA_MISMATCH")
    try:
        with zipfile.ZipFile(p) as z:
            data = _unique_files(z)
    except (zipfile.BadZipFile, OSError, UnicodeError, EOFError,
            RuntimeError, ValueError) as exc:
        raise RejectedEvidence("ZIP_OR_MEMBER_UNREADABLE") from exc
    try:
        _verify_indexes(data)
    except (UnicodeError, ValueError, KeyError, TypeError) as exc:
        raise RejectedEvidence("EVIDENCE_INDEX_UNREADABLE") from exc
    try:
        outcome = json.loads(data["outcome.txt"].decode("utf-8", "strict"),
                             object_pairs_hook=_pairs_no_duplicate)
    except (UnicodeError, json.JSONDecodeError, ValueError) as exc:
        raise RejectedEvidence("OUTCOME_NOT_TYPED_JSON") from exc
    require(type(outcome) is dict, "OUTCOME_NOT_OBJECT")
    result = outcome.get("result")
    require(result in {"PASS", "FAIL", "STOP"}, "OUTCOME_UNKNOWN")
    require(type(outcome.get("classification")) is str and
            bool(outcome["classification"]), "CLASSIFICATION_MISSING")
    require(type(outcome.get("gate")) is str and
            bool(outcome["gate"]), "PACKAGE_ID_MISSING")
    require("native_test_output.txt" in data and
            "native_checks.txt" in data and "run.log" in data,
            "ORIGINAL_STREAM_MEMBERS_MISSING")
    try:
        text = data["native_test_output.txt"].decode("utf-8", "strict")
    except UnicodeError as exc:
        raise RejectedEvidence("TRANSCRIPT_NOT_UTF8") from exc
    require("\0" not in text, "INVALID_TRANSCRIPT")
    require("G4_A05_CHILD_EXIT=" in text, "CHILD_EXIT_NOT_RECORDED")
    if result in {"FAIL", "STOP"}:
        phase = outcome.get("failure_phase")
        error = outcome.get("error")
        require(type(phase) is str and len(phase) >= 2 and phase != "NONE" and
                type(error) is str and len(error) >= 3 and error != "NONE",
                "FAIL_WITHOUT_PHASE_OR_ERROR")
        # A pre-child STOP is often exactly classified by the source pin or AST
        # result and need not fabricate a terminal Python traceback.
        child = str(outcome.get("child_exit_code", ""))
        child_started = child not in {"NOT_STARTED", "NONE", "-1", ""}
        if child_started:
            evidence_has_trace = TRACE_RE.search(text) is not None
            prefix_only = ("[LINE_TRUNCATED]" in text and
                           "MIDDLE_TRUNCATED_TAIL_PRESERVED" not in text)
            need_cause = evidence_has_trace or "PYTHON_" in text
            if need_cause:
                require(not prefix_only, "DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY")
                cause = CAUSE_RE.search(text)
                require(cause is not None,
                        "DIAGNOSTIC_INCOMPLETE_TERMINAL_CAUSE_MISSING")
                require(ERROR_LINE_RE.search(cause.group()) is not None,
                        "DIAGNOSTIC_INCOMPLETE_EXCEPTION_TYPE_MISSING")
                require("ModuleNotFoundError" not in text or
                        ("No module named" in text and
                         ("ModuleNotFoundError" in cause.group())),
                        "DIAGNOSTIC_INCOMPLETE_MISSING_MODULE_IDENTITY")
            else:
                # A typed child FAIL with no evidence of diagnostic context
                # cannot be promoted to diagnostically complete.
                require("G4_A05_NATIVE_CASE=FAIL " in text or
                        "CHILD_EXCEPTION=" in text or
                        "TERMINAL_EXCEPTION=" in text,
                        "DIAGNOSTIC_INCOMPLETE_CHILD_CAUSE_MISSING")
    return {
        "result": "EVIDENCE_DIAGNOSTIC_COMPLETE",
        "original_result": result,
        "evidence_sha256":sha256(original),
        "zip_crc": "PASS",
        "member_index": "PASS",
        "semantic_cause": "PRESENT_OR_NOT_APPLICABLE",
        "scope": "SOURCE_LEVEL_INDEPENDENT_ADJUDICATION_NOT_NATIVE",
    }
