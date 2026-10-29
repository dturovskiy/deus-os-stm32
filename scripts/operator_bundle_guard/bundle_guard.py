#!/usr/bin/env python3
"""DEUS OS V2 bundle builder and static prepublication guard (no hardware I/O).

All operator ZIPs must be generated with build(), not by hand-editing a manifest.
Validation is structural ONLY; use 'release' on a real Windows host plus
independent Mac-mini primitive receipts before ANY hardware operator handoff.
"""
from __future__ import annotations
import argparse
import hashlib
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import stat
import subprocess
import sys
import tempfile
import zipfile
import zlib

SCHEMA = "DEUS_OS_OPERATOR_BUNDLE_V2"
RUNNER = "DEUS_OS_RUNNER_RESULT_V1"
ROOT = frozenset(("apply.ps1", "manifest.json", "hashes.sha256", "PACKAGE_README.txt"))
IDENT = re.compile(r"stm32_os_[a-z0-9_]{5,120}\Z")
HEX = re.compile(r"[A-Fa-f0-9]{64}\Z")
INDEX = re.compile(r"([A-Fa-f0-9]{64})  ([A-Za-z0-9_./-]+)\Z")
PROFILES = frozenset(("OFFLINE", "HARDWARE_READONLY", "HARDWARE_MUTATING", "REMOTE_PUBLIC_TRUST_MUTATING", "HOST_NETWORK_DOWNLOAD"))
PUBLIC_TRUST_WORKFLOW = "gate3_macmini_owner_public_trust_enrollment"
PUBLIC_TRUST_PACKAGE = "stm32_os_gate3_macmini_owner_public_trust_enrollment_v1_20261027"
PUBLIC_TRUST_NAMESPACE = "deus-os-gate3-linux-public-trust-enrollment-v1"
WHEEL_DOWNLOAD_PACKAGE = "stm32_os_gate4_a05_python314_wheel_download_v1_20261028"
WHEEL_DOWNLOAD_WORKFLOW = "gate4_a05_pypi_wheel_acquisition"
MAX_ZIP = 32 * 1024 * 1024
MAX_MEMBER = 8 * 1024 * 1024
MAX_MEMBERS = 96


class Invalid(ValueError):
    pass


def need(condition, code):
    if not condition:
        raise Invalid(code)


def digest(b):
    return hashlib.sha256(b).hexdigest().upper()


def uniq(pairs):
    out = {}
    for key, val in pairs:
        need(key not in out, "DUPLICATE_JSON_KEY")
        out[key] = val
    return out


def parse_json(blob):
    need(len(blob) <= 512 * 1024, "OVERSIZE_JSON")
    try:
        return json.loads(blob.decode("utf-8", "strict"), object_pairs_hook=uniq,
                          parse_constant=lambda _: (_ for _ in ()).throw(Invalid("NONFINITE_JSON")))
    except (ValueError, UnicodeError) as exc:
        if isinstance(exc, Invalid):
            raise
        raise Invalid("INVALID_JSON") from exc


def valid_name(name):
    need(isinstance(name, str) and 1 <= len(name) <= 190, "INVALID_MEMBER_NAME")
    need(re.fullmatch(r"[A-Za-z0-9_./-]+", name) is not None, "INVALID_MEMBER_NAME")
    need(not PurePosixPath(name).is_absolute() and all(
        part not in ("", ".", "..") for part in name.split("/")), "PATH_TRAVERSAL")
    need(not name.startswith(".") and not name.endswith("/"), "INVALID_MEMBER_NAME")
    need(not name.lower().endswith((".zip", ".exe", ".dll", ".pem", ".key", ".pfx",
                                    ".bin", ".hex", ".log", ".evidence")), "UNSAFE_PAYLOAD")


