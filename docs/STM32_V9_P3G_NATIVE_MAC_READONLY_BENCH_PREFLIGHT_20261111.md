# STM32 normal9 — P3-G native Mac mini one-shot passive bench preflight

**2026-11-11 16:58 UTC / 17:58 Europe/Paris. Result: `P3G_HARDWARE_READONLY_BASELINE_PARTIAL_NODE_NOT_READY`.** This is a real-device observation of **installed signed normal9**, not of the uninstalled optional Wi-Fi/registry40 ARM V2 candidate. Authorizing request: owner's immediate `виконуй` following a specifically scoped **one read-only Mac mini STM32 preflight** (USB, PSP, watchdog, passive NodeLink; no HELLO/CRC, RESET/Flash, CH340, ESP GPIO/power or Wi-Fi RF).

## Original native job and independently checked custody

One unique file-backed Mac mini worker job `esp010-v9-p3g-readonly-bench-once-1111-a1` with `host_shell` read-only script and 55-second timeout. The script was written on the Mac-mounted private workspace, source SHA-pinned in the job, syntax checked via `bash -n` before USB access, then run once. Worker `deus-command-worker-v2`, actual `host=macmini`, `action=host_shell`, `timed_out=false`, **`status=command_failed; exit_code=7`**, elapsed15.079s. The script's deliberate final Node READY/VALID assertion failed; this is **not** a USB enumeration error or crash.

All original files, under `DEUS-MCP/macmini-workspace/automation/`, independently reopened by DEUS MCP:

| Original file | Size | SHA-256 |
| --- | ---: | --- |
| `command-queue/esp010-v9-p3g-readonly-bench-once-1111-a1.json` | 464 B | `F319F96E78A08F0EFB5C69D24B77FC40FD890771D7AD818CD84C551FA6278BD3` |
| `command-results/esp010-v9-p3g-readonly-bench-once-1111-a1.json` | 3,517 B | `127640F3CA4BA289617CE1938A18131DB23F14517BAE462E77435FC514A7ABBE` |
| `host-logs/esp010-v9-p3g-readonly-bench-once-1111-a1.log` | 2,746 B | `C49122B2A1454F5997F04D506B0AB3AB06024DA1B8D1F15938E43A6D36162BF6` |
| Private `host-work/esp010-v9-cli-candidate-20261109/p3g_normal9_mac_readonly_bench_preflight_20261111.sh` | 4,373 B | `6ADF7573D2E4DF251940233687950A56192F506CAE965C9C33E7A61096AD7C92` |

Independent semantic reconciliation **11/11 checks passed**: typed queue hash equals original queue SHA; correct job/worker; correct exit7/no timeout; **full raw log byte-identical** to receipt tail and reported length; actual source SHA; source contains only native management reads `bench-fw devices`, `info`, `health`×2, `node status`×2, `stacks`, `apps`. No HELLO, CRC job, STM32 enter/reset/update, ESP/CH340 operation, GPIO or RF command.

## Fresh physical observations (two distinct samples, 12.459s apart)

- Real USB system path `/sys/bus/usb/devices/1-8`, `1209:000E`, character node `/dev/bus/usb/001/101` owner `deus:deus` mode600, one Management-only runtime and **zero** bootloader devices.
- CLI original SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`; current device `Protocol=1 Service=3 Registry=39`; signed normal9 `SourceTree=edc5e08ae8dee2455994339c4c438e620c84df1a` verified.
- Health TICK `0x08E3B777 → 0x08E3E822`, delta **+12,459 ms**; watchdog reload `0x001D9C90 → 0x001D9CDF`, delta **+79**; `WDOG_ACTIVE=1`, `IWDG_RESET=0`, unchanged `RESET_FLAGS=0x14000000`. Device alive with active watchdog in sampled window.
- MSP reserved2048B, guard64B, **margin1564B**; console PSP1024B, **margin396B**; heartbeat PSP512B, **margin432B**. All 3 canaries1, `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS`, `SCHED_PROD_FAULT=0`, heartbeat fault0. **These are real *normal9* margins, not proof of Wi-Fi V2 stack safety.**
- Apps `RuntimeAbi=1 Registry=2 Active=0x0001 Faults=0`; `system.home RUNNING`, `device.info STOPPED`.
- **Node A and B both:** `NODE_READY=0`, `NODE_PENDING=0`, `VALID=0`, `JOBS_VERIFIED=1`, `JOBS_BAD_CRC=0`, `LAST_CRC32=0x7CEEDB1A`. The existing verified counter/CRC refer to the historical one-time HELLO+CRC, not fresh link verification. Native script intentionally emitted `P3G_NODE_PASSIVE_STABLE=NOT_ACCEPTED` and exit7.

## Interpretation and next physical boundary

The prior owner-native **P3-F** two independent 30-second passive windows on 2026-11-10 both observed `READY=1`, `VALID=1`; P3-G now sees `READY=0`, `VALID=0` with no watchdog reset or CRC-error increment. **Cause is NOT ATTESTED.** In the firmware transport design, session timeouts can lead to a fail-closed state that clears readiness; a stopped/unpowered ESP, missing USART2 replies, a transient link condition, or an intentionally lost session cannot be distinguished by passive management reads alone. Current checked V2 source is not a substitute for on-device normal9 binary-level causal evidence.

Do not classify the board as dead; the primary STM32 normal9 runtime, USB, watchdog and task stacks **passed**. Do not classify ESP or NodeLink as physically healthy; the passive session is **not READY**. This scoped preflight concludes as `PARTIAL_NODE_NOT_READY`, **not full PASS**.

**Smallest next required proof:** under *new explicit owner physical permission*, **one fresh nonzero-nonce NodeLink HELLO on installed normal9** through known USB Management `node start` and one passive status/health confirmation. No CRC calculation job, repeat/automatic retry, Flash, RESET, CH340, ESP boot GPIO/power, RF, firmware signing or Wi-Fi scan in that next scope. If no READY response, stop and diagnose wiring/ESP ownership/power separately after approval. Until then, do not initiate a normal10 STM32 update simply to cure NodeLink READY0. A later signed normal10 requires independently version-floor-compatible recovery (valid committed normal10/floor10 will reject rescue10; plan rescue11 and normal12 route, or an equivalently proven policy), separate owner signing and Flash GO.

No Git push or firmware mutation in this operation. `ESP01-010=PARTIAL`; **Gate4 NOT_AUTHORIZED**. Global project authority: `docs/CURRENT_STATE.md`.
