# Deus OS — Kernel Composition-Root Decomposition Plan

Status: **GATE 0 ACCEPTED — GATE 1 SOURCE DECOMPOSITION NEXT**

Boundary ID:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

Published parent:

- commit `25752fba557b1a1b518265a93bde05d3a6a3f9ad`;
- tree `24624db70923bdaa77f134cc956423511785c199`;
- subject `feat: add application runtime foundation`;
- `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`;
- accepted firmware candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`;
- BIN `48604` bytes / SHA-256 `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`;
- ELF `77836` bytes / SHA-256 `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`;
- MAP SHA-256 `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`;
- Flash `48604 / 65536`, SRAM `10032 / 20480`;
- minimum observed production task0/task1 margins `272 / 424` bytes;
- Application Runtime Gate 4 `PHYSICAL_OLED=PASS`.

Canonical trigger decision:

`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`

## 1. Purpose

Reduce measured god-module concentration in `src/kernel.c` before any new feature work, while preserving all accepted firmware behavior and public interfaces.

This boundary is architectural cleanup, not a feature boundary.

It must improve ownership locality, dependency direction, compile/link isolation and reviewability without introducing a framework, object graph, heap or new runtime mechanism.

## 2. Measured prestate

The accepted parent has:

- `src/kernel.c` exactly `5100` source lines;
- linked `console_execute_request`: `8644` bytes;
- linked `console_execute_scheduler_diagnostic`: `3284` bytes;
- linked `boot_desktop_ui_render`: `1420` bytes;
- linked `kernel_main`: `1116` bytes;
- Flash `48604` bytes;
- SRAM `10032` bytes;
- task0 minimum accepted margin `272` bytes.

The accepted code is not classified as spaghetti. The problem is that one translation unit now has too many independent reasons to change.

## 3. Dependency rule

`src/kernel.c` must converge toward a composition root.

Allowed composition-root responsibilities:

- top-level initialization ordering;
- explicit subsystem wiring;
- production task binding/start;
- genuinely top-level exception/IRQ entry glue where no lower owner exists;
- fatal top-level fail-closed handoff.

Responsibilities to extract from `src/kernel.c`:

1. application-runtime/system snapshot/event bridge and application-view adapter;
2. application command handlers and runtime introspection;
3. scheduler diagnostic orchestration;
4. transport-neutral command execution/domain dispatch;
5. production task/liveness service glue where it can own explicit bounded state;
6. low-level register helpers that already have a natural driver/platform owner.

Dependency direction must remain one-way: composition root -> subsystem/service -> driver/platform. New modules must not include `kernel.c` state through hidden externs.

## 4. No god object

A universal `kernel_context_t`, service locator, registry of arbitrary subsystem pointers, or giant dependency struct is forbidden.

Small domain-specific context/state structs are allowed only when they own one coherent responsibility and have bounded lifetime/storage.

Global state may only move when ownership moves with it.

## 5. Public behavior freeze

This boundary must not change:

- application IDs, ABI, lifecycle values or event IDs;
- `system.home` / `device.info` visible content;
- command/RPC IDs `0x0001..0x0023`;
- command-service version `2`;
- binary frame protocol version `1` or capability flags;
- USB VID/PID/descriptors/PMA layout;
- scheduler semantics, task count, priorities or stack capacities;
- IWDG ownership/reload policy;
- UART/CDC command semantics;
- OLED geometry, status semantics, one-framebuffer policy or dirty-region behavior;
- startup/vector/linker semantics.

No new feature may be mixed into the refactor.

## 6. Initial Gate 1 source boundary

Initially authorized new modules:

`include/kernel/application_runtime_bridge.h`
`src/kernel/application_runtime_bridge.c`
`include/kernel/application_commands.h`
`src/kernel/application_commands.c`
`include/kernel/scheduler_diagnostics.h`
`src/kernel/scheduler_diagnostics.c`

Initially authorized modified files:

`src/kernel.c`
`include/kernel/command_service.h` only if a transport-neutral handler contract is required, without public ABI change
`src/kernel/command_service.c` only if registry/dispatch wiring requires relocation, without command/RPC changes

Any expansion requires explicit review before mutation.

## 7. Structural acceptance targets

Gate 1 must prove:

- no hidden `extern` access to private `kernel.c` state;
- no circular include/dependency loop;
- no catch-all context object;
- extracted modules each have one coherent reason to change;
- command/RPC registry remains exactly `35`;
- source `kernel.c` decreases materially from `5100` lines;
- `console_execute_request` no longer owns the previous monolithic inlined domain implementation;
- no application callback or diagnostic moves into IRQ/Handler execution;
- no new heap/task/SVC/queue/mutex/timer/DMA/persistence mechanism.

Gate 2 records exact post-refactor line/function/object sizes. A target of at least ~30% `kernel.c` line reduction is expected, but semantic ownership and dependency quality are acceptance criteria, not line-count gaming.

## 8. Resource freeze

Because this boundary adds no feature:

- Flash target ceiling remains `48656` bytes;
- SRAM target ceiling remains `10104` bytes;
- task stacks remain exactly `1024 / 512`;
- hardware task margins remain >= `256` bytes;
- no dynamic stack allocation;
- GCC `-fstack-usage` remains required.

Any resource increase above the accepted parent requires a specific architectural justification; moving code into modules is not sufficient justification by itself.

## 9. Hardware equivalence

Gate 3 must retain:

- boot/scheduler/IWDG startup;
- default `system.home`;
- text and binary application lifecycle semantics;
- minute semantic event/no-rerender proof;
- `uiruntime` active-app restore;
- CDC/UART/binary pressure and malformed-frame recovery;
- physical micro-USB power-cycle recovery;
- IWDG recovery;
- final exact Flash readback;
- zero production/app faults;
- stack canaries and >=256-byte task margins.

Gate 4 physical OLED review remains mandatory if code movement touches UI/application integration paths, even when intended behavior is unchanged.

## 10. Next feature boundary

Only after this decomposition is accepted and published does roadmap order resume with:

`USB_MANAGEMENT_DEVICE_FOUNDATION`
