# Deus OS — Development / Acceptance Environment Topology

**2026-11-13 00:17 Europe/Paris — NORMAL11 REAL STM32 INSTALLED + POSTBOOT PASS; NEXT ESP SPLIT ADAPTER ROM (LATEST):** Mac systemd worker actual P3-A once `ENTER=Ok/Resetting`, `1209:000D FLOOR=10 COMMITTED=10 RecoveryIdle` original receipt SHA `2F7947C5DE3EDF24770BD4CBAE3AEC80E3997BBD5687CC9257189A823C04320`. After first P3-B a1 source-only 63-digit SHA typo failed before any Flash/marker, a2 exactly one signed normal11 package SHA `986AF02EB6DF09B94A6C0542FFACCB3A6DAA65D94DD4F57C4A639A512FFB4AC8` UPDATE **Ok/Committed v11**, original receipt SHA `AE4C4CBC8F76378024F45C1854EC9769C7B43384FB218F9E3A35F6028F657BAB`, one marker, no repeat. STM32 self-reenumerated `1209:000E`, new SourceTree `6aed9c0b4804fd46f10af8310bd1ce14530caa75`, Registry40; USB receipt SHA `7F3A58BC0757879CB145D192851148C8FE95355E910012C05B4A4E085CCACE83`. Physical deep passive 12s smoke original SHA `7EEFBA8D99777DD30CDCE349467D88469E0A4D25EE84E201A3673A4AE52F072C` completed PASS, tick+12350/WDOG+78/IWDG_RESET0, MSP/console/heartbeat1412/452/440B, canaries and apps PASS, node READY0/PENDING0. **STM32 normal11 is INSTALLED, not pending**; signing and Flash must NOT be repeated. Rescue11/return12 protected preserved. NEXT: investigate current split BOOT/EN via CH340, bridge `rom-sync-staged` identity and one readonly ESP ROM SYNC before separate app-only ESP 0x10000 update. User's known PA2/PA3 UART data and CH340 TX/RX disconnected; do NOT invoke STM32-owned ROM reset variant or pretend CH340 UART data is connected. Gate4 NOT_AUTHORIZED; no Git commit/push.

**2026-11-12 22:55 UTC — BACKGROUND MAC MINI WORKER END-TO-END PASS (LATEST):** User Mac terminal observed `Linger=yes`, `systemctl --user is-enabled=enabled`, `is-active=active`, main PID230216, `COMMAND_WORKER_READY` in journal for `deus-command-worker-v2.service`. Independent new `system_info` job original request SHA `7E5B54029449688DBAC867A759840C5D5F82CC9D90ABA873A94793A4BD7A0FFB` -> original Mac receipt SHA `062485ACE9AACC3528F058CFBBC2FB4DE03802B7649661AED04EB5E5DBFA2C7A5`, `completed`, `host=macmini`, receipt matched SHA, ACL0600. Mac queue/result dirs0700. Worker now runs locally under systemd, not SSH terminal, automatically restarts; after Mac reboot not yet live-tested, WSL SSHFS remount independently needed after WSL reboot. Native Host11 .NET10.0.115 Release PASS (DLL `DC82A9C7179A7DC8818787D056A20D1F109B8E8E7F34F0106C9EAA3AB4E601EC`) and signed normal11 protected pkg private Mac SHA `986AF02EB6DF09B94A6C0542FFACCB3A6DAA65D94DD4F57C4A639A512FFB4AC8` accepted. No STM32/ESP reset/Flash or signing. Follow-up read-only STM32 normal10 USB checks before any device update.

**2026-11-12 — MACMINI FOREGROUND WORKER TO USER SYSTEMD (INSTALL PASS, LINGER+HANDOFF PENDING):** Native real Mac `esp016-1112-mount-restore-health-01` receipt SHA `C6E38CBD23D5BFA2143BB7238BB200A16C12053E89113FA905CC9904E6B027A2` completed. Original offline Mac normal11 Host receipt `esp016-normal11-native-host-offline-20261112-01` SHA `B000D7EF99059CF864EFADF390C14518AB4AF1A16124DC3A34A0A5907EB36594` completed exit0, SDK10.0.115 zero warning/error DLL SHA `DC82A9C7179A7DC8818787D056A20D1F109B8E8E7F34F0106C9EAA3AB4E601EC`, signed normal11 SHA matched, 6/6 offline negatives before USB, no hardware. Native Mac user-systemd preflight found user bus functional via `XDG_RUNTIME_DIR=/run/user/1000` and `DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus`, **Linger=no**, sudo-noninteractive unavailable, existing foreground worker still holding flock. Owner authorized background persistence. Native installer worker job original receipt SHA `9A4A81182A5171C83C4E69CFF30D3C1A563E36D3E23F6D61063A56ED9D6B9A6D` reports `systemctl --user enable` and nonblocking start PASS for `deus-command-worker-v2.service` installed in Mac home user unit path mode0600 exact SHA `FBDD5A154817E2CB90E6D0DD4B8B5A74AD0A7380FFE57F67AD0AAF45D0DD958C`. `Restart=always/5s`, logs journald, `NoNewPrivileges`, private tmp, queue/result dirs `0700`, one-worker flock; prior `--enable-host-shell` remains unsandboxed trusted `deus` permission only. **Still must owner run `sudo loginctl enable-linger deus`, verify `Linger=yes`, Ctrl+C manual foreground and verify `systemctl --user is-active` active + fresh queue receipt.** Runbook `docs/MACMINI_COMMAND_WORKER_SYSTEMD_20261112.md`. This does not automatically restore disconnected WSL SSHFS; no STM32/ESP device operations.

**2026-11-12 — MAC MINI SSHFS RESTORED BY OWNER, INDEPENDENT MCP FUSE SOURCE AND MAC ORIGINALS PASS (LATEST):** Owner WSL printed `MAC_WORKSPACE_RESTORED=PASS` after exact trusted `sshfs` mount. DEUS MCP independent `/proc/self/mountinfo` task `task_1794522423099_f066207c12d9c738` shows `fuse.sshfs macmini:/home/deus/deus-workspace`, same original `command_worker.py` SHA `D2F9861122E6275BB4BE6EDC2D2EB8B182A48646D4E378D686E3E48DB4F70F71`, previous Mac `automation/command-queue`, `command-results`, historic v10 stage and receipts are visible. Previous missing-mount note is historical superseded. One fixed `system_info` queue and one separately bounded native Host offline build queue are present, but **neither has an original worker receipt**: Mac worker execution not established after remount. Source-only new private Mac stage `/home/deus/deus-workspace/automation/host-work/esp016-normal11-native-host-20261112` 0700 hosts signed normal11 `.pkg` 50,456B SHA `986AF02EB6DF09B94A6C0542FFACCB3A6DAA65D94DD4F57C4A639A512FFB4AC8` mode0600 and 76 verified Host sources compressed98,584B SHA `BFA4069CB28ECD4258C4AC6E5B1D54929B8AFEEE062076D8098056C56D4770B9`. Worker previously owner-opted `--enable-host-shell` needs to be active for queued offline build only. No queue requests touch STM32/ESP/USB/recovery or reset/Flash. Native Mac compile and hardware work NOT_ATTESTED.

