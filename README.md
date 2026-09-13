# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_13 -->
## Accepted project state — 2026-09-13

The current hardware-accepted baseline is:

- STM32F103C8T6 / Cortex-M3, 64 KiB Flash, 20 KiB SRAM.
- 72 MHz HSE/PLL clock and 1 kHz SysTick.
- fault capture for HardFault, MemManage, BusFault, and UsageFault.
- bidirectional USART1 console at 115200 8N1:
  - A9 = TX
  - A10 = RX
  - `ping -> PONG`
  - `health`, `fault`, `i2cscan`, OLED regression commands
  - `schedtest -> SCHED_FOUNDATION_OK`
  - `schedcoop -> SCHED_COOP_OK`
  - `schedpreempt -> SCHED_PREEMPT_OK`
  - `schedstack -> SCHED_STACK_WATER_OK`
  - `schedworkload -> SCHED_WORKLOAD_OK`.
- native 128x32 SSD1306-compatible OLED at I2C address `0x3C`:
  - B6 = SCL
  - B7 = SDA
  - 512-byte framebuffer
  - frozen accepted status bar + retained 21x3 console
  - dirty-page presentation integrated into the runtime UI lifecycle.
- Slice 8 runtime UI boot lifecycle accepted and published:
  - acceptance commit `4217865403d9725707f4c17e572317caf7fe733f`.
- Slice 9A scheduler foundation accepted and published:
  - acceptance commit `1a57f79cda42674219e774900ce07a0da8fedaf4`.
- Slice 9B cooperative scheduler activation accepted and published:
  - acceptance commit `1114621e9a6bc57d5471cf51a922c216b76bebe2`.
- PendSV timer-driven preemption accepted and published:
  - acceptance commit `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`
  - real timer-driven preemption passed across `34` complete runs.
- Scheduler stack canary/high-water instrumentation accepted and published:
  - acceptance commit `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`
  - task stacks remain exactly `2 x 512 bytes`
  - synthetic cooperative/preemptive task paths measured `72 bytes`
  - synthetic observed free margin `440 bytes`
  - canary intact across `34` `schedstack` commands / `68` scheduler runs.
- Command-gated substantive PSP workload is now hardware-accepted and awaiting its acceptance commit:
  - task 0 executes the frozen OLED runtime full render/present path on PSP
  - task 1 is a CPU-only preemption peer with no UART/I2C/OLED access and no voluntary yield
  - public `scheduler_start_preemptive()` wrapper added
  - read-only PendSV switch-count telemetry added
  - candidate binary `14292 bytes`
  - SHA-256 `8C124B0954D65E0F698AD1C62525E72FC4F569D5133EF8F0CE67A8297A80A2CF`
  - `.bss=1952 bytes`; `_ebss=0x200007A0`; SRAM headroom `18528 bytes`
  - planning feasibility estimate for `oled_runtime_ui_show()`: `212 + 64 = 276 bytes`
  - source-build feasibility estimate for workload task 0: `220 + 64 = 284 bytes`
  - source-build feasibility estimate for workload task 1: `12 + 64 = 76 bytes`
  - real hardware task 0 high-water: `328 / 512 bytes`; runtime margin `184 bytes`
  - real hardware task 1 high-water: `80 / 512 bytes`; runtime margin `432 bytes`
  - PendSV switch count observed `124..126` per workload run
  - `WORKLOAD_UI_RESULT=1`, `WORKLOAD_PEER_OVERLAP=1`, and both canaries intact for every accepted workload run
  - first real workload PASS, 32/32 stress PASS, final workload PASS: `34` complete workload runs
  - legacy `schedtest`, `schedcoop`, `schedpreempt`, `schedstack`, UART, I2C, and OLED regressions PASS
  - hardware v1 had exactly two false-negative boot checks because the harness still expected stale `FAULTREC=0x20000270`
  - linked `fault_record` is `0x20000284`; recovery v2 proved both captured v1 boot frames valid and passed two fresh corrected boot checks without reflashing
  - final exact target readback PASS
  - physical frozen OLED output confirmed unchanged:
    - `DEUS OS`
    - `BOOT OK`
    - `READY`.

The `.su` direct-call-chain feasibility estimate is not a conservative stack upper bound for this workload: task 0 measured `328 bytes` at runtime versus the `284-byte` source-build estimate. Runtime canary/high-water evidence is therefore authoritative for stack sizing; static call-chain analysis remains a feasibility/sanity tool unless strengthened.

For the exact tested frozen OLED render/present workload, a 512-byte PSP stack has `184 bytes` (35.9%) measured free margin. This does not yet prove a future console task, arbitrary OLED workload, or kernel/MSP stack budget.

Normal boot task migration remains deferred; console/OLED steady-state ownership still uses the existing kernel/MSP path.

Next scheduler boundary: define the production task ownership/stack budget using the accepted workload evidence, separately prove any console-task and MSP/kernel stack requirements, then design the normal-boot migration gate. Do not combine that design decision with an unreviewed steady-state migration.
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
- [x] USART1 A9/A10 bidirectional polling command/diagnostic console
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
- [ ] Task stack budget / high-water validation before normal-boot task migration
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

Native USB is planned to eventually consolidate normal console/control/update traffic onto the board's micro-USB connector.
<!-- END STM32_OS_DEV_LOOP -->
