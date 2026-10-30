# Deus OS — repository build scripts

The public `OS/scripts/` is for source-coupled build, image packaging, recovery utilities and offline PowerShell checks. Its active source files are PowerShell, not Python:

- `build_firmware.ps1`, `build_bootloader.ps1` and `output_directory_safety.ps1`: ARM toolchain build inputs and output safety.
- `create_firmware_update_package.ps1`, `create_asset_recovery_bundle.ps1`, `create_bootloader_recovery_bundle.ps1`: package/recovery source and reproducible build support.
- `stm32_asset_recovery.ps1`, `stm32_bootloader_recovery.ps1`, `stm32_readonly_flash_preflight.ps1`: separately authorized recovery and read-only inspection tooling; **not executed by repository CI**.
- `tests/public_sanitization_power_shell_offline.ps1`: synthetic, no-hardware PowerShell parser/resolver and recovery-artifact checks.

**Operator infrastructure is external.** All 73 original Python sources (including their 29 test files) and 27 companion sources/configs previously in `OS/scripts/bench`, `gate3_operator`, `gate4_entry`, `operator_bundle_guard`, and the three root-level Python files are preserved at workspace sibling `Tools/scripts/`. Full exact SHA-256/size custody and original paths: `../Tools/scripts/TOOLING_CUSTODY_MANIFEST.json` (relative to `OS/`). The seven previously archived historical sources also have SHA-preserved `Archive/` copies. No Python is compiled into the STM32 binary.

**CI isolation:** GitHub Actions in this repository build/test .NET Host, enforce source hygiene using shell/`jq`, and run the offline PowerShell portability workflow. They never fetch private/off-Git Tools or exercise physical hardware. Internal operator validation must be independently requalified from the new `Tools` paths; Gate4 remains NOT_AUTHORIZED. Build scripts do not authorize reset, erase, flashing, signing, keys or a release.

Current product state: `docs/CURRENT_STATE.md`. Bench loop: `docs/BENCH_DEVELOPMENT_WORKFLOW.md`. Scope-specific historical Gate/operator contracts remain in `docs/` and Git history; their original Evidence remains off Git.
