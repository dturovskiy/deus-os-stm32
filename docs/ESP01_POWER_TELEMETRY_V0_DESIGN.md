# Deus OS — ESP-01 GPIO ownership and measured power telemetry V0
Status: **DEFERRED BY OWNER (2026-11-08) — source-only previous design and mocks retained, NOT ON WIFI-FIRST CRITICAL PATH; NO HARDWARE AUTHORIZATION**
Date: 2026-11-08
Canonical global state: `docs/CURRENT_STATE.md`; physical-owner reference: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

## 1. Question: why can't the existing USB port simply measure amperes?

STM32F103C8 native USB full-speed device controller provides packet transport and USB signalling. Neither this controller nor the present wiring has a calibrated current-sensing front-end, a series shunt resistor or an INA-class current-sense ADC connected to it. USB descriptors' `bMaxPower` and host port budget are **negotiated/advertised amounts**, NOT USB bus-current or ESP branch current measurements. The Mac mini may expose some host/hub power budgets, but those are not grounded live milliamp readings per ESP. USB VBUS detection or an MCU ADC voltage measurement is likewise **not a current meter**. Software alone cannot recover the absent analog measurement.

USB Management already can carry sensor readings once Deus OS has a physical sensor, a valid I2C read path, status/quality bits and a safe host handler. This is a firmware feature request, not a built-in USB electrical feature.

## 2. Recommended minimal measured circuit (DESIGN ONLY, not installed)

* Source: one verified common USB/5V power distribution with a **separate** 3.3 V regulator for ESP-01, rated **>=500 mA output for ESP alone** and thermally suitable. The STM32+OLED can retain their existing 3.3V rail. Do NOT join two regulator outputs merely because both say 3.3V, and do NOT assume Blue Pill's second 3.3V pin is a second power source. Avoid assuming a Mac mini USB port or undocumented Blue Pill 5V header can supply a particular current without port/regulator qualification.
* ESP rail: **3.3V regulator output → 0.05Ω 1% (>=0.125W) current shunt → ESP VCC**. Mount the INA226 breakout high-side sense VIN+ **before** shunt and VIN− **after** shunt. Sensor bus voltage reading tracks the ESP side of the shunt. INA226 powered from suitable 3.3V with ground shared, GND common with Blue Pill and ESP.
* Proposed sensor address **0x40**, on the existing OLED I2C1: **B6 (PB6, I2C1_SCL) → INA226 SCL**, **B7 (PB7, I2C1_SDA) → INA226 SDA**, alongside OLED SSD1306 at **0x3C**. Ensure I2C pullups are to **3.3V** only, keep aggregate pullup resistance suitable for the bus, and no collisions with other devices. New wires and breakout are prospective only; no need to alter already-proven USART2 wiring before specific hardware GO.
* Existing USART2: **A2 (PA2, USART2_TX) → ESP RXD**, **A3 (PA3, USART2_RX) ← ESP TXD**, GND ↔ GND. For new controls propose **B0 (PB0) → GPIO0/BOOT** and **B1 (PB1) → EN/CH_PD** via 3.3V open-drain low/release-high with physical pullups; **B0 (PB0) ≠ STM32 BOOT0 jumper**. ESP GPIO2 boot strap and unpowered-ESP backfeed through PA2 need explicit protection review; no physical changes now.
* If the goal becomes **total USB input current** rather than **ESP-only branch current**, add a second high-side monitor **on the USB VBUS 5V input** (e.g. INA226 address 0x41) with safe external VBUS wiring, or use an actual calibrated inline USB power meter. The ESP-branch and USB-total sensors answer different questions. Only one monitor is proposed for v0.

No current/voltage values in this project are real sensor observations yet.

## 3. Device driver and interpretation contract

New actual source-only, not linked:
- `include/kernel/esp_power_monitor_v0.h`
- `src/kernel/esp_power_monitor_v0.c`

