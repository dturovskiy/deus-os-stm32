# STM32 OS — Project Handoff

<!-- BEGIN STM32_OS_CURRENT_HANDOFF_2026_09_13 -->
## Current authoritative handoff — 2026-09-17

### Published firmware and architecture baselines

The latest accepted and published firmware/source baseline is:

`39690c9ef103cbcf93272df8bad0359a934b7dc1` — `feat: optimize OLED dirty region updates`

Published tree:

`f195fac5ce733c36a1d955e0fbe687ee6c83b605`

Accepted firmware candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`, BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`, Flash `44560 / 65536`, SRAM `9848 / 20480`, Gate 4 `PHYSICAL_OLED=PASS`. Gates 0–7 are complete and ordinary non-force publication ended clean at ahead/behind `0/0`.

The previously published binary-framed transport baseline remains the underlying transport compatibility baseline:

The docs-only `OS_APPLICATION_AND_UI_MODEL_FOUNDATION` was subsequently published at:

`3dac2c4528fc77e87e1374ff47f56223d2b44e2c` — `docs: freeze application and ui model foundation`

Architecture-foundation tree:

`ff63c349a54a508725a460ce2c0d23d28fe1ec33`

Accepted binary transport candidate:

```text
tested candidate tree c2c3d9743c23ab02329a9652714862fafb5bb17c
binary                40720 bytes
BIN SHA-256            AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022
ELF                    65876 bytes
ELF SHA-256            4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B
Flash used             40720 / 65536
SRAM used              9752 / 20480
```

The binary framed transport foundation is fully accepted and published. USB CDC now carries the retained text shell plus binary RPC v1 with stable public method IDs, request correlation, CRC, bounded argument adaptation, atomic frame TX, malformed-frame recovery, reconnect recovery and deliberate IWDG-reset recovery.

### Latest published architecture foundation

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

Gates 0–7 documentation/architecture acceptance are complete and published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`. This boundary defines Deus OS as an independently operating deterministic embedded runtime, freezes the v1 static application/lifecycle model, defines splash -> desktop/home -> application-view UI ownership and real status-bar semantics, separates firmware responsibilities from future host Control Panel responsibilities, and keeps dynamic native application loading/update/bootloader work deferred to later boundaries.

Foundation gap review found no missing kernel blocker before boot/desktop/application work. Mandatory later contracts are now explicit: semantic application events separate from scheduler wake bits; complete system/build/platform/capability identity for host tooling; bounded recoverable persistence before Flash-resident settings/packages; retained previous-boot crash/reset diagnostics and structured observability; security/trust before network mutation or executable Flash update; controlled reboot/update handoff; and a portability boundary keeping arch/platform/drivers below kernel/services/apps/UI/protocol semantics. Generic timers, queues, synchronization and runtime statistics remain consumer-driven rather than speculative prerequisites.

Published command-service path inherited by this boundary:

```text
UART RX ring ----> independent text parser --------+
                                                     |
USB CDC RX ring -> text parser ---------------------+--> command_service_execute()
                                                     |
CDC binary parser -----------------------------------+
                                                             |
                                                             +--> explicit execution context
                                                             +--> shared method registry/handlers
                                                             +--> originating response adapter
