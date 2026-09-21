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

## Firmware build entrypoint

A repository-owned firmware build entrypoint is still required before future self-programming work.

It is intentionally **not** reconstructed by guessing historical compiler/linker flags. The repository preserves the accepted toolchain and partial build constraints but not the full historical optimization/link/section invocation that produced the accepted firmware candidate.

The future canonical build entrypoint must therefore be established from recovered acceptance harness evidence or by an explicit new reproducibility baseline that is independently built, hardware-smoke-tested and accepted.