def check_apply(data):
    need(data and not data.startswith(b"\xef\xbb\xbf") and b"\0" not in data,
         "INVALID_APPLY_BYTES")
    try:
        text = data.decode("utf-8", "strict")
    except UnicodeError as exc:
        raise Invalid("INVALID_APPLY_UTF8") from exc
    need(re.search(r"(?im)^\s*Set-StrictMode\s+-Version\s+3(?:\.0)?\s*$", text)
         is not None, "STRICTMODE_3_REQUIRED")
    need("DEUS_RUN_RESULT_PATH" in text, "RESULT_CONTRACT_NOT_USED")
    need(re.search(r"(?im)^\s*Set-StrictMode\s+-Version\s+Latest\b", text)
         is None, "STRICTMODE_LATEST_FORBIDDEN")
    need(re.search(r"(?im)^\s*(?:Write-Host|Write-Output)\b[^\r\n]*"
                   r"(?:FINAL_OUTCOME=|RESULT=PASS|RESULT=FAIL)", text)
         is None, "RUNNER_OWNS_RESULT")
    need(re.search(r"(?im)Set-MpPreference\s+[^\r\n]*"
                   r"(?:DisableRealtimeMonitoring|ExclusionPath)", text)
         is None, "DEFENDER_BYPASS_FORBIDDEN")
    # A-05 v4 did "$one=$one.Substring(0,600)" on a flattened traceback
    # and discarded its final exception. Reject that known unsafe pattern.
    # This is only a source anti-pattern guard; native writer FAIL-path
    # testing remains mandatory before admitting an operator.
    diag_prefix_only = re.search(
        r"(?im)\$(?:one|line|diag(?:nostic)?|trace(?:back)?|stderr|stdout|"
        r"nativeoutput|output|log|message|err(?:or)?)\s*=\s*"
        r"\$(?:one|line|diag(?:nostic)?|trace(?:back)?|stderr|stdout|"
        r"nativeoutput|output|log|message|err(?:or)?)"
        r"\.Substring\s*\(\s*0\s*,", text)
    if diag_prefix_only is not None:
        # A single-line head+tail expression may safely start with a
        # Substring(0,N); require a real Length-offset tail operation
        # AND an explicit middle-truncation marker on that same line.
        # Do not accept a marker alone as runtime proof of completeness.
        end = text.find("\n", diag_prefix_only.start())
        statement = text[diag_prefix_only.start():
                         len(text) if end < 0 else end]
        has_tail = re.search(
            r"\.Substring\s*\(\s*\$\w+\.Length\s*-\s*\d+", statement,
            flags=re.IGNORECASE) is not None
        need(has_tail and "MIDDLE_TRUNCATED_TAIL_PRESERVED" in statement,
             "EVIDENCE_PREFIX_ONLY_TRUNCATION_FORBIDDEN")
    return text


def validate_profile_scope(info):
    """Build/validate may carry a signed-gated candidate; never grant release."""
    if info.get("package_id") == PUBLIC_TRUST_PACKAGE:
        need(info.get("execution_profile") == "REMOTE_PUBLIC_TRUST_MUTATING",
             "PUBLIC_TRUST_PROFILE_REQUIRED")
    # A single owner-authorized acquisition workflow may contact official PyPI.
    # It is NOT OFFLINE and is never accepted as a generic network or install profile.
    # Only the exact one-purpose package can carry this network-capable profile.
    if info.get("execution_profile") == "HOST_NETWORK_DOWNLOAD":
        need(info.get("package_id") == WHEEL_DOWNLOAD_PACKAGE,
             "NETWORK_DOWNLOAD_PACKAGE_NOT_ALLOWED")
        need(info.get("workflow_id") == WHEEL_DOWNLOAD_WORKFLOW,
             "NETWORK_DOWNLOAD_WORKFLOW_NOT_ALLOWED")
        need(info.get("network_scope") == "PYPI_JSON_AND_FILESHOSTED_HTTPS_ONLY",
             "NETWORK_DOWNLOAD_DOMAIN_SCOPE_REQUIRED")
        need(info.get("host_mutation") == "DOWNLOAD_ONLY_NO_INSTALL",
             "NETWORK_DOWNLOAD_NO_INSTALL_REQUIRED")
        return
    if info.get("package_id") == WHEEL_DOWNLOAD_PACKAGE:
        need(False,"NETWORK_DOWNLOAD_PROFILE_REQUIRED")
    if info.get("execution_profile") != "REMOTE_PUBLIC_TRUST_MUTATING":
        return
    need(info.get("workflow_id") == PUBLIC_TRUST_WORKFLOW,
         "PUBLIC_TRUST_WORKFLOW_NOT_ALLOWED")
    need(info.get("package_id") == PUBLIC_TRUST_PACKAGE,
         "PUBLIC_TRUST_PACKAGE_NOT_ALLOWED")
    need(info.get("owner_authorization_namespace") == PUBLIC_TRUST_NAMESPACE,
         "PUBLIC_TRUST_OWNER_NAMESPACE_REQUIRED")
    need(info.get("authorization_profile") == "SIGNED_OWNER_APPROVAL_REQUIRED",
         "PUBLIC_TRUST_OWNER_SIGNED_GATE_REQUIRED")


