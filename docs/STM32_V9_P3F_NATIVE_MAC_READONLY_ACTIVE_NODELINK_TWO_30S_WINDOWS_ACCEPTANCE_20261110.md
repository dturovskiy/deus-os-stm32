# Deus OS — P3-F restored-Mac SSHFS / installed normal9 active Node Link stability (read-only)

**Date:** 2026-11-10, native samples 15:25:08–15:26:49 UTC (**16:25:08–16:26:49 Europe/Paris**). **Owner:** reported manual WSL SSHFS restore `MAC_WORKSPACE_RESTORED=PASS` and separately confirmed *OLED currently visually normal*. Owner's preceding instruction authorized a longer **READ-ONLY** observation of existing P3-E Node Link; it did **not** authorize replaying HELLO/CRC, altering ESP/CH340, resetting STM32, Flash, GPIO, permissions, or Git publication.

**Decision:** `P3F_PHYSICAL_NORMAL9_ACTIVE_LINK_READONLY_2x30S=ACCEPTED_SCOPED`, two distinct **30-second monitored windows** across **100,922 ms** actual STM32 tick from first to fourth sample. **Not** a 60-second uninterrupted recording: there was a `40,000 ms` tick interval between the end of A and start of B with **no Node polling**. Four endpoint samples all showed `NODE_READY=1`, `NODE_PENDING=0`, `JOBS_VERIFIED=1`, `JOBS_BAD_CRC=0`, `LAST_CRC32=0x7CEEDB1A`, `VALID=1`, unchanged since accepted P3-E morning one-HELLO/CRC event. All four watchdog health snapshots showed `WDOG_ACTIVE=1`, `IWDG_RESET=0`, `RESET_FLAGS=0x14000000`; tick and WDOG reload counters increased monotonically across **all four** snapshots, including interjob gap. Both native jobs `completed/exit_code=0/timed_out=false`; exact native original queue/result/full logs independent SHA **6/6** + request binding/full bytewise log/correct source+behavior PASS. No Node HELLO/CRC or any mutating action during P3-F.

**OLED evidence:** owner's own contemporaneous statement “Олед зараз працює нормально візуально”, **`OLED_VISUAL=OWNER_ATTESTED_PASS_SCOPED`**. Do not confuse it with remotely attested oscilloscope/I²C bus transactions, power, or visual pixel-frame capture. There was **no remote OLED image read or I²C probe**.

## Restoration, trust and native pre-execution limits

