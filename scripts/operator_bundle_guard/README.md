# DEUS OS — operator bundle single-authority guard (V1)

**Normative parent:** `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`. This directory is executable tooling, not a second project-state source of truth. **New operator ZIPs are authored only via `bundle_guard.py build`.** No handwritten/final-manifest patching after the package is built; `build` refuses overwrite. This is an additive construction/validation tool; **the separately installed, Windows-native accepted `deus-run v1.3.1` (Bundle locator upgrade, 2026-10-25) remains unchanged by this tooling** and is the sole Windows operator launcher.

**Sequential delivery invariant (owner rule, 2026-10-24):** Exactly **ONE** operator bundle for the current task, followed by exactly **ONE** admitted `deus-run` command. Stop and inspect the original result/evidence before authoring, importing, handing off or publishing any successor bundle. No parallel variants, no speculative follow-on ZIP, no second command. If two bundles were accidentally produced, keep the follow-on candidate **QUARANTINED / NOT FOR RUN OR PUBLICATION**. The canonical and more detailed rule is in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

**Owner evidence root 2026-10-24:** all new Windows operator evidence producers target the **existing canonical OS checkout sibling `Evidence`**, alongside `Tools` and `Logs`, OUTSIDE Git (no fallback to Downloads/TEMP/repo). New operator ZIPs are built directly into an independently verified owner-local off-Git staging root. A changed installed Windows runner lookup root requires native acceptance before any staged ZIP is described as runnable. The child still emits the actual absolute evidence path/SHA for runner verification. No automatic relocation of historical evidence, no firmware or secrets in `Evidence`. Canonical path/security/negative rules: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

**Prospective Gate-3 evidence requirement (2026-10-24):** every new custody/recovery-related Windows OFFLINE/native operator run must produce an original, integrity-indexed `.evidence.zip` on PASS **and** classified FAIL, not `NONE/NONE`. Its child must seal and hash `run.log`, `outcome.txt`, bounded safe native stdout/stderr and metadata, preserve phase-aware error evidence and report the actual archive path/SHA via `DEUS_RUN_RESULT_PATH`. The historical generic runner-handshake exception does **not** qualify real toolchain or protected-store preflight. Explicitly distinguish SKIPPED/NOT_CONFIGURED from checks PASSED. Full constraints and permitted historical exception are in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

## Explicit limited host-network download profile — 2026-10-28

The operator builder's `HOST_NETWORK_DOWNLOAD` profile is **NOT `OFFLINE`**. `bundle_guard.py` permits it **only** for package `stm32_os_gate4_a05_python314_wheel_download_v1_20261028`, workflow `gate4_a05_pypi_wheel_acquisition`, source manifest network scope `PYPI_JSON_AND_FILESHOSTED_HTTPS_ONLY`, and `host_mutation=DOWNLOAD_ONLY_NO_INSTALL`. Generic network operator IDs, alternate scopes, source relabeling as OFFLINE, and install requests are rejected during build and validate. `bundle_guard release` rejects this profile as it is an acquisition workflow, **not release admission**.

Actual source `scripts/gate4_entry/download_a05_cp314_pypi_wheels_v1.py` restricts HTTPS hosts and paths in Python, denies redirects/yanked releases, accepts exactly three pinned Windows regular cp314 wheels, checks both current official PyPI JSON hashes and actual downloaded bytes, CRC, wheel tags, METADATA and full PEP376 RECORD before creating one uploadable ZIP under workspace-parent `Wheelhouse`. It does not execute, import or install wheels, run pip, change Python, use keys, nonce or hardware. Own original indexed Windows PASS/FAIL Evidence is mandatory. Builder STATIC PASS does not establish Windows runner profile support or actual outbound network reachability. Profile and source-only tests: five additional guard negative/positive cases, 8 mocked downloader source tests; GitHub CI configured but actual execution not observed.

## Real Windows-native evidence-writer qualification — 2026-10-28 22:50

