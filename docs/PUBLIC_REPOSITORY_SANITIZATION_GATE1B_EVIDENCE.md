# Deus OS — Gate-1B Read-Only Consumer/Risk Evidence Ledger

> Status: **PARTIAL VERIFIED EVIDENCE / OWNER DECISIONS OPEN — NOT GATE-1 PASS**. No authorized migration, redaction, Git rewrite, STM32 action or firmware/script edit.

## Snapshot and evidence authority

- Git prestate: `300747134dbcd9e72687192b6e81fa9a72f995d9`, clean tracked index/worktree, `main` tracking `origin/main` with ahead/behind `0/0`; read-only audit of 258 tracked files. This ledger is a new output after that snapshot, not retroactively a 259th input.
- Frozen Gate-1A source matrix: `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`; 254/254 original rows, 141 PUBLIC / 15 REDACT / 49 PRIVATE / 49 RETAIN. This report does **not** rewrite those historical dispositions.
- The 98 rows below are all Gate-1A tentative PRIVATE and RETAIN cases. Original `Owner` field is a **role** (ACCEPTANCE/DESIGN/OPERATIONS/DOCS), not a named accountable approver. No personal owner has signed a publication/privacy decision.

## Evidence and corrected dependency interpretation

- The current HEAD texts of all 115 pre-Gate-1 Markdown paths (excluding four later `PUBLIC_REPOSITORY_SANITIZATION_GATE1_*` reports) were completely re-read; these are paths from the frozen inventory, not byte-identical historic snapshots. the 98 path decisions were compared against textual basename references from these baseline documents. Seven governance/ledger sources were then excluded from the `Non-ledger Markdown mentions` count: `CHANGELOG.md`, `DOCUMENTATION_MODEL.md`, `MASTER_EXECUTION_CHECKLIST.md`, `ROADMAP.md`, `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md`, and the two `PUBLIC_REPOSITORY_SANITIZATION` parent plan/acceptance documents.
- PRIVATE: **49** rows, **14** governance-only, **35** with other Markdown citations, **7** cited by README/script README. RETAIN: **49** rows, **6** governance-only, **43** other-Markdown-cited, **2** README/script-README-cited.
- Direct source/test/build configuration searches found no basename mentions of any of the 98 documents outside Markdown in the 258-path HEAD tracked tree. This does not rule out dynamically constructed paths, humans following prose instructions, external operator scripts or archival/evidence consumers.
- Literal Markdown relative-link parse over the current texts of the 115 baseline-path Markdown documents found seven explicit links, all targeting the existing `docs/OLED_UI_ACCEPTED_BASELINE.md`; no missing relative-link target. Most other `*.md` references are inline-code/text rather than actual Markdown links. A zero-broken-links result is **not** a sufficient removal proof.
- Static code/reference integrity: 64 C/assembly/header sources; all 94 local quoted includes resolved in the tracked `src/`, `include/` or `bootloader/` tree. Seven Host `.csproj` and one `.sln` have 19 resolvable project/solution references. This is not an ARM/Host rebuild or ABI equivalence proof.
- A local .NET environment query succeeded, but a separate attempted Host Release build returned a **control-plane ambiguous outcome** rather than an accepted build/test result. No local build PASS is claimed; exact-commit hosted CI remains required for accepting the docs-only publication.

## Historical and hosting boundary

