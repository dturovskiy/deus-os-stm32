# Deus OS — 2026-11-08 genuine Mac mini USB Management read-only signed STM32 v8 baseline

**Result:** \`ACCEPTED_SCOPED_PHYSICAL_READONLY_SIGNED_V8_BASELINE\`. Original **two Mac mini worker receipts, original queue requests and full logs all 6/6 SHA-256 independently verified**, plus exact Host CLI and original on-host diagnostic script SHA. **NOT** physical qualification of unsigned source-only v4. No code or firmware change; no BOOT/EN/RESET, CH340, STM32 Flash/update, signing or Git add/commit/push.

## 1. Permission and authority

On 2026-11-08 the owner explicitly authorized a **non-invasive USB Management check of the installed signed STM32 v8 via Mac mini**; not signing/flashing/entering bootloader/reset or controlling ESP CH340. No standing permission to extend to future hardware changes or move Gate4. Native Mac mini is the physical USB owner; read-only hardware access was performed via its owner-started \`deus-command-worker-v2 --enable-host-shell\` and the pre-existing reviewed native \`deus-cp.dll\`. Job requests went through the already established DEUS MCP/SSHFS-backed shared workspace. Each was one small JSON \`host_shell\` job on real \`macmini\` as unprivileged \`deus\` with a 55s timeout; no sudo, tty, CH340 port, bootloader requests, nor destructive methods.

The original Host v8 CLI \`DEUS-MCP/macmini-workspace/automation/host-work/esp010-v8-cli-source-20261107/host/src/DeusOs.Control.Cli/bin/Release/net10.0/deus-cp.dll\`: **53,248 B**, SHA256 **\`DE842365C0AB7F11FCF48E7115922D4A2A86B50F2082576EF6BDD81DE7DC4936\`**; checked on physical Mac path via mounted read-only hash. Reused the previously native accepted **non-destructive phase C** \`phase_c_v8_runtime_smoke.sh\`, **1,662 B** SHA256 **\`784B7F7923633A7A52BC0958176EF284A4CDE8A1A867740E126A07B20DD85C41\`**. This script makes \`bench-fw devices\`, \`info\`, \`ping\`, \`health\`, \`stacks\` on STM32 Management; no \`ENTER\`, \`UPDATE\` or ESP interface. The follow-up used \`health\` and **\`node status\`**, but no \`node start\` or CRC submission.

## 2. Original Mac worker source, data and custody

Original physical job #1 **\`esp010-v8-readonly-regression-1108-a1\`**, observed 2026-11-08 21:50:33 UTC = **22:50:33 Europe/Paris**:

| Artifact | Exact file under \`DEUS-MCP/macmini-workspace/automation/\` | Whole SHA-256 |
|---|---|---|
| Original request | \`command-queue/esp010-v8-readonly-regression-1108-a1.json\` (704 B) | \`9BE4E9F0EAF5DDB407923B7D8CE3AA283FFF23B30FF47F3099E7628C52B54FBB\` |
| Original typed receipt | \`command-results/esp010-v8-readonly-regression-1108-a1.json\` (2,684 B) | \`B4F17BB0E8CEA8960DE32B17140BD3E0A4B8EDED42E5C5653D66419C659DD048\` |
| Full actual original stdout/stderr | \`host-logs/esp010-v8-readonly-regression-1108-a1.log\` (1,905 B) | \`891AC08777FD36DFD8D7912163AADD5B1D0888A40CABBB58C182829CC39A425F\` |

Worker reported \`host=macmini\`, \`status=completed\`, \`exit_code=0\`, \`timed_out=false\`, wall 1.62s, user \`deus / unsandboxed Linux host\`, exact SHA-binding request↔receipt. Preflight proved fresh \`/sys/bus/usb/devices/1-8\`, \`1209:000e\`, bus1, actual character device \`/dev/bus/usb/001/098\`, \`deus:deus:600\` existing ACL. **No permission changes** or reconnect.

Original physical job #2 **\`esp010-v8-readonly-idle20-1108-a1\`**, observed 2026-11-08 21:51:53 UTC = **22:51:53 Europe/Paris**, ran ~20.86s:

| Artifact | Exact file under \`DEUS-MCP/macmini-workspace/automation/\` | Whole SHA-256 |
|---|---|---|
| Original request | \`command-queue/esp010-v8-readonly-idle20-1108-a1.json\` (937 B) | \`55587493E67D32F1FB237E31098152D3B8F7A914522EF11346AE09DF81A8929B\` |
| Original typed receipt | \`command-results/esp010-v8-readonly-idle20-1108-a1.json\` (1,333 B) | \`773651DA31404681A4249BD0CDBB619F1ED9FD01D31B5CB9911D0B9F2B85E749\` |
| Full actual original stdout/stderr | \`host-logs/esp010-v8-readonly-idle20-1108-a1.log\` (647 B) | \`C1ECBD0A30024A2425A661E656C52304B2F0C51554B2E378A8397A86EE055FD8\` |

Worker \`status=completed\`, \`exit_code=0\`, \`timed_out=false\`, 20.861s, user \`deus / unsandboxed Linux host\`. Direct commands were \`health\` then \`node status\`, 20s non-invasive sleep, same commands again, using exact pinned current Host v8 CLI SHA and \`1209:000e\` port preflight; **NO new ESP NodeLink job was sent**.

**Independent offline original evidence audit:** \`Tools/scripts/bench/audit_original_mac_v8_readonly_live_20261108.py\`, task **\`task_1794174849109_5b01bed15062b930\` exit0 PASS**. The auditor independently reopened six *original* files and verified each whole SHA, matching request_id/job_id/request SHA, typed worker/host/exit/timeout, exact full log **byte-for-byte** vs worker stdout tail (CRLF preserved), exact source and physical port preconditions, positive conditions and watchdog/stack/node metrics. Original data was neither reconstructed nor overwritten. First local auditor attempt failed solely from an incorrect workspace sibling path; fixed only this local auditor; original device jobs were **NOT retried**.

## 3. Real installed signed-v8 Management observations — PASS

- **Runtime-only USB:** \`DEVICES=runtime 000C/000E COUNT=1\`, \`DEVICE=runtime ... PORT=usb:001:8 Deus OS Management-Only\`, bootloader \`1209:000D COUNT=0\`. Installed STM32 remains v8; no bootloader activation.
- **Protocol/identity:** \`Protocol=1 Service=3 Registry=39\`, \`OS=DEUS_OS Platform=STM32F103C8 Arch=ARMV7M\`, **SourceTree \`2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926\`**, matching previously accepted original signed-v8 73-source index prefix; \`PONG\`.
- **Watchdog and reset:** \`WDOG_ACTIVE=1\`, \`IWDG_RESET=0\`, reset flags \`0x14000000\` unchanged. One actual \`HEALTH\` snapshot is not a watchdog expiry/recovery test. No watchdog reset deliberately triggered.
- **MSP stack:** reserved 2048 B, 64 B guard, capacity 1984 B, used high-water **428 B**, remaining **1556 B**, current used **104 B**; \`MSP_CANARY=1\`, \`MSP_STACK_OK\`.
- **Console PSP task:** capacity **1024 B**, high-water used **620 B**, margin **404 B**, canary intact. \`SCHED_PROD_MODE=COOPERATIVE\`, thread PSP in range, faults0; \`SCHED_PROD_OK\`.
- **Heartbeat PSP task:** capacity **512 B**, high-water used **80 B**, margin **432 B**, canary intact, heartbeat fault0; \`STACKS_STATUS=PASS\`.
- Both task margins exceed currently configured **256 B** minimum. However console margin narrowed from 536 B in 2026-11-07 original v8 acceptance to **404 B now**: +132 B accumulated stack high-water; this is within limits but validates why physical workload-specific high-water monitoring matters.

## 4. Twenty-second genuine runtime continuity

The two read-only health snapshots contain:
- \`TICK=0x057E1EEE → 0x057E6EB6\`, **+20,424 ms**.
- \`WDOG_RELOAD_COUNT=0x001867D5 → 0x00186854\`, **+127**.
- \`WDOG_ACTIVE=1\` and \`IWDG_RESET=0\` before and after; \`RESET_FLAGS=0x14000000\` unchanged.
- \`NODE_READY=0\`, \`NODE_PENDING=0\`, \`VALID=0\` before and after. Cumulative \`JOBS_VERIFIED=1\`, \`JOBS_BAD_CRC=0\`, \`LAST_CRC32=0xADF1C023\` **unchanged** over this observation window. The cumulative verified job/CRC reflects previous physical work; it is **not** a new live ESP HELLO/CRC test and does not prove an ESP absence/power state.
- No USB enum change or reset was requested. \`V8_20S_READONLY_USB_NO_NODE_START_NO_CH340_RESET_FLASH\`.

## 5. Acceptance boundary and next gate

**P1: READ-ONLY LIVE INSTALLED SIGNED V8 USB MANAGEMENT BASELINE — \`ACCEPTED_SCOPED\`.** Unique original Mac receipts under exact SHA, live USB target and protocol identity, task/MSP stack canaries, 20s watchdog+NodeLink idle, owner-supplied physical authorization and native Host release pins all satisfied.

**What is NOT attested:** This did not install or execute source-only **v4**; current signed normal v8 remains the MCU firmware. The independently accepted v4 Windows ARM resource candidate has \`os.bin=49020 B\`, SHA \`6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E\`, 4228 B Flash headroom and SRAM8480 B — **OFFLINE ARM only**, not physical runtime. No real v4 USB/OLED/display timing/USART2/RPC under load or full call-chain stack high-water evidence. No current ESP CH340 session was opened.

**P2/P3 still require separate, explicit owner authorization:** Source-locked v4-derived signing/metadata for correct bootloader version-floor (installed normal v8 at floor8; ordinary new image likely v9+), protected dual normal/rescue custody, and only then one separately authorized signed Flash/activation with known rollback/recovery procedure. Do not infer P2/P3 permission from this P1 read-only test. Existing STM32 normal v8/ESP v2 unchanged; no CH340, BOOT/EN, UART tty, ST-LINK, reset, Flash, signing, HMAC keys, Git stage/commit/push. \`Gate4=NOT_AUTHORIZED\`.

Canonical next readiness document: \`OS/docs/STM32_V4_FUNCTIONAL_REGRESSION_AND_PHYSICAL_ENTRY_PLAN_20261108.md\` (P0–P6). Original signed-v8 custody and version-floor caveat: \`OS/docs/ARTIFACT_CUSTODY_AND_RECOVERY.md\`; physical host topology: \`OS/docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md\`.
