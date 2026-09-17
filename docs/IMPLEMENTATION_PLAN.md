# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-17

### Transport-neutral shell/RPC foundation — PUBLISHED

Published commit/tree:

`0c33304d2db86e54d715905393f49147bb6dd2ea` /
`19ac9b95caca09e991842f6f9963864934b0a334`.

Accepted firmware:

`37196` bytes /
`90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`.

The static allocation-free 32-method command service, explicit response context, UART/CDC text adapters, argument parser, physical reconnect and post-IWDG CDC recovery are accepted and published.

### Published boundary — binary framed transport foundation — GATES 0–7 ACCEPTED

Boundary:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Gate 0 freezes:

- protocol v1 over the existing USB CDC stream, with UART retained as text-only emergency console;
- `A5 5A` binary magic and deterministic text/binary demultiplexing;
- little-endian fixed envelope with request ID, payload length and CRC-16/CCITT-FALSE;
- stable public RPC IDs `0x0001..0x0020`, independent of internal dispatch enum ordinals;
- `HELLO`, `RPC_REQUEST`, chunked `RPC_DATA`, final `RPC_END`, and `PROTOCOL_ERROR`;
- max four arguments / 31 bytes each / 132-byte request payload;
- 48-byte response chunks so maximum `RPC_DATA` frame is exactly one 64-byte USB FS packet;
- explicit `ALLOW_DESTRUCTIVE` flag for destructive methods;
- one narrow nonblocking all-or-none CDC TX-span API for frame atomicity;
- command execution remains through the published `command_service_execute()` path;
- no heap, new task/SVC/IPC/timer subsystem, USB descriptor/PMA redesign, OLED edit, host GUI, firmware update or bootloader.

Canonical protocol:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

Canonical design:
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`

Canonical acceptance:
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`

Accepted Gate 2/3 candidate:

- tested tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- binary safe surface `21/21`, scheduler BUSY `8/8`, binary pressure `128/128` unique IDs;
- retained CDC/UART pressure, physical reconnect, deliberate IWDG reboot and post-reset text+binary recovery: PASS;
- final Flash readback exact;
- Gate 4 `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 5 documentation/evidence finalization: PASS.

Next:

Binary Gate 7 publication is complete at `2fde9025a51021511e73a76b561f7983ca655e2f` / tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`.

### Published boundary — OS application and UI model foundation — GATES 0–7 ACCEPTED

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION` is documentation-only. It freezes the Deus OS product role, static application/lifecycle model, boot-splash/desktop/application-view ownership, real status-bar semantics, firmware-vs-host responsibilities and long-term package/update ordering before new source work.

Canonical design: `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`.
Canonical acceptance: `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`.
Canonical completeness review: `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

The completeness review confirms there is no kernel blocker before boot/desktop/application work. It adds these mandatory later contracts: semantic app events separate from scheduler wake bits; firmware/build/platform/system capability identity for host tooling; bounded recoverable persistence before Flash settings/packages; retained previous-boot crash/reset diagnostics and structured observability as later work; explicit security/trust before network mutation or executable Flash update; controlled update reboot handoff; and portability layering between arch/platform/drivers and kernel/services/apps/UI/protocol. Generic timers/queues/synchronization/runtime statistics remain consumer-driven; heap/filesystem/MPU/RTC/DMA/power frameworks remain deferred until justified.

Gates 0–7 are accepted and ordinary non-force publication is complete at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c` / tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`. The boundary changed documentation only; no firmware build/flash was required. Exact next firmware order begins with `BOOT_DESKTOP_UI_FOUNDATION`, then `APPLICATION_RUNTIME_FOUNDATION`, then `HOST_CONTROL_APPLICATION_FOUNDATION`.
<!-- END STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Objective

Build a small, understandable, fast bare-metal operating system for STM32F103-class Cortex-M3 hardware while learning ARM/Thumb assembly, exception mechanics, scheduling, memory layout, drivers, and low-level performance.

The project is not trying to reproduce Linux on a microcontroller. It is a compact embedded kernel with explicit control of the hardware.

## Primary target

- MCU family detected: STM32F101/F102/F103 Medium-density
- CPU: ARM Cortex-M3
- ISA: ARMv7-M / Thumb-2
- Flash target: 64 KiB
- SRAM target: 20 KiB
- programmer/debugger: ST-LINK V2
- debug/program transport: SWD
- development host: Windows 10, PowerShell 7
- compiler: Arm GNU Toolchain, `arm-none-eabi`
- programmer: STM32CubeProgrammer CLI

## Architectural principles

1. Bare metal first.
2. No STM32 HAL, Arduino framework, or FreeRTOS in the kernel baseline.
3. Assembly is used where architecture mechanics matter: startup, exception entry/exit, context switching, and measured hot paths.
4. C is used for policy, drivers, data structures, and readable kernel logic unless assembly has a measured benefit.
5. Static allocation is preferred in early kernel phases.
6. Interrupt handlers remain short and deterministic.
7. Hardware drivers do not own scheduler policy.
8. Networking remains outside the core kernel until scheduler, diagnostics, and local drivers are stable.
9. Every optimization requires a baseline measurement.
10. Hardware evidence and source/build evidence are tracked separately.

## Layering target

