# Deus OS ESP-01 v0 — standalone RTOS SDK application (ESP01-008)

**2026-11-15 00:21+01 — PRIOR FLASH VERSION/CH340 RUNTIME CONTRACT REUSED TO RESTORE PHYSICAL DIRECT NODELINK:** Historical ESP01-013 2026-11-05 source-bound app-only install/3/3 Flash verify is authoritative for installed v2 (131664B SHA `FB6929A584FBF9F4DF0032BFABC52DD794C901B5DC146C35FD6003AE9CC3F09E`), no later app Flash authorized/observed. Old v1 SHA `052A937EFE3336A1140AF24AA360D963291B397209B39EE2072F0C61E6799138`; WiFi lab 436448B SHA `541FB4F3A102685611E839A21F689098392C74542EDC53B88C4128EB2FE82070` remains NOT_INSTALLED and is outside `firmware/main`. On owner's new direct CH340 wiring, serial line `hupcl` was ON. Reused existing old guard to set `-hupcl` (Mac receipt SHA `F677BE0EC11BC74E41A29EAC51BFC229EF45610F434923328C25E347FBDE8FDE`), then **exact preexisting 2026-11-05 v2 host client** physically HELLO/PING/CRC32/IDEMPOTENCE/STATUS 5/5 PASS SHA `F8B8D450AA0534BF54729FB08C04C47C4F14A962F45A5EE9B49E550190599DB9` without app reset/Flash. Exact prior app-v2 private image/backup/bootloader/partition hashes rechecked in native Mac preflight SHA `8A7C59F402DB82BD0B720DDAB825C40EB0CBC0CC94B093814694F080E3DC889A`, no NEW on-chip read. Do not diagnose ESP firmware corruption or install WiFi lab solely because older ad-hoc probes failed with HUPCL_ON; verify host tty policy first.

**2026-11-10 — New Wi-Fi first ESP app candidate: SDK native Xtensa SOURCE_ONLY PASS (latest):** The isolated `wifi_lab/` now includes a Wi-Fi-aware DNL dispatcher (`wifi_node_dispatch_v0.{h,c}`), SDK scanner callbacks (`wifi_node_sdk_bridge_v0.{h,c}`), and UART task candidate (`uart0_wifi_candidate_v0.c`) along with optional bounded scan codec. It preserves frozen DNL REV2 HELLO/PING/CRC session/sequence/duplicate behavior and limits Wi-Fi scans to explicit on-demand commands; no credentials. Real Mac SDK mocks GCC O0/Os/UBSan dispatcher+scanner **6/6 PASS**, patched SDK nonzero scan-done status is ERROR and generation wrap fails closed. Real pinned ESP8266 SDK v3.4/Xtensa GCC8.4 **isolated `make all` PASS** and app-only BIN **436448B** SHA `541FB4F3A102685611E839A21F689098392C74542EDC53B88C4128EB2FE82070`, privately preserved offGit with mode0600, original native receipts independently audited SHA18/18. **Not in production `main`**, installed ESP v2 unchanged; this image, newly built bootloader/partitions, any RF work and STM32 transport integration are NOT authorized for installation. Follow `docs/ESP01_WIFI_STM32_FIRST_V0_PLAN.md` and `docs/CURRENT_STATE.md`. No hardware operation/Gate4.

**2026-11-10 — WIFI NODELINK EXTENSION V0 PROTOTYPE, SOURCE-ONLY NATIVE MAC O0/Os/UBSan PASS 3/3:** New off-app `wifi_lab/wifi_node_extension_v0.{h,c}` adds proposed WiFi CAPS/SCAN_START/SCAN_STATUS/AP_PAGE codec (`0x05..0x08`, replies `0x85..0x88`) based on **unchanged frozen DNL REV2**, pages 32B SSIDs as 14+14+4 over max24 payload, validates 32-bit scan generation, session/sequence, error/duplicate-safe envelope at caller boundary; **no SDK/RF side effects**. Test `tests/wifi_node_extension_v0_tests.c`, stdlib runner from OS root `python3 tests/test_esp_wifi_node_extension_v0.py`. Genuine Mac GCC15.2 O0/Os/UBSan 3/3 PASS, source six mirrored SHA6/6, original native receipt SHA `77ED537B89C4F7A08799916FB9AE2B71E33CB8204986DB6057288B706DC79C7E`; Python unittest wrapper also native 3/3 PASS receipt SHA `DD875832AAAE9669EC2544E07B0F8DA1DE962680051AD34527AE033F7D8BAF45`. No ESP v2 `main` integration, no ARM/Xtensa install/Flash, no actual WiFi scan yet; frozen DNL v2 worker replies UNSUPPORTED to proposed commands. Full wire details `docs/ESP01_WIFI_STM32_FIRST_V0_PLAN.md`.

