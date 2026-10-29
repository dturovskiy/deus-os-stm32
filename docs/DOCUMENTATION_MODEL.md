# Deus OS — Documentation Model and Source-of-Truth Precedence

Status: **CANONICAL DOCUMENTATION GOVERNANCE**

This file defines what each documentation class is allowed to mean.

The goal is to prevent several files from independently claiming to be the current project state.

**Gate acceptance workflow (2026-10-28):** The normative *minimal missing proof* sequence lives only in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`; its worked Gate3 retrospective is off Git at `Reports/Audits/GATE_ACCEPTANCE_FAST_PATH_AND_GATE3_G34B_POSTMORTEM_20261028.md`, indexed in `Reports/INDEX.md`. A new report, Project Source upload or synthetic receipt cannot define a new Gate predicate or replace original native evidence. Future acceptance must reuse previously accepted exact-byte proofs without repeating unchanged checks, independently review the original native evidence for the one missing predicate, and update `CURRENT_STATE` and the scoped checklist only after the separate decision. Project Source attachments are dated snapshots and must be refreshed by replacement, not presumed synchronized with Git.

**Tool-continuity clarification (2026-10-28):** A failed tool response and the actual artifact/evidence state are distinct. For any DEUS MCP task accepted before its result became unavailable, follow `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, section **“Tool-response uncertainty is not a failed artifact”**: independently verify the original target file/ZIP and canonical validation read-only, record only observed phases, and continue unrelated authorized engineering work. This is not a change to current-state authority, evidence requirements, owner GO or explicit tool safety controls. Do not propagate a transient response-fetch error as a product/Gate failure without original evidence.

## 0. Engineering proportionality and working-mode decision — 2026-10-29

**Default goal:** build and test Deus OS on the actual single STM32 development board. Optimize for the shortest reproducible feedback loop **code → relevant build/tests → device update → board smoke test**; avoid generating governance infrastructure as an end in itself. The canonical hands-on workflow is `docs/BENCH_DEVELOPMENT_WORKFLOW.md`.

**Two distinct modes, chosen by the actual task:**

- **BENCH DEVELOPMENT (default):** local firmware coding, board bring-up, development flashing, diagnostics and feature verification. Require only the checks that materially prevent an incorrect/incompatible image, bootloader/recovery damage or misleading result. One concise log is normally enough. Existing toolchains, signed firmware package and update client are reused. A new RA step, trusted-release service, approval ZIP, production nonce, full Gate campaign or independent evidence-auditor is **not** automatically a prerequisite to the ordinary bench loop.
- **FORMAL MIGRATION/RELEASE ACCEPTANCE (explicit scope):** when the requested result is an official Gate PASS, publication, production/distribution security or a separately specified signed physical operator, the original matching acceptance checklist and its original evidence/permission conditions still govern. BENCH evidence is not automatically a Gate PASS. No historical accepted result or pending requirement is silently rewritten by changing work mode.

**Decision discipline:** Before proposing tooling or new tests, identify (a) the concrete user-visible outcome on the board, (b) the shortest usable existing path, and (c) the smallest **unproven safety/compatibility fact** that could prevent it. Add a new framework, gate, runner, auditor, signer, dependency or long report **only when a demonstrated specific defect/requirement cannot be handled by an existing tool or a narrow local fix**. State the measurable benefit and scope first. No extra stages for hypothetical future product scaling. Stop when the requested outcome is observed; avoid retesting exact accepted unchanged inputs.

**Non-negotiable distinctions:** None of this grants implicit permission for reset/Flash, changes bootloader version-floor/recovery requirements, relaxes original evidence for formal acceptance, permits secret exposure, or overrides an explicit tool/client refusal. Request the exact owner GO when a physical state change is about to occur; a previously proposed enterprise approval architecture is not itself automatically that GO.

**Maintenance constraint:** `CURRENT_STATE.md` owns the active mode and current facts, `ROADMAP.md` its priority, `BENCH_DEVELOPMENT_WORKFLOW.md` the short bench procedure. Keep the long RA/Gate investigations as dated **historical or deferred** records. Do not duplicate this policy into new per-run reports or contradictory new sources of truth.

## 1. Precedence

When two sources appear to disagree, use this order:

1. **Git repository state**
   - authoritative for actual bytes, commits, trees, branch state and cleanliness;
   - documentation must never override Git identity.