```

Published source facts inherited unchanged at Gate 0:

- `include/kernel/command_service.h` and `src/kernel/command_service.c` define one static allocation-free command service;
- registry count is exactly `32`, with SAFE/DIAGNOSTIC/DESTRUCTIVE classification and max `4` args;
- command execution remains serialized in production task0 / Thread-PSP;
- UART/USB IRQs publish hardware bytes/events only and never execute command policy or reload IWDG;
- UART boot/fatal/recovery output remains an explicit emergency path;
- USB CDC RX/TX rings remain `1024` / `2048` bytes;
- at the Gate 0 baseline, the CDC TX API was byte-at-a-time and could partially enqueue a larger logical message before ring-full failure; Gate 1 therefore added one nonblocking all-or-none bounded span enqueue;
- USB descriptors, PMA map, endpoint lifecycle, scheduler, startup, linker, IWDG and frozen OLED/gfx/status-bar are otherwise guarded.

Normative binary v1 contract:

- magic `A5 5A` with CDC text/binary coexistence;
- version `1`, explicit frame type/flags/request ID/payload length;
- little-endian multi-byte fields;
- CRC-16/CCITT-FALSE (`poly 0x1021`, init `0xFFFF`);
- stable public RPC IDs `0x0001..0x0020`, separate from internal C enum ordinals;
- request ID `0` reserved; host requests use `1..65535`;
- max `4` args, max `31` bytes per arg, max request payload `132` bytes;
- response chunks max `48` command-output bytes, yielding a maximum `RPC_DATA` frame of exactly `64` wire bytes;
- explicit `ALLOW_DESTRUCTIVE` flag for destructive RPC;
- bad CRC executes no command;
- no heap/new task/SVC/IPC/timer subsystem/vendor-specific USB interface/firmware update/bootloader in this boundary.

Accepted binary transport candidate and evidence:

```text
tested candidate tree c2c3d9743c23ab02329a9652714862fafb5bb17c
binary                40720 bytes
BIN SHA-256            AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022
ELF                    65876 bytes
ELF SHA-256            4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B
MAP SHA-256            6A71B9E6F8FAB93870C95B967448FEEE6D1656DF1098A63DB90D122125609FD6
Flash used             40720 / 65536
SRAM used              9752 / 20480
Gate 2 evidence        D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A
Gate 3 log             F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B
Gate 3 evidence        8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565
Gate 4                 PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS
```

Gate 3 proved HELLO/request-ID correlation, all `21/21` binary safe methods, `8/8` live scheduler diagnostics as BUSY, malformed/CRC/oversize recovery, exact `help` chunk sequencing/accounting, partial-text preservation, `128/128` unique-ID binary pressure, retained CDC/UART pressure, physical reconnect, authorized binary watchdog trip, automatic post-IWDG UART/CDC text+binary recovery, and final exact Flash identity.

Gate 5 finalized documentation/evidence only; no source/build/flash/commit/push was performed by Gate 5.

Published shell/RPC evidence retained:

```text
Gate 2 evidence       34FB1B6D368A29AF5460174BBDA6AE81E479E5D11FBC0E25FDAC01617105BEA3
Gate 3 log            0DCECB16F505C60A07AC73790DFAE4D7BFDFAEE56B6154DC503A5AF17B989E70
Gate 3 evidence       0EE7342129866C86A1AAFBA42E942E2097C78BFDF3CF50D0C93F3A6B71E9A4E9
Gate 4                PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS
```

Published shell/RPC design/acceptance:
`docs/SHELL_RPC_FOUNDATION_PLAN.md`
`docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`

Published binary protocol:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

Published binary design:
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`

Published binary acceptance:
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`

Current product/application/UI model:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`

Current model acceptance:
`docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`

Current foundation completeness review:
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

Power rule remains: micro-USB is the normal target power source during USB runtime; ST-LINK 3.3 V and UART adapter VCC remain disconnected. UART TX/RX, SWDIO/SWCLK and grounds remain connected.

### Previous published UI boundary

**`BOOT_DESKTOP_UI_FOUNDATION` — Gates 0–7 accepted and published at `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`. Canonical design: `docs/BOOT_DESKTOP_UI_PLAN.md`; acceptance: `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.**

The published boundary delivers `BOOT_SPLASH -> DESKTOP_HOME`: a 1000 ms minimum visible splash dwell that does not delay scheduler/IWDG startup, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK indicators, monotonic uptime `HH:MM`, semantic snapshot suppression, and recovery-only SSD1306 initialization.

Accepted candidate/evidence:

```text
tested candidate tree  41e0c7cd345dd64d3b5336abf2fc46d446f19ecb
BIN                    41520 bytes
BIN SHA-256             A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42
ELF                    70700 bytes
ELF SHA-256             62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02
MAP SHA-256             D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8
Flash used              41520 / 65536
SRAM used               9792 / 20480
Gate 2 evidence         62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729
Gate 3 log              9481BEFADB5A8F1D17AF6FC0ACDE238FE66B6956940F56DC86A828CBFAA7F900
Gate 3 evidence         2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD
Gate 4                  PHYSICAL_OLED=PASS
```

Gate 3 passed CDC `128/128` zero drops, UART `128/128` zero drops/errors, binary `128/128` unique request IDs, bad-CRC/oversize/split-frame recovery, physical micro-USB reconnect, authorized IWDG reboot with automatic transport recovery, task margins `640` / `424` bytes, three USB enumerations and exact final Flash readback. Physical OLED review confirmed clean digit/text transitions with no unintended content, flicker, blank/off pulse or stale pixels.

Publication commit/tree is `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8` / `d27cf8246fb7563b2327955ffc06428b9d843b2a`; final `HEAD == origin/main == FETCH_HEAD` and ahead/behind `0/0`.

### Latest published firmware boundary

**`OLED_DIRTY_REGION_OPTIMIZATION` — Gates 0–7 accepted and published at `39690c9ef103cbcf93272df8bad0359a934b7dc1`, tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`. Canonical design: `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`; acceptance: `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`; deferred engineering policy: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.**

