# Deus OS — STM32 P3-B: real Mac mini signed normal v9 one-shot firmware update

**Date:** 2026-11-10 00:31–00:33 Europe/Paris. **Scope:** owner explicitly authorized exactly one real P3-B update of the SHA-pinned signed normal v9 after fresh bootloader INFO8/8. This did not authorize automatic retries, rescue v9, additional ENTER/RESET, CH340/ESP work, Git commit/push, or P3-C postboot operations.

**Verdict:** `P3_B_MAC_NATIVE_NORMAL_V9_ONE_UPDATE_COMMITTED=PASS_SCOPED`. Genuine Mac worker returned `completed`, `exit_code=0`, `timed_out=false`, with real `UPDATE_COMMITTED=Ok/Committed VERSION=9`. **P3-C postboot runtime/source/watchdog/USB checks NOT_RUN**, no fresh postcommit USB or bootloader INFO. `GATE4=NOT_AUTHORIZED`. The update result proves bootloader-reported successful commit; it must not be rephrased as an independently verified running-v9 health result.

## Frozen identities and device admission

- Exact Mac mini worker: host `macmini`, unprivileged `deus-command-worker-v2`, `host_shell` real native Linux; existing shared DEUS-MCP Mac worker queue. No sudo or privilege repair.
- Isolated v9 candidate `deus-cp.dll` 65,024 B SHA-256 `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`, unchanged; reviewed v9 C# source SHA `ACC3D74653FDEFFD1E6F9585A5AAF4A669A733F86C7E4FE1F44D4C3B49F84DF1`. Accepted installed/legacy v8 Host not replaced.
- **Only candidate sent:** `deus_os_000e_bridge_optimized_full_normal_v9_20261109.pkg`, signed 49,100 B, whole SHA-256 `DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B`, parsed image 49,020 B, firmware version 9. Exact private Mac copy mode `0600`, protected source original and same-store backup all SHA identical after update. Signed rescue9 SHA `4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928` not used.
- The P3-A original bootloader INFO receipt SHA `7424B777F9915C82E90DFF0D09642EE2EC1CE9E8AA5EAAAEE7AC32959110E83D` and its consumed ENTER marker SHA `DE03DF0C51BCDAEDF7CAC7FCE25B6BD2F8F8F122730F85B9D97CF50BF6422228` were required. No ENTER replay.
- Fresh physical pre-update discovery: sysfs `/sys/bus/usb/devices/1-8`, VID:PID `1209:000D`, port `usb:001:8`, node `/dev/bus/usb/001/100`, exactly one bootloader, zero runtime `000C/000E`. Fresh read-only `BOOT_INFO=PASS FLOOR=8 COMMITTED=8 STATE=RecoveryIdle`, `FLASH=NOT_STARTED`, before any attempt marker. The actual compiled C# updater separately obtained the same INFO immediately before UPDATE.

## Original Mac mini task evidence

Original genuine task files live under `DEUS-MCP/macmini-workspace/automation/` in `command-queue/`, `command-results/`, and `host-logs/`. The checksum values were independently reread with DEUS MCP after execution; the original Mac worker receipts/logs were inspected, not synthesized.

| Task | Original queue SHA256 | Original result SHA256 | Original full log SHA256 | Status |
|---|---|---|---|---|
| `esp010-v9-p3b-offline-preflight-1110-a1` | `A9DF35B52927D5F9792E5E5350AD8846D3C65BAC92E77139D176C73431A48F83` | `16FBD75A847642192C4D9DB79C74D2AE511F5AEFCF3968AF7CBAE5E25154F451` | `0CF16AF02A29C50FB149C72622AC48AAD3DDDB05161ED969FDC40DE8411D4FE8` | `completed / exit0`, native `bash -n`, genuine signed normal9 `check-normal` PASS, **NO USB** |
| `esp010-v9-p3b-normal-update-once-1110-a1` | `F0850926CDEAE14F1789E3C59F1C818D5C5328B807E93832142170F820A22A8A` | `FABE432FE9FDCE1FC5CFFB5FCBD6337003256FFDB6E411B2114BEC5A59D6E78A` | `327E91A15548C103C6ED95345B68CBF33A009AC44110591AD524A8382C469DD3` | `completed / exit0 / no timeout`, 2.572 s, **one physical signed normal9 update** |

Real physical output (verbatim decisive lines):

```text
P3B_FRESH_PHYSICAL_BOOT_INFO_8_8=PASS FLASH_NOT_STARTED=YES
P3B_UPDATE_ATTEMPT_MARKER_CREATED=YES; ONE_SIGNED_NORMAL9_ONLY=YES
BOOT_INFO=PASS FLOOR=8 COMMITTED=8 STATE=RecoveryIdle
UPDATE_BEGIN=YES PACKAGE_SHA256=DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B ONCE_ONLY=YES
UPDATE_COMMITTED=Ok/Committed VERSION=9 REBOOT_AND_POSTINSTALL_SMOKE_REQUIRED=YES
P3B_ONE_NORMAL9_UPDATE_COMMITTED=PASS; NO_RESET_NO_RESCUE_NO_CH340; P3C_SEPARATE
```

Exactly one `UPDATE_BEGIN` and one `UPDATE_COMMITTED` were present in the original full Mac log. One native `bench-fw-v9 update-normal-v9 --device usb:001:8 --package <exact signed normal9> --confirm-update yes` call was in the frozen script; it does not contain `enter-v9`, `recover-rescue-v9` or `sudo`.

## One-shot marker, integrity, restrictions and next step

- Exact executed Bash file `p3b_v9_normal_update_once_20261110.sh` 3,636 B SHA `7FC5349BE356D4143F90BF8E7FD1EF27091B5BE45222AF305E126DE3D85897F0`; the unique queued command also verified this hash and native `bash -n` before execution.
- Permanent private marker `.p3b_20261110_once_normal_update_attempted`, mode `0600`, SHA `60BADF3A044186A8A5ADFC4EB5AE4D389D89098D0E5142CBB258357776E21211`. **Never delete this marker, replay this task/script, or automatically resend UPDATE**, even if later state appears unclear.
- Native command stdout additionally preserved separately, SHA `7A406689A467DA0682160315DB93C63325657A2BC5AE7F12205357914D4DCB37`. Original full worker job SHA set reread **6/6 PASS**. No new signer, rescue9, CH340, ESP power/EN/BOOT, USB permission change, Git stage/commit/push.
- **Postcommit state limitation:** bootloader *reported* successful `Committed VERSION=9`. This report does **not** assert a new live `INFO floor9/committed9` readback or active `1209:000E` v9 runtime, because neither was queried after the write. No additional physical command follows within P3-B.
- **Next P3-C (separate owner GO):** read-only discover latest USB mode / node ownership at exact STM32 sysfs `1-8`; if 000E runtime is accessible, verify the new firmware's actual SourceTree, Management Service/Registry, PONG and watchdog/MSP/PSP/scheduler health. If not accessible, STOP and report observed state; do not modify ACL, ENTER, reset, recover or retry firmware without independent explicit permission. Rescue9 is a separate conditional scenario and not part of normal9 P3-B.
