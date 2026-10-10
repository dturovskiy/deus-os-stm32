# Deus OS — Kernel Composition Root Final Cleanup Plan

Status: **RDC-06 CLOSED / PUBLISHED — implementation `7bc5f27ce3447f0c907d2643e7db67669371210a`; historical Gate-0 Amendment A preserved below**

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

## 6. Gate-0 Amendment A — 2026-10-09 (PUBLICATION REQUIRED BEFORE SOURCE CHANGES)

### 6.1. Rejected original Gate-1 cut and provenance

The initial Gate-0 plan/acceptance pair was published docs-only at commit `5373838906122692bd4a5d804e462ea884c62618`, tree `8e1dc0fe4fa4da026523e2e2038429cef2bcdd5c`. This historical original contract selected a phased `schedprod` snapshot/presentation extraction to `scheduler_diagnostics.c`. The measured implementation was **not accepted**:

- The last isolated baseline/candidate ARM build evidence is `stm32_os_rdc06_gate2_isolated_arm_build_static_v3_20261009_120329_641.evidence.zip`, SHA-256 `56C8862146DEA8730478B6A14E7F0F1B9B9AF8C349883B81C2DA4FE5F23647DB`; its `FINAL_OUTCOME=FAIL` is specifically `STACK_STATIC_REGRESSION`, not an ARM compilation failure.
- Exact candidate tree `edf2e81ba72f384467406e997a98181c80d0f72f`; separate baseline and candidate builds both returned `BUILD_OUTCOME=PASS`. Flash `51972 -> 52220` (+248 bytes), SRAM `10968 -> 10968`; the 53,248-byte application Flash ceiling was respected.
- Compiler `.su`: baseline `console_execute_request=184` bytes; candidate `console_execute_request=240` plus nested `scheduler_diagnostics_write_production=16`, conservative cumulative `256 > 184` (+72). This fails the frozen static non-regression criterion. The actual hardware high-water was **not measured**; no target I/O, flash, commit or push was authorized/performed.
- The static order of 34 `SCHED_PROD_*` tokens and phased late sampling passed, but neither proves stack safety or complete runtime equivalence.
- The three-file `schedprod` WIP was rejected under the explicit Gate-1 engineering-rejection rule and reverted to the exact published bytes of `src/kernel.c`, `src/kernel/scheduler_diagnostics.c`, and `include/kernel/scheduler_diagnostics.h`. Clean `HEAD == origin/main == 5373838906122692bd4a5d804e462ea884c62618`, empty staged and unstaged diff verified. This rejection is a superseding engineering decision, not a retroactive change to the accepted initial Gate-0 history.

### 6.2. Replacement ownership decision — help/command registry presentation

The bounded replacement is the existing `help` command implementation presently co-located with composition/production integration in `src/kernel.c`: `console_write_command_descriptor` and `console_command_help`. Both operate only through command registry, descriptor and bounded `command_service_context_t` output contracts. Those registry and output primitives already belong to `src/kernel/command_service.c` and `include/kernel/command_service.h`. There is **no private kernel task, stack, watchdog, IRQ, framebuffer, global state, hidden external binding, allocator or mutable snapshot dependency** in this pair.

After this amendment is independently accepted and published, one coherent implementation slice may:

1. Move the two help/registry presentation functions into existing `src/kernel/command_service.c`, using its public typed registry and output primitives.
2. Expose one explicit `command_service_execute_help(request, context)` function in `include/kernel/command_service.h`.
3. Replace the corresponding `help` safe-method branch in `src/kernel.c` with delegation and delete only the now-redundant local help formatter/handler.

**Exact authorized Gate-1 implementation paths:** `src/kernel.c`, `src/kernel/command_service.c`, `include/kernel/command_service.h`. No other source, build script, linker, startup, Host, CI or project file change. The previously selected three-path `schedprod` implementation scope is **superseded** and no longer authorizes source edits. If the help cut is not independently cohesive, abort and re-freeze scope rather than increase coupling.

### 6.3. Behavior, budget and boundary invariants

