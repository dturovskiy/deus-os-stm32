# Deus OS — Application Runtime Foundation Plan

Status: **GATE 0 ARCHITECTURE / SOURCE BOUNDARY FROZEN — GATE 1 NEXT**

Boundary ID:

`APPLICATION_RUNTIME_FOUNDATION`

Published repository prestate:

- `HEAD == origin/main == 39690c9ef103cbcf93272df8bad0359a934b7dc1`;
- tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`;
- subject `feat: optimize OLED dirty region updates`;
- worktree/index clean, ahead/behind `0/0`;
- accepted firmware source candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- accepted BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- accepted ELF `71308` bytes / SHA-256 `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`;
- accepted MAP SHA-256 `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`;
- Gate 3 hardware/runtime acceptance PASS;
- Gate 4 `PHYSICAL_OLED=PASS`;
- `OLED_DIRTY_REGION_OPTIMIZATION` published by ordinary non-force push at the repository prestate above.

Canonical architecture parent:

- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`;
- `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`;
- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

## 1. Purpose

Implement the first real application runtime on the already accepted two-task Deus OS substrate without turning the STM32 into a dynamic process loader or adding speculative RTOS machinery.

The boundary must deliver:

- a static firmware-linked application registry with explicit stable application IDs;
- deterministic application lifecycle state;
- one active foreground application at a time;
- a bounded semantic application-event contract separate from scheduler event bits;
- a bounded service snapshot contract for system/USB/network/time state;
- a bounded three-row application view model rendered through the existing OLED console path;
- transport-neutral runtime inspection/control through the existing command service and binary RPC adapter;
- no heap, no dynamic task creation, no application-owned IRQ policy and no arbitrary uploaded native code.

This boundary is the firmware implementation of the previously published application model. It is not the host Control Panel, package loader, filesystem, production WinUSB management transport, update subsystem or networking layer.

## 2. Runtime ownership model

The accepted production task topology remains authoritative:

```text
task0  console / runtime / application dispatch
       priority 128
       stack 1024 bytes

task1  production heartbeat / IWDG liveness
       priority 255
       stack 512 bytes

MSP    bootstrap / scheduler host idle / faults
```

Application callbacks execute only from task0 / Thread-PSP. They must never run from USB/UART IRQ handlers, SysTick, PendSV or watchdog/fault Handler mode.

No application receives a dedicated scheduler task in v1. Scheduler event bits remain internal wake/notification state and are not exposed as application event IDs.

## 3. Static application registry v1

Applications are statically linked descriptors stored in firmware. Registry order is not application identity.

Stable v1 application IDs:

```text
0x0001  system.home
0x0002  device.info
```

`system.home` is the mandatory fallback and initial foreground application. Its rendered content preserves the accepted current home appearance exactly:

```text
DEUS OS
DESKTOP
READY
```

`device.info` is the second built-in system application and provides the first visible application-view transition:

```text
DEUS OS
DEVICE INFO
STM32F103
```

Both applications are firmware-owned system applications. This boundary does not define removable/uploaded target applications.

Registry descriptors must contain at least:

- stable `uint16_t` application ID;
- stable short name;
- application ABI version;
- bounded flags;
- lifecycle callbacks;
- semantic event callback;
- bounded render callback.

No descriptor contains dynamically allocated state or target-code relocation metadata.

## 4. Lifecycle v1

Host-visible/runtime-visible lifecycle values are frozen as:

```text
0  REGISTERED
1  STOPPED
2  STARTING
3  RUNNING
4  BLOCKED
5  STOPPING
6  FAILED
```

A descriptor enters `REGISTERED` when the static registry is initialized and transitions to `STOPPED` after its one-time initialization succeeds. `REGISTERED` is therefore an explicit host-visible lifecycle value, not an inferred table-membership alias.

