# Gate3 one-reset/one-INFO v2 — integrated source-only candidate

**State: UNRELEASED, QUARANTINED, NO HARDWARE EXECUTION.** The old signed v1 stage in Bundle remains byte-identical and quarantined. This directory is **not** a canonical `deus-run` ZIP, `apply.ps1` hardware runner or new owner signature.

Integrated code seams:

- `Program.cs`: exact rev5 installed runtime source-tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd` and FirmwareUpdate capability before its only possible ENTER. `HardwareExecutionAdmitted=false` blocks all hardware commands; `selftest` remains offline.
- `remote_handoff_v3.py`: imports and hash-pins the **byte-identical** 59-test `remote_result_guard.py`; verifies exact unique ENTER, INFO and complete remote receipt, not substrings. The `__main__` entry is deliberately blocked even if invoked with legacy owner-token arguments.
- `remote_receipt_bridge.py`: remains a Python **Linux-side offline reference bridge** for original JSON bytes, duplicate-key rejection and guard.adjudicate. It is NOT required or invoked by the Windows acceptance path. No SSH, sudo or USB.
- `WindowsNativeJsonGuard.psm1` parses original raw remote JSON with PowerShell 7 / built-in `.NET System.Text.Json`, rejects duplicate keys and strictly validates ENTER/INFO/USB/source pins; `WindowsResultOwner.psm1` imports it and writes one fail-only `DEUS_OS_RUNNER_RESULT_V1` with SHA-checked evidence. Even a structurally valid receipt is downgraded to `SCOPED_RECEIPT_VALID_SOURCE_ONLY_NOT_RELEASED`; no hardware PASS is possible. **Python is not required on Windows.** It cannot launch SSH or a device.

**Proof already obtained:** 12/12 targeted Python source/receipt bridge checks on DEUS MCP Linux, including rejection of stale provisional PASS, with `SSH=NO USB=NO RESET=NO FLASH=NO`. The unchanged owner Windows-native Host 168/168, offline result guard 59/59 and standalone Windows fail-owner 10/10 remain separate, reusable historical proof, not assertions about this new composition.

**Native Windows source-only qualification for THESE exact v2 bytes: PASS (owner PowerShell terminal).** Owner confirmed the nine-file SHA inventory and PowerShell native AST; four-project C# Release `--warnaserror` build and compiled `selftest` both passed. Following the one-line harness fix, six native Windows bridge/evidence scenarios also passed: `NOT_STARTED`, `SSH_TIMEOUT`, `RAW_DUPLICATE`, `REMOTE_EXIT_FAILED`, `VALID_SCOPED_RECEIPT`, and `WRONG_RUNTIME_TREE`. Final `G3_SOURCE_V2_WINDOWS_BRIDGE=PASS COUNT=6 LIVE_USB=NO SSH=NO RESET=NO FLASH=NO`, `GATE3_V2_WINDOWS_BRIDGE_SCOPED=PASS`. The corrected test has SHA256 `6F4325EA5575506FB3EA8007094C59F1B76892DEBF119E15E6DE3099D02EC0DB`. This is terminal-only synthetic qualification: temporary evidence ZIPs are deleted and are not an original preserved native Gate3 evidence archive. A physical `apply.ps1` entrypoint with exactly one source-bound result writer is **not yet built or released**; it would require a separate authorized physical operator source, full native PASS/FAIL evidence, new owner signing and a separately admitted execution lane. Do not promote this prototype merely by removing the guards.

The complete **source-only physical entrypoint design and release-blocker handoff** is `PHYSICAL_OPERATOR_V2_AUTHORING_HANDOFF.md`. It specifies exactly one Windows/SSH/Ubuntu physical flow and evidence contract but is not executable. Client-layer denial of prior physical/synthetic executable generators must not be circumvented by creating an alternative runnable wrapper or ZIP.

**Unsigned source inventory:** `source_candidate_lock.json` records byte sizes/hashes for **nine** reviewed source files but is NOT an Ed25519 signature or an authorization. The directory contains `QUARANTINED_NOT_FOR_RUN.txt` to prevent canonical bundle_guard building from this source root.

**Targeted non-device checks from OS root:**

```text
python scripts/gate3_operator/source_candidate_v2/test_source_integration.py
```

Windows PowerShell 7: verify the nine-file unsigned SHA inventory using native `Get-FileHash`, parse both `.psm1` and the `.ps1` through the native AST, build the source-only C# project, and run `test_windows_bridge_offline.ps1`. **Do not invoke Python on Windows**. Do not use the original hardware `apply.ps1` or issue any device command.