**2026-11-08 — WI-FI FIRST, NEXT SOURCE-ONLY EXPERIMENT:** The installed ESP app v2, which runs UART0 Node Link HELLO/PING/CRC32/STATUS, currently does NOT call Wi-Fi SDK scan/connect. New isolated prototype source `../wifi_lab/wifi_scan_v0.c` + `.h` uses ESP8266 RTOS SDK v3.4 nonblocking STA Wi-Fi scan/status, callback completion and 8-second timeout, top 4 AP entries with SSID up to 32 bytes, no SSID/password provisioning. The lab directory is outside auto-compiled `main`, so the accepted ESP v2 build layout is not altered by this new code. SDK in-memory host mocks at `tests/run_wifi_scan_v0_sdk_mock_tests.py` GCC O0/Os/UBSan **3/3 PASS**, DEUS MCP task `task_1794096247922_5e8744bd862a99ea`. **No native Xtensa build, actual RF/Wi-Fi scan, Node Link scan extension, ESP Flash, STM32 firmware change or physical work yet.** Minimal milestones and power caution in `docs/ESP01_WIFI_STM32_FIRST_V0_PLAN.md`; owner defers INA226/current monitoring.

**ESP01-013 PHYSICAL ESP-01 UART0 SW V2 APP-ONLY FLASH + INDEPENDENT THREE-REGION VERIFY + DUAL UART REBOOT PASS (2026-11-05 00:30 Europe/Paris; fresh explicit owner GO):** On the **unchanged standalone Mac mini CH340 ESP-01**, installed the **new application only** at `0x10000`, **131,664 B SHA `FB6929A584FBF9F4DF0032BFABC52DD794C901B5DC146C35FD6003AE9CC3F09E`**, using already accepted SDK3.4/Xtensa GCC8.4 source build. Original installed bootloader at `0x00000` and partition table at `0x08000` **not written**. SHA/size preflight confirmed protected full 1MiB stock backup, original accepted bootloader/partition/old-app source images, new application source/BIN, exact CH340 and existing `-hupcl`. Actual esptool **write_flash --verify PASS**; separate new-app `verify_flash` PASS; later independent **all THREE on-chip regions digest matched** (original bootloader, original partitions, new application) 3/3 PASS, followed by `--after hard_reset`. After reset, original immutable Nov3 standalone Node Link 5/5 physical UART smoke **PASS**, and newly authored v2 Python host client on real CH340 also **5/5 PASS**; both passed in separate processes **before and after** three-region post-write verification. Direct original receipt SHA: Flash `13FA476C207653FB5A2406D97967B1B70BDD134062062A1D5EB931030E16880F`; all-three read-only verify `73773DECAD167096CA8CF9C68477861DA66FF08B627E97581C44BFECE0A7D191`; v2-client after reset `2D2B7B7F8BB70C53089740929FDF3591ED8F7724B0450EB933F8C094AA9EF2AE`; old-client after reset `8BF80FFEF03BB5EBC7742BE95CC66B500C28686C1B29AB164BBA5929E36198E8`. Full original evidence and acceptance boundaries in `Coordination/tasks/ESP01-013/results/ESP01_013__PHYSICAL_V2_APP_ONLY_FLASH_VERIFY_AND_DUAL_UART_REBOOT_PASS_20261105.md`. **CURRENT INSTALLED ESP-01 APP IS V2, PHYSICAL STANDALONE 5/5 AFTER RESET ACCEPTED**. Original Nov3 installation acceptance remains historical. Source robustness recovery branches are host-mock proven but not physically failure-injected. No owner-requested USB replug/power/wiring changes, no STM32 reset/Flash, no STM32↔ESP physical E2E, no commit/push; v2 live proof limited to Mac mini standalone UART.