- Earlier same-day DEUS MCP `DEUS-MCP/macmini-workspace/` was an empty WSL mountpoint. Owner personally used existing WSL `sshfs macmini:/home/deus/deus-workspace ...` source and got `MAC_WORKSPACE_RESTORED=PASS`; independent new DEUS MCP `stat_many` then read original on-host P3-E receipt SHA `983629C47782158CC63F757F72083D7600AFF26513C18A691DB4A201DCAEFCD3`, private P3-E markers SHA `6D8764D79857F0A60CCFE5E81D4F6CF0790B0676B575DE247F3860744571C820` and `2FBAD2E69354E0BE5D7C578427F92F218F4DEA7A2144CC7380EFE1D1D32445D7`, isolated normal9 v9 CLI DLL SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`. **SSHFS mount restored was verified as actually backed by Mac data**; original immutable evidence retained.
- Attempt `esp010-v9-p3f-active-link-preflight-ro-1110-a1` was **`status=error` pre-execution**, worker `ValueError: Command must be a nonempty string <=1000 characters`, wall0, original error receipt SHA `7B332544285C04D984B6E75A8FD496D7C51B6FE05ADF473BBFE55AB1FB309400`; no USB open/host shell execution.
- Original 60s two-sleep file `p3f_v9_active_link_60s_readonly_after_mount_restore_20261110.sh` SHA `BD6EF9EC8F5DF10B24F7BD7CC1DBD7534111836FD3F00E789E57BE550F1FF8C1` was source-pinned and native `bash -n` checked (`P3F_NATIVE_MAC_SYNTAX_SOURCE_LOCK=PASS_NO_USB_OPEN`, receipt SHA `0BDA4FFCE355D813DCBC9C8BF8D8158FAE768BB5C164203421E6B4C0DAD31BFF`), **never executed on USB**: physical request `esp010-v9-p3f-node-active-readonly-60s-1110-a1` rejected **before execution** with `ValueError: timeout_seconds must be an integer 1..60` (request 145; original error receipt SHA `941FCB949AF88BE56C2719786027DC70E63C61BD886A3CD00A0C96FD3182581D`, wall0). Original controlled error receipts were not overwritten or mislabeled as hardware errors.
- Accepted workaround **within read-only owner scope**: one separate Mac source `p3f_v9_readonly_active_link_30s_segment_20261110.sh` **4,417 B, SHA `883C94E9A6DFE48B332581042065846B7B10B73A2D697B4354C4A141653AE2D6`**. Executes native `bash -n` before actual `bash`; script pins original P3-E receipt and both consumed markers, exact physical `/sys/bus/usb/devices/1-8`, VID/PID `1209:000E`, `/dev/bus/usb/001/101` owner `deus:deus:600`, exactly runtime1/bootloader0 `usb:001:8`, Protocol1 Service3 Registry39, SourceTree `edc5e08ae8dee2455994339c4c438e620c84df1a`. Reads only `health` and `node status` before/after 30s sleep, then `stacks` and `apps`. Both native jobs maximum `timeout_seconds=55`; job B additionally SHA-pins actual original job A receipt. Neither script invokes `node start`, `node crc`, ENTER, RESET, update, CH340 or ESP controls.

## Original physical Mac worker evidence

Under existing `DEUS-MCP/macmini-workspace/automation/`, original filenames share same stem and extension among `command-queue/<job_id>.json`, `command-results/<job_id>.json`, `host-logs/<job_id>.log`. Both workers `host=macmini`, `worker=deus-command-worker-v2`, `scope=host_bash_as_deus_no_os_sandbox`, `completed exit_code=0 timed_out=false`.

| Native job ID | Queue SHA256 | Receipt SHA256 | Full log SHA256 |
|---|---|---|---|
| `esp010-v9-p3f-readonly-segment-a30-1110-a1` | `0B5699866C5216FFABC4B55C3CAFA6ED87B07AFA9635B04C8C5114551108C6E3` | `234E4F6B6AFAFC4E1F416D63569955B6CE9722422F6B5E15E963B8F51B02E6DF` | `D1F51E0D6232019E5A281DE9497F36B29A659A18976D8AF379E3D8FF146A870A` |
| `esp010-v9-p3f-readonly-segment-b30-1110-a1` | `6C4CDBC86B4B93B63C7CAA7A4E9B16134D3BFD5E38295E27F0D6BDCB734CD1C7` | `96C626C856911AEC4CD61D04CECA1B380E9C05CF5C9DEEABFEB48F4918CF0E67` | `1C8FABD032E908CA5E207047C1F95D6D151BA1C88288F4C243FDEC26F341696B` |

Each full original log 2,622 B; typed receipt 3,386 B. Job A elapsed **31.809 s**, job B **31.805 s**. An SSHFS read of job B log briefly exposed only the initial 385B while worker was finishing; **later independent re-open** returned full 2622B and the exact SHA above, matching completed typed receipt byte-for-byte. All 6 original file SHAs, request hashes/jobs, worker identity, source digest, full-length logs and semantic assertions independently reread: **6/6 PASS**.

## Four fresh physical snapshots, cross-job deltas

| Snapshot | TICK | WDOG_RELOAD_COUNT | Node READY/PENDING/Verified/BadCRC/CRC |
|---|---|---|---|
| A at 16:25:08 | `0x036806FE` | `0x00115E0E` | `1/0/1/0/7CEEDB1A` |
| A at 16:25:39 | `0x03687DF9` | `0x001160A8` | unchanged |
| B at 16:26:18 | `0x03691A39` | `0x00116411` | unchanged |
| B at 16:26:49 | `0x03699138` | `0x001166AB` | unchanged |

Measured from actual `TICK`: A **+30,459ms**, inter-job period **+40,000ms** (no polling), B **+30,463ms**, first-to-last **+100,922ms**. WDOG reload deltas **+666 / +873 / +666**; first-to-last **+2,205**. No IWDG reset flag or unchanged `RESET_FLAGS` transitions. This is four bounded samples, not an oscilloscope trace nor proof of all intermediate events.

At both post-window stack checks: `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS`, all three canaries1, scheduler faults0. Stack high-water margins unchanged: MSP **1564B**, console **396B**, heartbeat **432B** (all >=256B bench policy). Applications `RuntimeAbi1 Registry2 Active=0x0001 Faults=0`, `system.home RUNNING`, `device.info STOPPED`. No CRC counter changed in this observation, so **no new Node computation** was sent.

## Scope and next boundary

`P3F=ACCEPTED_SCOPED` for **normal9 physical active NodeLink status stability in two 30-second passive windows** and source/USB/watchdog/stacks/apps continuity over 100.922s total measured span. Earlier `P3E=ACCEPTED_SCOPED` for one real HELLO+CRC remains intact; old consumed HELLO+CRC markers not replayed. Owner confirms OLED visual operation, not electrical I²C timing proof. More intensive active traffic, node absence/power transitions, error injection, CH340, reset/power-cycle/bootloader floor9, ESP firmware work, and formal Gate4 remain separate, not automatically authorized. `ESP01-010=PARTIAL`, `GATE4=NOT_AUTHORIZED`. No Git add/commit/push.
