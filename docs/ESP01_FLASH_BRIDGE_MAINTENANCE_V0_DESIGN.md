# ESP-01 over STM32 USB Management — maintenance bridge v0

Status: **SOURCE-ONLY USB/USART2 INTEGRATION NATIVE ARM BUDGET PASS / HOST MOCK USB PASS / NOT INSTALLED / NO DEVICE AUTHORIZATION**
Scope owner: optional ESP-01 node BENCH development; `CURRENT_STATE.md` remains sole current-status authority.

**2026-11-08 — FINAL SOURCE ISOLATION CORRECTION (supersedes earlier paths in dated §0d):** Experimental C implementations `esp_gpio_board_v0.c`, `esp_boot_control_v0.c` and `esp_power_monitor_v0.c` are physically located in `nodes/esp8266/bridge/board_lab/`; their public headers remain under `include/`. Their old `src/drivers/` / `src/kernel/` references in dated notes are HISTORICAL and no longer valid. Build script's strict source inventory remains **36/36 MATCH, none extra/missing** under read-only test task `task_1794097030919_6da9277b560637fc`. Host-only tests on moved original sources: GPIO O0/Os/UBSan 3/3 PASS task `task_1794097009021_7c2da56a66daa2b4`; boot-state/power O0/Os/UBSan 3/3 PASS task `task_1794097005790_d5f61ac8ee9e5d60`. None is linked into source-integrated unsigned ARM bridge; no signed image or device changed. Missing Mac mini SLIP over USB serial proxy, GPIO/BOOT control binding and STM32 ARM size qualification, owner-approved physical handover and read-only ROM SYNC. OTA, network and application Wi-Fi are a separate later capability; INA226 deferred.



## 0. Integration checkpoint — 2026-11-08 (source-only, NOT deployed)

The previously isolated C core has now been copied into product build paths:
`src/kernel/esp_bridge_core_v0.c` and `include/kernel/esp_bridge_core_v0.h`.
The existing `src/kernel/usb_management.c` source dispatches candidate
`BINARY_FRAME_TYPE_ESP_BRIDGE_REQUEST=0x05` to the core and serializes a
`BINARY_FRAME_TYPE_ESP_BRIDGE_RESPONSE=0x87` using the current Management
workspace and CRC16. The existing `src/kernel/node_transport.c` has explicit
task0-only `node_transport_maintenance_enter/leave`; when maintenance is
active, Node Link must not consume USART2 RX or transmit HELLO/PING/retries.
`scripts/build_firmware.ps1` includes the new C source and preserves
all accepted `-Os` optimizations of scheduler diagnostics and four OLED/UI
modules.

**Hardware admission is deliberately unconditional DENIED:** the new
`usb_bridge_can_enter()` returns 0; no GPIO0/EN/RST lines are programmed,
driven, assigned or connected through this candidate, and neither USB nor
ESP ROM programming is enabled on installed normal v8. The source includes
bounded USART2 TX/RX callback code, but it is unreachable through USB OPEN
until a separate authorized hardware implementation replaces that
fail-closed admission condition. Original STM32 firmware update/bootloader,
USB RPC and Asset Transfer dispatch are preserved; a concurrent STM32
firmware-enter request is refused if maintenance ever owns USART2.

Actual DEUS MCP managed source tests:
`task_1794092877018_03008a027a45f650` (production
`src/kernel/esp_bridge_core_v0.c` mock tests -O0/-Os/UBSan 3/3 PASS),
`task_1794092880028_03facea264619067` (four host GCC
`-fsyntax-only -Werror` firmware modules 4/4 PASS, real
`node_transport.c` with UART mock -O0/-Os/UBSan 3/3 PASS, hard-deny
dispatcher source guard PASS). No physical ports opened.