Accepted candidate/evidence:

```text
tested candidate tree  75f05f689970b760604112b30346b0c328bfaff2
BIN                    44560 bytes
BIN SHA-256             93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677
ELF                    71308 bytes
ELF SHA-256             E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C
MAP SHA-256             00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09
Flash used              44560 / 65536
SRAM used               9848 / 20480
Gate 2 evidence         FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC
Gate 3 log              887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6
Gate 3 evidence         76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214
Gate 4 evidence         1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6
Gate 4                  PHYSICAL_OLED=PASS
```

The implementation keeps one 512-byte framebuffer and adds exact per-page X dirty spans. Hardware measured the prior full semantic refresh at `572` payload bytes / `36` writes, clean present at `0`, one-byte update at `9`, minute `00:00 -> 00:01` at `11` over `x=123..125`, and USB indicator transition at `11` over `x=9..11`. Post-`oledstatus` and post-`oleddirty` task margins are task0 `328` bytes / task1 `424` bytes. Final Flash readback is exact; CDC/UART/binary pressure, malformed-frame recovery, physical reconnect and IWDG recovery all pass.

Gate 5 added the canonical deferred optimization/robustness/observability/storage/test-profile backlog and `.gitattributes` LF policy without changing the accepted firmware candidate. Gate 6 committed the exact reviewed source/docs set as `39690c9ef103cbcf93272df8bad0359a934b7dc1`; Gate 7 published it by ordinary non-force push. Gate 6 evidence is `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence is `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

### Current implementation boundary

