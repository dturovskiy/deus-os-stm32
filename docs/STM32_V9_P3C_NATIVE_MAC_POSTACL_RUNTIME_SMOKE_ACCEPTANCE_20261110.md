# Deus OS — STM32 P3-C real Mac mini post-v9 runtime read-only smoke acceptance

## Supplement — P3-D subsequent 20-second read-only runtime/optional-node continuity, 2026-11-10 02:12 Europe/Paris

The owner said to continue after P3-C. A separate **non-state-changing** actual Mac mini task `esp010-v9-p3d-idle20-node-readonly-1110-a1` ran native unprivileged `deus` `completed/exit_code=0/timed_out=false`, wall 21.712 seconds, and pinned accepted P3-C physical receipt, normal9 Host SHA and permanent P3-B one-shot marker. Exact source `p3d_v9_idle20_node_status_readonly_20261110.sh` 4160B SHA `75365E5B74F8ED8F9FDF696DCE40E87B43F1210425BA1E83A2DC202F3F93B3BC`. Fresh before/after USB identity: `1209:000E`, sysfs `1-8`, bus1, devnum101, node `/dev/bus/usb/001/101 deus:deus:600`, SourceTree `edc5e08ae8dee2455994339c4c438e620c84df1a`. Read-only `health` / `node status` snapshots separated by 20s; read-only `stacks` / `apps` at end.

| Exact original Mac file | SHA256 | Bytes |
|---|---|---:|
| `command-queue/esp010-v9-p3d-idle20-node-readonly-1110-a1.json` | `96FE28852C360E6D501FBC4254AEC22084113FA9EFD3F3FA4D9BA217DF74FDC2` | 431 |
| `command-results/esp010-v9-p3d-idle20-node-readonly-1110-a1.json` | `5E9E3ED1A2A0AE96EB071F760B9E9D389E5A569F3777BB02F28664BACED22D20` | 3265 |
| `host-logs/esp010-v9-p3d-idle20-node-readonly-1110-a1.log` | `995BBD3C5D36AB439CA93461B6381FA3BF098F1719934A44FD1FFFFDE11170E2` | 2467 |

**Independent read-only original audit:** original SHA3/3; source request SHA in receipt and original job ID agree, true host and worker, stdout full log 2467 B exact receipt match, exit0/no timeout, no unexpected state-changing actions and all health assertions PASS. **Measured:** TICK `0x005AF6F8 → 0x005B46D7` (+20,447 ms), WDOG reload `0x00008BC3 → 0x00008C42` (+127), WDOG ACTIVE1, IWDG_RESET0, reset flags `0x14000000` unchanged. Node `READY0 PENDING0 JOBS_VERIFIED0 JOBS_BAD_CRC0 VALID0` both samples, no sessions submitted and no new jobs counted. MSP/console/heartbeat margins 1564/488/432 B, all relevant canaries1, `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS`. Apps `RuntimeAbi=1 Registry=2 Active=0x0001 Faults=0`: `system.home` RUNNING, `device.info` STOPPED. This supports **stable idle optional-node state**, not proof of physical ESP connectivity/absence, and not a new ESP HELLO+CRC E2E acceptance. **P3-D idle/WDOG/Node status accepted scoped**, P3-C source/USB/stack smoke unchanged.

**Still open:** physical OLED/I²C visual test, actual STM32↔ESP active link on normal9, power-cycle persistence, direct postcommit floor9 bootloader INFO. No ENTER/RESET/Flash/recovery/CH340/control, no Node start or CRC, no Git commit/push.

**Date:** 2026-11-10 02:03 Europe/Paris (Mac native receipt `2026-11-10T01:03:01+00:00`). **Owner:** explicitly continued earlier P3-C READ-ONLY smoke; owner personally executed only the tightly scoped USB-node permission repair between read-only P3-C discovery and this smoke, as shown in terminal output in conversation. **Classification:** BENCH physical scoped evidence; no formal Gate4 admission.

