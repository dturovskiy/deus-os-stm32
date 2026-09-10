# STM32 OS — Master Execution Checklist

This file is the canonical execution index for the project.

## Current boundary

Current accepted hardware baseline:

- STM32F101/F102/F103 Medium-density target detected over SWD
- Cortex-M3
- 64 KiB NVM reported by programmer
- 20 KiB SRAM target in linker script
- custom Reset_Handler and vector table
- direct-register GPIO
- 72 MHz system clock
- 1 kHz SysTick
- WFI idle loop
- dedicated Cortex-M fault handlers
- exception-frame and SCB fault capture in SRAM
- controlled UsageFault proven on hardware
- production kernel restored and running
- no HAL, Arduino, or FreeRTOS in the kernel

Current implementation slice:

- Phase 5 polling diagnostic console is hardware accepted
- I2C1 master on B6/B7 at 100 kHz is hardware accepted
- `i2cscan` discovers the connected OLED at 7-bit address `0x3C`
- generic bounded I2C write support is hardware validated
- `oledping` sends SSD1306 command control byte `0x00` plus NOP `0xE3`
- `oledping` -> `OLED_CMD_OK` is hardware accepted
- the device still ACKs at `0x3C` after the command transaction
- full SSD1306 initialization and framebuffer output are not accepted yet
- next target: SSD1306 initialization sequence and first visible display output

Acceptance for the current slice requires:

- explicit USART/GPIO register configuration
- no HAL dependency
- clean Cortex-M3/Thumb build
- UART symbols/configuration validated in linked image
- successful flash + verify
- physical serial output proven on external UART receiver
- normal SysTick/PC13 behavior remains intact

## Ordered execution

### Phase 0 — Toolchain and target
- [x] ARM GNU bare-metal toolchain installed
- [x] STM32CubeProgrammer installed
- [x] ST-LINK V2 firmware updated
- [x] SWD target connection proven
- [x] full original 64 KiB Flash backup captured

### Phase 1 — Boot baseline
- [x] custom linker script
- [x] vector table
- [x] initial MSP = 0x20005000
- [x] Reset_Handler
- [x] .data initialization
- [x] .bss zeroing
- [x] kernel_main entry
- [x] ELF and BIN generation
- [x] first custom image flashed and verified

### Phase 2 — GPIO baseline
- [x] direct memory-mapped RCC access
- [x] direct GPIOC configuration
- [x] PC13 output
- [x] visible LED blink proven on hardware

### Phase 3 — Clock and kernel time
- [x] validate 72 MHz clock configuration in linked image
- [x] validate 1 kHz SysTick vector and ISR
- [x] flash clock/SysTick image
- [x] prove PC13 blink is SysTick-driven
- [x] expose stable kernel_ticks API
- [x] define wraparound-safe time comparisons

### Phase 4 — Fault handling
- [x] dedicated HardFault handler
- [x] capture stacked r0-r3, r12, lr, pc, xPSR
- [x] capture CFSR/HFSR/MMFAR/BFAR
- [x] deterministic panic state
- [x] diagnostic output path through panic LED + SWD fault_record capture

### Phase 5 — UART diagnostic console
- [x] select UART peripheral and pins
- [x] minimal polling TX
- [x] kernel log primitive
- [x] boot banner
- [x] hexadecimal register dump (`fault`: fault_record + CFSR/HFSR/SHCSR)
- [x] RX path + bidirectional command console
- [x] `uptime` command reports live kernel milliseconds
- [x] `health` command reports live tick + PC13 output-latch state
- [x] `fault` read-only fault diagnostics command
- [ ] later: IRQ/DMA TX if justified

### Phase 6 — I2C and SSD1306 kernel console
- [ ] I2C peripheral initialization
- [ ] device-address probe
- [ ] SSD1306 init
- [ ] text framebuffer
- [ ] kernel status screen
- [ ] fault/status diagnostics on OLED

### Phase 7 — Task model
- [ ] define task control block
- [ ] static task allocation
- [ ] separate PSP stacks
- [ ] task initial exception frame
- [ ] cooperative yield
- [ ] task lifecycle rules