def check_hardware_runner_result_writers(info, members):
    """Static STOP for ambiguous mutating runner result ownership.

    This is a targeted negative gate, NOT signed or native hardware admission.
    A delegated second reporter can perform reset before its wrong gate ID
    reaches the parent deus-run result verifier.
    """
    if info.get("execution_profile") != "HARDWARE_MUTATING":
        return
    apply = check_apply(members["apply.ps1"])
    match = re.search(r"(?im)^\s*\$PackageId\s*=\s*['\"](?P<id>stm32_os_[a-z0-9_]+)['\"]\s*$", apply)
    need(match is not None, "HARDWARE_RESULT_GATE_ID_REQUIRED")
    need(match.group("id") == info["package_id"],
         "HARDWARE_RESULT_GATE_ID_DRIFT")
    for name, data in members.items():
        if name == "apply.ps1" or not name.casefold().endswith(".ps1"):
            continue
        need(b"DEUS_RUN_RESULT_PATH" not in data,
             "HARDWARE_MULTIPLE_RUNNER_RESULT_WRITERS:" + name)


def entries(blob):
    need(0 < len(blob) <= MAX_ZIP, "ZIP_SIZE_LIMIT")
    files, seen, total = {}, set(), 0
    try:
        with zipfile.ZipFile(io.BytesIO(blob)) as z:
            need(len(z.infolist()) <= MAX_MEMBERS, "MEMBER_COUNT_LIMIT")
            for item in z.infolist():
                name = item.filename
                valid_name(name)
                need(name.casefold() not in seen, "DUPLICATE_MEMBER_CASE")
                seen.add(name.casefold())
                need(not item.is_dir() and not stat.S_ISLNK(item.external_attr >> 16),
                     "SYMLINK_OR_DIRECTORY_FORBIDDEN")
                need(item.file_size <= MAX_MEMBER, "MEMBER_SIZE_LIMIT")
                need(item.flag_bits & 1 == 0, "ENCRYPTED_MEMBER")
                need(item.compress_type in (zipfile.ZIP_DEFLATED, zipfile.ZIP_STORED),
                     "BAD_COMPRESSION")
                need(item.file_size <= max(1024, item.compress_size * 800),
                     "SUSPICIOUS_ZIP_RATIO")
                total += item.file_size
                need(total <= MAX_ZIP, "TOTAL_UNCOMPRESSED_LIMIT")
                files[name] = z.read(item)
    except (zipfile.BadZipFile, RuntimeError, EOFError, zlib.error) as exc:
        raise Invalid("CORRUPT_ZIP_OR_CRC") from exc
    need(ROOT <= files.keys(), "MISSING_ROOT_FILES")
    return files