`BLOCKED` is retained because the published application-model foundation froze it in the lifecycle vocabulary. In the initial runtime it is a valid, reserved ABI state but the two initial system applications do not transition into it: they have no dedicated scheduler tasks and there is no application wait/block API in this boundary. Application `BLOCKED` must never be implemented by exposing or aliasing `SCHEDULER_TASK_BLOCKED`.

Lifecycle rules:

- each descriptor begins `REGISTERED` and is initialized at most once per boot;
- successful one-time initialization transitions `REGISTERED -> STOPPED`; initialization failure transitions directly to `FAILED`;
- `system.home` starts automatically when the boot/desktop runtime reaches normal home readiness;
- starting the already active application is an idempotent success;
- starting another registered application stops the old foreground app, starts the target and marks its view dirty;
- stopping a non-home foreground application returns to `system.home`;
- stopping `system.home` while it is the foreground app is an idempotent no-op that keeps the mandatory fallback running;
- a failed target start marks that target `FAILED`, increments a bounded runtime fault counter and restores `system.home`;
- an application callback cannot transfer IWDG, IRQ, scheduler or direct peripheral ownership.

No lifecycle transition may busy-spin, sleep, wait on scheduler events or perform unbounded work.

## 5. Semantic application events v1

Application events are distinct from scheduler event masks.

Stable event type IDs:

```text
0x0001  APPLICATION_EVENT_RUNTIME_READY
0x0002  APPLICATION_EVENT_UPTIME_MINUTE_CHANGED
0x0003  APPLICATION_EVENT_SYSTEM_HEALTH_CHANGED
0x0004  APPLICATION_EVENT_USB_STATE_CHANGED
0x0005  APPLICATION_EVENT_NETWORK_STATE_CHANGED
```

Stable source IDs:

```text
0x0001  APPLICATION_EVENT_SOURCE_RUNTIME
0x0002  APPLICATION_EVENT_SOURCE_TIME
0x0003  APPLICATION_EVENT_SOURCE_SYSTEM
0x0004  APPLICATION_EVENT_SOURCE_USB
0x0005  APPLICATION_EVENT_SOURCE_NETWORK
```

The event object is fixed-size and pointer-free, conceptually:

```text
uint16_t type
uint16_t source
uint32_t value0
uint32_t value1
```

No event queue is introduced in this boundary. Delivery is serialized synchronously from task0. If a later real producer/consumer topology requires queued delivery, queue depth, backpressure, coalescing and drop semantics require a separate explicit review.

Event callbacks return bounded effect flags. The initial required effect is `VIEW_DIRTY`; no event callback directly draws to the framebuffer.

## 6. Service snapshot v1

Applications consume current system state through a small by-value service snapshot rather than reading STM32 registers or transport-driver globals directly.

The snapshot must expose only state with established semantics:

- monotonic uptime / displayed uptime minute;
- production system-health boolean/state;
- USB configured state;
- network online state, currently false/offline until a real network service exists.

The service snapshot is valid only for the duration of the callback invocation. Applications must not retain caller-owned pointers after return.

No generic service locator, dependency-injection framework or heap-backed object model is introduced.

## 7. Application view model

Applications do not write SSD1306, `mono_fb_t` or `oled_console_t` directly.

The runtime owns one bounded application view model containing exactly three rows of at most 21 visible characters plus terminators. The UI adapter copies the view into the existing retained OLED console only when the active application view is dirty.

The system status bar remains system-owned and outside application rendering ownership.

Rendering rules:

- splash remains boot/desktop owned;
- after home readiness, the active app owns only the existing three content rows;
- status-only minute/SYSTEM/USB changes do not force application rerender unless the app event callback returns `VIEW_DIRTY`;
- application switch rerenders only the console/content region, not the whole OLED;
- `uiruntime` full restore renders the currently active application, not a hardcoded home view;
- the accepted OLED dirty-region optimization and one-framebuffer policy remain authoritative.

## 8. Command/RPC surface

Existing public RPC IDs `0x0001..0x0020` remain unchanged.

Append exactly three methods:

```text
0x0021  applist   SAFE  argc 0
0x0022  appstart  SAFE  argc 1
0x0023  appstop   SAFE  argc 0
```

Resulting registry count: `35`.

`COMMAND_SERVICE_FOUNDATION_VERSION` advances from `1` to `2` because the stable transport-neutral service surface grows. Binary framed protocol version remains `1`; envelope, CRC, request framing, max four arguments and response chunking remain unchanged.

`applist` must report registry entries, stable IDs, lifecycle state and active marker deterministically.

`appstart <id>` accepts an exact numeric application ID and performs the bounded lifecycle switch described above.

`appstop` returns the foreground application to `system.home`.

`rpcinfo` gains additive application-runtime fields including application-runtime ABI version, registry count and active app ID. Existing `rpcinfo` fields remain compatible.

These commands are intentionally transport-neutral so later WinUSB management transport can reuse the same command/RPC semantics without redefining application control.

## 9. Failure containment

Application runtime failure must remain bounded:

- invalid application ID -> `BAD_ARGS`/not-found style command result without state mutation;
- callback failure -> target app state `FAILED`, bounded fault counter increment, mandatory fallback to `system.home`;
- render failure -> retain previous valid screen where possible, mark runtime/view dirty for retry and do not invalidate watchdog ownership;
- no callback may reload IWDG;
- no callback may disable interrupts indefinitely;
- no app state may alias scheduler TCB state or USB/UART ring ownership.

The application runtime is not a protection boundary. MPU/process isolation remains out of scope.

## 10. Resource budget

The accepted prestate is Flash `44560` bytes and static SRAM `9848` bytes.

Gate 2 ceilings for this boundary:

```text
Flash <= 48656 bytes   (+4096 bytes maximum over accepted prestate)
SRAM  <= 10104 bytes   (+256 bytes maximum over accepted prestate)
```

The ceilings are budgets, not targets. Smaller is preferred.

Task stack capacities remain exactly `1024 / 512`. Hardware task0/task1 margins must remain at least `256` bytes after lifecycle/event/render diagnostics.

GCC `-fstack-usage` remains part of build evidence for new application-runtime functions and any modified task0 call paths.

No general heap is allowed.

## 11. Initial Gate 1 source boundary

Gate 1 is initially authorized to modify only:

```text
include/kernel/application_runtime.h      NEW
src/kernel/application_runtime.c          NEW
include/kernel/command_service.h
src/kernel/command_service.c
src/kernel.c
```

Published SHA-256 guards for existing files:

```text
include/kernel/command_service.h  E11F19270B003A3065A1F7B4B27831E50AC54C7C1B20A540F1C0D51D1B078A3D
src/kernel/command_service.c      F064E3D9A0B3383917E2B30D1BB6227A8BA870A16D49210A735EBF8796B26430
src/kernel.c                      21120F6A6C824B8880C4FE096F2E8B3B9DB9F66879082774A85C13B20A1C8A69
```

No scheduler, USB driver, binary framing, OLED driver/gfx, linker or startup edit is authorized initially. Any source-boundary expansion requires an explicit Gate 1 review before mutation.

## 12. Gate 1 deterministic/static proof requirements

Gate 1 must prove at minimum:

- registry count exactly `2` application descriptors with IDs `0x0001` and `0x0002`;
- registry lookup is by explicit app ID, not ordinal identity;
- lifecycle enum is exactly `REGISTERED=0`, `STOPPED=1`, `STARTING=2`, `RUNNING=3`, `BLOCKED=4`, `STOPPING=5`, `FAILED=6` and its transition table obeys this plan;
- home fallback is guaranteed after target-start failure;
- event object size/layout is bounded and contains no pointer payload;
- event type/source IDs are stable constants and not scheduler event-bit aliases;
- synchronous event delivery occurs only from task0 / Thread-PSP integration points;
- app callbacks cannot be invoked from IRQ handlers;
- view model bounds are exactly three rows x 21 visible characters;
- application code has no direct register/SSD1306/framebuffer ownership;
- command registry keeps all prior RPC IDs unchanged and appends only `0x0021..0x0023`;
- `COMMAND_SERVICE_FOUNDATION_VERSION == 2` and registry count is `35`;
- binary protocol version/capability flags are not abused as application identity;
- no heap, queue, mutex, new task, SVC or timer subsystem is introduced.

