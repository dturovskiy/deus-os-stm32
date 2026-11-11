# STM32 normal10 — prepared SWD-first bench and signed recovery source-lock

**2026-11-11 19:02 canonical operator ZIP addendum (source/static scope):** One approved-format source stage `../Bundle/_stage_stm32_os_normal10_swd_hotplug_readonly_backup_v1_20261111/` was authored from prior real operator patterns with 4 source members including exact pinned payload. The official, byte-pinned `../Tools/scripts/operator_bundle_guard/bundle_guard.py` built and validated **one** ZIP `../Bundle/stm32_os_normal10_swd_hotplug_readonly_backup_v1_20261111.zip`; 10,637B whole SHA-256 `EA830EB546D1721959615A6BCEF4851DB7849D93A89DF45A61897C5C673A8A05`; 5 members; manifest `DEUS_OS_OPERATOR_BUNDLE_V2`, `HARDWARE_READONLY`. Managed original builder and verifier task `task_1794420156898_1a68ef10e10c9beb` exit0. This supersedes source-only no-bundle status from prior attempt; **does not constitute native Windows AST/Fail Evidence or hardware qualification**, and no device IO was performed. The one Windows-native operator command is uniquely determined by this package ID/SHA but must not be run before its separate physical admission.


2026-11-11. Scope: SOURCE_ONLY. No Windows PowerShell AST, Windows-native SWD, protected signing, physical Flash, reset, ESP GPIO, CH340 or Wi-Fi RF was performed for this preparation. Global state authority: docs/CURRENT_STATE.md. This is not a Gate4 decision.

## Accepted prior evidence and exact source identities

- Physical P3-H: signed normal9 on Mac mini USB 1209:000E, registry39, original one HELLO / NodeLink READY1 twice; watchdog and stacks PASS. Original: docs/STM32_V9_P3H_NATIVE_MAC_ONE_HELLO_REESTABLISHED_20261111.md.
- Existing native Windows ARM V2 unsigned image: 52,052B SHA256 6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73; Flash 52052/53248; SRAM 8584/11264; uninstalled. Source-index SHA256 D892610A4B842FE84B17C68DA2546BD29C5698B66B9B76D408BB430D93CD1BC5; previous exact 84/84 source SHA match accepted, no rerun needed.
- Installed normal9 protected signed package: 49,100B SHA256 DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B. Its proven native image is 49,020B SHA256 6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E.
- Public bootloader SHA256 A7A52D4962F48D7C95506FB97613BC6A5F7DA661BC07CF3CE850D86247EE25E3. Valid app: candidate > floor; invalid app: candidate >= floor. Startup with valid metadata and no explicit update token jumps directly to application. Signed-valid but runtime-broken normal10 may therefore remove usable USB bootloader entry.

## Proposed packages — exact public headers, NOT signed artifacts

The machine-readable frozen source-only lock is scripts/normal10_release_source_lock_20261111.json. Public format: product 0x534F4544, target 0x0410, origin 0x08002000, image-format1, header48B, HMAC tag32B, DEUSHDR1 domain.

| Role / version | Image (bytes) | Image SHA-256 | Public header SHA-256 (48B) | Planned .pkg bytes |
|---|---:|---|---|---:|
| normal10 / 10 | 52052, accepted Wi-Fi ARM V2 | 6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73 | 350083010A8BB790232712187B776C001EA91485A2578931BC8C34568559606F | 52132 |
| rescue11 / 11 | 49020, tested normal9 image | 6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E | 0ED885C8F271EF1A50EFF35CCCE03126DD83B585D1A239C448050E30346A4364 | 49100 |
| normal12 / 12 | 49020, SAME tested normal9 image | 6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E | F877446802694EF39AA909710CDE6A0DB37D96B498E02D5092EC43DCE917F28A | 49100 |

Choosing proven normal9 image for both rescue11 and normal12 preserves tested USB 000E Management/ENTER behavior, unlike the old legacy-000C rescue9 image. Version11>10 and version12>11 are bootloader-eligible if the bootloader is actually reachable. The old signed rescue9 is NOT floor10-eligible. New HMACs, real .pkg byte hashes and custody do not exist yet. Do not fabricate them.

## Stage A — prepared Windows-only SWD read-only backup (NOT RUN)

