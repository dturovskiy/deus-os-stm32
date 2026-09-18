# Roadmap

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Current roadmap checkpoint — 2026-09-17

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

### Transport-neutral shell/RPC foundation — PUBLISHED

- [x] commit `0c33304d2db86e54d715905393f49147bb6dd2ea`;
- [x] tree `19ac9b95caca09e991842f6f9963864934b0a334`;
- [x] accepted firmware `37196` bytes / `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- [x] static allocation-free 32-method command service;
- [x] UART/CDC shared semantics, parser/origin isolation and retained hardware regressions accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0` before the next boundary.

### Binary framed transport foundation — PUBLISHED

- [x] commit `2fde9025a51021511e73a76b561f7983ca655e2f`;
- [x] tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- [x] accepted firmware `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- [x] text/binary CDC coexistence, stable RPC IDs, CRC, request correlation and destructive authorization accepted;
- [x] malformed/pressure/reconnect/IWDG recovery hardware accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

Canonical protocol/design/acceptance:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`

### Published architecture foundation — OS application and UI model

Boundary:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

This documentation-only architecture boundary is fully accepted and published at commit `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`, tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`. The accepted architecture freezes:

- Deus OS product role as an independently operating embedded runtime;
- initial static application registry/lifecycle model;
- boot splash -> desktop/home -> application view lifecycle;
- real SYSTEM/USB/NETWORK indicator semantics;
- initial uptime `HH:MM` status time;
- firmware versus Control Panel responsibility split;
- target apps versus host plugins versus target resource packages;
- deferral of arbitrary native ARM loading, filesystem and update/bootloader work.

Canonical design/acceptance/gap review:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`
`docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

Foundation completeness constraints retained:

- scheduler event bits stay an internal wake primitive; `APPLICATION_RUNTIME_FOUNDATION` defines a separate bounded semantic app-event/service contract;
- host tooling must gain explicit firmware/build/platform/service/application identity/capability discovery rather than treating USB VID/PID or protocol flags as the whole OS identity;
- Flash-resident settings/packages require a bounded versioned persistence contract with integrity, atomic commit/recovery and wear policy;
- bounded previous-boot crash/reset retention and structured observability remain later independently accepted work;
- CRC/destructive-intent flags are not authentication; network mutation and executable Flash update require explicit security/trust review;
- portability preserves separation of arch/platform/drivers from kernel/services/apps/UI/protocol semantics without a speculative heavyweight HAL.

### Published implementation boundary — Boot / desktop UI foundation

`BOOT_DESKTOP_UI_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**.

This slice is intentionally smaller than the application runtime. It implements exactly `BOOT_SPLASH -> DESKTOP_HOME`, a nonblocking 1000 ms minimum visible splash dwell, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK status semantics, monotonic uptime `HH:MM`, redraw only on visible semantic change, and no steady-state SSD1306 reinitialization/display-off during routine refresh. Gate 2/3 accepted tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; Gate 4 is `PHYSICAL_OLED=PASS`.

Canonical design/acceptance:
`docs/BOOT_DESKTOP_UI_PLAN.md`
`docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`

### Published implementation boundary — OLED dirty-region optimization

`OLED_DIRTY_REGION_OPTIMIZATION` — **GATES 0–7 ACCEPTED / PUBLISHED `39690c9ef103cbcf93272df8bad0359a934b7dc1`**.

Accepted candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`, BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`, Flash `44560 / 65536`, SRAM `9848 / 20480`. Hardware accepted exact dirty-region transfers: historical full semantic refresh `572` payload bytes / `36` writes, clean `0`, one-byte narrow `9`, minute `11`, USB indicator `11`. Task0/task1 post-diagnostic margins remain `328 / 424` bytes. Gate 4 is `PHYSICAL_OLED=PASS`.

Evidence: Gate 2 `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`; Gate 3 `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`; Gate 4 `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`.

Publication commit/tree: `39690c9ef103cbcf93272df8bad0359a934b7dc1` / `f195fac5ce733c36a1d955e0fbe687ee6c83b605`; ordinary non-force publication complete; final ahead/behind `0/0`. Gate 6 evidence `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

Canonical design/acceptance/backlog:
`docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`
`docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`
`docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

### Current implementation boundary — Application runtime foundation