def validate_blob(blob, expected_name=None, expected_sha=None):
    if expected_sha is not None:
        need(isinstance(expected_sha, str) and HEX.fullmatch(expected_sha),
             "BAD_EXPECTED_SHA")
        need(digest(blob) == expected_sha.upper(), "WHOLE_ZIP_SHA_MISMATCH")
    files = entries(blob)
    need("QUARANTINED_NOT_FOR_RUN.txt" not in files,
         "QUARANTINED_SOURCE_NOT_FOR_BUILD")
    info = parse_json(files["manifest.json"])
    need(type(info) is dict and info.get("schema") == SCHEMA, "WRONG_MANIFEST_SCHEMA")
    pid = info.get("package_id")
    need(type(pid) is str and IDENT.fullmatch(pid), "INVALID_PACKAGE_ID")
    if expected_name is not None:
        need(Path(expected_name).name == pid + ".zip", "PACKAGE_NAME_DRIFT")
    need(info.get("runner_result_contract") == RUNNER, "RUNNER_CONTRACT_DRIFT")
    sha = info.get("apply_sha256")
    need(type(sha) is str and HEX.fullmatch(sha), "INVALID_APPLY_SHA")
    need(digest(files["apply.ps1"]) == sha.upper(), "MANIFEST_APPLY_SHA_DRIFT")
    check_apply(files["apply.ps1"])
    need(info.get("execution_profile") in PROFILES, "EXECUTION_PROFILE_REQUIRED")
    if info["execution_profile"] != "OFFLINE":
        need(type(info.get("workflow_id")) is str and
             re.fullmatch(r"[a-z0-9][a-z0-9_]{3,63}", info["workflow_id"]),
             "HARDWARE_WORKFLOW_ID_REQUIRED")
    validate_profile_scope(info)
    check_hardware_runner_result_writers(info, files)
    try:
        rows = files["hashes.sha256"].decode("ascii", "strict").splitlines()
    except UnicodeError as exc:
        raise Invalid("NONASCII_HASH_INDEX") from exc
    need(bool(rows) and all(rows), "EMPTY_HASH_INDEX_ROW")
    indexed = {}
    for row in rows:
        match = INDEX.fullmatch(row)
        need(match is not None, "MALFORMED_HASH_INDEX")
        name = match.group(2)
        valid_name(name)
        need(name not in indexed, "DUPLICATE_HASH_ENTRY")
        need(name != "hashes.sha256", "SELF_HASH_ENTRY")
        indexed[name] = match.group(1).upper()
    need(set(indexed) == set(files) - {"hashes.sha256"}, "INDEX_SET_MISMATCH")
    for name, hashval in indexed.items():
        need(digest(files[name]) == hashval, "INDEX_HASH_DRIFT:" + name)
    return {"package_id": pid, "sha256": digest(blob), "members": len(files),
            "apply_sha256": digest(files["apply.ps1"]), "profile": info["execution_profile"],
            "qualification": "STATIC_ONLY_NOT_NATIVE_OR_HARDWARE"}


def validate_file(path, expected_sha=None):
    return validate_blob(path.read_bytes(), path.name, expected_sha)