**2026-11-12 — Mac mini SSHFS missing; verified read-only from connected DEUS MCP (LATEST):** Read-only managed Python task `task_1794521997317_c08a5f34aebcd132` examined actual `/proc/self/mountinfo`: `DEUS-MCP/macmini-workspace` exists as plain mode0755 local directory, **IS_MOUNTPOINT=false, FSTYPE=NONE**; expected remote worker queue is invisible because Mac SSHFS is unmounted. `DEUS-MCP/msi-workspace` separately remains `fuse.sshfs` mounted. SSHFS/fusermount3 binaries installed. This supersedes previous restored-2026-11-10 Mac connection for current connectivity, not proof the physical Mac worker or files are absent. Source accepted mount: WSL `sshfs macmini:/home/deus/deus-workspace ~/projects/DEUS-MCP/macmini-workspace -o reconnect,ServerAliveInterval=15` but remote SSH/owner credentials not tested; do not write to unmounted path, no task enqueue or physical USB/STM32/ESP actions. After owner reattaches, verify true `mountpoint` plus original queue/receipts before job.

> **Public-safe operational view:** Local operator paths, login identities and SSH aliases are placeholders. Substitute values only from separately authorized private operational configuration. This document does not authorize hardware actions; public protocol, ownership and pin-map invariants remain unchanged.

Status: **CANONICAL OPERATIONAL ENVIRONMENT / HARDWARE-OWNERSHIP REFERENCE**

**2026-11-12 02:05 — CANONICAL CURRENT STM32–ESP SIGNAL ROUTE / NO CH340 UART DATA:** At current owner-declared bench, **STM32 PA2 USART2_TX → ESP-01 RXD, STM32 PA3 USART2_RX ← ESP-01 TXD, shared GND.** ESP CH340 still handles power/control, but **its TX/RX are electrically detached from ESP UART**. This supersedes the historical 2026-11-04 *standalone ESP CH340 UART* description before later STM32 interconnection. Current normal10 Mac USB→STM32→ESP HELLO ACK+CRC `0x3A820F15` original worker SHA `FB782769D45D7A90B955AEF3B928BE7013BAF2CEE9E1724DF9A33A52B7420258` confirms usable USART2 data path at that observation; software alone does not measure physical continuity at later times. Mac CH340 sysfs1-7 existence does not imply CH340 TX/RX continuity to ESP. Thus a 2026-11-12 `esptool` CH340 `chip_id` no-RX attempt was WRONG_TRANSPORT; preserve real FAIL but do not infer ESP ROM failure or request rewire of known PA2/PA3. Installed STM32 USB ESP ROM bridge entry currently HARD_DENY `usb_bridge_can_enter() return0`, GPIO0/EN driver not linked: next step is STM32 bridge source engineering/qualification, not repeated CH340 UART probing or board reconnection.



**2026-11-10 16:27 — WSL SSHFS MAC MINI MOUNT RESTORED BY OWNER, VERIFIED BY DEUS MCP (LATEST):** Owner WSL ENVY observed `MAC_WORKSPACE_RESTORED=PASS` after reattaching existing `macmini:/home/deus/deus-workspace` mount. Live DEUS MCP subsequently read exact original Mac P3-E receipts/private markers and SHA-pinned Host DLL; two new **native** physical Mac read-only STM32 NodeLink monitor jobs A/B each completed exit0, source identity `1209:000E usb:001:8`, original SHA6/6. User reports attached OLED visually normal, categorized OWNER_ATTESTED, not I²C waveform. Earlier 13:48 Mac workspace empty/NOT_RUN state is historical. No automatic mount repair or changes to Mac ACL/CH340, ESP, MCU Flash/reset; bounded Mac host-shell timeout1..60s and max command1000 chars apply. See `docs/STM32_V9_P3F_NATIVE_MAC_READONLY_ACTIVE_NODELINK_TWO_30S_WINDOWS_ACCEPTANCE_20261110.md`.

**2026-11-10 ~13:48 — WSL→MAC MINI SSHFS MOUNT VISIBILITY LOST (LATEST):** Actual live DEUS MCP `root[0]:/DEUS-MCP/macmini-workspace` directory exists but `list_dir` returned no entries; `automation/` and known worker original receipts/Host DLL paths return missing. DEUS MCP server projects profile itself is running/serving WSL `/home/deus/projects`; this does NOT imply native Mac host or device is off, nor that original files were deleted. Historical proven manual mount source/target: owner WSL ENVY `sshfs macmini:/home/deus/deus-workspace ~/projects/DEUS-MCP/macmini-workspace -o reconnect,ServerAliveInterval=15` (coordination handoff 2026-10-28). No automatic remount or alternative device action was attempted. Owner may inspect WSL `mountpoint` and restore trusted mount; then independent folder/receipt SHA and fresh Mac physical identity are required. Until then `P3-F_ACTIVE_LINK_60S=BLOCKED_MAC_SHARE_NOT_RUN`, while OLED owner visually reports normal operation; hardware not accessed this time.

**2026-11-10 02:29 — MAC NORMAL9 STM32 ↔ ESP-01 USART2 ACTIVE E2E NOW VERIFIED SCOPED (LATEST):** Mac mini STM32 Management `1209:000E` sysfs1-8/devnum101 `/dev/bus/usb/001/101`, `deus:deus:600`. Source-pinned normal9 owner-authorized one `node start` and one CRC from Management via **Blue Pill A2 (PA2 USART2_TX) → ESP RXD; A3 (PA3 USART2_RX) ← ESP TXD; common GND**. Real ESP ACK ready1, one CRC `0x7CEEDB1A` verified1 BAD_CRC0 VALID1, watchdog/stacks PASS. CH340 supplies ESP VCC/control as historic bench wiring but its UART/permission/serial was **not accessed**, no GPIO power/BOOT/EN, no reset/Flash. Preserve initial post-HELLO native script text-CRLF `exit1` as a false assertion and separate ACK+CRC proof; exact original CRC receipt SHA `983629C47782158CC63F757F72083D7600AFF26513C18A691DB4A201DCAEFCD3`. Hardware wiring was previously owner-attested under v8; no fresh physical cable inspection asserted. Report `docs/STM32_V9_P3E_NATIVE_MAC_ONE_HELLO_ONE_CRC32_E2E_ACCEPTANCE_20261110.md`.

**2026-11-10 02:03 — AFTER OWNER MANUAL NATIVE MAC USB PERMISSION-ONLY OPERATION, P3-C NORMAL9 MANAGEMENT RUNTIME ALIVE (LATEST):** STM32 sysfs `/sys/bus/usb/devices/1-8`, physical device `1209:000E` bus1 devnum101 logical `usb:001:8` char node `/dev/bus/usb/001/101` now `deus:deus:600` (owner script output and new independent Mac worker actual stat), one runtime and zero 000D. Unprivileged worker successful USB info/PONG/health/stacks on signed normal9 optimized-v4 SourceTree `edc5e08ae8dee2455994339c4c438e620c84df1a`, WDOG+canaries+scheduler PASS, original Mac receipt SHA `977CAFAA268AB41C429F9647D9687AC09074AA0F45887F6CCA05D7DD165BCCB3`. Owner chmod/chown happened **only** manually, not by worker. No CH340 port1-7, ESP, RESET/Flash/recovery. Power cycle may reenumerate a new node/ACL; re-admit new USB identity before any future command, never assume node101 persists. Full `docs/STM32_V9_P3C_NATIVE_MAC_POSTACL_RUNTIME_SMOKE_ACCEPTANCE_20261110.md`.