**Verdict:** `P3_C_REAL_MAC_POSTINSTALL_NORMAL_V9_RUNTIME_SMOKE=PASS_SCOPED`; `P3_B_NORMAL_V9_ONE_COMMIT=PASS_SCOPED` unchanged. Genuine native Mac read-only CLI returned `completed/exit0/timed_out=false`; verified physical STM32 `1209:000E`, exact original v4/normal9 `SourceTree=edc5e08ae8dee2455994339c4c438e620c84df1a`, Management Protocol1 Service3 Registry39, `PONG`, watchdog active/no IWDG reset, safe MSP/PSP cooperative scheduler and three stack canaries, final `STACKS_STATUS=PASS`. **Do not claim a byte-for-byte Flash readback or exhaustive v4 hardware acceptance:** OLED/I2C on-board behavior, ESP actual bridge/CH340, physical power-cycle persistence, 20s watchdog soak and postcommit bootloader floor9 have NOT been newly tested by this run. `GATE4=NOT_AUTHORIZED`.

## Read-only postcommit recovery from previous P3-C ACL blocker

1. Earlier P3-C genuine Mac read-only discovery `esp010-v9-p3c-discovery-ro-1110-a1`, receipt SHA `E6F690CCB4A9C300C46DCF3D93187ECC63356CB85648ABD68B3958F7F2707987`, physically saw `/sys/bus/usb/devices/1-8 1209:000E`, devnum101, node `/dev/bus/usb/001/101 root:root:664`; did not open USB nor modify ACL. Historic report `docs/STM32_V9_P3C_POSTCOMMIT_USB_READONLY_ACCESS_BLOCKED_20261110.md`. The state was **protocol blocked**, not a firmware failure.
2. User-provided native Mac terminal output: exact prepared script SHA `A4071980E50B8AD268712A4794EC439FCE1476DD79D5233E31129E79E8089662` checked OK, sudo authentication local only, original owner script output `P3C_USB_PERMISSION_ONLY_GRANTED=PASS NODE=/dev/bus/usb/001/101 ID=1209:000E OWNER=deus GROUP=deus MODE=600` and `P3C_NO_RESET_NO_FLASH_NO_CH340_NO_ESP_ACTION=YES`. **This is OWNER-REPORTED manual native execution, not a DEUS MCP-run sudo operation**. No password/key captured or shared.
3. Subsequently independently read from genuine unprivileged Mac worker, new node ACL `deus:deus:600`, exactly expected sysfs VID/PID/bus/devnum and one runtime, no bootloader. No permission alteration during P3-C smoke.

## Exact original native Mac worker evidence

