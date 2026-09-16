# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-16

### C4.0 — IWDG production liveness foundation — PUBLISHED

Published commit/tree remain:

`3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca` /
`e024d425e97f878c170ccfc41f0505dce79277a3`.

### Native USB Device core foundation — GATES 0–5 ACCEPTED

Boundary:

`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Accepted source candidate tree:

`4b798382ef843ef8f488115624d48c0cd1506c75`

Accepted firmware:

`43812` bytes /
`1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`.

Accepted implementation:

- direct-register STM32F103 USB FS Device core; no HAL;
- PA11 `USB_DM`, PA12 `USB_DP`;
- accepted 72 MHz system clock retained;
- RCC `USBPRE=0`, therefore USB clock is 72 MHz / 1.5 = 48 MHz;
- USB low-priority IRQ20 ownership;
- explicit BTABLE/PMA ownership and EP0 64-byte control buffers;
- bounded EP0 standard-request engine;
- delayed `SET_ADDRESS` application after status IN;
- development VID/PID `1209:000A`, private testing only;
- vendor-specific zero-data-endpoint interface for the core-only boundary;
- no CDC class/data endpoints, new task, SVC, IPC, timer subsystem, runtime-statistics subsystem or OLED edit.

Hardware acceptance:

- exact device descriptor and exact configuration descriptor read by Windows through control transfers;
- nonzero addressed state proven (`20`, then `21` after physical reconnect);
- configuration `0` accepted for this vendor-specific/no-client-driver foundation;
- physical disconnect/reconnect recovered without reflashing;
- destructive IWDG reboot `8748 ms`, post-reset `IWDG_RESET=1`;
- USB remained recoverable after IWDG reset;
- safe surface `20/20` pre/post;
- timed/priority/scheduler BUSY/UART pressure regressions retained;
- final Flash readback exact.

Gate 4 OLED disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 5 docs/evidence finalization is complete.

Next:

**Gate 6 local acceptance commit, then Gate 7 ordinary non-force publication.**

After publication the next USB slice is **CDC ACM diagnostic/command console**.
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
