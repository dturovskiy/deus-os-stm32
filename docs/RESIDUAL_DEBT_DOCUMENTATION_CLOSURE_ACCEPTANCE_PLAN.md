# Deus OS — RDC-08 Residual Debt Documentation Closure Acceptance Plan

Status: **GATES 0–7 ACCEPTED/PUBLISHED — GATE-8 FINAL DOCS RELEASE / CI PREDICATE DETERMINES RDC-08 CLOSED vs OPEN**

Canonical design: `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md`

Program authority: `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`

Global state authority: `docs/CURRENT_STATE.md`

Frozen Gate-0 parent: `80ccdf5534ac65a830dcec5cc022e916521d2fb4`.

## Gate 0 — independent audit and two-file scope freeze

**Required PASS evidence**

1. Local DEUS-MCP repository resolves to the Windows-backed OS worktree, not Mac-mini USB host. Baseline `HEAD == origin/main == 80ccdf5534ac65a830dcec5cc022e916521d2fb4`, clean index/worktree and ahead/behind `0/0`, recorded *before* mutation.
2. Verify RDC-01..07 CLOSED/PUBLISHED from Git plus canonical contracts, with RDC-08 the sole mandatory remaining RDC item, no current product-feature boundary.
3. Read complete `docs/*.md` inventory and inspect tracked firmware/Host/bootloader/scripts/linker/CI surfaces; classify observed current-state duplication, inventory history, open checkboxes, stale-versus-historical assertions, path/artifact/secret-search limitations.
4. Gate-0 worktree candidate adds exactly two named docs; NO existing document, product, script, CI, Git policy or target state is altered. At acceptance, staged-name-status must equal `A` for the two named files and nothing else.
5. Review both full documents for exact parent identity, evidence-bound findings, explicit anti-goals, source-of-truth precedence, later Gate tests and fail-closed conditions. No baseline audit observation is misstated as a successful new firmware build or hardware test.
6. `git diff --cached --check` PASS on the nonempty actual staged candidate; exact two-path local docs-only commit; ordinary non-force push after fresh expected-parent verification; fresh post-push `HEAD == origin/main == FETCH_HEAD`, clean index/worktree, `0/0`.
7. GitHub Actions hosted Core `>=84` and Transport `>=24` genuine tests, zero failed/skipped, Release warnings/errors zero and tracked-file/non-vacuous hygiene PASS on the **exact Gate-0 publication commit**. Host CI is not a hardware/ARM equivalence substitute.

**Observed pre-mutation audit (read-only; not Gate-0 publication PASS)**

- WSL-DEUS repo view `/home/deus/projects/deus-os-stm32/OS`, canonical Windows `D:\Projects\STM32\OS`; `main == origin/main == 80ccdf5`, clean `0/0`.
- 250 tracked files. `docs/`: 108 Markdown, 39 matching `*_ACCEPTANCE_PLAN.md` names and 42 other `*_PLAN.md` names. All 108 docs and the 136 tracked non-Markdown source/tool/config surfaces were fully read through DEUS-MCP.
- 30 initially unchecked Markdown items: `BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` one; `MASTER_EXECUTION_CHECKLIST.md` 19; `ROADMAP.md` ten. RDC-08 remains next; generic UI/kernel/networking candidates remain deferred.
- Relative Markdown `.md` links: no unresolved paths found in the audited syntax subset. Historical unmatched plan-name exceptions explicitly described in `DOCUMENTATION_MODEL.md`.
- Baseline `CURRENT_STATE.md` line 140: 5,864 characters of detailed RDC Gate history, conflicting with concise-current-state governance. Section 8 of `DOCUMENTATION_MODEL.md`: correctly marked FDC-10 snapshot `90/30/33`, not today's inventory `108/39/42`. Historical `planned`/FDC-10 references require classification, not indiscriminate rewriting.
- `git diff --check` and staged whitespace check at pristine parent: PASS; current tracked artifact filenames and all-history path-name search: no prohibited firmware/build/ZIP/log/key class. Current-text obvious private-key/token signatures: none. This is **not** exhaustive secret-history scanning, program verification or device testing.
- Baseline board evidence remains historical: authenticated application revision 5 and the previously accepted bootloader; no target SWD, UART, native USB, Flash, reset, signing-key or hardware I/O was performed.

