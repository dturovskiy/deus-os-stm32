# DEUS OS — Harness Incident Register and Fail-Closed Authoring Standard

**Status:** HISTORICAL / OPERATIONAL INDEX, **not** a separate source of truth. Parent contract: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`. Exact machine-readable incident index: `scripts/operator_bundle_guard/incidents.json`. Product-state authority: `docs/CURRENT_STATE.md`. Source bytes: Git.

## What existed, what was missing

The existing playbook already defined many `HARNESS-*` failure classes and the mandatory **primitive-first** rule. Historical incidents also appeared in individual scoped acceptance documents (Asset/AMSI, bootloader connect mode, Linux-vs-Windows USB domain ownership, missing failure artifacts, PowerShell collection/terminal errors, byte-domain normalization). There was **no single test-indexed current registry and no central source generator/negative CI gate** enforcing those rules for operator ZIP delivery. This file indexes known failure modes without changing their historical gate results.

## Real 2026-10-22 collector incidents — frozen, NO v5

| ID | Real observation | Classification and remaining proof |
| --- | --- | --- |
| `HARNESS-000C-HELLO-OFFSET-20261022` | v1 remote HELLO interpreted LINE_CAPACITY as 36 RPC count | **Proven harness parser defect**; full source-derived vector + real Linux native primitive required |
| `HARNESS-MANIFEST-APPLY-SHA-20261022` | v2 was rejected by runner as `MANIFEST_APPLY_SHA_DRIFT` | **Proven packaging defect**; deterministic manifest and index now generated from final bytes, replayed by regression test |
| `HARNESS-STRICTMODE-LATEST-20261022` | v1–v4 contain `Set-StrictMode -Version Latest` | **Proven authoring contract violation**; static gate requires fixed version 3.0 |
| `HARNESS-RPC-STATUS-LOSS-20261022` | v3 first sample complete, next RPC returned nonzero RPC_END status; domain/code omitted | **Cause unresolved**; needs exact RPC context and independent native primitive |
| `HARNESS-LIBUSB-ERROR-COLLAPSE-20261022` | v4 `SAMPLE_0_COMMAND_schedprod_LIBUSB_BULK_READ_TIMEOUT_OR_ERROR` | **Cause unresolved**; libusb signed return code/elapsed/bytes/partial frames not recorded |
| `HARNESS-REAL-SCHEDPROD-FLOWCONTROL-20261022` | v3 `schedprod` generated 1183 payload bytes over 33 RPC_DATA frames; ~1733 wire bytes, Mgmt TX ring=1024B | **Product risk, not proven product defect**: USB IRQ may drain the ring concurrently. Native TX/drop/IRQ trace and deterministic queue-pressure tests required |
| `HARNESS-POSTSTATE-MISCLASSIFICATION-20261022` | Remote capture failed yet outcome used `ENVIRONMENT_OR_READONLY_000C_CAPTURE_BLOCKED` / POSTSTATE | **Harness classification defect**; failed RPC phase needs its own label; environment not established |
| `HARNESS-RPC-STALE-READER-20261022` | Source-only primitive could accept extra bytes after RPC_END or reuse a failed session with partial stash | **Source-only guarded** by five negative/contract tests; still pending real Mac mini qualification |
| `HARNESS-CI-PATH-FILTER-BYPASS-20261022` | Earlier bundle guard CI triggered only for selected paths | **Statically corrected** to run for every push/PR; machine-indexed regression; GitHub branch protection unverified |

**2026-10-22 follow-up:** A new offline run reaches **28/28 PASS** after fail-closed terminal trailing-byte detection, reader poisoning after failure, real `sequence` fields and monotonic per-RPC event timing. The Bundle Guard now runs without path-limited GitHub Actions triggers, regression `test_32_guard_ci_runs_without_path_filters` passed. Branch protection API returned HTTP 403 for the connected integration: required-check enforcement is **UNVERIFIED**, not assumed. No native USB host has been reached and no firmware/Host production change was made.

**2026-10-22 additional bounded repair:** New `scripts/operator_bundle_guard/libusb_frame_primitive.py` and **23/23 actual DEUS MCP offline tests** `test_libusb_frame_primitive.py` cover exact failure-class attribution (signed libusb -7/-4/-9/-6/-1 and unknown), partial transfer/stash, RPC_END status, framed 64-B-or-smaller USB packets and no hidden retry or deadline reset. This **fixes the source-only diagnostic primitive** but does not close `HARNESS-LIBUSB-ERROR-COLLAPSE-20261022`, because the Mac mini native USB proof has not been run. The previous collected v4 binary is still frozen/defective. The current Host C# libusb native wrapper **already preserves the numeric libusb error in the exception**; it does not preserve partial read bytes in that exception. Firmware `management_tx_drop_count` and `binary_rpc.tx_failures` are RAM counters **not present in the fixed 36-RPC command registry**, so their physical values cannot be obtained through the present read-only command allowlist. Do not misreport the counters as physically inspected.

The three original local v1/v3/v4 evidence ZIPs are authoritative for actual results. **No raw evidence ZIP or raw logs should be committed to public/private GitHub**; Windows Downloads owns the local originals. Nothing in this audit declares the installed rev5 broken or any `000E` effect measured.

## Mandatory authoring gate (new)

The tested generator and validator are in `scripts/operator_bundle_guard/`.

- **One producer:** no direct ZIP generation or handwritten/rewritten `manifest.json`. `bundle_guard.py build` owns all package metadata and SHA, refuses overwrite, and rechecks exact final ZIP.
- **Fail-closed static validator:** rejects whole-ZIP drift, missing/unindexed/duplicate/path-traversal files, SHA/CRC failures, wrong package ID/runner-result schema, old StrictMode Latest, absent result path, duplicate RESULT banners, obvious Defender bypass, disallowed key/binary files and missing execution profile.
- **Regression registry:** status `GUARDED_STATIC` is invalid unless an actual negative test exists in `test_bundle_guard.py`. A native/firmware issue must remain `PENDING_REAL_PRIMITIVE` / `OPEN_UNPROVEN_CAUSE` until proven.
- **Release admission:** must be executed in real Windows PowerShell 7 domain (not a Linux-only mock) and carry native Windows runner and Linux USB primitive evidence for the *exact payload*, plus manual review of original facts. Hardware ZIP declares a specific `workflow_id`; the guard **denies releasing `000c_session_collector` while its native or product blockers remain open**. The receipt currently is unsigned metadata, not physical source authenticity. **All `HARDWARE_MUTATING` releases are unconditionally blocked by this generic tool**, even with a forged receipt; destructive campaigns need a separate signed gate and explicit authorization.
- **CI:** new `.github/workflows/operator-bundle-guard.yml` executes every negative test and prevents accidentally committing operator/evidence ZIPs into public OS source.
- **Runner:** installed Windows `deus-run v1.3.0` remains the sole launcher; no alternate invocation and no second runner in bundle. Repository CI and helper scripts do not automatically modify the installed runner or GitHub branch-protection settings.

This is not a new hardware bundle and does not require the owner to rerun `000C`. **No composed collector may be delivered until real Linux long-RPC transfer is understood, existing v3/v4 evidence replayed, Windows/SSH/Linux primitives independently executed and all relevant blockers closed.**

## Required engineering decision on firmware TX

Source chain: `src/kernel.c` Management binding → `src/kernel/binary_rpc.c` output flush (48 data bytes maximum per frame, newline flush) → `src/drivers/usb_device.c::usb_management_write_span_atomic` (ring 1024 bytes, atomic reject on insufficient room). The TX ring is serviced by USB IRQ. For observed v3 `schedprod`, 1183 B payload + 33 × 16 B frame overhead + 22 B terminal frame = **1733 B** of complete wire frames. With no concurrent ring draining, this exceeds 1024; with sufficient IRQ drainage it can complete, as the successful v3 first sample demonstrates. Therefore **do not simply increase the ring (SRAM budget) or add a busy-wait/retry**. Exact non-secret frame sizes from the real v3 output are now covered by `scripts/operator_bundle_guard/test_tx_pressure.py` (real DEUS MCP Python model 5/5 PASS): no-IRQ-drain simulation rejects frame 20, full 64-byte/event drain succeeds. This is a queue-pressure counterexample, **not** proof that physical IRQ servicing failed. Need a native bounded trace of ACK/IRQ/output producer timing and loss counters; if backpressure is proven faulty, design a protocol/IRQ-safe bounded flow-control correction and separate ARM/Host tests before firmware commit.

Gate facts unchanged: **Gate2 PARTIAL 12/14; Gate3 NOT_ACCEPTED; Gate4 NOT_AUTHORIZED; installed rev5 000C not modified, 000E not installed.**
