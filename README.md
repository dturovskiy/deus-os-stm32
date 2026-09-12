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
  - `schedcoop -> SCHED_COOP_OK`
  - `schedpreempt -> SCHED_PREEMPT_OK`
  - `schedstack -> SCHED_STACK_WATER_OK`.
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
- Slice 9B cooperative scheduler activation accepted and published:
  - acceptance commit `1114621e9a6bc57d5471cf51a922c216b76bebe2`
  - task Thread mode runs on PSP
  - SVC `#0/#1/#2` provide start / voluntary yield / task-return exit
  - parked kernel/MSP context is restored after all prepared tasks complete
  - deterministic cooperative sequence `0x10 -> 0x20 -> 0x11 -> 0x21`.
- PendSV timer-driven preemption accepted and published:
  - acceptance commit `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`
  - SysTick calls `scheduler_tick()`
  - scheduler tick pends PendSV only while a preemptive scheduler run is active
  - PendSV runs at the lowest system-handler priority
  - PendSV saves/restores `r4-r11` on PSP
  - MSP-origin PendSV is a no-op before any PSP touch
  - final-exit and abort paths clear stale pending PendSV before kernel/MSP restoration
  - preemption acceptance uses two CPU-bound tasks that never call `scheduler_yield()`
  - deterministic sequence `0x30 -> 0x40 -> 0x31 -> 0x41 -> 0x42 -> 0x32`
  - real timer-driven preemption path passed across `34` complete runs.
- Task stack canary/high-water instrumentation is hardware-accepted and awaiting its acceptance commit:
  - existing task stacks remain exactly `2 x 512 bytes`
  - canary/high-water recording occurs on SVC yield, SVC exit, and PendSV switch paths
  - stack scanning/measurement executes from Handler mode/MSP, not from the measured PSP stack
  - `schedstack` runs both cooperative and preemptive scheduler paths
  - all four measured task paths used exactly `72 bytes`
  - observed free margin is `440 bytes` per 512-byte synthetic test stack
  - canaries remained intact across all accepted runs
  - first real `schedstack` PASS
  - 32/32 stack-water stress commands PASS
  - final post-reset `schedstack` PASS
  - total accepted stack-water commands: `34`
  - total underlying scheduler runs: `68`
  - full UART/I2C/OLED/scheduler regression PASS
  - candidate binary `13408 bytes`
  - SHA-256 `4A57F4559AAC3BDAE8FEF5FD3B51F3DEA9033FC917DA19032754796459959D42`
  - `.bss=1936 bytes`; `_ebss=0x20000790`; SRAM headroom `18544 bytes`.
- Physical OLED output was confirmed unchanged after the final accepted run:
  - `DEUS OS`
  - `BOOT OK`
  - `READY`.
- Normal boot task migration remains deferred; console/OLED still run on the existing kernel/MSP path.

The `72-byte` high-water result validates the current synthetic scheduler test tasks only. It is not a blanket production stack-size proof for console/OLED or other substantive workloads.

Next scheduler boundary: run a representative real workload in a command-gated PSP task and measure its canary/high-water behavior before any normal-boot migration.
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
