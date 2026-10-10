#!/usr/bin/env python3
"""Bounded read-only Git history signature screen. No blob content or paths printed.

This is NOT a substitute for owner approval, threat modelling, or external replica audit.
Use: python scripts/public_repository_history_audit.py [--ref main]
"""
from __future__ import annotations
import argparse
import collections
import json
import re
import subprocess
import sys
from pathlib import Path

SIGNATURES = {
    "pem_private_key_header": rb"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----",
    "github_credential_prefix": rb"(?:gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,})",
    "aws_access_key_id": rb"\b(?:AKIA|ASIA)[A-Z0-9]{16}\b",
    "openai_style_key": rb"\bsk-(?:proj-|svcacct-)?[A-Za-z0-9_-]{20,}",
    "quoted_secret_assignment": rb"\b(?:password|passwd|api[_-]?key|access[_-]?token|secret[_-]?key|private[_-]?key)\s*[:=]\s*[\"'][^\"'\r\n]{8,}[\"']",
    "bearer_credential": rb"Authorization\s*:\s*Bearer\s+[A-Za-z0-9._=-]{15,}",
    "slack_token": rb"\bxox[baprs]-[A-Za-z0-9-]{16,}",
    "jwt_like": rb"\beyJ[A-Za-z0-9_-]{15,}\.eyJ[A-Za-z0-9_-]{15,}\.[A-Za-z0-9_-]{16,}",
}
PATTERNS = {name: re.compile(pattern, re.I if name in ("quoted_secret_assignment", "bearer_credential") else 0) for name, pattern in SIGNATURES.items()}
ENVIRONMENT = {
    "windows_user_or_project_path": re.compile(rb"\b[A-Za-z]:\\(?:Users\\|Projects\\|Tools\\)", re.I),
    "unix_home_path": re.compile(rb"/(?:home|Users)/[A-Za-z0-9_.-]+/"),
}
HEX = re.compile(r"^[0-9a-f]{40}$")

def run_git(*args: str, data: bytes | None = None) -> bytes:
    cmd = ["git", *args]
    result = subprocess.run(cmd, input=data, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    if result.returncode:
        # Git stderr may contain a sensitive path: do not print it.
        raise RuntimeError("Git command failed: " + args[0] + "; exit=" + str(result.returncode))
    return result.stdout

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", default="main", help="local Git commit/ref to audit; default main")
    args = ap.parse_args()
    if not re.fullmatch(r"[A-Za-z0-9_.-]{1,100}", args.ref):
        raise ValueError("Unsafe ref format")
    root = Path(run_git("rev-parse", "--show-toplevel").decode().strip())
    if root != Path.cwd().resolve():
        # Keep proof bound to the actual caller-selected repo.
        raise RuntimeError("Run from repository root")
    commit = run_git("rev-parse", "--verify", args.ref + "^{commit}").decode().strip()
    raw = run_git("rev-list", "--objects", args.ref).splitlines()
    ids = []
    seen = set()
    for line in raw:
        sha = line.split(b" ", 1)[0].decode("ascii")
        if not HEX.fullmatch(sha):
            raise ValueError("Malformed reachable object identifier")
        if sha not in seen:
            seen.add(sha)
            ids.append(sha)
    report = {
        "ref": args.ref, "commit": commit, "reachable_object_count": len(ids),
        "objects_by_type": {}, "blob_objects_checked": 0, "blob_bytes_checked": 0,
        "binary_blob_count": 0, "nul_bearing_blob_count": 0,
        "credential_pattern_blob_counts": {k: 0 for k in PATTERNS},
        "environment_pattern_blob_counts": {k: 0 for k in ENVIRONMENT},
        "unreadable_or_truncated_blob_count": 0,
        "scope": "reachable objects under specified commit only; excludes stash/unreachable, external caches, and unknown secret formats",
    }
    # Enumerate exact object types and sizes first. Keep object contents inside process.
    types = {}
    for off in range(0, len(ids), 160):
        chunk = ids[off:off + 160]
        meta = run_git("cat-file", "--batch-check", data=("\n".join(chunk) + "\n").encode()).splitlines()
        if len(meta) != len(chunk):
            raise RuntimeError("Incomplete Git object metadata")
        for oid, line in zip(chunk, meta):
            fields = line.decode("ascii").split()
            if len(fields) != 3 or fields[0] != oid or not fields[2].isdigit():
                raise RuntimeError("Invalid Git batch-check response")
            types[oid] = (fields[1], int(fields[2]))
            report["objects_by_type"][fields[1]] = report["objects_by_type"].get(fields[1], 0) + 1
    blobs = [x for x in ids if types[x][0] == "blob"]
    for off in range(0, len(blobs), 40):
        chunk = blobs[off:off + 40]
        response = run_git("cat-file", "--batch", data=("\n".join(chunk) + "\n").encode())
        pos = 0
        for oid in chunk:
            end = response.find(b"\n", pos)
            if end == -1:
                raise RuntimeError("Truncated blob header")
            header = response[pos:end].decode("ascii").split()
            if len(header) != 3 or header[0] != oid or header[1] != "blob":
                raise RuntimeError("Incorrect blob header")
            size = int(header[2])
            data_start = end + 1
            data_end = data_start + size
            if data_end >= len(response) or response[data_end:data_end + 1] != b"\n" or size != types[oid][1]:
                report["unreadable_or_truncated_blob_count"] += 1
                raise RuntimeError("Truncated or mismatched blob")
            blob = memoryview(response)[data_start:data_end].tobytes()
            report["blob_objects_checked"] += 1
            report["blob_bytes_checked"] += size
            if b"\x00" in blob:
                report["nul_bearing_blob_count"] += 1
            if b"\x00" in blob or blob.count(b"\n") == 0:
                report["binary_blob_count"] += 1
            for name, pattern in PATTERNS.items():
                if pattern.search(blob):
                    report["credential_pattern_blob_counts"][name] += 1
            for name, pattern in ENVIRONMENT.items():
                if pattern.search(blob):
                    report["environment_pattern_blob_counts"][name] += 1
            pos = data_end + 1
        if pos != len(response):
            raise RuntimeError("Unexpected Git batch output bytes")
    print(json.dumps(report, sort_keys=True, separators=(",", ":")))
    return 0 if report["unreadable_or_truncated_blob_count"] == 0 else 2

if __name__ == "__main__":
    try:
        sys.exit(main())
    except Exception as exc:
        # Exception messages must not contain paths or contents.
        print(json.dumps({"status":"FAILED", "category":type(exc).__name__}, sort_keys=True), file=sys.stderr)
        sys.exit(2)