One genuine owner Windows `deus-run` input OFFLINE ZIP SHA `3603821E93217EE9617193D70E6868BFE933A82CDD175E2D7A3A314761F9DD93` returned PASS after native PowerShell writer tests. **Original**, immutable Evidence `Evidence/stm32_os_evidence_writer_native_offline_v1_20261028_20261028_225007_bdf95aa0c2ea4bd3ba4f3afdda259b53.evidence.zip` 8,644 B SHA `63D9D6567A98979DD4C2D2DE11677AC52039E2117A8B2A980F9BA92CFFE4AB6D` independently audited by `audit_writer_native_original_v1.py`: outer CRC10/10 SHA9/9 size8/8, 8/8 original native markers, **4/4 original nested indexed sample ZIPs** with proper FAIL terminal exception and exit17, timeout, prechild STOP and PASS. This proves the **exact writer's native failure-path** only, not all future writer formats/implementations, a production controller, Windows `cryptography` readiness or Gate4 authorization. Optional original off-Git verification is regression-tested with clean-CI skip only when original owner Evidence is absent. Report in off-Git Gate4/Readiness. No Git commit or push.

## Source-only original Evidence semantic guard — 2026-10-28

`evidence_diagnostic_guard_v1.py` is a read-only, independently SHA-pinned Evidence ZIP auditor for the six-member `outcome.txt / native_test_output.txt / native_checks.txt / run.log` evidence format with `hashes.sha256` and `sizes.txt`. It checks original full SHA/CRC/index and **then** tests whether a typed native FAIL retains a short original exception cause instead of only a prefix-truncated traceback. It deliberately rejects the original A-05 controller Windows v4 Evidence as `DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY` (without modifying original bytes). Its `test_evidence_diagnostic_guard_v1.py` contains 17 positive/negative source-level tests including the real owner original v4 negative, synthetic trace-tail preservation, missing exception, duplicate/index corruption, timeout and clean PASS. **17/17 managed DEUS MCP Python PASS.**

**Actual guard integration 2026-10-28:** `bundle_guard.py build` **and** `validate` now reject the known bad diagnostic variable assignment `$one=$one.Substring(0,600)` (and equivalent prefix-only diagnostic variables) as `EVIDENCE_PREFIX_ONLY_TRUNCATION_FORBIDDEN`. A same-statement head+tail expression with explicit `MIDDLE_TRUNCATED_TAIL_PRESERVED` remains only **STATIC_ALLOWED**, never native-qualified. An explicit `bundle_guard.py evidence <original.evidence.zip> --sha256 <64_HEX_ORIGINAL_EVIDENCE_SHA>` runs independent, read-only integrity **and semantic** analysis on the supported six-member host-source Evidence schema. It refuses authentic A-05 v4 as `ORIGINAL_EVIDENCE_REJECTED:DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY`; the source test checks this exact CLI result. Guard regression suite: **53 tests / 52 PASS, 1 native-Windows-only SKIP**; independent semantic suite **15/15 PASS** via managed DEUS MCP. GitHub Actions workflow invokes both through `unittest discover` and syntax-compiles both sources. The tests referring to historical off-Git Evidence SKIP only when original private Windows archive is absent (clean public CI); synthetic negatives run unconditionally.

**Trust limitation:** both the CLI and the anti-pattern builder check are **SOURCE_ONLY** controls. Neither automatically proves a new Windows writer emits diagnostically complete FAIL Evidence, nor substitutes for original Windows selftest with real produced indexed FAIL ZIP. Other evidence schemas need their own reviewed semantic assessor; do not infer complete Evidence from plain ZIP CRC, source pin, pass marker counts or `STATIC_ALLOWED`.

## Build-task result retrieval uncertainty — 2026-10-28

