# STM32 OS

<!-- BEGIN STM32_OS_ACCEPTED_STATE_2026_09_12 -->
## Accepted project state — 2026-09-12

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
  - `schedcoop -> SCHED_COOP_OK`.
- native 128x32 SSD1306-compatible OLED at I2C address `0x3C`:
  - B6 = SCL
  - B7 = SDA
  - 512-byte framebuffer
  - frozen accepted status bar + retained 21x3 console
  - dirty-page presentation integrated into the runtime UI lifecycle.
- Slice 8 runtime UI boot lifecycle accepted:
  - binary `10148 bytes`
  - SHA-256 `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`.
- Slice 9A scheduler foundation accepted and published:
  - acceptance commit `1a57f79cda42674219e774900ce07a0da8fedaf4`
  - two static TCBs
  - two 512-byte static task stacks
  - synthetic Cortex-M initial frames
  - foundation self-test `SCHED_FOUNDATION_OK`.
- Slice 9B cooperative scheduler activation hardware-accepted:
  - task Thread mode runs on PSP
  - SVC `#0` starts the first prepared task
  - SVC `#1` performs voluntary cooperative yield
  - SVC `#2` handles normal task return/exit
  - parked kernel/MSP context is restored after all prepared tasks complete
  - initial stacked PC remains halfword-aligned with Thumb state supplied by xPSR.T
  - stacked LR retains the Thumb function-pointer bit for normal `BX LR` task return
  - deterministic cooperative sequence `0x10 -> 0x20 -> 0x11 -> 0x21`
  - first real `schedcoop` PASS, 32/32 stress runs PASS, final post-reset `schedcoop` PASS
  - real task-return path PASS across `34` complete cooperative runs
  - PendSV remains deferred and SysTick does not schedule tasks
  - no preemption yet
  - candidate binary `11792 bytes`
  - SHA-256 `27C5327125BFAC97526F80F83248620152893F7AD92B46F6C56542843F29B885`
  - `_ebss=0x20000748`; SRAM headroom `18616 bytes`.
- Full UART/I2C/OLED/scheduler regression passed after cooperative activation.
- Physical OLED output was confirmed unchanged after the final accepted run.

Next scheduler boundary: implement and independently prove the PendSV context-switch mechanism while keeping SysTick preemption deferred.
<!-- END STM32_OS_ACCEPTED_STATE_2026_09_12 -->

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
- [x] Cooperative scheduler activation: PSP tasks + SVC start/yield/exit, no preemption
- [ ] PendSV context switching
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
