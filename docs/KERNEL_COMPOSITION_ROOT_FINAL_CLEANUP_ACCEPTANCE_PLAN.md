# Deus OS — Kernel Composition Root Final Cleanup Acceptance Plan

Status: **RDC-06 GATE-0 AMENDMENT A ACCEPTANCE CONTRACT — PUBLICATION REQUIRED BEFORE GATE-1**

Boundary: `KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP`
Design: `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_PLAN.md`
Published baseline commit: `a27e08800b87eaa5574a4fe3aa88da0ec5b004c2`
Published baseline tree: `7147c9d156a5c8dcf967d80f48e248dee58d85c2`

## Gate 0 — read-only audit, docs freeze and publication

PASS requires:

1. Confirm clean `main`: `HEAD == origin/main` equals frozen baseline; real index empty; neither new scoped plan is present, and `.github/workflows/ci.yml` is tracked. A new hosted CI job for this baseline has succeeded, but this is not source/hardware acceptance for RDC-06.
2. Hash the exact `src/kernel.c` (`D8DD5A9223B815A66424D1F2B3737DBB0456C7CF83FFF43D439BA9033007D16E`) and confirm the `schedprod` entrypoint and existing scheduler diagnostics owner. Confirm only two new scoped Markdown documents are installed at this step. Do not stage, commit or push in the installer.
3. Confirm Gate-0 plan is limited to formatting/verdict extraction, not production-state ownership movement; initially authorized later implementation paths are **exactly** `src/kernel.c`, `src/kernel/scheduler_diagnostics.c`, `include/kernel/scheduler_diagnostics.h`. Re-audit the proposed snapshot and reject if the cut adds a god object or exposes mutable root state by hidden `extern`.
4. Prove output/exception/priority/stack invariants and gate order are explicit. No new feature, target I/O, build-script mutation or device Flash change is authorized by the design installer.
5. Compare exact post-install WIP paths (two `??` Markdown docs), verify both SHA-256, stage remains empty and byte-identical semantic index, and validate whitespace. On any precommit FAIL, remove only files created by the installer and preserve the initial real Git state.
6. A separate accepted local docs-only commit and separate ordinary non-force publication are necessary before Gate-1 source edits; fresh fetch must prove clean `HEAD == origin/main == FETCH_HEAD`, ahead/behind `0/0`.

## Gate 1 — source ownership and exact scope

- Exactly three authorized source/header files change; no new files unless Gate 0 is expressly amended and republished.
- Production counters, watchdog/IWDG, stack storage, IRQ/CPU observation, scheduler lifecycle, task ownership and mutable objects remain private to their current owner. Diagnostic module consumes only a bounded immutable snapshot for the lifetime of one call; no stored binding, implicit `extern`, heap, callback registry or universal context.
- `SCHED_PROD_*` key/value records, ordering, hex conventions, missing-task sentinel values, current success predicate and `SCHED_PROD_OK/ERR` are byte-for-byte behavior-compatible at the console/RPC-facing surface.
- Freeze/inspect exact source tree and verify no unrelated host, driver, OLED, application, firmware update, startup/vector, linker or protocol file changes.

## Gate 2 — isolated deterministic build and static validation

- The prestate baseline and candidate firmware builds both use the exact source-tree identity required by the current build script and a fresh isolated owned output directory. No implicit reuse of `obj`, firmware output, source tree or generated header.
- Capture compiler version, command arguments, stdout/stderr, hashes of ELF/BIN/MAP, section/symbol/resource reports and changed functions. Reject undefined/multiply-defined symbols, new unwanted dependencies, stack regressions, malformed vector/header, unexpected SDK/toolchain drift or a missing result.
- Honor the 52-KiB application ceiling, normal runtime SRAM and accepted task/interrupt margins; prove any Flash/SRAM growth is justified and bounded. Compare representative diagnostics and all relevant success/failure predicates (active/inactive scheduler, task presence, state, stack bounds and faults).
- Raw BIN/disassembly may change because code has moved. Any equivalence claim must be limited to what the actual binary/static/deterministic proof establishes; changed bytes are not proof of a product regression, and unchanged bytes are not a substitute for behavioral proof.
- No ST-LINK, UART, USB enumeration, physical OLED or Flash operations at Gate 2.

## Gate 3 — exact-candidate physical acceptance

- Confirm the same accepted candidate tree/BIN is deployed by the established signed/update or authorized SWD path, then prove exact application Flash readback, bootloader/persistence/update-metadata ownership, and runtime liveness. Never infer deployment from a successful build.
- Check normal USB/UART/RPC, `schedprod` record presence/order/semantics including `SCHED_PROD_OK`, task0/task1 state, stack minimums, IWDG and existing OLED/UI physical outcome. Repeat physical reconnect/power interruption only if explicitly required by a new risk; it is not a generic requirement of this diagnostic move.
- Respect `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`: Windows owns ST-LINK/CH340, Mac-mini Ubuntu owns native USB. Use bounded processes and retain all raw evidence. Gate-3 FAIL must distinguish harness/environment from actual product failure.

## Gate 4 — one local acceptance commit

- Commit only the exactly accepted scoped implementation/documentation path set after Gate-3 PASS, with fresh staged-path, `git diff --cached --check`, clean poststate and exact tree binding. Do not push or touch physical target during Git commit.

