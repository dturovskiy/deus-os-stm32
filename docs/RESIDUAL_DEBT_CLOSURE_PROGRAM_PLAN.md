# Deus OS — Residual Debt Closure Program Plan

Status: **ACTIVE — RDC-01 CLOSED / RDC-02 NEXT**

Program ID:

`RESIDUAL_DEBT_CLOSURE_PROGRAM`

Baseline:

`d649e133cbe39fa6155520c657136ca99cee61ae`

## 1. Policy

Before any new product feature boundary is promoted, Deus OS must close every **confirmed current technical, tooling/process, maintainability and documentation debt** identified by accepted audits or by reproducible repository evidence.

This program is debt-first maintenance, not feature work.

A future idea is **not** promoted merely because it appears in a backlog. Generic timers, queues, DMA, filesystem, networking, richer observability, low-power/tickless operation and similar consumer-driven capabilities remain deferred until a concrete consumer/problem exists.

A current deficiency is debt when all of the following are true:

1. it exists in the current published repository or operating workflow;
2. it is independently observable/reproducible or already confirmed by an accepted audit;
3. it has a bounded closure criterion;
4. closing it does not require inventing a speculative framework or product feature.

No new Host Management Service/Web/networking or other product-feature boundary may begin until every mandatory RDC item below is CLOSED/PUBLISHED and the final documentation sweep confirms no unclassified current debt remains.

## 2. Mandatory residual-debt ledger

### RDC-01 — CLOSED / PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b` — Asset transaction/session recovery hardening

Confirmed debt:

- request-correlation poisoning is already closed by published `HOST_ASSET_REQUEST_CORRELATION_HARDENING`;
- after BEGIN may have created a volatile transfer session, a WRITE/COMMIT failure can still leave that session alive until target reset or idle timeout;
- the published Asset protocol already provides idempotent `ABORT`, but Host Core does not currently use it for bounded transaction cleanup.

Required boundary:

`HOST_ASSET_TRANSACTION_RECOVERY_HARDENING`

