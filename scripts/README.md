# Deus OS — Repository Scripts

This directory contains versioned build, board-inspection and candidate-recovery utilities. It is not part of the target firmware runtime and does not authorize product functionality by itself.

Canonical operational references:

- current project/gate state: `docs/CURRENT_STATE.md`;
- current Windows / Mac-mini / USB / UART / ST-LINK ownership: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`;
- harness/evidence construction rules: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`;
- Asset recovery contract: `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md`;
- published Firmware Update / Bootloader design/acceptance: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` and `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`.

## Script classes

Do not treat every `.ps1` in this directory as an operator-facing acceptance harness.

1. **Candidate/build utilities** produce or manipulate candidate-owned artifacts. They may be invoked by a bounded acceptance orchestrator.
2. **Standalone diagnostic utilities** predate the current harness presentation/process policy and are retained for their narrow purpose; they are not templates for new acceptance packages.
3. **Operator acceptance packages** are generated separately as self-contained ZIPs and must obey the complete harness/evidence playbook: bounded external processes, execution-domain ownership, timestamps, section banners, categories, evidence hashing and final red/green RESULT block.

During an active Gate-2-or-later acceptance campaign, do not rewrite candidate-owned build/recovery scripts merely to modernize their console presentation. If their bytes/semantics must change, regenerate and re-accept the candidate/recovery chain as required by the playbook.

## `build_firmware.ps1`

Purpose:

- repository-owned firmware build entrypoint;
- exact current source list and per-source optimization policy;
- generated build-identity header bound to the caller-supplied firmware source tree;
- linker/resource checks;
- ELF/BIN/MAP generation;
- undefined-symbol rejection;
- no target I/O.

Required parameters:

- `ProjectRoot`;
- `OutputDir`;
- `FirmwareSourceTree` — exact 40-character lowercase Git tree identity owned by the acceptance flow.

Default toolchain root:

`D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi`

Representative direct build from PowerShell 7:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File ".\scripts\build_firmware.ps1" -ProjectRoot (Get-Location).Path -OutputDir ".\build\firmware" -FirmwareSourceTree "<accepted-candidate-tree>"
```

The script is build-only. It does not invoke STM32CubeProgrammer, ST-LINK, USB, UART, reset or target Flash programming.

Historical Host Control and Asset candidate sizes remain reproducibility history. The latest published firmware-update candidate identity is owned by `docs/CURRENT_STATE.md` and the published Firmware Update / Bootloader acceptance record; callers must always pass the exact source-tree identity required by the acceptance flow rather than infer it from an older boundary.

When used by an acceptance package, invoke this script through the bounded external-process primitive required by the harness playbook. Do not copy its internal native-tool invocation style into a new monolithic acceptance harness.

## `build_bootloader.ps1`

Purpose:

- build the published 8-KiB bootloader with an explicit external 32-byte update-verification key;
- keep generated key source/object material in a unique temporary directory and delete it in `finally`;
- enforce Flash, `.data+.bss`, conventional-SRAM, linker-symbol and undefined-symbol ceilings;
- emit BIN/ELF/MAP only to caller-selected build output.

**Security:** the resulting bootloader BIN/ELF contains the verification key and is therefore sensitive. Do not place raw key-bearing bootloader output in shareable evidence or publication artifacts.

## `create_firmware_update_package.ps1`

Purpose:

- create the frozen v1 package: 48-byte header + 32-byte HMAC-SHA-256 authenticator + relocated application bytes;
- validate product/target/version/image bounds and application origin;
- consume the update key as an external input and clear the in-memory key array before exit.

Generated signed packages are operational update artifacts. Their distribution policy is separate from repository source publication.

## `create_bootloader_recovery_bundle.ps1` / `stm32_bootloader_recovery.ps1`

Purpose:

- construct and operate the candidate-bound bootloader recovery image for pages 0..61;
- preserve or deliberately clean persistence pages 62/63 according to the selected recovery mode;
- enforce explicit page erase, no mass erase, no read-unprotect and no option-byte mutation;
- perform exact post-program readback checks.

