# STM32 OS — Project Handoff

<!-- BEGIN STM32_OS_CURRENT_HANDOFF_2026_09_13 -->
## Current authoritative handoff — 2026-09-15

### Published repository parent

`main`, `origin/main` and remote `main` remain synchronized at:

`33f15f1d23dfa31fabf9f2c83542a5f34046cde8` — `feat: migrate normal boot to production scheduler ownership`

Tree: `a5447420a58b0aed7cd541069979fa665df40aa9`.

C3.7 is accepted in the working tree but is **not yet committed or published**.

### Accepted C3.7 candidate

```text
build\scheduler_timed_blocking_v1\os.bin
22900 bytes
366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A
```

Source:

```text
include/kernel/scheduler.h F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547
src/kernel/scheduler.c     90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618
src/kernel.c               44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC
```

### Accepted architecture

- production task 0 remains cooperative Thread/PSP owner;
- task 1 remains UNUSED;
- host MSP remains WFE idle + exception stack;
- one BLOCKED state plus deadline metadata;
- `scheduler_sleep_ms()` + `scheduler_wait_events_timeout()`;
- SVC 4 timed block;
- `kernel_ticks` remains clock authority;
- event/timeout first atomic transition wins;
- timeout publishes READY then `SEV`;
- production UART wait remains untimed;
- ring is payload; event is notification;
- priorities remain deferred.

### Accepted hardware

```text
timed phases        4/4
sleep elapsed       50 ms
timeout elapsed     50 ms
event elapsed       164 ms
event mask          0x00000001
timed RX delta      26
timed idle delta    2904
safe surface        19/19
diagnostic BUSY     8/8
burst regression    4 x 32 = 128/128 PONG
production stack    580 used / 444 margin
MSP                 340 used / 1644 margin
RX drop/error/depth 0/0/0
final Flash         exact
OLED runtime        PASS
physical OLED       PASS
```

Evidence:

```text
build log      6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84
build evidence 431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199
hardware log   3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769
hardware ev    ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4
```

### Exact next gate

**C3.7 Gate 6 — local acceptance commit.**

Do not rebuild or reflash for Gate 6. Do not start priorities. After the local
commit passes, Gate 7 is a plain non-force fast-forward push.
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

Native USB is planned to eventually consolidate normal console/control/update traffic onto the board's micro-USB connector. The provisional Windows/Linux host application name is **Deus OS CP** (`Deus OS Control Panel`).
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

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_HANDOFF_CURRENT_20260913 -->
## Published scheduler wait/wake checkpoint — 2026-09-13

This is the accepted historical checkpoint that immediately precedes normal-boot ownership migration.

Published commit:

`bfed76e0de1c52bf65e60f66c029cc41710fabd0` — `feat: add scheduler steady-state wait/wake foundation`

Accepted candidate:

- `build\scheduler_wait_wake_foundation_v2\os.bin`
- `19932` bytes
- SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`

Accepted model:

- explicit BLOCKED state;
- SVC task wait;
- ISR-safe event signal/wake;
- pending-event lost-wakeup protection;
- host-MSP WFE idle;
- UART event after ring publication;
- event consumer re-checks FIFO/condition;
- four real UART IRQ wait/wake rounds PASS;
- lifecycle/RX/MSP/OLED regression PASS;
- physical OLED PASS.

This checkpoint is published and closed.

The current next boundary is defined by the top authoritative handoff section and by:
- `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md`
- `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`
<!-- END STM32_OS_SCHED_WAIT_WAKE_HANDOFF_CURRENT_20260913 -->
