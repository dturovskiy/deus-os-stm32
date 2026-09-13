# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Accepted implementation checkpoint — 2026-09-13

The runtime baseline now includes the hardware-accepted native 128x32 OLED UI lifecycle, published scheduler foundation/cooperative/PendSV stages, published stack telemetry, and a hardware-accepted command-gated production-like OLED workload running preemptively on PSP.

### Scheduler state

Accepted/published scheduler milestones:

- Slice 9A foundation: commit `1a57f79cda42674219e774900ce07a0da8fedaf4`.
- Slice 9B cooperative activation: commit `1114621e9a6bc57d5471cf51a922c216b76bebe2`.
- PendSV timer-driven preemption: commit `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.
- Stack canary/high-water telemetry: commit `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.

The stack telemetry baseline provides:

- two 512-byte aligned static task stacks
- SVC/PendSV high-water record points
- Handler/MSP stack scanning
- capacity/high-water/canary query API
- `schedstack -> SCHED_STACK_WATER_OK`
- synthetic task high-water `72 / 512 bytes`
- synthetic observed margin `440 bytes`
- canary intact across `34` commands / `68` underlying scheduler runs.

The current substantive workload milestone adds:

- public `scheduler_start_preemptive()` wrapper
- read-only PendSV switch-count query
- `schedworkload -> SCHED_WORKLOAD_OK`
- workload task 0: frozen OLED runtime full render/present on PSP
- workload task 1: CPU-only peer, no UART/I2C/OLED and no voluntary yield
- real task overlap under timer-driven PendSV preemption
- existing `2 x 512-byte` task stacks unchanged.

### Stack sizing evidence

Planning feasibility estimate:

- `oled_runtime_ui_show()` direct-call-chain estimate: `212 bytes`
- Cortex-M3 asynchronous context reserve used by the audit: `64 bytes`
- combined estimate: `276 bytes`
- nominal margin in a 512-byte stack: `236 bytes`.

Source-build workload estimates:

- task 0: `220 + 64 = 284 bytes`; nominal margin `228 bytes`
- task 1: `12 + 64 = 76 bytes`; nominal margin `436 bytes`.

Hardware runtime evidence:

- task 0 high-water: `328 bytes`; measured margin `184 bytes`
- task 1 high-water: `80 bytes`; measured margin `432 bytes`
- capacity: `512 bytes`
- task canaries intact in every accepted workload run
- PendSV switches: `124..126`
- `WORKLOAD_UI_RESULT=1` and `WORKLOAD_PEER_OVERLAP=1` in every accepted workload run
- `34` complete workload commands accepted.

The task-0 runtime result (`328 bytes`) exceeds the `.su`-based source-build estimate (`284 bytes`) by `44 bytes`. Therefore the present direct-call-chain calculation is a feasibility estimate, not a conservative upper bound. Runtime watermark/canary evidence is authoritative for sizing until static analysis is strengthened.

For the exact tested frozen OLED full render/present workload, `512 bytes` is hardware-validated with `184 bytes` (35.9%) measured free margin. This result must not be generalized to a future console task, arbitrary application task, or MSP/kernel stack.

### Hardware acceptance

- candidate binary: `14292 bytes`
- SHA-256: `8C124B0954D65E0F698AD1C62525E72FC4F569D5133EF8F0CE67A8297A80A2CF`
- `.bss=1952 bytes`
- `_ebss=0x200007A0`
- SRAM headroom: `18528 bytes`
- exact program/verify/readback PASS
- first workload PASS
- 32/32 workload stress PASS
- 4/4 return-to-kernel checkpoint pings PASS
- final workload PASS
- scheduler/UART/I2C/OLED regressions PASS
- final exact target identity PASS
- physical frozen OLED `DEUS OS / BOOT OK / READY` PASS.

Hardware v1 reported only `InitialBoot` and `FinalBoot` failures because its boot matcher still expected `FAULTREC=0x20000270`. The linked candidate places `fault_record` at `0x20000284`. Recovery v2 cryptographically bound v1 evidence, proved those were the only false results, validated both captured v1 frames, then passed two fresh corrected boot checks and fresh runtime/readback verification without reflashing.

Normal boot still uses the existing polling kernel/MSP execution path. The scheduler is not yet the steady-state owner of console/OLED work.

### Revised scheduler sequence

Stage A1 — **accepted / published**:
- scheduler foundation.

Stage A2 — **accepted / published**:
- cooperative PSP/SVC scheduler activation.

Stage B — **accepted / published**:
- timer-driven PendSV preemption.

Stage C1 — **accepted / published**:
- task-stack canary/high-water instrumentation
- synthetic scheduler path high-water proof.

Stage C2 — **hardware + physical accepted / commit pending**:
- representative frozen OLED workload on preemptive PSP
- CPU-only peer overlap proof
- real workload high-water/canary proof
- 512-byte stack hardware validation for the exact OLED workload
- regression preservation.

Stage C3 — next:
- define production task ownership
- choose/document stack budgets from runtime evidence
- prove console-task PSP stack if console ownership will migrate
- establish a separate kernel/MSP stack budget
- specify the normal-boot migration/rollback gate.

Stage D:
- normal boot task migration
- idle task / steady-state scheduler ownership
- sleep queues
- priorities
- timers / IPC / synchronization.

The frozen OLED geometry remains unchanged and is not part of the scheduler migration decision.
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
- USB stack
- TCP/IP directly on STM32
- user/kernel privilege separation
- MPU isolation
- firmware update protocol
- multicore support
- graphical game engine

These can be revisited only after the scheduler and diagnostics baseline is stable.
