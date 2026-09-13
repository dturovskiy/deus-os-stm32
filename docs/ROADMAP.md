# Roadmap

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Current roadmap checkpoint — 2026-09-13

Earlier unchecked Phase 0-2 rows are historical planning debt; the hardware baseline has already advanced beyond them.

Accepted through this checkpoint:

- [x] Boot / custom startup / linker / hardware execution.
- [x] Clock, GPIO, SysTick, monotonic time.
- [x] Fault diagnostics.
- [x] Bidirectional USART1 command console.
- [x] I2C1 master and SSD1306 at `0x3C`.
- [x] Native 128x32 OLED framebuffer/text/status/retained-console stack.
- [x] Circular retained-console scrolling.
- [x] Dirty-page presentation and runtime UI integration.
- [x] Runtime boot UI lifecycle.
- [x] Scheduler foundation.
- [x] Cooperative scheduler activation.
- [x] PendSV timer-driven preemption.
- [x] Scheduler stack canary/high-water instrumentation:
  - published at `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`
  - `72 / 512 bytes` measured for all cooperative/preemptive synthetic task paths
  - canary intact across `34` commands / `68` scheduler runs.
- [x] Representative substantive preemptive PSP OLED workload:
  - command-gated `schedworkload`
  - frozen OLED runtime full render/present on task 0
  - CPU-only no-yield task 1
  - task 0 runtime high-water `328 / 512 bytes`; margin `184 bytes`
  - task 1 runtime high-water `80 / 512 bytes`; margin `432 bytes`
  - PendSV switches `124..126`
  - UI result / peer overlap / canaries PASS across `34` complete workload runs
  - full scheduler/UART/I2C/OLED regression preserved
  - corrected boot proof and exact target readback PASS
  - physical frozen OLED PASS.

Stack-analysis rule:

- [x] `.su` direct-call-chain analysis is useful as a feasibility estimate.
- [x] The task-0 runtime high-water (`328 bytes`) exceeded the source-build estimate (`284 bytes`), so the current static method is not a conservative upper bound.
- [x] Runtime watermark/canary evidence controls sizing for the accepted workload.

Scheduler work still open:

- [x] 512-byte stack validated for the exact tested frozen OLED render/present PSP workload.
- [x] Production ownership/stack-budget decision audit:
  - full current console estimate `524 + 64 = 588 bytes`
  - 512-byte full-console PSP stack rejected
  - active nested scheduler diagnostics rejected until lifecycle separation
  - MSP runtime budget remains separate.
- [x] USART1 RX IRQ/ring-buffer prerequisite:
  - IRQ37 sole `USART1_DR` reader
  - 128-byte SPSC ring
  - existing `uart_try_getc()` consumer
  - WFI idle restored
  - observed hardware high-water `29 / 128`, zero drops/errors
  - exact `+167` IRQ/byte deltas across four burst rounds and fresh reset.
- [ ] explicit production task ownership model
- [x] USART1 RX IRQ + 128-byte ring-buffer foundation with real backlog/no-loss proof
- [ ] kernel/MSP runtime high-water / guard proof
- [ ] console-task PSP stack budget if console ownership migrates
- [ ] production scheduler diagnostic isolation / lifecycle
- [ ] normal boot task migration
- [ ] idle/wait model / steady-state scheduler ownership
- [ ] `sleep()`
- [ ] priorities

Production ownership decision is complete: direct normal-boot migration is blocked by separate console-stack, scheduler-lifecycle, and MSP-budget requirements. USART1 polling RX was the first prerequisite and is now replaced by hardware-accepted IRQ/ring receive. Next active boundary: **MSP runtime high-water / guard instrumentation**. Normal-boot migration remains deferred.
<!-- END STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->

## Phase 0 - Boot baseline

- [x] ARM GNU toolchain
- [x] Linker script
- [x] Vector table
- [x] Reset_Handler
- [x] `.data` initialization
- [x] `.bss` initialization
- [x] Minimal `kernel_main()`
- [x] ELF/BIN generation
- [ ] Flash first kernel image
- [ ] Prove execution on hardware

## Phase 1 - Core hardware

- [ ] GPIO driver
- [ ] PC13 status LED
- [ ] Clock configuration
- [ ] SysTick
- [ ] Monotonic kernel time

## Phase 2 - Diagnostics

- [ ] HardFault diagnostics
- [ ] UART logging
- [ ] I2C driver
- [ ] SSD1306 driver
- [ ] OLED kernel console

## Phase 3 - Scheduler

- [x] Task control block foundation
- [x] Separate static task stacks + synthetic initial frames
- [x] Cooperative scheduling
- [x] PendSV context switch
- [x] Command-gated SysTick preemption proof
- [x] Task stack budget / high-water validation for the accepted command-gated frozen OLED workload
- [ ] Normal boot task migration
- [ ] `sleep()`
- [ ] Priorities

## Phase 4 - Kernel services

- [ ] Timers
- [ ] Message queues
- [ ] Synchronization primitives
- [ ] Watchdog integration
- [ ] Runtime statistics

## Phase 5 - Networking

- [ ] ESP-01 / ESP8266 UART transport
- [ ] Framed STM32 <-> ESP protocol
- [ ] Network service boundary
- [ ] Remote control API
- [ ] Web control panel

<!-- BEGIN STM32_OS_USB_STRATEGY -->
## Native USB strategy

The Blue Pill micro-USB connector is a planned first-class OS transport, not only a power connector.

Target progression:

1. Minimal STM32F103 USB Device core on PA11/PA12, implemented without HAL.
2. USB CDC ACM diagnostic/command console.
3. Bidirectional kernel shell/RPC transport over USB.
4. Binary transport for structured telemetry, files, bitmap/framebuffer chunks, and host-rendered UI primitives.
5. Host-side client capable of presenting an interactive remote UI / desktop-like view of the STM32 OS.
6. USB firmware-update path and a small recoverable bootloader so normal development can eventually use the native micro-USB cable without the external UART adapter.
7. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary transport.

Constraint: STM32F103C8 is a USB Device target here, not a general USB Host platform. Keyboard/mouse emulation is possible as USB HID device behavior; directly hosting commodity USB peripherals is outside the baseline architecture.
<!-- END STM32_OS_USB_STRATEGY -->
