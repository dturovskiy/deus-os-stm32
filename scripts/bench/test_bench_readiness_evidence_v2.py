#!/usr/bin/env python3
"""Offline, side-effect-contained evidence diagnostics regression for bench v2.

No board/USB/SSH, no signed-package reads, no production runner. The PowerShell
phase executes only extracted helper definitions with synthetic child commands.
NATIVE WINDOWS remains unqualified even when Linux pwsh is available.
"""
from __future__ import annotations
from io import BytesIO
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[2]
STAGE = ROOT.parent / "Bundle" / "_stage_stm32_os_bench_single_run_readiness_v2_20261029"
SRC = STAGE / "apply.ps1"
SPEC = STAGE / "bundle_spec.json"
PIN = "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"


def check(ok, code):
    if not ok:
        raise AssertionError(code)


def checks_static(s, spec):
    expected = "stm32_os_bench_single_run_readiness_v2_20261029"
    assertions = {
        "identity": expected in s and spec["package_id"] == expected,
        "profile": spec["execution_profile"] == "HARDWARE_READONLY",
        "runner_contract": spec["runner_result_contract"] == "DEUS_OS_RUNNER_RESULT_V1",
        "candidate_pin": PIN in s,
        "bounded_process": all(x in s for x in ("WaitForExit($limitMs)", "Kill($true)", "ReadToEndAsync()", "25000", "22000")),
        "parent_diagnostics": all(x in s for x in ("TERMINAL_EXCEPTION=", "failure_phase=$phase", "SEAL_CHECKS=", "CHECKS=")),
        "captured_streams": all(x in s for x in ("windows_stdout.txt", "windows_stderr.txt", "ssh_stdout.txt", "ssh_stderr.txt")),
        "tail_preserved": "[MIDDLE_TRUNCATED_TAIL_PRESERVED CHARS_OMITTED=" in s,
        "negative_check": "Add('FAIL '+$name)" in s,
        "readonly_ssh": "lsusb -d 1209:000c" in s and "lsusb -d 1209:000d" in s,
        "no_mutating_commands": not re.search(r"['\"]bench-fw['\"]\s*,\s*['\"](?:enter|update)['\"]", s),
        "exact_native_output": "DEVICE_IO=NONE$" in s and " -cmatch " in s,
        "single_result_writer": s.count("NewText ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))") == 1,
        "child_status_separation": "ATTEMPTED_NO_RESULT" in s and "'NOT_RUN'" in s,
        "failure_diagnostic_seal": "EVIDENCE_SEAL_FAILED" in s and "FIRST_FAILURE=" in s,
    }
    for name, result in assertions.items():
        check(result, "STATIC_" + name.upper())
    return len(assertions)


def verify_evidence(path):
    check(path.is_file(), "EVIDENCE_ABSENT")
    with ZipFile(path) as z:
        files = z.namelist()
        check(len(files) == 9 and len(files) == len(set(files)), "MEMBER_COUNT_OR_DUPLICATE")
        check(z.testzip() is None, "ZIP_CRC")
        blobs = {n: z.read(n) for n in files}
    def index(name):
        values = {}
        for line in blobs[name].decode("utf-8").splitlines():
            value, rel = line.split("  ", 1)
            check(rel not in values, "INDEX_DUPLICATE")
            values[rel] = value
        return values
    hashes = index("hashes.sha256")
    sizes = index("sizes.txt")
    check(set(hashes) == set(blobs)-{"hashes.sha256"}, "HASH_COVERAGE")
    check(set(sizes) == set(blobs)-{"hashes.sha256", "sizes.txt"}, "SIZE_COVERAGE")
    for name, h in hashes.items():
        check(hashlib.sha256(blobs[name]).hexdigest().upper() == h.upper(), "SHA_"+name)
    for name, length in sizes.items():
        check(len(blobs[name]) == int(length), "SIZE_"+name)
    outcome = json.loads(blobs["outcome.txt"].decode("utf-8"))
    check(outcome["result"] == "FAIL" and outcome["failure_phase"] == "SYNTHETIC_FAILURE", "FAIL_OUTCOME")
    check(outcome["check_fail_count"] == 1 and outcome["check_pass_count"] == 1, "CASE_COUNTS")
    check(outcome["physical_observation"] == "NOT_ATTESTED", "NO_FABRICATED_PHYSICAL_RESULT")
    check(outcome['child_status']['WINDOWS'].startswith('EXIT=13') and outcome['child_status']['SSH'] == 'NOT_RUN', 'CHILD_STATE_MISSING')
    check("ModuleNotFoundError: No module named 'cryptography'" in outcome["terminal_exception"], "TERMINAL_CAUSE")
    check(b"FAIL NEGATIVE_INJECTED" in blobs["native_checks.txt"], "FAILED_CHECK_SEALED")
    check(b"MIDDLE_TRUNCATED_TAIL_PRESERVED" in blobs["windows_stderr.txt"], "TAIL_MARKER")
    check(b"ModuleNotFoundError: No module named 'cryptography'" in blobs["windows_stderr.txt"], "TAIL_EXCEPTION")
    check(b"fake stdout" in blobs["windows_stdout.txt"], "CHILD_STDOUT")
    check(b"fake ssh stderr" in blobs["ssh_stderr.txt"], "CHILD_STDERR")
    return len(files)


