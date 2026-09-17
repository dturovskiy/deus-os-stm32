# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-17

Published firmware/source baseline:

`39690c9ef103cbcf93272df8bad0359a934b7dc1` — `feat: optimize OLED dirty region updates`

Published tree:

`f195fac5ce733c36a1d955e0fbe687ee6c83b605`

OLED dirty-region optimization — **GATES 0–7 ACCEPTED / PUBLISHED**

Accepted firmware:

- tested candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- binary `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`;
- exact dirty-span OLED transfer: full semantic baseline `572` payload bytes / `36` writes, clean `0`, one-byte update `9`, minute `11`, USB `11`;
- task0/task1 post-diagnostic margins `328 / 424` bytes;
- retained CDC/UART/binary pressure, reconnect, malformed-frame recovery and IWDG recovery accepted;
- physical OLED `PHYSICAL_OLED=PASS` with no blank/off pulse, stale pixels, clipping or console corruption;
- final Flash readback exact;
- ordinary non-force publication complete; local/remote ahead-behind `0/0`.

Latest published architecture foundation:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`**

This docs-only boundary defines Deus OS as a deterministic embedded device runtime, freezes the initial static application/lifecycle model, separates firmware ownership from future host Control Panel responsibilities, defines boot-splash/desktop/application-view UI lifecycle and real status-bar semantics, and explicitly defers arbitrary uploaded ARM executables, filesystem, bootloader and firmware update to later independent boundaries.

Canonical architecture docs:

- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`
- `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`
- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

The foundation-gap review confirms no missing kernel blocker before boot/desktop/application work. It additionally freezes later contracts for semantic application events, system identity/capabilities, bounded persistence safety, crash/reset observability, security/trust before network mutation or firmware update, and portability layering. Generic timers/queues/synchronization/runtime statistics remain consumer-driven rather than speculative prerequisites.

Previous published UI boundary:

`BOOT_DESKTOP_UI_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**.

Canonical design/acceptance:

- `docs/BOOT_DESKTOP_UI_PLAN.md`
- `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`

The accepted revision-2 implementation provides a nonblocking `BOOT_SPLASH -> DESKTOP_HOME` lifecycle on the existing 128x32 UI: 1000 ms minimum visible splash dwell without delaying scheduler/IWDG startup, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK indicator semantics, monotonic uptime `HH:MM`, redraw only on visible semantic change, and no new task/queue/timer subsystem. Steady-state redraw never re-enters SSD1306 initialization/display-off; panel reinitialization is recovery-only.

Accepted Gate 2/3 candidate: tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN `41520` bytes / SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`, ELF `70700` bytes / SHA-256 `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`, Flash `41520 / 65536`, SRAM `9792 / 20480`. Gate 3 retained CDC/UART/binary pressure, reconnect, IWDG recovery and exact final Flash readback all passed. Gate 4 is operator-confirmed `PHYSICAL_OLED=PASS`: minute/text transitions are clean, with no blank pulse, flicker, stale pixels or unintended redraw artifacts.

Latest published firmware boundary:

`OLED_DIRTY_REGION_OPTIMIZATION` — **GATES 0–7 ACCEPTED / PUBLISHED `39690c9ef103cbcf93272df8bad0359a934b7dc1`**.

Publication tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`; Gate 6 evidence `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

Canonical design/acceptance/backlog:

- `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`
- `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`
- `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

Current implementation boundary:

`APPLICATION_RUNTIME_FOUNDATION` — **GATE 0 ACCEPTED / GATE 1 NEXT**.

Gate 0 freezes a static two-application runtime (`system.home=0x0001`, `device.info=0x0002`), task0-only lifecycle/event dispatch, bounded pointer-free semantic events, a bounded three-row application view model, and transport-neutral `applist/appstart/appstop` methods appended as RPC IDs `0x0021..0x0023` without renumbering the existing `0x0001..0x0020` surface. No heap, queue, new task/SVC, dynamic loader, filesystem or USB redesign is introduced.

Canonical design/acceptance:

- `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`
- `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`

Implementation order:

`OLED_DIRTY_REGION_OPTIMIZATION` -> `APPLICATION_RUNTIME_FOUNDATION` -> `USB_MANAGEMENT_DEVICE_FOUNDATION` -> `HOST_CONTROL_APPLICATION_FOUNDATION` -> asset/config transfer -> recoverable firmware update/bootloader -> networking extensions.

`USB_MANAGEMENT_DEVICE_FOUNDATION` will make the production Windows-facing device a vendor-specific WinUSB management device rather than a COM-port-first CDC console. The existing binary framed RPC remains the management protocol above the transport; production USB naming/identity, Microsoft OS descriptors, a stable device-interface GUID and WinUSB bulk transport belong to that boundary. CDC may remain only as an explicit debug/recovery profile if later justified.

The preceding docs-only application/UI foundation is fully accepted and was published by ordinary non-force fast-forward at commit `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`, tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`.
<!-- END STM32_OS_ACCEPTED_STATE_2026_09_14 -->

A small bare-metal operating system for the STM32F103 Cortex-M3.

The project is built from scratch to study low-level ARM programming,
microcontroller architecture, exceptions, interrupts, scheduling, drivers,
and operating-system fundamentals.

The low-level kernel intentionally does not use STM32 HAL, Arduino, or FreeRTOS.

## Target

- MCU family: STM32F103 medium-density
- CPU: ARM Cortex-M3
- Architecture: ARMv7-M / Thumb-2
- Flash target: 64 KiB
- SRAM target: 20 KiB
- Debug/program interface: SWD
- Programmer/debugger: ST-LINK V2

