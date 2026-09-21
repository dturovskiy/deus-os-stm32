# Deus OS — Documentation Model and Source-of-Truth Precedence

Status: **CANONICAL DOCUMENTATION GOVERNANCE**

This file defines what each documentation class is allowed to mean.

The goal is to prevent several files from independently claiming to be the current project state.

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

After publication it becomes an immutable historical scoped contract.

### Boundary `*_ACCEPTANCE_PLAN.md`

Owns only that boundary’s proof criteria and accepted-gate record.

It does not become the global current-state file after publication.

### Evidence / logs

Own actual run proof:

- build/runtime measurements;
- physical acceptance;
- final hashes;
- failure classification;
- publication proof.

### `DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

Owns trigger-driven possible improvements that are **not active work** until promoted by a new accepted boundary.

### `CHANGELOG.md`

Owns chronology only.

### `MASTER_EXECUTION_CHECKLIST.md`

Execution ledger and historical checklist only.

Unchecked items in historical sections do not automatically become active work.

### `PROJECT_HANDOFF.md`

Operator/handoff/reference material only.

It may point to current state, but must not duplicate or override it.

### `IMPLEMENTATION_PLAN.md`

Historical umbrella implementation rationale and old phase plan.

New active boundaries must use dedicated canonical plans instead.

## 3. Current classification of documentation

### A. Global current-state authority and documentation governance

- `CURRENT_STATE.md` — **sole current-state authority**
- `DOCUMENTATION_MODEL.md` — **documentation-role/precedence governance (this file)**

### B. Stable global contracts

- `ARCHITECTURE.md` — architecture/invariants
- `ROADMAP.md` — forward ordering
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

### D. Accepted protocol/hardware/UI reference contracts

These describe accepted narrow contracts rather than active boundaries:

- `BINARY_FRAMED_TRANSPORT_PROTOCOL.md`
- `OLED_SSD1306_HARDWARE_PROFILE.md`
- `OLED_UI_ACCEPTED_BASELINE.md`
- `OLED_CONSOLE_API_CONTRACT.md`
- `OLED_CONSOLE_ARCHITECTURE.md`
- `OLED_CONSOLE_ACCEPTANCE_PLAN.md`

### E. Historical implementation records

Retained for engineering history and rationale, not current authority:

- `IMPLEMENTATION_PLAN.md`
- `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`
- `FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`
- `MASTER_EXECUTION_CHECKLIST.md`
- `PROJECT_HANDOFF.md`

### F. Deferred future-consumer designs

These are **not active roadmap boundaries**:

- `OLED_UI_LAYOUT_PLAN.md`
- `OLED_STATUS_BAR_PLAN.md`

They may become consumers of the future Asset/Configuration foundation, but only after a dedicated boundary explicitly promotes them.

### G. Gate-0-deferred boundary records

These canonical files record a reviewed boundary that deliberately stopped before implementation:

- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`
- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

Their status is `DEFERRED_NO_REAL_CONSUMER`. They do not authorize Gate 1 or persistent Flash work.

## 4. Current and future boundaries

### Current product implementation state

No product feature implementation boundary is active.

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` completed its initial Gate 0 consumer audit and was deferred because no current target consumer has a frozen schema requiring persistence.

It may be reopened only after a concrete consumer is promoted and Gate 0 is re-frozen.

Infrastructure readiness work such as a repository-owned firmware build entrypoint and read-only MCU/Flash preflight may proceed without turning the deferred product boundary into active implementation.

### Planned after an accepted Asset/Configuration boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`

No canonical plan/acceptance files yet.

### Later

Networking/service extensions.

No canonical implementation boundary is active yet.

## 5. Promotion rule for deferred work

A deferred idea becomes active only when all are true:

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

Live documentation inventory after the Asset/Configuration Gate 0 review:

- Markdown files in `docs/`: `55`;
- same-stem `*_ACCEPTANCE_PLAN.md` records: `18`;
- non-acceptance `*_PLAN.md`-named files: `21`;
- unmatched same-stem plan names: `IMPLEMENTATION_PLAN.md`, `OLED_CONSOLE_IMPLEMENTATION_PLAN.md`, `OLED_STATUS_BAR_PLAN.md`, `OLED_UI_LAYOUT_PLAN.md`;
- all four unmatched plan names are explicitly historical or deferred, so none is an orphan active boundary;
- the Asset/Configuration plan and acceptance files form a matched pair and record `DEFERRED_NO_REAL_CONSUMER`;
- no product implementation boundary is currently active.

This inventory is descriptive governance data, not a replacement for live Git or `CURRENT_STATE.md`.
