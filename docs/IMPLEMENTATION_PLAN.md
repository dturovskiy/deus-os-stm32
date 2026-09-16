# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-16

### USB CDC ACM console foundation — PUBLISHED

Published commit/tree:

`5a8a45618b87b3069fd7cbac6119035b6ac4ad2c` /
`bbc6b24e28279075c41410e8abdace89c4b805b7`.

Published candidate:

`58548` bytes /
`D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`.

The direct-register native USB stack, CDC ACM class/data plane, Windows `usbser` binding, UART/CDC parser isolation, pressure tests, physical reconnect, automatic software-reset attach, post-IWDG CDC recovery, OLED conditional N/A review, local commit and ordinary non-force publication are complete.

### Current boundary — transport-neutral shell/RPC foundation — GATES 0–5 ACCEPTED / GATE 6 NEXT

Boundary:

`SHELL_RPC_FOUNDATION`

Gate 1 source now establishes:

- one static allocation-free command service above UART and USB CDC;
- deterministic registry metadata: method name, class, argument bounds and an internal dispatch key;
- explicit execution context carrying a generic response writer, opaque writer context, origin RX-event mask and latched writer-failure state;
- semantic service statuses `OK / NOT_FOUND / BAD_ARGS / BUSY / INTERNAL_ERROR`;
- `OK` means service dispatch completed; method-specific operational results remain in the existing payload tokens such as `OLED_*_ERR` or `SCHED_*_ERR`;
- retained `32`-byte text line capacity and bounded maximum `4` argument tokens;
- exact legacy command names/output compatibility;
- new foundation introspection only: `help` and `rpcinfo`;
- independent UART/CDC parser state retained;
- normal command responses route only through the explicit originating context; UART-specific boot/fatal output is isolated as an emergency path;
- no global active-transport selector remains in normal command execution;
- no binary framing, host application, firmware update, bootloader, heap, new task/SVC/IPC, scheduler-core change, USB-driver change or OLED edit.

Accepted Gate 2/3 candidate:

- tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`;
- BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- Flash `37196` bytes, SRAM `9216` bytes;
- Gate 3 shared-service proof PASS on UART and CDC, including `help`, `rpcinfo`, argument/editing/overflow handling and origin-only responses;
- retained CDC safe surface `20/20` pre/post-IWDG, scheduler BUSY `8/8`, packet boundary `38/38`, CDC pressure `128/128`, UART pressure `128/128`;
- physical reconnect and post-IWDG automatic `usbser` recovery PASS;
- final Flash readback exact;
- Gate 4 `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 5 evidence/docs finalization complete.

Canonical design:
`docs/SHELL_RPC_FOUNDATION_PLAN.md`

Canonical acceptance:
`docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`

Next:

**Gate 6 — local acceptance commit of the hardware-accepted shell/RPC foundation. No push until the local commit is verified.**
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
