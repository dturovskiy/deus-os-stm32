# Deus OS — Kernel Composition Root Final Cleanup Plan

Status: **RDC-06 GATE-0 SOURCE AUDIT / SCOPED DESIGN CANDIDATE — NOT ACCEPTED/PUBLISHED**

Boundary: `KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP` (RDC-06)
Published source baseline: `a27e08800b87eaa5574a4fe3aa88da0ec5b004c2`
Published tree: `7147c9d156a5c8dcf967d80f48e248dee58d85c2`
State owner: `docs/CURRENT_STATE.md`
Acceptance owner: `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_ACCEPTANCE_PLAN.md`
Historical precedent: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`, `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md` (completed earlier boundary, not a new authorization).

## 1. Audit of the current published source (Gate 0)

The accepted `src/kernel.c` is 2,961 source lines (2,962 LF-delimited records including terminal empty record), 79,549 bytes; SHA-256 `D8DD5A9223B815A66424D1F2B3737DBB0456C7CF83FFF43D439BA9033007D16E`. This is **not** the earlier 5,100-line composition-root baseline. The earlier decomposition is completed; do not reopen its already accepted module boundaries merely to chase line count.

Measured responsibility regions (approximate line ranges on this exact baseline; the function identities, not these approximate ranges, are authoritative):

- Lines 1–350 and 2645–2758: boot/exception/SysTick/watchdog/cortex register integration and emergency UART/fault capture. These are root/platform owners; do **not** extract them under this slice.
- Lines 390–1012: OLED diagnostic/proof rendering and console adapters. These share framebuffer, I2C and UI state; moving them wholesale would risk state encapsulation and display equivalence.
- Lines 1014–1616: boot/desktop UI orchestration, `system_service_state` refresh and UI console surface. A future UI-specific ownership decision requires a separate dependency review.
- Lines 1617–1987: I2C, UART, fault, MSP-stack and CDC diagnostic output. Hardware/exception ownership remains in the root for this slice.
- Lines 1988–2440: command help/application glue, `console_production_scheduler_stats`, safe-method dispatch and scheduler diagnostics binding.
- Lines 2441–2644: console line-input and UART/CDC frame-drain glue plus production task callbacks. This is partly natural composition-root transport wiring.
- Lines 2758–end: `kernel_main` bootstrap, task registration, IWDG start and scheduler activation. This remains the composition root.

The `schedprod` handler currently lives entirely in `kernel.c` (approximately lines 2010–2240): it collects scheduler/task metrics, prints many `SCHED_PROD_*` lines, and makes the final `SCHED_PROD_OK/ERR` verdict. The repository **already** has an independent `src/kernel/scheduler_diagnostics.c` (1,418 lines) and `include/kernel/scheduler_diagnostics.h`. Its diagnostic-only responsibility and explicit `scheduler_diagnostics_bindings_t` precedent make it the strongest **bounded initial extraction candidate**. This is a responsibility/locality issue, not proof of broken runtime behavior.

## 2. Selected bounded Gate-1 slice

Move **only the presentation and diagnostic verdict for `schedprod`** into the existing scheduler diagnostics module. The root must still own its private production task counters/stacks/watchdog/liveness state, CPU CONTROL/PSP observation and bootstrap/scheduler lifecycle. It constructs a narrow immutable, `schedprod`-specific by-value snapshot (or equivalent bounded explicit parameters) from the values already captured, and supplies that snapshot to an explicit scheduler-diagnostics entrypoint. The callee writes the same output records and computes the same predicate without reading hidden `extern` state or caching a borrowed snapshot beyond the call.

The external `schedprod` contract is frozen: exact key names and order, hexadecimal/text formatting, `SCHED_PROD_MODE=COOPERATIVE`, error/missing-task sentinel semantics, canary and minimum margin checks, CONTROL/PSP validation, task0/task1 state and priority rules, fault and heartbeat flags, and final `SCHED_PROD_OK/ERR`. Preserve the original sampling order and the single command dispatch. No new API is public to apps or Host.

The **exact initially authorized implementation path set**, after separate Gate-0 design publication, is:

1. `src/kernel.c` — replace only `schedprod` presentation/verdict with bounded snapshot production and delegation;
2. `src/kernel/scheduler_diagnostics.c` — diagnostic output/verdict owner;
3. `include/kernel/scheduler_diagnostics.h` — one specific typed snapshot and bounded diagnostic function contract.

No new C file, build-source list change, linker/startup/assembler/protocol/Host/firmware update/test project modification is selected. Any genuinely required fourth path or new module, global mutable binding, caller-visible ABI change, or test fixture expansion requires an **explicit Gate-0 scope amendment before source mutation**, not a silent harness workaround. Gate 1 may abort with an engineering rejection of the selected cut if explicit snapshot coupling turns out worse than the original locality.

## 3. Retained ownership and non-goals

- Retain two production tasks, their priority, task stacks (1024 / 512 bytes), 256-byte minimum margins, scheduler semantics, SysTick/IWDG and reset ownership, and existing watchdog reload policy.
- Retain all console/RPC identifiers, reply framing, `SCHED_PROD_*` formatting, UART/CDC transport behavior, diagnostic busy semantics, fault records and application runtime/OLED output.
- No heap, generic context, service locator, callback framework, new scheduler abstraction, hidden cross-module `extern` counters, duplicated mutable state, physical interface change or new product feature.
- The accepted display geometry and `PHYSICAL_OLED=PASS` provenance remain historical until candidate-specific hardware acceptance; CI/Host tests alone are not an OLED acceptance substitute.
- Do not alter the completed original `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` decisions or claim a lower line count by moving complexity into another god module. The proposed move must improve diagnostic ownership and dependency direction on review.

## 4. Gates and required proof

- **Gate 0 — source audit and design freeze:** publish this plan and acceptance contract docs-only; independently verify baseline/source SHA, real affected symbols/calls and exact three-path candidate scope. No firmware/Flash/target mutation and no implementation change in Gate 0.
- **Gate 1 — one scoped implementation:** only the three frozen C/header paths; source-level diff audit proves private state remains owned by the root, no hidden `extern`, no added global mutation, and exact `schedprod` key/format/order/predicate preservation. Exact candidate source tree and diff are retained.
- **Gate 2 — deterministic non-bench validation:** separately build exact baseline and candidate from independently identified Git trees using the existing Windows GNU Arm toolchain and accepted build script. Capture ELF/MAP/BIN hashes, object/symbol/link invariants, compiler diagnostics, Flash/SRAM against the actual accepted firmware layout (`application <= 53,248` bytes), and measured stacks. Use explicit formatting/predicate fixtures where possible. Relocation can change raw BIN and addresses; **do not fabricate byte equality** if linking changes. No target I/O.
- **Gate 3 — exact-candidate hardware/runtime acceptance:** only after Gate-2 PASS, use the accepted Windows ST-LINK, Windows UART, Mac-mini USB execution domains as appropriate. Prove exact candidate identity/readback, scheduler/console/IWDG/USB/application liveness, `schedprod` exact response shape/verdict, OLED/UI and physical OLED when required by the affected acceptance scope. No unnecessary USB unplug/replug. Preserve bootloader, update metadata and persistence page ownership.
- **Gate 4 — local acceptance commit:** exact frozen path set and accepted candidate; normal commit, clean index and worktree, no push.
- **Gate 5 — ordinary non-force publication:** fresh-fetch exact parent, no force, `HEAD == origin/main == FETCH_HEAD`, clean `0/0`. New hosted CI on the published SHA must pass Host Core (at least 84) and Transport (at least 24) before final closure. A source-change CI run is supplementary to — not a substitute for — target-runtime Gate-3 proof.

## 5. Gate-0 exit and RDC-07 boundary

The installation of these documents is not itself Gate-0 acceptance. First capture exact docs-only WIP, then one separately accepted docs commit and non-force publication. `CURRENT_STATE.md` remains the active-boundary authority. RDC-07 may not start merely because a code slice compiles; RDC-06 remains open until its accepted source, required hardware evidence and publication are complete. Any remaining broad UI/console concentration must be reclassified honestly at RDC-06 closure or explicitly assigned rather than silently declared resolved.