def build(src, dest):
    need(src.is_dir(), "SOURCE_DIRECTORY_REQUIRED")
    need(not (src / "QUARANTINED_NOT_FOR_RUN.txt").exists(),
         "QUARANTINED_SOURCE_NOT_FOR_BUILD")
    need(not dest.exists(), "REFUSE_OVERWRITE")
    members = {}
    for path in sorted(src.rglob("*")):
        if path.is_dir() and not path.is_symlink():
            continue
        need(path.is_file() and not path.is_symlink(), "SOURCE_SYMLINK_FORBIDDEN")
        name = path.relative_to(src).as_posix()
        valid_name(name)
        need(name not in ("manifest.json", "hashes.sha256"), "BUILDER_OWNS_MANIFEST")
        members[name] = path.read_bytes()
    need({"bundle_spec.json", "apply.ps1", "PACKAGE_README.txt"} <= members.keys(),
         "BUILD_INPUT_MISSING")
    spec = parse_json(members.pop("bundle_spec.json"))
    need(type(spec) is dict and spec.get("schema") == SCHEMA, "WRONG_MANIFEST_SCHEMA")
    pid = spec.get("package_id")
    need(type(pid) is str and IDENT.fullmatch(pid), "INVALID_PACKAGE_ID")
    need(dest.name == pid + ".zip", "PACKAGE_NAME_DRIFT")
    need(spec.get("runner_result_contract") == RUNNER, "RUNNER_CONTRACT_DRIFT")
    need(spec.get("execution_profile") in PROFILES, "EXECUTION_PROFILE_REQUIRED")
    if spec["execution_profile"] != "OFFLINE":
        need(type(spec.get("workflow_id")) is str and
             re.fullmatch(r"[a-z0-9][a-z0-9_]{3,63}", spec["workflow_id"]),
             "HARDWARE_WORKFLOW_ID_REQUIRED")
    validate_profile_scope(spec)
    check_apply(members["apply.ps1"])
    check_hardware_runner_result_writers(spec, members)
    spec["apply_sha256"] = digest(members["apply.ps1"])
    members["manifest.json"] = (json.dumps(spec, indent=2, sort_keys=True) + "\n").encode()
    members["hashes.sha256"] = "".join(
        digest(value) + "  " + key + "\n" for key, value in sorted(members.items())).encode("ascii")
    output = io.BytesIO()
    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, value in sorted(members.items()):
            zi = zipfile.ZipInfo(name, (2020, 1, 1, 0, 0, 0))
            zi.compress_type = zipfile.ZIP_DEFLATED
            zi.create_system = 3
            zi.external_attr = 0o100644 << 16
            z.writestr(zi, value, compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    payload = output.getvalue()
    validate_blob(payload, dest.name)
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_bytes(payload)
    result = validate_file(dest)
    need(result["sha256"] == digest(payload), "POSTWRITE_HASH_DRIFT")
    return result


def check_hardware_incident_blockers(payload):
    """A new v5 of a workflow with unresolved native failures is forbidden."""
    manifest = parse_json(entries(payload)["manifest.json"])
    if manifest["execution_profile"] == "OFFLINE":
        return
    workflow = manifest["workflow_id"]
    register = Path(__file__).resolve().with_name("incidents.json")
    need(register.is_file(), "HARNESS_REGISTRY_MISSING")
    catalog = parse_json(register.read_bytes())
    need(catalog.get("schema") == "DEUS_HARNESS_INCIDENT_REGISTRY_V1",
         "HARNESS_REGISTRY_SCHEMA_INVALID")
    blockers = [v["id"] for v in catalog["entries"]
                if v.get("scope") == workflow and
                   v.get("status") != "GUARDED_STATIC"]
    need(not blockers, "OPEN_NATIVE_PRIMITIVE_BLOCKERS:" + ",".join(blockers))


def windows_ast_parse(script_bytes):
    """Independent native PowerShell parser; never executes apply.ps1."""
    need(os.name == "nt", "NATIVE_WINDOWS_REQUIRED")
    import shutil
    pwsh = shutil.which("pwsh")
    need(pwsh is not None, "POWERSHELL_7_REQUIRED")
    with tempfile.TemporaryDirectory(prefix="deus_bundle_ast_") as temp:
        src = Path(temp) / "apply.ps1"
        driver = Path(temp) / "parse_syntax_only.ps1"
        src.write_bytes(script_bytes)
        driver.write_text(
            "param([Parameter(Mandatory=$true)][string]$LiteralPath)\n"
            "$e=$null; $t=$null\n"
            "[void][System.Management.Automation.Language.Parser]::ParseFile("
            "$LiteralPath,[ref]$t,[ref]$e)\n"
            "if ($e.Count -gt 0) { exit 72 }\nexit 0\n",
            encoding="utf-8")
        cp = subprocess.run([pwsh, "-NoProfile", "-NonInteractive", "-File",
                             str(driver), str(src)],
                            capture_output=True, text=True, timeout=15, check=False)
        need(cp.returncode == 0, "WINDOWS_POWERSHELL_AST_FAILED")


def release(path, expected_sha, proof):
    """Fail closed unless real Windows PowerShell AST and native primitive receipt.

    WARNING: receipt JSON is UNSIGNED; needs independent human verification.
    """
    res = validate_file(path, expected_sha)
    need(res["profile"] not in ("HARDWARE_MUTATING", "REMOTE_PUBLIC_TRUST_MUTATING"),
         "MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE")
    need(res["profile"] != "HOST_NETWORK_DOWNLOAD",
         "NETWORK_DOWNLOAD_NOT_A_RELEASE_ADMISSION")
    check_hardware_incident_blockers(path.read_bytes())
    windows_ast_parse(entries(path.read_bytes())["apply.ps1"])
    need(proof.is_file(), "NATIVE_PRIMITIVE_RECEIPT_MISSING")
    receipt = parse_json(proof.read_bytes())
    need(type(receipt) is dict and receipt.get("schema") == "DEUS_NATIVE_PRIMITIVE_RECEIPT_V1",
         "NATIVE_PROOF_SCHEMA_INVALID")
    need(receipt.get("bundle_sha256") == res["sha256"] and
         receipt.get("apply_sha256") == res["apply_sha256"], "NATIVE_PROOF_BYTE_DRIFT")
    need(type(receipt.get("windows_ps_ast_exit")) is int and
         receipt["windows_ps_ast_exit"] == 0 and
         type(receipt.get("windows_runner_smoke_exit")) is int and
         receipt["windows_runner_smoke_exit"] == 0, "WINDOWS_PRIMITIVES_NOT_PROVEN")
    if res["profile"] != "OFFLINE":
        need(type(receipt.get("linux_usb_primitive_exit")) is int and
             receipt["linux_usb_primitive_exit"] == 0 and
             receipt.get("linux_usb_primitive_host") == "macmini" and
             receipt.get("linux_usb_primitive_scope") == "EXACT_PAYLOAD_AND_REAL_DEVICE",
             "LINUX_USB_PRIMITIVE_NOT_PROVEN")
    res["qualification"] = "NATIVE_AST_AND_SELF_DECLARED_PRIMITIVE_RECEIPT_NOT_HARDWARE_ACCEPTANCE"
    return res


def main(argv=None):
    p = argparse.ArgumentParser(description="DEUS bundle constructor/validator. Never opens hardware.")
    commands = p.add_subparsers(dest="action", required=True)
    cmd = commands.add_parser("validate")
    cmd.add_argument("zip", type=Path)
    cmd.add_argument("--sha256")
    cmd = commands.add_parser("build")
    cmd.add_argument("source", type=Path)
    cmd.add_argument("zip", type=Path)
    cmd = commands.add_parser("release")
    cmd.add_argument("zip", type=Path)
    cmd.add_argument("--sha256", required=True)
    cmd.add_argument("--primitive-receipt", type=Path, required=True)
    # Independently assess ORIGINAL owner Evidence without rebuilding it.
    # This is not an equivalent to a native Windows writer negative test;
    # only known source/host evidence schemas can be assessed here.
    cmd = commands.add_parser("evidence")
    cmd.add_argument("zip", type=Path)
    cmd.add_argument("--sha256", required=True)
    args = p.parse_args(argv)
    try:
        if args.action == "build":
            result = build(args.source, args.zip)
        elif args.action == "release":
            result = release(args.zip, args.sha256, args.primitive_receipt)
        elif args.action == "evidence":
            from evidence_diagnostic_guard_v1 import (
                RejectedEvidence, diagnose_original_evidence)
            try:
                result = diagnose_original_evidence(args.zip, args.sha256)
            except RejectedEvidence as exc:
                raise Invalid("ORIGINAL_EVIDENCE_REJECTED:" + exc.reason) from exc
        else:
            result = validate_file(args.zip, args.sha256)
        print(json.dumps({"result": "PASS", **result}, sort_keys=True))
        return 0
    except (Invalid, OSError, subprocess.TimeoutExpired) as exc:
        print(json.dumps({"result": "FAIL", "classification": "HARNESS_AUTHORING_REJECTED",
                          "reason": str(exc)}, sort_keys=True), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
