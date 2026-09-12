# STM32 OS — Project Handoff

<!-- BEGIN STM32_OS_CURRENT_HANDOFF_2026_09_12 -->
## Current authoritative handoff — 2026-09-12

This section supersedes older “current state”, “exact next boundary”, TX-only UART, 128x64 OLED, and scheduler-non-goal text that remains below as historical milestone evidence.

### Repository / current acceptance state

```text
Root:                     D:\Projects\STM32\OS
Branch:                   main
Published HEAD:           44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4
origin/main:               44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4
Current acceptance commit: pending
```

Current hardware-accepted source change set:

```text
 M include/kernel/scheduler.h
 M src/kernel.c
 M src/kernel/scheduler.c
```

Exact hardware-accepted source hashes:

```text
src/kernel.c
C17B0F14BA942DF14D21F078C0C5676A76E18BB06144B284E6D19D9D4465BE1E

include/kernel/scheduler.h
1BB9CF9020AF075FC0286FF121B0225D4507A60F78C91C001937442AC8BB9E72

src/kernel/scheduler.c
6962C998B24679E5FE4F5AEEDD6900FCA0D50F738611F431EBD116EC1FA89B42

src/startup.s
DBAAF4FB8B98B3B114C3F4D8A7ABAEAA3B8B5C6006BDC816D91CA5073B70B90D
```

### Current hardware

- STM32F103C8T6 / Cortex-M3, 64 KiB Flash, 20 KiB SRAM.
- ST-LINK V2, SWD 4000 KHz.
- HW-193 / CH340 on COM3, 115200 8N1.
- UART: A9 -> RXD, TXD -> A10, common GND.
- OLED: native 128x32 SSD1306-compatible, I2C `0x3C`, B6=SCL, B7=SDA.

### Frozen OLED baseline

The accepted OLED styling/geometry remains frozen and authoritative in `docs/OLED_UI_ACCEPTED_BASELINE.md`.

Key current geometry:

- status bar `128x9`, y `0..8`
- blank y `9`
- console clip x `1`, y `10`, w `126`, h `22`
- retained semantic console `21x3`
- font `5x6`, cell `6x7`
- no side/bottom frame below status
- notification field x `20..107`, y `2..6` reserved/not rendered.

### Accepted scheduler/runtime state

Slice 9A foundation is accepted and published at commit `1a57f79cda42674219e774900ce07a0da8fedaf4`.

Slice 9B cooperative activation is accepted and published at commit `1114621e9a6bc57d5471cf51a922c216b76bebe2`.

PendSV preemption is accepted and published at commit `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.

Current stack high-water milestone is hardware-accepted:

- candidate binary `13408 bytes`
- SHA-256 `4A57F4559AAC3BDAE8FEF5FD3B51F3DEA9033FC917DA19032754796459959D42`
- `.bss=1936 bytes`
- `_ebss=0x20000790`
- SRAM headroom `18544 bytes`
- two static TCBs and two 512-byte static task stacks
- normal task execution remains PSP-based
- cooperative SVC start/yield/exit path remains valid
- timer-driven PendSV preemption remains valid
- `schedtest -> SCHED_FOUNDATION_OK`
- `schedcoop -> SCHED_COOP_OK`
- `schedpreempt -> SCHED_PREEMPT_OK`
- `schedstack -> SCHED_STACK_WATER_OK`
- high-water record points: SVC yield, SVC exit, PendSV switch
- stack scanning executes from Handler mode/MSP
- cooperative task 0 high-water: `72 bytes`
- cooperative task 1 high-water: `72 bytes`
- preemptive task 0 high-water: `72 bytes`
- preemptive task 1 high-water: `72 bytes`
- capacity: `512 bytes`
- observed free margin: `440 bytes`
- canary intact across every accepted run
- static synthetic-task worst-case `72 bytes` matched runtime high-water `72 bytes`.

Final hardware acceptance:

- exact candidate program/verify/readback PASS
- exact reset boot frame PASS
- first real stack-water run PASS
- 32/32 stack-water stress commands PASS
- 4/4 return-to-kernel ping checkpoints PASS
- post-stress SysTick health PASS
- foundation/cooperative/preemptive regressions PASS
- full I2C/OLED regression PASS
- frozen runtime UI restore PASS
- final reset + stack-water + cooperative + preemptive + health PASS
- final exact flash identity PASS
- total accepted stack-water commands: `34`
- total underlying scheduler runs: `68`
- physical OLED output confirmed unchanged:
  - `DEUS OS`
  - `BOOT OK`
  - `READY`.

The `72-byte` result is authoritative for the current synthetic scheduler test workloads only. It does not prove an arbitrary console/OLED production task fits in a 512-byte stack.

Normal boot task migration is still deferred. Console/OLED remain on the existing kernel/MSP path.

### Exact next boundary

Run a representative substantive workload in a command-gated PSP task and measure its real canary/high-water behavior. Use that result to choose/justify the first production task stack budget. Do not move normal-boot console/OLED ownership until that workload-specific proof passes.

### Acceptance lifecycle

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART regression
 -> physical visual acceptance when hardware-visible
 -> docs/evidence
 -> acceptance commit
 -> push
```