**One canonical source-SHA-pinned OFFLINE ARM measurement candidate** is now
available outside Git at
`Bundle/stm32_os_esp01_flash_bridge_integration_arm_v1_20261108.zip`
(**whole ZIP SHA256
`A1ABD775B450C4C501396901248CE8215F6C276C465178E4788D4CA9414175DF`**).
Its original 75-file source-index SHA is `8a9d166f2907307ee495ac66f4bb483f9dba8aa1`.
Only five previously accepted source files changed, two source files
added, **68/68 other original accepted Windows candidate files were
preserved exactly**. Bundle Guard canonical build + independent validate
PASS **STATIC_ONLY**, independent byte/source scope auditor
`task_1794093222638_38ca5aa983381fb9` PASS (81/81 ZIP CRC,
75/75 indexed source SHA+size). Windows native Cortex-M3 ARM
**NOT_RUN**; no valid new Flash/SRAM consumption can yet be asserted.
A successful build will check 36 C stack-usage files, Flash <=53,248,
SRAM <=11,264, original Git pre/post and original indexed Evidence.
A FAIL must be assessed from its one original Evidence, not repeated
automatically. No signed update, owner hardware GO, GPIO0 wiring or
ROM SYNC is implied.

## 0a. Native resource and USB dispatcher acceptance — 2026-11-08 00:18 Europe/Paris

The **exact previous canonical source-only integration ZIP**, SHA256 `A1ABD775B450C4C501396901248CE8215F6C276C465178E4788D4CA9414175DF`, was run once by the Windows owner with `deus-run v1.3.2-source-candidate`. Its original immutable Windows Evidence is `Evidence/stm32_os_esp01_flash_bridge_integration_arm_v1_20261108_20261108_001811_ebb8f9e2f64946558e863a72dd98abfb.evidence.zip`, 6165 B SHA `578D40B4DDC06A407D6318BEA5B5D8FC76E149634949A8D6ED9E7DCED88707FE`. Independent DEUS MCP original Evidence reader `Tools/scripts/bench/audit_esp_bridge_integration_original_windows_20261108.py` task `task_1794093559818_1ed4cc4c48cff3b4` exit0: full ZIP SHA+CRC 7/7, indexed member SHA5/5 and sizes5/5, original ARM exit0 and result PASS, original source helper SHA locked, Windows Git pre/post equal, 36 stack-usage files. Firmware candidate is **52,372/53,248 Flash (876 B spare)**, **8,480/11,264 static SRAM (2,784 B spare)**, +904 Flash/+12 SRAM over prior UI-optimized baseline. Original `os.bin` SHA `782677204EE527B0537C2E2EA904CA7D42BE1A9919F06632353C82A55D511093` is UNSIGNED and NOT INSTALLED.

A further no-device **behavioral mock of the real firmware dispatcher** was implemented at `nodes/esp8266/bridge/test_usb_bridge_dispatch_offline.c`, launched by `run_usb_bridge_dispatch_offline.py`: DEUS MCP task `task_1794093831230_e2ab8667880abdb7` exit0, gcc O0/Os/UBSan **3/3 PASS**. It compiles product `usb_management.c`, `binary_frame.c`, `esp_bridge_core_v0.c` and replaces **only four ARM-specific reset `__asm` instructions in a temporary x86 test copy**; the live production ARM source stays byte-identical to the native accepted ZIP. In-memory USB OUT and IN tests verify DENIED response on valid `0x05 OPEN` with correct CRC/request ID, no UART TX/RX/arm, unopened TX/RX/CLOSE rejected, malformed length/CRC/flags/reserved/request ID, USB IN failure, bus reset, deconfiguration, existing Asset Transfer, RPC and intentionally INVALID STM32 firmware-update request (no reset). The first new test run was invalid because its mock ingress failed to increment `inbound_len`; corrected **only in test**, reran clean 3/3 PASS. This tests **current deliberately disabled maintenance admission**; it does NOT prove enabled ESP ROM programming, UART timing under load, GPIO/EN/BOOT wiring or safe CH340 ownership.

**Next scope:** no need for another unchanged native resource test. Review exact ESP adapter 3.3 V power ownership, pull-ups, GPIO0/EN/RST safe levels and STM32 available control pins (Blue Pill silkscreen notation), preserving NodeLink/USB/watchdog and maintaining explicit owner GO for any physical wiring or reset. Only then implement and separately prove a bounded ROM maintenance mode; do not automatically write Flash. Gate4 remains NOT_AUTHORIZED.

## 0b. Existing wiring and owner instructions — correction 2026-11-08