**Gate-0 result — ACCEPTED / PUBLISHED (2026-10-10):** the frozen parent was `80ccdf5534ac65a830dcec5cc022e916521d2fb4`; exact staged and committed path set was the two new `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_*` Markdown files, `A/A` only, `git diff --cached --check` PASS. Normal local commit `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2` changed exactly two paths. Ordinary non-force push `80ccdf5..86abbc5 main -> main`; independent GitHub ref and post-push fresh-fetch `HEAD == origin/main == FETCH_HEAD == 86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`, clean worktree/index and ahead/behind `0/0`. Hosted GitHub Actions [run 38075603101](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38075603101) on this exact SHA completed **success**: locked Host Release build `0 warnings/0 errors`, Core `84/84`, Transport `24/24`, failed/skipped `0/0`, `TRACKED_OUTPUT_HYGIENE=PASS count=252`, non-vacuous diff hygiene PASS. No STM32, USB, SWD, Flash or signing-key I/O occurred. The present separate docs-only status reconciliation records facts **after** they were proven; Gate-1 classification is next. RDC-08 itself remains OPEN.

## Gate 1 — exhaustive source/document claims matrix

**PASS:** every tracked docs Markdown plus root/script reference Markdown is inventoried, with a source-linked classification of current claims, historical gates, open checkboxes, active-next references, future/deferred terms, topology, firmware ownership and plan pairings. Classifications: `CURRENT_CORRECT`, `CURRENT_DEBT`, `HISTORICAL`, `DEFERRED`, `NOT_APPLICABLE`. Exact required Gate-2 docs-only changed-path allowlist frozen.

**FAIL:** unexamined file, active orphan plan, silent promotion of optional features, conflation of WSL worktree with Mac mini USB host, or inferred current device state from old logs alone.

### Gate-1 measured full-source matrix — AUDIT COMPLETE / PUBLICATION PENDING

Frozen classification input: local DEUS-MCP read-only `HEAD == origin/main == FETCH_HEAD == 5a9b8030b7c2d021cd6ba68c0561f6c7b1bf1824`; clean worktree/index, ahead/behind `0/0` before candidate edits. The Git-tracked `ls-files` enumerated **252 paths**, of which **113 Markdown files** (110 in `docs/`, `CHANGELOG.md`, `README.md` and `scripts/README.md`). Every Markdown file was read without truncation; the table below is source/path-linked and exhaustive at this frozen commit.

**Classification method:** Count case-insensitive lexical appearances of `active|next|planned|future|deferred|pending|current|blocked|open|gate-0|gate 0|RDC-08|FDC-10` or `[ ]`; these are broad **lexical candidates**, not independently verified defects. For each row the classification describes the *authority role* of the source and its potentially current gate/status claims. Within `HISTORICAL` sources, superseded future-tense milestone narration stays historical; `CURRENT_CORRECT` sources can legitimately describe historical facts and deferred proposals; `DEFERRED` content cannot authorize an active feature. `CURRENT_DEBT` means a specifically identified present-tense defect, not that all statements in that document are wrong. Routine technical occurrences such as “scheduler remains active” are `NOT_APPLICABLE` to project-gate status.

Measured totals: 113 files, 2214 lexical matches (**broad matching, includes non-status words**), 30 unchecked checkbox lines. Per-source breakdown by authority classification: `HISTORICAL` 79 files/1351 candidates; `CURRENT_CORRECT` 26 files/593 candidates; `CURRENT_DEBT` 4 files/165 candidates; `DEFERRED` 4 files/105 candidates.

