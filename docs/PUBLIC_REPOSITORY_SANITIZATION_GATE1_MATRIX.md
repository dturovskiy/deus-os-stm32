# Deus OS — Gate-1 Tracked-Path Publication Classification Matrix

Status: **GATE-1 READ-ONLY CURRENT-TREE FIRST PASS; NOT A COMPLETED HISTORICAL/EXTERNAL SECURITY AUDIT; NO DELETION/MIGRATION AUTHORIZED**

Owner: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md`; acceptance: `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`; global current state: `docs/CURRENT_STATE.md`.

## Frozen audited candidate, measurement and classification rules

- Exact pre-audit `main`: `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`. The baseline `git ls-files -s` returned **254 tracked paths**, all **254/254 current file contents** fully read through DEUS-MCP with no truncation or failed reads, each anchored by its **Git blob object ID prefix** and path below. This is a snapshot before this report is added; *do not* call the new report path part of the 254-file input.
- Historical pre-Gate-0 252 tracked paths remain a distinct correct snapshot; **254 includes two new sanitization plan/acceptance paths** added at Gate-0. Files modified after this snapshot require refreshed blob SHA and classification.
- Counts from deterministic path/whole-current-file triage: **PUBLIC 141**, **REDACT 15**, **PRIVATE 49**, **RETAIN 49**; sum **254**. These are **proposed dispositions** with different downstream dependency/security proof requirements, not proof that any exact file is safe to delete, nor a leaked-key claim.
- `PUBLIC` = expected public source/API/CI/legal consumer; subject to independent history/surface verification. `REDACT` = mixed content or host/path detail requires public-safe replacement preserving private original. `PRIVATE` = historical evidence/operator content tentatively assigned to private archive *only after* verified preservation and consumer impact. `RETAIN` = unresolved contract/consumer **blocks removal** pending owner-specific decision.
- `WIN_PATH / HOME_PATH / HOST_ALIAS` flags are **lexical current-text indicators**, not raw sensitive values. `none` does not imply guaranteed absence of sensitive information. Risk categories are review hypotheses, not incident verdicts. Git blob OIDs are public revision identifiers, **not** secret values.
- No source, executable PowerShell, firmware, private evidence, Git remote, history, target or physical bench changes occurred while constructing this matrix.

## Mandatory dependency and publication safeguards

1. The **seven PowerShell default-path owners** identified in the Gate-0 follow-up remain `REDACT` candidates; changing a default toolchain/programmer path is a **behavioral tooling edit**, not docs-only sanitization. Gate-1 must prove parameter override, wrapper propagation and Windows ARM/recovery tests before freezing any permitted Gate-4 edit.
2. Preserve **public source, linker, API/protocol specs, CI and LICENSE** as functional dependencies; redact operator-specific details from public README/current-state/roadmap as needed without inventing a new competing global state authority.
3. `PRIVATE` historical plan/acceptance designations **cannot** be used to justify immediate removal: archive source/evidence and validate all references, accepted ABI decisions and maintainer reproducibility first. Public technical extracts must exist before any file transition.
4. For any `RETAIN` row the owning consumer must resolve its disposition before Gate-3 strategy authorization. Do not auto-promote to PUBLIC solely to meet a checkbox.
5. This public matrix intentionally stores paths, public Git blob prefixes, broad risk and owner categories **only**. Any future credential strings, account identities, complete raw paths, ephemeral target IDs, private acceptance files or sensitive logs belong solely in an approved access-controlled evidence sink.

## Complete per-path matrix — 254/254 assessed

| Tracked path | Git blob ID prefix | Proposed | Owner | Review concern | Dependency consumer | Host-text flags |
|---|---|---|---|---|---|---|
| `.gitattributes` | `9344e7d6988f` | **PUBLIC** | PROJECT | LICENSE_NOTICE | Repository publication | none |
| `.github/workflows/ci.yml` | `f97d392bcbe1` | **PUBLIC** | CI | BUILD_OR_CI_DEPENDENCY | Public release validation | none |
| `.gitignore` | `db83f7a9a389` | **PUBLIC** | PROJECT | LICENSE_NOTICE | Repository publication | none |
| `CHANGELOG.md` | `7f8609bbed5c` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `LICENSE` | `66fe034f34b2` | **PUBLIC** | PROJECT | LICENSE_NOTICE | Repository publication | none |
| `README.md` | `a00b47422e38` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `bootloader/bootloader.c` | `ed34ad98f6dc` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM build and update safety | none |
| `bootloader/startup.s` | `afb877367b99` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM build and update safety | none |
| `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md` | `c7d3db7b269a` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md` | `40e7fff0aa29` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_ACCEPTANCE_PLAN.md` | `1d80f80caa52` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md` | `ebcea5692309` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/ARCHITECTURE.md` | `f5600a6e3ef4` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` | `a7b0139ecafb` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md` | `fc002737f4ec` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md` | `d84121495c7a` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` | `6deab5635041` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` | `3d93c3b50827` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md` | `e69ff9f1a095` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | HOST_ALIAS |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md` | `a3f8fc7850a0` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md` | `c15366213cef` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md` | `7b240864efb0` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/BINARY_FRAMED_TRANSPORT_PLAN.md` | `599976045bf0` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md` | `9ec55c2ad86e` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` | `1a330d04007d` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` | `ca9109f84e47` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | WIN_PATH+HOME_PATH |
| `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md` | `335ec8c4498c` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/BOOT_DESKTOP_UI_PLAN.md` | `c55aef138287` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/CURRENT_STATE.md` | `7dddc782c670` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | WIN_PATH |
| `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` | `b1f6b5421690` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` | `e466879d779a` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | WIN_PATH+HOME_PATH+HOST_ALIAS |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_ACCEPTANCE_PLAN.md` | `1570f6f19d6b` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` | `21579d8aaf6e` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/DOCUMENTATION_MODEL.md` | `2fdc30a358c1` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md` | `a9b8ed26028e` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | HOST_ALIAS |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` | `65d15b859458` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md` | `61996efd0ab8` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md` | `f21620845e40` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md` | `b72fd9cbc879` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` | `b7880fe55eab` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | WIN_PATH+HOME_PATH+HOST_ALIAS |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_ACCEPTANCE_PLAN.md` | `461ef8c973ee` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_PLAN.md` | `532c1a801f33` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | `afd4a09ba102` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_PLAN.md` | `dcaf594cce18` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md` | `76b32bcea4df` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` | `b440c46d7fc2` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_DEAD_SURFACE_CLEANUP_ACCEPTANCE_PLAN.md` | `3380ec4caea3` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_DEAD_SURFACE_CLEANUP_PLAN.md` | `eef3f8064b46` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` | `9be59a8e67b1` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_ACCEPTANCE_PLAN.md` | `65ea0fd48d7a` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` | `2bfa419109b6` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | `39a535d15415` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` | `de490d410c0d` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_ACCEPTANCE_PLAN.md` | `afe9d6520001` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md` | `697d3ceff80e` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_ACCEPTANCE_PLAN.md` | `60a5d3939afe` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` | `cd3574d84699` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_ACCEPTANCE_PLAN.md` | `81487419fd1d` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md` | `5f92e8e1815a` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/IMPLEMENTATION_PLAN.md` | `c65086193d9e` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md` | `50e2a3221d44` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/IWDG_LIVENESS_FOUNDATION_PLAN.md` | `2c6f9f208113` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_ACCEPTANCE_PLAN.md` | `21a6688124e2` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md` | `538a468f509e` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md` | `44316cb04e0b` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md` | `5d65038ec9fa` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md` | `285ddc044636` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_ACCEPTANCE_PLAN.md` | `0ad67f26ba89` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_PLAN.md` | `2ade5bec52a6` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/MASTER_EXECUTION_CHECKLIST.md` | `99247a8cf5a6` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | none |
| `docs/NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md` | `130fd4889cbf` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/NATIVE_USB_DEVICE_CORE_PLAN.md` | `13ad7cb0792f` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md` | `457c72345aab` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md` | `4fd825fd5a00` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md` | `38906aa071bd` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/OLED_CONSOLE_API_CONTRACT.md` | `fddaf94eb6a5` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/OLED_CONSOLE_ARCHITECTURE.md` | `38d324d631b4` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md` | `38ef9a97c8df` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md` | `df5ea79c594b` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md` | `283d17cff804` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/OLED_SSD1306_HARDWARE_PROFILE.md` | `77d6747dc002` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/OLED_STATUS_BAR_PLAN.md` | `49d42d46d5e4` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/OLED_UI_ACCEPTED_BASELINE.md` | `2cb827690756` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md` | `7d9e40cc3a36` | **RETAIN** | DOCS | PUBLIC_ABI + HISTORICAL_LEDGER | Protocol / recovery / future development | none |
| `docs/OLED_UI_LAYOUT_PLAN.md` | `149ac101703f` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md` | `f5bcfbec4464` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` | `a9eaa8dff4c4` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md` | `d2a0c989f336` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md` | `fd739ad20d3c` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/PRODUCTION_HEARTBEAT_TASK_ACCEPTANCE_PLAN.md` | `eb0fa5333340` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/PRODUCTION_HEARTBEAT_TASK_PLAN.md` | `d9e2ddec55c2` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/PROJECT_HANDOFF.md` | `b30bcd36e4fd` | **PRIVATE** | OPERATIONS | IDENTITY_OR_TOPOLOGY + GATE_EVIDENCE | Bench operation / acceptance provenance | WIN_PATH+HOST_ALIAS |
| `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md` | `a889963d6987` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` | `a7daf5079381` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md` | `c3dfe9cb61e8` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/REPOSITORY_CI_BASELINE_PLAN.md` | `165e7ee75985` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md` | `a7cd5fe5e159` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` | `c521aed4d91b` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` | `cf9e381d39c2` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | WIN_PATH+HOME_PATH |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` | `85a818c8abdc` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | WIN_PATH+HOME_PATH |
| `docs/ROADMAP.md` | `62868a47196a` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | none |
| `docs/SCHEDULER_FIXED_PRIORITY_ACCEPTANCE_PLAN.md` | `41b36f133869` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/SCHEDULER_FIXED_PRIORITY_PLAN.md` | `f485e17e4953` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/SCHEDULER_TIMED_BLOCKING_ACCEPTANCE_PLAN.md` | `3167e081d500` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/SCHEDULER_TIMED_BLOCKING_PLAN.md` | `05f48da27a81` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_ACCEPTANCE_PLAN.md` | `9bc09ee87095` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md` | `0998796c3910` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md` | `5bd5bbe36229` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/SHELL_RPC_FOUNDATION_PLAN.md` | `27a280d86821` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/TARGET_DEAD_API_CLEANUP_ACCEPTANCE_PLAN.md` | `107b3a385c91` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/TARGET_DEAD_API_CLEANUP_PLAN.md` | `941b338cdbf2` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_ACCEPTANCE_PLAN.md` | `a93c58390004` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md` | `324783b8e543` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_ACCEPTANCE_PLAN.md` | `4514b5d4cdc3` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` | `32799b593d20` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md` | `b21025d8d53d` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/USB_CDC_ACM_CONSOLE_PLAN.md` | `17cd346179d5` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `docs/USB_IDENTITY_POLICY.md` | `5f659f794c89` | **PUBLIC** | PRODUCT | PUBLIC_ABI | Interop / source of truth / public contribution | none |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md` | `ca544b8c0321` | **PRIVATE** | ACCEPTANCE | GATE_EVIDENCE | Internal audit / maintenance provenance | none |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` | `de9c7911257d` | **RETAIN** | DESIGN | GATE_EVIDENCE + PUBLIC_ABI | Scoped architecture / historic contracts | none |
| `host/DeusOs.Control.sln` | `51f36a25f65f` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/Directory.Build.props` | `73bbe6fab9a3` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/Directory.Packages.props` | `020de803302b` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/global.json` | `df32a2e39c21` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Cli/DeusOs.Control.Cli.csproj` | `4d8adef90f43` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Cli/Program.cs` | `34d500c01b49` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Cli/packages.lock.json` | `2848f7396973` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/ApplicationListParser.cs` | `3e6e0b4c1072` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/AssetTransferClient.cs` | `ce68c64f14e9` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/AssetTransferProtocol.cs` | `782c0bb028a0` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/BinaryFrame.cs` | `a3074f7dcf68` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/BinaryFrameCodec.cs` | `3106e0ce46c1` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/BinaryFrameDecoder.cs` | `021bf2e896a3` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/Crc16CcittFalse.cs` | `2217c2746865` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/Crc32IsoHdlc.cs` | `e6443b4c339b` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/DeusDeviceClient.cs` | `d62127b22b33` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/DeusDeviceSession.cs` | `086012686294` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/DeusOs.Control.Core.csproj` | `cae5b6efba4c` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/DeusRpcClient.cs` | `5d51d2647111` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/DeviceProtocolChannel.cs` | `e761b0066a73` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs` | `09426b12d71d` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/HelloParser.cs` | `f40d0f5e0fb2` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/HostExceptions.cs` | `31b0184ae602` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/KeyValueTokens.cs` | `f366804fc6dc` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs` | `917e3c8d00a2` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/ManagementStateModels.cs` | `b0ebf9e2b019` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/Models.cs` | `2bc2cb511685` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/ProtocolConstants.cs` | `0c5266d2737d` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/RequestIdAllocator.cs` | `b14e74453b29` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/RpcRequestEncoder.cs` | `a255e1b1ab66` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/SystemInfoParser.cs` | `1c4b87704319` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/TransportAbstractions.cs` | `65f1158b1740` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Core/packages.lock.json` | `6afd6786b95a` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/App.cs` | `afdf43a77d14` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/DesktopController.cs` | `a20535c5559a` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/DeusOs.Control.Desktop.csproj` | `576fc57ec891` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/MainWindow.cs` | `8970c318e18c` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/Program.cs` | `d8132e8997e6` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Desktop/packages.lock.json` | `3cbfaf4ad188` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Linux/DeusOs.Control.Transport.Linux.csproj` | `293bdc37a28a` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs` | `d85fdc04c542` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbDiscovery.cs` | `5da1803c8470` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs` | `613e251adf2d` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Linux/packages.lock.json` | `d806d08b7c27` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Windows/DeusOs.Control.Transport.Windows.csproj` | `87fcbee70966` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Windows/WindowsBootloaderWinUsbDiscovery.cs` | `600602357d3e` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbDiscovery.cs` | `4250f4dc727a` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbTransport.cs` | `d7b73d9f7870` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/src/DeusOs.Control.Transport.Windows/packages.lock.json` | `d806d08b7c27` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/AssetTransferTests.cs` | `d460d63a103c` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs` | `2fe6ab5977d1` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/DeusOs.Control.Core.Tests.csproj` | `1481fc4021d2` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/FirmwareUpdateTests.cs` | `48b95e8a431b` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs` | `50f32b196467` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/ManagementStateModelTests.cs` | `782d1d1b7bab` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/ProtocolTests.cs` | `2640cb8d95ab` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Core.Tests/packages.lock.json` | `74ac56062e06` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Transport.Tests/DeusOs.Control.Transport.Tests.csproj` | `5999c428655d` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Transport.Tests/TransportContractTests.cs` | `fcf496facdd1` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `host/tests/DeusOs.Control.Transport.Tests/packages.lock.json` | `9e76fe16c830` | **PUBLIC** | HOST | BUILD_OR_CI_DEPENDENCY | Host solution, transport tests and CLI/Desktop | none |
| `include/drivers/flash_persistence.h` | `077b0dda2b1d` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/i2c1.h` | `4d44d0a71004` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/iwdg.h` | `f4ec619a0772` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/ssd1306.h` | `d0aabb25fda4` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/status_led.h` | `88048518555a` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/stm32f103_clock.h` | `8ed0f4dce53e` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/usart1.h` | `920df25d8f9d` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/drivers/usb_device.h` | `9d827440c3a6` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/gfx/font3x5.h` | `690710f1d4c5` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/gfx/font5x6.h` | `9ceef6d8fa9a` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/gfx/font5x7.h` | `79257ab7b095` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/gfx/mono_fb.h` | `046731ab98ce` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/gfx/text_renderer.h` | `f80784c20f76` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/application_commands.h` | `ae07f098baf0` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/application_runtime.h` | `f9141319bb9f` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/application_runtime_bridge.h` | `5f85f3e1e653` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/asset_persistence.h` | `929a490a1f48` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/asset_transfer.h` | `d7f4a44de12f` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/binary_frame.h` | `f9457bb386d1` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/binary_rpc.h` | `506a04e9824a` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/command_service.h` | `b187c984b181` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/oled_console.h` | `8188867a6031` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/oled_status_bar.h` | `70ae92201ff0` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/oled_ui_layout.h` | `ff1acc86057c` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/scheduler.h` | `39faab7ae1df` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/scheduler_diagnostics.h` | `6aeee3990bfb` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/system_identity.h` | `a2b0afd7920b` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/system_service_state.h` | `ab8c446297f4` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/time.h` | `d194203e3ec0` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `include/kernel/usb_management.h` | `ef44129d0a57` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `linker/stm32f103c8.ld` | `284c29636eb6` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `linker/stm32f103c8_bootloader.ld` | `db69d178bcd8` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `scripts/README.md` | `7a012d64ec30` | **REDACT** | DOCS | IDENTITY_OR_TOPOLOGY + HISTORICAL_LEDGER | External onboarding / current-state or engineering history | WIN_PATH+HOST_ALIAS |
| `scripts/build_bootloader.ps1` | `50113303ed73` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/build_firmware.ps1` | `fc13f411ed0e` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/create_asset_recovery_bundle.ps1` | `794cd362ce24` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/create_bootloader_recovery_bundle.ps1` | `fe251048805c` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/create_firmware_update_package.ps1` | `4d4b87d52acd` | **PUBLIC** | TOOLING | BUILD_OR_CI_DEPENDENCY | Build or controlled recovery tooling | none |
| `scripts/output_directory_safety.ps1` | `84fc569088f4` | **PUBLIC** | TOOLING | BUILD_OR_CI_DEPENDENCY | Build or controlled recovery tooling | none |
| `scripts/stm32_asset_recovery.ps1` | `f8b26d0061a2` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/stm32_bootloader_recovery.ps1` | `1101d59ae003` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `scripts/stm32_readonly_flash_preflight.ps1` | `902d76609977` | **REDACT** | TOOLING | IDENTITY_OR_TOPOLOGY + BUILD_OR_CI_DEPENDENCY | Windows ARM/programmer/recovery consumers | WIN_PATH |
| `src/drivers/flash_persistence.c` | `30dfa110a073` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/i2c1.c` | `33bb5f1658de` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/iwdg.c` | `db57876e82f2` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/ssd1306.c` | `3f1e06c1a2bc` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/status_led.c` | `4749c64ff074` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/stm32f103_clock.c` | `43ee73642e7a` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/usart1.c` | `32b2635bf855` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/drivers/usb_device.c` | `3b05a0659c35` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/gfx/font3x5.c` | `7484e56ca928` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/gfx/font5x6.c` | `f8a9744d2060` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/gfx/font5x7.c` | `49d50ca24e51` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/gfx/mono_fb.c` | `8d39d8f5a10d` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/gfx/text_renderer.c` | `9fc3df6628e7` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel.c` | `73e13a8bcde8` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/application_commands.c` | `234e01ffe318` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/application_runtime.c` | `ac063990ecb8` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/application_runtime_bridge.c` | `615aa76e7745` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/asset_persistence.c` | `b24f105e98f6` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/asset_transfer.c` | `b85ab015dc48` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/binary_frame.c` | `dd9b6abda441` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/binary_rpc.c` | `3d8a1fca60da` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/command_service.c` | `3ef5bc9da47d` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/oled_console.c` | `84f646248486` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/oled_status_bar.c` | `2249b76fe5fd` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/oled_ui_layout.c` | `61a45ce56d58` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/oled_ui_layout_config_v1.c` | `46f98d8a6c6d` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/scheduler.c` | `1987b90c1dc7` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/scheduler_diagnostics.c` | `3df673ecea23` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/system_identity.c` | `aaee19d256d7` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/system_service_state.c` | `e0a2e3e11a2e` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/kernel/usb_management.c` | `718b07004c93` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |
| `src/startup.s` | `7fbaaa93ab7a` | **PUBLIC** | FIRMWARE | PUBLIC_ABI | ARM firmware build | none |

## Coverage notes and open work

- **CURRENT-TREE:** file coverage **254/254**, content reads complete. Manual adjudication of risk/owner and precise cross-file build/include/test dependencies still required; automatic paths and lexical flags are **first-pass proposals**.
- **HISTORY:** current path blob OIDs and Git commit/name history provide provenance, but no full byte-for-byte content-history secret audit is claimed from this matrix. Old original documents are still accessible in Git's published history.
- **GITHUB:** host-side artifact/log/ref/release/issue/PR checks and their limits belong to the separate Gate-1 evidence/acceptance record, not to implicit PASS claims in individual rows.
- **ACCEPTANCE:** this matrix alone does **not** close Gate-1 while `RETAIN` remains unresolved, the approved private preservation destination is absent, or the history/hosting checks are not adequately covered. Do not start Gate-2 or private/archive/public rewriting until its requirements are explicitly accepted.
