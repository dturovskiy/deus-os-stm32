# Deus OS — Kernel composition-root decomposition decision

Status: **ARCHITECTURAL GROWTH STOP — NEXT BOUNDARY REQUIRED BEFORE NEW FEATURES**

Decision ID:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

## 1. Why this boundary is now mandatory

The accepted `APPLICATION_RUNTIME_FOUNDATION` remains functionally clean and hardware-accepted, but integration growth has made `src/kernel.c` a god-module risk.

Measured current state on the accepted Application Runtime candidate:

- `src/kernel.c`: exactly `5100` source lines;
- linked `console_execute_request` section: `0x21C4 = 8644` bytes;
- linked `console_execute_scheduler_diagnostic` section: `0x0CD4 = 3284` bytes;
- linked `boot_desktop_ui_render` section: `0x058C = 1420` bytes;
- linked `kernel_main` section: `0x045C = 1116` bytes;
- Gate 2 accepted firmware Flash: `48604 / 65536`;
- Gate 3 minimum observed production task0 margin after binary application activity: `272` bytes against the frozen `256`-byte minimum.

Line count alone is not the architectural criterion. The blocker is concentration of independent reasons to change in one translation unit: composition/bootstrap, production task wiring, command dispatch, scheduler diagnostics, OLED/UI integration, application-runtime bridging, transport adaptation and direct low-level helpers.

No additional product feature may extend this concentration before decomposition is accepted.

## 2. What is not wrong

This decision does not classify the accepted firmware as spaghetti code.

The current architecture still has explicit ownership boundaries:

- scheduler, application runtime, command registry/service, binary framing/RPC, OLED console/status, SSD1306 and USB core are separate modules;
- `application_runtime_t` is bounded and cohesive; it is not a god object;
- application callbacks remain task0 / Thread-PSP only;
- no heap, task-per-app model, generic queue/mutex framework or dynamic loader exists;
- hardware acceptance shows deterministic lifecycle, bounded resources and intact liveness.

The problem is integration concentration, not uncontrolled dependency cycles.

## 3. Decomposition rule

Do not replace the god-module with a god object such as a universal `kernel_context_t` carrying unrelated subsystem state.

Decompose by ownership and reason-to-change, preserving one-way dependencies and bounded explicit state.

Initial extraction candidates, subject to Gate 0 source-boundary review:

1. application-runtime/UI bridge — service snapshot -> semantic events and application view -> existing OLED console;
2. application command handlers — `applist/appstart/appstop` and additive runtime introspection;
3. scheduler diagnostic command surface — diagnostic orchestration outside ordinary request dispatch;
4. transport-neutral command execution glue separated from UART/CDC adapters;
5. production task/liveness wiring separated from diagnostic implementation;
6. remaining low-level UART/I2C/register helpers moved to appropriate driver/platform modules where ownership already exists.

`src/kernel.c` should converge toward a composition root: top-level initialization order, module wiring, task binding/start and genuinely top-level exception/IRQ entry glue.

## 4. Hard invariants

The decomposition boundary is behavior-preserving.

It must not change:

- application ABI, IDs, lifecycle or view semantics;
- command/RPC IDs or binary frame protocol;
- USB descriptors/PMA behavior;
- scheduler semantics, task count, priorities or stack capacities;
- IWDG ownership;
- accepted OLED geometry/appearance;
- one-framebuffer/dirty-region behavior;
- startup/linker/vector semantics;
- hardware-visible output except where diagnostic text explicitly reports decomposition metadata.

No new heap, queue, task, SVC, generic timer, DMA framework, persistence or host transport is justified by decomposition.

## 5. Acceptance direction

Gate 0 must freeze module boundaries and dependency direction before moving code.

Subsequent gates must prove:

- exact behavior-preserving build and hardware equivalence;
- prior Application Runtime text/binary/lifecycle/event behavior retained;
- USB/IWDG/pressure/readback regressions retained;
- task0/task1 stack margins remain >= `256` bytes;
- Flash/SRAM are measured and must not grow materially without explicit justification;
- `src/kernel.c` responsibility and size decrease materially;
- no new catch-all context/god object appears;
- no circular module dependency is introduced.

## 6. Roadmap consequence

After `APPLICATION_RUNTIME_FOUNDATION` publication, exact next boundary is:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

Only after that boundary is accepted may normal feature ordering resume with:

`USB_MANAGEMENT_DEVICE_FOUNDATION` -> `HOST_CONTROL_APPLICATION_FOUNDATION` -> transfer/update/network work.
