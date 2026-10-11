# Deus OS — Gate-1B File Disposition Policy and Owner Decision Register

> **Status: PROPOSED PUBLICATION POLICY / FILE-BY-FILE TECHNICAL REVIEW PUBLISHED at `760a1177f1d7e304421d2f1b6e70dfd0d02e5f80` with exact-SHA [CI 38137514271](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38137514271) SUCCESS; OWNER APPROVAL NOT RECORDED.** The original decision register did not itself authorize implementation. Subsequently, an explicit **documentation-only execution** direction supported a separately backed-up operator-literal cleanup recorded in `docs/PUBLIC_REPOSITORY_SANITIZATION_DOCS_ONLY_ACCEPTANCE.md`. That bounded docs-only action did **not** sign off on all 113 disposition rows or authorize program-script changes, deletion, Git-history rewriting, migration or hardware work. Subsequent seven-script archive-first portability edits and operator/Windows offline proof were separately directed and accepted as a technical sublane (see `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_SCRIPT_PORTABILITY_AUDIT.md`). **Those changes still do not constitute policy-owner sign-off on this register.** The frozen matrix and Gate-1 final acceptance remain unchanged.

## Authority and immutable inputs

- Baseline verified before writing: published `main == origin/main == FETCH_HEAD == a2894380c7937424eed1b97e2de19ee018cb96cc`, clean ahead/behind `0/0`; 261 tracked paths. Exact-SHA hosted [CI 38095015789](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38095015789) SUCCESS, Core `84/84`, Transport `24/24`, Release zero warnings/errors, hygiene 261 PASS.
- Frozen first-pass 254-path matrix `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_MATRIX.md` (PUBLIC 141 / REDACT 15 / PRIVATE 49 / RETAIN 49) is historical input, not overwritten or counted as live personal-privacy acceptance. The seven subsequently published PUBLIC audit/tooling paths are reconciled separately in `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_RECONCILIATION.md`.
- All 261 current tracked text files were re-read for this decision drafting. Exactly 113 baseline items (49 PRIVATE + 49 RETAIN + 15 REDACT) were assigned a bounded recommended disposition below. The 141 baseline PUBLIC and seven post-baseline PUBLIC files remain **proposed public keep / no edits**; no automatic re-certification of unknown secret formats is inferred.
- Older Git-history acceptance: the published scanner independently read 1,381/1,381 Git blob objects reachable under `a2894380c7937424eed1b97e2de19ee018cb96cc`, 31,011,705 bytes, zero unreadable/truncated and zero selected credential-pattern matches. Historical workstation/home path occurrences remain and cannot be erased by a future-only content edit.

## Design choice: public contracts first, private operator values only

Do **not** archive 49 `PRIVATE` entries indiscriminately. Their Gate-1A class indicated a *candidate handling review*, not verified confidential contents. Thirty-seven of forty historical acceptance-plan documents have no old matrix exposure flags, and most carry stable evidence/contract obligations. Preserve public architecture, acceptance, deterministic fault-injection and recovery safety documentation where no sensitive operator-specific value is established; redact operator literals and privatize original material only where justified.

**In-place public paths.** Preserve README/onboarding/acceptance link targets. Before each approved redaction, archive the byte-identical original, record its Git blob and owner and publish a safe public version at the *same path*. Never replace original accepted recovery contract with an untested paraphrase. External historical clones and commit URLs remain a residual issue.

## Proposed counts, not permission

| Frozen class | KEEP_PUBLIC unchanged, subject to final review | PUBLIC_REWRITE / PORTABLE_SCRIPT, after archive and authorization | Original files |
|---|---:|---:|---:|
| PRIVATE | 43 | 6 | 49 |
| RETAIN | 47 | 2 | 49 |
| REDACT | 0 | 15 | 15 |
| **TOTAL** | **90** | **23** | **113** |

