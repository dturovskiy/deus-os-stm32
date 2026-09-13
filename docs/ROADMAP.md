# Roadmap

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Current roadmap checkpoint — 2026-09-13

Earlier unchecked Phase 0-2 rows are historical planning debt; this checkpoint is authoritative.

Accepted through this checkpoint:

- [x] Boot/startup/linker, clock/GPIO/SysTick/time and fault diagnostics.
- [x] Bidirectional USART1 command console with IRQ37 sole-DR-reader + 128-byte SPSC ring.
- [x] I2C1 + native frozen 128x32 SSD1306 runtime UI.
- [x] Scheduler foundation, cooperative SVC activation and PendSV preemption.
- [x] Scheduler stack canary/high-water instrumentation.
- [x] Substantive PSP OLED workload: `328 / 512` task 0, `80 / 512` peer.
- [x] MSP runtime guard/high-water: `2048-byte` reservation, `1984-byte` usable capacity.
- [x] Console PSP stack budget: exact 17-command safe surface, accepted `1024-byte` stack; published high-water `600`, minimum margin `424`.
- [x] Production scheduler lifecycle / scheduler-diagnostic isolation:
  - active `scheduler_init()` reset rejected before mutation
  - read-only `scheduler_is_active()`
  - six invasive diagnostics return `SCHED_DIAG_BUSY` while active and preserve idle behavior
  - `schedisolate` hardware proof `4/4`
  - isolation task `160 / 512`, peer `88 / 512`, switches `16`, canaries intact
  - legacy diagnostics PASS before and after isolation
  - console PSP regression + RX/MSP/OLED/final-flash identity PASS
  - physical OLED PASS.

Current production scheduler work still open:

- [ ] persistent runnable vs blocked/waiting task state model
- [ ] deterministic wake/event primitive suitable for UART/event-driven work
- [ ] stable steady-state scheduler idle ownership / no busy-spin
- [ ] explicit production task ownership model for normal boot
- [ ] normal boot task migration
- [ ] `sleep()` built on accepted blocking/wake semantics
- [ ] priorities

The next active boundary is **production scheduler steady-state wait/wake foundation**. Normal boot remains MSP-owned and the production scheduler remains inactive during normal boot until this persistent-task/idle model is proven. Normal-boot migration is a later separate gate.
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
5. Cross-platform Windows/Linux host application. Provisional name: **Deus OS CP** (`Deus OS Control Panel`); the name may change later without changing protocol architecture.
6. Host-side interactive/control UI and CLI-style control surface over the stable protocol.
7. USB firmware-update path and a small recoverable bootloader so normal development can eventually use the native micro-USB cable without the external UART adapter.
8. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary transport.

Initial host integration should avoid requiring a custom kernel-mode USB driver: CDC ACM is the first compatibility target. A later vendor-specific bulk interface through standard userspace USB facilities may be considered only if CDC becomes a throughput/latency limitation.

Constraint: STM32F103C8 is a USB Device target here, not a general USB Host platform. Keyboard/mouse emulation is possible as USB HID device behavior; directly hosting commodity USB peripherals is outside the baseline architecture.
<!-- END STM32_OS_USB_STRATEGY -->

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_ROADMAP_ACCEPTED_20260913 -->
## Scheduler steady-state wait/wake foundation — accepted 2026-09-13

Completed:

- explicit blocked task state;
- SVC event wait;
- ISR-safe event signal/wake;
- pending-event lost-wakeup protection;
- non-busy scheduler idle on preserved host MSP using `WFE`;
- PSP resume after event-driven wake;
- UART RX event integration;
- repeated hardware proof and full scheduler/RX/MSP/OLED regression.

Accepted candidate: `19932` bytes, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`.

### Next

1. **Normal-boot production task ownership / migration**
   - move from the current MSP-owned normal boot to deliberate production task ownership;
   - preserve accepted event wait/wake and interrupt-driven idle semantics;
   - preserve UART ring ownership and frozen OLED behavior.

2. **Timer-backed sleep integration**
   - build `sleep()` / timed blocking on the accepted wait/wake model;
   - do not create a parallel blocking mechanism.

3. **Scheduler priorities**
   - add only after production ownership and timer blocking semantics are independently accepted.

Harness/evidence/recovery rules live in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.
<!-- END STM32_OS_SCHED_WAIT_WAKE_ROADMAP_ACCEPTED_20260913 -->
