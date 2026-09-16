# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-16

Published baseline:

`5a8a45618b87b3069fd7cbac6119035b6ac4ad2c` — `feat: add USB CDC ACM console foundation`

Published tree:

`bbc6b24e28279075c41410e8abdace89c4b805b7`

USB CDC ACM diagnostic/command console foundation — **PUBLISHED**

Accepted core candidate:

- source candidate tree: `4b798382ef843ef8f488115624d48c0cd1506c75`;
- binary: `43812` bytes;
- SHA-256: `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`;
- private-test identity `1209:000A` / `Deus OS USB Core`;
- exact EP0 enumeration/address proof, reconnect recovery, post-IWDG recovery, retained UART/scheduler/IWDG regressions, final Flash exact;
- OLED Gate 4: `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Current architecture boundary:

`SHELL_RPC_FOUNDATION` — **GATES 0–5 ACCEPTED / GATE 6 NEXT**

Accepted CDC candidate:

- tested source tree `d815a8357b9f77850c08ff071f47be8d2b5d4b53`;
- binary `58548` bytes;
- SHA-256 `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- private-test identity `1209:000B` / `Deus OS CDC Console`;
- Windows inbox `usbser.sys`, configuration `1`, no custom INF;
- CDC Control + CDC Data over EP1 notification, EP2 OUT, EP3 IN;
- task0 owns both UART and CDC command transport with independent parser state and origin-bound responses;
- bounded CDC RX/TX rings `1024` / `2048` bytes;
- automatic USB detach/attach recovery across MCU/IWDG reset via bounded PA12/D+ low pulse;
- software-reset attach, physical reconnect, post-IWDG CDC reopen, class-control, pressure, scheduler, UART and dual-transport acceptance PASS;
- final Flash exact;
- OLED Gate 4: `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Canonical design/acceptance:

- `docs/USB_CDC_ACM_CONSOLE_PLAN.md`
- `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md`

Accepted shell/RPC candidate:

- candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`;
- binary `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- static allocation-free registry with `32` methods;
- transport-neutral execution context and response writer;
- `help` / `help ping` / `rpcinfo`, bounded `4` args and `32`-byte line capacity;
- UART/CDC editing, parser isolation, origin-bound responses and retained pressure/scheduler/IWDG regressions hardware accepted;
- physical USB reconnect, post-IWDG automatic CDC recovery and final exact Flash readback PASS;
- OLED Gate 4: `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Next gate:

**Gate 6 — local acceptance commit of the accepted shell/RPC foundation.**

Canonical planning:

- `docs/SHELL_RPC_FOUNDATION_PLAN.md`
- `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`
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