Contract for future board integration:
- Explicitly set the **actually fitted shunt value in milliohms** via `esp_power_monitor_v0_configure()`; no default is silently assumed. Mocked 50mΩ example yields shunt 2.5µV/LSB → 0.05mA/LSB; bus voltage 1.25mV/LSB. No floating point. No INA226 calibration register is needed **when interpreting raw bus/shunt registers directly**; hardware register source must be genuine.
- A board-provided I2C callback `read_reg16(context,7_bit_addr,reg,output)` must return 1 for successful read, 0 for sensor NACK/absent, -1 for I2C bus error. Expected raw register addresses are `0x02` for bus voltage and `0x01` for shunt voltage, address `0x40`.
- OS status is **NOT_AVAILABLE** (no configured shunt or no sensor), **VALID**, **STALE** (sample age >1000ms), **IO_ERROR**, or **OUT_OF_RANGE** (model flags 3.0–3.6V as diagnostic range; independent of guaranteed safe peak operation). No sensor → NEVER fabricate 0mA. Firmware must publish age and units.
- Future SAFE/read-only `powerstat` CommandService/RPC and `deus-cp power status` client should report `source=INA226|NONE`, `status`, `bus_mv`, `current_ma`, `power_mw`, `sample_age_ms`, `shunt_mohm`, and associated supply branch. This **has not been registered in CommandService, USB ABI or CLI**: do not claim current installed v8 supports the command. Keep it separate from STM32 bootloader/FW update.
- Current production `src/drivers/i2c1.c` provides only `i2c1_probe()` and `i2c1_write()`, **no register read/repeated START and 2-byte RX support**. Adding a verified read-register operation and task-level arbitration with existing SSD1306 100kHz transactions is necessary before actual telemetry. Never block the watchdog with unbounded I2C polling. On sensor NACK or bus error, continue OLED/Node Link/USB operation, report NOT_AVAILABLE or IO_ERROR; do not treat I2C errors as power-off evidence.
- Real supply capacity cannot be proved by a software reading alone. INA226 sampling/averaging may miss sub-millisecond radio current spikes; regulator/data-sheet budget and physical margin are still required. The 500mA Espressif specification is **recommended supply capability**, not a constant device current or an automatic software overcurrent-trip value.

## 4. BOOT/EN firmware component already implemented but NOT integrated

New actual source-only:
- `include/kernel/esp_boot_control_v0.h`
- `src/kernel/esp_boot_control_v0.c`

An inert callback-driven state machine with **DISABLED** as reset state, explicit local board provisioning, **NORMAL_LINK_VERIFIED**, **ROM_EN_LOW**, **ROM_SETTLING**, **ROM_AWAIT_SYNC**, **APP_EN_LOW**, **APP_SETTLING**, **APP_AWAIT_HELLO**, and sticky **FAULT**. Uses time-based EN low guard20ms and settle100ms, progress timeout1,000ms, open-drain low/release-high semantics, rail/Node idle checks, real fresh HELLO before reporting normal ready, and best-effort EN low on error. A timeout does **not** claim ROM handshake. Callback outputs are memory mocked; source does not write GPIO registers. These intervals are initial design choices, **not empirically proven ESP boot timings**. The live production bridge `usb_bridge_can_enter()` still RETURNS 0 and this module is **not** called by it.

Real firmware integration must ensure no concurrent Node Link pending traffic, fail-closed on USB reset/partial TX, and a separate owner-approved boot-strap/EN physical configuration. A newly signed normal-v9+ update may eventually be required because installed v8 does not contain these features and the bootloader version floor must be respected. The existing native ARM source-only bridge v0 measured **Flash 52,372/53,248B (876B margin)** and static SRAM8,480/11,264; neither new module has been sized for ARM yet, so one cannot assume they fit or recommend installation.

## 5. Original software-only proof

`nodes/esp8266/bridge/run_esp_boot_and_power_v0_offline.py`, with current source test `test_esp_boot_and_power_v0.c`, was executed via DEUS MCP task `task_1794095521035_9fc8d25050c0481c` exit0; GCC `-O0/-Os/UBSan` **3/3 PASS**. Covers ROM/normal state paths, 20ms/100ms transitions, power loss, callback failure, sticky fault, unprovisioned refusal, unknown/stale/bus-NACK/error sensor, calibrated 50mΩ vs 100mΩ raw current, 3300mV /500mA/1650mW example and 32-bit timestamp wrap. **No USB, GPIO, I2C, ESP, physical boot, Flash or board work occurred**.

Future order: (1) I2C register-read + bus arbitration host mocks, (2) source-only GPIO driver adapter and signed boot-mode configuration, (3) SAFE read-only CLI `powerstat` with correct NO_SENSOR handling, (4) one native ARM fit proof only after integrated source changes, (5) owner-approved hardware migration and instrumented power/USB load test, (6) read-only ROM SYNC/CHIP ID and recovery to normal fresh HELLO, (7) separately authorized ESP Flash only if requested.

Primary technical references: TI `https://www.ti.com/product/INA226`; Espressif ESP8266 startup `https://docs.espressif.com/projects/esp-techpedia/en/latest/esp-friends/get-started/try-firmware/try-firmware-hardware/esp8266.html`; Espressif ROM strap `https://docs.espressif.com/projects/esptool/en/latest/esp8266/advanced-topics/boot-mode-selection.html`; STM32 USB hardware application note `https://www.st.com/content/st_com/en/technical-documents/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.html`.
