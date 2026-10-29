#!/usr/bin/env python3
"""No-device original-byte decision bridge; NOT a hardware launcher."""
from __future__ import annotations
import json
from pathlib import Path
import sys
import remote_result_guard as guard

MAX_INPUT = 64 * 1024


def unique_pairs(pairs):
    obj = {}
    for k, v in pairs:
        if k in obj:
            raise ValueError("DUPLICATE_JSON_KEY")
        obj[k] = v
    return obj


def original_remote_receipt(path: Path):
    if path.is_symlink() or not path.is_file():
        raise ValueError("REMOTE_JSON_NOT_REGULAR")
    size = path.stat().st_size
    if not (0 < size <= MAX_INPUT):
        raise ValueError("REMOTE_JSON_OVERSIZE_OR_EMPTY")
    # Parse the SCP-original bytes before any lossy PowerShell conversion.
    obj = json.loads(path.read_bytes().decode("utf-8", "strict"),
                     object_pairs_hook=unique_pairs,
                     parse_constant=lambda _v: (_ for _ in ()).throw(ValueError("NONFINITE_JSON")))
    if type(obj) is not dict:
        raise ValueError("REMOTE_JSON_TOPLEVEL_NOT_OBJECT")
    return obj


def main(argv):
    if len(argv) != 4 or argv[1] not in ("started", "not-started"):
        raise SystemExit("EXACT_BOUNDARY_EXIT_AND_RECEIPT_ARGUMENTS_REQUIRED")
    started = argv[1] == "started"
    try:
        exit_code = None if argv[2] == "UNAVAILABLE" else int(argv[2])
        if exit_code is not None and not (-65535 <= exit_code <= 65535):
            raise ValueError("EXIT_RANGE")
        receipt = None if argv[3] == "MISSING" else original_remote_receipt(Path(argv[3]))
        d = guard.adjudicate(remote_started=started, process_exit=exit_code,
                             remote_receipt=receipt)
        reason = d.reason
    except (ValueError, UnicodeError, OSError, TypeError, KeyError) as exc:
        d = guard.adjudicate(remote_started=started, process_exit=None,
                             remote_receipt=None)
        reason = "ORIGINAL_REMOTE_INPUT_INVALID_" + type(exc).__name__
    print(json.dumps({
        "schema": "DEUS_G3_REMOTE_RESULT_DECISION_V1",
        "result": "SCOPED_PASS" if d.accepted else "FAIL",
        "classification": d.classification,
        "reset_may_have_been_attempted": d.reset_may_have_been_attempted,
        "info_may_have_been_attempted": d.info_may_have_been_attempted,
        "retry_authorized": False,
        "flash_authorized": False,
        "full_gate3_accepted": False,
        "reason": reason
    }, sort_keys=True, separators=(",", ":"), allow_nan=False))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
