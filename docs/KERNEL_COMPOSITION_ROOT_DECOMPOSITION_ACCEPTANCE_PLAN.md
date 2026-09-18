# Deus OS — Kernel Composition-Root Decomposition Acceptance Plan

Status: **GATE 0 ACCEPTED — GATE 1 SOURCE DECOMPOSITION NEXT**

Boundary ID:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

Canonical design:

`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`

Published parent:

- commit `25752fba557b1a1b518265a93bde05d3a6a3f9ad`;
- tree `24624db70923bdaa77f134cc956423511785c199`;
- accepted firmware candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`;
- BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`;
- Flash `48604 / 65536`, SRAM `10032 / 20480`;
- task0/task1 minimum accepted margins `272 / 424`;
- Application Runtime `PHYSICAL_OLED=PASS`.

## Gate 0 — architecture/source-boundary freeze

Gate 0 is docs/static review only.

Required freeze:

- behavior-preserving decomposition only;
- no product feature work;
- no public ABI/protocol/USB/scheduler/OLED change;
- no universal `kernel_context_t`/service locator/god object;
- no hidden extern access to private `kernel.c` state;
- no dependency cycles;
- explicit domain ownership and one-way dependency direction;
- initial authorized source set exactly as frozen by the design plan;
- Gate 2 resource ceilings stay Flash <= `48656`, SRAM <= `10104`;
- task stacks stay `1024 / 512`, runtime margins >= `256`.

Gate 0 acceptance records the measured parent:

- `src/kernel.c = 5100` lines;
- `console_execute_request = 8644` linked bytes;
- `console_execute_scheduler_diagnostic = 3284` linked bytes;
- `boot_desktop_ui_render = 1420` linked bytes;
- `kernel_main = 1116` linked bytes.

## Gate 1 — source decomposition/static proof

Required:

- changed paths remain inside frozen source boundary or are explicitly expanded before mutation;
- extracted modules have coherent ownership;
- `src/kernel.c` materially decreases;
- no public command/RPC renumbering;
- command registry remains `35`;
- application runtime ABI/lifecycle/event constants unchanged;
- no transport/USB descriptor change;
- no task/scheduler/IWDG policy change;
- no heap, new task, SVC, queue, mutex, generic timer, DMA or persistence;
- `git diff --check` PASS;
- no commit/flash.

## Gate 2 — fresh build/link/resource/structure validation

Fresh Arm GNU build with `-Wall -Wextra -Werror -fstack-usage`.

Require:

- warning-free compile/link;
- undefined symbols `0`;
- vectors/startup/linker/USB identity unchanged;
- Flash <= `48656`;
- SRAM <= `10104`;
- task stacks exactly `1024 / 512`;
- exact candidate tree/BIN/ELF/MAP identities;
- exact old/new `kernel.c` line count;
- linked size comparison for former monolithic command/diagnostic/render/main sections;
- dependency/source-boundary proof;
- real Git index untouched.

## Gate 3 — hardware/runtime equivalence

Flash only exact Gate 2 candidate.

Require retained Application Runtime acceptance:

- boot/scheduler/IWDG startup;
- default home and two-app registry;
- text/binary app lifecycle and idempotence;
- invalid-start no mutation;
- minute semantic event without view rerender;
- active-app `uiruntime` restore;
- CDC/UART/binary pressure `128/128`;
- malformed/CRC/oversize/split recovery;
- physical micro-USB power-cycle recovery;
- IWDG recovery;
- production/app fault counters zero;
- stack canaries intact;
- task0/task1 margins >= `256`;
- final exact Flash readback;
- exact repository source state.

## Gate 4 — physical OLED equivalence

If UI/application integration code moved, require explicit `PHYSICAL_OLED=PASS` confirming:

- splash/home unchanged;
- device.info unchanged;
- status bar unchanged;
- app switch no blank/off pulse;
- no stale glyphs/clipping;
- minute/status updates do not corrupt content;
- `uiruntime` restores active app;
- appstop restores home.

## Gate 5 — docs/evidence finalization

Synchronize canonical docs and record:

- exact module ownership after decomposition;
- before/after `kernel.c` line count;
- before/after linked function/object sizes;
- exact resource changes;
- hardware and physical equivalence;
- any justified source-boundary changes.

## Gate 6 — local acceptance commit

Require exact reviewed source/docs set, complete evidence, `git diff --cached --check` PASS, clean repo after commit and direct remote parent.

## Gate 7 — ordinary non-force publication

One ordinary `git push origin main:main`, then fresh fetch requiring:

- `HEAD == origin/main == FETCH_HEAD`;
- clean repo;
- ahead/behind `0/0`.

After Gate 7, exact next boundary:

`USB_MANAGEMENT_DEVICE_FOUNDATION`

## Acceptance blockers

Fail the boundary for:

- semantic/visible feature change hidden as refactor;
- public RPC/protocol/USB/scheduler behavior drift;
- catch-all god object replacing the god-module;
- hidden cross-module extern state;
- circular dependencies;
- new heap/task/SVC/queue/mutex/timer/DMA/persistence machinery;
- task margin < `256`;
- Flash/SRAM above frozen ceilings without explicit pre-approved architectural justification;
- loss of retained hardware/physical behavior.
