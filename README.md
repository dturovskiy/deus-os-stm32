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
  - RX ownership is now interrupt-driven: `USART1_IRQHandler` (IRQ37) is the sole reader of `USART1_DR`
  - RX bytes enter a 128-byte single-producer/single-consumer ring
  - the existing `uart_try_getc()` remains the consumer API used by the MSP-owned console
  - normal idle uses `WFI` again because SysTick/USART1 IRQs wake the core
  - `rxstat -> RX_IRQ_RING_OK`
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

Published scheduler/runtime milestones:

- Slice 8 runtime UI boot lifecycle: `4217865403d9725707f4c17e572317caf7fe733f`.
- Slice 9A scheduler foundation: `1a57f79cda42674219e774900ce07a0da8fedaf4`.
- Slice 9B cooperative scheduler activation: `1114621e9a6bc57d5471cf51a922c216b76bebe2`.
- PendSV timer-driven preemption: `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.
- Scheduler stack canary/high-water instrumentation: `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.
- Substantive command-gated PSP workload: `8f6b922a7d2e55abc3133702e7571057f995da5d`.
- USART1 RX IRQ + 128-byte ring-buffer foundation: `53054e16c5b4dbb54626b0f080c9492629ccb285`.

Accepted substantive PSP workload evidence remains authoritative:

- exact frozen OLED render/present workload task: `328 / 512 bytes`, measured free margin `184 bytes`
- CPU-only peer: `80 / 512 bytes`, measured free margin `432 bytes`
- PendSV switch count observed `124..126`
- `34` complete workload runs
- both task canaries intact in every accepted run
- static direct-call-chain calculation is a feasibility estimate, not a conservative upper bound.

The production-ownership decision audit then proved that direct normal-boot migration is still blocked:

- current scheduler is a global run-to-completion host launcher
- scheduler diagnostic self-tests reinitialize global scheduler state and are unsafe inside an active production scheduler
- scheduler blocked/sleep/wake states are absent
- full current console linked feasibility estimate is `524 bytes`; with the 64-byte architecture context reserve this is `588 bytes`, so a 512-byte console PSP stack is rejected
- kernel/MSP runtime stack budget was still unproven at that audit boundary.

The first prerequisite from that decision is now hardware-accepted: USART1 RX IRQ + ring-buffer foundation.

RX IRQ/ring accepted candidate:

- source delta: `src/kernel.c`, `src/startup.s`
- candidate binary: `15084 bytes`
- SHA-256: `E25DC54C149EB9DA7B26F5378868DAA790847A1C4C7B4CFB970F294CFB738EFB`
- `.bss=2112 bytes`; `_ebss=0x20000840`; SRAM headroom `18368 bytes`
- vector table contains USART1 at external IRQ37
- linked `fault_record=0x20000320`
- USART1 IRQ own static frame: `12 bytes`
- USART1 NVIC priority: `0x80`
- hardware burst proof:
  - four rounds of `32 x ping` produced all `32/32` exact `PONG` responses per round
  - each round plus the following `rxstat` produced exactly `+167` IRQs and `+167` received bytes
  - observed ring high-water: `29 / 128 bytes`
  - RX drops: `0`
  - RX errors: `0`
  - depth after every accepted burst: `0`
  - fresh-reset burst repeated the exact `167` IRQ / `167` byte result with high-water `29`
- scheduler, health, I2C, OLED and substantive PSP workload regressions remained PASS
- final exact target readback matched the accepted candidate
- physical frozen OLED output remained:
  - `DEUS OS`
  - `BOOT OK`
  - `READY`.

The second prerequisite is now hardware- and physically accepted: dedicated MSP runtime high-water/guard instrumentation.

MSP runtime stack proof:

- source delta: `src/kernel.c`, `src/startup.s`, `linker/stm32f103c8.ld`
- candidate binary: `15620 bytes`
- SHA-256: `C15634184CAB7BA3C5CE503773EB7BA4BB45DDB4B32A3D399F6BE587C518D907`
- dedicated MSP reservation: `2048 bytes` at `0x20004800..0x20005000`
- guard: `64 bytes` at `0x20004800..0x20004840`
- usable watermark capacity: `1984 bytes`
- Reset_Handler fills guard + watermark before its first `BL kernel_main`
- initial measured MSP use: `400 bytes`; margin `1584 bytes`
- accepted stress high-water: `596 bytes`; minimum measured margin `1388 bytes`
- MSP canary remained intact
- `12 / 12` nested-pressure rounds passed across OLED status, PendSV preemption diagnostics, and substantive PSP workload
- each composite round included `16 x ping`; RX high-water reached `80 / 128 bytes` with zero drops/errors and depth returning to zero
- fresh-reset pressure proof passed with MSP use `572 bytes`, margin `1412 bytes`
- final scheduler workload and exact flash readback passed
- physical frozen OLED remained `DEUS OS / BOOT OK / READY`.

Normal boot task migration remains deferred. The console is still MSP-owned and the production scheduler is not started during normal boot.

The next controlled boundary is **console PSP stack-budget foundation**. The current full-console 512-byte PSP size remains rejected; console workload sizing must be proven explicitly before any normal-boot ownership migration. Scheduler diagnostic isolation/lifecycle and a future wait/block model remain separate later gates.
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
- [ ] Console PSP stack budget and production scheduler ownership migration
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
