# Deus OS — Repository Scripts

This directory contains versioned build, board-inspection and candidate-recovery utilities. It is not part of the target firmware runtime and does not authorize product functionality by itself.

Canonical operational references:

- current project/gate state: `docs/CURRENT_STATE.md`;
- current Windows / Linux USB host / USB / UART / ST-LINK ownership: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`;
- harness/evidence construction rules: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`;
- Asset recovery contract: `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md`;
- published Firmware Update / Bootloader design/acceptance: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` and `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`.

**Workspace archive classification (2026-10-29):** New off-Git sibling `Archive/` owns the intended destinations for dated single-use audit scripts and quarantined operator candidates, while `OS/scripts/` retains reusable build/validation source. An exact 132-file source SHA/path classification manifest is stored outside Git at `Reports/Audits/WORKSPACE_REORGANIZATION_MANIFEST_20261029.json`. **The relocation is complete for this inventory:** 30 individual one-off source files are SHA-verified in off-Git `Archive`, and 102 are SHA-verified in `OS` (11 tested Gate4 imports restored to source) as reusable/product source or source-lock/CI dependencies. Corresponding ZIP originals and historical proofs are in separate sibling directories. No one-off archive script should be invoked merely because a file looks old. Do not blindly `git add -A`, stage operator source candidates, or delete a source referenced by an active tool.

**Latest signed-v6 bench outcome (2026-10-29):** Physical owner Windows/Ubuntu update successfully installed `1209:000E` Management-only IF0, and the real device returned USB info/ping/health with watchdog active. Prior statements below describing `000C` composite IF2 and CDC as the *current* runtime are historical; physical UART for v6 remains unverified. Native operator ZIPs, staged packages, original Evidence and protected firmware belong in the workspace sibling directories outside this source repository. The reusable tracked build/recovery scripts here are source tooling, not distributed operator ZIPs.

**Local bench fast path (2026-10-29):** `docs/BENCH_DEVELOPMENT_WORKFLOW.md` is the concise procedure for a single STM32 board. Use `scripts/bench/check_candidate.py` for the existing signed v6/rescue package byte/format/vector check; use the existing `FirmwareUpdateClient` and target host for an approved real update. Do not require the formal Git/Evidence gate harness for every local source change; preserve detailed acceptance for explicit formal claims. See `docs/DOCUMENTATION_MODEL.md` section 0.

## Permanent Gate 3 owner-signed operator audit (2026-10-28)

Use `scripts/gate3_operator/gate3_auditor.py` and
`scripts/gate3_operator/README.md` for the **reusable read-only source and
Ed25519 owner-consent verifier**. The same versioned module checks new owner
evidence ZIPs, exact SHA-indexed 14-file source-lock, original signer trust,
expiry, one-reset scope and optional exact candidate V2 ZIP bytes. It does not
build or release physical operator ZIPs, perform native Windows AST, or access
STM32; an audit PASS is *not* Full Gate 3. Its regression suite is
`scripts/gate3_operator/test_gate3_auditor.py`. The adjacent `policy.json` stores
current owner-consent evidence hash for zero-argument recurrent verification.

## Script classes

Do not treat every `.ps1` in this directory as an operator-facing acceptance harness.

1. **Candidate/build utilities** produce or manipulate candidate-owned artifacts. They may be invoked by a bounded acceptance orchestrator.
2. **Standalone diagnostic utilities** predate the current harness presentation/process policy and are retained for their narrow purpose; they are not templates for new acceptance packages.
3. **Operator acceptance packages** are generated separately as self-contained ZIPs and must obey the complete harness/evidence playbook: bounded external processes, execution-domain ownership, timestamps, section banners, categories, evidence hashing and final red/green RESULT block.

During an active Gate-2-or-later acceptance campaign, do not rewrite candidate-owned build/recovery scripts merely to modernize their console presentation. If their bytes/semantics must change, regenerate and re-accept the candidate/recovery chain as required by the playbook.

## Generated output reset safety — RDC-04

The current firmware builder and Bootloader recovery generator, plus the historical-only Asset recovery generator, use the shared `output_directory_safety.ps1` function `Reset-DeusGeneratedOutputDirectory`. It validates path/owner/reparse-point constraints and performs the reset as one fail-closed operation, using `.deus-generated-output.json` ownership metadata. Existing external output directories require the exact owner marker; a caller may not select repository control/source paths, drive/home/temp roots, a traversed reparse point, or an unowned external directory for recursive removal.

The marker is tooling metadata and is not part of firmware BIN or generated recovery payloads. Output generator callers have three distinct stable owner IDs. The historical Asset script is retained solely for its pre-Bootloader geometry and is **not** current recovery tooling. RDC-04 Gates 0–4 are accepted and published at `7fda7b8225f05c487f6d5d7154443e8369e74571`; the disposable 27/27 safety matrix and firmware/recovery deterministic equivalence required no target I/O. Historical Asset remains non-operational. See `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` and `_ACCEPTANCE_PLAN.md` for the exact criteria and evidence.

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

`<WINDOWS_OPERATOR_PATH>`

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

The target's normal USB/power is currently attached to the Linux USB host (Ubuntu). A complete acceptance recovery proof therefore cannot stop at Windows SWD readback/reset: any required post-reset USB HELLO/RPC/runtime proof belongs to the Linux USB execution domain and must be coordinated by the outer acceptance harness through SSH/SCP.

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
- Linux USB host (Ubuntu) via `<SSH_USER>@<USB_HOST>`: target native USB/power, Linux libusb management IF2 and Linux CDC observation.

No new script may assume that all physical interfaces are local to Windows. See `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