**2026-11-10 01:46 — P3-C POSTCOMMIT REAL STM32 RUNTIME USB ENUMERATED; NEW NODE ROOT ACL BLOCKS LIBUSB (LATEST):** Native Mac mini worker read real sysfs `/sys/bus/usb/devices/1-8`: `1209:000E` bus1 DEVNUM101, node `/dev/bus/usb/001/101` `root:root:664`, not writable by `deus`; unlike last pre-P3B `1209:000D` node100. Original P3-C receipt SHA `E6F690CCB4A9C300C46DCF3D93187ECC63356CB85648ABD68B3958F7F2707987`, no USB open, no permission change/CH340/ESP/reset/Flash. **USB runtime profile detection only — installed v9 running/source/health protocol not authenticated**. A manual guarded helper for only node101 permission correction was staged & native syntax-checked but **not run**; node must remain exact or helper rejects. Details `docs/STM32_V9_P3C_POSTCOMMIT_USB_READONLY_ACCESS_BLOCKED_20261110.md`.

**2026-11-10 00:33 — MAC PHYSICAL STM32 NORMAL9 FLASH COMMIT; POSTBOOT ENUMERATION UNKNOWN (LATEST):** P3-B owner-approved exactly one signed normal9 update on native Mac mini unprivileged worker, pre-update genuine STM32 sysfs1-8 VID:PID `1209:000D` logical `usb:001:8` char node `/dev/bus/usb/001/100`, runtime0 bootloader1, INFO8/8 RecoveryIdle. After one update, original Mac `UPDATE_COMMITTED=Ok/Committed VERSION=9` receipt SHA `FABE432FE9FDCE1FC5CFFB5FCBD6337003256FFDB6E411B2114BEC5A59D6E78A`; marker0600 prevents auto resend, 6/6 original queue/receipt/log SHA reread. **Last PRE-update mode was bootloader; do not claim the current post-update mode is still 000D or already runtime 000E** without fresh discovery. No USB ACL changes, CH3401-7, ESP actions, external power or manual reset. Future P3-C needs distinct owner GO for read-only actual postboot discovery/health; no fresh hardware commands authorized by this record. Report `docs/STM32_V9_P3B_NATIVE_MAC_NORMAL9_ONCE_COMMIT_ACCEPTANCE_20261110.md`.

**2026-11-10 00:20 Europe/Paris — PHYSICAL MAC STM32 NOW IN BOOTLOADER 000D AFTER OWNER P3-A ONE ENTER/RESET (LATEST):** Original runtime `1209:000E` via Mac mini sysfs `1-8` / logical `usb:001:8`, node `/dev/bus/usb/001/098 deus:deus:600` read-only v8 SourceTree/service/PONG/WDOG+stacks PASS; one owner authorized v9-host source-locked `ENTER=Ok/Resetting` original receipt `DC901FC9329F6B3EC5AEB44BF2CA7244DE5F9F89E14BA03D5A222BF5448B9BA3`; private 0600 one-time marker prevents re-entry. Bootloader now genuine `1209:000D` on SAME physical USB 1-8 / port `usb:001:8`, **new** char device `/dev/bus/usb/001/100`, `root:plugdev:660`; unprivileged deus successfully opened without ACL/sudo change. Native `bench-fw-v9 info` original Mac receipt SHA `7424B777F9915C82E90DFF0D09642EE2EC1CE9E8AA5EAAAEE7AC32959110E83D`, `FLOOR=8 COMMITTED=8 RecoveryIdle`, `FLASH=NOT_STARTED`. Real bootloader runtime 000E count0, boot000D count1. This device is **currently in bootloader not executing signed-v8 runtime**, but signed normal v8 payload and floor8 metadata intact. Source locks and four Mac original receipts/logs exact SHA12/12 PASS `task_1794266534902_1e9d3d96a6fe5a68`. First preflight script CRLF FAIL did not reset; corrected preflight PASS. No CH340 at 1-7 touched, PA2/PA3 ESP link not used; no firmware erase/Flash, no permission mutation. **Do not re-run ENTER**. P3-B requires separate owner explicit GO and fresh INFO8/8; current bootloader may reenumerate independently; don't assume port device number forever. `docs/STM32_V9_P3A_NATIVE_MAC_ENTER_BOOT_INFO_ACCEPTANCE_20261110.md`.

