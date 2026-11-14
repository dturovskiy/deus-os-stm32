# ESP01-016 — STM32 ESP bridge OPEN admission telemetry v1 (installed normal13)

**Актуальне на 2026-11-14 19:25:** normal14 вже зібрано Windows-native й підписано захищеним попереднім ключем; оригінальний Evidence та exact BIN незалежно перевірені. **На фізичному STM32 все ще normal13 із FAULTED=1; normal14 не встановлено, ESP ROM SYNC досі не підтверджено.** Наведені нижче датовані дії normal13 і попередні рішення — історичні, старі one-shot-команди заборонено повторювати.

**2026-11-14 02:00 UTC STOP — One CH340/USART2 ESP ROM attempt consumed; `UartUncertain` and latched bridge fault:** Original v13 ROM marker SHA `4DDF1430EADCC0E02CF863E0311D8D2A31A60E39489B256EEE9DEDDCE23D636C`, original child diagnostic JSON SHA `5102A30BADB23F9562D4A39A382F40442C71E90C8144174C838F12D46E054B5E`; no accepted ROM proof. Child OPEN admitted, CH340 ROM-control phase was invoked, but in WAIT_ROM_SYNC target returned `UartUncertain`, .NET child exited2 and was reaped. The captured diagnostic does **not** identify whether TX or RX callback faulted or electrically prove GPIO0/EN levels. A separate *passive-only* Mac info/health/node-status job SHA `A9C9C8EED57E83DE545D2B680279273B5CFFDB8A1D8DDFD3551015AB0FF07892` exit0 confirms SourceTree normal13, watchdog healthy, Node idle, **ACTIVE0 FAULTED1 UART_ATTEMPTED0 LAST_OPEN_REASON1 MAINTENANCE0 FW_RESET_PENDING0 SPLIT1**. Target source `esp_bridge_core_v0_abort` clears `uart_attempted` while preserving `faulted`, so post-stop zero UART_ATTEMPTED does not prove zero UART activity. **Do not replay** old/new ROM scripts, reset STM32, re-OPEN bridge, retry CH340/ESP strap or Flash ESP; fault remains latched. Next permitted work is offline diagnosis only, then a separately scoped physical-recovery decision. Old next-action prose below is historical.

**Status 2026-11-14 01:06 UTC:** **normal13 PHYSICALLY INSTALLED and read-only telemetry LIVE**, after owner foreground one-shot signed update normal12→normal13. Original Mac install log 1,363B SHA `C8A892E2FB5C838DF01AADDE684AD63768F3A35D3B254942A0E2B5892DFB4047` and consumed marker SHA `11DC40923334C8C35069DF505AAFEA8D42505A04E6D061DCEB04C0F467B710AA`. Runtime SourceTree `288b25818a250c79bd81b678dfebd61142bd139c`, watchdog healthy, Node idle. The physical `node status` second-line fields are **ACTIVE=0, FAULTED=0, UART_ATTEMPTED=0, LAST_OPEN_REASON=0, MAINTENANCE_OWNED=0, FW_RESET_PENDING=0, SPLIT_PROFILE=1**; independent read-only Mac job receipt SHA `7C30F2E3CB671E2C3415B51ADACA553952918ACA9F906351BD28FC5813992D6C` reconfirms. **No bridge OPEN has been issued since normal13 boot**; reason zero is not new Denied evidence and does not disclose the earlier normal12 failure. Old two ROM one-shot markers consumed, ESP ROM/Flash not attempted in normal13. Prior signed v12 remains in private recovery custody; v13 one-shot marker must not be replayed.

## Minimal existing read-only route

Add a **second output line** to existing `node status` (command/RPC ID `0x0027`), leaving its original `NODE_READY=... NODE_PENDING=... ... VALID=...` first line byte-compatible. No new RPC registry ID, USB frame type, capability, target BOOT/EN control, or reset path.

New second line prints hexadecimal 32-bit values:

```text
ESP_BRIDGE_ACTIVE=0x00000000 FAULTED=0x00000000 UART_ATTEMPTED=0x00000000 LAST_OPEN_REASON=0x00000000 MAINTENANCE_OWNED=0x00000000 FW_RESET_PENDING=0x00000000 SPLIT_PROFILE=0x00000001
```

The example above is an **illustrative fresh-state output, not physical evidence**.

Read-only source functions: `usb_management_esp_bridge_diagnostics_get()`, `node_transport_maintenance_is_owned()`. Both only copy in-memory fields. `node status` never invokes ESP `OPEN`, `can_enter`, `enter`, GPIO0/EN, USART2 TX/RX, reset or Flash. Output remains in the existing chunked, bounded RPC channel (Host aggregates up to 64 KiB).

### Last OPEN reason enum (decimal / hex32)