def python_schema_negative_fixtures():
    """Tests the independent index/semantic reader, not the PowerShell writer."""
    canonical = {
        "outcome.txt": json.dumps({
            "schema":"DEUS_BENCH_CROSS_HOST_READINESS_V1",
            "gate":"bench_synthetic_fail", "result":"FAIL",
            "classification":"SYNTHETIC_FAIL", "failure_phase":"SYNTHETIC_FAILURE",
            "error":"MOCK_MISSING_DEPENDENCY",
            "terminal_exception":"ModuleNotFoundError: No module named 'cryptography'",
            "check_pass_count":1, "check_fail_count":1,
            "physical_observation":"NOT_ATTESTED",
            "child_status":{"WINDOWS":"EXIT=13;TIMEOUT=False","SSH":"NOT_RUN"},
        }).encode("utf-8"),
        "run.log":b"TERMINAL_EXCEPTION=ModuleNotFoundError: No module named 'cryptography'\n",
        "native_checks.txt":b"PASS BASELINE\nFAIL NEGATIVE_INJECTED\n",
        "windows_stdout.txt":b"fake stdout",
        "windows_stderr.txt":b"x [MIDDLE_TRUNCATED_TAIL_PRESERVED CHARS_OMITTED=8000] ModuleNotFoundError: No module named 'cryptography'",
        "ssh_stdout.txt":b"NOT_RUN",
        "ssh_stderr.txt":b"fake ssh stderr",
    }

    def fixture_bytes(changes=None, invalidate_index=False):
        members = dict(canonical)
        members.update(changes or {})
        members["sizes.txt"] = b"".join(
            (str(len(content))+"  "+name+"\n").encode("ascii")
            for name, content in sorted(members.items()))
        hashes = b"".join(
            (hashlib.sha256(content).hexdigest().upper()+"  "+name+"\n").encode("ascii")
            for name, content in sorted(members.items()))
        if invalidate_index:
            hashes = (b"0" if hashes[:1] != b"0" else b"F") + hashes[1:]
        members["hashes.sha256"] = hashes
        result = BytesIO()
        from zipfile import ZIP_DEFLATED
        with ZipFile(result, "w", ZIP_DEFLATED) as z:
            for name, content in sorted(members.items()):
                z.writestr(name, content)
        return result.getvalue()

    def expected_reject(path, code, changes=None, invalidate_index=False):
        path.write_bytes(fixture_bytes(changes, invalidate_index))
        try:
            verify_evidence(path)
        except AssertionError as exc:
            check(code in str(exc), "WRONG_REJECTION:"+str(exc))
        else:
            raise AssertionError("UNEXPECTED_SEMANTIC_PASS:"+code)

    with tempfile.TemporaryDirectory(prefix="deus_bench_schema_negatives_") as temp:
        path = Path(temp)/"fixture.zip"
        path.write_bytes(fixture_bytes())
        check(verify_evidence(path) == 9, "BASELINE_SYNTHETIC_FIXTURE")
        outcome = json.loads(canonical["outcome.txt"])
        outcome["terminal_exception"] = "NONE"
        expected_reject(path, "TERMINAL_CAUSE", {"outcome.txt":json.dumps(outcome).encode()})
        expected_reject(path, "TAIL_EXCEPTION", {"windows_stderr.txt":b"prefix [MIDDLE_TRUNCATED_TAIL_PRESERVED] lost"})
        expected_reject(path, "TAIL_MARKER", {"windows_stderr.txt":b"ModuleNotFoundError: No module named 'cryptography'"})
        expected_reject(path, "FAILED_CHECK_SEALED", {"native_checks.txt":b"PASS BASELINE\n"})
        expected_reject(path, "SHA_", invalidate_index=True)
    return 6