**A response-fetch failure is not a failed ZIP build.** If DEUS MCP accepted a `bundle_guard.py build` managed task but subsequently cannot return its task result, retain the original task ID and diagnostic; **do not blindly rebuild, assume missing file, or call the target/Gate FAILED**. Inspect the same output ZIP via **read-only** `stat_many` (presence, size, exact whole SHA) and independently perform normal `bundle_guard.py validate <existing_zip> --sha256 <observed_sha>` when permitted. Record the actual observed result and provenance: `ARTIFACT_PRESENT` does not imply `STATIC_PASS`; `STATIC_PASS` does not imply Windows-native or physical evidence. A blocked or denied *specific action* stays blocked: do not disguise or reroute it to evade a tool policy. Ambiguous real USB/reset/Flash can change the board, so **STOP without automatic replay**.

**Observed A-04 OFFLINE case:** managed build task accepted, result fetch unavailable; independent artifact `Bundle/stm32_os_gate4_a04_offline_stop_native_v1_20261028.zip` 6,415 bytes, SHA `4C4BEAB523898C5F0A17B13AC3F3B28BAC16586D1AE2C4B1AC79BCC9D57ABEA9`; separate canonical `validate` exit0 PASS, four `OFFLINE` members. Build-task exit remains unobserved, Windows native A-04 still pending. Normative handling and incident: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, `docs/HARNESS_FAILURE_REGISTRY.md`.

## Narrow remote PUBLIC-owner trust mutation profile — signed gate required

The separate `REMOTE_PUBLIC_TRUST_MUTATING` profile is reserved for the
single `gate3_macmini_owner_public_trust_enrollment` workflow and exact package
`stm32_os_gate3_macmini_owner_public_trust_enrollment_v1_20261027`, with
`owner_authorization_namespace=deus-os-gate3-linux-public-trust-enrollment-v1`
and `authorization_profile=SIGNED_OWNER_APPROVAL_REQUIRED`.
It is **never OFFLINE or HARDWARE_READONLY**, even if its allowed side effect is
only an unprivileged Linux user's new public `allowed_signers` file.
`validate` and `build` prove archive shape only; the generic `release`
**categorically rejects both `HARDWARE_MUTATING` and
`REMOTE_PUBLIC_TRUST_MUTATING`**. Nothing in this extension authorizes
USB, Flash, reset, sudo, SSH login trust changes, or release without a separately
verified owner Ed25519 SSHSIG under the exact isolated public-enrollment namespace.
Negative cases are guarded by regression tests 33–42. The first signed approval
bundle is OFFLINE and may be processed only after exact original Windows evidence
acceptance. See the Gate3 acceptance report for original SHA provenance.

## Physical 000C→000D source-locked safety STOP (2026-10-28)

For HARDWARE_MUTATING only, the builder and validator require an explicit
exact top-level `$PackageId` matching manifest.package_id, and reject a
second embedded PowerShell result-file writer using
`DEUS_RUN_RESULT_PATH`. They also reject any source or submitted ZIP
containing `QUARANTINED_NOT_FOR_RUN.txt`. Regression tests 43–49 prove
these negative boundaries (48 pass, one native-Windows-only skip in the
full 49-test suite as of 2026-10-28). The original signed 14-file physical
stage remains quarantined and **must not be packaged** under this guard.

The safe architectural choice is a single source-locked entrypoint with its
ORIGINAL signed package_id and ONE result/evidence owner. It needs a
SEPARATE signed, verified and natively accepted hardware release BEFORE
execution; ZIP structural validation is insufficient. Missing/ambiguous
results after a possibly sent reset mean `RESET_POSSIBLY_ATTEMPTED`, not
`RESET_ATTEMPTS=0`. An earlier nested-runner generator was blocked by the
ChatGPT client security layer, and an OFFLINE-only synthetic generator was
also blocked before it reached DEUS MCP. Do not route either through an
alternate tool. See
`Reports/ManagementOnlyRuntimeMigration/Gate3/Acceptance/GATE3_SINGLE_RESULT_OWNER_HARDWARE_RELEASE_STOP_20261028.md`
under the workspace sibling Reports tree.

## One safe producer / three different proof levels