| Value | Constant | Meaning |
|---:|---|---|
| 0 | `ESP_BRIDGE_OPEN_NOT_SEEN` | No syntactically valid OPEN since MCU init |
| 1 | `ESP_BRIDGE_OPEN_ACCEPTED` | Last OPEN accepted |
| 2 | `ESP_BRIDGE_OPEN_DENY_SEQUENCE` | Nonzero initial sequence |
| 3 | `ESP_BRIDGE_OPEN_DENY_ACTIVE` | Existing bridge session active |
| 4 | `ESP_BRIDGE_OPEN_DENY_FAULTED` | Bridge UART/uncertain fault latch |
| 5 | `ESP_BRIDGE_OPEN_DENY_CALLBACKS` | Missing required privileged callback |
| 6 | `ESP_BRIDGE_OPEN_DENY_CAN_ENTER` | Firmware-local admission predicate failed; in split mode this gates firmware-update reset pending |
| 7 | `ESP_BRIDGE_OPEN_DENY_ENTER` | Exclusive NodeLink USART2 maintenance refused |

Values reflect **first failing predicate in original short-circuit order**. An active **and** faulted session reports `3`; separate `FAULTED=1` removes ambiguity. `LAST_OPEN_REASON` is latched from the last well-formed OPEN until another one or MCU reinitialization, and is not reset by a routine CLOSE or pre-UART abort. A malformed OPEN yields the distinct `BAD_LENGTH` wire status and does not update the enum. `MAINTENANCE_OWNED` and `FW_RESET_PENDING` are snapshots at **read time**, not necessarily at the earlier denial instant; `NODE_PENDING` is already on the first status line.

**Important limitation:** this addition cannot recover the exact reason behind the already-consumed normal12 Denied event. Nor does the output independently prove electrical CH340 DTR/RTS→ESP GPIO0/EN polarity. Future on-target diagnostics require independently approved firmware image/signing/version-floor/physical install and a *new scoped* read-only observation.

## Source and tests

Changed production source files (working tree only): `src/kernel.c`, `src/kernel/esp_bridge_core_v0.c`, `include/kernel/esp_bridge_core_v0.h`, `src/kernel/node_transport.c`, `include/kernel/node_transport.h`, `src/kernel/usb_management.c`, `include/kernel/usb_management.h`.

Offline C tests and source assertions updated in `nodes/esp8266/bridge/test_esp_bridge_core_v0.c`, `test_usb_bridge_split_current_wiring.c`, `test_node_maintenance_source_offline.c`, and `run_usb_bridge_split_current_wiring.py`. Bridge core `O0/Os/UBSan` 3/3 PASS `task_1794611504727_4ce41cdb9ddb7258`; USB dispatcher `O0/Os/UBSan` 3/3 PASS `task_1794611471442_2bcecc00b1f8eb58`; Node maintenance `O0/Os/UBSan` 3/3 PASS plus source syntax 4/4 `task_1794611474520_0f377f68917009e1`. The initial core test assertion incorrectly expected FAULTED in a simultaneously ACTIVE + FAULTED state and was corrected and rerun; this was a test expectation, not a target-side patch.

New **separate** Mac native stage (source clone, not the original normal12 stage): `~/deus-workspace/automation/host-work/esp016-bridge-admission-telemetry-offline-20261114`. Exact seven production source bytes copied from DEUS MCP and SHA-matched before build. Mac worker job `esp016-admission-telemetry-mac-native-arm-20261114-a1`, original receipt SHA `7D8AB1CFFB4A5B8B94C81D9F4898307B12DDE7537DDF34110FCA32F2438EF8DD`, exit0: native ARM GCC `splitopt3` + partial LTO builds 41 C units; Flash **50,808 / 53,248 bytes**, SRAM **8,596 / 11,264 bytes**, both PASS, BIN SHA `F52675E03A61277C71A24E5264E7D6A68790FDD433C368988D15AAE3D36197CE`. This binary is an **unsigned synthetic-SourceTree preview** (`4b45fac5e5c9bc964c29b5b141d62a20b07b2a2b`), **not** approved/suitable for direct device flashing. The signed v12 ProtectedRecovery package and installed MCU were unchanged. No owner-specific signing material, reset, GPIO or device I/O was accessed.

## Next decision

**New accepted observation 2026-11-14:** Normal13 one-shot physical OPEN returned `Ok`, immediate CLOSE returned `Ok`. Original Mac log SHA `61D9A84B41FB524027ED6B9E559C46E65CC702565AC296A6B46655A3A83C719F`, consumed one-shot marker SHA `FAA56756B6FD4CE778099BA7249C271335E39BF30A5AC7355AF285F14B3BAA6F`. Post-CLOSE physically read-only `LAST_OPEN_REASON=1`, `ACTIVE=FAULTED=UART_ATTEMPTED=MAINTENANCE_OWNED=0`; watchdog healthy. Therefore *normal13 bridge admission is proven*, although the historical normal12 Denied predicate was not reconstructed. **Next:** independently staged new normal13 CH340 GPIO0/EN + STM32 USART2 **ONE ESP ROM SLIP SYNC** operator `nodes/esp8266/bridge/esp016_normal13_rom_sync_one_shot_20261114.py` SHA `3DC01F53A4D1F022B07B4192101D41EFD02A2E00D47E4D5DA2B9D47553595C9D`, original Mac plan/no-GO negative/physical READONLY preflight PASS receipt `08893F2B143173F99D6FD73399FE8B9A2600E1605C6AAB1B34F2E90A5ED0E30B`. New ROM physical marker/proof **ABSENT**; one fresh owner foreground authorization required. No ESP app Flash, no reuse of consumed old normal12 or new normal13 admission markers.