def powershell_synthetic(s, pwsh):
    marker = "\ntry{\n  $phase='WINDOWS_PRECONDITIONS'"
    check(s.count(marker) == 1, "FUNCTION_BOUNDARY")
    start = s.index("function Need(")
    body = s[start:s.index(marker)]
    with tempfile.TemporaryDirectory(prefix="deus_bench_v2_offline_") as tmp:
        tmp_dir = Path(tmp)
        root = tmp_dir / "Evidence"
        root.mkdir()
        driver = tmp_dir / "driver.ps1"
        driver.write_text(
            "Set-StrictMode -Version 3\n$ErrorActionPreference='Stop'\n"
            + body + "\n"
            + "$utf=[Text.UTF8Encoding]::new($false,$true);$nl=[Environment]::NewLine\n"
            + "$PackageId='bench_synthetic_fail';$evidenceRoot='" + str(root).replace("'", "''") + "'\n"
            + "$startedAt=(Get-Date).ToString('o');$script:sealChecks=0\n"
            + "$outcome='FAIL';$class='SYNTHETIC_FAIL';$phase='SYNTHETIC_FAILURE'\n"
            + "$reason='MOCK_MISSING_DEPENDENCY';$lastError=\"ModuleNotFoundError: No module named 'cryptography'\"\n"
            + "$cliSha='NOT_RUN';$sourceSha='SYNTHETIC';$report=[Collections.Generic.List[string]]::new()\n"
            + "$checks=[Collections.Generic.List[string]]::new()\n"
            + "$childStatus=[ordered]@{WINDOWS='EXIT=13;TIMEOUT=False';SSH='NOT_RUN'}\n"
            + "$childStreams=[ordered]@{windows_stdout='fake stdout';windows_stderr='';ssh_stdout='NOT_RUN';ssh_stderr='fake ssh stderr'}\n"
            + "Check 'BASELINE' $true\n"
            + "try{Check 'NEGATIVE_INJECTED' $false;throw 'FALSE_PASS'}catch{"
            + " if($_.Exception.Message -notmatch 'CHECK_FAILED_NEGATIVE_INJECTED'){throw}}\n"
            + "$long=('x'*6500)+[Environment]::NewLine+\"ModuleNotFoundError: No module named 'cryptography'\"\n"
            + "$childStreams.windows_stderr=Clean $long 1800\n"
            + "if($childStreams.windows_stderr -notmatch 'MIDDLE_TRUNCATED_TAIL_PRESERVED'){throw 'NO_TRUNCATION_MARKER'}\n"
            + "if($childStreams.windows_stderr -notmatch 'No module named'){throw 'LOST_TERMINAL_EXCEPTION'}\n"
            + "$self=[Diagnostics.Process]::GetCurrentProcess().MainModule.FileName\n"
            + "$child=RunChild $self @('-NoProfile','-NonInteractive','-Command','[Console]::Out.Write(\"synthetic-out\");[Console]::Error.Write(\"synthetic-err\");exit 13') 10000\n"
            + "if($child.Exit -ne 13 -or $child.TimedOut -or $child.Stdout -notmatch 'synthetic-out' -or $child.Stderr -notmatch 'synthetic-err'){throw 'FAILED_CHILD_CAPTURE'}\n"
            + "$slow=RunChild $self @('-NoProfile','-NonInteractive','-Command','Start-Sleep -Seconds 3') 100\n"
            + "if(-not $slow.TimedOut -or $slow.Exit -ne -99){throw 'TIMEOUT_NOT_CLASSIFIED'}\n"
            + "try{$nil=RunChild '__DEUS_TEST_MISSING_EXE__' @() 1000;throw 'FALSE_MISSING_EXE_PASS'}catch{if($_.Exception.Message -match 'FALSE_MISSING_EXE_PASS'){throw}}\n"
            + "$sealed=SealEvidence\n"
            + "if($script:sealChecks -ne 10){throw 'BAD_SEAL_CHECK_COUNT'}\n"
            + "$before=$evidenceRoot;$evidenceRoot=Join-Path $before 'absent'\n"
            + "try{$nil=SealEvidence;throw 'FALSE_SEAL_PASS'}catch{if($_.Exception.Message -notmatch 'EVIDENCE_ROOT_MISSING'){throw}}\n"
            + "Write-Output ('OFFLINE_EVIDENCE='+$sealed.Path)\n",
            encoding="utf-8",
        )
        ast = tmp_dir / "ast.ps1"
        ast.write_text(
            "param([string]$P)\n$tokens=$null;$errors=$null\n"
            + "[void][System.Management.Automation.Language.Parser]::ParseFile($P,[ref]$tokens,[ref]$errors)\n"
            + "if($errors.Count -gt 0){$errors | ForEach-Object {Write-Error $_};exit 72}\n"
            + "Write-Output 'AST=PASS'\n", encoding="utf-8",
        )
        for target in (SRC, driver):
            proc = subprocess.run([pwsh, "-NoLogo", "-NoProfile", "-NonInteractive", "-File", str(ast), str(target)],
                                  text=True, capture_output=True, timeout=15, check=False)
            check(proc.returncode == 0, "POWERSHELL_AST_"+target.name+":"+proc.stderr[-400:])
        result = subprocess.run([pwsh, "-NoLogo", "-NoProfile", "-NonInteractive", "-File", str(driver)],
                                text=True, capture_output=True, timeout=30, check=False)
        check(result.returncode == 0, "POWERSHELL_SYNTHETIC:"+result.stderr[-1500:]+result.stdout[-600:])
        evidence_line = [l for l in result.stdout.splitlines() if l.startswith("OFFLINE_EVIDENCE=")]
        check(len(evidence_line) == 1, "NO_EVIDENCE_RECEIPT")
        count = verify_evidence(Path(evidence_line[0].split("=",1)[1]))
        return {"powershell_ast":"PASS", "synthetic_child_fail_timeout_missing_exe":"PASS",
                "synthetic_evidence_crc_sha_semantics":"PASS", "original_zip_members":count}