```text
application / future control services
               |
kernel services / IPC / timers
               |
scheduler / task management
               |
exceptions / SysTick / PendSV
               |
device drivers
               |
register-level STM32 hardware access
               |
startup / linker / vector table
```

## Boot architecture

The CPU begins from Flash at `0x08000000`.

- vector[0] -> initial MSP
- vector[1] -> Reset_Handler with Thumb bit
- Reset_Handler copies `.data`
- Reset_Handler clears `.bss`
- Reset_Handler calls kernel_main
- kernel_main initializes platform facilities
- later, scheduler becomes the steady-state execution owner

## Memory strategy

Current:

- `.isr_vector`, `.text`, `.rodata` in Flash
- `.data`, `.bss` in SRAM
- initial MSP at top of 20 KiB SRAM

Future:

- explicit kernel stack budget
- explicit task stack regions
- static task-control-block pool
- no general-purpose heap until there is a concrete requirement
- stack guards/watermarks before adding many tasks

## Time model

Target initial timebase:

- SYSCLK/HCLK: 72 MHz
- SysTick: 1 kHz
- `kernel_ticks`: 32-bit monotonic millisecond tick

Later:

- wraparound-safe deadline comparisons
- optional higher-resolution cycle timing for benchmarks
- tickless idle only after scheduler correctness is proven

## Exception model

Required dedicated handlers:

- Reset
- NMI
- HardFault
- MemManage
- BusFault
- UsageFault
- SVCall
- PendSV
- SysTick

Unused handlers may initially terminate in a deterministic default panic loop.

## Scheduler plan

Stage A:
- cooperative tasks
- static TCBs
- PSP-based task stacks

Stage B:
- PendSV context switch
- assembly save/restore of callee-saved registers

Stage C:
- SysTick preemption
- fixed-priority scheduling
- idle task

Stage D:
- sleep queues
- timers
- IPC
- synchronization

## Diagnostics plan

Order:

1. LED/status pin
2. HardFault capture
3. UART text logging
4. SSD1306 local kernel console
5. structured runtime counters

OLED is a status console, not the primary high-volume log transport.

## Networking plan

Future ESP-01/ESP8266 role:

- connected over UART
- ESP side owns Wi-Fi association and TCP/IP-facing complexity
- STM32 owns kernel and device logic
- communication uses a framed, versioned protocol
- web control is implemented above that protocol
- network failures must not destabilize scheduler/interrupt core

## Performance goals

Track at minimum:

- Flash footprint
- static SRAM footprint
- per-task stack/TCB cost
- interrupt latency
- context-switch cycles
- scheduler overhead
- idle behavior
- driver throughput where relevant

"Fast" means measured latency/throughput with deterministic behavior, not simply writing more assembly.

## Development workflow

For each slice:

1. read MASTER_EXECUTION_CHECKLIST.md
2. read PROJECT_HANDOFF.md
3. define exact source files to change
4. build only
5. inspect symbols/vector/disassembly as relevant
6. produce a log
7. only then flash
8. verify programmer success
9. verify physical behavior
10. update PROJECT_HANDOFF.md after acceptance

## Deferred topics

Do not implement yet:

- dynamic heap allocator
- filesystem
- USB stack implementation (architecture planned; implementation deferred until the current scheduler lifecycle boundary is closed)
- TCP/IP directly on STM32
- user/kernel privilege separation
- MPU isolation
- firmware update protocol
- multicore support
- graphical game engine

These can be revisited only after the scheduler and diagnostics baseline is stable.

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_IMPLEMENTATION_ACCEPTED_20260913 -->
## Accepted scheduler steady-state wait/wake foundation — 2026-09-13

The scheduler now has the minimum accepted production blocking model required before normal-boot ownership migration:

- `UNUSED`, `READY`, `DONE`, and explicit `BLOCKED` task states;
- per-task wait/wake event state;
- SVC-based task wait transition;
- ISR-safe event signalling;
- pending-event protection against lost wakeups;
- stable host-MSP `WFE` idle while all incomplete tasks are blocked;
- PSP task resume after IRQ wake;
- UART event integration after RX ring publication;
- framing-aware FIFO re-check semantics for event-driven consumers.

Hardware proof covers repeated block -> idle -> UART IRQ -> wake -> resume cycles, canaries, stack margins, lifecycle isolation, legacy scheduler regressions, RX pressure, MSP telemetry, OLED runtime restoration, target Flash identity, and manual physical OLED acceptance.

### Next implementation slice

**Normal-boot production task ownership / migration**

The detailed design is now canonical in
`docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md`, with gate criteria in
`docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`.

The first migration uses one cooperative production console task on the accepted
1024-byte PSP stack, leaves slot 1 UNUSED, preserves IRQ/ring ownership, and
keeps host MSP as scheduler idle owner using WFE. The task drains the RX ring
before waiting on the UART event. Initial OLED composition remains bootstrap
work; runtime OLED/I2C application calls move with console ownership to PSP.

Still deferred to later independent gates:

- `sleep()` / timer-backed blocking on top of the accepted event model;
- scheduler priorities;
- broader application/task topology changes.
<!-- END STM32_OS_SCHED_WAIT_WAKE_IMPLEMENTATION_ACCEPTED_20260913 -->