Documentation sync, acceptance commit, and push remain separate gates.
<!-- END STM32_OS_CURRENT_HANDOFF_2026_09_12 -->

Updated: 2026-09-12

## Project identity

```text
Project root: D:\Projects\STM32\OS
Tools root:   D:\Projects\STM32\Tools
Downloads:    C:\Users\DETU\Downloads\
```

## Hardware

Target board is a Blue Pill-style STM32 board.

Programmer evidence:

- ST-LINK V2
- firmware V2J48S7
- SWD connection works at 4000 kHz
- target voltage observed around 3.15-3.16 V

Target evidence from STM32CubeProgrammer:

- Device ID: 0x410
- Revision ID: Rev X
- Device: STM32F101/F102/F103 Medium-density
- NVM size: 64 KBytes
- CPU: Cortex-M3

Known wiring:

```text
STM32  <-> ST-LINK
GND        GND
DCLK       SWCLK
DIO        SWDIO
3.3V       3.3V
```

Do not simultaneously power the Blue Pill from its micro-USB while 3.3 V power is being supplied from this ST-LINK wiring.

Available future display:

- SSD1306-class OLED
- 128x64
- 4-pin I2C-style module: GND, VCC, SCK/SCL, SDA

Possible future networking hardware:

- ESP-01 / ESP8266

## Host/tooling

Host:

- Windows 10 Home
- AMD64 host architecture
- PowerShell 7

ARM toolchain:

```text
D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi\bin
```

Observed versions:

- arm-none-eabi-gcc 15.3.1
- GNU assembler 2.45.1.20260126
- GNU ld 2.45.1.20260126
- GDB 16.3.90.20250906-git

STM32CubeProgrammer CLI:

```text
D:\Projects\STM32\Tools\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe
```

Observed STM32CubeProgrammer version:

- v2.23.0

Important operational fact:

- fully close STM32CubeProgrammer GUI before using CLI, because the GUI can hold the ST-LINK device and cause `DEV_CONNECT_ERR`

OpenOCD is not installed/required yet. Add it when instruction-level interactive debugging becomes useful.

VS Code is optional; the workflow can be driven entirely from PowerShell plus generated scripts/logs.

## Project layout

```text
D:\Projects\STM32\OS\
├── build\
├── docs\
├── include\
├── linker\
│   └── stm32f103c8.ld
├── scripts\
└── src\
    ├── startup.s
    └── kernel.c
```

## Accepted milestones

### Toolchain
Accepted.

A Cortex-M3/Thumb object was successfully compiled and disassembled.

### Target discovery
Accepted.

ST-LINK connected to the target and STM32CubeProgrammer reported Device ID 0x410, Cortex-M3, 64 KiB NVM.

### Original Flash backup
Accepted.

A full 64 KiB original Flash backup was captured before replacing the previous firmware.

### Custom boot image
Accepted.

The initial custom image had:

```text
Initial MSP  = 0x20005000
Reset vector = 0x08000041
```

Custom Reset_Handler initializes `.data`, clears `.bss`, and enters `kernel_main()`.

### First hardware execution
Accepted.

Custom kernel image was:

- flashed successfully
- verified successfully
- reset successfully
- physically proven by blinking the Blue Pill PC13 LED

The accepted PC13 image was 208 bytes.

## Current source state

Current production source contains:

- custom Reset_Handler and core vector table
- 72 MHz clock setup from 8 MHz HSE using PLL x9
- SysTick at 1 kHz
- global `kernel_ticks`
- WFI idle loop
- NMI handler
- HardFault handler
- MemManage handler
- BusFault handler
- UsageFault handler
- assembly MSP/PSP selection using EXC_RETURN
- C `fault_capture()`
- persistent `fault_record` in SRAM
- capture of stacked r0-r3, r12, lr, pc, xPSR
- capture of ICSR, VTOR, SHCSR, CFSR, HFSR, DFSR, MMFAR, BFAR, AFSR
- deterministic panic LED pattern

Current production image evidence:

```text
Image size:       624 bytes
Initial MSP:      0x20005000
Reset vector:     0x08000041
fault_record:     0x20000000
SHA-256:          F3E4FB67EC8C160BA0EE03EE7A47C6D03694773652498CBB576D8D192BD1BE26
```

Production kernel is currently flashed and PC13 is physically confirmed to blink evenly from the SysTick path.

## Accepted Clock/SysTick hardware milestone

Accepted on 2026-09-10.

Validated build evidence:

- text: 320 bytes
- data: 0 bytes
- bss: 12 bytes
- Initial MSP: 0x20005000
- Reset vector: 0x08000041
- SysTick vector: 0x08000081
- `Reset_Handler`, `SysTick_Handler`, `kernel_main`, and `kernel_ticks` present
- `WFI` present in linked code
- image SHA-256: 578C1A734685DA36E50AE7BC3F9F470769FCC4709AEC69B3CFD64737F42310AA

Hardware acceptance evidence:

- image flashed successfully through STM32CubeProgrammer CLI
- download verified successfully
- MCU software reset completed
- PC13 continued blinking physically after the SysTick image was flashed

This proves the current kernel reaches `kernel_main()`, switches to the intended clock configuration, enables SysTick interrupts, repeatedly executes the SysTick handler, and returns to the `WFI` idle loop between interrupts.

## Accepted fault-diagnostics hardware milestone

Accepted on 2026-09-10.

Production fault-diagnostics image:

```text
Image size:   624 bytes
SHA-256:      F3E4FB67EC8C160BA0EE03EE7A47C6D03694773652498CBB576D8D192BD1BE26
fault_record: 0x20000000
```

Controlled UsageFault test evidence:

```text
fault_record.magic      = 0xFA17FA17
exception_number        = 6
EXC_RETURN              = 0xFFFFFFF9
stacked_sp              = 0x20004FE0
stack_valid             = 1
stacked pc              = 0x0800025C
CFSR                    = 0x00010000
ICSR.VECTACTIVE         = 6
```

The stacked PC matched the intentionally executed `UDF #0` instruction at `0x0800025C`.

Physical acceptance:

- panic LED repeated six short blinks followed by a pause
- fault_record was read successfully over SWD HOTPLUG without reset
- normal 624-byte production image was restored afterward
- production image verified successfully
- PC13 returned to normal even SysTick-driven blinking

## Exact next boundary

Next implementation slice: UART diagnostic console.

First target:

1. use USART1
2. use PA9 as TX
3. configure GPIO/USART registers directly
4. polling transmit only
5. emit a small kernel boot banner
6. add hexadecimal output primitive
7. preserve normal SysTick/PC13 behavior
8. validate source/build before flashing
9. perform physical UART-output proof only when a suitable USB-UART receiver is available

Do not add interrupt-driven UART, DMA, printf, libc, OLED, scheduler, or ESP8266 in this slice.

## Current non-goals

Do not implement yet:

- scheduler
- PendSV context switching
- OLED driver
- ESP8266 networking
- dynamic heap
- filesystem
- HAL
- Arduino
- FreeRTOS

## Script/output convention

ChatGPT-generated user-run artifacts:

```text
/mnt/data/<artifact>
```

User downloads them to:

```text
C:\Users\DETU\Downloads\
```