**Do not request new photographs, cable mapping, 3.3V measurements or GPIO0/EN inspection.** The owner already documented and proved the **real** STM32 `A2 (PA2, USART2_TX)` ↔ ESP RXD and `A3 (PA3, USART2_RX)` ↔ ESP TXD Node Link at `node start c9e459c9` / `node crc DEUS-V8-ESP` (READY=1, CRC32=0xADF1C023, 20s health PASS). ESP VCC/EN/BOOT are still managed by the existing CH340 adapter; CH340 UART TX/RX are detached. This split is confirmed by `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` and its explicit **2026-11-04 owner boundary prohibiting new power/GPIO0/USB reconnection diagnostics**.

**The previous sections' assumption that future STM32 GPIO0/EN/RST ownership and new wiring are necessarily required is only one architectural possibility, not an established requirement.** An alternative **software design hypothesis** is: keep the *existing* CH340 adapter as the sole ESP BOOT/EN controller, use STM32 USART2 as the sole ESP UART data controller, and coordinate their state through an explicit Mac mini host↔STM32 Management handshake. Whether adapter modem controls can safely put ESP in ROM boot with UART detached is **not yet qualified** and must not be inferred from successful normal-app HELLO/CRC. A historical owner-authorized direct CH340 ROM `chip_id` and app-only `write_flash --verify` were real with CH340 data connected *at that earlier time*; their authorization and old wiring do not transfer to current crosslink.

The next **software-only** tasks are: specify a fail-closed `adapter-control-owned` / `STM32-maintenance-owned` handover, preserve no-ESP-TX contention, model interrupted USB sessions/partial UART writes/timeouts with mock tests, and keep `usb_bridge_can_enter()` at literal DENY until a separately authorized control path and an accepted device firmware version exist. No physical test or owner's new wiring answer is a prerequisite to continuing code and model work. No CH340 DTR/RTS/EN or STM32 reset/Flash without a fresh, specific owner's GO.

## 0c. OWNER-PROPOSED SINGLE-STM32 ESP OWNERSHIP TOPOLOGY — 2026-11-08 (NOT WIRED, NOT AUTHORIZED)

The owner proposed retiring Mac mini CH340 entirely: preserve already verified STM32 **A2 (PA2, USART2_TX) → ESP RXD**, **A3 (PA3, USART2_RX) ← ESP TXD**, common GND, and move ESP **BOOT (GPIO0)** and **EN/CH_PD** controls plus ESP 3.3 V supply to STM32-managed infrastructure. This is an **engineering proposal, not a reported wiring change or physical GO**. The former split-host CH340-controlled alternative remains documented historically but is not assumed mandatory.

**Provisional control pin reservation, source-checked only** (not confirmed physical availability, no wires connected):

| Blue Pill board marking (MCU) | Proposed ESP-01 terminal | Owner/control |
|---|---|---|
| A2 (PA2, USART2_TX) | RXD | existing, physically tested 2026-11-07 |
| A3 (PA3, USART2_RX) | TXD | existing, physically tested 2026-11-07 |
| B0 (PB0) | BOOT = ESP GPIO0 | NEW proposed GPIO open-drain low/release-high, external 3.3 V pull-up |
| B1 (PB1) | ESP EN = CHIP_PU/CH_PD | NEW proposed GPIO open-drain low/release-high, external 3.3 V pull-up, power-on delay |
| GND | ESP GND | existing common ground |
| regulated 3.3 V supply *with proved capacity* | ESP VCC | NEW source path still unqualified; NOT an MCU GPIO or a second regulator |

Search of current `OS/src` finds no PB0/PB1 assignments, but this is not an electrical / wiring attestation. **B0 (PB0) is not the STM32 BOOT0 jumper.** OLED B6 (PB6)/B7 (PB7), USB A11/A12, SWD SWDIO(PA13)/SWCLK(PA14), Windows diagnostic UART A9/A10 remain unchanged.

