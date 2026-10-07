# Deus OS — Kernel Composition-Root Convergence Acceptance Plan

Status: **FDC-05 / GATES 0–7 ACCEPTED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`**

Canonical design: `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md`

## Gate 1 — source/static

Require exact FDC-05 path scope, natural driver ownership, no scheduler/application/UI dependency from low-level drivers, no hidden extracted-state externs, no dependency cycle and no universal context/service locator. `src/kernel.c` must stop defining the extracted peripheral register/state domains. `src/drivers/ssd1306.c` must consume `drivers/i2c1.h`; the historical manual `i2c1_write()` forward declaration and stale statement that I2C1 remains in `kernel.c` are forbidden after extraction.

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

## Accepted Gates 1–5 evidence

Exact accepted candidate tree is `bb99acf111dfa3a78193b4e5d3376fa077defa1e`; application is `51972/53248` bytes, SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`. Consolidated Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` proves warning-clean build/resource/stack/static ownership checks and retained public ABI. Consolidated hardware Gate-4 evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` proves runtime equivalence, UART/I2C/USB/scheduler/IWDG health, exact final Flash and physical OLED PASS. Gate 5 documentation/architecture reconciliation is complete; Gates 6–7 are accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8` by ordinary non-force push with fresh-fetch clean `0/0`.

## Failure classes

- `FDC05_SOURCE_SCOPE_DRIFT`
- `FDC05_OWNERSHIP_COUPLING`
- `FDC05_BUILD_OR_RESOURCE_FAILURE`
- `FDC05_PUBLIC_ABI_DRIFT`
- `FDC05_HARDWARE_EQUIVALENCE_FAILURE`
- `FDC05_DOC_RECONCILIATION_FAILURE`

No source-only result closes FDC-05 without Gate-2/3 proof.
