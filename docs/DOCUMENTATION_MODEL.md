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

After publication its accepted gate/result record becomes a historical scoped contract. Later documents must not silently rewrite the accepted facts. A clearly labelled **post-publication audit/closure addendum** may be appended when a later audit identifies residual debt that belongs to that scope; such an addendum is forward-looking, does not retroactively invalidate the published boundary and must defer current activation/order to `CURRENT_STATE.md` / `ROADMAP.md`.

### Boundary `*_ACCEPTANCE_PLAN.md`

Owns only that boundary’s proof criteria and accepted-gate record.

It does not become the global current-state file after publication.

### `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`

Owns the current operational development/acceptance topology:

- canonical repository host/path;
- physical owner host for target USB, UART and ST-LINK/SWD;
- Windows -> Mac-mini SSH/SCP coordination boundary;
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

It may point to current state, but must not duplicate or override it.

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

No product feature boundary is currently active. The last named product-feature boundary remains `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`; post-publication hardening `FDC-01..09` is published, including shared FDC-05..08 commit `6aa2df19ab02c14bde38833e738fe825008102e8`, and FDC-10 documentation/source-of-truth reconciliation is published docs-only at `9c02e27f50349c9a0240ab24d34720580c3f3269`. A subsequent audit has promoted `RESIDUAL_DEBT_CLOSURE_PROGRAM` as the mandatory current maintenance sequence. Closure of FDC-01..10 therefore does not yet permit product feature promotion; `docs/CURRENT_STATE.md` owns the active RDC item/order.

### Later

Networking/service/security extensions remain the next broad roadmap area but are not active until a concrete consumer/problem is promoted through the normal plan + acceptance-plan rule.

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

Live documentation inventory at FDC-10 final reconciliation:

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
- latest named product-feature boundary remains `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`; later FDC commits are hardening/closure, not a new product-feature boundary;
- no new product feature boundary is currently active. `FDC-01..FDC-10` are accepted. `RDC-01..RDC-07` are CLOSED/PUBLISHED; `RDC-08` and the entire program close only on externally verified Gate-8 publication/fresh-fetch/exact-SHA hosted CI. Future service/Web/network work requires a distinct explicit boundary even after closure.

This inventory is descriptive governance data, not a replacement for live Git or `CURRENT_STATE.md`.

### RDC-08 Gate-1 snapshot — 2026-10-10 (frozen input `c646f3e3ac342cbaef025d115e5be96bd49cf140`)

- `git ls-files` at the published Gate-1 inventory commit: **252 tracked files**, **113 Markdown files** in total, of which **110 are in `docs/`** and three are `README.md`, `CHANGELOG.md`, `scripts/README.md`.
- `docs/` contains **40 `*_ACCEPTANCE_PLAN.md`** and **43 other `*_PLAN.md`** files. Exactly **four plan names** have no same-stem acceptance: `IMPLEMENTATION_PLAN.md` (historical umbrella), `OLED_CONSOLE_IMPLEMENTATION_PLAN.md` (historically paired by meaning with `OLED_CONSOLE_ACCEPTANCE_PLAN.md`), `OLED_STATUS_BAR_PLAN.md` and `OLED_UI_LAYOUT_PLAN.md` (deferred UI proposals). The only unmatched same-stem acceptance filename is `OLED_CONSOLE_ACCEPTANCE_PLAN.md`. **No active orphan plan or acceptance exists.**
- RDC-08 scoped maintenance pair: `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` / `_ACCEPTANCE_PLAN.md` (Gate-0 through Gate-7 accepted/published, Gate-8 closure conditioned on actual final publication and exact-SHA CI). The residual-debt program pair is the governance/exit-criterion authority and becomes historical after that same release predicate succeeds; all other completed/published pairs remain scoped historical contracts or deferred proposals.
- The exhaustive path-linked 113-source classification table and all **30** still-open checkbox entries are recorded in `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` (Gate-1). Its findings are **audit evidence at the stated Git commit**, not an instruction to treat counts as timeless live facts.
- The FDC-10 `90/30/33` figures above are deliberately preserved as valid historical audit provenance rather than overwritten by the new snapshot.