Preserve exact `help` zero/one/invalid-argument semantics; `HELP_COUNT`, `HELP_METHODS`, `HELP_METHOD`, `CLASS`, `MIN_ARGS`, `MAX_ARGS` records; ordering, separators, `\r\n`, hexadecimal formatting, descriptor registry order and missing-name behavior. Preserve all command method IDs, RPC/USB/UART framing, registry ABI, `write_failed` behavior and no output for null descriptor. This is a move of existing behavior, **not** an extensible help/plugin framework or extra command.

Re-run exact isolated baseline/candidate ARM builds. Enforce Flash <=53,248, SRAM <=11,264, original stack limits and no unmeasured production-stack regression. Compiler `.su` is necessary but does not replace measured runtime high-water. If a specific stack frame increases, classify and prove the entire actual nested call path before acceptance, without arbitrarily raising a threshold. Compare deterministic `help` outputs/branches with actual fixtures and independently review the exact source diff. Gate-3 must bind firmware bytes to target readback and prove the affected `help` responses plus existing console, scheduler/IWDG, USB/UART, application/UI/OLED liveness as required by the scoped acceptance contract; avoid unnecessary USB power cycling.

Remaining `kernel.c` console/OLED coupling is **not** silently declared closed by this smaller cut. Re-audit and classify it explicitly at RDC-06 closure (or amend scope with independently justified evidence), before advancing RDC-07.

### 6.4. Publication lock

**Historical Gate-0 precondition (subsequently satisfied):** this amendment and its acceptance pair required a separate docs-only normal commit and non-force publication before `help` source mutation. That requirement was satisfied by published commit `3925e1d1e0a30e49c0694f746c5bbd3d7912d81c`. The subsequent exactly scoped implementation was accepted in `7bc5f27ce3447f0c907d2643e7db67669371210a`, and the unrelated bootloader EPnR correction was committed separately at `83e57f609bab4bcc8a61ea2222629bf9066e910d`. Historical original Gate-0 `schedprod` rejection remains valid.


## 7. Final RDC-06 disposition — 2026-10-10

RDC-06 is **CLOSED/PUBLISHED** by the exactly scoped three-path `help` ownership change, commit `7bc5f27ce3447f0c907d2643e7db67669371210a`, tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. The original snapshot-based `schedprod` experiment stays **REJECTED** for a measured static stack regression; it was not silently accepted or revived. The Gate-0 Amendment A contract was published beforehand at `3925e1d1e0a30e49c0694f746c5bbd3d7912d81c`.

The final Gate-3 hardware acceptance (evidence-only collation SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD`) binds the actual revision-5 signed USB deployment and whole-Flash readback to the accepted application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, the correct `help` command catalog (36 items), USB/RPC/application/scheduler/IWDG, Windows CH340 UART `COM3` at `115200 8N1`, and explicit operator `PHYSICAL_OLED=PASS`. The single-file USB EPnR CTR event-preservation repair was intentionally **not** staged with the three-file RDC-06 commit; it was independently committed as `83e57f609bab4bcc8a61ea2222629bf9066e910d`, after on-chip targeted Flash verification, 72/72 real INFO USB regression and a successful signed application update.

Normal non-force publication and fresh fetch proved `HEAD == origin/main == FETCH_HEAD == 83e57f609bab4bcc8a61ea2222629bf9066e910d` with clean index/worktree, ahead/behind `0/0`. Hosted GitHub CI run `38011195238` on that exact SHA completed success: Core `84/84`, Transport `24/24`, zero failed/skipped, Release `0 warnings / 0 errors`. No key, raw Flash, application binary, test ZIP or other private operator artifact was committed.

**Remaining ownership classification:** the composition root retains its natural MCU boot/IRQ/exception and production task integration, UART/CDC command dispatch, platform telemetry binding and OLED/UI composition. The bounded `help` presentation move closed the concrete command-registry ownership debt without transferring private runtime state. A hypothetical larger UI/console extraction has no approved current source scope; it remains conditional on new reproducible ownership/consumer evidence, not a hidden incomplete RDC-06 gate. Next canonical maintenance boundary is **RDC-07 Gate-0**, followed by RDC-08. New product features remain blocked.
