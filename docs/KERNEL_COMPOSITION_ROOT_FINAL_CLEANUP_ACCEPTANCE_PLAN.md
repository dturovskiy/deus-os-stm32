# Deus OS — Kernel Composition Root Final Cleanup Acceptance Plan

Status: **RDC-06 GATE-0 ACCEPTANCE CONTRACT CANDIDATE — NOT ACCEPTED/PUBLISHED**

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