**2026-11-09 23:35 — MAC MINI ISOLATED V9 HOST SOURCE-ONLY NATIVE BUILD (LATEST, NO DEVICE):** Actual Mac mini private stage `DEUS-MCP/macmini-workspace/automation/host-work/esp010-v9-cli-candidate-20261109/`, accepted old Mac v8 source original index 56/56 exact SHA retained, new compiled candidate `deus-cp.dll` 65024B SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048` SDK10 warnings0/errors0, original Mac receipt SHA `76890F130E205F12E750D55AB1A0A30B5570583BF94D1E24AF72CE282B029C01`. Native offline signed-v9 package check positives3/negatives12 original receipt SHA `68C5FCF33BE394A85CECF623AA23698533979029A7BBFAB92C4C014AE6A0BCE7`, actual C# policy480 combinations and signed package2/4 original receipt SHA `019767A4B51726559370462EF677051DA342D993EAD36407EC9FBD27A58879CF`, independent original Mac SHA audits PASS tasks `task_1794263476838_234a1efc11761968`, `task_1794263695991_b430b588d665d971`. Mac private testing copies of signed normal9/rescue9 have mode0600 and original Windows protected copies/Backups unchanged. **Original Mac signed-v8 CLI remains untouched; new candidate is not the installed wrapper.** No Mac USB/sysfs/libusb access, CH340, A2(PA2)/A3(PA3), reset, Flash or STM32 image changes occurred; physical v9 not attested. Next source plan only `docs/STM32_V9_P3_MAC_NATIVE_HOST_SOURCE_ONLY_READINESS_20261109.md`.

**2026-11-08 22:50–22:52 Europe/Paris fresh Mac mini USB STM32 Management READONLY revalidation:** Owner explicitly authorized only non-invasive STM32 signed-v8 runtime USB. Live Mac worker physical `host=macmini` job original receipt SHA `B4F17BB0E8CEA8960DE32B17140BD3E0A4B8EDED42E5C5653D66419C659DD048`: current `/sys/bus/usb/devices/1-8` ID `1209:000e`, BUS1 device `/dev/bus/usb/001/098`, existing ACL `deus:deus:600` (NOT changed), one Management runtime on `usb:001:8`, bootloader 000D count0, accepted signed-v8 SourceTree, PONG, WDOG/PSP/MSP PASS. Companion 20s passive job original receipt SHA `773651DA31404681A4249BD0CDBB619F1ED9FD01D31B5CB9911D0B9F2B85E749`: +20424ms ticks, +127 WDOG reload, no reset or new CRC faults. Independent exact original queue+receipts+logs SHA6/6 audit task `task_1794174849109_5b01bed15062b930` PASS. **Separate CH340 `1-7` neither discovered nor opened by task commands**; no tty, DTR/RTS, BOOT/EN/Flash/reset, hardware v4 not installed. P1 signed-v8 baseline ACCEPTED_SCOPED, source-only v4 still unsigned physical NOT_ATTESTED. Full `docs/STM32_V8_MACMINI_USB_MANAGEMENT_READONLY_BASELINE_20261108.md`.

**2026-11-07 22:38 — PHYSICAL LINK OWNERSHIP UPDATE (supersedes earlier “ESP only on CH340 UART” lines):** Owner reports ESP-01 data pins moved to Blue Pill **A2 (PA2, USART2_TX) → ESP RXD**, **A3 (PA3, USART2_RX) ← ESP TXD**, shared GND; ESP VCC 3.3V/EN/BOOT remain on CH340 adapter, but CH340 TX/RX are disconnected from ESP UART. Separate USB devices remain Mac mini STM32 Management `1-8` VID1209/PID000E (`usb:001:8`) and CH340 `1-7` VID1A86/PID7523, but all NEW link test commands targeted **STM32 Management only** and did not open CH340/issue DTR/RTS, BOOT, EN or reset. Real physical `node start` HELLO acknowledged READY1, one CRC32 `DEUS-V8-ESP` verified `0xADF1C023` with zero bad CRC, 20s watchdog/stack/scheduler health PASS; original Mac result SHA HELLO `993D7FF3B256358D7AEC3FC82FBC6B26CC3E3374C5C8A3023A62679706C7481D`, CRC `0A020A0DCFF541AC0C2E755E43E4F1668251D197EAAE2ACFC6B1BC49B437EF08`, health `ABF9ADAC125428A9A9092891F9B7DD032433BD2784C0518283C2272082DA3434`. This proves functional crossed-UART traffic but not independently measured rail current, 3.3V margins, boot-pin electrical levels or unpowered-ESP behavior. Do not connect both STM32 and CH340 TX to ESP RX. See `CURRENT_STATE.md` for the active status.

**2026-11-07 21:34 — Mac mini current STM32 signed-v8 USB permission failure RESOLVED, actual Management PASS:** Owner manually executed exact staged root helper and reported `USB_RUNTIME_000E_ACCESS_GRANTED ... NODE=/dev/bus/usb/001/098 ... OWNER=deus GROUP=deus MODE=600` with no Flash/reset. The unprivileged original Mac worker independently checked current sysfs `1-8` 1209:000E bus1, actual device node `deus:deus 600` and accessed the real device via libusb. Original receipt SHA `E8C0A84C79CBF1C9E073ABFEFA2A4441C3813EDAACB307617C50C56C14F1278D` exit0 proves runtime000E one and bootloader000D zero; SourceTree `2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926` matches signed-v8 source index, PONG, watchdog/stacks/scheduler PASS. Additional original 20sec no-node-session idle receipt SHA `A493866C961A7EB36DC3549BA3DAD7E401B9E8F309C4957F80D2253D5B21394C` exit0; independent source-bound auditor task `task_1794083722169_73e4dfb919ee368d` PASS. Earlier post-reenumeration root:root 664 / libusb -3 was genuine temporary HOST ACL failure; now repaired **ONLY for this USB node**, no global/permanent udev rule. No extra reset/Flash and separate CH340 ESP USB on `1-7` untouched; do not conflate with STM32 PA2/PA3 physical crosslink. Main report `Coordination/tasks/ESP01-010/results/ESP01_010__REAL_SIGNED_NORMAL_V8_ONCE_COMMITTED_USB_ACCESS_POSTBOOT_PENDING_20261107.md`.


**2026-11-07 21:16+ current live physical USB TOPOLOGY AFTER USER-AUTHORIZED SIGNED NORMAL V8 INSTALL:** Mac mini command worker `deus` owns native STM32 USB `/sys/bus/usb/devices/1-8`, vendor1209/product000e, `usb:001:8`, current device number **98** `/dev/bus/usb/001/098`; the independent ESP-01 CH340 remains on `1-7` vendor1a86/product7523, untouched. The Mac owner worker performed one STM32 `ENTER=Ok/Resetting` from previously live signed v7, actual bootloader `1209:000D floor7 committed7 RecoveryIdle`, then exactly one signed normal8 `UPDATE_COMMITTED=Ok/Committed VERSION=8` (original command receipt SHA `F23C842D017D53CCB9E3B851705706F314FDE54EA584A930E1214A11770A8081`), returning to runtime000E. Bootloader000D no longer enumerated. New STM32 000E node **root:root 0664**, so unprivileged uid1000 `deus` lacks write access and `libusb_open` fails `LIBUSB_ERROR_ACCESS(-3)`, genuine postboot smoke FAIL original SHA `B16928C3ECB44437F670590EFDBA84927211DBFF868D28C7BEED4EA4396B39F5`. Previously accepted v7 one-shot Windows owner operator solved the identical reenumerated-node symptom by interactive Ubuntu sudo `chown deus; chmod 600` on precise verified node; NO standing root authority in the current Mac `host_shell` worker (`stdin=DEVNULL`; explicit no sudo). A 1,293B `phase_d_v8_usb_permission_only.sh` SHA `9F705B262CF8400A9502A52FA1CEAB4157B7D1AFF093370F9E417237C4E87CA0` is staged with exact VID/PID/port/unique-node and owner-mode guards, owner **not executed**; human terminal `sudo bash /home/deus/deus-workspace/automation/host-work/esp010-v8-signed-private-20261107/phase_d_v8_usb_permission_only.sh` uses masked interactive OS password there only. Avoid tty/DTR/RTS/CH340 or USB unplug/power cycle, no repeated reset/Flash. After node permission-only fix, next original read-only Management `info/ping/health/stacks`. These are current observations; paragraphs below describing signed v7 as installed are historical. Scoped report `Coordination/tasks/ESP01-010/results/ESP01_010__REAL_SIGNED_NORMAL_V8_ONCE_COMMITTED_USB_ACCESS_POSTBOOT_PENDING_20261107.md`.


**2026-11-06 20:29 Europe/Paris latest installed STM32 on physical Mac mini:** Owner specifically approved and Windows-native `deus-run` completed one signed normal-v7 update, original Evidence SHA `3A4F7933503B6C4E8862A54DB5DCF0164F740B1BB9A915624A71755CF6C87984` independently validated. Original real bootloader `1209:000D` INFO before update was floor6/committed6/RecoveryIdle; one signed update returned `Ok/Committed VERSION=7`. Current independent live Management USB **`1209:000E usb:001:8`** runtime, zero bootloader, **Registry39** SourceTree `66ef68d9c6e72d23233a7d7369631a9c2b9f3b14`, ping, watchdog, MSP/PSP/heartbeat canaries, scheduler status PASS; physical Mac read-only original receipt SHA `F94AF344874E83290241E18BD048C612487B3E848E9F2227D6C0F1421ACD2F7E`. STM32 update authority is **consumed**; no further automatic reset, Flash or bootloader floor query by resetting. The separate standalone ESP-01 CH340 tty belongs to another transport and **was not touched**. Physical STM32↔ESP E2E and v7 OLED visual remain unverified. Preserve below 18:42 live-v6 lines as dated historical snapshots; follow `docs/CURRENT_STATE.md` for current installed firmware.


**2026-11-06 18:42 live STM32 Management USB (read-only) actually accessed:** Owner-controlled Mac mini worker used the USB libusb Host Core to open **only STM32 Management** `1209:000E usb:001:8`, read info/ping/health/apps/rpcinfo and SAFE `mspstat`/`schedprod`; all PASS with alive watchdog and canaries. No ESP CH340 UART opened, no tty/DTR/RTS/stty, no reset or firmware update. Linux `bench-fw devices` enumerated one runtime000E and **no bootloader000D**. Original signed normal-v7 file separately transferred byte-exact from ProtectedRecovery into private Mac staging (0700/0600); physical client binary staged but owner-installed legacy CLI not changed. Distinguish **allowed read-only STM32 USB Management telemetry** from physical USB-UART actions and reset/Flash ownership boundary below. Scope and original receipts: `Coordination/tasks/ESP01-010/results/ESP01_010__V7_REAL_V6_USB_READONLY_AND_HOST_UPDATER_NATIVE_PREP_20261106.md`.

**2026-11-06 Mac native CLI software qualification supplement:** Under Mac mini worker-owned isolated source child, 43/43 source files copied and SHA checked, new Linux .NET10.0.114 `dotnet restore` (explicit empty offline source) and `build -c Release --no-restore` PASS; `deus-cp.dll` 46,592B SHA `E8830FCF6BEC8D687D78B690C513DF668D45A0983E37D62826DD5BB5563FD155`, actual `--help` and `recovery contract` pass. Source Git, owner installed `deus-cp`, STM32 and ESP are not modified. No USB transport opened or hardware permission implied. See ESP01-010 2026-11-06 scoped report.

**Mac mini live host-only readback 2026-11-06:** Owner-started `command_worker.py --enable-host-shell` is active, original Bash receipt completed at Mac mini. Read-only `/sys/bus/usb/devices` returned STM32 `1209:000E` on `1-8` and separate CH340 `1A86:7523` on `1-7`, by-id UART symlink present. Installed Ubuntu legacy `deus-cp` wrapper and old cli-v6 directory exist; worker default PATH excludes user-local bin. Ubuntu SDK .NET 10.0.114 and NuGet cache include xUnit/Avalonia; **no full source checkout** in scoped Mac locations. No USB/UART port was opened; device management, reset, Flash and electrical STM32↔ESP link **not tested**. These observations do not revise the owner-defined no-hardware-action boundary below. Record: `Coordination/tasks/ESP01-010/results/ESP01_010__MACMINI_NATIVE_ENV_AND_V5_SOURCE_LOCKED_CALLBACK_SCOPE_20261106.md`.

**OWNER-DEFINED WORK BOUNDARY (2026-11-04; supersedes all prior suggestions of electrical or USB reconnection diagnostics):** Owner confirms the physical bench, independent ESP-01 Mac mini CH340 UART, cabling and power are correctly configured and unchanged. **Hardware ownership belongs exclusively to the owner; assistant scope is software/source/build/tests/evidence/documentation.** Do NOT request physical USB unplug/replug, 3V3/current measurements, EN/GPIO0/GPIO2 inspection or power diagnosis, and do NOT treat those as prerequisites or task blockers. A CH340 serial interface alone cannot measure ESP VCC/current. **Without a new explicit, specific owner request**, do NOT open/control hardware USB-UART, drive DTR/RTS/EN, change tty mode (`stty`/`hupcl`), reset or flash either device, change USB ownership or connect/disconnect devices. Prior reports of owner-approved bench experiments and the observed successful `-hupcl` test remain historical evidence only, **not standing authorization** to repeat them or introduce a permanent host policy. Do not change previously observed hardware state in response to this note. Prioritize software-only STM32 ESP01-010 tasks: static stack/call-path and IRQ analysis, timing/latency tests, trusted boot challenge design, build/source reproducibility and documentation. A physically connected STM32↔ESP E2E result remains NOT_ATTESTED, but the assistant must not demand hardware action as a prerequisite to continuing permitted software work.

**ESP01-013 MAC MINI CH340 `hupcl` ROOT-CAUSE CANDIDATE / PHYSICAL BENCH MITIGATION PASS (2026-11-04 23:43 Europe/Paris, owner GO):** Owner's unchanged standalone ESP-01 on its **own Mac mini CH340** (not STM32 UART) had repeated `NO_VALID_NODE_LINK_REPLY` after independent UART client/ROM hard-reset lifecycle. Exact ESP bootloader/partitions/app **on-chip Flash 3/3 digest PASS**, protected 1MiB stock backup exact, normal ROM 74880 boot `mode:(3,7)` / app at `0x10000` PASS. A new direct Linux `stty -F ... -a` observation showed **`hupcl` ON** on this verified by-id CH340; `HUPCL` deasserts modem-control lines on last close and is capable of disturbing ESP reset/boot wiring. Owner-authorized exact-host runtime-only `stty -F <ESP_CH340_BY_ID> -hupcl` applied, verified persisting across pyserial, **not a permanent OS setting**. Under `-hupcl` real ESP original Node Link **5/5 PASS** in separate client, fresh random challenge PASS, 25s idle then fresh challenge PASS, full separate 5/5 PASS, and **the previously failing `esptool chip_id --after hard_reset` followed by original 5/5 now PASS**. Final host receipt `DEUS-MCP/macmini-workspace/automation/command-results/esp013-final-accept-hupcl-usb-1104-w1.json` SHA `2CB3B40716EDDEC5156CD5461A2000A74ADCEDD0A34062A838D89B611E9E47E7`, exit0, **HUPCL_OFF + ESP 5/5 + separate installed STM32 signed-v6 Management info/ping/health PASS**, no Flash. Verified idempotent Mac mini local guarded reapplication script 986 B SHA `EFA5780EE10F25332ED5481DC3C5D3EC1E9DB9600BA9176FD7A14C8C2E5FA2DA`; original host receipt SHA `4C7FF7758B7923F91B337293C457DD13037ADBABB44647CC4D1E79A2D80A6F69`. Full chronological native Evidence, failures and pass comparisons in `Coordination/tasks/ESP01-013/results/ESP01_013__FULL_ROM_FLASH_BOOT_UART_DIAGNOSTIC_20261104.md` addendum; bounded rerun/rollback `Coordination/tasks/ESP01-013/artifacts/ESP01_013__MAC_CH340_RUNTIME_HUPCL_MITIGATION_RUNBOOK_20261104.md`. **Current standalone ESP Node Link = PHYSICALLY RESPONDING WITH `-hupcl`, BENCH MITIGATION ACCEPTED**; exact EN/GPIO0 waveform, 3V3 power margin, prolonged unattended uptime and persistence across USB replug NOT_ATTESTED. No blind reflash, no permanent udev/service, no STM32 USART2-v4 installation, no STM32↔ESP wiring/E2E. No Git commit/push.

**2026-11-04 physical bench supplement (ESP01-013 vs STM32 UART separation):** The **standalone ESP-01 node**, installed Nov3, is directly connected to **its own CH340 `1a86:7523` 115200/8N1 USB-UART on the Ubuntu Mac mini** (`/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0`, `/dev/ttyUSB0`) and is powered by that USB-UART adapter 3V3. Owner states this ESP wiring has **not changed** since the Nov3 accepted flash and physical UART tests. This CH340 does **NOT** imply any electrical connection to STM32 PA2/PA3 USART2; real STM32↔ESP two-node wiring and E2E remain NOT_ATTESTED. STM32 separately enumerates on Mac mini as signed-v6 USB Management-only `1209:000E`, read-only info/ping/health still PASS Nov4. The earlier Windows STM32 USART1 COM/CH340 topology in this document is a **different interface** and must never be conflated with this standalone Mac mini ESP node. The current ESP runtime intermittent UART-client-lifecycle diagnosis and recoverable controlled normal boot are in scoped `Coordination/tasks/ESP01-013/results/ESP01_013__FULL_ROM_FLASH_BOOT_UART_DIAGNOSTIC_20261104.md`; original three on-chip image regions exact-verified, no reflash. This note records observed topology only; it does not authorize hardware installation.

**Latest owner-observed physical USB topology (2026-10-29 19:54):** Windows `deus-run` used SSH to the Ubuntu Mac mini, where a single real STM32 `1209:000C` was discovered, explicitly entered `1209:000D` bootloader, committed signed firmware version 6 at floor 5, rebooted and enumerated as **`1209:000E` Management-only IF0** on the same physical port `usb:001:8`. The Ubuntu `deus-cp` then returned `Protocol=1 Service=3 Registry=36`, `PONG` and watchdog-active `HEALTH`. This supersedes earlier date-bound `000C`/CDC current-device descriptions below; those are **history**, not the new USB profile. Windows continues to own SWD/ST-LINK and UART wiring unless physically reverified otherwise. The owner reports OLED looks as expected; UART v6 physical traffic is not yet separately attested. Formal Gate4 remains not authorized.

Last canonical-layout audit: 2026-10-07. Temporary owner-authorized Windows native-USB attachment: 2026-10-16 (Desktop Inspector Gate 3b). Latest **date-bound** Linux Management observation: **Ubuntu Mac mini on 2026-10-20**, installed `1209:000C` vendor Management IF2, Host Core `mspstat/schedprod/fault` read-only PASS; the historical 2026-10-19 observation additionally verified existing CDC IF0/IF1. The 2026-10-20 test **did not open CDC** and is not proof of future `1209:000E`/IF0 enumeration. On 2026-10-28, separate owner-supplied **terminal-only** SSH output again observed current `1209:000C` at Ubuntu `usb1/1-8`, device number `090`, CDC IF0/IF1 and vendor IF2, node `root:plugdev 0660`, `deus` group membership and shell `-r/-w` checks YES; **no original indexed Evidence ZIP, `libusb_open`, `000D`, reset or INFO was produced**, so this observation does not advance Gate3. Hardware attachment must be reverified for any future physical gate.

This document owns the current development, hardware-attachment and acceptance-execution topology. It does **not** define product acceptance state, active gate status or firmware architecture; those remain owned by `docs/CURRENT_STATE.md`, the active boundary plan/acceptance pair and `docs/ARCHITECTURE.md`.

The purpose of this file is to prevent acceptance harnesses from executing a correct operation on the wrong machine or looking for a physical interface on a host that does not own it.

## 1. Canonical physical topology

**Current observed bench and history are distinct:** the canonical native STM32 USB/VBUS on Ubuntu Mac mini plus Windows CH340/UART and ST-LINK/SWD topology was **again positively observed on 2026-10-19**. Independent Linux real-device evidence shows native USB `1209:000C` Management vendor IF2 and working CDC `ttyACM0` while Windows CH340 COM3 served an actual USART1 `help help` response; a later Management-only experiment on the **old** `000C/IF2` firmware encountered a host-probe failure, not proof the USB moved. The explicitly temporary 2026-10-16 Windows WinUSB attachment remains a **historical** Desktop Gate 3b step, not the latest observed wiring. This evidence fixes the previous obsolete 'return to Ubuntu NOT_VERIFIED' label **as of 2026-10-19 only**. Before any later hardware gate, reverify target ownership by bounded read-only discovery; don't rely on undated wiring assumptions, replug cables, change drivers or power-cycle for this documentation update.

The canonical/default bench design is split across two execution hosts:

```text
                         SSH control / artifact transfer
