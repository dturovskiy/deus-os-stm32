# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_13 -->
## Accepted project state — 2026-09-13

The current published baseline is commit `8bc1510265d0377adafda663d320c1e55f5b2c3a` (`feat: add console PSP stack-budget probe`). The lifecycle/diagnostic-isolation source delta below is hardware- and physically accepted but not yet committed.

Current hardware/runtime baseline:

- STM32F103C8T6 / Cortex-M3, 64 KiB Flash, 20 KiB SRAM.
- 72 MHz HSE/PLL clock and 1 kHz SysTick.
- fault capture for HardFault, MemManage, BusFault, and UsageFault.
- bidirectional USART1 console at 115200 8N1, with IRQ37 as the sole `USART1_DR` reader and a 128-byte SPSC RX ring.
- normal boot console remains MSP-owned and returns to `WFI` when idle.
- native 128x32 SSD1306-compatible OLED at I2C `0x3C`; frozen accepted UI remains `DEUS OS / BOOT OK / READY`.

Published scheduler/runtime milestones:

- Slice 8 runtime UI boot lifecycle: `4217865403d9725707f4c17e572317caf7fe733f`.
- Slice 9A scheduler foundation: `1a57f79cda42674219e774900ce07a0da8fedaf4`.
- Slice 9B cooperative scheduler activation: `1114621e9a6bc57d5471cf51a922c216b76bebe2`.
- PendSV timer-driven preemption: `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.
- Scheduler stack canary/high-water instrumentation: `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.
- Substantive command-gated PSP workload: `8f6b922a7d2e55abc3133702e7571057f995da5d`.
- USART1 RX IRQ + 128-byte ring-buffer foundation: `53054e16c5b4dbb54626b0f080c9492629ccb285`.
- MSP runtime high-water/guard telemetry: `d3efae463cd65e087f0c1a3556de640105ed1b44`.
- Console PSP stack-budget probe: `8bc1510265d0377adafda663d320c1e55f5b2c3a`.

Previously accepted sizing remains authoritative:

- substantive OLED PSP workload: `328 / 512 bytes`; peer `80 / 512 bytes`; canaries intact.
- dedicated MSP reservation: `2048 bytes`; usable `1984 bytes`; accepted historical stress high-water `596 bytes` with `1388-byte` minimum margin.
- safe console PSP surface: exact `17` non-scheduler commands on a dedicated `1024-byte` stack; published acceptance high-water `600 bytes`, minimum margin `424 bytes`; 512-byte full-console migration remains rejected.

The production scheduler lifecycle / scheduler-diagnostic isolation gate is now hardware- and physically accepted:

- source delta remains exactly `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- `scheduler_init()` is status-returning and rejects reset while the scheduler is active before any global scheduler-state mutation.
- `scheduler_is_active()` is a read-only lifecycle query.
- the six published invasive scheduler diagnostics are centrally gated while active and return exact `SCHED_DIAG_BUSY`; idle behavior/output remains preserved.
- new offline `schedisolate` proves the active-reset rejection and diagnostic gate under real PendSV preemption.
- candidate binary: `18324 bytes`.
- SHA-256: `9AFDE9AC5AF196E98A2896BAC0DD7AE610414FB5A2888F1D499D2A5E0A12794B`.
- `.bss=5248 bytes`; `_ebss=0x20000C80`; RAM gap below MSP reservation `15232 bytes`.
- linked `fault_record=0x20000764`; console probe stack `0x20000060 / 1024 bytes`.
- `4 / 4` lifecycle-isolation rounds passed, each with exactly `6` `SCHED_DIAG_BUSY` lines, `INIT_REJECT=1`, `BLOCKED_DIAGNOSTICS=6`, `ACTIVE_PRESERVED=1`, `OVERLAP=1`, and both task canaries intact.
- isolation task high-water: `160 / 512 bytes`; peer high-water: `88 / 512 bytes`; `16` switches.
- all six published scheduler diagnostics passed before isolation and again after isolation.
- standalone console PSP regression: `2 / 2` PASS.
- composite console PSP + UART pressure: `4 / 4` PASS with exact `+105` IRQ / `+105` byte deltas each round.
- accepted console PSP high-water in this gate: `560 / 1024 bytes`; minimum margin `464 bytes`; peer `88 / 512 bytes`; maximum probe switches `693`.
- RX high-water `80 / 128`; drops `0`; errors `0`; depth returned to zero after stress.
- MSP high-water `320 / 1984`; minimum margin `1664`; canary intact.
- fresh reset + fresh isolation + fresh console probe passed.
- final exact target readback matched the candidate.
- manual physical OLED acceptance confirmed unchanged: `DEUS OS / BOOT OK / READY`.

This closes the scheduler-lifecycle contradiction without changing normal-boot ownership. The production scheduler is still **not** started during normal boot, and normal-boot console migration remains deferred.

The next controlled boundary is **production scheduler steady-state wait/wake foundation**: define and prove persistent-task blocked/waiting and wake/event semantics plus stable idle ownership without migrating normal boot yet. Normal-boot task ownership migration remains a later separate gate.

Native USB remains a planned first-class transport. The provisional cross-platform Windows/Linux host application name is **Deus OS CP** (`Deus OS Control Panel`); naming may change without changing transport/protocol architecture.
<!-- END STM32_OS_ACCEPTED_STATE_2026_09_13 -->

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
- [ ] Native USB Device / CDC console
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
- [ ] Production scheduler lifecycle / diagnostic isolation
- [ ] Normal-boot scheduler ownership migration
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

Next implementation boundary after publication: **normal-boot production task ownership / migration**. `sleep()` / timer blocking and scheduler priorities remain separate later gates.
<!-- END STM32_OS_SCHED_WAIT_WAKE_ACCEPTED_20260913 -->