## Portable tool-path selection (Gate-1B scoped Windows offline PASS, 2026-10-11)

The seven PowerShell build/recovery tools no longer embed an operator-specific installation directory. On the Windows execution host, configure a real installed tool explicitly before running an accepted build or recovery workflow:

- ARM GNU Toolchain: pass `-ToolchainRoot '<installed-arm-toolchain-root>'` to either build script, or set the process environment variable `DEUS_ARM_TOOLCHAIN_ROOT` to that directory. If neither is supplied, the tool resolver requires **exactly one** `arm-none-eabi-gcc.exe` on `PATH` and derives its toolchain root from `bin`; it checks all four required executables before building.
- STM32CubeProgrammer: pass `-ProgrammerCli '<installed-STM32_Programmer_CLI.exe>'` to the recovery/preflight wrapper, or set `DEUS_STM32_PROGRAMMER_CLI` to the installed CLI's absolute path. If neither is supplied, exactly one `STM32_Programmer_CLI.exe` must be discoverable on `PATH`. Missing/ambiguous discovery fails closed, never guesses.
- An explicitly specified parameter takes precedence over the process environment, even if the supplied value is invalid (the command then fails rather than falling back). Paths containing spaces are handled as single PowerShell arguments. The generated preserve/clean recovery wrappers forward the selected `-ProgrammerCli` without embedding a particular machine's installation path.
- `VALIDATE_ONLY` remains the available **non-flashing** mode for a complete bootloader recovery bundle. `PRESERVE_PERSISTENCE` and `CLEAN_STATE` still touch real Flash and require separate explicit hardware authority. A portable resolver does not change that prohibition.
- The source-only `scripts/tests/public_sanitization_power_shell_offline.ps1` test exercises PowerShell parsing, generated-wrapper parsing and mocked path discovery using disposable dummy files. It additionally creates a nondeployable historical Asset recovery bundle from dummy bytes, validates its manifest/padding/file-list/hashes, and parses the emitted wrappers. It **never executes ARM/CubeProgrammer binaries, reads keys, invokes generated recovery wrappers, or touches the board**. Static Linux/WSL checks and ordinary Host CI do not replace Windows PowerShell offline acceptance or actual firmware/recovery binary-equivalence evidence.

Legacy bundles already created against accepted SHA-256 manifests remain immutable historical evidence. Newly generated wrapper bytes and their manifest hashes must be reaccepted against an exact new candidate rather than falsely claimed byte-identical to older bundle files.

Tool resolution failures report only the missing executable name or a generic setup hint, not the supplied absolute tool path; this applies to stderr as well as normal recovery logs. Never put full operator paths into public evidence ZIP/log files.