| Source (at pinned Git) | Marker hits | Open boxes | Classification |
|---|---:|---:|---|
| `CHANGELOG.md` | 241 | 0 | `HISTORICAL` |
| `README.md` | 8 | 0 | `CURRENT_CORRECT` |
| `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md` | 17 | 0 | `HISTORICAL` |
| `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md` | 21 | 0 | `HISTORICAL` |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_ACCEPTANCE_PLAN.md` | 0 | 0 | `HISTORICAL` |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md` | 6 | 0 | `HISTORICAL` |
| `docs/ARCHITECTURE.md` | 70 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` | 4 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md` | 11 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md` | 14 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` | 13 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` | 6 | 0 | `CURRENT_CORRECT` |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md` | 47 | 0 | `HISTORICAL` |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md` | 59 | 0 | `HISTORICAL` |
| `docs/ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md` | 18 | 0 | `CURRENT_CORRECT` |
| `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md` | 14 | 0 | `HISTORICAL` |
| `docs/BINARY_FRAMED_TRANSPORT_PLAN.md` | 7 | 0 | `HISTORICAL` |
| `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md` | 10 | 0 | `CURRENT_CORRECT` |
| `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` | 19 | 1 | `CURRENT_DEBT` |
| `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` | 14 | 0 | `CURRENT_DEBT` |
| `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md` | 12 | 0 | `HISTORICAL` |
| `docs/BOOT_DESKTOP_UI_PLAN.md` | 16 | 0 | `HISTORICAL` |
| `docs/CURRENT_STATE.md` | 53 | 0 | `CURRENT_DEBT` |
| `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` | 41 | 0 | `DEFERRED` |
| `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` | 23 | 0 | `CURRENT_CORRECT` |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_ACCEPTANCE_PLAN.md` | 17 | 0 | `HISTORICAL` |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` | 27 | 0 | `HISTORICAL` |
| `docs/DOCUMENTATION_MODEL.md` | 79 | 0 | `CURRENT_DEBT` |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md` | 27 | 0 | `HISTORICAL` |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` | 42 | 0 | `HISTORICAL` |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md` | 4 | 0 | `CURRENT_CORRECT` |
| `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md` | 22 | 0 | `CURRENT_CORRECT` |
| `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md` | 34 | 0 | `HISTORICAL` |
| `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` | 57 | 0 | `CURRENT_CORRECT` |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_ACCEPTANCE_PLAN.md` | 3 | 0 | `HISTORICAL` |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_PLAN.md` | 3 | 0 | `HISTORICAL` |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | 2 | 0 | `HISTORICAL` |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_PLAN.md` | 8 | 0 | `HISTORICAL` |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md` | 24 | 0 | `HISTORICAL` |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` | 27 | 0 | `HISTORICAL` |
| `docs/HOST_DEAD_SURFACE_CLEANUP_ACCEPTANCE_PLAN.md` | 4 | 0 | `HISTORICAL` |
| `docs/HOST_DEAD_SURFACE_CLEANUP_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` | 29 | 0 | `DEFERRED` |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_ACCEPTANCE_PLAN.md` | 12 | 0 | `HISTORICAL` |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` | 14 | 0 | `HISTORICAL` |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | 11 | 0 | `HISTORICAL` |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` | 11 | 0 | `HISTORICAL` |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_ACCEPTANCE_PLAN.md` | 7 | 0 | `HISTORICAL` |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_ACCEPTANCE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` | 9 | 0 | `HISTORICAL` |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_ACCEPTANCE_PLAN.md` | 3 | 0 | `HISTORICAL` |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md` | 8 | 0 | `HISTORICAL` |
| `docs/IMPLEMENTATION_PLAN.md` | 31 | 0 | `HISTORICAL` |
| `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md` | 4 | 0 | `HISTORICAL` |
| `docs/IWDG_LIVENESS_FOUNDATION_PLAN.md` | 4 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_ACCEPTANCE_PLAN.md` | 2 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md` | 2 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md` | 9 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md` | 5 | 0 | `CURRENT_CORRECT` |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md` | 4 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_ACCEPTANCE_PLAN.md` | 18 | 0 | `HISTORICAL` |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_PLAN.md` | 23 | 0 | `HISTORICAL` |
| `docs/MASTER_EXECUTION_CHECKLIST.md` | 121 | 19 | `HISTORICAL` |
| `docs/NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md` | 11 | 0 | `HISTORICAL` |
| `docs/NATIVE_USB_DEVICE_CORE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md` | 22 | 0 | `HISTORICAL` |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md` | 25 | 0 | `HISTORICAL` |
| `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md` | 6 | 0 | `HISTORICAL` |
| `docs/OLED_CONSOLE_API_CONTRACT.md` | 10 | 0 | `CURRENT_CORRECT` |
| `docs/OLED_CONSOLE_ARCHITECTURE.md` | 11 | 0 | `CURRENT_CORRECT` |
| `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md` | 3 | 0 | `HISTORICAL` |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md` | 5 | 0 | `HISTORICAL` |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md` | 11 | 0 | `HISTORICAL` |
| `docs/OLED_SSD1306_HARDWARE_PROFILE.md` | 3 | 0 | `CURRENT_CORRECT` |
| `docs/OLED_STATUS_BAR_PLAN.md` | 12 | 0 | `DEFERRED` |
| `docs/OLED_UI_ACCEPTED_BASELINE.md` | 12 | 0 | `CURRENT_CORRECT` |
| `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md` | 14 | 0 | `CURRENT_CORRECT` |
| `docs/OLED_UI_LAYOUT_PLAN.md` | 23 | 0 | `DEFERRED` |
| `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md` | 19 | 0 | `HISTORICAL` |
| `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` | 19 | 0 | `HISTORICAL` |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md` | 20 | 0 | `HISTORICAL` |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md` | 15 | 0 | `HISTORICAL` |
| `docs/PRODUCTION_HEARTBEAT_TASK_ACCEPTANCE_PLAN.md` | 5 | 0 | `HISTORICAL` |
| `docs/PRODUCTION_HEARTBEAT_TASK_PLAN.md` | 6 | 0 | `HISTORICAL` |
| `docs/PROJECT_HANDOFF.md` | 18 | 0 | `HISTORICAL` |
| `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/REPOSITORY_CI_BASELINE_PLAN.md` | 8 | 0 | `HISTORICAL` |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md` | 37 | 0 | `CURRENT_CORRECT` |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` | 40 | 0 | `CURRENT_CORRECT` |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` | 58 | 0 | `CURRENT_CORRECT` |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` | 74 | 0 | `CURRENT_CORRECT` |
| `docs/ROADMAP.md` | 52 | 10 | `CURRENT_CORRECT` |
| `docs/SCHEDULER_FIXED_PRIORITY_ACCEPTANCE_PLAN.md` | 8 | 0 | `HISTORICAL` |
| `docs/SCHEDULER_FIXED_PRIORITY_PLAN.md` | 17 | 0 | `HISTORICAL` |
| `docs/SCHEDULER_TIMED_BLOCKING_ACCEPTANCE_PLAN.md` | 6 | 0 | `HISTORICAL` |
| `docs/SCHEDULER_TIMED_BLOCKING_PLAN.md` | 27 | 0 | `HISTORICAL` |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_ACCEPTANCE_PLAN.md` | 0 | 0 | `HISTORICAL` |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md` | 1 | 0 | `HISTORICAL` |
| `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md` | 11 | 0 | `HISTORICAL` |
| `docs/SHELL_RPC_FOUNDATION_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/TARGET_DEAD_API_CLEANUP_ACCEPTANCE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/TARGET_DEAD_API_CLEANUP_PLAN.md` | 9 | 0 | `HISTORICAL` |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_ACCEPTANCE_PLAN.md` | 0 | 0 | `HISTORICAL` |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md` | 1 | 0 | `HISTORICAL` |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_ACCEPTANCE_PLAN.md` | 5 | 0 | `HISTORICAL` |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` | 15 | 0 | `HISTORICAL` |
| `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/USB_CDC_ACM_CONSOLE_PLAN.md` | 7 | 0 | `HISTORICAL` |
| `docs/USB_IDENTITY_POLICY.md` | 1 | 0 | `CURRENT_CORRECT` |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` | 10 | 0 | `HISTORICAL` |
| `scripts/README.md` | 16 | 0 | `CURRENT_CORRECT` |

**Adjudicated exceptions / pinpointed evidence:**

- `CURRENT_DEBT`: `docs/CURRENT_STATE.md:36,90,108–140` duplicates long accepted histories; its line 140 now exceeds 5,900 characters after Gate-0 status advancement. Global-state role is constrained by `docs/DOCUMENTATION_MODEL.md:38–55`. The 37 distinct 40-hex Git IDs, 29 distinct 64-hex hashes and five evidence artifact names present in `CURRENT_STATE.md` all have at least one duplicate elsewhere among tracked Markdown sources; this supports lossless consolidation **only after** a per-fact review and retention of current device revision 5, product identity and precise pointers.
- `CURRENT_DEBT`: `docs/DOCUMENTATION_MODEL.md:323–348` records the **correct historical** FDC-10 snapshot `90/30/33`; current frozen Gate-1 input has 110 docs Markdown / 40 acceptance / 43 other plans. Add a separate date/commit-scoped RDC-08 inventory, never erase FDC-10 history.
- `CURRENT_DEBT`: `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md:75` and `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md:44` still say RDC-08 Gate-0 is needed. These were once correct at RDC-07 closure but now require an explicit **historical-at-closure** qualifier plus pointer to published Gate-0 `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`, without reopening RDC-07. The still-unchecked RDC-08 box remains unchecked until the entire RDC item closes.
- `HISTORICAL`: `docs/ARCHITECTURE.md:965,1033,1069` contains C3.9/C4.0 “planned” subsections **followed by** accepted records and is a chronology of the now-implemented heartbeat/watchdog. No feature or technical defect is implied; leave the accepted sections intact. `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md:155–170` expressly frames FDC closure statements as historical; do not reinterpret its earlier “feature selection” narrative as current RDC authorization.
- All **30** open boxes are located in `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` (1 — RDC-08), `docs/MASTER_EXECUTION_CHECKLIST.md` (19 — 1 RDC-08 and 18 deferred UI/RTC/kernel-log work) and `docs/ROADMAP.md` (10 — 1 RDC-08, four deferred generic kernel services and five deferred networking tasks). Every box is classified; no additional active mandatory Gate is hidden.
- All non-RDC-08 scoped completed plan/acceptance pairs are accepted historical records; matched active plan pairs are the RDC governance pair and RDC-08 pair. Unmatched plan names: `IMPLEMENTATION_PLAN.md`, `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`, `OLED_STATUS_BAR_PLAN.md`, `OLED_UI_LAYOUT_PLAN.md`; lone unmatched acceptance: `OLED_CONSOLE_ACCEPTANCE_PLAN.md`. All five are historical, deferred or an explicitly documented semantic OLED pair, **not** orphan active work.
- Relative Markdown link-resolution check found **zero unresolved links** among tracked Markdown inputs; referenced uppercase `.md` filenames also resolved to tracked docs/root paths. Host topology remains Windows-backed WSL DEUS-MCP view for Git, Mac-mini Ubuntu for native USB/normal VBUS, Windows for ST-LINK/UART.
- `README.md`, `scripts/README.md`, stable protocols, Flash decision, deferred Host Management reference, `docs/ARCHITECTURE.md` and `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` show no newly proven present-tense conflict; therefore no blanket edits are authorized. Full source-code behavior, signed firmware bytes and physical STM32 were **not** retested at Gate-1.

**Exact Gate-2 content-edit allowlist (frozen; no wildcard expansion):**

1. `docs/CURRENT_STATE.md` — collapse documented historical duplication while preserving current firmware and source-of-truth pointers.
2. `docs/DOCUMENTATION_MODEL.md` — add exact new dated inventory and RDC-08 active-plan pairing; retain the historical FDC-10 section.
3. `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` — label its old “Gate-0 next” disposition historical.
4. `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` — label the historical unchecked RDC-08 pointer while preserving RDC-07 PASS.
5. `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` — record precise Gate-1/Gate-2 scope and eventual status.
6. `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` — Gate-2 execution/proof and eventual status.

Only these six paths may receive **Gate-2 content changes**. Chronological `CHANGELOG.md` and current-status mirrors (`ROADMAP`, RDC program/checklist) are reserved for the separately controlled closure/adjudication publication, not an invitation for Gate-2 bulk rewrite. Any newly discovered true current defect outside this allowlist requires an explicit, reviewed Gate-1 scope amendment *before* writing that path.

**Gate-1 read-only result and ACCEPTED/PUBLISHED proof:** all 113 Markdown files inspected; all unchecked boxes and active pairings classified; exact six-file Gate-2 content allowlist frozen. Gate-1 matrix/local acceptance commit `c646f3e3ac342cbaef025d115e5be96bd49cf140` changed **only this acceptance Markdown path**, with staged whitespace PASS. Ordinary non-force fast-forward `5a9b803..c646f3e main -> main`; independent GitHub ref and fresh-fetch `HEAD == origin/main == FETCH_HEAD == c646f3e3ac342cbaef025d115e5be96bd49cf140`, clean worktree/index, ahead/behind `0/0`. Hosted GitHub Actions [run 38078396770](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38078396770) completed SUCCESS on exact SHA: Release zero warnings/errors, Core `84/84`, Transport `24/24`, failed/skipped `0/0`, tracked hygiene `252` and non-vacuous diff hygiene PASS. No target, source, workflow, keys or Flash were touched. **Gate-1 ACCEPTED/PUBLISHED; Gate-2 is authorized only within the above six-file scope.**

## Gate 2 — scoped documentation reconciliation

**PASS:** only Gate-1-approved `docs/*.md` and if proven necessary `CHANGELOG.md`, `README.md` or `scripts/README.md` change. `CURRENT_STATE` retains a concise complete snapshot and current physical firmware identity, strips duplicated history *only after* proving canonical scoped retention. Fresh RDC-08 inventory added while historical FDC-10 inventory remains labelled and intact. Corrections target only actually competing present-tense assertions.

**FAIL:** losing a unique gate/evidence fact, rewriting accepted history, introducing a second global source of truth, changing a current product contract, or touching executable files.

**Gate-2 exact candidate review — PASS (local documentation candidate, before Gate-6 commit):** six Gate-1-frozen content paths only: `CURRENT_STATE.md`, `DOCUMENTATION_MODEL.md`, the RDC-07 bootloader readability plan/acceptance pair and this RDC-08 plan/acceptance pair. `CURRENT_STATE.md` is reduced from 29,513 to 13,142 characters (55% less), with preserved named product boundary, accepted physical signed application revision 5, source tree, SHA-256, version floor, 72/72 post-CTR USB INFO hardware evidence, CH340 UART and operator `PHYSICAL_OLED=PASS`, current RDC-08 gating and canonical source map. All **30 removed** former 40-hex IDs, **27 removed** former 64-hex SHA-256 values and **five removed** evidence ZIP filename occurrences remain present in **other tracked Markdown records**; there are no uniquely orphaned SHA/evidence identifiers. The FDC-10 historical `90/30/33` snapshot is preserved, with separate exact `110/40/43` Gate-1 inventory at commit `c646f3e3ac342cbaef025d115e5be96bd49cf140`. RDC-07 Gate-0-now-outdated wording was explicitly historicized without reopening any Gate. No executable or target artifact changed.

## Gate 3 — independent source-of-truth and regression review

**PASS:** independent reread/grep after Gate-2 produces a complete classification of all `[ ]`, `planned`, `next`, `active`, `future`, `deferred` and RDC/FDC status references. Confirm actual relative links and textual filename references, correct plan pairing and explicit supersession of historical claims. Review removed paragraphs against source documents; check no stale requirement reappears.

**FAIL:** any unclassified current contradiction, unreferenced evidence loss, broken active contract reference, stale current gate or false `CLOSED/PUBLISHED` assertion.

**Gate-3 independent cross-document review — PASS (local candidate):** re-read all 113 Markdown sources after candidate edits, no unread/truncated documents and **zero broken relative Markdown links**; validated current physical revision-5 identity, current Gate-2 disposition and one global current-state authority. The unchanged 30 unchecked lines remain classified: three RDC-08 mirror/pointer boxes, 18 deferred UI/RTC/kernel-log boxes, four future generic kernel-service boxes and five future networking boxes. Active RDC governance and RDC-08 pairs match by stem; the same historical/deferred four unmatched plans and one OLED semantic acceptance pair remain classified, not orphaned. C3.9/C4.0 `planned` headings and FDC historical chronology remain explicitly historical; no bulk rewrite or technical contract mutation. Candidate proof does not claim new hardware execution.

## Gate 4 — Git/artifact/security hygiene

**PASS:** exact docs-only diff, real nonempty `git diff --check`, gitignore/attributes policy unchanged and functioning, no tracked generated outputs/archive/keys, current-tree credential-pattern audit, Git-history path-name review and disclosure that content/history scan coverage is bounded. Current source/protocol/linker/test/build paths unchanged from Gate-0 parent.

**FAIL:** forbidden path, private key/credential match not safely adjudicated, tracked binary/evidence artifact, empty/vacuous check or any worktree/index drift.

**Gate-4 hygiene review — PASS (local candidate):** the tracked Git-history path-name inventory spans 252 distinct paths and contains no prohibited build/firmware BIN/ELF/MAP/evidence ZIP/log/standalone key artifact type. Fresh current-tree scan covered 113 Markdown plus 139 other tracked text files; no PEM private-key header, recognizable GitHub/AWS/Slack/Stripe token pattern was found. This is **signature/path coverage only**, not an exhaustive secret-history-content, entropy or third-party credential inspection. `git diff --check` PASS; `.gitignore`, `.gitattributes`, Git policy, firmware, bootloader, Host, tests, script/CI paths and physical target remain unchanged. No hazardous output deletion, USB/VBUS switching, SWD or Flash access occurred.

## Gate 5 — RDC program closure readiness

**PASS:** RDC-01..07 remain historical CLOSED/PUBLISHED; RDC-08 candidate passes Gate 1–4; no unclassified confirmed current technical/tooling/documentation debt; deferred ideas stay deferred; `CURRENT_STATE`/roadmap/program/checklist/changelog are consistent about the *pending publication* until Gate 8 actually passes. Product features remain unpromoted.

**FAIL:** premature claim that RDC-08 or program is CLOSED before commit/push/CI, or automatic feature activation.

**Gate-5 closure-readiness review — PASS / PROGRAM STILL OPEN:** exact current candidate changed-path set is **11 Markdown paths**: six Gate-1-frozen Gate-2 content paths and five separately identified **Gate-5 status/chronology mirrors** (`CHANGELOG.md`, `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md`, `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`, `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md`). Status mirrors correct the previously current `Gate-1 NEXT` wording without claiming Gate-7/Gate-8 publication already happened. `RDC-01..07` remain CLOSED/PUBLISHED, `RDC-08` stays OPEN, deferred consumer ideas remain unpromoted and no new feature is chosen. No new confirmed, unclassified current technical/tooling/documentation blocker emerged in this scoped read-only/code-and-document audit. Branch protection/rulesets remain an operator-configurable governance enhancement unless a required policy exists; absent such policy they are **not automatically a firmware defect**. Gate-5 approval authorizes only exact docs-only Gate-6 staging/commit, **not** treating the entire RDC program as complete.

## Gate 6 — normal docs-only acceptance commit

**PASS:** nonempty exact stage allowlist, independent full patch review, `git diff --cached --check` PASS, normal local commit, exact resulting changed-path set and clean post-commit worktree/index. No force, amend of published history or hidden artifacts.

**FAIL:** scope/content drift or missing proofs.

## Gate 7 — ordinary candidate publication and CI proof

**PASS:** fresh-fetch upstream equal to expected direct parent, normal non-force fast-forward, fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`, exact-published-SHA hosted Host Release/Core/Transport/hygiene PASS. This accepts the candidate but does **not** prematurely assert the later closure-record publication.

**FAIL:** rejected/force push, remote drift, CI failure/pending, missing poststate or premature closure wording.

## Gate 8 — published final closure record

**PASS:** only after Gate-7 acceptance, synchronize `CURRENT_STATE`, `ROADMAP`, the RDC program/checklist, this acceptance record and chronology to record the verified conclusion. Review exact docs-only staged paths, `git diff --cached --check`, commit normally and publish through non-force fresh-fetch equality/clean `0/0`; confirm exact-SHA hosted CI success. At this point `RDC-08 CLOSED/PUBLISHED` and the entire RDC program closure are factual; any next product boundary still requires its **own** authorization.

**FAIL:** claim of final closure before publication, source/tooling path change, or CI/poststate failure.

### Gate-7 publication and Gate-8 final-release predicate

**Gate-8 final-release predicate:** the RDC-08 and overarching RDC program are CLOSED/PUBLISHED **if and only if** this final docs-only Gate-8 commit is actually published by an ordinary non-force fast-forward, fresh fetch proves `HEAD == origin/main == FETCH_HEAD` with clean index/worktree and ahead/behind `0/0`, and GitHub-hosted CI on **that exact Gate-8 commit SHA** completes SUCCESS (Release 0 warnings/errors, Core >=84, Transport >=24, failed/skipped 0 and hygiene PASS). If any condition is unsatisfied, the program remains OPEN. The commit cannot truthfully embed its own future SHA or pre-assert the result of its hosted CI; Git/CI are the live evidence for this conditional status.

**Gate-7 verified evidence:** exact 11-Markdown Gate-6 commit `faabfe75bbb3b805548092c9f298ac862c38ef1d` published by ordinary non-force push `c646f3e..faabfe7`, post-push independent `HEAD == origin/main == FETCH_HEAD == faabfe75bbb3b805548092c9f298ac862c38ef1d` clean `0/0`; GitHub Actions [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) exact-SHA SUCCESS, Release 0 warnings/errors, Core 84/84, Transport 24/24, zero failed/skipped, tracked 252 and non-vacuous hygiene PASS.

**Gate-8 doc-only release candidate:** exact current source-of-truth/program/roadmap/checklist/acceptance/chronology reconciliation, no source/target changes. Final result must be adjudicated only from the future Git publish/fresh-fetch and hosted CI, not preclaimed in this text.

## Mandatory rejection and evidence discipline

Classify failures precisely as `RDC08_PRESTATE_DRIFT`, `RDC08_SCOPE_VIOLATION`, `RDC08_UNCLASSIFIED_CURRENT_DEBT`, `RDC08_HISTORY_LOSS`, `RDC08_SOURCE_OF_TRUTH_CONFLICT`, `RDC08_ARTIFACT_OR_SECRET_HYGIENE`, `RDC08_WHITESPACE_OR_LINK_FAILURE`, `RDC08_PUBLICATION_DRIFT`, `RDC08_CI_FAILURE` or `RDC08_ENVIRONMENT_PROOF_UNAVAILABLE`.

A reported read-only audit is **not** itself an accepted published Gate. Do not claim `PHYSICAL_OLED` was newly tested, reflash firmware, generate synthetic deployable packages, export production keys, change USB power ownership or create gratuitous ZIP/evidence bundles. Record only facts actually verified.
