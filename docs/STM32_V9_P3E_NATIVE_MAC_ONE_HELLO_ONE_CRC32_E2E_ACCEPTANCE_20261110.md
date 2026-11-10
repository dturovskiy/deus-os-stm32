# Deus OS — P3-E physical STM32 normal v9 → ESP-01 NodeLink one HELLO / one CRC32

**Observed:** 2026-11-10 02:25–02:29 Europe/Paris (original Mac worker timestamps 01:25–01:28 UTC). **Authorization:** owner explicitly said **"ВИконуємо"** after specifically offered active STM32↔ESP-01 on normal9 one `Node HELLO + CRC32` test. Scope excludes STM32/ESP Flash, RESET, entering bootloader, CH340 UART, GPIO/EN/BOOT control, USB permission changes, long-running workloads, Gate4 release, and Git stage/commit/push.

**Decision:** `P3_E_PHYSICAL_STM32_NORMAL9_ESP_NODE_HELLO_AND_CRC32=ACCEPTED_SCOPED`; new actual signed-normal9 STM32 runtime at `1209:000E` exact `/sys/bus/usb/devices/1-8`, `usb:001:8`, `/dev/bus/usb/001/101` `deus:deus:600` reported SourceTree `edc5e08ae8dee2455994339c4c438e620c84df1a` (accepted 75-file optimized v4 source-index prefix), Protocol 1 / Service 3 / Registry 39. **One actual `node start` HELLO was queued with fresh nonce `6e4039d9`**, subsequent distinct read-only Mac job observed **`NODE_READY=1 PENDING=0`**, and **one distinct `node crc DEUS-V9-ESP-P3E`** was queued. Fresh Node status returned **`JOBS_VERIFIED=1 JOBS_BAD_CRC=0 LAST_CRC32=0x7CEEDB1A VALID=1`**, matching independently computed Python `zlib.crc32` for exactly 15 printable ASCII bytes. Watchdog active, IWDG_RESET0, MSP/PSP/scheduler PASS. This qualifies one E2E HELLO+CRC transaction, not long-term link stability, fault injection, OLED/I2C or unrestricted ESP access. `ESP01-010=PARTIAL`; `GATE4=NOT_AUTHORIZED`.

## Frozen original identities and restrictive execution