1. **Construction (pure/offline):** A *source directory* contains immutable final `apply.ps1`, `PACKAGE_README.txt`, `bundle_spec.json` and any approved `payload/` sources. `bundle_spec.json` contains `schema=DEUS_OS_OPERATOR_BUNDLE_V2`, exact `package_id`, `runner_result_contract=DEUS_OS_RUNNER_RESULT_V1`, `execution_profile=OFFLINE|HARDWARE_READONLY|HARDWARE_MUTATING`. It must **not** contain `manifest.json`, `hashes.sha256`, or any generated build outputs. Hardware profiles must also define exact `workflow_id` (for current collector: `000c_session_collector`); authoring output never constitutes permission to execute it. Run:
   `python scripts/operator_bundle_guard/bundle_guard.py build <source-directory> <output>/<package_id>.zip`.
   The builder writes **the final apply hash** into `manifest.json`, indexes every included member (including manifest), produces a deterministic ZIP, independently reopens/revalidates and prints the entire ZIP SHA-256. It cannot overwrite an existing output.
2. **Static verification (no hardware; NOT release):** Run `python scripts/operator_bundle_guard/bundle_guard.py validate <bundle.zip> --sha256 <whole-ZIP-SHA256>`. Rejects hash/CRC/manifests, package ID mismatch, missing/extra index entries, duplicate files/case collisions/JSON keys, traversal, symlinks, restricted binary/key payloads, `Set-StrictMode Latest`, missing 3.0 pin/result-path reference, duplicate runner-owned banners and forbidden Defender bypass patterns. **Static PASS never authorizes handing hardware ZIP to operator.**
3. **Native-authoring admission (must execute on the ACTUAL Windows operator domain):** `python scripts/operator_bundle_guard/bundle_guard.py release <bundle.zip> --sha256 <SHA> --primitive-receipt <reviewed-native-primitive-receipt.json>`. This runs a PowerShell 7 parser on the exact extracted `apply.ps1`; requires matching original ZIP/apply SHA and real primitive receipt. For hardware profiles, it requires Mac-mini real USB primitive receipt on the **exact payload** plus Windows runner/AST proof. **`HARDWARE_MUTATING` is categorically denied by this generic `release` command**, regardless of any claimed receipt: destructive work needs a separate signed owner-authorized gate. A receipt is **unsigned JSON and not cryptographic proof of a physical device**: independent human verification of raw execution facts and hardware authorization is still mandatory. An offline Linux or ChatGPT environment cannot pass this admission command. It never executes the operator `apply.ps1` itself.

For a hardware run, prove Windows PowerShell startup/exit/result file, SSH semantics and Mac-mini libusb bounded multi-frame response independently **on their respective real execution hosts**, including negative/timeout paths; prove no RX/TX framing assumptions or suppressed partial data. A single program doing those all at once before primitive validation is forbidden.

### Native primitive receipt fields

JSON `schema=DEUS_NATIVE_PRIMITIVE_RECEIPT_V1`, `bundle_sha256`, `apply_sha256`, `windows_ps_ast_exit=0`, `windows_runner_smoke_exit=0`, and for USB hardware `linux_usb_primitive_exit=0`, `linux_usb_primitive_host=macmini`, `linux_usb_primitive_scope=EXACT_PAYLOAD_AND_REAL_DEVICE`. These **must be tied to real independently retained raw stdout/stderr, exact code hashes and exit codes**; the guard checks format and hash consistency but does not authenticate an operator merely because JSON says PASS. New original operator evidence belongs in the validated Windows OS-sibling `Evidence` directory, not Downloads or GitHub; older originals keep their existing custody.

**Terminal and result:** User runs only `deus-run <package_id> -ExpectedSha256 <whole-zip-SHA256>`. The V2 child must populate exact `DEUS_RUN_RESULT_PATH` schema, materialize evidence ZIP on classified fail and pass, avoid stdout/result banners and exit nonzero for any FAIL. These semantic behaviors additionally need Windows native tests; static text checks are defense in depth, not a substitute.