def offline_operator_static_checks():
    stage = ROOT.parent / "Bundle" / "_stage_stm32_os_bench_evidence_offline_negative_v1_20261029"
    payload = stage / "bench_v2_source.ps1"
    source = (stage/"apply.ps1").read_text(encoding="utf-8")
    spec = json.loads((stage/"bundle_spec.json").read_text(encoding="utf-8"))
    pinned = "E1C556CA831F054D625B335C8C9FF2EA48A487593A9C93015E5495D5E020AF37"
    expectations = {
        "exact_helper_bytes": hashlib.sha256(payload.read_bytes()).hexdigest().upper() == pinned,
        "exact_source_pair": payload.read_bytes() == SRC.read_bytes(),
        "operator_pin": pinned in source,
        "offline_profile": spec["execution_profile"] == "OFFLINE",
        "package_id": "stm32_os_bench_evidence_offline_negative_v1_20261029" in source,
        "no_direct_device_run": not re.search(r"RunChild\s+['\"](?:ssh|lsusb|st-info|STM32CubeProgrammer)",source,re.I),
        "no_bench_fw_actual_run": not re.search(r"RunChild[^\n]*bench-fw",source,re.I),
        "source_ast_pin": "[System.Management.Automation.Language.Parser]::ParseFile" in source,
        "synthetic_timeout": "Start-Sleep -Seconds 3" in source and " 100" in source,
        "typed_fail_record": "terminal_exception=$exception" in source and "test_fail_count=" in source,
        "evidence_indexed": "hashes.sha256" in source and "sizes.txt" in source,
        "positive_and_negative_seal": "SealOriginal" in source and "EVIDENCE_SEAL_FAILED" in source,
        "runner_single_result": source.count("Put ([IO.Path]::GetFullPath($env:DEUS_RUN_RESULT_PATH))") == 1,
        "no_flash_commands": not re.search(r"(?im)^\s*(?:st-flash|STM32CubeProgrammer|dfu-util|openocd)\b",source),
    }
    for key, value in expectations.items():
        check(value, "OFFLINE_OPERATOR_"+key.upper())
    return len(expectations)


def main():
    s = SRC.read_text(encoding="utf-8")
    spec = json.loads(SPEC.read_text(encoding="utf-8"))
    count = checks_static(s, spec)
    offline_operator_checks = offline_operator_static_checks()
    synthetic_schema_cases = python_schema_negative_fixtures()
    pwsh = shutil.which("pwsh")
    tests = powershell_synthetic(s, pwsh) if pwsh else {
        "powershell_ast": "NOT_RUN_PWSH_UNAVAILABLE",
        "synthetic_child_fail_timeout_missing_exe": "NOT_RUN_PWSH_UNAVAILABLE",
        "synthetic_evidence_crc_sha_semantics": "NOT_RUN_PWSH_UNAVAILABLE",
    }
    print(json.dumps({"result":"PASS_STATIC_ONLY" if not pwsh else "PASS_LINUX_OFFLINE_SYNTHETIC",
                      "static_checks":count, "offline_operator_static_checks":offline_operator_checks,
                      "synthetic_schema_cases":synthetic_schema_cases,
                      "synthetic_schema_scope":"PYTHON_READER_ONLY_NOT_POWERSHELL_WRITER",
                      "pwsh_available":bool(pwsh), **tests,
                      "native_windows":"NOT_ATTESTED", "device_io":"NONE"},sort_keys=True))
    return 0

if __name__ == "__main__":
    try:
        sys.exit(main())
    except (AssertionError, OSError, ValueError, subprocess.TimeoutExpired) as exc:
        print(json.dumps({"result":"FAIL","error_type":type(exc).__name__,
                          "error":str(exc)}),file=sys.stderr)
        sys.exit(1)