PowerShell invocation shape:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File "C:\Users\DETU\Downloads\<script>.ps1"
```

Scripts should:

- use PowerShell 7
- use strict/error-stop behavior
- use exact known paths
- fail closed on unexpected source state where relevant
- avoid hidden package installation
- avoid Git mutation unless explicitly requested
- avoid flashing unless the script is explicitly a flash script
- write useful diagnostic logs to Downloads
- distinguish build validation from hardware acceptance

## New-chat startup rule

A new chat should first read:

1. docs/MASTER_EXECUTION_CHECKLIST.md
2. docs/IMPLEMENTATION_PLAN.md
3. docs/PROJECT_HANDOFF.md

Then continue from the exact next boundary above without restarting already accepted setup steps.

<!-- BEGIN STM32_OS_UART_HW_ACCEPTANCE -->
## Current hardware state — UART accepted

As of 2026-09-10, the first diagnostic UART slice is hardware-proven.

- USART1_TX = PA9
- 115200 8N1
- polling TX
- direct-register implementation
- boot banner and 32-bit hexadecimal output helper
- accepted image: D:\Projects\STM32\OS\build\os.bin
- size: 960 bytes
- SHA-256: historical value not preserved in this handoff
- STM32CubeProgrammer download/verify/reset: PASS
- UART capture on COM3: PASS
- observed output:
  - STM32 OS
  - BOOT OK
  - SYSCLK=0x044AA200
  - TICK_HZ=0x000003E8
  - FAULTREC=0x20000000
- evidence: historical local evidence path not preserved in this handoff

The HW-193 adapter is now a proven low-level diagnostic path. Future native micro-USB / USB-device support remains a separate implementation slice; ST-LINK remains the recovery/debug programmer until such a path is implemented.

### Exact next boundary

Close remaining Phase 3 time API / wraparound-safe comparison items before advancing to the next architecture feature.
<!-- END STM32_OS_UART_HW_ACCEPTANCE -->

<!-- BEGIN STM32_OS_DEV_LOOP -->
## Current development loop

The project now has an automated hardware validation loop:

source -> build -> ELF/bin validation -> ST-LINK flash -> verify -> reset -> UART capture -> PASS/FAIL -> evidence log

Current UART is TX-only from STM32 to host. A later RX/command-console slice will extend this to a fully bidirectional automated test loop.

Native USB is planned to eventually consolidate normal console/control/update traffic onto the board's micro-USB connector.
<!-- END STM32_OS_DEV_LOOP -->

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
- Image SHA-256: E672398FACBB3BA83E8F05DF4D7165ACFC1849D34BF3589EA81E114ED2DC72E9
- fault_record address is resolved from ELF rather than treated as a permanent ABI address.
- Evidence: historical local evidence path not preserved in this handoff

Phase 3 governance debt is closed.
<!-- END STM32_OS_PHASE3_TIME_ACCEPTANCE -->

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

<!-- BEGIN STM32_OS_SSD1306_VISIBLE_ACCEPTANCE -->
### First visible SSD1306 output acceptance — 2026-09-10

- Device: SSD1306-compatible 128x64 OLED at 7-bit address `0x3C`
- Board wiring names: `B6=SCL`, `B7=SDA`
- Bus: `I2C1`, `100 kHz`
- Console command: `oledtest`
- Initialization sequence: hardware accepted
- Addressing mode: horizontal
- Display window: columns `0..127`, pages `0..7`
- Display RAM payload: `1024 bytes`
- Test pattern: alternating `0xAA` / `0x55` checkerboard
- Firmware image size: `2444 bytes`
- Firmware SHA-256: `46D97925518CD8924278D6A27709E2BEF96AB89E2C49E98E01E88BA40625A5D7`
- Flash + verify + reset: PASS at 4000 KHz SWD
- Boot banner capture over COM3: PASS
- Dynamic `fault_record` verification against ELF: PASS at `0x2000001C`
- `ping` / `uptime` / `fault` regressions: PASS
- `health` / automated PC13 regression: PASS
- I2C scan before display transaction: PASS, OLED at `0x3C`
- `oledping`: PASS, `OLED_CMD_OK`
- `oledtest`: PASS, `OLED_TEST_OK`
- I2C scan after display transaction: PASS, OLED still at `0x3C`
- Kernel remained responsive after the 1024-byte display write: PASS
- Physical display output: PASS; user entered `YES` after visually confirming checkerboard output
- This proves real visible OLED output, not only I2C ACK/protocol behavior
- Full framebuffer-backed kernel console and text rendering remain pending
- Next implementation boundary: **framebuffer-backed drawing primitives and minimal text rendering**
<!-- END STM32_OS_SSD1306_VISIBLE_ACCEPTANCE -->