New source: scripts/normal10_swd_readonly_backup.ps1.
This bench script requires an existing protected non-reparse directory under ProtectedRecovery, a concrete STM32CubeProgrammer CLI and a selected 950/480/240/125 kHz frequency. Exactly one scoped native Windows PowerShell 7 run must first pass AST/synthetic validation, and requires separate real Windows hardware GO. The file must not be run under an earlier or blanket preflight authorization.

Its hardcoded command allowlist requires port=SWD mode=HOTPLUG for connect, three register reads (Flash size 64KiB; FLASH_OBR 0x000003FC; FLASH_WRPR 0xFFFFFFFF), option-byte display only, and ONE --upload 0x08000000 0x10000 to a new private path. It checks target ID0x410 and 2.0-3.6V. No target erase/program/reset/option-byte modification, no USB/CH340/ESP radio. It seals receipt.txt and raw 65536-byte preflash64k.bin plus SHA only in protected store. Full 64-KiB readback contains sensitive bootloader key material: do not copy to Git, Bundle, Evidence, chat or reports.

WARNING: HOTPLUG SWD attachment can still affect live CPU execution. P3-H historical normal9 USB PASS is NOT post-SWD proof. Next separately authorized Mac mini read-only health after Stage A must confirm USB 1209:000E, normal9, canaries, PSP/MSP, IWDG and watchdog. Missing runtime => STOP without auto RESET. The older scripts/stm32_readonly_flash_preflight.ps1 is NOT this operator; it lacks explicit HOTPLUG and writes legacy Downloads logs.

## Stage B — protected signer preparation (NOT RUN)

Reuse the accepted public scripts/create_firmware_update_package.ps1 format, with an independently reviewed, **new** owner-native protected signing controller; do not rerun consumed v9 signer operator or expose a raw HMAC key file. Native protected controller must authenticate existing signed normal9 with existing escrow key in memory and compare exact stored SHA, extract its 49020B image solely inside ProtectedRecovery, verify image digest, lock the unsigned Wi-Fi BIN SHA, sign versions 10/11/12 once into new exclusive paths under the established Windows ACL policy, then read back and independently verify header SHA, HMAC, image, version, target, and whole .pkg SHA/size. The existing P2 CredUI/escrow workflow is reusable by reviewed source, not reusable as a consumed executable ZIP. Signing, escrow access and any protected output require a separate explicit authorization.

## Stage C — out-of-band recovery and one normal10 update

A signed-valid but nonfunctional v10 might autoboot with no Management ENTER. Real Windows ST-LINK/SWD accessibility and exact full 64-KiB pre-update Flash backup are therefore **critical preconditions**. For reachable USB bootloader, use normal10 then rescue11 then normal12 with strictly increasing floor. For inaccessible bootloader, a separately authorized destructive SWD restore must restore original bootloader/app/metadata pages 0..61 from exact preflash64k.bin and choose explicitly whether to preserve current or restore saved persistence pages 62..63. This is bench-only debug recovery that may change the version floor via physical programming, not an authenticated production rollback. No present script authorizes or performs that restore. Historical Asset recovery 54KiB artifacts and an old Gate4 baseline must NOT be reused blindly for this 0x08002000 relocated layout.

After separate signing/HMAC PASS, owner-approved bootloader entry and fresh 1209:000D INFO (physical floor/committed), one explicit SHA-locked normal10 update may be considered. Post-flash test: runtime 1209:000E registry40, USB, stack canaries/PSP/MSP, IWDG+watchdog reload growth, scheduler, OLED and NodeLink, stop on first failure. ESP Wi-Fi firmware flash/RF scan is later separate scope. V2 PSP worst-case remains NOT_ATTESTED; normal9 margins are not V2 evidence.

## This preparation's original offline check

The network-denied managed test scripts/tests/normal10_offline_readiness_contract_check.py completed EXIT0, task_1794418755205_e2d3d73d9fc6ba49. It recomputed 3 public header SHA/length pairs and checked source bootloader SHA/policy, source-only status, SWD HOTPLUG/allowlist and banned target-write arguments. Initial test task_1794418212651_a80c5f0041b9ad66 failed at a test-only variable-name assertion (expected candidate, actual cand); corrected only that checker, no firmware/script mutation. This is SOURCE_STATIC, not Windows AST, signed package, physical SWD/restore or post-SWD USB proof.

Decision: source preparation complete; next missing **real** predicate is Windows-native HOTPLUG read-only backup + fresh Mac runtime postcheck under separate physical permission. Gate4 NOT_AUTHORIZED; ESP01-010 PARTIAL; original normal9 and ESP v2 untouched.