**ESP01-013 SOFTWARE-ONLY UART0 V2 (2026-11-04 UTC; SOURCE/NATIVE XTENSA PASS, NEW FIRMWARE NOT INSTALLED):** Original accepted Nov3 ESP-01 on-device app is **not proven defective**: original 3 Flash regions SHA-matched accepted images, normal boot and real Node Link 5/5 were previously demonstrated; intermittent Mac mini CH340 client failures correlated with Linux tty `hupcl` and were previously mitigated, not caused by a proven corrupt image. New source fix `nodes/esp8266/firmware/main/uart0_adapter.c` 2,866B SHA `1A22DB990B32911A51C3AD7A8D1F8422ABBA3C028606A60C0CC1E60C288AD5A4`: recover from transient UART init error with RTOS backoff instead of permanently returning; on SDK negative RX avoid busy-spin and discard partial frame. Frozen Node Link C codec + worker unchanged. **Actual no-device host GCC O0/O2/UBSan UART0 mock tests 15/15 PASS**; Python CH340 host HUPCL-on-open-fd policy 6/6 mock PASS; explicit-opt-in Node Link host v2 7/7 in-memory tests PASS; *host v2 has not opened a real port*. Real Mac mini pinned **SDK v3.4, Xtensa GCC8.4.0** native fresh staged **`make all PASS`**, immutable host receipt SHA `520E888434BC6A77B878281460CD62A1A8960559EECAFBD0503BC6D9CB9524E8`. New **application BIN 131,664B SHA `FB6929A584FBF9F4DF0032BFABC52DD794C901B5DC146C35FD6003AE9CC3F09E`**, real ELF/MAP generated, offline `esptool image_info` checksum valid and fits 1MB factory app partition. **NOT INSTALLED / NO PHYSICAL V2 RUN**; existing Nov3 application BIN 131,536B SHA `052A937EFE3336A1140AF24AA360D963291B397209B39EE2072F0C61E6799138` remains historically accepted. Rebuilt bootloader output hash differs from original, so **DO NOT REPLACE BOOTLOADER** from this build; app-only candidate. All native outputs private offGit. Full software report `Coordination/tasks/ESP01-013/results/ESP01_013__UART0_FIRMWARE_SW_V2_NATIVE_XTENSA_AND_HOST_POLICY_OFFLINE_20261104.md`. **Owner hardware boundary respected:** no CH340/ESP/STM32 UART open, termios change, reset, Flash, wiring, power experiment, signing, commit or push in this software task. Continue software-only work without requesting physical troubleshooting.

**Current status (2026-11-01): ESP01-008 native Xtensa source build ACCEPTED; ESP01-009 host integration emulator 36/36 and owner native v3 config-derived 1MiB Flash geometry/static ELF audit ACCEPTED OFFDEVICE.** Original v3 Evidence SHA256 `787A9BA39F8594BA22FCB3217BC5BA1E6317F51D6787AE98C90F3739E083B378`, ZIP CRC6/6, indexed SHA5/5. Physical ESP startup/Flash/UART, live free-heap margin and STM32 link remain unverified. ESP01-010 STM32 USART2 source-only review is PARTIAL. Historical v1/v2 statuses below are dated provenance, not the current next action.

**Scope:** A000018 / E003 / T008 / G001 source-only SDK v3.4 application
and UART0 adapter. Do **not** flash/power/reset any device. Do **not**
replace the owner's unrelated ESP-01 Lamp Control files or existing ESP-01
firmware: this is a separate Deus OS v0 node.

## Inputs / provenance

Pinned ESP8266 RTOS SDK **v3.4** Git
`89a3f254b63819035f65d9c5dcdae8864f1a6a8a`, GCC **8.4.0**, selected
1-MiB ESP-01 flash (original owner vendor/read-only evidence).
Frozen Deus Node Link REV2 source SHA-256
`899E2FBF4366CE91C31D6B1E6B481195DEECE5DF21AD4495F78E1D450DE406BA`.
Private source-scoped owner acceptance under sibling
`Coordination/receipts/A000016__E002__T006__G001__OWNER_ACCEPTED_SOURCE_ONLY_FROZEN_V0_20261101.json`.
Canonical pure C11 codec source in `OS/shared/node_link/`; compiled into the
app via the *thin* `main/node_link_impl.c` translation unit. No copied codec.

## App design and limitations

- `main/node_runtime.[ch]`: deterministic, allocation-free receive stream /
  frame / request-response worker. One fixed 40-byte frame and bounded worker
  replay/HELLO state. CRC32 jobs are strictly limited to 0..16 bytes.
- `main/uart0_adapter.c`: SDK `uart_param_config` / `uart_driver_install`,
  `uart_read_bytes` / `uart_write_bytes`, UART0 default Tx/Rx, 115200 8N1,
  no application console logging and no Wi-Fi task. The **SDK driver itself
  allocates ring buffers**; the no-heap claim applies only to codec/runtime
  application source, not vendor FreeRTOS UART implementation.
- ESP8266 UART0 is also ROM boot/debug UART. **ROM boot messages cannot be
  suppressed by this application** and the STM32 peer must resynchronize
  using magic + version + length + CRC. There is no claim of clean wire
  before firmware starts. Firmware native console may also print until
  separately configured; such output is transport noise, not a Node Link frame.
- SDK UART0 driver uses an rx/tx ring of 256 B each; runtime reads 32 B chunks
  and transmits <=40 B replies; finite 10-ms RX blocking budget. Does not
  assert physical timing, voltage, reset recovery or driver-memory fit.
- No physical ESP/STM32 observation, secure authentication, GPIO wiring,
  Flash control or arbitrary command execution is present.
