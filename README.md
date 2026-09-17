# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-17

Published firmware/source baseline:

`d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8` — `feat: add boot desktop UI foundation`

Published tree:

`d27cf8246fb7563b2327955ffc06428b9d843b2a`

Boot / desktop UI foundation — **GATES 0–7 ACCEPTED / PUBLISHED**

Accepted firmware:

- tested candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`;
- binary `41520` bytes / SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`;
- Flash `41520 / 65536`, SRAM `9792 / 20480`;
- nonblocking `BOOT_SPLASH -> DESKTOP_HOME`, real SYSTEM/USB/NETWORK status semantics and monotonic uptime accepted;
- steady-state semantic refresh never re-enters SSD1306 initialization/display-off;
- retained CDC/UART/binary pressure, reconnect, malformed-frame recovery and IWDG recovery accepted;
- physical OLED `PHYSICAL_OLED=PASS` with clean minute/text transitions and no blank pulse/flicker/stale pixels;
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

Latest published firmware boundary:

`BOOT_DESKTOP_UI_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**.

Canonical design/acceptance:

- `docs/BOOT_DESKTOP_UI_PLAN.md`
- `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`

The accepted revision-2 implementation provides a nonblocking `BOOT_SPLASH -> DESKTOP_HOME` lifecycle on the existing 128x32 UI: 1000 ms minimum visible splash dwell without delaying scheduler/IWDG startup, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK indicator semantics, monotonic uptime `HH:MM`, redraw only on visible semantic change, and no new task/queue/timer subsystem. Steady-state redraw never re-enters SSD1306 initialization/display-off; panel reinitialization is recovery-only.

Accepted Gate 2/3 candidate: tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN `41520` bytes / SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`, ELF `70700` bytes / SHA-256 `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`, Flash `41520 / 65536`, SRAM `9792 / 20480`. Gate 3 retained CDC/UART/binary pressure, reconnect, IWDG recovery and exact final Flash readback all passed. Gate 4 is operator-confirmed `PHYSICAL_OLED=PASS`: minute/text transitions are clean, with no blank pulse, flicker, stale pixels or unintended redraw artifacts.

Current implementation boundary:

`OLED_DIRTY_REGION_OPTIMIZATION` — **GATES 0–5 ACCEPTED / GATE 6 NEXT**.

Accepted candidate/evidence:

- candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- ELF `71308` bytes / SHA-256 `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`;
- Gate 2 evidence `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`;
- Gate 3 evidence `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`;
- Gate 4 evidence `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`, `PHYSICAL_OLED=PASS`;
- hardware transfer proof: historical full semantic refresh `572` payload bytes / `36` writes, clean present `0`, one-byte narrow update `9`, minute update `11`, USB update `11`;
- task0 margin after `oledstatus` and after `oleddirty` = `328` bytes; task1 margin = `424` bytes;
- Gate 5 documentation/evidence finalization is accepted; no firmware source change occurs in Gate 5.

Canonical design/acceptance/backlog:

- `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`
- `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`
- `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

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
- [x] OLED dirty-region transfer optimization (Gates 0–5 accepted; publication pending)
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