2. **`docs/CURRENT_STATE.md`**
   - sole global source of truth for the current product/project state;
   - owns latest completed product boundary, active next boundary, current accepted candidates and current deferred-vs-active classification.

3. **Scoped canonical contracts**
   - `docs/ARCHITECTURE.md` for stable architecture/invariants;
   - `docs/ROADMAP.md` for forward sequencing;
   - matching `*_PLAN.md` for a boundary design contract;
   - matching `*_ACCEPTANCE_PLAN.md` for acceptance/proof criteria;
   - protocol/policy/reference documents for their exact narrow domain.

4. **Acceptance evidence/logs**
   - authoritative proof that a particular gate/run actually passed;
   - summaries in Markdown do not replace the accepted evidence.

5. **Historical/reference documents**
   - `CHANGELOG.md`, `MASTER_EXECUTION_CHECKLIST.md`, `PROJECT_HANDOFF.md`, `IMPLEMENTATION_PLAN.md` and historical plan material;
   - useful context, but they do not define current project state.

### Off-Git analytical reports — directory role (2026-10-26)

The workspace-parent sibling `Reports/` stores **non-executable engineering analysis**, not originals required to run an operator and not accepted native evidence. Gate-specific reports use **initiative → Gate → kind**: currently `Reports/ManagementOnlyRuntimeMigration/Gate3/Acceptance/`, `Reports/ManagementOnlyRuntimeMigration/Gate3/Architecture/` and `Reports/ManagementOnlyRuntimeMigration/Gate3/Audits/`. An unrelated feature may have its **own Gate3** without collision. Cross-initiative historical audits stay in `Reports/Audits/`. `Reports/INDEX.md` is an index, and `Reports/REPORT_MIGRATION_MANIFEST_20261026.json` preserves migrated file identities and SHA before/after link updates. These records are **outside the public and private Git repositories**.

`Bundle/` is reserved for canonical operator ZIPs, their source/staging, exact inputs to previously accepted packages, and runner-local configuration. Never relocate `Bundle/_recovered_gate2_host165_exact_20261026/GATE2_EXACT_SOURCE_TREE_RECONSTRUCTION_PROOF.json` merely because it is JSON: it is a **SHA-pinned operator input**. Neither `Reports` nor `Bundle` replaces canonical `CURRENT_STATE.md`, scoped plans, or original `Evidence/*.evidence.zip`; encrypted escrow and signed rescue images remain only in `ProtectedRecovery`. Refer to reports by the fully qualified `Reports/<initiative>/<GateN>/<kind>/<filename>` (or `Reports/Audits/<filename>` for cross-initiative material) and keep direct evidence ZIP hashes in accepted proofs. A report is an analysis, never standalone proof of a hardware Gate PASS.
### Private operator Markdown and vendor-tool documentation (2026-10-26)

The workspace sibling `Tools/` is reserved for installed/extracted **vendor tools, toolchains and their bundled licenses, manifests, examples, README and documentation**. **Do not remove or relocate files inside vendored installations** merely because an extension looks like documentation. The eight historical DEUS-authored `Tools/DEUS_OS_*.md` private operational files have byte-identical historical originals in the existing private documentation repository under `private_docs_archive/operator_docs/legacy_tools_20261009/`; that archive is their **one retained custody location**, not an active operator contract or new project-state source. Immutable history must not be rewritten to 'correct' old hardcoded `Tools/` paths; resolve via the archive index instead.

The private archive holds historical/sensitive runbooks, signer/topology notes and unpublished governance drafts; `Reports/` holds off-Git engineering analysis, while `OS/docs` retains approved public-safe canonical plans and policies. This classification does **not** imply a historical signer runbook is still an executable/current production procedure; owner-approved, source-pinned tool behavior and original evidence govern actual runs. Current source authority remains Git; `CURRENT_STATE.md` remains the only global project-state source.



### Historical evidence quarantine — not an accepted Gate channel (2026-10-27)

