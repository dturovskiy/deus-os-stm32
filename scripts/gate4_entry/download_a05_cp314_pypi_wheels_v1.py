#!/usr/bin/env python3
"""Explicit NETWORK download, never install: exactly 3 SHA-pinned Windows CPython wheels.

Uses only urllib stdlib and existing nonexecuting wheel verifier.
PyPI HTTPS release JSON -> exact expected file + release SHA -> canonical
files.pythonhosted.org HTTPS byte stream -> whole SHA + ZIP CRC + RECORD.
Commit one out-of-Git Wheelhouse ZIP only after all inputs verify.
"""
from __future__ import annotations

import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import urllib.error
import urllib.parse
import urllib.request
import uuid
import zipfile

from audit_a05_windows_wheelhouse_v1 import verify_wheel

MAX_METADATA = 2_000_000
MAX_WHEEL = 12_000_000
WHEELHOUSE_STEM = "a05_python314_regular_wheels_20261028"
PROFILES = {"cryptography": "cp311-abi3-win_amd64",
            "cffi": "cp314-cp314-win_amd64",
            "pycparser": "py3-none-any"}


class DownloadStop(Exception):
    pass


def need(ok: bool, reason: str):
    if not ok:
        raise DownloadStop(reason)


def digest(data: bytes):
    return hashlib.sha256(data).hexdigest().upper()


def _is_safe_parent(path: Path):
    p = path.absolute()
    for ancestor in (p, *p.parents):
        if ancestor.exists():
            need(not ancestor.is_symlink(), "SYMLINK_PATH_FORBIDDEN")
    need(p.is_absolute(), "ABSOLUTE_WHEELHOUSE_REQUIRED")
    return p


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        raise DownloadStop("UNEXPECTED_HTTPS_REDIRECT")


def _fetch(url: str, max_bytes: int, output=None):
    u = urllib.parse.urlsplit(url)
    if output is None:
        need(u.scheme == "https" and u.hostname == "pypi.org"
             and u.path.startswith("/pypi/") and u.path.endswith("/json")
             and not u.username and not u.password
             and not u.query and not u.fragment,
             "OFFICIAL_PYPI_METADATA_URL_ONLY")
    else:
        need(u.scheme == "https" and u.hostname == "files.pythonhosted.org"
             and u.path.startswith("/packages/") and u.path.endswith(".whl")
             and not u.username and not u.password and not u.query and not u.fragment,
             "OFFICIAL_PYPI_FILE_URL_ONLY")
    opener = urllib.request.build_opener(NoRedirect())
    req = urllib.request.Request(url, headers={
        "User-Agent": "DEUS-OS-A05-wheel-custody/1.0",
        "Accept": "application/json" if output is None else "application/octet-stream",
    })
    h = hashlib.sha256()
    count = 0
    buf = io.BytesIO() if output is None else None
    with opener.open(req, timeout=25) as resp:
        need(resp.status == 200, "PYPI_HTTP_NON_200")
        while True:
            part = resp.read(262144)
            if not part:
                break
            count += len(part)
            need(count <= max_bytes, "PYPI_RESPONSE_TOO_LARGE")
            h.update(part)
            if output is None:
                buf.write(part)
            else:
                output.write(part)
    return (buf.getvalue() if output is None else None, count, h.hexdigest().upper())


def _strict_json(pairs):
    out = {}
    for k, v in pairs:
        need(k not in out, "DUPLICATE_PYPI_JSON_KEY")
        out[k] = v
    return out


def select_exact_pypi_url(metadata: dict, package: str, version: str,
                          filename: str, sha256: str):
    info = metadata.get("info")
    need(isinstance(info, dict) and
         info.get("name", "").lower().replace("_", "-") == package and
         info.get("version") == version, "RELEASE_IDENTITY_MISMATCH")
    urls = metadata.get("urls")
    need(isinstance(urls, list), "MISSING_RELEASE_FILES")
    found = [x for x in urls if isinstance(x, dict) and x.get("filename") == filename]
    need(len(found) == 1, "EXACT_WHEEL_NOT_ON_PYPI")
    row = found[0]
    need(row.get("packagetype") == "bdist_wheel", "NOT_WHEEL_ON_PYPI")
    need(row.get("yanked") is False, "YANKED_PYPI_WHEEL")
    need(isinstance(row.get("digests"), dict) and
         str(row["digests"].get("sha256", "")).upper() == sha256,
         "PYPI_PUBLISHED_SHA_MISMATCH")
    need(isinstance(row.get("size"), int) and
         1000 <= row["size"] <= MAX_WHEEL, "PYPI_WHEEL_SIZE_BOUNDS")
    url = row.get("url")
    need(isinstance(url, str), "MISSING_OFFICIAL_FILE_URL")
    parts = urllib.parse.urlsplit(url)
    need(parts.scheme == "https" and parts.hostname == "files.pythonhosted.org" and
         parts.path.endswith("/" + filename) and
         parts.path.startswith("/packages/") and
         not parts.query and not parts.fragment, "PYPI_FILE_URL_WRONG_HOST")
    return url, row["size"]