Windows PowerShell 7  ------------------------------------>  Linux USB host (Ubuntu)
      |                                                        |
      |                                                        |  native USB
      | ST-LINK / SWD                                          |  VBUS + data
      | UART / CH340                                           |
      v                                                        v
                    +---------------------------+
                    |       STM32 target        |
                    |   STM32F103 / Deus OS     |
                    +---------------------------+
```

Canonical/default intended ownership (reverify physical attachment before execution):

- **STM32 native USB** belongs to the Linux USB host (Ubuntu) in the canonical bench and was last positively observed there **2026-10-19** (`1209:000C`, Management IF2; CDC IF0/IF1). Future attachment must still be rediscovered.
- **STM32 UART/CH340 is physically connected to Windows.**
- **STM32 ST-LINK/SWD is physically connected to Windows.**
- Windows is the primary orchestration/build/debug host.
- Windows reaches the Ubuntu USB host through OpenSSH, using `<SSH_USER>@<USB_HOST>` / SSH alias `<USB_HOST_ALIAS>`.

This split-host topology is the **expected/default operating model**, not proof of current cable attachment. Every new hardware acceptance package must independently discover the physical USB-owning host first; persistent topology changes require an explicitly approved document update.

## 2. Windows execution domain

Canonical repository:

`<WINDOWS_OPERATOR_PATH>`

Tools root:

`<WINDOWS_OPERATOR_PATH>`

Operator input ZIP staging: an independently verified owner-local directory outside all Git worktrees. The exact path and Windows runner search configuration belong to private operator configuration. Any changed root needs native runner lookup acceptance before a ZIP is described as runnable.

Original operator **evidence output** directory for future runs (owner-created, independently preflighted):

`<PARENT_OF_CANONICAL_OS_REPO>\Evidence` — sibling of `<PARENT_OF_CANONICAL_OS_REPO>\Tools` and `\Logs`; **outside** Public/Private Git; never use for BIN, `.pkg`, secrets or signer escrow. The owner reported manually relocating historical Downloads evidence into the new sibling `Evidence` on 2026-10-24; the individual moved files' SHA-256/CRC/new locator have **not** been independently verified in this repository. New operator runs must preserve immutable originals, and any relocation must retain exact hashes. See `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

