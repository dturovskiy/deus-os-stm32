# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-13

Published baseline: `8bc1510265d0377adafda663d320c1e55f5b2c3a` (`feat: add console PSP stack-budget probe`). Current lifecycle-isolation source is hardware/physical accepted and commit-pending.

Closed scheduler prerequisites:

- scheduler foundation, cooperative SVC activation, PendSV preemption, stack canary/high-water telemetry.
- substantive OLED PSP workload: `328 / 512 bytes`; CPU peer `80 / 512 bytes`.
- USART1 RX IRQ + 128-byte SPSC ring; IRQ37 sole `USART1_DR` reader; zero-loss stress accepted.
- MSP runtime guard/high-water: `2048-byte` reservation, `1984-byte` capacity, canary accepted.
- safe console PSP budget: exact `17` non-scheduler commands on `1024 bytes`; published high-water `600 bytes`, minimum margin `424 bytes`; 512-byte full-console migration remains rejected.

Stage C3.4 — production scheduler lifecycle / diagnostic isolation — HARDWARE + PHYSICAL ACCEPTED / COMMIT PENDING:

- [x] `scheduler_init()` returns status and rejects active reset before any global scheduler mutation.
- [x] `scheduler_is_active()` added as a read-only lifecycle query.
- [x] all previously published `scheduler_init()` call sites check the result fail-closed.
- [x] six invasive scheduler diagnostics are centrally isolated from active scheduler execution.
- [x] active diagnostic contract is exact `SCHED_DIAG_BUSY`; idle diagnostic handlers/output remain preserved.
- [x] new `schedisolate` offline diagnostic runs real preemption and proves reset rejection, diagnostic blocking, active preservation, overlap, switches and both task canaries.
- [x] source delta remains exactly `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- [x] candidate `18324 bytes`, SHA-256 `9AFDE9AC5AF196E98A2896BAC0DD7AE610414FB5A2888F1D499D2A5E0A12794B`.
- [x] `.bss=5248 bytes`; `_ebss=0x20000C80`; RAM gap below MSP `15232 bytes`; `fault_record=0x20000764`.
- [x] console probe stack remains exact `1024 bytes` at `0x20000060`; internal scheduler stack pool remains `2 x 512 bytes`.
- [x] lifecycle isolation `4/4` PASS; each round: exactly six busy lines, `INIT_REJECT=1`, `BLOCKED_DIAGNOSTICS=6`, `ACTIVE_PRESERVED=1`, `OVERLAP=1`, canaries intact.
- [x] isolation task high-water `160 / 512`; peer `88 / 512`; switches `16`.
- [x] six legacy scheduler diagnostics PASS before isolation and after isolation.
- [x] standalone console probe regression `2/2` PASS.
- [x] composite console PSP + UART pressure `4/4` PASS with exact `+105` IRQ / `+105` byte deltas each round.
- [x] console PSP high-water `560 / 1024`; minimum margin `464`; peer `88 / 512`; maximum switches `693`.
- [x] RX high-water `80 / 128`; zero drops/errors; depth zero after accepted stress.
- [x] MSP high-water `320 / 1984`; minimum margin `1664`; canary intact.
- [x] fresh-reset isolation + console probe PASS.
- [x] final exact flash identity PASS.
- [x] manual physical OLED remained `DEUS OS / BOOT OK / READY`.
- [x] normal boot ownership unchanged: MSP console; production scheduler not started.

Stage C3.5 — NEXT ACTIVE GATE: production scheduler steady-state wait/wake foundation.

Goals for this gate, still without normal-boot migration:

- define explicit persistent task states for runnable vs blocked/waiting execution.
- define deterministic wake/event semantics suitable for UART/event-driven work without busy-spin.
- define stable scheduler idle ownership and return/park behavior.
- prove PendSV/context/canary invariants across block/wake transitions.
- preserve lifecycle diagnostic isolation, the accepted `1024-byte` safe-console PSP budget, RX no-loss behavior, MSP guard and frozen OLED UI.

Later separate gates remain:

- explicit normal-boot production task ownership/migration.
- `sleep()`/timer integration on top of the accepted wait/wake model.
- priorities.

Native USB remains a later transport slice. The provisional Windows/Linux host application name is **Deus OS CP** (`Deus OS Control Panel`); naming may change without changing protocol architecture.

The frozen OLED geometry remains unchanged and is not reopened by scheduler work.
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

Constraints for that slice:

- preserve the accepted wait/wake model rather than introducing a second blocking mechanism;
- migrate ownership deliberately from the current MSP-owned `console_poll(); WFI` normal boot;
- preserve USART1 single-reader/ring-buffer ownership;
- preserve frozen OLED behavior;
- keep lifecycle diagnostics isolated from active production scheduler state;
- prove normal-boot idle remains non-busy and interrupt-driven.

Still deferred to later independent gates:

- `sleep()` / timer-backed blocking on top of the accepted event model;
- scheduler priorities;
- broader application/task topology changes.
<!-- END STM32_OS_SCHED_WAIT_WAKE_IMPLEMENTATION_ACCEPTED_20260913 -->