def retrieve_three(stage: Path, output_dir: Path):
    selection = json.loads((stage / "a05_windows_python314_regular_wheel_selection_v1.json")
                           .read_text(encoding="utf-8"), object_pairs_hook=_strict_json)
    need(selection["schema"] == "DEUS_A05_WINDOWS_PYTHON314_REGULAR_WHEEL_SELECTION_V1",
         "SELECTION_SCHEMA_DRIFT")
    need(selection["owner_windows_inventory"]["target_wheel_abi"] == "cp314" and
         selection["owner_windows_inventory"]["gil_disabled"] is False,
         "NOT_REGULAR_CP314")
    wheels = selection["selected_wheels"]
    need([x["name"] for x in wheels] == ["cryptography", "cffi", "pycparser"],
         "EXACT_THREE_WHEELS_REQUIRED")
    output_dir = _is_safe_parent(output_dir)
    need(output_dir.name == "Wheelhouse" and
         output_dir.parent.is_dir(), "WHEELHOUSE_MUST_BE_PROJECT_SIBLING")
    output_dir.mkdir(exist_ok=True)
    run_id = uuid.uuid4().hex
    work = Path(tempfile.mkdtemp(prefix=".a05_wheel_stage_", dir=str(output_dir)))
    result = []
    archive_path = output_dir / f"{WHEELHOUSE_STEM}_{run_id}.zip"
    try:
        for entry in wheels:
            name, version, filename = (entry["name"], entry["version"], entry["filename"])
            expected = entry["sha256"].upper()
            need(name in PROFILES and filename.endswith("-" + PROFILES[name] + ".whl"),
                 "ABI_FILENAME_CONTRACT")
            need(len(expected) == 64 and all(c in "0123456789ABCDEF" for c in expected),
                 "INVALID_LOCK_SHA")
            url = f"https://pypi.org/pypi/{name}/{version}/json"
            raw, meta_size, _ = _fetch(url, MAX_METADATA)
            meta = json.loads(raw.decode("utf-8"), object_pairs_hook=_strict_json)
            wheel_url, advertised = select_exact_pypi_url(
                meta, name, version, filename, expected)
            target = work / filename
            with target.open("xb") as output:
                _, received, got = _fetch(wheel_url, MAX_WHEEL, output)
                output.flush()
                os.fsync(output.fileno())
            need(advertised == received, "WHEEL_PYPI_SIZE_MISMATCH_" + name)
            need(got == expected and digest(target.read_bytes()) == expected,
                 "WHEEL_SHA_MISMATCH_" + name)
            # Reuses independent, non-executing wheel inner RECORD verifier.
            verified = verify_wheel(target, expected, name, version, PROFILES[name])
            result.append({"filename": filename, "sha256": expected,
                           "bytes": received, "record_count": verified["records"],
                           "source": "PYPI_HTTPS_WITH_PINNED_SHA"})
            print("WHEEL_DOWNLOAD_VERIFIED=" + name + " SHA256=" + expected +
                  " BYTES=" + str(received), flush=True)
        sums = "".join(x["sha256"] + "  " + x["filename"] + "\n" for x in result)
        (work / "SHA256SUMS.txt").write_text(sums, encoding="ascii")
        # A ZIP makes the three actual .whl files easy to upload together.
        # Include a verifiable index but never any private Windows path.
        with zipfile.ZipFile(archive_path, "x", zipfile.ZIP_STORED,
                             allowZip64=False) as archive:
            for entry in result:
                archive.write(work / entry["filename"], entry["filename"])
            archive.write(work / "SHA256SUMS.txt", "SHA256SUMS.txt")
        with zipfile.ZipFile(archive_path) as readback:
            need(len(readback.namelist()) == 4 and readback.testzip() is None,
                 "UPLOAD_ARCHIVE_CRC_OR_MEMBER_COUNT")
            for entry in result:
                need(digest(readback.read(entry["filename"])) == entry["sha256"],
                     "UPLOAD_ARCHIVE_BYTE_DRIFT")
        return {"schema": "DEUS_A05_WHEELHOUSE_DOWNLOAD_RECEIPT_V1",
                "result": "PASS", "status": "THREE_WHEELS_DOWNLOADED_AND_VERIFIED_NO_INSTALL",
                "wheelhouse_archive_basename": archive_path.name,
                "wheelhouse_archive_sha256": digest(archive_path.read_bytes()),
                "wheelhouse_archive_bytes": archive_path.stat().st_size,
                "wheels": result, "downloaded_files": 3, "installed_files": 0,
                "network_scope": "PYPI_METADATA_AND_FILESHOSTED_ONLY",
                "no_python_environment_change": True}
    except BaseException:
        if archive_path.exists():
            archive_path.unlink()
        raise
    finally:
        shutil.rmtree(work, ignore_errors=True)


def main():
    try:
        need(os.name == "nt", "WINDOWS_ONLY")
        need(len(sys.argv) == 2, "EXACT_OUTPUT_ROOT_ARG_REQUIRED")
        stage = Path(__file__).resolve().parent
        result = retrieve_three(stage, Path(sys.argv[1]))
        print("DEUS_WHEEL_DOWNLOAD_JSON=" + json.dumps(
            result, sort_keys=True, separators=(",", ":")), flush=True)
        return 0
    except BaseException as exc:
        msg = str(exc).replace("\r", " ").replace("\n", " ")
        if len(msg) > 400:
            msg = msg[:90] + " [MIDDLE_TRUNCATED_TAIL_PRESERVED] " + msg[-250:]
        failure = {"result": "FAIL", "exception_type": type(exc).__name__,
                   "terminal_message": msg, "download_installed": False}
        print("DEUS_WHEEL_DOWNLOAD_JSON=" + json.dumps(
            failure, sort_keys=True, separators=(",", ":")), flush=True)
        print("TERMINAL_EXCEPTION=" + type(exc).__name__ + ": " + msg, flush=True)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
