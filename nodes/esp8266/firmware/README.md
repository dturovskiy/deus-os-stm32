# Deus OS ESP-01 v0 — standalone RTOS SDK application (ESP01-008)

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