`Evidence/Quarantine/<dated-scope>/` is reserved for **byte-preserved historical artifacts with uncertain wrapper/source provenance**, isolated from the ordinary owner-produced `Evidence/*.evidence.zip` namespace. Neither a file's ZIP CRC/SHA-integrity nor an old `FINAL_OUTCOME=PASS` text field attests that it is a canonical original, grants Gate acceptance, or authorizes hardware I/O. Each quarantine directory must contain a custody manifest recording the **full old path/name**, actual exact length/SHA, new path, mismatched claims if any, independent readback and explicit `UNATTESTED` status. Operator/recovery ingestion and Gate counters must never auto-consume quarantined artifacts. Signer images and secrets still belong exclusively in `ProtectedRecovery`, not `Evidence` or `Reports`.

Owner-authorized 2026-10-27 example: `Evidence/Quarantine/2026-10-09_RDC06/` contains the mismatched-wrapper RDC-06 read-only-preflight ZIP and a 54-line historical operator status summary, copied/read back with exact original SHA and removed from the `Tools/` root only afterward; ZIP CRC **135/135** and member SHA **134/134**. Manifest `CUSTODY_MANIFEST_V1.json` SHA `7BA725EE1E1E859902B1E6CC159B53E9775AA9932812CEB59E2716A8F2FB91EB`. Details in `Reports/Audits/TOOLS_MARKDOWN_CUSTODY_DEDUP_AUDIT_20261026.md`. Current product authority remains `CURRENT_STATE.md`; this housekeeping does **not** change Gate3 status.



### Cross-repository source/operator/evidence custody (current owner rule 2026-10-22)

**Artifact-location update (2026-10-23):** `docs/ARTIFACT_CUSTODY_AND_RECOVERY.md` governs only where artifacts are retained and how exact-file hashes are checked. Private `operator_artifacts/RECOVERY_CUSTODY_LEDGER_V1.json` records file identities and last verified custody status, not global Gate state. `scripts/artifact_ledger.py` implements read-only explicit-locator checks. Automatic insertion into existing signing/build workflows has not yet been implemented and requires a separate reviewed change.

The PUBLIC OS Git repository owns actual code, canonical CURRENT_STATE, public-safe policy, acceptance source and SHA/result **references**. GitHub PRIVATE `dturovskiy/deus-os-stm32-private-docs` may retain security-screened **original runnable operator ZIPs**, approved source candidates and necessary documentation, not routine execution evidence. **For new Windows `deus-run` executions, the original `.evidence.zip` and native logs stay in the existing OS-repository sibling `Evidence` directory (same workspace parent as `Tools`/`Logs`, outside both Git histories).** Operator ZIP staging and installed runner search settings are owner-local off-Git configuration; changed lookup roots require independently verified native acceptance. Historical input ZIP originals are not moved automatically. Owner reports separately moving older Downloads evidence to `Evidence` on 2026-10-24; individual moved historical files retain their original hashes only if separately verified at their new location. Neither current nor historical raw evidence is routinely uploaded or mirrored to PUBLIC or PRIVATE GitHub. The distinct protected firmware/signing store is **not** `Evidence`. Normative path resolution and failure handling are in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`. Older evidence/review archives are historical, not a pattern for future storage. Raw logs, keys, escrow/signing and recovery firmware/Flash/SWD images are excluded from Git; separate protected off-Git custody requires its own owner decision.

A ZIP name, hash, ChatGPT attachment, registry or report is not proof of verified private storage. Any approved private runnable ZIP requires non-force Git publication, verified remote readback and original-byte SHA-256; an import failure (including mode-0600 rejection) is a blocker, not custody. Independently verify original evidence **locally** when interpreting a run, but only put compact result, classification, SHA, gate and limitations in Git docs. Normative authoring/retention protocol lives in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` and the executable builder in `scripts/operator_bundle_guard/`. The historical failure index in `docs/HARNESS_FAILURE_REGISTRY.md` does not supersede current-state and acceptance authority.

## 2. Allowed responsibilities

### `CURRENT_STATE.md`

May contain:

- latest completed product boundary;
- accepted firmware/host candidate identities;
- active next boundary and exact next gate;
- high-level planned order after that boundary;
- current deferred/trigger-driven categories;
- current architecture blocker/non-blocker summary;
- pointers to canonical scoped documents.

Must not contain:

- live repository `HEAD` as a hard-coded “current” value;
- full gate-by-gate history;
- detailed protocol specification;
- detailed architecture implementation rationale.

### `ARCHITECTURE.md`

Owns:

- layer/ownership/dependency rules;
- runtime semantics;
- invariants;
- stable hardware/software responsibility boundaries;
- portability and anti-goals.

It must not be the authoritative source for “what gate is next”.

### `ROADMAP.md`

Owns:

- ordering of future boundaries;
- prerequisite relationships;
- long-range phases.

It may record completed milestones for context, but `CURRENT_STATE.md` owns the active/current classification.

### Boundary `*_PLAN.md`

Owns only that boundary’s frozen design contract:

- scope;
- source ownership;
- APIs/protocol changes;
- invariants;
- explicit non-goals;
- gate sequence.

After publication its accepted gate/result record becomes a historical scoped contract. Later documents must not silently rewrite the accepted facts. A clearly labelled **post-publication audit/closure addendum** may be appended when a later audit identifies residual debt that belongs to that scope; such an addendum is forward-looking, does not retroactively invalidate the published boundary and must defer current activation/order to `CURRENT_STATE.md` / `ROADMAP.md`.

### Boundary `*_ACCEPTANCE_PLAN.md`

Owns only that boundary’s proof criteria and accepted-gate record.

It does not become the global current-state file after publication.

### `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`

Owns the current operational development/acceptance topology:

- canonical repository host/path;
- physical owner host for target USB, UART and ST-LINK/SWD;
- Windows -> Linux USB host SSH/SCP coordination boundary;
- target power/VBUS ownership relevant to physical power-cycle acceptance;
- execution-domain mapping rules for multi-host harnesses.

It does not define product acceptance state; `CURRENT_STATE.md` and accepted evidence still own that.

### Evidence / logs

Own actual run proof:

- build/runtime measurements;
- physical acceptance;
- final hashes;
- failure classification;
- publication proof.

### `DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

Owns trigger-driven possible improvements that are **not active work** until promoted by `CURRENT_STATE.md` / `ROADMAP.md` into a concrete closure or feature boundary. Once an item is explicitly promoted, the backlog retains provenance while current activation/order comes from those higher-precedence documents; promoted items must not continue to be described elsewhere as merely optional/deferred.

### `CHANGELOG.md`

Owns chronology only.

### `MASTER_EXECUTION_CHECKLIST.md`

Execution ledger and historical checklist only.

Unchecked items in historical sections do not automatically become active work. A specifically labelled current closure ledger may mirror obligations already promoted by `CURRENT_STATE.md` / `ROADMAP.md`; that mirror is operational tracking, not independent activation authority.

### `PROJECT_HANDOFF.md`

Operator/handoff/reference material only.

It may point to current state, but must not duplicate or override it. Operator bundle format, runner and terminal-result obligations are **normative only in `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`**; `PROJECT_HANDOFF.md` gives a concise invocation and a pointer. Copies uploaded to ChatGPT Project **Sources** are retrieval snapshots, not another independent source of truth: live Git/source bytes, then `CURRENT_STATE.md` for product state, then the domain's canonical document and accepted evidence retain their normal precedence.

### `IMPLEMENTATION_PLAN.md`

Historical umbrella implementation rationale and old phase plan.

New active boundaries must use dedicated canonical plans instead.

## 3. Current classification of documentation

### A. Global current-state authority and documentation governance

- `CURRENT_STATE.md` — **sole current-state authority**
- `DOCUMENTATION_MODEL.md` — **documentation-role/precedence governance (this file)**

The RDC-08 scoped documentation closure record is governed by `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` and `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md`. After the verified Gate-8 release predicate is satisfied these are retained **closed scoped historical records**, never a second global current-state authority or a new product feature.

### B. Stable global contracts

- `ARCHITECTURE.md` — architecture/invariants
- `ROADMAP.md` — forward ordering
- `FLASH_OWNERSHIP_LAYOUT_DECISION.md` — shared internal-Flash ownership/reset/application/metadata/persistence decision; the final map is now realized by the published Firmware Update / Bootloader boundary, while the file remains non-authorizing by itself
- `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` — current canonical development/acceptance host, cable and physical-interface ownership topology
- `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` — harness/evidence operating rules
- `DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` — deferred/trigger policy
- `USB_IDENTITY_POLICY.md` — private-development USB identity policy

### C. Completed/published boundary design + acceptance records

These are complete and retained as scoped historical contracts:

- `SCHEDULER_TIMED_BLOCKING_PLAN.md`
- `SCHEDULER_TIMED_BLOCKING_ACCEPTANCE_PLAN.md`
- `SCHEDULER_FIXED_PRIORITY_PLAN.md`
- `SCHEDULER_FIXED_PRIORITY_ACCEPTANCE_PLAN.md`
- `PRODUCTION_HEARTBEAT_TASK_PLAN.md`
- `PRODUCTION_HEARTBEAT_TASK_ACCEPTANCE_PLAN.md`
- `IWDG_LIVENESS_FOUNDATION_PLAN.md`
- `IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md`
- `NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md`
- `NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`
- `NATIVE_USB_DEVICE_CORE_PLAN.md`
- `NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md`
- `USB_CDC_ACM_CONSOLE_PLAN.md`
- `USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md`
- `SHELL_RPC_FOUNDATION_PLAN.md`
- `SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`
- `BINARY_FRAMED_TRANSPORT_PLAN.md`
- `BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`
- `OS_APPLICATION_AND_UI_MODEL_PLAN.md`
- `OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`
- `BOOT_DESKTOP_UI_PLAN.md`
- `BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`
- `OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`
- `OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`
- `APPLICATION_RUNTIME_FOUNDATION_PLAN.md`
- `APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`
- `KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`
- `KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`
- `KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`
- `USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md`
- `USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md`
- `HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`
- `HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`
- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`
- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`
- `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md`
- `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md`
- `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`
- `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`
- `DESKTOP_PUBLISHED_ASSET_INSPECTOR_PLAN.md` — Host-only consumer; published source/CI and later operator-attested physical fallback UI Gate 3b accepted
- `DESKTOP_PUBLISHED_ASSET_INSPECTOR_ACCEPTANCE_PLAN.md` — scoped read-only fallback evidence; positive committed-layout hardware read remains unverified

### D. Accepted protocol/hardware/UI references and promoted narrow consumer contracts

These describe accepted narrow reference contracts or explicitly promoted consumer contracts rather than source implementation authority:

- `BINARY_FRAMED_TRANSPORT_PROTOCOL.md`
- `FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md` — frozen published firmware-update package/wire ABI
- `ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md` — frozen additive binary transfer ABI consumed by the published Asset boundary
- `ASSET_CONFIGURATION_PERSISTENCE_V1.md` — frozen A/B persistence envelope/atomic selection contract consumed by the published Asset boundary
- `ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` — frozen Flash/SRAM/stack ceilings that governed the published Asset acceptance
- `ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md` — frozen wear/timing/watchdog/USB-continuity contract satisfied by the published Asset boundary
- `ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` — frozen deterministic reset/corruption/power-retention matrix satisfied by published Gate-5 acceptance
- `ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` — frozen candidate-bound ST-LINK recovery/restoration contract satisfied by published Gate-5 acceptance
- `OLED_SSD1306_HARDWARE_PROFILE.md`
- `OLED_UI_ACCEPTED_BASELINE.md`
- `OLED_CONSOLE_API_CONTRACT.md`
- `OLED_CONSOLE_ARCHITECTURE.md`
- `OLED_CONSOLE_ACCEPTANCE_PLAN.md`
- `OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md` — promoted concrete consumer contract consumed by the published Asset boundary; not source implementation authority by itself

### E. Historical implementation records

Retained for engineering history and rationale, not current authority:

- `IMPLEMENTATION_PLAN.md`
- `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`
- `FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`
- `MASTER_EXECUTION_CHECKLIST.md`
- `PROJECT_HANDOFF.md`

### F. Deferred future-consumer / planning references

These are **not active roadmap boundaries**:

- `OLED_UI_LAYOUT_PLAN.md`
- `OLED_STATUS_BAR_PLAN.md`
- `HOST_MANAGEMENT_PRESENTATION_MODEL.md`

The broad OLED plans remain deferred. A deliberately narrow subset has now been separately promoted as `OLED_UI_LAYOUT_CONFIG_V1` in `OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`; that promotion does not activate the rest of the configurable-layout/preset/asset roadmap.

`HOST_MANAGEMENT_PRESENTATION_MODEL.md` is a non-authorizing forward architecture reference: it formalizes CLI/Desktop/Web presentation roles above the accepted Host Control Core without activating Web, networking, packaging/distribution or target-firmware implementation.