The generated recovery ZIP is **PRIVATE / SENSITIVE**. It does not contain a standalone raw key file, but `recovery_region_62pages.bin` contains the compiled key-bearing bootloader. The generated README explicitly forbids adding that ZIP to the repository, shareable evidence or publication artifacts.

These scripts are published recovery/build tooling, not generic operator-facing acceptance harness templates. Hardware acceptance collectors remain responsible for bounded external-process execution, topology ownership, evidence isolation and final operator presentation.

## `create_asset_recovery_bundle.ps1` — historical Asset-phase tooling

This generator is retained for reproducibility of the published pre-Bootloader Asset boundary only. It assumes the historical application-at-`0x08000000` Asset geometry and **must not** be used for the current Bootloader-boundary relocated application. Current recovery is owned by `create_bootloader_recovery_bundle.ps1` / `stm32_bootloader_recovery.ps1` and the published Firmware Update / Bootloader recovery contract.

Historical purpose:

- construct a candidate-bound `ASSET_CONFIGURATION_STLINK_RECOVERY_V1` bundle;
- consume explicit caller-owned provenance:
  - `BaseHead`;
  - `CandidateTree`;
  - `FirmwareSourceTree`;
- package exact `os.bin`, a padded 54-KiB application region, recovery script, wrappers, manifest and hashes;
- remain independent of hidden `.git` state inside a clean materialized candidate.

The generated `application_region_54k.bin` is exactly 55296 bytes: `os.bin` followed by `0xFF` padding.

This generator is candidate/recovery identity material. During active Gate 5, changing it or the recovery script requires re-evaluating the Gate-2/recovery bindings; do not patch it as a substitute for fixing an outer acceptance harness.

## `stm32_asset_recovery.ps1`

Purpose:

- implement the candidate-bound ST-LINK recovery contract;
- modes:
  - `PRESERVE_PERSISTENCE`;
  - `CLEAN_STATE`;
- explicit page erase only;
- no mass erase;
- no read-unprotect;
- no option-byte mutation;
- program only after explicit erase using `--skiperase`;
- independent full 64-KiB readback verification.

Current physical execution domain:

- **Windows**, because ST-LINK/SWD and STM32CubeProgrammer are physically attached there.

The target's normal USB/power is currently attached to the Ubuntu Mac-mini host. A complete acceptance recovery proof therefore cannot stop at Windows SWD readback/reset: any required post-reset USB HELLO/RPC/runtime proof belongs to the Mac-mini USB execution domain and must be coordinated by the outer acceptance harness through SSH/SCP.

This recovery utility predates some of the current generic operator-facing harness presentation/process requirements and is candidate-bound. Do not use it as the template for a new acceptance collector. The outer collector owns bounded process execution, cross-host coordination, evidence packaging and final presentation.

## `stm32_readonly_flash_preflight.ps1`

Purpose:

- read the real-board STM32 identification/protection state;
- read DBGMCU IDCODE;
- read the factory Flash-size register;
- read FLASH OBR/WRPR;
- display option bytes;
- request no target nonvolatile mutation.

Safety contract:

- no erase;
- no program/download;
- no read-unprotect;
- no option-byte programming;
- no target reset/start requested by the script;
- `-ob` permitted only as `-ob displ`.

Current execution domain:

- **Windows**, because ST-LINK/SWD is physically attached there.

Historical accepted board facts include DEV_ID `0x410`, 64-KiB Flash, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF`, and accepted SWD operation at 950 kHz.

This script is an older standalone collector and does not embody every newer generic harness rule (for example the current candidate-harness process wrapper/presentation contract). Retain it as a narrow diagnostic/reference utility; new acceptance packages must follow `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

## Current split-host rule

For the current bench:

- Windows: repository, build, ST-LINK/SWD, CubeProgrammer, CH340/UART, final evidence ZIP;
- Ubuntu on Mac mini via `deus@macmini`: target native USB/power, Linux libusb management IF2 and Linux CDC observation.

No new script may assume that all physical interfaces are local to Windows. See `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.