- P3-B original normal9 update bootloader-commit receipt SHA `FABE432FE9FDCE1FC5CFFB5FCBD6337003256FFDB6E411B2114BEC5A59D6E78A` and one-shot update marker SHA `60BADF3A044186A8A5ADFC4EB5AE4D389D89098D0E5142CBB258357776E21211` untouched. P3-D 20s normal9 idle original receipt SHA `5E9E3ED1A2A0AE96EB071F760B9E9D389E5A569F3777BB02F28664BACED22D20`, all Node status counters 0 pre-P3-E. Isolated source-pinned v9 Host DLL SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`, v9 Host class SHA `ACC3D74653FDEFFD1E6F9585A5AAF4A669A733F86C7E4FE1F44D4C3B49F84DF1` unchanged.
- Historic actual pin wiring accepted on v8 and retained as documented topology: STM32 Blue Pill **A2 (PA2, USART2_TX)** to ESP RXD; **A3 (PA3, USART2_RX)** from ESP TXD; shared ground. Separate CH340 supplies/controls ESP power but its UART TX/RX is not used by this test. No cable/CH340 access or fresh electrical reinspection. The new normal9 E2E traffic traverses STM32 Management USB → NodeLink USART2 → ESP-01 and reverse for ACK/CRC result.
- First exact pre-reviewed Mac source `host-work/esp010-v9-cli-candidate-20261109/p3e_v9_one_node_hello_one_crc_20261110.sh`, **4,943B SHA `72C3E0D660EB9826EB7A63BCE2B1A6ACCB375731DF9267B2F67BD70B04036281`**, had only one `node start` and one *conditional* `node crc`. Offline native Mac source/hash and CRC check PASS without USB. Its dedicated marker **created BEFORE HELLO**: `.p3e_20261110_once_hello_crc_attempted`, mode600 SHA `6D8764D79857F0A60CCFE5E81D4F6CF0790B0676B575DE247F3860744571C820`. **Consumed, never replay HELLO**.
- Crucial preserved original controlled script assertion failure **`command_failed exit1`** just after real `NODE_HELLO_QUEUED\r\n`: `grep -Fxq` wrongly expected LF-only and rejected CRLF. Original full log contains the actual positive queue marker and `P3E_STOP phase=ONE_NODE_START_ATTEMPT exit=1`; no CRC sent in this job. **It is a real test-script text-validation FAIL, not a failed MCU/ESP HELLO.** Do not overwrite/sanitize original result or relabel it exit0.
- A separate fresh **read-only** native job, source- and marker-pinned, saw `NODE_READY=1 PENDING=0 VERIFIED=0 BAD_CRC=0`, confirming the one already-enqueued HELLO ACK with **zero HELLO retries**.
- Only then a separate source-pinned **CRC-only** script `p3e_v9_crc_only_after_hello_ack_20261110.sh`, **3,912B SHA `24DE9CD710FD6395367FDD8AAB834319869D8DD429BAAB7265283AA5BB0331C7`**, Mac native `bash -n` and Python CRC independent precheck PASS without USB. It re-pinned previous Mac ACK receipt and original HELLO marker, obtained fresh `READY1 PENDING0 VERIFIED0 BADCRC0`, and created `.p3e_20261110_once_crc_attempted` **before** one CRC operation (mode600 SHA `2FBAD2E69354E0BE5D7C578427F92F218F4DEA7A2144CC7380EFE1D1D32445D7`). It explicitly normalized CR in Host one-line output and did not call `node start`. **Consumed, never replay CRC**.

## Genuine native Mac source/queue/typed receipt/full log custody

Worker `macmini`, `deus-command-worker-v2`, `host_bash_as_deus_no_os_sandbox`. Original files live under `DEUS-MCP/macmini-workspace/automation/` in `command-queue`, `command-results`, `host-logs` with corresponding exact job IDs. Their original SHA256 metadata were independently re-read with DEUS MCP. **Original 15 files / SHA15/15 / job-ID + request-hash binding5/5 / full log+typed receipt byte-content and count5/5 / proper native worker identity5/5 all PASS**. Precisely one original job is exit1 (the text assertion fail, not hardware failure); other four jobs completed exit0 without timeout. Never overwrite originals.

| Job ID after `esp010-v9-p3e-` prefix | Queue SHA256 | Receipt SHA256 | Full log SHA256 | Native result |
|---|---|---|---|---|
| `offline-source-preflight-1110-a1` | `C57E7B941B78F1DA289593FD6FEC980AA9F395B516BAFF6C7DD8D53DBA2702F9` | `12CB37D940172D102A17E80E211B9AF10F615D6E0B6486BB44830703FCCC98A1` | `3D17630D1B48FFD9209801B49F5003C6D2F176C9727747AF2BCCC1AF960ADB8E` | completed/exit0, offline only |
| `hello-once-crc-conditional-1110-a1` | `AC7C2B74D43CB350C47E1C1FF3D0C6E6B073ADFC4824931749730BE3DC3C4815` | `855C3CE861ED99E1FF18A3E1AD2011D909F95DFC56A25F3B28C8F946CBF6C009` | `0B63AF594A902FDE67DC74EC3CD888047CD3B767BB3EBFD577A3C0080F57A066` | original `command_failed/exit1`, after ONE queued HELLO |
| `posthello-status-readonly-1110-a1` | `B8BC9608EAA2A252EBCE0E8640F86476D9699A9B64147367B4F3D6C51F2CB291` | `1A682257667CCD9ED1FFB00D4D81C3711B2D405410416522359F8039C65317EC` | `53D21332628011546FAFB20FB0927BE79C6BC2B1BF26D69EC5E802CE3B5680FD` | completed/exit0, ACK read-only |
| `crc-continuation-syntax-1110-a1` | `CEB21CE5AF8F396EEF075617E78F9FCC7C85EDB6E01BC79264126F6A8465D1FD` | `52B6470802B8C1F1BE3E2B6AF4C6DBFA1427FAA85339D6D8C1D3E8E606868A29` | `EDAD45AE11E32955221C16BB217EFC3C7CFA397D4674EFEF03C79A4CC90B0959` | completed/exit0, offline only |
| `crc-once-after-ack-1110-a1` | `DF9ACA7B798B4AE1C0D9BE117B06E2A9658BAB22206613F119C88BAA0976BF08` | `983629C47782158CC63F757F72083D7600AFF26513C18A691DB4A201DCAEFCD3` | `6E3558747A80BDB53D9BC8A594F6944EEBF9C1CE93DD84FEA9EE4E3824140E4C` | completed/exit0, ONE CRC verified + posthealth |

## Decisive actual device output and health

```text
P3E_HELLO_ONESHOT_MARKER=CREATED NONCE=6e4039d9
P3E_HELLO_COMMAND=NODE_HELLO_QUEUED<CRLF>
P3E_STOP phase=ONE_NODE_START_ATTEMPT exit=1 NO_RETRY_NO_RESET_NO_FLASH_NO_CH340=YES
# Separate subsequent actual Mac read-only probe:
NODE_READY=0x00000001 NODE_PENDING=0x00000000 JOBS_VERIFIED=0x00000000 JOBS_BAD_CRC=0x00000000 LAST_CRC32=0x00000000 VALID=0x00000000
# Separate conditional one CRC-only physical job:
P3E_CRC_BEFORE=NODE_READY=0x00000001 NODE_PENDING=0x00000000 JOBS_VERIFIED=0x00000000 JOBS_BAD_CRC=0x00000000 LAST_CRC32=0x00000000 VALID=0x00000000
P3E_ONE_CRC_MARKER=CREATED_BEFORE_ACTION
P3E_CRC_QUEUE=NODE_CRC_QUEUED
P3E_CRC_STATUS_1=NODE_READY=0x00000001 NODE_PENDING=0x00000000 JOBS_VERIFIED=0x00000001 JOBS_BAD_CRC=0x00000000 LAST_CRC32=0x7CEEDB1A VALID=0x00000001
P3E_CRC32_ONE_V9_JOB_VERIFIED=PASS_VALUE_7CEEDB1A
P3E_V9_REAL_STM32_ESP_ONE_HELLO_AND_ONE_CRC_E2E=PASS_SCOPED
```

Post-CRC native physical read-only `HEALTH` `WDOG_ACTIVE=1 IWDG_RESET=0 RESET_FLAGS=0x14000000`; `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS`, all MSP/console/heartbeat canaries intact and scheduler fault0. Actual high-water margins **MSP 1564B, console 396B, heartbeat 432B**. Previously idle v9 console margin was **488B** before active NodeLink; it decreased **92B** with actual active work, still above accepted **256B task-margin policy**. No claim that this is worst-case WCET or maximal physical load.

## Boundary and next milestone

**Accepted:** normal9 physical management → STM32 USART2 ↔ ESP-01 application ACK plus one verified CRC32 payload E2E, WDOG/stack post-condition, original native source/receipts preserved. **Not accepted/authorized:** sustained link under stress, absent/unpowered ESP electrical stability, further CRC jobs, Wi-Fi/ESP maintenance/Flash, OLED/I2C physical visual presentation, reboot/power-cycle persistence, bootloader floor9 independent postcommit readback, formal Gate4. The consumed HELLO and CRC one-shot markers prohibit automatic repeated attempts. Any next hardware mutation needs separately explicit owner consent. No Git stage, commit or push.