**Power prerequisite:** available Blue Pill `3.3V` header pins generally share **the same regulator rail**, including the display already on one `3.3V` pin; a spare header does NOT provide additional regulator current. Espressif recommends a regulated 3.3 V source able to supply **>=500 mA** for ESP8266 alone (ESP-Techpedia startup conditions / Espressif Hardware Design Guidelines v2.8), decoupled near ESP (at least local 10 µF and 0.1 µF). The real Blue Pill regulator part, its actual USB input margin and its budget after STM32+OLED are **not established** in our evidence; therefore directly attaching ESP VCC to the remaining `3.3V` pin is **NOT APPROVED BY THIS DESIGN**. Preferred robust design: **one existing Mac mini↔STM32 USB cable**, with a dedicated suitably rated 3.3 V regulator for ESP powered from the same proven 5 V source (not a second USB-UART), and common GND; avoid two powered sources/backfeed and ensure dedicated LDO thermal headroom. An electrically documented adequate Blue Pill regulator could substitute only after owner-selected qualification; do not assume.

**Control electrical default:** either (a) verified 3.3 V-tolerant STM32 GPIO configured open-drain, releasing to separate pull-ups (e.g. 10kΩ), or (b) external low-side open-drain MOSFET/transistor controls and the same pull-ups. The code must put GPIO0 HIGH (released) and EN HIGH (enabled) by default even while STM32 bootloader resets/pins are high-Z. ESP EN MUST never float. Open-drain only pulls pins LOW, never pushes a HIGH level from a different supply rail. Actual adapter/harness must be mechanically removed from these lines, and **no CH340 DTR/RTS simultaneous drivers**. Avoid tying ESP GPIO2 directly to VCC; ESP boot strap rules remain binding.

**ROM-only transition design (not deployed):**
1. Host requests a maintenance transaction; STM32 accepts only if no pending Node Link job/STM32 firmware reset and explicit GPIO ownership is qualified. Quiesce Node Link, block PING/HELLO/retries and reserve USART2 RX.
2. Pull EN LOW; pull GPIO0 LOW; after bounded EN-low guard, release EN HIGH after stable 3.3 V. Keep GPIO0 LOW through boot strap sampling and ROM SYNC start; no host firmware bytes until ROM download identity/SLIP handshake positive.
3. Host sends ROM SLIP packets across STM32 management 0x05/0x87 in 40 B UART chunks; transport ACK != ROM write verification. A USB failure, timeout, reset or UART overflow goes FAULTED, never retries uncertain partial UART TX.
4. To return to normal application: ensure no flash write in progress, set GPIO0 released HIGH, drive EN LOW then release HIGH with bounded sequencing; require fresh Node Link nonce+HELLO and health. Failed restoration is failure, not implicit READY.
5. Test only OFFLINE until owner separately authorizes physical wiring, safe power migration, signed STM32 firmware install and first **read-only** ESP ROM SYNC/ID. Actual ESP Flash write requires another explicit authorization.

**Current confirmed firmware budget:** native Windows source-integrated bridge Flash **52,372/53,248 B**, spare **876 B**; SRAM **8,480/11,264 B**, spare 2784 B; 36 stack files, original Windows Evidence SHA `578D40B4DDC06A407D6318BEA5B5D8FC76E149634949A8D6ED9E7DCED88707FE`. GPIO control code **not yet implemented**, so 876B is not validated enough for the added boot control sequencer. Signed installed normal v8 does NOT include the bridge/controls. Any future signed normal firmware requires separate version/recovery-floor qualification, not overwrite of signed v8.