## 13. Gate 2 build/resource proof requirements

Require fresh exact-source GNU build with `-Wall -Wextra -Werror` and `-fstack-usage`.

Required:

- 17 C + 1 ASM topology unless Gate 1 explicitly justifies otherwise;
- undefined symbols `0`;
- existing USB descriptors/PMA/startup/linker hashes unchanged;
- existing public RPC IDs `0x0001..0x0020` unchanged;
- appended application RPC IDs exactly `0x0021..0x0023`;
- command registry count exactly `35`;
- Flash/SRAM within frozen ceilings;
- task stack capacities unchanged;
- no unbounded local allocation in runtime callbacks/control paths;
- exact BIN/ELF/MAP/source hashes and candidate tree recorded;
- `git diff --check` PASS.

## 14. Gate 3 hardware/runtime proof requirements

Flash only the exact Gate 2 candidate, with readback-before-flash and skip-if-exact behavior.

Application-runtime proof:

- normal boot reaches `system.home` RUNNING with active ID `0x0001`;
- `applist` reports exactly two descriptors and deterministic lifecycle states;
- `appstart 0x0002` switches to `device.info`, state RUNNING, active ID `0x0002`;
- `appstop` returns to `system.home` and leaves home RUNNING;
- repeated start of the active app is idempotent;
- invalid app ID performs no lifecycle/view mutation;
- binary RPC invocation of application control returns the same semantics as text command service;
- actual USB disconnect/reconnect produces bounded semantic USB application-event delivery without running app policy in IRQ context;
- application event delivery does not rerender app content unless the callback marks the view dirty;
- `uiruntime` restores the currently active application view;
- task0/task1 stack canaries remain intact with >=256-byte margins;
- production fault state remains zero;
- retained CDC/UART/binary pressure, malformed-frame recovery, reconnect, IWDG reboot/recovery and final exact Flash readback remain PASS.

## 15. Gate 4 physical OLED proof

Physical review is mandatory because application switching changes normal content ownership.

Require explicit `PHYSICAL_OLED=PASS` after observing:

- normal splash -> home remains unchanged and flicker-free;
- `appstart 0x0002` shows exactly `DEUS OS / DEVICE INFO / STM32F103` inside the existing content rows;
- status bar geometry/content is not corrupted by application switching;
- `appstop` returns exactly to `DEUS OS / DESKTOP / READY`;
- minute and USB status changes while `device.info` is active do not overwrite/corrupt its content rows;
- no stale glyphs or blank/off pulse during app switch;
- `uiruntime` restores the active app correctly.

## 16. Gates 5–7

Gate 5 synchronizes canonical documentation/evidence and records exact candidate/resource/hardware/physical results.

Gate 6 creates one local acceptance commit from the exact reviewed source/docs set, with clean repo afterward and remote still at the direct parent.

Gate 7 performs one ordinary non-force `git push origin main:main`, then fresh-fetch verification of `HEAD == origin/main == FETCH_HEAD`, clean repo and ahead/behind `0/0`.

After publication, exact next boundary:

`USB_MANAGEMENT_DEVICE_FOUNDATION`

## 17. Non-goals

This boundary does not add:

- dynamic native-code loading or relocation;
- removable/uploaded target applications;
- host Control Panel implementation;
- production WinUSB management descriptors/endpoints;
- general message queue, mutex/semaphore or timer callback subsystem;
- one scheduler task per app;
- heap or filesystem;
- persistent app installation/configuration;
- networking;
- firmware update/bootloader;
- MPU/process isolation;
- RTC/tickless/DMA frameworks;
- new OLED geometry/theme/font system.