- `sdkconfig.defaults` is the source candidate for 1-MiB DIO flash. The
  owner native pinned SDK `make defconfig` **PASS** confirms effective 1-MiB
  Flash selection (`sdkconfig` SHA `20A52F076AD70077687FF917889207022FEDF12CC7431E9D51A89FAD99B306F0`),
  and the actual v4 Xtensa `make all` **PASS** produced ELF/BIN/MAP as original
  Ubuntu-host size/SHA attestations. The subsequent ESP01-009 owner native v3
  audit accepted SDK-config-derived nonoverlapping 1MiB Flash offsets and 70
  static ELF sections, **not actual Flash programming, boot, live UART or
  runtime free heap**.

## Build mode (owner Ubuntu host only, separately scoped)

SDK v3.4 normally uses the Make project `Makefile` and per-component
`component.mk`. Use exactly the previously provisioned pinned owner Ubuntu
SDK/GCC/Python workspace and keep build outputs **outside Git**. This source
file intentionally gives no Flash command; follow accepted operator path
only after a target source-build package is separately prepared and checked.

## Actual evidence scope (2026-11-01)

Source-only Linux native GCC C11 host integration with the portable module:
`-O0` 12/12, `-O2` 12/12, UBSan 12/12 — **36/36 host PASS**.
Run via sibling `Tools/scripts/esp8266/test_esp01_node_runtime_host.py`.
**Actual owner native v4 SDK build PASS (2026-11-01):** v3 first produced genuine Xtensa queue.h / uart adapter header-order FAIL; v4 fixed only the UART adapter include order to put `FreeRTOS.h` before `driver/uart.h` while preserving all other firmware source hashes. Original v4 `deus-run` Evidence SHA `4F2FDAC6D76B3E6F275F3183673E989C970CFEBE2CD3F4ABCB2CEB0A49B4D48D` independently verified (CRC 6/6, SHA index 5/5, original source hashes 9/9). Pinned SDK v3.4 / GCC8.4.0 native 1MiB `make defconfig` and `make all` PASS, original Ubuntu reported 2 ELF + 3 BIN + 2 MAP SHA/size; main application BIN 131,536 B SHA `052A937EFE3336A1140AF24AA360D963291B397209B39EE2072F0C61E6799138`. Source host tests 36/36 PASS and mock header-order positive/negative checks PASS. **This is no-device firmware source-build evidence, not ESP boot/Flash, firmware-image 1MiB layout fit or emulator/STM32 integration**. ESP01-008 accepted within scope; ESP01-009 owns separate resource/Flash/timeout proof.
**ESP01-009 additional source-only integration proof:** controller-to-worker serialized byte-stream C emulator (new `tests/node_link_offline_integration_009.c`) passed 12/12 O0, 12/12 O2, 12/12 UBSan = 36/36 host PASS, including HELLO/CRC32, duplicate, lost reply/retry, stale session, timeout, max payload, noise and sequence wrap. Real ESP/STM32 UART not exercised. A separate **read-only** inspection of already built Ubuntu v4 ELF/BIN/MAP is statically packaged (`Bundle/stm32_os_esp01_009_native_resource_audit_v1_20261101.zip` SHA `A9D370C98286340E70348525344AD30936038C6DDB6A85E08209DF5EA744C4F6`), and owner native v1 has now been executed: original Evidence SHA `8D2A97D318DFE5F3DEB421618DD54885DC783D8F0B912A942DC95A1B4EDF6E53` independently proves source rehash 9/9, exact v4 ELF/BIN/MAP byte rehash 7/7, and partition NVS/PHY/factory decode PASS; v1 classified FAIL `ACTUAL_FLASH_OFFSETS_MISSING` because no generated flash-args file existed. Corrected source-only read-only v2 audit ZIP `Bundle/stm32_os_esp01_009_native_resource_audit_v2_20261101.zip` SHA `F32C7A50905C79BA2A193BE523E7896134477C518824BC20EC9F1FFB4E295610` verifies pinned SDK Make/actual sdkconfig values to derive bootloader/table/app offsets and ELF static sections. The v2 audit was subsequently executed by the owner and genuinely failed from a 39-character pinned SDK Make Git-blob SHA typo (not a firmware failure). Corrected owner v3 native Windows/Ubuntu audit then PASS: source 9/9, existing actual v4 artifacts 7/7, SDK Make blobs 4/4, nonoverlapping bootloader/partition/app offsets and 70 static ELF sections; original indexed v3 Evidence SHA `787A9BA39F8594BA22FCB3217BC5BA1E6317F51D6787AE98C90F3739E083B378`, independently verified CRC6/6, SHA5/5. Runtime heap is NOT_ATTESTED; preserve original ESP firmware and exact Evidence; no device Flash operation.

The current private coordination board is sibling `Coordination/INDEX.md`;
ESP01-008 and ESP01-009 are recorded accepted in their exact off-device scope.
The only global product-status authority is `OS/docs/CURRENT_STATE.md`.