Additionally **148** already-PUBLIC tracked paths (141 original + 7 subsequently accepted) have no change proposed. The plan therefore preserves **all 261 current public path names** as an explicit default; up to **23 in-place content modifications** would require archive-first, change-specific proof and an authorized Gate-4 implementation. An archive may also preserve complete old repository provenance, but the 90 KEEP paths are not candidates for silent deletion.

### Actual source evidence underlying the risk decision

- One clearly operator-bound topology document contains multiple workstation/home path literals; `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` is a large mixed public safety + bench-specific evidence guide and also has local paths; `docs/PROJECT_HANDOFF.md` embeds workstation-specific setup. Retain publicly the invariant descriptions and archive local operator detail before a path-preserving rewrite.
- Three old PRIVATE acceptance plans contain historical operator/host risk flags (two host aliases, one Windows/home path indicator), so keep their accepted ABI/CI/evidence conclusions but minimize those values.
- Of the 49 RETAIN technical documents, exactly two have flagged Windows/home path examples: `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` and `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md`; other RETAIN documents default to preservation rather than wholesale move.
- The 15 REDACT candidates comprise **eight public documentation/navigation sources** and **seven executable PowerShell build/recovery/preflight scripts**. The scripts expose caller-overridable toolchain/programmer defaults; replacement must not change image, key, geometry, flash range, fail-close or recovery-mode semantics.

## Read-only 113-path decision register

Columns: `Baseline` is the historical candidate classification. `Proposal` is an unapproved future public handling code. `Original preservation` is a prerequisite, not evidence a private archive already exists. `POLICY_PENDING` requires human publication/privacy approval; generic roles `ACCEPTANCE`, `DESIGN`, `OPERATIONS`, `DOCS` are not named people.