## Exact Linux bulk-IN primitive hardening (2026-10-22, SOURCE-ONLY)

`libusb_frame_primitive.py` is the independent, injected USB **IN-only read/framing** contract. It does **not** load or open libusb itself, USB endpoints, UART, SWD, or reset anything. It explicitly requires a backend returning **signed libusb result code, actual transferred bytes and length**, and records request ID, per-read status and elapsed milliseconds, bounded partial tail, recent decoded frame headers, received chunk count and RPC_END status domain/code. A single absolute per-RPC deadline constrains every sub-read; **no hidden retries or reconnect**. A negative result is always FAIL regardless of received progress. It is a source candidate for later independently native-validated collector composition; **not a new runnable operator bundle**.

`test_libusb_frame_primitive.py` uses 33 source-conformant RPC_DATA frames with exact real v3 total payload length **1183 B / 1733 wire B including RPC_END**, with only 64-B-or-smaller USB packets. It deliberately rejects a fake 1733-B "USB packet" as physically invalid, and exercises all libusb errors including timeout with partial bytes, disconnect/PIPE/BUSY/IO/unknown, lost RPC_END, CRC/sequence/request mismatch, nonzero RPC status and bounded budgets. Offline DEUS MCP: **23/23 PASS** at initial publication; **28/28 PASS** after targeted tail-byte rejection, fail-poisoning, decoded sequence fields and monotonic per-RPC read timing. All five new tests execute without USB. Existing `test_tx_pressure.py`: **5/5 PASS**. The source logic is tested, but **native libusb primitive on actual Mac mini remains unproven and is still blocked by the incident register**. The installed C# Linux transport already includes signed libusb error name/code in exception messages; it is the separate Python/ctypes collector v4 that flattened them. The C# native wrapper does not attach the partial received byte count on a failing bulk read, which is a separate future diagnostic improvement subject to Host source-scope review.

**Unknown physical cause:** rev5 firmware's `usb_device_diagnostics.management_tx_drop_count` and `binary_rpc_state_t.diagnostics.tx_failures` are internal RAM diagnostics; the fixed 36-command RPC registry does **not** export a method to query either counter. Their values have **not** been measured by a read-only USB RPC. Native transport traces and a separately authorized read-only debug route or design-review instrumentation are required before claiming TX ring overflow occurred. No watchdog, USB or firmware recovery modification is authorized by source-only simulation.

## Regression evidence and limits

`test_bundle_guard.py` is the executable negative matrix, and `incidents.json` is the unique machine-indexed historical defect register. An incident cannot be marked `GUARDED_STATIC` without an existing negative test name; `PENDING_REAL_PRIMITIVE` and `OPEN_UNPROVEN_CAUSE` remain release blockers for affected hardware workflows. CI now executes the full suite on **every push and pull request**, rather than only on changes matching a path filter; `test_32_guard_ci_runs_without_path_filters` rejects the old omission. This is still not proof that GitHub branch protection enforces the check. **Review workflow must additionally run native release admission before delivering any hardware ZIP; CI running on Linux alone cannot do this.**

The exact original Windows v1-v4 `000C` collector ZIPs were independently replayed as fixtures offline: v2 is rejected for `MANIFEST_APPLY_SHA_DRIFT`; v1, v3 and v4 fail `STRICTMODE_3_REQUIRED`. No extra v5 bundle is created while the unresolved Linux long-RPC/firmware TX-flow-control problem exists. The installed firmware, recovery/bootloader and Gate4 admission remain unchanged.

**Security boundary:** ZIP lexing cannot prove a script harmless or detect all secrets; payload authorship, approval, native primitive evidence and independent security review are separate mandatory controls. Automatic static PASS is not physical Gate2/3 acceptance. To make this process enforced for all contributors, require its GitHub Actions check as a protected-branch check, and use native Windows admission in the publishing workflow; those external settings and the installed runner are not implicitly changed by adding this tool.