### G. Published Asset/Configuration boundary records

These canonical files preserve the original fail-closed deferral, the reopened Gate-0
contract freeze, implementation acceptance and final publication:

- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`
- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

Current status is `GATES 0–7 ACCEPTED / PUBLISHED
562e786ffa734da055c23144ec4256bc8961bbaf`. Campaign A/B, FI-9 idempotency, the
7/7 corruption matrix and one physical VBUS retention proof are accepted; final
persistence wear remains `46/64`. Publication-activation firmware tree
`12f0a0ffaa4597d9ada8b78ecee324d77db79d84` advertises capabilities `0x0000003F`
and passed production `PublishedOnly` hardware smoke. The historical
`DEFERRED_NO_REAL_CONSUMER` result remains part of the record and explains why
implementation was originally blocked.

### H. Published pre-Bootloader recovery records

- `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md`
- `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md`

This boundary is complete/published at `a8f92f83c2ba8917ad183b1a099c9e21199c9463`.

### I. Published Firmware Update / Bootloader boundary records

- `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`
- `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`
- `FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`

This boundary is complete/published at commit `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`, tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`; Gate-7 publication evidence SHA-256 is `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`. Its original published v2 deployment proof remains historical. Subsequent FDC-05..08 hardening is published at `6aa2df19ab02c14bde38833e738fe825008102e8`; at its 2026-10-07 acceptance, the physical bench ran that exact accepted candidate-v3 application (SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`) with capability mask `0x0000007F`. **This is historical FDC-05..08 evidence, not the current physical image:** the subsequent RDC-06 signed application revision 5 is documented solely as the current physical state in `docs/CURRENT_STATE.md` (§4), with original acceptance evidence retained.

## 4. Current and future boundaries

### Current product implementation state

**Current product governance (updated after 2026-10-16):** `DESKTOP_PUBLISHED_ASSET_INSPECTOR` is a completed Host-only product consumer: implemented/published at `4729341e732ffd61e7fe2b0e48973f2c98726a5d` (Core 87/87 and Transport 24/24 source/CI PASS); Ubuntu USB/CLI Gate 3b.1 PASS; real connected Windows Avalonia manual published-asset fallback Gate 3b **operator-attested PASS**. The positive physical read of a previously committed OLED layout remains **NOT_VERIFIED** and is not part of the closed fallback acceptance. The last completed **target-resident** named feature remains `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`; subsequent FDC/RDC are maintenance closures. **`MANAGEMENT_ONLY_RUNTIME_MIGRATION` has since completed 16/16 scoped Gate-1 offline checks; the only active next step is Gate-2 offline reconciliation, not an installed firmware or a physical authorization.** Exact current status and evidence boundary remain solely in `docs/CURRENT_STATE.md`; this file governs document roles, not gate decisions. Earlier proposed service EXE / `HostManagementRuntime` wrapper remain **`NO_GO_UNCONSUMED_RUNTIME_WRAPPER`**. Web/HTTP, TUI and network require separately promoted scopes and security/acceptance decisions; `CURRENT_STATE.md` alone owns the current project disposition.

**2026-10-20 historical Gate-0 activation checkpoint (superseded by later Gate-1 acceptance):** `MANAGEMENT_ONLY_RUNTIME_MIGRATION_PLAN.md` and `MANAGEMENT_ONLY_RUNTIME_MIGRATION_ACCEPTANCE_PLAN.md` are **tracked, Gate-0-published** scoped design and evidence-adjudication records, not additional global project-state sources. They preserve real experimental candidate `000E` offline builds and current `000C` read-only physical preflight but have **formal Gate 0 ACCEPTED/PUBLISHED**; at that historical checkpoint `ACTIVE_GATE=GATE1_OFFLINE` (later succeeded by scoped 16/16 Gate-1 acceptance and Gate-2 offline reconciliation), and still no firmware flash authorization. The bounded design was approved after the 2026-10-20 conflict audit, then the paired contracts and global status/policy were published in `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09` with [exact-SHA CI 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) **SUCCESS**. Gate-1 source qualification remains a distinct acceptance boundary; Gate 4 requires separate owner GO. Diagnostic allowlist/recovery decisions cannot be inferred from preliminary checked evidence or from selecting an architecture direction.

### Later

The broad next roadmap remains Host-management consumer/UI development, followed by separately gated network/security extensions. A **concrete user-facing consumer and its operating/security contract** must be promoted before implementation; no speculative library/daemon is authorized just because previous documentation gates passed.

Host-management presentation evolution is described by `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`. That reference does not itself promote a Web/service/network boundary.

## 5. Promotion rule for deferred work

A deferred idea or future feature becomes active only when all are true, **and only after all mandatory confirmed-debt programs are closed**:

1. a concrete consumer/problem exists;
2. `CURRENT_STATE.md` names it as the active next boundary or an accepted prerequisite;
3. a dedicated canonical plan freezes scope and non-goals;
4. an acceptance plan freezes proof criteria.

A `PLANNED` or `DEFERRED` standalone document does not authorize implementation by itself.

## 6. Publication rule

When a boundary is published:

- its plan/acceptance become historical scoped contracts;
- `CURRENT_STATE.md` advances to the next boundary;
- `ROADMAP.md` may record the completed milestone/order;
- `CHANGELOG.md` records chronology;
- execution/handoff ledgers may be updated for history;
- old documents must not retain competing “current” or “next” authority.

## 7. Anti-duplication rule

Do not repeat exact current-state facts across README, architecture, roadmap, handoff, implementation plan and checklist.

Those files should link to `CURRENT_STATE.md` instead.

Duplication is allowed only when the value is intrinsic to the scoped contract itself, such as an accepted boundary commit or protocol version inside that boundary’s canonical plan.


## 8. Audit inventory

**Historical FDC-10 inventory snapshot (not a live count):**

- Markdown files in `docs/`: `90`;
- `*_ACCEPTANCE_PLAN.md` records: `30`;
- non-acceptance `*_PLAN.md`-named files: `33`;
- unmatched same-stem plan names: `IMPLEMENTATION_PLAN.md`, `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`, `OLED_STATUS_BAR_PLAN.md`, `OLED_UI_LAYOUT_PLAN.md`;
- the sole unmatched same-stem acceptance name is `OLED_CONSOLE_ACCEPTANCE_PLAN.md`, which is the historical semantic pair for `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`;
- the remaining unmatched plan names are explicitly historical or deferred, so none is an orphan active boundary;
- `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` is the canonical operational topology reference, not a product boundary plan;
- the Asset/Configuration plan and acceptance files form a matched published pair and preserve the historical `DEFERRED_NO_REAL_CONSUMER` record plus final Gates 0–7 publication;
- the pre-Bootloader recovery plan and acceptance files form a matched published pair at `a8f92f83c2ba8917ad183b1a099c9e21199c9463`;
- `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` and `_ACCEPTANCE_PLAN.md` form a matched published pair; `FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md` is their frozen published wire/image ABI contract;
- `HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` and `_ACCEPTANCE_PLAN.md` form the closed/published `FDC-01` pair at `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`;
- `HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` and `_ACCEPTANCE_PLAN.md` form the closed/published `FDC-02` pair at `42245d9d71504482fb189d8351ecce7049542145`;
- `HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md` and `_ACCEPTANCE_PLAN.md` form the closed/published `FDC-03` pair at `0d9adfd8d0ed11478194e2268ede3c57379c8294`;
- `HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md` and `_ACCEPTANCE_PLAN.md` form the closed/published `FDC-04` pair at `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`;
- `HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` and `_ACCEPTANCE_PLAN.md` form the closed/published `FDC-09` pair at `b88a9eee43095665326787cc0345822218c1ba73`;
- `KERNEL_COMPOSITION_ROOT_CONVERGENCE`, `SEMANTIC_SYSTEM_SERVICE_STATE`, `APPLICATION_STOP_FAILURE_HARDENING` and `TARGET_UPDATE_ROBUSTNESS_CLOSURE` plan/acceptance pairs are closed/published together at `6aa2df19ab02c14bde38833e738fe825008102e8` as FDC-05..08;
- `DOCUMENTATION_CONSISTENCY_CLOSURE` is the final FDC-10 docs-only pair, published at `9c02e27f50349c9a0240ab24d34720580c3f3269`, and records the repo-wide reconciliation that completed the ten-item closure program;
- `RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` and `_ACCEPTANCE_PLAN.md` own historical post-FDC debt-first governance; their final RDC-08 Gate-8 release predicate determines when the mandatory program becomes CLOSED/PUBLISHED and product-feature **selection** becomes eligible;
- at that historical inventory checkpoint, the last **target-resident** named feature was `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`; later FDC commits were hardening/closure. The subsequently accepted Host-only Desktop Inspector is recorded in §4 above;
- at the 2026-10-10 RDC-08 closeout, FDC-01..10 and RDC-01..08 were CLOSED/PUBLISHED; the exact Gate-8 predicate was fulfilled at `c309428b03615a9afd514e3c7c8886a82a5112e9` ([hosted CI 38079465787](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079465787) SUCCESS). Later Desktop Inspector source/CI and physical read-only fallback acceptance are a separate Host product event. Future service/Web/network work still requires explicit promotion, plan/acceptance and security review.

This inventory is descriptive governance data, not a replacement for live Git or `CURRENT_STATE.md`.

### RDC-08 Gate-1 snapshot — 2026-10-10 (frozen input `c646f3e3ac342cbaef025d115e5be96bd49cf140`)

- `git ls-files` at the published Gate-1 inventory commit: **252 tracked files**, **113 Markdown files** in total, of which **110 are in `docs/`** and three are `README.md`, `CHANGELOG.md`, `scripts/README.md`.
- `docs/` contains **40 `*_ACCEPTANCE_PLAN.md`** and **43 other `*_PLAN.md`** files. Exactly **four plan names** have no same-stem acceptance: `IMPLEMENTATION_PLAN.md` (historical umbrella), `OLED_CONSOLE_IMPLEMENTATION_PLAN.md` (historically paired by meaning with `OLED_CONSOLE_ACCEPTANCE_PLAN.md`), `OLED_STATUS_BAR_PLAN.md` and `OLED_UI_LAYOUT_PLAN.md` (deferred UI proposals). The only unmatched same-stem acceptance filename is `OLED_CONSOLE_ACCEPTANCE_PLAN.md`. **No active orphan plan or acceptance exists.**
- RDC-08 scoped maintenance pair: `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` / `_ACCEPTANCE_PLAN.md` (Gate-0 through Gate-7 accepted/published, Gate-8 closure conditioned on actual final publication and exact-SHA CI). The residual-debt program pair is the governance/exit-criterion authority and becomes historical after that same release predicate succeeds; all other completed/published pairs remain scoped historical contracts or deferred proposals.
- The exhaustive path-linked 113-source classification table and all **30** still-open checkbox entries are recorded in `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` (Gate-1). Its findings are **audit evidence at the stated Git commit**, not an instruction to treat counts as timeless live facts.
- The FDC-10 `90/30/33` figures above are deliberately preserved as valid historical audit provenance rather than overwritten by the new snapshot.

## 9. Public/private publication boundary (new independent review)

**Documentation authority is not a license to publish all underlying engineering evidence.** The completed FDC-10/RDC-08 classifications apply to repository consistency and traceability; they do not decide which files should remain public. `PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` and `_ACCEPTANCE_PLAN.md` own the proposed external-release classification, private preservation, history/hosting exposure audit and explicit approval gates. At the historical Gate-0 checkpoint, scope was accepted/published at `558306fe618d1ee69240fa11d24437cd38fb8e2b` with exact-SHA hosted CI `38080940176`; **that checkpoint** authorized only a Gate-1 read-only classification, not deletion or a public-history purge. The later owner-bounded prospective-only Gate-2–7 process **completed**; actual closure is documented in `docs/PUBLIC_REPOSITORY_SANITIZATION_FINAL_ACCEPTANCE.md`, without rewriting the old Git history. The three new `PUBLIC_REPOSITORY_SANITIZATION_GATE1_*` reports are **frozen baseline audit evidence** (current-tree triage, public hosting/history coverage, static dependency inventory), not competing current-state authorities, credential-clearance certificates or permissions for Gate-2 migration. Their 254-path count is correct at their exact baseline even after new documentation is added. Preserve the same single current-state authority and stable ABI/engineering evidence *in the appropriate approved public/private view*, with exact provenance mapping. Never expose private details by copying them into a supposedly sanitized public plan, index, log or CI artifact. A Git `.gitignore` rule does not remove already tracked or historic content.