Selected portable protected recovery store for future Windows custody: `<PARENT_OF_CANONICAL_OS_REPO>\\ProtectedRecovery`, at the **same level** as `OS`, `Tools`, `Logs`, `Evidence`; the destination is not a Git directory, an Evidence folder, or the previous `%LOCALAPPDATA%/DeusOS/ProtectedRecovery`. Selection does not establish creation/ACL/security: provision it separately on real Windows, reject reparse/UNC/unexpected ACLs, and never assume Windows per-user ACL or DPAPI survives moving the workspace to another PC. Native `deus-run` Evidence output stays in sibling `Evidence`. Full normative contract: `docs/ARTIFACT_CUSTODY_AND_RECOVERY.md`.

Primary shell:

- PowerShell 7.

Windows owns:

- canonical Git working tree and normal Git mutation;
- GNU Arm firmware builds;
- .NET Windows restore/build/test work;
- STM32CubeProgrammer;
- ST-LINK/SWD target access, Flash readback/programming and reset;
- CH340/UART diagnostics;
- top-level acceptance orchestration;
- SSH/SCP control of the remote Ubuntu USB domain.

Current stable hardware/tool facts used by acceptance when relevant:

- ST-LINK V2, firmware `V2J48S7`;
- accepted SWD speed `950 kHz`;
- read-only fallback ladder `950, 480, 240, 125 kHz`;
- CH340 / USART1, historically `<UART_COM_PORT>`, `115200 8N1`;
- STM32CubeProgrammer `2.23`;
- Arm GNU Toolchain `15.3.Rel1` / GCC `15.3.1`;
- Windows .NET 10 policy is owned by `host/global.json`; the installed SDK selected by that policy is re-proven by each relevant harness.