### Phase 8 — Scheduler
- [ ] scheduler policy
- [ ] PendSV handler
- [ ] assembly context save/restore
- [ ] SysTick-triggered preemption
- [ ] idle task
- [ ] priorities
- [ ] scheduler invariants
- [ ] context-switch benchmark

### Phase 9 — Kernel services
- [ ] sleep/deadline API
- [ ] software timers
- [ ] message queues
- [ ] synchronization primitive
- [ ] watchdog strategy
- [ ] runtime statistics

### Phase 10 — Networking bridge
- [ ] ESP-01/ESP8266 UART electrical/interface plan
- [ ] framed STM32 <-> ESP protocol
- [ ] transport driver
- [ ] network service boundary
- [ ] remote-control API
- [ ] web control panel
- [ ] keep Wi-Fi policy outside core scheduler/IRQ code

## Global acceptance rules

Every implementation slice must:

- preserve a bootable vector table
- build with Cortex-M3/Thumb flags
- remain freestanding
- avoid hidden HAL/Arduino/FreeRTOS dependencies
- keep hardware-specific register access explicit until intentionally abstracted
- produce an ELF, BIN, map file, and validation log
- distinguish build success from hardware proof
- avoid flashing until build validation succeeds
- keep diagnostic scripts non-destructive unless their purpose explicitly includes flashing
- record any hardware-proven milestone in PROJECT_HANDOFF.md

## Authoritative documents

Read in this order for a new chat:

1. `docs/MASTER_EXECUTION_CHECKLIST.md`
2. `docs/IMPLEMENTATION_PLAN.md`
3. `docs/PROJECT_HANDOFF.md`
4. `docs/ARCHITECTURE.md`
5. `docs/ROADMAP.md`
6. `README.md`

When these documents disagree about current project state, PROJECT_HANDOFF.md owns the dynamic current-state snapshot, while this file owns execution order.

<!-- BEGIN STM32_OS_UART_HW_ACCEPTANCE -->
### UART hardware acceptance — 2026-09-10

- Status: **hardware accepted**
- Peripheral: USART1
- TX pin: PA9
- Format: 115200 8N1
- Transport mode: polling TX
- Image size: 960 bytes
- Image SHA-256: $ExpectedHash
- Flash/verify/reset: PASS via STM32CubeProgrammer
- Host capture: PASS on COM3
- Boot banner:
  - STM32 OS
  - BOOT OK
  - SYSCLK=0x044AA200
  - TICK_HZ=0x000003E8
  - FAULTREC=0x20000000
- Existing fault diagnostics and SysTick/PC13 behavior preserved by the accepted build.
- Evidence log: $EvidencePath

**Next execution boundary:** Close remaining Phase 3 time API / wraparound-safe comparison items before advancing to the next architecture feature.
<!-- END STM32_OS_UART_HW_ACCEPTANCE -->

<!-- BEGIN STM32_OS_PHASE3_TIME_ACCEPTANCE -->
### Phase 3 time API acceptance — 2026-09-10

- Status: **hardware accepted**
- Stable monotonic API: kernel_time_now()
- Deadline comparison: kernel_time_reached(now, deadline)
- Elapsed comparison: kernel_time_elapsed(start, duration)
- Wraparound semantics validated across normal and 32-bit rollover cases.
- Hardware regression: PASS
- UART regression: PASS
- PC13/SysTick behavior preserved
- Accepted image: D:\Projects\STM32\OS\build\os.bin
- Image SHA-256: $ExpectedTimeApiHash
- ault_record address is resolved from ELF rather than treated as a permanent ABI address.
- Evidence: $EvidencePath

Phase 3 governance debt is closed.
<!-- END STM32_OS_PHASE3_TIME_ACCEPTANCE -->

<!-- BEGIN STM32_OS_USB_STRATEGY -->
## Native USB strategy

The Blue Pill micro-USB connector is a planned first-class OS transport, not only a power connector.

Target progression:

1. Minimal STM32F103 USB Device core on PA11/PA12, implemented without HAL.
2. USB CDC ACM diagnostic/command console.
3. Bidirectional kernel shell/RPC transport over USB.
4. Binary transport for structured telemetry, files, bitmap/framebuffer chunks, and host-rendered UI primitives.
5. Host-side client capable of presenting an interactive remote UI / desktop-like view of the STM32 OS.
6. USB firmware-update path and a small recoverable bootloader so normal development can eventually use the native micro-USB cable without the external UART adapter.
7. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary transport.

Constraint: STM32F103C8 is a USB Device target here, not a general USB Host platform. Keyboard/mouse emulation is possible as USB HID device behavior; directly hosting commodity USB peripherals is outside the baseline architecture.
<!-- END STM32_OS_USB_STRATEGY -->

<!-- BEGIN STM32_OS_TIME_API_HW_REGRESSION -->
### Time API hardware regression — 2026-09-10

- Image size: 1000 bytes
- Image SHA-256: E672398FACBB3BA83E8F05DF4D7165ACFC1849D34BF3589EA81E114ED2DC72E9
- Flash + verify + reset: PASS
- UART boot banner: PASS
- FAULTREC=0x20000004 observed and matched the address resolved from ELF
- PC13 SysTick heartbeat: **physically confirmed after this image was flashed**
- Phase 3 time API remains hardware accepted
- Next implementation boundary: **USART1 RX / bidirectional command console**
<!-- END STM32_OS_TIME_API_HW_REGRESSION -->

<!-- BEGIN STM32_OS_UART_RX_ACCEPTANCE -->
### USART1 RX / bidirectional console acceptance — 2026-09-10

- Firmware image size: `1140 bytes`
- Firmware SHA-256: `13715B1E206AAD6A8F2EE82E96584623954905C7CAB03611E87514E1D37BE954`
- USART1 TX: `PA9`
- USART1 RX: `PA10`
- Serial format: `115200 8N1`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Host command: `ping`
- MCU response: `PONG`
- Full host -> PA10 RX -> parser -> PA9 TX -> host path: PASS
- `fault_record` observed at `0x2000001C`; tooling must continue resolving its address dynamically
- PC13 SysTick heartbeat after this image: **physically confirmed even**
- Next implementation boundary: **extend the command set with kernel introspection commands**
<!-- END STM32_OS_UART_RX_ACCEPTANCE -->

<!-- BEGIN STM32_OS_UPTIME_ACCEPTANCE -->
### Console `uptime` acceptance — 2026-09-10

- Firmware image size: `1216 bytes`
- Firmware SHA-256: `67C08D182D7CF152BD01F449420C9C5445F0D40F3A49D43B6E1A82BBBFCCC988`
- Command: `uptime`
- Response format: `UPTIME_MS=0xXXXXXXXX`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Dynamic `fault_record` verification against ELF: PASS at `0x2000001C`
- Existing `ping` -> `PONG` regression: PASS
- First uptime sample: `1936 ms`
- Second uptime sample: `2938 ms`
- Measured delta: `1002 ms`
- SysTick/time API live progression: PASS
- PC13 regression policy: for console-only changes, automated SysTick/UART health evidence is sufficient; physical LED confirmation is only required when GPIOC/PC13, clock, SysTick, startup/fault paths, or power-related behavior changes
- Next implementation boundary: **add further kernel introspection commands**
<!-- END STM32_OS_UPTIME_ACCEPTANCE -->

<!-- BEGIN STM32_OS_HEALTH_ACCEPTANCE -->
### Console `health` acceptance — 2026-09-10

- Firmware image size: `1292 bytes`
- Firmware SHA-256: `5BA9FED465FD9F7988B3156CA612EFB88021B1071C6137ABF1D7D1B82042C203`
- Command: `health`
- Response format: `HEALTH TICK=0xXXXXXXXX PC13=0x00000000|0x00000001`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Dynamic `fault_record` verification against ELF: PASS at `0x2000001C`
- Existing `ping` -> `PONG` regression: PASS
- Existing `uptime` regression: PASS
- Automated health samples: `10`
- Total observed SysTick progression: `3399 ms`
- Observed PC13 output-latch states: `0` and `1`
- Observed PC13 state transitions: `7`
- Automated SysTick/PC13 regression: PASS
- For console-only changes, this automated health evidence replaces routine manual LED confirmation
- Physical LED confirmation remains reserved for changes that affect GPIOC/PC13, clock, SysTick, startup/fault paths, or power-related behavior
- Next implementation boundary: **read-only fault diagnostics console command**
<!-- END STM32_OS_HEALTH_ACCEPTANCE -->

