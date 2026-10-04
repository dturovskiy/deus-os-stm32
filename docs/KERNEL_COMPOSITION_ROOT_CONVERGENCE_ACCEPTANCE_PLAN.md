# Deus OS — Kernel Composition-Root Convergence Acceptance Plan

Status: **FDC-05 / GATE 0 ACCEPTED CONTRACT**

Canonical design: `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md`

## Gate 1 — source/static

Require exact FDC-05 path scope, natural driver ownership, no scheduler/application/UI dependency from low-level drivers, no hidden extracted-state externs, no dependency cycle and no universal context/service locator. `src/kernel.c` must stop defining the extracted peripheral register/state domains.

## Gate 2 — build/resource/ABI

Require fresh ARM GNU build with `-Wall -Wextra -Werror`, `git diff --check`, exact discovered source-list agreement, undefined symbols zero, stack-usage inventory complete, application Flash <= current 52-KiB executable ceiling, static SRAM <= current accepted build ceiling, task stacks unchanged unless separately justified, and no public command/RPC/application/binary/USB ABI renumbering.

Record exact candidate tree, BIN/ELF/MAP identities, Flash/SRAM and stack margins.

## Gate 3 — hardware equivalence

On the exact Gate-2 candidate prove at minimum:

- normal boot and advancing ticks;
- UART command path and RX diagnostics with zero drops/errors under accepted pressure;
- I2C scan/OLED command path;
- USB management + CDC retained behavior;
- application lifecycle smoke;
- scheduler/IWDG health and canaries;
- heartbeat PC13 state/progress;
- final Flash readback equals candidate.

Do not add extra physical reconnect cycles without a specific acceptance need.

## Gate 4 — OLED

Because FDC-05 is ownership-only, expected disposition is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS` if UI/rendering bytes and behavior are unchanged. If implementation touches visible UI semantics, reopen and require physical review.

## Gates 5–7

Reconcile canonical docs/evidence, then one normal local acceptance commit and one ordinary non-force publication. Final proof requires fresh fetch, `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## Failure classes

- `FDC05_SOURCE_SCOPE_DRIFT`
- `FDC05_OWNERSHIP_COUPLING`
- `FDC05_BUILD_OR_RESOURCE_FAILURE`
- `FDC05_PUBLIC_ABI_DRIFT`
- `FDC05_HARDWARE_EQUIVALENCE_FAILURE`
- `FDC05_DOC_RECONCILIATION_FAILURE`

No source-only result closes FDC-05 without Gate-2/3 proof.