COM numbers are enumeration facts, not permanent identities. A harness that needs UART must discover/verify the non-target CH340 instead of blindly trusting a historical COM number.

## 3. Linux USB execution domain (Ubuntu)

SSH endpoint:

- canonical operator form: `<SSH_USER>@<USB_HOST>`;
- accepted alias form: `<USB_HOST_ALIAS>`.

The Ubuntu host owns the target's native USB connection. Historical accepted environment evidence records:

- host name `<USB_HOST_ALIAS>`;
- user `<SSH_USER>`;
- Ubuntu `26.04.1 LTS`, x86_64;
- Linux .NET SDK `10.0.112`;
- libusb `1.0.29`;
- usbutils `019`.

The target composite USB device is `1209:000C` / `Deus OS Device`:

- CDC IF0/1 is normally owned by Linux `cdc_acm`;
- management IF2 is accessed by the Linux libusb adapter;
- the target USB cable also supplies normal target VBUS/power in this bench topology.

The firmware-update bootloader enumerates separately as private-test `1209:000D` / `Deus OS Bootloader`. Enumeration alone does not guarantee unprivileged libusb open authority: accepted hardware evidence has observed the current `/dev/bus/usb/BBB/DDD` node as `root:root` mode `664`, causing `LIBUSB_ERROR_ACCESS (-3)` for user `<SSH_USER>`. When a bootloader transaction requires permission repair, the accepted bench fixture is bounded to the exact currently enumerated ephemeral node: one operator-visible interactive privilege step may change that node to owner `deus`, mode `600`; no password is captured, no persistent udev rule is created, and the actual bootloader protocol must then execute unprivileged as `deus`. Bus/device tokens from `lsusb` are already zero-padded decimal strings and must be used as strings when constructing `/dev/bus/usb/$bus/$dev` (for example device `099`), not numerically reinterpreted.

A Windows harness MUST NOT assume that STM32 USB/WinUSB/CDC is locally enumerable merely from the canonical Ubuntu-owned design. Windows WinUSB is an accepted transport **when physical discovery confirms a Windows-attached target** (last observed at the temporary 2026-10-16 UI test). Conversely Ubuntu libusb must not be assumed reachable until its USB preflight passes; the later return of the cable is unverified.

## 4. Repository views are not the Linux USB host

The canonical repository is the Windows tree `<WINDOWS_OPERATOR_PATH>`.

**DEUS MCP runs in WSL on the Windows development machine, not on the Linux USB host.** Its project root `<WSL_OPERATOR_PATH>` and repository path `<WSL_OPERATOR_PATH>` are local WSL views of the Windows-backed working tree. A DEUS MCP file/Git operation is therefore **local WSL repository access**, not remote Linux USB execution, and it does not by itself prove Windows PowerShell/ARM toolchain availability. Windows remains the accepted GNU Arm build/SWD/UART execution domain; native USB remains on Linux USB host (Ubuntu).

Within the Windows/WSL environment, the Windows checkout and local WSL bind/view refer to the same Windows-backed repository object. Operator-specific absolute paths are intentionally omitted from this public view:

- Windows checkout: `<WINDOWS_OPERATOR_PATH>`;
- local WSL view: `<WSL_OPERATOR_PATH>`.

That WSL bind/view relationship is local to the Windows development environment. It MUST NOT be confused with the remote Ubuntu host reached as `<SSH_USER>@<USB_HOST>`.

The Linux USB host does not need a persistent clone at `<WSL_OPERATOR_PATH>`. A historical harness that assumed such a remote repository failed correctly. Cross-host acceptance should normally materialize the exact required candidate/payload on Windows, transfer it to a unique remote `/tmp` directory by SCP, verify its hash remotely, execute there, return evidence, and remove the temporary remote directory.

## 5. Power and signal ownership

Normal current bench policy:

- target normal power/VBUS comes from the USB cable attached to the Linux USB host (Ubuntu) host;
- ST-LINK provides SWD signals/ground but must not simultaneously provide a conflicting 3.3 V source;
- UART adapter provides TX/RX/ground; adapter VCC remains disconnected when the target is normally USB-powered.

Consequences:

- a **physical target power cycle** means removal/restoration of the target USB power connection on the Linux USB host side;
- disconnecting ST-LINK is not the normal target power-cycle primitive;
- an SWD software reset is initiated from Windows but USB disappearance/re-enumeration must be observed on the Linux USB host (Ubuntu) host;
- physical USB reconnect evidence belongs to the USB-owning host even when the top-level package is orchestrated from Windows.

### 5.1 Target wiring / pin map

The current physical bench must be interpreted together with the target-side pin map below.
These pins are stable hardware ownership facts for the current STM32F103/Blue-Pill-class
target unless a later hardware change explicitly updates this document:

| Function | Physical Blue Pill marking → official STM32F103 GPIO / function | Physical connection / owner |
| --- | --- | --- |
| Native USB FS | **A11 (PA11, USB_DM)**; **A12 (PA12, USB_DP)** | USB data + normal target VBUS/power to Linux USB host (Ubuntu) |
| Emergency UART / CH340 | **A9 (PA9, USART1_TX)**; **A10 (PA10, USART1_RX)** | CH340 on Windows, 115200 8N1; TX/RX/GND connected, adapter VCC disconnected while USB powers target |
| ESP Node Link / USART2 | **A2 (PA2, USART2_TX)**; **A3 (PA3, USART2_RX)** | Assigned in installed signed v8 firmware; direct STM32↔ESP UART wiring **not yet physically connected or attested** |
| SWD debug/recovery | SWD connector **SWDIO (PA13)**; **SWCLK (PA14)** | ST-LINK on Windows with shared ground; ST-LINK 3.3 V disconnected while USB powers target; use connector silkscreen, not assumed side-header A13/A14 |
| OLED / I2C1 | **B6 (PB6, I2C1_SCL)**; **B7 (PB7, I2C1_SDA)** | SSD1306-class 128x32 display |
| Heartbeat LED | **C13 (PC13)** | on-board active-low LED |

Consequences for harnesses and product boundaries:

- native USB acceptance must not repurpose **A11 (PA11) / A12 (PA12)** or look for the current target USB on Windows unless the cable topology is explicitly changed and re-proven;
- UART diagnostics remain on **A9 (PA9) / A10 (PA10)** and are not a silent fallback product-update carrier;
- SWD connector **SWDIO (PA13) / SWCLK (PA14)** remains recovery/debug authority, not normal product update;
- bootloader/update code must not seize **B6 (PB6) / B7 (PB7)** or require the OLED/I2C path for update correctness;
- any pin, cable, VCC or host-owner change is a topology change and requires this document to be updated before the next composed hardware gate.

## 6. Acceptance execution-domain matrix

| Operation | Required execution domain |
| --- | --- |
| Git pre/poststate, temporary index, candidate materialization | Windows canonical repository |
| GNU Arm firmware build | Windows |
| Windows .NET restore/build/tests | Windows, with CWD under `host/` |
| STM32CubeProgrammer / ST-LINK read, erase, program, reset | Windows |
| UART / CH340 diagnostics | Windows |
| Target native USB discovery / `lsusb` | Linux USB host (Ubuntu) |
| Linux libusb management IF2 runtime | Linux USB host (Ubuntu) |
| Linux CDC presence/retention | Linux USB host (Ubuntu) |
| Physical USB disconnect/reconnect / USB-powered retention | Linux USB connection, with remote observation |
| Cross-host coordination | Windows PowerShell -> SSH/SCP -> `<SSH_USER>@<USB_HOST>` |
| Human OLED visual confirmation | physical target/operator |

