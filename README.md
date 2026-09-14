# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_14 -->
## Accepted project state — 2026-09-14

Current published Git baseline remains:

`bfed76e0de1c52bf65e60f66c029cc41710fabd0` — `feat: add scheduler steady-state wait/wake foundation`

The normal-boot production ownership migration is now **hardware + physical accepted locally** and awaits only acceptance commit + non-force publication.

Accepted C3.6 candidate:

- path: `build\normal_boot_production_ownership_atomic_racefix_v1\os.bin`
- bytes: `21832`
- SHA-256: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`
- scheduler core: `src/kernel/scheduler.c` SHA-256 `FA649EF24569AEE653A1CA022778237F76FF236A3F0B1B38FDDA8FB06F867649`
- production ownership source: `src/kernel.c` SHA-256 `FE046521F7A017B3DE204E984ECC392CA471C82824530B00EFCBFDFA433658F1`

Accepted production runtime:

- exactly one long-lived cooperative production console/runtime task on PSP;
- task slot 0 = production console, slot 1 = `UNUSED`;
- 1024-byte production PSP stack;
- MSP after bootstrap = scheduler host/idle + exception stack;
- host no-ready idle uses `WFE`;
- USART1 IRQ remains sole `DR` reader and ring producer;
- UART event is notification only; RX ring remains payload authority;
- production task uses drain-first / wait-second;
- runtime OLED/I2C application work runs from production PSP context;
- all eight invasive scheduler diagnostics remain blocked with exact `SCHED_DIAG_BUSY`;
- unexpected scheduler return remains fail-closed.

Hardware acceptance:

- boot-adjacent `ping -> PONG` PASS;
- complete safe production surface `18/18` PASS;
- invasive scheduler diagnostics `8/8` BUSY PASS;
- atomic scheduler race regression `4/4 rounds x 32 unpaced ping` PASS;
- aggregate burst responses `128/128 PONG`;
- exact RX byte/IRQ delta `200` per burst round;
- RX drops/errors/final depth = `0 / 0 / 0`;
- production PSP high-water = `580 / 1024`, free margin `444`, canary intact;
- final MSP high-water = `320 / 1984`, free margin `1664`, canary intact;
- final scheduler telemetry: active, cooperative, PSP in range, task 1 UNUSED, fault `0`;
- final target Flash readback exact accepted candidate;
- `uiruntime` restored the frozen product UI.

Manual physical OLED acceptance:

- `OLED PASS` confirmed on 2026-09-14;
- frozen UI visually accepted as `DEUS OS / BOOT OK / READY`.

The hardware-discovered host-idle race is closed by PRIMASK-atomic READY/BLOCKED/terminal classification. Terminal abort occurs while IRQs remain masked; blocked idle restores PRIMASK before `WFE`, with existing `SEV` semantics preserving wake safety.

Next execution gates:

1. local acceptance commit;
2. non-force fast-forward push;
3. then timer-backed `sleep()` / timed blocking.
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