| Path | Baseline | Proposal | Existing exposure flags | README ref | Original preservation | Approval |
|---|---|---|---|:---:|---|---|
| `CHANGELOG.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `README.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/ARCHITECTURE.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` | PRIVATE | KEEP_PUBLIC_SAFETY | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` | PRIVATE | KEEP_PUBLIC_RECOVERY | — | YES | KEEP_BYTES | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | PUBLIC_REDACT_ACCEPTANCE | HOST_ALIAS | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/BINARY_FRAMED_TRANSPORT_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` | RETAIN | PUBLIC_REDACT_HISTORY | WIN_PATH+HOME_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/BOOT_DESKTOP_UI_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/CURRENT_STATE.md` | REDACT | PUBLIC_REDACT_ENTRY | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` | PRIVATE | PUBLIC_REWRITE_OPERATOR | WIN_PATH+HOME_PATH+HOST_ALIAS | YES | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/DOCUMENTATION_MODEL.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | PUBLIC_REDACT_ACCEPTANCE | HOST_ALIAS | YES | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | YES | KEEP_BYTES | POLICY_PENDING |
| `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md` | PRIVATE | KEEP_PUBLIC_ARCH | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` | PRIVATE | PUBLIC_REWRITE_HARNESS | WIN_PATH+HOME_PATH+HOST_ALIAS | YES | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_DEAD_SURFACE_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_DEAD_SURFACE_CLEANUP_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/IMPLEMENTATION_PLAN.md` | PRIVATE | KEEP_PUBLIC_HISTORY | — | YES | KEEP_BYTES | POLICY_PENDING |
| `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/IWDG_LIVENESS_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md` | PRIVATE | KEEP_PUBLIC_ARCH | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/MASTER_EXECUTION_CHECKLIST.md` | PRIVATE | KEEP_PUBLIC_GOVERNANCE | — | YES | KEEP_BYTES | POLICY_PENDING |
| `docs/NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/NATIVE_USB_DEVICE_CORE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_CONSOLE_API_CONTRACT.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_CONSOLE_ARCHITECTURE.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_SSD1306_HARDWARE_PROFILE.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_STATUS_BAR_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_UI_ACCEPTED_BASELINE.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OLED_UI_LAYOUT_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/PRODUCTION_HEARTBEAT_TASK_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/PRODUCTION_HEARTBEAT_TASK_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/PROJECT_HANDOFF.md` | PRIVATE | PUBLIC_REWRITE_HANDOFF | WIN_PATH+HOST_ALIAS | YES | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/REPOSITORY_CI_BASELINE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | PUBLIC_REDACT_ACCEPTANCE | WIN_PATH+HOME_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` | RETAIN | PUBLIC_REDACT_HISTORY | WIN_PATH+HOME_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/ROADMAP.md` | REDACT | PUBLIC_EDITORIAL_REVIEW | — | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `docs/SCHEDULER_FIXED_PRIORITY_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SCHEDULER_FIXED_PRIORITY_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SCHEDULER_TIMED_BLOCKING_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SCHEDULER_TIMED_BLOCKING_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/SHELL_RPC_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TARGET_DEAD_API_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TARGET_DEAD_API_CLEANUP_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | YES | KEEP_BYTES | POLICY_PENDING |
| `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/USB_CDC_ACM_CONSOLE_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | KEEP_PUBLIC_ACCEPTANCE | — | — | KEEP_BYTES | POLICY_PENDING |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` | RETAIN | KEEP_PUBLIC_CONTRACT | — | — | KEEP_BYTES | POLICY_PENDING |
| `scripts/README.md` | REDACT | PUBLIC_REDACT_ENTRY | WIN_PATH+HOST_ALIAS | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/build_bootloader.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/build_firmware.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/create_asset_recovery_bundle.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/create_bootloader_recovery_bundle.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/stm32_asset_recovery.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/stm32_bootloader_recovery.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |
| `scripts/stm32_readonly_flash_preflight.ps1` | REDACT | PUBLIC_PORTABLE_SCRIPT | WIN_PATH | — | ARCHIVE_BEFORE_EDIT | POLICY_PENDING |

## Contract-specific approval conditions

### A. 49 PRIVATE proposals: preserve public evidence and safety

- `KEEP_PUBLIC_ACCEPTANCE` = 37 accepted plan histories with no lexical location flags: preserve publicly as accepted engineering evidence; do not break cross-references or reinterpret approvals.
- `PUBLIC_REDACT_ACCEPTANCE` = three flagged acceptance plans: retain test counts, original candidate/hash/Gate provenance and failure classifications; isolate workstation/host aliases while keeping verifiable conclusions.
- Operations: `KEEP_PUBLIC_SAFETY`, `KEEP_PUBLIC_RECOVERY`, `KEEP_PUBLIC_ARCH`, `KEEP_PUBLIC_HISTORY`, `KEEP_PUBLIC_GOVERNANCE` keep six technical/safety/governance contracts. `PUBLIC_REWRITE_OPERATOR`, `PUBLIC_REWRITE_HARNESS`, `PUBLIC_REWRITE_HANDOFF` are three archive-first rewrites, retaining existing public paths.
- Recovery changes must continue to distinguish a historical Asset recovery contract from current Bootloader recovery and preserve all erase/readback/persistence/validation guardrails. No physical operations are part of Gate-1.

### B. 49 RETAIN proposals: no accidental ABI loss

- 47 `KEEP_PUBLIC_CONTRACT` paths keep technical interfaces, pin/USB/scheduler/bootloader boundaries, accepted OLED baseline and evidence navigation.
- Two `PUBLIC_REDACT_HISTORY` paths require narrow historical path/example sanitization; public technical invariants and acceptance hashes must remain unchanged.

### C. Eight documentary REDACT proposals

- Six docs without selected absolute-path indicators receive `PUBLIC_EDITORIAL_REVIEW`: preserve content by default, review prose for operator identifiers and redact only documented occurrences.
- Two docs with old location/alias flags (`docs/CURRENT_STATE.md` and `scripts/README.md`) receive `PUBLIC_REDACT_ENTRY`. Maintain authoritative current-state and safe cross-platform onboarding. The remaining six include `README.md` and architecture/governance docs; editorial review cannot silently be treated as a byte-for-byte secrecy guarantee.

### D. Seven PowerShell defaults: one coordinated, separate behavioral acceptance

Scripts (must all be covered together): `build_bootloader.ps1`, `build_firmware.ps1`, `create_asset_recovery_bundle.ps1`, `create_bootloader_recovery_bundle.ps1`, `stm32_asset_recovery.ps1`, `stm32_bootloader_recovery.ps1`, `stm32_readonly_flash_preflight.ps1`.

Proposed resolver contract for later approval: explicit CLI `-ToolchainRoot` or `-ProgrammerCli` override wins; otherwise use a declared environment/config path only when valid, then deterministic installed-tool discovery; fail closed and identify missing prerequisites, never silently select a programmer executable among multiple candidates. Do not leak concrete local absolute paths into emitted script, manifest, logs or evidence.

**Non-negotiable tests:** unchanged tool version capture, exact successful Windows ARM build/bootloader artifact/byte/hash when equivalence is required, key-bound/nondeployable test isolation, path override precedence with whitespace paths, rejection of missing/multiple programmer installations, equal packaging output file lists/hashes except approved metadata, generated wrapper `ProgrammerCli` forwarding (both preserve and clean), no unexpected Flash/USB/SWD side effects in mocked offline acceptance, preserved output-directory anti-clobber policy, existing 950/480/240/125 kHz fallback ownership, final independent readback and persistence safety. Real target operation only under separately approved hardware gate.

## README/consumer compatibility (nine at-risk paths)

`README.md` and `scripts/README.md` together reference seven historically PRIVATE and two RETAIN targets. Every one of the nine paths must continue to exist publicly and retain the public contract before an original may be archived. For operator-bound pages keep the same filename but publish a safe general version; no stale cross-reference. A docs link scanner is insufficient without checking filename-as-text references and runtime/CI contracts.

## Residual-risk policy and Gate-1 decision record

Git history is immutable for the existing public `main` unless an exceptional strategy is explicitly approved. A *future-only* release-view cleanup leaves historical workstation paths and pre-redaction document bytes readable from old commits. The full reachable-main technical blob scan proves only recognized-pattern screening, not universal confidential-value absence. GitHub cache, expired artifacts, external clones, mirrors and search-index history cannot be conclusively disproved by this connector access. An explicit **ACCEPT_RESIDUAL_HISTORY** vs **REQUIRE_NEW_PUBLIC_HISTORY_STRATEGY** choice is required before Gate-1 closure. **The later owner decision in this file selects `ACCEPT_RESIDUAL_HISTORY` for known historical operator-path exposure only; it does not finalize Gate-1B/1 or approve unknown credential disclosures.**

### Minimal approvals required for promotion (grouped, not 113 separate invented sign-offs)

1. **Publication owner:** approve the grouped *43 keep / 6 rewrite PRIVATE*, *47 keep / 2 rewrite RETAIN*, *0 keep / 15 review-or-rewrite REDACT* policy and the individual exceptions above, or specify exceptions. Baseline role labels are not signatures.
2. **Private preservation owner:** approve an actually access-controlled archive destination, retention and access policy, preservation manifest and the intended independent restore/unauthorized-access-denial tests. That archive is a Gate-2 action; no archive is claimed in this document.
3. **Security/release owner:** approve how to handle unavoidable already-published history/third-party surfaces; no history rewriting, new public remote or force push is silently authorized.
4. **Build/recovery owner:** approve the seven-script refactor specification and the Windows/ARM/recovery evidence lane, which must happen only after archive-first and approved scope; Host CI 84/84 + 24/24 is not sufficient.

### Live-publication and full-history evidence refresh — 2026-10-11 (READ-ONLY TECHNICAL PROOF; owner history strategy now decided below)

**Exact current public candidate:** `8682bb4fba6e42ded9a7cd5fd4f16c432d3f8b5e` (local `HEAD == origin/main`, clean `0/0` before the documentation-only record). The existing read-only `scripts/public_repository_history_audit.py --ref main` ran successfully with no blob/path content emitted: **2,290/2,290** reachable Git objects, **164 commits / 606 trees / 1,520 blobs**; **1,520/1,520 blobs / 36,708,469 bytes** read without truncation or errors, nine NUL-bearing blobs. All eight implemented recognized credential-signature categories returned **zero blob matches**. Historical environment-pattern matches remain in **157 Windows path-bearing blobs** and **21 Unix home path-bearing blobs**. These are per-pattern blob counts, not proof of unique secrets, affected people, or current exposure severity; an unrecognized/encoded secret is outside the scanner's claim. Historic reachable objects are still present; current text sanitization never erased them.

**External surfaces bounded today:** authenticated GitHub metadata reports the current public repo `dturovskiy/deus-os-stm32` as PUBLIC, one active default branch (`main`), zero currently reported forks and zero releases; the local public clone has no tags. Five individually queried hosted Actions runs (38159948561, 38157871458, 38156059550, 38151414436, 38079465787) each report zero current workflow artifacts. This is **not** an exhaustive 164-commit GitHub-cloud/history/cache/clone/expired-artifact audit. The archive repo `dturovskiy/deus-os-stm32-private-docs` is marked PRIVATE by authenticated GitHub metadata; the previously accepted independent authenticated remote restore verified 36/36 archival originals. **Anonymous access denial remains NOT_CHECKED**: authenticated visibility is not a negative test by a credential-free principal; the attempted separate public-web lookup was not a reliable HTTP response. Third-party mirrors, preexisting clones, indexes and deleted/expired artifacts remain NOT_CHECKED.

**113-row decision-registry integrity rechecked:** exactly 113 distinct tracked paths at current 267-tracked-path HEAD, no missing path or duplicate; **PRIVATE 49 / RETAIN 49 / REDACT 15**; **90 KEEP_PUBLIC**, **23 review/redact/rewrite**, and **POLICY_PENDING 113/113**. The approved *technical* archived-first Windows/script changes and remote restore do **not** impersonate policy-owner acceptance.

**Owner decision 2026-10-11 — ACCEPT_RESIDUAL_HISTORY (limited scope):** the operator explicitly selected strategy A: retain the existing public Git commit history and perform any subsequent approved sanitization prospectively, without rewriting the repository history. The owner understands and accepts the residual visibility of historically published local workstation/project/tool paths, given the read-only aggregate history review and zero matches against the eight recognized credential-pattern categories. These paths may still reveal user/host identifiers or development topology, and the scanner does not prove the universal absence of other secrets. The approval is **not** an approval to publish additional raw private evidence, to expose signing keys/credentials, to dismiss future confirmed leaks, to change GitHub visibility or remotes, or to sign off the 113 individual publication dispositions or missing anonymous-access test. Third-party forks, clones and caches remain outside verified coverage. **Recorded decision token: `ACCEPT_RESIDUAL_HISTORY`; residual-path risk accepted, full Gate-1B/1 still OPEN and Gate-2 BLOCKED.** If a material new secret is identified, re-open the risk review before implementing any history or public-release change.

**Remaining exact gates:** approval of the 113 individual path decisions under the grouped four-role model above; independently verified credential-free private archive denial (or expressly approved scoped limitation); residual risk and retention policy for external surfaces beyond the accepted historic paths; and later Gate-2 preservation requirements. Full Gate-1B/1 OPEN; Gate-2 BLOCKED. No claim of universal privacy clearance.

**Gate state:** current read-only decision register and policy design may be accepted as a bounded Gate-1B evidence slice. Full Gate-1B privacy sign-off remains OPEN pending the four approvals; Gate-1 final OPEN; Gate-2 NOT AUTHORIZED; Gates 3–7 not automatically activated. No delete/move/edit of the 113 original paths is authorized by this register.
