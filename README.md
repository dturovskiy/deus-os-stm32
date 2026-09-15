# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-15

Published parent remains:

`90df6a690230c9800c0d8497d5597f87a5ae0409` — `feat: add scheduler timed blocking foundation`

Published parent tree:

`c9e8cfd4bfa1dbbcc6d4d907b9c601c0d70fec59`

C3.8 static fixed-priority scheduling is **hardware accepted and awaiting local
acceptance commit / publication**.

Accepted C3.8 firmware:

- path: `build\scheduler_fixed_priority_v1\os.bin`
- bytes: `23792`
- SHA-256: `492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F`
- `src/kernel.c`: `235813864C83E7E813A31288DBE45635AF9948213CC3352A039FC4AC31D82A8D`
- `src/kernel/scheduler.c`: `B59B08373662B841C2CC077C92DE18D7FA21DA6DCE4E1DEE435F86AB58566C88`
- `include/kernel/scheduler.h`: `A48DCBB90D08FAD03F2D426AA2A129A8D8858204380D4A518D7094A318F5D841`

Accepted priority architecture:

- static task priority range `0..255`;
- lower numeric value means higher priority;
- default priority `128`;
- priority mutation is rejected while scheduler is active;
- one shared READY selector owns SVC/cooperative/host/PendSV choice;
- equal priorities retain round-robin tie order;
- cooperative production mode remains non-preemptive;
- task 0 remains production console/runtime at priority `128`;
- task 1 remains UNUSED;
- no new SVC;
- timed/event blocking semantics remain unchanged.

Hardware acceptance:

- `schedprio` phase `1/1` PASS;
- selector self-test exact `0x0000003F`;
- task0 priority `128 -> 128` across rejected live mutation;
- cooperative preempt-switch count `0`;
- safe production surface `20/20`;
- timed phases `4/4`;
- sleep `50 ms`, timeout `50 ms`;
- external UART event wake `171 ms`, event mask `0x00000001`;
- timed RX delta `26`, timed host-idle delta `3004`;
- invasive scheduler diagnostics `8/8` exact `SCHED_DIAG_BUSY`;
- retained race regression `4 x 32`, `128/128 PONG`;
- RX drops/errors/final depth `0/0/0`;
- production PSP stack `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash readback exact candidate;
- automated OLED runtime restore PASS.

Gate 4 physical OLED disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

OLED/gfx sources are byte-identical to C3.7 and no display behavior was changed.

Accepted evidence:

- build log SHA-256 `306AC5D97125F5BEFB4B2DB95E602ED8F6541E32CF364AA61ACD3D93A0E946D0`;
- build evidence SHA-256 `33E699282A456B79A0F15C8B2B6DF756BAD5979867091863599575A851CEA000`;
- hardware log SHA-256 `005D646FD613506896BC1A3961DDA752D9D424AD7F4AD0CFAAEE377C50E05222`;
- hardware evidence SHA-256 `41665A892477FB195D00DD722A093F69B09AB2A66669EB872DDB7A3518EACC6C`.

Current gate:

**C3.8 Gate 5 — documentation/evidence finalization complete when this exact
document set passes its finalization harness.**

Next:

1. Gate 6 — local acceptance commit;
2. Gate 7 — ordinary non-force fast-forward publication.
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
