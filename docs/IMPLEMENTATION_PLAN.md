# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-13

Published scheduler/runtime baseline now includes:

- scheduler foundation/cooperative/PendSV stages
- stack canary/high-water telemetry
- substantive command-gated OLED workload published as `8f6b922a7d2e55abc3133702e7571057f995da5d`.

Accepted substantive PSP sizing:

- task 0 frozen OLED render/present: `328 / 512 bytes`, measured margin `184 bytes`
- task 1 CPU peer: `80 / 512 bytes`, measured margin `432 bytes`
- static direct-call-chain analysis remains feasibility-only.

The production ownership decision audit established:

- normal boot still executes `kernel_main()` / console on MSP
- current scheduler is a global run-to-completion host launcher
- scheduler wait/block/sleep states are absent
- scheduler self-tests reinitialize the global scheduler and must not run nested inside an active production scheduler
- full current console linked estimate: `524 bytes`
- console + 64-byte architecture context reserve: `588 bytes`
- therefore a 512-byte PSP stack is rejected for the full current console command surface
- kernel/MSP runtime sizing was a separate requirement and is now closed by Stage C3.2 below.

Stage C3.1 — USART1 RX IRQ/ring-buffer prerequisite — hardware + physical accepted:

- `USART1_IRQHandler` / external IRQ37 is the sole `USART1_DR` reader
- RXNE interrupt enabled; USART1 NVIC priority `0x80`
- 128-byte SPSC RX ring
- existing `uart_try_getc()` consumes from the ring
- console remains MSP-owned
- production scheduler remains inactive during normal boot
- `WFI` idle restored
- `rxstat -> RX_IRQ_RING_OK`
- candidate `15084 bytes`
- SHA-256 `E25DC54C149EB9DA7B26F5378868DAA790847A1C4C7B4CFB970F294CFB738EFB`
- USART1 IRQ own static frame `12 bytes`
- SRAM headroom `18368 bytes`
- hardware ring high-water `29 / 128 bytes`
- zero drops and zero errors
- four `32 x ping` rounds each returned exact 32 PONG responses
- each round plus telemetry produced exact `+167` IRQ and byte deltas
- fresh reset repeated exact `167` IRQ / `167` bytes, high-water `29`
- scheduler/workload/health/I2C/OLED regression preserved
- physical frozen OLED remained `DEUS OS / BOOT OK / READY`.

Stage C3.2 — MSP runtime high-water / guard — hardware + physical accepted:

- dedicated MSP reservation `2048 bytes` at `0x20004800..0x20005000`
- `64-byte` bottom guard/canary
- `1984-byte` measurable watermark capacity
- reset assembly initializes guard + watermark before first `BL kernel_main`
- read-only `mspstat -> MSP_STACK_OK`
- candidate `15620 bytes`
- SHA-256 `C15634184CAB7BA3C5CE503773EB7BA4BB45DDB4B32A3D399F6BE587C518D907`
- initial MSP use `400 bytes`; margin `1584 bytes`
- accepted stress high-water `596 bytes`; minimum margin `1388 bytes`
- acceptance floor `512 bytes`
- canary intact
- `12/12` composite pressure rounds passed across OLED status, scheduler preemption, and substantive workload
- RX pressure high-water `80 / 128 bytes`, zero drops/errors, depth zero
- fresh reset accepted at `572 bytes` used / `1412 bytes` margin
- final workload, health, OLED/I2C and exact flash identity passed
- console remains MSP-owned
- production scheduler remains inactive during normal boot
- physical OLED remained `DEUS OS / BOOT OK / READY`.

Stage C3.3 — next:

- prove the console PSP workload stack budget with runtime watermark/canary evidence
- choose a console task stack size from measured workload evidence, not the rejected 512-byte assumption
- retain MSP ownership until that sizing gate is accepted.

Later, separate gates are still required for:

- production scheduler lifecycle / diagnostic isolation
- wait/block/wake or equivalent idle/event semantics
- normal boot task migration.

The frozen OLED geometry remains unchanged and is not part of these scheduler/stack ownership decisions.
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
