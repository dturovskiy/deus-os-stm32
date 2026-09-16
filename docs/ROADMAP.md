# Roadmap

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Current roadmap checkpoint — 2026-09-16

### Native USB Device core foundation — PUBLISHED

- [x] commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`;
- [x] tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`;
- [x] accepted firmware `43812` bytes / `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

### USB CDC ACM diagnostic/command console — PUBLISHED

- [x] commit `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`;
- [x] tree `bbc6b24e28279075c41410e8abdace89c4b805b7`;
- [x] accepted firmware `58548` bytes / `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- [x] Windows `usbser` CDC, physical reconnect, automatic software-reset/post-IWDG attach and retained regressions accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

### Current boundary — transport-neutral shell/RPC foundation

Boundary:

`SHELL_RPC_FOUNDATION`

Roadmap target:

- [x] Gate 0 canonical design/acceptance planning;
- [x] static allocation-free command-service registry;
- [x] transport-neutral response writer/context and origin RX-event semantics;
- [x] semantic service statuses independent of UART/CDC;
- [x] bounded in-place tokenization, line capacity `32`, maximum `4` args;
- [x] all legacy command names and output contracts retained;
- [x] `help` and `rpcinfo` foundation introspection;
- [x] UART/CDC parser-state and response-origin isolation retained;
- [x] retained scheduler/IWDG/stack/MSP/pressure/reconnect regressions;
- [x] OLED Gate 4 conditional N/A with frozen hashes exact and `uiruntime` pre/post-IWDG PASS;
- [x] docs/evidence finalization;
- [ ] local acceptance commit — **NEXT**;
- [ ] ordinary non-force publication.

Canonical design:
`docs/SHELL_RPC_FOUNDATION_PLAN.md`

Canonical acceptance:
`docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`

After shell/RPC publication the next transport boundary is **binary framed transport**.

### Phase 4 ordering constraint

Generic timer callbacks remain deferred until a real consumer requires them.
Message queues and synchronization remain deferred until a real cross-task
ownership boundary requires them. Runtime statistics remain a later
observability slice. The frozen OLED/status-bar remains unchanged until its
dedicated UI slice.

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
- [x] Normal boot task migration
- [x] `sleep()`
- [x] Priorities

## Phase 4 - Kernel services

- [ ] Timers
- [ ] Message queues
- [ ] Synchronization primitives
- [x] Watchdog integration
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