## Current status

Built locally:

- [x] ARM GNU bare-metal toolchain
- [x] Custom linker script
- [x] Custom vector table
- [x] Custom Reset_Handler
- [x] `.data` initialization
- [x] `.bss` initialization
- [x] `kernel_main()`
- [x] ELF/BIN image generation
- [x] First flash of our kernel
- [x] GPIO status LED
- [x] Clock configuration
- [x] SysTick
- [x] Fault diagnostics
- [x] USART1 A9/A10 bidirectional IRQ/ring-buffer command/diagnostic console
- [x] Stable 1 ms kernel time API with wraparound-safe comparisons
- [x] USART1 RX / bidirectional command console
- [x] `uptime` kernel introspection command
- [x] `health` automated SysTick/PC13 regression command
- [x] `fault` read-only fault diagnostics / SCB register dump command
- [x] Native USB Device core foundation
- [x] USB CDC ACM diagnostic/command console
- [x] Transport-neutral shell/RPC command-service foundation
- [x] Binary framed USB CDC RPC transport foundation
- [x] I2C1 master + hardware bus scan (B6/B7, 100 kHz, SSD1306 at 0x3C)
- [x] SSD1306 command transport at 0x3C (`oledping` / NOP transaction)
- [x] Native 128x32 SSD1306 runtime UI with frozen status bar + retained 21x3 console
- [x] SSD1306 retained kernel/status console
- [x] Scheduler foundation: static TCBs/stacks + synthetic initial task frames
- [x] Cooperative scheduler activation: PSP tasks + SVC start/yield/exit
- [x] PendSV context switching + command-gated SysTick preemption proof
- [x] Representative PSP task high-water validation for the accepted frozen OLED workload
- [x] USART1 RX IRQ + 128-byte ring-buffer foundation with hardware burst proof
- [x] Kernel/MSP runtime high-water / guard proof
- [x] Console PSP stack budget for the accepted 17-command safe surface
- [x] Production scheduler lifecycle / diagnostic isolation
- [x] Production scheduler steady-state block/event/wake foundation
- [x] Normal-boot scheduler ownership migration
- [x] Boot splash / desktop runtime UI foundation (hardware + physical OLED accepted)
- [x] OLED dirty-region transfer optimization (Gates 0–7 accepted / published)
- [ ] Application runtime foundation (Gate 0 accepted / Gate 1 next)
- [ ] IPC primitives
- [ ] ESP8266 networking

## Project structure

```text
OS/
├── build/
├── docs/
├── include/
├── linker/
│   └── stm32f103c8.ld
├── scripts/
└── src/
    ├── startup.s
    └── kernel.c
```

## Design principles

- Bare metal first.
- Keep the boot path explicit.
- Prefer direct memory-mapped register access while learning the hardware.
- Add abstractions only after the underlying mechanism is understood.
- Measure before optimizing.
- Keep platform-specific code isolated from kernel policy.
- Do not introduce networking into the kernel until the local kernel baseline is stable.

## License

MIT

<!-- BEGIN STM32_OS_DEV_LOOP -->
## Current development loop

The project now has an automated hardware validation loop:

source -> build -> ELF/bin validation -> ST-LINK flash -> verify -> reset -> UART capture -> PASS/FAIL -> evidence log

Current USART1 is bidirectional at 115200 8N1 and is part of the automated hardware acceptance loop.

Native USB is planned to eventually consolidate normal console/control/update traffic onto the board's micro-USB connector. The provisional Windows/Linux host application name is **Deus OS CP** (`Deus OS Control Panel`); naming may be revised later.
<!-- END STM32_OS_DEV_LOOP -->

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_ACCEPTED_20260913 -->
## Production scheduler steady-state wait/wake foundation — accepted 2026-09-13

The production scheduler now has an accepted steady-state wait/wake foundation while normal boot remains MSP-owned.

Accepted behavior:

- explicit `SCHEDULER_TASK_BLOCKED` state alongside UNUSED / READY / DONE;
- task-side event wait through `scheduler_wait_events()` / SVC #3;
- ISR-safe `scheduler_event_signal()` wake path;
- pending-event handling closes the wait-vs-signal race;
- no-runnable scheduler state parks the preserved host MSP with `WFE` rather than busy-spinning or falsely completing the run;
- UART RX IRQ publishes an event only after the RX byte is committed to the ring;
- event consumers treat an event as a notification to re-check the FIFO/condition, not as the payload itself;
- normal reset still runs the product console on MSP and does not automatically start the production scheduler.

Accepted firmware:

- candidate: `build\scheduler_wait_wake_foundation_v2\os.bin`
- size: **19932 bytes**
- SHA-256: `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`
- hardware wait/wake rounds: **4/4 PASS**
- legacy scheduler regression: **6/6 PASS before and after**
- lifecycle diagnostic isolation: **7/7 blocked while active**
- UART RX: zero drops / zero errors
- task and MSP canaries: intact
- frozen OLED runtime regression: PASS
- physical OLED appearance: **operator-confirmed PASS**

Harness/evidence/recovery procedures are documented in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

Next implementation boundary: **normal-boot production task ownership / migration**. The canonical design is in `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md` and its hardware/gate contract is in `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`. `sleep()` / timer blocking and scheduler priorities remain separate later gates.
<!-- END STM32_OS_SCHED_WAIT_WAKE_ACCEPTED_20260913 -->