## Gate 5 — non-force publication and closure

- Require fresh parent check, ordinary fast-forward push and fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean `0/0` and matching candidate tree; collect actual GitHub Actions run tied to the published commit, whose Core/Transport steps pass with executed tests `>=84/24`, zero failed/skipped.
- Reconcile `CURRENT_STATE`, residual debt program/checklist/roadmap and this scoped plan before declaring **RDC-06 CLOSED/PUBLISHED** and activating RDC-07. No unclassified additional composition-root coupling may be quietly left as closed debt.

## Mandatory evidence and stop rules

Every operator-facing package returns exactly one self-contained `.evidence.zip`: `run.log`, `outcome.txt`, raw native command stdout/stderr, SHA-256 manifest and candidate/commit identity. `FINAL_OUTCOME=FAIL` implies a nonzero exit even if evidence was produced. Use independent execution-domain proof before reusing harness primitives. No force-push, secret material, firmware mutation at docs/static gates or invented hardware success. Never downgrade missing test counts or reported timeouts to PASS.

## Gate-0 Amendment A — 2026-10-09 (ACCEPTANCE CONDITIONS FOR PUBLISHED SCOPE)

The published original `schedprod` Gate-0 contract at `5373838906122692bd4a5d804e462ea884c62618` is retained above as historical provenance, but its selected source slice is superseded **only after** this amendment is accepted and published.

**Engineering rejection prerequisite:** Independently verify the Gate-2 v3 evidence SHA-256 `56C8862146DEA8730478B6A14E7F0F1B9B9AF8C349883B81C2DA4FE5F23647DB`, candidate full tree `edf2e81ba72f384467406e997a98181c80d0f72f`, two actual successful isolated builds, original/candidate Flash `51972/52220`, SRAM `10968/10968`, stack `console_execute_request: 184/240` and writer `16`, conservative nested `256 > 184`, and `GATE2=FAIL`. Do not reclassify this as harmless, CI-covered, hardware-accepted or an index/harness error. Confirm exact rollback SHA-256 `D8DD5A9223B815A66424D1F2B3737DBB0456C7CF83FFF43D439BA9033007D16E`, `9E5FAE8EB41FC227FDEC9A6104202A8DEF4CF138B2331FCB70B114FA87E224DA`, `6D95E88AEC72355E2455FDAD120C44A96CB5264973C065CDF6B3D06A9B27DC83`; clean index and no target mutation.

**Amendment prestate:** `HEAD == origin/main == 5373838906122692bd4a5d804e462ea884c62618`; exact baseline tree `8e1dc0fe4fa4da026523e2e2038429cef2bcdd5c`. Exactly two approved WIP documentation paths and no other tracked/untracked/staged changes. `src/kernel/command_service.c` SHA-256 `3658BF2DD386CC74B80FE78E3507F1D8B3F96EEC5A95801D5302CDC2E06FE5EF` and `include/kernel/command_service.h` SHA-256 `821C882324E3513F29C0220A64EFE86356F01C5EED9B772EB95729F452D63F0A` are frozen.

**Scope review:** Confirm `console_write_command_descriptor` and `console_command_help` use only public registry/descriptor/output APIs and no kernel-private state. The amended Gate-1 authorization after publication is exactly `src/kernel.c`, `src/kernel/command_service.c`, `include/kernel/command_service.h`; the failed `schedprod` path set is revoked. Check that the minimal `command_service_execute_help` API introduces no generic dispatcher/framework or command ID change.

**Gate-1 static PASS:** exact three authorized paths only, byte-accurate functional equivalence for zero/one/invalid arguments, missing descriptor/name behavior, command registry ordering, all `HELP_*` output records, text/hex separators and transport `write_failed` semantics. No hidden globals, new state, callbacks or additional source files.

**Gate-2 build/behavior PASS:** independent isolated clean baseline/candidate ARM builds on exact Git-tree identities, actual ELF/MAP/BIN hashes, symbol and `.su` measurements, Flash/SRAM ceilings, no unaccepted stack regression, deterministic representative `help` fixtures including failure branches, and unchanged unrelated `schedprod`. No physical I/O or firmware mutation.

**Gate-3 hardware/runtime:** after full Gate-2 PASS, same candidate firmware identity/readback; bounded Windows ST-LINK/UART and Mac-mini USB tests as needed, `help` output/semantics, scheduler/IWDG/application/USB/OLED liveness, preservation of bootloader/persistence ownership. No automatic repeated power cycles. Physical/OLED outcomes must be explicitly evidenced, never inferred from builds.

**Gate-4/5:** exact acceptance commit, ordinary non-force push and fresh fetch; hosted GitHub CI Core >=84/84 and Transport >=24/24 on the published SHA; reconcile `CURRENT_STATE`, residual-debt ledger, roadmap and checklists before claiming RDC-06 closure. Other composition-root concentration remains a documented open classification until explicitly accepted.

**Gate-0 amendment exit:** locally validate and review exactly these two docs-only WIP files, then one local documentation commit and separate non-force publication with `HEAD == origin/main == FETCH_HEAD`, clean index/worktree, ahead/behind `0/0`. Until then, **do not mutate the new source scope**.
