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
- [x] Scheduler foundation:
  - static TCBs
  - separate static task stacks
  - synthetic Cortex-M initial task frames
  - deterministic foundation self-test.

Scheduler work still open:

- [ ] cooperative task execution / scheduler activation
- [ ] PSP ownership for running tasks
- [ ] PendSV context switching
- [ ] SysTick preemption
- [ ] `sleep()`
- [ ] priorities

Next active boundary: **cooperative scheduler activation (Slice 9B)**.
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
- [ ] Cooperative scheduling
- [ ] PendSV context switch
- [ ] Preemptive scheduling
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