- `main`: 143 reachable commits, zero merge commits; local `--all`: 146 commits including three reachable only via local stash, which is **not** public publication evidence. No stash content is included in this public ledger.
- `git log -p main` reviewed as bounded groups, with an initially truncated six-commit batch independently re-read in six single-commit operations. All 143 commit patch texts were received without truncation in the final accounting. Regex screening of added/removed diff lines found no recognized PEM-private-key header, common GitHub/AWS/OpenAI token prefixes, evident quoted password/API-key assignment or Bearer header. Historical operator paths **are present**. This is **not** a full bytewise/entropy/content-type review of every reachable blob or inaccessible object, nor a credential-compromise verdict.
- Independent `git log --raw --abbrev=40 main` counted **1,356 distinct reachable historical blob OIDs** and **1,357 add/modify change events** (258 additions, 1,099 modifications; no deletions or merges). `git log --numstat main` reported **10 binary-diff events**, all on early `CHANGELOG.md` versions. All 10 corresponding blob objects were fetched by SHA from GitHub and signature-screened without truncation; nine contain one NUL character and none matched the selected key/token signatures. These objects are **not silently cleared as ordinary text diffs**. The other historical blob objects have not been independently read/entropy-scanned one-by-one, so exhaustive per-blob Gate-1 proof remains OPEN.
- GitHub at prestate: one public `main`, zero current tags/releases/PRs/issues, 25 listed Actions runs (24 success, one failed). All 25 *currently enumerated* run-artifact lists returned zero artifacts. Expired/deleted artifacts and unlisted history are not excluded. The earliest failed run has no returned job log. GitHub caches, unreachable Git objects and third-party clones/mirrors/search caches remain **NOT_CHECKED**.

## Seven PowerShell defaults — verified source-level behavior, not tested execution

- `build_bootloader.ps1`, `build_firmware.ps1`: caller-overridable `$ToolchainRoot` with a workstation-specific default. Build paths/compile invariants must be preserved when proposing relocation.
- `create_asset_recovery_bundle.ps1`, `create_bootloader_recovery_bundle.ps1`: build/copy generated recovery wrappers containing fixed `$ProgrammerCli` defaults; wrappers forward the supplied override to `recovery-common.ps1`. The Asset recovery path is historical/non-operational, not current acceptance recovery.
- `stm32_asset_recovery.ps1`, `stm32_bootloader_recovery.ps1`, `stm32_readonly_flash_preflight.ps1`: caller-overridable programmer CLI path with workstation-specific defaults; preflight has read-only safety gating. Editing the defaults is an executable behavior change that requires separate proof.
- No executable source/script, generated package, key, signing identity, Flash, USB or physical STM32 state was changed in this audit.

## Per-file review ledger — no decisions implied

Definitions: `Non-ledger Markdown mentions` = number of other baseline Markdown files (excluding seven governance sources) that textually mention the exact file basename; this is *not* proof of a functional consumer. `Readme` = referenced from either tracked README. `Proposed handling` denotes a verification route, **not** a committed publication or private-transfer decision. `OWNER_PENDING` means a contract/privacy owner and evidence-preservation authority still need to approve final treatment.

