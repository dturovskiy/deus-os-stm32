# Roadmap

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_12 -->
## Current roadmap checkpoint — 2026-09-12

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
- [x] PendSV timer-driven preemption:
  - active PendSV vector
  - SysTick scheduler hook
  - lowest-priority PendSV
  - `r4-r11` PSP context save/restore
  - CPU-bound no-yield hardware proof
  - stale-pending PendSV safety
  - `34` complete preemption acceptance runs.
- [x] Scheduler stack canary/high-water instrumentation:
  - command-gated `schedstack`
  - SVC/PendSV record points
  - Handler/MSP stack scanning
  - `72 / 512 bytes` measured for all cooperative/preemptive synthetic task paths
  - `440 bytes` observed margin
  - canary intact across `34` commands / `68` scheduler runs
  - full scheduler/UART/I2C/OLED regression preserved.

Scheduler work still open:

- [ ] representative substantive PSP workload + high-water proof
- [ ] production task-stack budget selection
- [ ] normal boot task migration
- [ ] idle task / steady-state scheduler ownership
- [ ] `sleep()`
- [ ] priorities

Next active boundary: **command-gated representative real workload with canary/high-water measurement**. Normal-boot console/OLED migration remains deferred until workload-specific stack requirements are explicit.
<!-- END STM32_OS_ROADMAP_CHECKPOINT_2026_09_12 -->

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
- [ ] Task stack budget / high-water validation
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