One distinct job `esp010-v9-p3c-postacl-runtime-ro-1110-a1`, `host=macmini`, `worker=deus-command-worker-v2`, `scope=host_bash_as_deus_no_os_sandbox`, `completed exit_code=0 timed_out=false`, ~1.018 sec. No ENTER, RESET, UPDATE, rescue, sudo, chown/chmod, CH340 or ESP operations. Exact private source-locked script `p3c_v9_runtime_smoke_readonly_after_owner_acl_20261110.sh` 2855 B SHA `6C827D968CD980C55FC9CAD0F2C8E6DD84053DF54913B9887C17126DFACA58AD`; verified unchanged isolated Host DLL SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`, v9 Host C# SHA `ACC3D74653FDEFFD1E6F9585A5AAF4A669A733F86C7E4FE1F44D4C3B49F84DF1`, previous physical P3-B original receipt and irreversible one-shot marker SHAs. Program uses only `bench-fw devices` plus `--device usb:001:8 info`, `ping`, `health`, `stacks`.

Original existing native files in `DEUS-MCP/macmini-workspace/automation`:

| Original | SHA-256 | Bytes |
|---|---|---:|
| `command-queue/esp010-v9-p3c-postacl-runtime-ro-1110-a1.json` | `D69675BB3AF9CEEBA7CFFB9B7A8D2F33ACC7CC01FEF210889EAAAA4054EFE0BC` | 442 |
| `command-results/esp010-v9-p3c-postacl-runtime-ro-1110-a1.json` | `977CAFAA268AB41C429F9647D9687AC09074AA0F45887F6CCA05D7DD165BCCB3` | 2889 |
| `host-logs/esp010-v9-p3c-postacl-runtime-ro-1110-a1.log` | `CBF48BFB18F5F723243B4962D8F64C242D92D65636EAD8D4EFF9CF7465518FFB` | 2099 |

**Independent read-only DEUS MCP original files verification PASS:** three original SHA-256 digests 3/3, original `job_id` and `request_sha256` bound to queue, real Mac worker identity/exit/no timeout, full original stdout/stderr log exactly matches typed receipt and log byte count, semantic source+protocol+health+stack assertions, and no physical mutation.

## Real installed-v9 read-only results

```text
P3C_HOST_SOURCE_AND_P3B_COMMIT_PINS=PASS
P3C_PINNED_RUNTIME_NODE=PASS_1209_000E_USB_001_8_NODE101_OWNER_DEUS_MODE600
DEVICES=runtime 000C/000E COUNT=1
DEVICE=runtime 000C/000E PORT=usb:001:8 Deus OS Management-Only
DEVICES=bootloader 000D COUNT=0
Protocol=1 Service=3 Registry=39
OS=DEUS_OS Platform=STM32F103C8 Arch=ARMV7M
SourceTree=edc5e08ae8dee2455994339c4c438e620c84df1a
Capabilities=0x0000007F UnitIdKind=0
P3C_V4_SIGNED_V9_RUNTIME_SOURCE_IDENTITY=PASS
PONG
HEALTH TICK=0x0052A9BD PC13=0x00000001 WDOG_ACTIVE=0x00000001 WDOG_RELOAD_COUNT=0x00007EFC RESET_FLAGS=0x14000000 IWDG_RESET=0x00000000
MSP_STACK_OK
SCHED_PROD_OK
STACKS_STATUS=PASS
P3C_RUNTIME_V9_SOURCE_PONG_WDOG_STACKS=PASS_SCOPED
P3C_READ_ONLY_NO_RESET_NO_FLASH_NO_RESCUE_NO_CH340_NO_ESP_ACTION=YES
```

Full original `host-logs` retains every raw individual scheduler/stack field; no replacement or fabricated receipt. Specific independently parsed margins: **MSP 1564 B**, console task **488 B**, heartbeat task **432 B**; all respective canaries `1`, scheduler active/cooperative, heartbeat started and fault0, `SCHED_PROD_FAULT=0`, no IWDG reset. Trusted source provenance: accepted unsigned optimized v4 Windows native original 49,020 B image SHA `6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E`, 75-file index full SHA `EDC5E08AE8DEE2455994339C4C438E620C84DF1A5975F501CCD431DC9AD5BBC9`, exactly its 40-hex prefix in live SourceTree, paired owner-signed normal v9 protected package 49,100 B SHA `DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B` successfully committed in earlier P3-B. This demonstrates running firmware source identity and health, not on-chip full Flash dump or cryptographic attestation of running bytes.

## Decision and remaining scope

**P3-C read-only postinstall runtime smoke accepted scoped.** Original P3-C blocked state is historical (reason removed by owner's separate one-node permission operation). No more USB privilege change, no reflash, no ENTER, no RESET or rescue required for this acceptance. Next optional BENCH milestone: targeted v4 feature-level physical tests (OLED, I2C, ESP/Node bridge) and/or deliberate observation interval, each with suitable separate intent. Antirollback floor9 postcommit can only be independently re-read in appropriate bootloader mode and is NOT implied by `PONG`. Formal Gate4 remains NOT_AUTHORIZED. No Git stage/commit/push.