| Tracked path | Gate-1A class | Owner role | Non-ledger Markdown mentions | Readme | Existing flags | Proposed handling | Sign-off |
|---|---|---|---:|:---:|---|---|---|
| `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` | PRIVATE | OPERATIONS | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md` | RETAIN | DOCS | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` | RETAIN | DOCS | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` | PRIVATE | OPERATIONS | 3 | Y | none | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | HOST_ALIAS | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/BINARY_FRAMED_TRANSPORT_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` | RETAIN | DESIGN | 1 | N | WIN_PATH+HOME_PATH | KEEP_PENDING_REDACTION | OWNER_PENDING |
| `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/BOOT_DESKTOP_UI_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` | PRIVATE | OPERATIONS | 10 | Y | WIN_PATH+HOME_PATH+HOST_ALIAS | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 3 | Y | HOST_ALIAS | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 5 | Y | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md` | PRIVATE | OPERATIONS | 7 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` | PRIVATE | OPERATIONS | 7 | Y | WIN_PATH+HOME_PATH+HOST_ALIAS | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 3 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 5 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_DEAD_SURFACE_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/HOST_DEAD_SURFACE_CLEANUP_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` | RETAIN | DESIGN | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md` | RETAIN | DESIGN | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` | RETAIN | DESIGN | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/IMPLEMENTATION_PLAN.md` | PRIVATE | OPERATIONS | 11 | Y | none | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/IWDG_LIVENESS_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md` | PRIVATE | OPERATIONS | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/MASTER_EXECUTION_CHECKLIST.md` | PRIVATE | OPERATIONS | 14 | Y | none | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/NATIVE_USB_DEVICE_CORE_PLAN.md` | RETAIN | DESIGN | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md` | RETAIN | DESIGN | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/OLED_CONSOLE_API_CONTRACT.md` | RETAIN | DOCS | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_CONSOLE_ARCHITECTURE.md` | RETAIN | DOCS | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_SSD1306_HARDWARE_PROFILE.md` | RETAIN | DOCS | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_STATUS_BAR_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_UI_ACCEPTED_BASELINE.md` | RETAIN | DOCS | 7 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md` | RETAIN | DOCS | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OLED_UI_LAYOUT_PLAN.md` | RETAIN | DESIGN | 9 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 3 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` | RETAIN | DESIGN | 7 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/PRODUCTION_HEARTBEAT_TASK_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/PRODUCTION_HEARTBEAT_TASK_PLAN.md` | RETAIN | DESIGN | 0 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/PROJECT_HANDOFF.md` | PRIVATE | OPERATIONS | 11 | Y | WIN_PATH+HOST_ALIAS | ARCHIVE+PUBLIC_ENTRY | OWNER_PENDING |
| `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/REPOSITORY_CI_BASELINE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` | RETAIN | DESIGN | 5 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 3 | N | WIN_PATH+HOME_PATH | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` | RETAIN | DESIGN | 1 | N | WIN_PATH+HOME_PATH | KEEP_PENDING_REDACTION | OWNER_PENDING |
| `docs/SCHEDULER_FIXED_PRIORITY_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/SCHEDULER_FIXED_PRIORITY_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/SCHEDULER_TIMED_BLOCKING_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/SCHEDULER_TIMED_BLOCKING_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/SHELL_RPC_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 2 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/TARGET_DEAD_API_CLEANUP_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/TARGET_DEAD_API_CLEANUP_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 0 | N | none | ARCHIVE+PROVENANCE_REVIEW | OWNER_PENDING |
| `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` | RETAIN | DESIGN | 2 | Y | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 1 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/USB_CDC_ACM_CONSOLE_PLAN.md` | RETAIN | DESIGN | 1 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md` | PRIVATE | ACCEPTANCE | 2 | N | none | ARCHIVE+CONSUMER_REVIEW | OWNER_PENDING |
| `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` | RETAIN | DESIGN | 3 | N | none | KEEP_PENDING_ABI_REVIEW | OWNER_PENDING |

### Proposed handling code

- `ARCHIVE+PUBLIC_ENTRY`: archive only after ensuring a safe public replacement for README/onboarding and recovery/safety references; verify restored provenance and hyperlinks.
- `ARCHIVE+CONSUMER_REVIEW`: archive candidate only after extracting necessary publicly consumed contracts and updating reference graphs; preserve accepted evidence.
- `ARCHIVE+PROVENANCE_REVIEW`: governance-only basename citations do not imply that evidence may be deleted; check retention, original evidence and source-of-truth links.
- `KEEP_PENDING_REDACTION`: retain until possible operator-specific values are minimized without altering build/recovery/safety semantics.
- `KEEP_PENDING_ABI_REVIEW`: retain until API/protocol/safety and external-onboarding consumer needs are decided; no automatic promotion to PUBLIC.

## Acceptance blockers and next proof

1. **98/98 owner publication decisions remain OPEN**: no individual privacy/contract sign-off. PRIVATE is *candidate archive routing*, RETAIN is *hold*, not an approved final publication plan.
2. Preserve public protocol, firmware, Host CLI and recovery contracts with explicit published replacement targets. At least nine README-mentioned PRIVATE/RETAIN files require re-link adjudication before relocation.
3. Git-history patch screening is not equivalent to a full blob byte/entropy audit, and external clones, historical caches and unreachable/deleted surfaces remain unverified.
4. Windows/ARM/recovery executable equivalence and independent restoration of complete evidence must precede Gate-2 acceptance or Gate-4 edits; Host CI SUCCESS alone does not establish them.
5. Any Git-history rewrite or new-public-repository strategy is reserved for Gate-3 and requires separate explicit authorization.

**Verdict: Gate-1B evidence progress VERIFIED; Gate-1B final owner/security/consumer adjudication OPEN; full Gate-1 OPEN; Gate-2 NOT AUTHORIZED.**