References for forward engineering (external, not proof of the board's regulator): Espressif esptool ESP8266 Boot Mode Selection `https://docs.espressif.com/projects/esptool/en/latest/esp8266/advanced-topics/boot-mode-selection.html` and Espressif ESP-Techpedia ESP8266 startup conditions `https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/get-started/try-firmware/try-firmware-hardware/esp8266.html`.

## 0d. Source-only BOOT/EN and honest USB-visible current telemetry — 2026-11-08

As separately requested by the owner, **actual new but unlinked** `src/kernel/esp_boot_control_v0.c` + `include/kernel/esp_boot_control_v0.h` model safe B0 (PB0) GPIO0 and B1 (PB1) EN open-drain transitions; `src/kernel/esp_power_monitor_v0.c` + `include/kernel/esp_power_monitor_v0.h` model INA226 16-bit current/voltage readings from ESP-only 3.3V rail. Neither module is called or linked by signed v8 or the previous accepted 52,372B ARM source snapshot; no GPIO/I2C register writes occur. GPIO control initial state DISABLED, local rail+ownership+Node checks required, EN20ms LOW guard +100ms boot settling, sticky FAULT, ROM_AWAIT_SYNC requires external verified ROM reply, normal requires fresh HELLO. INA226 raw input needs explicit fitted shunt value, sensor at candidate I2C1 0x40 (OLED0x3C), missing sensor reported NOT_AVAILABLE, stale and bus faults distinguished. Physical sensor and separate verified 3.3V ESP supply are NOT installed. **The USB peripheral does not measure current**; it can report measurements only from a new wired sensor. Scope/test report: `docs/ESP01_POWER_TELEMETRY_V0_DESIGN.md`; mock source GCC -O0/-Os/UBSan 3/3 PASS `task_1794095521035_9fc8d25050c0481c`. Still missing: I2C1 register read, task-level bus arbitration, a SAFE `powerstat` CommandService/USB path, GPIO low-level adapter, and new native ARM size proof (876B headroom). No physical work or owner GO has occurred.

**2026-11-08 — STATUS AFTER OWNER DIRECT FLASH READINESS QUESTION (LATEST):** The board-control mock driver `nodes/esp8266/bridge/board_lab/esp_gpio_board_v0.c` and header `include/drivers/esp_gpio_board_v0.h` now exist for prospective **B0 (PB0) → ESP GPIO0/BOOT** and **B1 (PB1) → ESP EN/CH_PD**. It sets PB0/PB1 STM32F103 2MHz **open-drain**, releases ODR latches HIGH before GPIO output enable, modifies only CRL PB0/PB1 nibbles, and requires explicit locally qualified initialization. Actual source was tested with in-memory MMIO mocks under -O0/-Os/UBSan 3/3 PASS, original DEUS MCP `task_1794096910986_344b4424e57bdc36`; no device touched. **Important build rule:** `scripts/build_firmware.ps1` fails on any C source in `src` not in its approved source/object list, so this new file was deliberately moved into `nodes/esp8266/bridge/board_lab` and **NOT** included in the existing v8/accepted offline ARM build. This is not a ROM-ready adapter by itself. Current candidate bridge `usb_bridge_can_enter()` still hard-denies, firmware is unsigned and uninstalled; new driver and existing BOOT/EN FSM still require binding, budgeted ARM integration, high-/low-boot recovery tests, Mac mini host SLIP/esptool transport to proprietary USB 0x05/0x87, signed install, and later separate owner physical wiring/control and initial read-only ROM SYNC. ESP GPIO0/EN pullups must physically exist to 3.3V; do not assume CH340 adapter auto-reset still drives them once its control path is removed; GPIO2/GPIO15 strapping state must stay appropriate. ESP current CH340 3.3V power is proven for Node Link, but *not proof* of Blue Pill shared regulator 3.3V power capacity under Wi-Fi load. No INA226 necessary to develop ROM bridge, no automatic hardware/Flash permission.

## 1. Motivation and measured memory constraints

The owner has already proved physical STM32 Blue Pill **A2 (PA2, USART2_TX) -> ESP RX** and **A3 (PA3, USART2_RX) <- ESP TX**, common GND. STM32 has USB Management to Mac mini, first UART + ST-LINK to Windows. CH340 on Mac mini still supplies/controls ESP but its RX/TX are detached. **No wiring change is required now.**

The installed STM32 is signed normal v8. Genuine **OFFLINE Windows ARM15.3** source-only optimization experiments, with no device action, produced:

| Exact candidate | Flash B | Headroom to 53,248 B | Static SRAM B | Execution |
|---|---:|---:|---:|---|
| Installed-v8 source baseline (source equivalent to installed firmware; not a new install) | 53,196 | 52 | 8,468 | already physical v8 |
| scheduler_diagnostics.c `-O2 -> -Os` | 52,924 | 324 | 8,468 | OFFLINE native ARM PASS |
| previous change + four OLED/UI units `-O2 -> -Os` | **51,468** | **1,780** | **8,468** | OFFLINE native ARM PASS |

Latest original Windows Evidence: `../Evidence/stm32_os_esp01_flash_bridge_ui_os_arm_native_v1_20261107_20261107_235036_7192c3d7f8a24aa7b25c3ca34342500d.evidence.zip` SHA256 `2F08D05E60DFB665B45A3175ADABB5733426C4884729D76A7951C81F6D3713F2`. Independent DEUS MCP auditor verified ZIP CRC7/7, indexed SHA5/5, sizes5/5, exact native exit0, Git unchanged, 35 stack-usage files and no hardware actions. Candidate `os.bin` SHA256 `79B10B33B6CD4D7C714CF32E4FD5A8F69214A29BADD401FE543B61A7F5991A7E` is **unsigned and NOT installed**. Latest source-index synthetic identity `899a222270255abb3da1a0d7b22075155abaec2a` is **not** a live Git tree or signature.

**1,780 B is an available Flash ceiling for candidate additions, not a proven allowance for a complete safe bridge.** The real budget must include mode arbitration, UART failure recovery, USB protocol glue, optional control GPIO, stack and reserved margin. No assumption that all 1,780 B may safely be spent. `-Os` changes may affect timing; no OLED/scheduler runtime qualification of these candidates exists.

## 2. Division of responsibilities

- **Host (Mac mini)** owns firmware image, ESP8266 ROM SLIP packets, checksum/verification, timeouts and explicit user-initiated maintenance workflow. Neither ESP image nor SLIP/esptool engine is stored in STM32 Flash.
- **STM32 normal firmware** owns a small, *mutually exclusive*, bounded transport mode `NODE_LINK` vs `MAINTENANCE`; only routes already framed bytes over existing USB Management + USART2, exposes exact failures and keeps watchdog/Management responsive.
- **ESP-01** owns its actual ROM bootloader. Only separately controlled ESP GPIO0/EN/RST with verified electrical ownership can select ROM mode. This control does not exist in installed v8.

The existing STM32 USB framed transport uses magic `A5 5A`, protocol v1, CRC16 CCITT-FALSE, request payload up to 132 B and USB IN frame up to 64 B. USART2 has `USART2_NODE_MAX_TX_BYTES=40`, RX queue 256 and `usart2_node_try_read()` returns -1 on loss, then discards the partial queue. `node_transport_service()` sends periodic PING after the Node is ready. **Any maintenance TX while Node Link is active is forbidden**.

## 3. Proposed Management wire contract (NOT DEPLOYED)

The source-only Python model `nodes/esp8266/host/esp_flash_bridge_v0.py` has 15/15 OFFLINE host tests. Its proposed request/response frame types **0x05/0x87 are not allocated in deployed v8**.

| Field | Request payload | Response payload |
|---|---|---|
| Operation | `op: u8` | `op: u8` |
| Session | `session: u32 LE` | `session: u32 LE` |
| Sequence | `sequence: u16 LE` | `sequence: u16 LE` |
| Status | — | `status: u8` |
| Data length | `count: u8` | `count: u8` |
| Data | `count` bytes, max 40 | `count` bytes, max 40 |

`OPEN=1, TX=2, RX=3, CLOSE=4`. Total wire sizes: request <= **60 B**; response <= **61 B**. All outer frames carry CRC16 and nonzero request-id. Echo `request_id, session, sequence, op`, reject stale, crossed, duplicate or out-of-order requests. New `session` is host-generated; authorization must **not** be inferred from mere knowledge of a session ID. The model status set is `OK, DENIED, BAD_SESSION, BAD_SEQUENCE, BAD_LENGTH, UART_UNCERTAIN`.

**Critical engineering clarification:** An acknowledgment that an individual byte was written to the USART data register is *not* ESP-01 ROM acceptance. An interrupted partial `TX` cannot be retried automatically. The Host controls final ROM command correlation and Flash verification.

## 4. Mode ownership and terminal safety rules

Default after STM32 startup: `NODE_LINK`, no maintenance boot changes, no background serial bridge.

Proposed transition sequence (must be implemented/tested, not current behavior):

1. Owner explicitly requests maintenance. Reject if firmware update reset path is pending, another operation owns USB/UART, or safe control-line ownership/power conditions are unverified.
2. Stop automatic Node Link PING/TX from task0. A new explicit `node_transport_maintenance_enter` must preserve an unambiguous locked state; a simple `NODE_READY=0` observation is not enough. Disarm USART2 RXNEIE and flush Node Link partial frames/queues atomically.
3. Do not enter an ESP ROM session until **STM32 sole control** of GPIO0 and RST/EN has been physically designed and verified, including strap states, reliable regulated 3.3 V and absence of CH340 control contention. Initial source-only mode must return `DENIED` for `OPEN` if control is absent.
4. Only after host/firmware explicit state machine agrees, arm RX and route up to 40 raw bytes per command. Handle RX overflow, UART error/partial TX and USB TX backpressure by **FAULTED / no retry**, not successful ACK.
5. `CLOSE`, USB bus reset, timeout or device disappearance must **not** report ESP back in normal application mode and must **not** automatically reset ESP, flash, reboot STM32 or reopen CH340. Require independent restoration and fresh Node Link HELLO nonce before accepting `READY` again.

Do not starve the STM32 watchdog in a USART `TXE` wait: current driver has bounded `TXE_SPIN_LIMIT=30000`, not an empirically calibrated WCET. Maintain existing Management USB, scheduler/stack checks and one-consumer ownership of the USART2 RX queue. Later firmware update protocol (`0x04/0x86`) is for STM32 recovery entry and MUST remain separate.

## 5. Electrical/ownership prerequisites (not implemented)

Current physical data lines remain **A2 (PA2, USART2_TX)** and **A3 (PA3, USART2_RX)**. Potential extra STM32 pins **B0 (PB0)**, **B1 (PB1)**, **B8 (PB8)**, **B9 (PB9)** are **UNASSIGNED CANDIDATES ONLY**, not attached. B0 (PB0) is **not** the STM32 BOOT0 jumper. Existing OLED **B6 (PB6)** / **B7 (PB7)**, diagnostic UART **A9 (PA9)** / **A10 (PA10)**, USB **A11 (PA11)** / **A12 (PA12)**, and SWD **SWDIO (PA13)** / **SWCLK (PA14)** stay reserved.

ESP8266 strap selection: GPIO0 low *during reset* selects serial ROM; GPIO0 high selects flash boot; GPIO2 high/pulled up, GPIO15 low. Avoid hard-driving GPIO2 directly high because ESP ROM may also output on it. EN/RST and GPIO0 require electrically safe pull-up/open-drain or appropriate switched control, actual adapter circuit review, no simultaneous driving by CH340 and STM32. The current CH340 remains a power/control owner and must NOT be opened while STM32 is wired into ESP UART. Retire CH340 only after independently verified 3.3 V power capacity, noise/voltage margin and STM32 sole reset/boot ownership. Reference: https://docs.espressif.com/projects/esptool/en/latest/esp8266/advanced-topics/boot-mode-selection.html

## 6. Next scoped implementation work (source-only)

1. Implement and host-mock test an MCU-side **exclusive, fail-closed mode arbiter**, with `OPEN` disabled until hardware control ownership exists. Test byte-accurate frame 0x05/0x87, payload caps, replay, RX-loss, partial-TX, USB reset, watchdog-tick continuation, and Node Link state isolation.
2. Build exactly one **OFFLINE native ARM** candidate derived from the already accepted 51,468 B source-locked optimization snapshot, with only the minimal bridge C/headers, real ELF/MAP/BIN/stack usage and exact source diff. No need to repeat already accepted v8 HELLO/CRC or prior three optimizer builds.
3. Stop and revisit architecture if Flash headroom / runtime stack is inadequate. Consider a separate signed maintenance image only after version-floor/recovery compatibility planning; do not reallocate protected bootloader or increase the linker ceiling casually.
4. Only after owner approval + verified wiring: first bounded read-only ROM bootloader SYNC/identify, **NOT** ESP Flash write. Subsequent Flash+verify requires another explicit physical acceptance step. Existing signed normal v8 remains untouched until separately authorized update.

**This design document is not a deployed feature, not a hardware acceptance, and not Gate4 authorization.**