<!-- BEGIN STM32_OS_FAULT_CONSOLE_ACCEPTANCE -->
### Read-only `fault` console acceptance — 2026-09-10

- Firmware image size: `1564 bytes`
- Firmware SHA-256: `185899CE66C3B8683088284939BED267C9E473EFC9317386913595EE15271072`
- Command: `fault`
- Operation: read-only diagnostics
- Dynamic `fault_record` address: `0x2000001C`, matched against ELF
- Clean-boot `fault_record`: all reported capture fields zero
- Clean-boot CFSR: `0x00000000`
- Clean-boot HFSR: `0x00000000`
- SHCSR: `0x00070000`; MemManage, BusFault, and UsageFault handlers enabled
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Existing `ping` regression: PASS
- Existing `uptime` regression: PASS
- Existing `health` regression: PASS
- Automated health samples: `8`
- Total observed SysTick progression: `2598 ms`
- Observed PC13 output-latch states: `0` and `1`
- Observed PC13 state transitions: `5`
- Hexadecimal register dump requirement: satisfied by `fault` via CFSR/HFSR/SHCSR output
- No routine manual LED confirmation required for this console-only slice
- Next implementation boundary: **Phase 6 minimal I2C master bring-up for SSD1306**
<!-- END STM32_OS_FAULT_CONSOLE_ACCEPTANCE -->

<!-- BEGIN STM32_OS_I2C1_SCAN_ACCEPTANCE -->
### Phase 6 I2C1 hardware scan acceptance — 2026-09-10

- Peripheral: `I2C1`
- Board wiring names: `B6=SCL`, `B7=SDA`
- Bus speed: `100 kHz`
- Console command: `i2cscan`
- Scan range: `0x08..0x77`
- Firmware image size: `1972 bytes`
- Firmware SHA-256: `B8F894631076F34EA259E36C0A6744D4FAA6A3B69657F58CEFED72893D25945B`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Dynamic `fault_record` verification against ELF: PASS at `0x2000001C`
- `ping` regression: PASS
- `uptime` regression: PASS
- `fault` regression: PASS
- `health` / automated PC13 regression: PASS
- I2C1 bus transaction: PASS
- Devices discovered: `1`
- SSD1306 candidate 7-bit address: `0x3C`
- Next implementation boundary: **first SSD1306 command transaction to address `0x3C`**
<!-- END STM32_OS_I2C1_SCAN_ACCEPTANCE -->

<!-- BEGIN STM32_OS_SSD1306_NOP_ACCEPTANCE -->
### First SSD1306 command transaction acceptance — 2026-09-10

- Device: connected SSD1306 candidate at 7-bit address `0x3C`
- Board wiring names: `B6=SCL`, `B7=SDA`
- Bus: `I2C1`, `100 kHz`
- Console command: `oledping`
- I2C payload: command control byte `0x00`, then SSD1306 NOP command `0xE3`
- Expected console response: `OLED_CMD_OK`
- Firmware image size: `2252 bytes`
- Firmware SHA-256: `AA5F092F9C25E6ED07B2F23DEAFF40AF358F07F89B8544EB1BAE2D47605CC9E6`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Dynamic `fault_record` verification against ELF: PASS at `0x2000001C`
- `ping` regression: PASS
- `uptime` regression: PASS
- `fault` regression: PASS
- `health` / automated PC13 regression: PASS
- I2C scan before transaction: PASS, device present at `0x3C`
- SSD1306 NOP write: PASS, `oledping` returned `OLED_CMD_OK`
- I2C scan after transaction: PASS, device remained present at `0x3C`
- No visible OLED change was expected because `0xE3` is a no-operation command
- Full SSD1306 initialization/display rendering remains pending
- Next implementation boundary: **SSD1306 initialization sequence and first visible display output**
<!-- END STM32_OS_SSD1306_NOP_ACCEPTANCE -->
