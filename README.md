# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-16

Published baseline remains:

`3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca` — `feat: add IWDG liveness foundation`

Published tree:

`e024d425e97f878c170ccfc41f0505dce79277a3`

Native STM32F103 USB Device core foundation — **GATES 0–5 ACCEPTED / LOCAL COMMIT PENDING**

Boundary:

`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Accepted candidate:

- source candidate tree: `4b798382ef843ef8f488115624d48c0cd1506c75`;
- binary: `43812` bytes;
- SHA-256: `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`;
- direct-register STM32F103 USB FS Device core on PA11/PA12;
- accepted 72 MHz SYSCLK retained, `USBPRE=0` gives 48 MHz USB clock from PLL /1.5;
- EP0/PMA/BTABLE ownership and delayed `SET_ADDRESS` implemented;
- development identity: VID `0x1209`, PID `0x000A`, private testing only;
- no CDC ACM data transport in this boundary.

Hardware acceptance:

- host obtained exact device and configuration descriptors through EP0 control transfers;
- initial enumeration: address `20`, configuration `0`;
- physical micro-USB disconnect/reconnect recovered without reflashing at address `21`;
- destructive IWDG reset recovered UART and USB; reboot observed after `8748 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- USB proof completed in three sessions;
- safe production surface `20/20` before and after IWDG;
- timed event wake PASS with controlled UART RX event;
- invasive scheduler diagnostics `8/8` exact BUSY;
- UART race `128/128 PONG`, zero RX drops/errors;
- final Flash readback exactly matched the accepted candidate while USB VBUS remained the sole target power source.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 5 docs/evidence finalization is accepted. No build/flash/commit/push occurred in Gate 5.

Next gate:

**Gate 6 — local acceptance commit.**

After publication, the next architecture slice is **USB CDC ACM diagnostic/command console**.
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
- [ ] USB CDC ACM diagnostic/command console
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
