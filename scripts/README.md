# Deus OS — Repository Scripts

This directory contains versioned operator/infrastructure scripts. It is not part of the target firmware runtime and does not authorize product functionality by itself.

## `stm32_readonly_flash_preflight.ps1`

Purpose:

- capture real-board STM32 identification before any future self-programming Flash work;
- read the DBGMCU IDCODE register;
- read the factory Flash-size register;
- display option bytes through STM32CubeProgrammer;
- preserve a timestamped log and SHA-256 under `$env:USERPROFILE\Downloads\`.

Safety contract:

- no erase command;
- no download/program command;
- no `-w8/-w16/-w32`;
- no read-unprotect command;
- no option-byte programming;
- no reset/start command requested by the script;
- `-ob` is permitted only as `-ob displ`.

The script fails closed if its own hard-coded command arguments violate these rules.

Run from PowerShell 7:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File ".\scripts\stm32_readonly_flash_preflight.ps1"
```

Close the STM32CubeProgrammer GUI before running the CLI workflow.

A script-level `READ_ONLY_PREFLIGHT=PASS` means all requested read-only CLI operations completed. Hardware acceptance still requires reviewing the generated log and recording the exact device/revision/Flash/protection state in the relevant future boundary evidence.

## `build_firmware.ps1`

Purpose:

- provide the versioned repository-owned firmware build entrypoint;
- preserve the exact recovered Host Control Gate-2 C/startup/link/objcopy invocation;
- generate the accepted `deus_build_identity.h` binding;
- fail closed if the current C source set differs from the recovered accepted source list;
- build `os.elf`, `os.map` and the canonical `os.bin` without performing target I/O.

The current accepted build is bound to firmware source candidate tree `b895955f7738aceb6fca0272d510cc433378c6ab` and Arm GNU Toolchain 15.3.Rel1 / GCC `15.3.1`. Independent reproducibility acceptance produced the exact 50652-byte BIN SHA-256 `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F` with `text/data/bss = 50540/112/11616` and Flash/SRAM `50652/11728`.

Run from the repository root in PowerShell 7:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File ".\scripts\build_firmware.ps1" -ProjectRoot (Get-Location).Path -OutputDir ".\build\firmware"
```

The default toolchain path is `D:\Projects\STM32\Tools\arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi`.

The entrypoint is build-only: it does **not** invoke STM32CubeProgrammer, ST-LINK, USB, UART, reset or Flash programming. `os.bin` is the reproducible firmware identity artifact. ELF/MAP contain path-dependent debug/map metadata and are not required to be byte-identical across output directories.

The accepted source-tree value is deliberately explicit rather than inferred from the current repository root tree. Future firmware boundaries must compute/freeze a new candidate tree through their acceptance flow and update the build contract together with any authorized source-list change; silently carrying the old source identity forward is not permitted.