A composed hardware gate may legitimately span both hosts. It must not collapse these domains into one generic "host".

## 7. Mandatory cross-host harness rules

Any acceptance package that needs both Windows-owned SWD/UART and Linux USB host-owned USB must:

1. identify every step with its execution domain before execution;
2. preflight the Windows resources it will need before destructive target mutation;
3. preflight SSH alias resolution and non-interactive public-key access before destructive target mutation;
4. preflight remote `hostname`, user, OS and target USB presence before destructive target mutation when USB is required;
5. use bounded external-process execution on both hosts;
6. close stdin and capture stdout/stderr independently;
7. use hard timeouts and process-tree cancellation;
8. transfer exact candidate/payload bytes by a bounded SCP primitive and verify SHA-256 after transfer;
9. use a unique remote temporary directory; never depend on a persistent remote repository unless that exact repository is independently proven for the run;
10. return remote raw evidence into the single authoritative Windows-side evidence ZIP;
11. clean remote temporary state after evidence transfer;
12. preserve exact host/domain labels in chronological evidence.

Recommended evidence tokens include:

```text
ORCHESTRATOR_DOMAIN=WINDOWS
CANONICAL_REPOSITORY=<WINDOWS_OPERATOR_PATH>
USB_EXECUTION_DOMAIN=MACMINI_UBUNTU
USB_SSH_TARGET=<SSH_USER>@<USB_HOST>
UART_EXECUTION_DOMAIN=WINDOWS
STLINK_EXECUTION_DOMAIN=WINDOWS
```

Remote and local command records should also carry an explicit host/domain label so identical commands cannot be mistaken for execution on the other machine.

## 8. Reusable primitive baseline

The current environment has already produced accepted executed proofs for several building blocks used by later composed harnesses:

- bounded Windows child-process execution with redirected stdin/stdout/stderr, hard timeout and process-tree cancellation via `Kill(true)`;
- Windows OpenSSH alias/public-key execution and SCP transfer to the Linux USB host Ubuntu domain, including bounded remote temporary-directory workflows;
- Windows .NET locked restore followed by Release build with `host/global.json` in scope; the dedicated repair primitive passed after the corruption-matrix v3 harness exposed a stale restore-state assumption;
- Linux libusb access to the target management interface on the Linux USB execution domain in prior Host Control acceptance.

These proofs authorize reuse of the **primitive semantics**, not blind reuse of stale paths/candidate hashes. Every composed harness still performs lightweight preflight of the actual endpoint/tool/interface it is about to use, and any topology/toolchain change that affects a primitive requires re-proof before destructive composition.

## 9. USB-management rule for the current bench

For current published Asset/Configuration operation and any later acceptance that reuses this topology:

- management IF2 traffic must be exercised through Linux/libusb on the Linux USB host Ubuntu host unless the operator explicitly rewires target USB to Windows and that topology change is re-proven;
- the Linux adapter is a first-class implementation, not a test shim: it discovers `1209:000C`, validates vendor IF2 `FF/00/00`, claims only interface 2 and uses EP4 OUT/IN `0x04/0x84`; it must not detach a kernel driver from IF2;
- production `deus-cp config ...` uses `AssetAccessPolicy.PublishedOnly`; the published target advertises the accepted Asset capability, so ordinary CLI Asset operations are governed by the published capability/identity contract rather than the historical pre-publication gate state;
- historical Asset Gates 1–5 intentionally kept system capability bit 5 OFF and used `AssetAccessPolicy.PrepublicationAcceptance` only inside acceptance-only helpers/runners; that historical mechanism must not be treated as current product policy or reintroduced into the production CLI;
- Windows may still drive ST-LINK fault selectors, Flash corruption/readback and reset when a separately authorized acceptance boundary requires those operations;
- Windows may still observe UART diagnostics;
- after a Windows-driven reset or Flash operation, runtime USB recovery/HELLO/RPC proof must be performed on the Linux USB host;
- a package that looks only for local Windows WinUSB and declares the target absent is a harness-topology defect, not product evidence.

Failure class: `HARNESS-EXECUTION-DOMAIN-OWNERSHIP-01`.

## 10. Bundle / archive construction for this ecosystem

The operator-facing workflow remains:

The operator workflow remains: canonical `DEUS_OS_OPERATOR_BUNDLE_V2` construction in a validated off-Git staging directory, independently pinned SHA/manifest and native release checks, execution through the installed `deus-run <package_id> -ExpectedSha256 <sha>`, and original evidence retained in the approved local evidence store. Staging lookup changes require separate native Windows validation.

Windows DEUS Runner **v1.3.0** was accepted on 2026-10-19; the actual installed runner was **upgraded to v1.3.1 on 2026-10-25**, original Windows evidence SHA-256 `536A882802F0B7192E97459B3E4C35BF95BE462BE3F3670236037F6A4005121C`, and native search of OS-sibling off-Git `Bundle` was accepted from nested OS directories. The legacy Downloads and current working-directory searches remain compatibility sources. It owns package selection, SHA/manifest validation, AST checking, child invocation and the **last green/red RESULT**. New bundle scripts report JSON via `DEUS_RUN_RESULT_PATH` under `DEUS_OS_RUNNER_RESULT_V1`. No custom `Expand-Archive; py ...`, per-ZIP launcher, or lone `DIAGNOSTIC_EXIT=1` is a valid new-package operator interface. Historical V2 packages without the structured contract remain supported by the runner in legacy exit-only mode.

A delivery ZIP that spans both hosts should contain, as applicable:

- `apply.ps1` — Windows orchestrator;
- `manifest.json`;
- `PACKAGE_README.txt`;
- `hashes.sha256`;
- Windows-local payloads;
- a bounded Linux payload script/archive to SCP to the Linux USB host;
- exact candidate artifacts required by the gate.

The gate's `apply.ps1` owns the final evidence package; it copies remote Linux evidence back into the single Windows-assembled ZIP. The shared `deus-run` owns terminal finalization and verifies the declared evidence ZIP SHA in structured-result mode. The operator must not manually merge Windows and Linux evidence.

The package must follow `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` for timestamps, section banners, categories, process control, evidence hashing, final red/green RESULT presentation and failure classification.

## 11. Topology-sensitive anti-patterns

Do not:

- search Windows for the current target USB device and treat absence as product failure;
- use local Windows WinUSB when the target cable is physically attached to the Linux USB host;
- assume the Linux USB host contains the canonical repository;
- confuse WSL `<WSL_OPERATOR_PATH>` with the remote Linux USB host filesystem;
- run CubeProgrammer on the Linux USB host in the current topology;
- run CH340/UART acceptance on the Linux USB host in the current topology;
- use ST-LINK 3.3 V or UART-adapter VCC as an accidental second target supply;
- call a Windows SWD reset a complete runtime proof without remote USB recovery verification;
- require the operator to manually combine two evidence archives.

## 12. Change control

If any cable ownership or execution host changes, update this document before composing the next hardware acceptance harness.

A topology change invalidates only assumptions that depend on that topology; it does not by itself invalidate accepted protocol/product contracts. The affected hardware-access and SSH/SCP primitives must be re-proven on the new execution domain before a composed gate is run.
