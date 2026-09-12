# Changelog

<!-- BEGIN STM32_OS_CHANGELOG_2026_09_12 -->
## 2026-09-12

### Accepted — OLED runtime lifecycle (Slice 8)

- Frozen 128x32 OLED UI is initialized as part of the normal boot/runtime lifecycle.
- Boot UART contract includes `OLED_RUNTIME_UI_OK`.
- Full UART/OLED regression passed.
- Physical OLED result accepted.
- Accepted binary: `10148 bytes`.
- SHA-256: `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`.
- Acceptance commit: `4217865403d9725707f4c17e572317caf7fe733f`.

### Accepted — scheduler foundation (Slice 9A)

- Added `include/kernel/scheduler.h` and `src/kernel/scheduler.c`.
- Added two static TCBs, two 512-byte static task stacks, and synthetic initial Cortex-M task frames.
- Added deterministic foundation self-test:
  - `schedtest -> SCHED_FOUNDATION_OK`.
- Slice 9A deliberately left PSP activation, context switching, SVC/PendSV scheduler ownership, SysTick scheduling, and preemption deferred.
- Accepted binary: `10752 bytes`.
- SHA-256: `29CA6F248B94A861497D2A97C723B4E945208FC4002C363956753599AE38BFBC`.
- Acceptance commit: `1a57f79cda42674219e774900ce07a0da8fedaf4`.

### Accepted — cooperative scheduler activation (Slice 9B)

- Corrected the synthetic-frame task return boundary before activation:
  - stacked initial PC keeps bit 0 clear and relies on stacked xPSR.T during exception return
  - stacked LR retains the Thumb function-pointer bit required by normal `BX LR` return.
- Activated cooperative task execution on PSP.
- SVC is now the cooperative scheduler exception:
  - SVC `#0`: start first prepared task
  - SVC `#1`: voluntary yield
  - SVC `#2`: task returned / exit current task.
- Added kernel/MSP parking and restoration after all prepared tasks finish.
- Added deterministic real-execution acceptance command:
  - `schedcoop -> SCHED_COOP_OK`
  - expected internal sequence `0x10 -> 0x20 -> 0x11 -> 0x21`.
- Marked scheduler state shared between Thread mode and SVC handling as volatile where required by the compiler model.
- Hardware acceptance:
  - exact flash readback PASS
  - boot/health/UART PASS
  - first real cooperative run PASS
  - 32/32 repeated cooperative runs PASS
  - post-stress health and foundation self-test PASS
  - final post-reset cooperative run PASS
  - real normal task-return path PASS across `34` complete runs
  - full I2C/OLED legacy regression PASS
  - physical frozen OLED output confirmed unchanged.
- Candidate binary: `11792 bytes`.
- SHA-256: `27C5327125BFAC97526F80F83248620152893F7AD92B46F6C56542843F29B885`.
- `.bss`: `1864 bytes`; `_ebss=0x20000748`; SRAM headroom `18616 bytes`.
- PendSV, SysTick-driven scheduling, and preemption remain deliberately deferred.

### Next

- Implement and independently validate the PendSV context-switch mechanism.
- Keep SysTick-driven preemption as a later scheduler gate.
<!-- END STM32_OS_CHANGELOG_2026_09_12 -->

All notable project milestones are recorded here.

This project is currently pre-release; entries are milestone-oriented rather than semantic-version release notes.

## [Unreleased]

<!-- BEGIN STM32_OS_CHANGELOG_2026_09_10 -->
## 2026-09-10

### Added
- Custom Cortex-M3 startup, vector table, .data initialization, and .bss clearing.
- Direct-register 72 MHz clock configuration and SysTick timebase.
- Fault diagnostics for HardFault, MemManage, BusFault, and UsageFault with stacked-context capture.
- Hardware-proven controlled UsageFault diagnostic path and LED panic signaling.
- USART1 polling TX console on PA9 at 115200 8N1.
- UART boot banner and 32-bit hexadecimal output helper.
- Stable kernel time API: kernel_time_now().
- Wraparound-safe deadline and elapsed-time helpers.
- Automated PowerShell hardware loop for build/preflight, ST-LINK flash/verify/reset, UART capture, and acceptance logging.

### Verified
- ST-LINK SWD programming and verification.
- SysTick-driven PC13 heartbeat.
- UART boot output through external USB-UART adapter.
- Time API build and hardware regression image SHA-256: E672398FACBB3BA83E8F05DF4D7165ACFC1849D34BF3589EA81E114ED2DC72E9.
- Dynamic fault_record ELF symbol resolution after its RAM address moved to 0x20000004.

- Time-API hardware regression passed on the 1000-byte image; UART banner matched dynamic `fault_record=0x20000004`, and the PC13 SysTick heartbeat was physically reconfirmed.

- USART1 bidirectional console hardware milestone passed: PA10 RX + PA9 TX at 115200 8N1, automated `ping` -> `PONG`, image SHA-256 `13715B1E206AAD6A8F2EE82E96584623954905C7CAB03611E87514E1D37BE954`, PC13 heartbeat reconfirmed.

- Console `uptime` hardware milestone passed: live `kernel_time_now()` values advanced from `1936 ms` to `2938 ms` (delta `1002 ms`), `ping` regression passed, dynamic `fault_record` matched ELF, image SHA-256 `67C08D182D7CF152BD01F449420C9C5445F0D40F3A49D43B6E1A82BBBFCCC988`.

- Console `health` hardware milestone passed: 10 automated samples showed `3399 ms` SysTick progression, both PC13 output-latch states, and 7 state transitions; image SHA-256 `5BA9FED465FD9F7988B3156CA612EFB88021B1071C6137ABF1D7D1B82042C203`.

- Read-only `fault` console milestone passed: clean fault_record/CFSR/HFSR, SHCSR `0x00070000`, dynamic ELF-matched FAULTREC, and full ping/uptime/health regression; image SHA-256 `185899CE66C3B8683088284939BED267C9E473EFC9317386913595EE15271072`.

- Phase 6 I2C1 hardware scan passed: B6/B7 at 100 kHz, one device ACKed at 7-bit address `0x3C`, with full console/fault/health regression; image SHA-256 `B8F894631076F34EA259E36C0A6744D4FAA6A3B69657F58CEFED72893D25945B`.

- First SSD1306 command transaction passed: `oledping` sent control `0x00` + NOP `0xE3` to address `0x3C`, returned `OLED_CMD_OK`, and the device remained visible to `i2cscan`; image SHA-256 `AA5F092F9C25E6ED07B2F23DEAFF40AF358F07F89B8544EB1BAE2D47605CC9E6`.

- First visible SSD1306 output passed: `oledtest` initialized the 128x64 panel at `0x3C`, wrote a full 1024-byte `0xAA/0x55` checkerboard, returned `OLED_TEST_OK`, remained responsive, and the user physically confirmed the visible pattern; image SHA-256 `46D97925518CD8924278D6A27709E2BEF96AB89E2C49E98E01E88BA40625A5D7`.

### Planned
- USART1 RX and bidirectional kernel command console.
- Native USB Device support on PA11/PA12.
- USB CDC console, binary/RPC transport, host-rendered UI path, and later USB firmware update/bootloader.
- OLED SSD1306 system/status console.
- Scheduler/tasks and PendSV context switching.
<!-- END STM32_OS_CHANGELOG_2026_09_10 -->