Closure accepted at `699382a58c4c9570cdf36a05693044461e87c58b`. Gate-2 evidence SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA` proves Core `84/84`, warning-clean Release build and zero target/Flash mutation. Host Core now performs one bounded best-effort same-transfer `ABORT` while a volatile session may be active, preserves the primary failure and performs no blind mutation retry.

### RDC-02 — Host dead-surface cleanup

Confirmed current dead remnants include:

- Linux `ValidateManagementTopology()` after profile-based `ValidateTopology(...)` ownership;
- unused `libusb_get_device_address` P/Invoke.

Before deletion, exact current references/history/contracts must be rechecked. Compatibility/operator APIs explicitly retained by published contracts — including raw `SysInfoAsync()` and protocol `EncodeAbort()` — are not dead code and must remain.

Required boundary:

`HOST_DEAD_SURFACE_CLEANUP`

### RDC-03 — Target dead API cleanup

Confirmed declaration/implementation-only target surface currently includes:

- nine unused `application_runtime_*` accessors left after composition-root decomposition;
- `oled_ui_layout_config_v1_apply()`, while current consumers use validate/activate ownership directly.

`binary_frame_encode()` is explicitly retained by the frozen Binary Framed protocol as a checked compatibility API and is excluded.

Required boundary:

`TARGET_DEAD_API_CLEANUP`

Closure requires warning-clean GNU build, map/symbol proof, resource/stack checks and behavior/hardware proof appropriate to the exact target-source candidate.

### RDC-04 — Repository PowerShell destructive-output safety

Confirmed tooling footgun:

- repository-owned build/recovery scripts accept caller-controlled `OutputDir`;
- at least `build_firmware.ps1`, `create_asset_recovery_bundle.ps1` and `create_bootloader_recovery_bundle.ps1` can recursively remove that directory before regeneration.

Required boundary:

`TOOLING_OUTPUT_DIRECTORY_SAFETY`

The fix must be fail-closed, avoid three divergent copy-pasted validators, distinguish current versus historical tooling, and prove allowed/forbidden path semantics under PowerShell 7 before destructive action. Any change to `build_firmware.ps1` must respect firmware candidate-identity consequences.

### RDC-05 — Minimal continuous-integration baseline

Confirmed process debt:

- the repository currently has no tracked CI workflow;
- deterministic host/build/static checks therefore rely entirely on manual/operator execution.

Required boundary:

`REPOSITORY_CI_BASELINE`

Minimum scope: deterministic host Core/Transport tests/builds and repository/static hygiene that can run without bench hardware. CI must not pretend to replace Windows/Mac-mini/STM32 physical acceptance.

### RDC-06 — Remaining kernel composition-root ownership cleanup

Current `src/kernel.c` is approximately 2961 lines after the accepted decomposition and remains a composition/orchestration/console hotspot.

This is not a line-count exercise. The boundary may extract only coherent ownership with an existing reason-to-change and must preserve the accepted anti-framework rules.

Required boundary:

`KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP`

Gate 0 must first re-audit current responsibilities and may reduce/reshape scope if some concentration is demonstrated to be intentional composition-root ownership rather than debt.

### RDC-07 — Bootloader readability / maintainability normalization

`bootloader/bootloader.c` remains a compact security-sensitive implementation (~715 lines) with readability debt from compressed/minified sections.

Required boundary:

`BOOTLOADER_READABILITY_CLEANUP`

This is behavior-preserving only. It must not change protocol, trust, Flash ownership, timing policy or recovery behavior. Proof must include exact source review plus build/disassembly/BIN equivalence appropriate to formatting-only work.

### RDC-08 — Final documentation/source-of-truth debt sweep

After RDC-01..07 close, perform one final repo-wide documentation/governance reconciliation:

- current/next/active wording;
- stale gate/publication states;
- historical versus current assertions;
- open checkboxes;
- current topology/tooling ownership;
- dead-code/debt references;
- artifact/security hygiene;
- `git diff --check`.

Required boundary:

`RESIDUAL_DEBT_DOCUMENTATION_CLOSURE`

This final gate must leave no unclassified current technical/documentation debt before product feature selection resumes.

## 3. Mandatory order

Default order:

1. RDC-01 Asset transaction/session recovery;
2. RDC-02 Host dead-surface cleanup;
3. RDC-03 Target dead API cleanup;
4. RDC-04 tooling OutputDir safety;
5. RDC-05 minimal CI baseline;
6. RDC-06 kernel composition-root cleanup;
7. RDC-07 bootloader readability cleanup;
8. RDC-08 final documentation/source-of-truth sweep.

The order may change only through an explicit docs-only governance amendment that records why dependency/risk ordering changed.

## 4. Anti-goals

This program does not authorize:

- new user-facing product features;
- Host Management Service/Web/network implementation;
- speculative HAL/service-locator/universal context;
- heap/dynamic allocation;
- generic queue/mutex/timer framework without a consumer;
- unrelated protocol or Flash-layout redesign;
- refactoring merely to reduce line count;
- deletion of retained compatibility APIs merely because current call count is zero.

## 5. Per-item rule

Every RDC item gets its own canonical plan + acceptance plan before source/tooling mutation.

Each item must:

- freeze exact baseline and path scope;
- classify product vs harness/environment failures;
- reuse proven primitives;
- provide deterministic proof before hardware proof where applicable;
- commit normally;
- publish only by ordinary non-force fast-forward;
- finish with fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## 6. Exit criterion

`RESIDUAL_DEBT_CLOSURE_PROGRAM` is complete only when RDC-01..RDC-08 are CLOSED/PUBLISHED and the RDC-08 final audit finds no unclassified current technical, tooling/process, maintainability or documentation debt.

Only then may `CURRENT_STATE.md` promote a new product feature boundary.
