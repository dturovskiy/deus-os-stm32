# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_12 -->
## Accepted implementation checkpoint — 2026-09-12

The runtime baseline now includes the hardware-accepted native 128x32 OLED UI lifecycle, scheduler foundation, cooperative scheduler activation, and command-gated real PendSV preemption.

### Scheduler state

Accepted Slice 9A established:

- a static two-entry TCB pool
- two 512-byte aligned static task stacks
- saved-SP / stack-range / state metadata
- synthetic Cortex-M initial frames
- deterministic `scheduler_self_test()`
- `schedtest -> SCHED_FOUNDATION_OK`.

Accepted Slice 9B added:

- corrected initial-frame return semantics
- Thread-mode task execution on PSP
- SVC `#0` start
- SVC `#1` voluntary cooperative yield
- SVC `#2` task-return/exit
- kernel/MSP parking and restoration
- deterministic two-task cooperative round-robin
- `scheduler_cooperative_self_test()`
- `schedcoop -> SCHED_COOP_OK`
- hardware-proven sequence `0x10 -> 0x20 -> 0x11 -> 0x21`.

The current preemption milestone now adds:

- active `PendSV_Handler`
- `scheduler_tick()` hook from the existing 1 kHz SysTick
- PendSV request only while an explicit preemptive scheduler run is active
- PendSV priority configured to lowest
- PSP software-frame save/restore for `r4-r11`
- EXC_RETURN/SPSEL guard before PSP access
- pending-PendSV clear on abort and final scheduler exit
- `scheduler_preemptive_self_test()`
- `schedpreempt -> SCHED_PREEMPT_OK`
- two CPU-bound acceptance tasks with no voluntary scheduler yield
- deterministic preemption sequence `0x30 -> 0x40 -> 0x31 -> 0x41 -> 0x42 -> 0x32`
- minimum three real PendSV switches per accepted run
- `34` complete hardware preemption runs with return-to-kernel checks
- cooperative/foundation/UART/I2C/OLED regressions preserved.

Normal boot still uses the existing polling kernel/MSP execution path. The scheduler is not yet the steady-state owner of console/OLED work.

### Revised scheduler sequence

Stage A1 — **accepted**:

- static TCB data model
- static task stacks
- synthetic initial task frames
- invariants/self-test.

Stage A2 — **accepted / published**:

- cooperative scheduler activation
- PSP ownership for running tasks
- SVC start/yield/exit mechanism
- deterministic real task execution and return-to-kernel proof.

Stage B — **hardware accepted / commit pending**:

- PendSV context switching
- SysTick-driven command-gated preemption
- CPU-bound no-yield hardware proof
- final-exit/abort pending-PendSV safety
- regression preservation.

Stage C — next:

- determine real stack requirements/high-water behavior
- enlarge or restructure task stacks if required
- define safe steady-state scheduler task stack budget
- keep console/OLED on kernel/MSP until this gate passes.

Stage D:

- normal boot task migration
- idle task / steady-state scheduler ownership
- sleep queues
- priorities
- timers / IPC / synchronization.

### Current accepted image

- candidate binary: `12740 bytes`
- SHA-256: `E1D02C22AF7739DB3EE71E9CB9FF65D0A5F78F8C0ED61D632EFC1444040A7E4A`
- `.bss`: `1920 bytes`
- scheduler static stacks: `1024 bytes` total (`512 bytes` per synthetic task; still test-only for tiny workloads)
- `_ebss=0x20000780`
- remaining SRAM headroom: `18560 bytes`.

The frozen OLED geometry is not part of the scheduler work and remains unchanged.
<!-- END STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_12 -->

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
- USB stack
- TCP/IP directly on STM32
- user/kernel privilege separation
- MPU isolation
- firmware update protocol
- multicore support
- graphical game engine

These can be revisited only after the scheduler and diagnostics baseline is stable.
