# STM32 normal9 — P3-H one fresh native Mac mini HELLO recovers NodeLink

**Physical execution:** 2026-11-11 17:10 UTC (18:10 Europe/Paris). **Status:** `P3H_NORMAL9_ONE_FRESH_HELLO_ACK_READY_ACCEPTED_SCOPED`. This report is dated original-proof analysis; global project-state authority remains `docs/CURRENT_STATE.md`.

## Authorized scope and actual result

User approved continuation immediately after P3-G passive `READY=0` and the explicitly proposed one HELLO/status/health test. Under that **narrow hardware scope**, one native Mac mini host command issued **one** `node start <fresh_nonce>` request on unchanged installed signed normal9; no CRC compute job, no second host HELLO, RESET, Flash, CH340, ESP GPIO/boot/power, Wi-Fi radio or RF scan.

The native worker `deus-command-worker-v2`, host `macmini`, reported `completed`, **exit0**, `timed_out=false`, elapsed **3.472 seconds**. Its script SHA-pinned the original installed normal9 .NET Host CLI (SHA256 `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`), previous P3-G original receipt (SHA `127640F3CA4BA289617CE1938A18131DB23F14517BAE462E77435FC514A7ABBE`), native Mac user/USB `1209:000E`, `usb:001:8`, `Protocol1 Service3 Registry39`, and installed `SourceTree edc5e08ae8dee2455994339c4c438e620c84df1a` before acting. A new private **one-shot marker** was created before the only host `node start`, source-pinned and never deleted/replayed. The fresh nonzero nonce was `8be5ef5d`. The original firmware controller can implement its bounded own HELLO protocol retry if needed; this evidence establishes **exactly one host-side command**, not an oscilloscope count of UART HELLO frames.

## Independent original Mac mini evidence custody (13/13 checks PASS)

| Original private file under `DEUS-MCP/macmini-workspace/automation/` | Size | SHA256 |
| --- | ---: | --- |
| `command-queue/esp010-v9-p3h-one-hello-no-crc-1111-a1.json` | 455B | `B2267BB904F011A4BE365EFF7FBA6AC3F5A636CFEDF0BC5CEC75D7C2610F9DAD` |
| `command-results/esp010-v9-p3h-one-hello-no-crc-1111-a1.json` | 1506B | `863BB6A856B189A09F8B818F01E91BEAD751581D71560AADC0F23F3E5E235B6` |
| `host-logs/esp010-v9-p3h-one-hello-no-crc-1111-a1.log` | 809B | `2A5E05EE9E2A97A1E775A19A3E71F2FDA844DA5AE16107A52ADC863E49F580EFE` |
| `host-work/esp010-v9-cli-candidate-20261109/p3h_normal9_one_fresh_hello_no_crc_20261111.sh` | 3603B | `3164A558ECB519FADFCD8D7E7C8FEC4D2F25D2EEFD96BC7F4DAD977A53E017E0` |
| `host-work/esp010-v9-cli-candidate-20261109/.p3h_20261111_one_hello_attempted` | 67B | `1A88FE3EF3E5870A3383BE43D90E828944E2D7C16DC79B56A2924BB9003FC8A7` |

Independent checks: exact file presence, queue/receipt SHA binding, genuine Mac worker completed exit0, full log byte-identical to typed receipt and matching bytes, current script hash, original-once marker and nonce, exactly one host `node start` syntactic call, before-state expected, exactly one `NODE_HELLO_QUEUED`, **both** after-status `READY=1/PENDING=0`, old `JOBS_VERIFIED=1/JOBS_BAD_CRC=0/LAST_CRC32=0x7CEEDB1A` unchanged, WDOG active/IWDG reset0 and stacks PASS, and no prohibited operation in the audited script/log: **13/13 PASS**.

| Observation | P3-G before | P3-H after 350ms | P3-H after an additional 1.4s |
| --- | --- | --- | --- |
| NodeLink READY / PENDING | 0 / 0 | **1 / 0** | **1 / 0** |
| Job verified / bad CRC | 1 / 0 | 1 / 0 | 1 / 0 |
| LAST_CRC32 | Historical `0x7CEEDB1A` | Unchanged | Unchanged |
| VALID (last job) | 0 | 0 | 0 |

`VALID=0` **does not negate successful HELLO**; it refers to the last CRC job result, which was explicitly not rerun. This proof establishes that STM32 and ESP can still negotiate a live DNL session. It does **not** identify why the previous session became not ready, guarantee a 30-minute future session or attest new Wi-Fi/registry40 code (still uninstalled). A further HELLO/CRC retry is not indicated.

## Next engineering boundary

The live normal9/ESP app v2 pair is suitable to preserve as the known-good baseline. Existing Windows native ARM V2 Wi-Fi source-only BIN 52,052B SHA `6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73` remains uninstalled. Its exact 84 frozen sources were independently rehashed against current Git working tree and original stage on **2026-11-11: 84/84 current and stage size+SHA PASS** using source-index SHA `D892610A4B842FE84B17C68DA2546BD29C5698B66B9B76D408BB430D93CD1BC5`. This demonstrates source identity, not signed release or runtime PSP safety.

**Important physical update risk:** `bootloader/bootloader.c` first scans authenticated metadata and if no explicit backup-token update request exists while `committed_version!=0`, directly executes `handoff_app()`. A valid-signature, hash-matching but nonfunctional normal10 can therefore prevent USB Management `ENTER_BOOTLOADER` although rescue11 is properly signed. **Rescue11+normal12 packets alone do not make that case recoverable:** separately verify a real physical out-of-band ST-LINK/SWD/recovery-entry path and exact backup/restore under a distinct owner scope before normal10 Flash. Physical SWD presence is documented in a dated topology, **not freshly attested here**. Any Windows private signing, new protected package, bootloader/Flash operation or ESP Wi-Fi RF requires a separate explicit scoped owner GO. `Gate4=NOT_AUTHORIZED`, `ESP01-010=PARTIAL`.

**No Git push; no protected firmware/keys, ELF/BIN or original Mac Evidence bytes copied into Git.**