**`USB_MANAGEMENT_DEVICE_FOUNDATION` — Gates 0–7 accepted / published `1f88083843c6aae9fd228ad2d677f9252b889a11` (tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`). Current boundary is `HOST_CONTROL_APPLICATION_FOUNDATION` Gate 0 accepted / Gate 1 source implementation next.**

Accepted runtime candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c` implements static `system.home=0x0001` and `device.info=0x0002`, one foreground application, explicit lifecycle states, task0-only callback/event dispatch, fixed pointer-free semantic events, a bounded system-state snapshot, and a three-row application view beneath the system-owned status bar. Existing RPC IDs `0x0001..0x0020` remain unchanged; `applist/appstart/appstop` append `0x0021..0x0023`. BIN is `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; SRAM is `10032/20480`; minimum observed task0/task1 margins are `272/424` bytes. Gate 3 and physical OLED Gate 4 are PASS. Evidence SHA-256: Gate 2 `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`; Gate 3 `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; Gate 3 log `703C41A7493D078E456A5721EAFEFF821A152D217FB3F30AB8C809D7452A6B36`; Gate 4 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`.

Initial Gate 1 source boundary is exactly new `include/kernel/application_runtime.h`, new `src/kernel/application_runtime.c`, plus `include/kernel/command_service.h`, `src/kernel/command_service.c`, and `src/kernel.c`.

Application Runtime publication commit/tree: `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`; ordinary non-force push complete; final ahead/behind `0/0`. Gate 5 evidence `817D12D74D88F1C0F31C502B0715F038FF356382E56FC0D5421DC237239D78BC`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`; Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`. Kernel decomposition publication is complete at `fa75307fb392718a1d10d52770a6a111c97208e7`. Current USB management design: `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md`; acceptance: `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.

Current decomposition acceptance: `src/kernel.c 5100 -> 3405` lines (-33.235%). `application_runtime_bridge` owns mutable application runtime integration state/event snapshot/view adaptation; `application_commands` owns `rpcinfo/applist/appstart/appstop`; `scheduler_diagnostics` owns diagnostic orchestration; root-private production scheduler observability remains in the composition root. Exact source boundary is seven paths and no command-service expansion was needed.

Accepted candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` / `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536`, SRAM `10032/20480`, final task margins `384/424`. Gate 3 retained text/binary lifecycle, diagnostic BUSY `8/8`, CDC/UART/binary pressure `128/128`, malformed/CRC/oversize/split recovery, physical reconnect, IWDG recovery, zero faults and exact final Flash readback.

Evidence SHA-256: Gate 2 `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; Gate 2 authoritative build log `88A0AEA8569D404FC8F498124F042F2142EA43440CF9D9CF4096A3C3D12585E0`; Gate 2 evidence-repair log `904C5C476D3A4ED29CAB80532B30FE5B1DD757DF770CC8B1ACEA6AEC1C18A25F`; Gate 3 `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; Gate 3 log `4F3714F2D8772F032D3D15CFA5C3A61E69B53D69C60E8D0E77101297153D5856`; Gate 4 `PHYSICAL_OLED=PASS`. Gate 6/7 publication is complete at `fa75307fb392718a1d10d52770a6a111c97208e7`; final remote state was clean `0/0`.

Post-decomposition architecture debt is explicitly deferred in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`: further `kernel.c` composition-root convergence; system/service state becoming upstream of UI rather than reconstructed from presentation state; splitting OLED adaptation from `application_runtime_bridge` only when a second presentation consumer exists; preserving `scheduler_diagnostics` as a synchronous/non-reentrant borrowed-binding contract unless a dedicated concurrent model is accepted; and freezing fail-closed stop semantics before applications can own resources whose release may fail. None of these items blocks the current USB-management boundary or authorizes speculative HAL/framework/god-context work.

Published USB management Gates 0–7 are **PASS** on repaired descriptor candidate tree `46841b52d351277deb134a6f4709619087b477af`: BIN `50172` / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`, Flash/SRAM `50172/11728` within frozen ceilings `54780/11824`, task stacks `1024/512`, undefined symbols `0`, stack-usage files `21/21`. Gate 2 evidence/log SHA-256 are `B6101C2FAD615DC41856BD1DE88C46E92517759EC3AC1F56ACB2030B89252AF3` / `245F15303355C47473C15CB05838DB495764F97D228DB1113B3C7693765D429F`. Gate 3 composite hardware acceptance (`v11+v14`) proves automatic inbox WinUSB binding on management interface 2, exact protocol/service/registry surface, application lifecycle and rejection/recovery behavior, real physical USB reconnect, authorized IWDG reboot/recovery with bounded return to `system.home`, WinUSB pressure `128/128`, CDC parser/partial-frame isolation, CDC binary continuity, UART `32/32`, management RX/TX drops `0/0`, task margins `448/424`, intact canaries/zero scheduler faults, and final Flash exactly equal to the Gate-2 BIN. Gate 3 evidence/log SHA-256 are `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6` / `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`; no Flash was performed in the final continuation. The exact five-path Gate 1 implementation boundary plus two-path binary-RPC SRAM remediation remains unchanged. Composite identity is `1209:000C` / `Deus OS Device`, CDC interfaces 0–1 under IAD, management interface 2 `FF/00/00` over EP4 `0x04/0x84` bulk64, PMA `0x180/0x1C0`, MS OS 2.0 vendor code `0x20` / set length `178` / first-configuration subset selector `0`, `WINUSB` only for IF2, stable GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`; OLED/UI/application-rendering source did not change. Gate 5 documentation finalization is PASS; Gate 6 committed the exact accepted source/docs set as `1f88083843c6aae9fd228ad2d677f9252b889a11`, tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`; Gate 7 published it by ordinary non-force push and fresh fetch proved `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`. `HOST_CONTROL_APPLICATION_FOUNDATION` Gate 0 is accepted. Canonical design/acceptance: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` and `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`. Gate 1 authorizes only the frozen five-path target `sysinfo` addition plus the exact `host/` solution tree; no other firmware source mutation is authorized.

<!-- END STM32_OS_CURRENT_HANDOFF_2026_09_13 -->
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

Current accepted display:

- SSD1306-class OLED
- native 128x32
- I2C address `0x3C`
- 4-pin I2C module: GND, VCC, SCK/SCL, SDA

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
- runtime `fault_record` in SRAM `.bss` (cleared on reset; not a retained previous-boot crash record)
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