`APPLICATION_RUNTIME_FOUNDATION` — **GATES 0–5 ACCEPTED / GATE 6 NEXT**.

Accepted candidate: tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`; BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; ELF `77836` bytes / `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`; MAP `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`; Flash `48604/65536`; SRAM `10032/20480`; named runtime static state `184` bytes. Hardware accepted text/binary lifecycle, idempotent/invalid-start behavior, minute semantic-event delivery without rerender, CDC/UART/binary pressure `128/128`, power-cycle USB recovery, IWDG recovery and final exact Flash readback. Minimum observed task0/task1 margins after full application/binary activity are `272/424` bytes. Gate 4 is `PHYSICAL_OLED=PASS`.

Canonical design/acceptance:
`docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`
`docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`

Architectural follow-up:
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`

Implementation order:

1. `APPLICATION_RUNTIME_FOUNDATION` — Gates 0–5 accepted; Gate 6/7 publication next;
2. `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — mandatory behavior-preserving decomposition of the measured `src/kernel.c` god-module concentration before any new feature growth;
3. `USB_MANAGEMENT_DEVICE_FOUNDATION` — production Windows-facing vendor-specific WinUSB device (`Deus OS Device`), stable device-interface GUID, Microsoft OS descriptors, accepted binary RPC reused above transport; CDC/COM no longer the primary production host API;
4. `HOST_CONTROL_APPLICATION_FOUNDATION`;
5. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION`;
6. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`;
7. networking/service extensions.

Deferred optimization/robustness items are trigger-driven, not new immediate roadmap boundaries: I2C IRQ/DMA only after measured bus pressure; scheduler ready-set acceleration only after materially larger task count and measured overhead; CRC acceleration only for measured streaming cost; bounded minimal-copy/backpressure for WinUSB and transfer paths; tickless only for a real power requirement; structured binary event tracing rather than printf-heavy logging; bounded storage/block-device layers when a consumer exists.

### Phase 4 ordering constraint

Generic timer callbacks remain deferred until a real consumer requires them.
Message queues and synchronization remain deferred until a real cross-task
ownership boundary requires them. Runtime statistics remain a later
observability slice. The scheduler already has internal PRIMASK-protected
critical sections; that does not imply a public mutex/semaphore API. Heap,
filesystem, generic DMA framework, RTC/wall-clock service, MPU isolation and
general power-management framework are also consumer-driven, not missing
prerequisites. The 128x32 OLED geometry/gfx baseline remains frozen. `BOOT_DESKTOP_UI_FOUNDATION` established runtime-owned content/status semantics, and the accepted `OLED_DIRTY_REGION_OPTIMIZATION` changes only differential rendering/transfer behavior without changing that geometry.

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

1. Minimal STM32F103 USB Device core on PA11/PA12, implemented without HAL — published.
2. USB CDC ACM diagnostic/command console — published.
3. Transport-neutral shell/RPC service layer — published.
4. Binary framed RPC transport with CRC/request correlation — published; v1 is control/RPC only, not file transfer or firmware update.
5. Deus OS product/application/UI model freeze — published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`.
6. Boot/desktop UI foundation — published at `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`.
7. OLED dirty-region optimization — published at `39690c9ef103cbcf93272df8bad0359a934b7dc1`; exact changed page/column spans retained with one 512-byte framebuffer.
8. Static application runtime foundation — Gate 0 accepted; Gate 1 next.
9. Production USB management-device foundation: vendor-specific WinUSB `Deus OS Device`, Microsoft OS descriptors, stable interface GUID and the accepted binary RPC above transport; CDC becomes debug/recovery rather than the primary production API.
10. Cross-platform Windows/Linux host application. Provisional name: **Deus OS CP** (`Deus OS Control Panel`); it manages the stable firmware runtime rather than defining it.
11. Add bounded versioned asset/configuration transfer for non-executable packages.
12. Add a recoverable USB firmware-update path and small bootloader as a separate safety boundary.
13. Add networking/service extensions over the same application/service model.
14. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary management transport.

Production Windows integration should avoid a custom kernel-mode driver while also avoiding COM-port-first product identity. `USB_MANAGEMENT_DEVICE_FOUNDATION` therefore targets the Windows inbox WinUSB stack with firmware-supplied device identity/interface metadata. CDC remains valuable as an explicit development/debug/recovery profile, not as the final management surface.

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
