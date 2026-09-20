# Roadmap

Status: **CANONICAL FORWARD SEQUENCING — CURRENT PROJECT STATE IS OWNED BY `docs/CURRENT_STATE.md`**

The roadmap defines ordering and prerequisites. It does not independently define which boundary is active.

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Published boundary chronology and forward order

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

### Published implementation boundary — Application runtime foundation

`APPLICATION_RUNTIME_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `25752fba557b1a1b518265a93bde05d3a6a3f9ad`**.

Accepted candidate: tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`; BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; ELF `77836` bytes / `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`; MAP `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`; Flash `48604/65536`; SRAM `10032/20480`; named runtime static state `184` bytes. Hardware accepted text/binary lifecycle, idempotent/invalid-start behavior, minute semantic-event delivery without rerender, CDC/UART/binary pressure `128/128`, power-cycle USB recovery, IWDG recovery and final exact Flash readback. Minimum observed task0/task1 margins after full application/binary activity are `272/424` bytes. Gate 4 is `PHYSICAL_OLED=PASS`. Evidence: Gate 2 `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`; Gate 3 `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; Gate 4 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`. Publication commit/tree `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`; Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`.

Canonical design/acceptance:
`docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`
`docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`

### Published boundary — Kernel composition-root decomposition

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — **GATES 0–7 ACCEPTED / PUBLISHED `fa75307fb392718a1d10d52770a6a111c97208e7`**.

The accepted decomposition reduces `src/kernel.c` from `5100` to `3405` lines (-33.235%). `application_runtime_bridge` owns mutable application-runtime integration state/event snapshot/view adaptation; `application_commands` owns `rpcinfo/applist/appstart/appstop`; `scheduler_diagnostics` owns diagnostic orchestration. Root-private production scheduler observability remains in the composition root. No universal `kernel_context_t`, service locator, hidden extracted-state extern, heap, new task/SVC/queue/mutex/generic timer/DMA/persistence machinery or dependency cycle was introduced.

Accepted candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` bytes / `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536`, SRAM `10032/20480`, final task margins `384/424`. Gate 2 evidence `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; Gate 3 evidence `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; Gate 4 `PHYSICAL_OLED=PASS`.

Canonical decision/design/acceptance:
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`

### Published implementation boundary — USB management device foundation

`USB_MANAGEMENT_DEVICE_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `1f88083843c6aae9fd228ad2d677f9252b889a11`**.

Frozen topology is hardware-accepted on candidate tree `46841b52d351277deb134a6f4709619087b477af`, BIN `50172` / `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`: composite private-test `1209:000C` / `Deus OS Device`; CDC interfaces 0–1 retained as secondary diagnostics; vendor interface 2 automatically bound to inbox WinUSB over EP4 OUT/IN bulk64; Microsoft OS 2.0 first-configuration subset selector `0`; stable device-interface GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`; binary RPC v1 / command-service v2 / 35 methods unchanged. Gate 3 composite evidence/log are `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6` / `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`; WinUSB `128/128`, UART `32/32`, management drops `0/0`, task margins `448/424`, physical reconnect and IWDG recovery PASS, final Flash exact. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`. Canonical design/acceptance: `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` and `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.

### Published implementation boundary — Host control application foundation

`HOST_CONTROL_APPLICATION_FOUNDATION` is accepted through Gate 7 and published at `e0f49f168542fa1cf49bca451e01b0c077aa8d18`, tree `42c77f2cf3d7e9f7f5c1ff24d9d437f61a397be6`. Final candidate: firmware tree `b895955f7738aceb6fca0272d510cc433378c6ab`, host tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`, BIN `50652` / `FB68993FC998DE77B61FAF9F4949E4E124B95FF867BB456EBA401C9F2709F13F`. Windows WinUSB CLI/hardware, Windows Avalonia Desktop and Ubuntu 26.04.1/libusb hardware runtime all pass. Linux claims IF2 only and leaves CDC IF0/1 on `cdc_acm`; same-port reconnect passes with stable bus+port locator; 128 unique pings pass; final management/CDC drops are zero and final Flash is exact. Canonical design/acceptance: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` and `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`.
Implementation order:

1. `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — Gates 0–7 accepted / published `fa75307fb392718a1d10d52770a6a111c97208e7`;
2. `USB_MANAGEMENT_DEVICE_FOUNDATION` — Gates 0–7 accepted / published `1f88083843c6aae9fd228ad2d677f9252b889a11`;
3. `HOST_CONTROL_APPLICATION_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `e0f49f168542fa1cf49bca451e01b0c077aa8d18`**; accepted firmware/host trees `b895955f7738aceb6fca0272d510cc433378c6ab` / `2c5afd9914851300aed15e321cf69c3a2c3daeed`; Windows CLI + Desktop and real Linux libusb runtime/reconnect PASS; final management/CDC drops `0/0`, final Flash exact;
4. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — ordered after Host Control; activation/current gate is tracked only in `docs/CURRENT_STATE.md`;
5. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`;
6. networking/service extensions.

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
8. Static application runtime foundation — published at `25752fba557b1a1b518265a93bde05d3a6a3f9ad`.
9. Kernel composition-root decomposition — published at `fa75307fb392718a1d10d52770a6a111c97208e7`.
10. Production USB management-device foundation — published at `1f88083843c6aae9fd228ad2d677f9252b889a11`; vendor-specific WinUSB `Deus OS Device`, Microsoft OS descriptors, stable interface GUID and accepted binary RPC above transport; CDC is debug/recovery rather than the primary production API.
11. Cross-platform Windows/Linux host application — **Gate 0 accepted / Gate 1 next**. Provisional name: **Deus OS CP** (`Deus OS Control Panel`); C#/.NET 10 Core/CLI + Avalonia desktop, Windows WinUSB/Linux libusb adapters and system identity/capability discovery are frozen.
12. Add bounded versioned asset/configuration transfer for non-executable packages.
13. Add a recoverable USB firmware-update path and small bootloader as a separate safety boundary.
14. Add networking/service extensions over the same application/service model.
15. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary management transport.

Production Windows integration uses the accepted inbox WinUSB stack without a custom kernel-mode driver or COM-port-first product identity. `USB_MANAGEMENT_DEVICE_FOUNDATION` supplies the firmware device/interface metadata; CDC remains an explicit development/debug/recovery profile, not the primary management surface. `HOST_CONTROL_APPLICATION_FOUNDATION` now consumes that accepted interface.

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

### Historical next steps from this 2026-09-13 checkpoint

All items below were subsequently implemented through later accepted/published boundaries; they are retained only as chronology.

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
