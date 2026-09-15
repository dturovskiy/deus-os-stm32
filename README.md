# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-15

Current published parent remains:

`33f15f1d23dfa31fabf9f2c83542a5f34046cde8` — `feat: migrate normal boot to production scheduler ownership`

Published parent tree:

`a5447420a58b0aed7cd541069979fa665df40aa9`

C3.7 timed blocking is **hardware + physical OLED accepted and awaiting local acceptance commit / publication**.

Accepted C3.7 firmware:

- path: `build\scheduler_timed_blocking_v1\os.bin`
- bytes: `22900`
- SHA-256: `366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A`
- `src/kernel.c`: `44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC`
- `src/kernel/scheduler.c`: `90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618`
- `include/kernel/scheduler.h`: `F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547`

Accepted runtime architecture:

- one `SCHEDULER_TASK_BLOCKED` state plus explicit deadline metadata;
- `scheduler_sleep_ms()` and `scheduler_wait_events_timeout()`;
- SVC 4 timed-block path;
- existing 1 ms `kernel_ticks` remains clock authority;
- timeout comparison remains wrap-safe with max horizon `0x7FFFFFFF ms`;
- event wake versus timeout wake is first atomic `BLOCKED -> READY` transition wins;
- timeout wake publishes READY then `SEV`;
- normal production UART wait remains untimed;
- ring remains payload authority; event remains notification only.

Hardware acceptance:

- timed phases `4/4` PASS;
- sleep elapsed `50 ms`;
- timeout elapsed `50 ms`;
- external UART event wake elapsed `164 ms`, event mask `0x00000001`;
- timed RX byte/IRQ delta `26/26`;
- timed host-MSP idle delta `2904`;
- no double wake;
- injected ring payload survived and produced exact `PONG`;
- safe production surface `19/19`;
- invasive scheduler diagnostics `8/8` exact `SCHED_DIAG_BUSY`;
- retained race regression `4 x 32`, `128/128 PONG`;
- RX drops/errors/final depth `0/0/0`;
- production PSP stack `580 used / 444 margin`, canary intact;
- MSP `340 used / 1644 margin`, canary intact;
- final Flash readback exact candidate;
- automated OLED runtime restore PASS;
- physical frozen OLED `DEUS OS / BOOT OK / READY` PASS.

Accepted evidence:

- build log SHA-256 `6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84`;
- build evidence SHA-256 `431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199`;
- hardware log SHA-256 `3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769`;
- hardware evidence SHA-256 `ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4`;
- physical OLED token: `OLED PASS`.

Current gate:

**C3.7 Gate 5 — documentation/evidence finalization complete when this exact
document set passes its finalization harness.**

Next:

1. Gate 6 — local acceptance commit;
2. Gate 7 — non-force fast-forward publication;
3. after publication: scheduler priorities.

Do not begin priorities before C3.7 publication completes.
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
- [x] Production scheduler lifecycle / diagnostic isolation
- [x] Production scheduler steady-state block/event/wake foundation
- [x] Normal-boot scheduler ownership migration
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
