# Deus OS — Host Session State-Event Reentrancy Hardening Acceptance Plan

Status: **GATES 0–3 ACCEPTED / GATE 4 LOCAL ACCEPTANCE COMMIT PENDING / `FDC-02`**

Canonical design: `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md`

Baseline commit: `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`

## Gate 0 — contract/source-boundary freeze

Gate 0 is documentation-only.

Required:

- root cause is frozen as synchronous execution of arbitrary `StateChanged` subscriber code while the session operation gate is owned;
- `DeusDeviceSession` remains the sole lifecycle/state owner;
- `State` mutation remains serialized under the existing session operation model;
- public `StateChanged` signature remains unchanged;
- notification callbacks are selected to run asynchronously outside the session critical section;
- one private per-session serialized notification owner is selected; no global dispatcher/event framework;
- transition order is preserved without coalescing distinct states;
- callback completion is observational and never awaited by lifecycle operations;
- callback exceptions are isolated from session state and later notifications;
- a callback may invoke another session operation without deadlocking the current owner;
- Dispose remains bounded and does not wait for arbitrary subscriber completion;
- exact authorized source/test paths are frozen;
- Desktop/native transport/target mutation is explicitly excluded.

Gate 0 passes only when `CURRENT_STATE`, `ROADMAP`, `MASTER_EXECUTION_CHECKLIST`, `DOCUMENTATION_MODEL` and the Host Control addendum point to this exact active slice.

## Gate 1 — implementation/static ownership review

Allowed source/tests are exactly:

- `host/src/DeusOs.Control.Core/DeusDeviceSession.cs`
- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`

Static acceptance requires:

- no subscriber invocation executes inline from `SetState()`;
- `_operationGate` remains the session mutation/client-ownership serializer;
- exactly one per-session notification serialization mechanism exists;
- no new public event API or presentation dependency enters Core;
- callback exception handling cannot fault the lifecycle operation/dispatcher;
- callback work is not awaited by Connect/Execute/recovery/Disconnect/Dispose;
- dispose ordering cannot convert a reentrant callback into an operation-gate disposal race;
- FDC-01 channel/RPC/firmware-update source remains byte-unchanged;
- no source outside the frozen Host Core/test path set changed.

## Gate 2 — deterministic host validation

Required Core tests:

- exact connect state order `Discovered -> Opening -> Negotiating -> Ready`;
- reentrant subscriber session operation completes without deadlock;
- synchronous subscriber-triggered disconnect completes and reaches `Disconnected`;
- automatic recovery state transitions remain ordered through `Recovering -> Discovered -> Opening -> Negotiating -> Ready`;
- throwing subscriber is isolated and later subscriber/later notifications still execute;
- Disconnect/Dispose race is bounded, idempotent and deadlock-free;
- unsubscribe semantics match the frozen contract;
- all existing FDC-01 correlation and previous Core tests remain PASS.

Every deadlock-sensitive test must have a bounded deadline and fail diagnostically instead of hanging indefinitely.

Validation requirements:

- all `DeusOs.Control.Core.Tests` PASS; exact final count recorded in evidence;
- `DeusOs.Control.Core` Release build PASS;
- `DeusOs.Control.Desktop` Release build PASS;
- `git diff --check` PASS;
- exact changed path set equals the frozen FDC-02 source/test set plus canonical docs only;
- no target I/O, Flash, firmware, bootloader or native transport mutation.

## Gate 3 — documentation/closure reconciliation

After Gate 2 PASS:

- mark FDC-02 implementation/test sub-checkboxes `[x]` in `MASTER_EXECUTION_CHECKLIST`;
- record exact changed source/test paths, test count and build results;
- update `CURRENT_STATE` so Slice A proceeds to the next remaining host closure item rather than claiming Host Management Service is unblocked;
- update `ROADMAP` FDC-02 disposition to closure/published state while preserving remaining FDC items;
- update this acceptance record with exact evidence identity;
- preserve historical Host Control and FDC-01 acceptance unchanged.

## Gate 4 — local acceptance commit

Require:

- only accepted FDC-02 source/tests/docs staged;
- no FDC-01/native transport/Desktop source/target drift;
- `git diff --cached --check` clean;
- one normal local commit;
- clean worktree/index after commit;
- remote still direct parent before publication.

## Gate 5 — ordinary non-force publication

Require fresh remote-state/direct-parent proof and ordinary non-force push only.

Post-push require:

- `HEAD == origin/main == FETCH_HEAD` after fresh synchronization;
- clean worktree/index;
- ahead/behind `0/0`;
- GitHub `main` independently resolves to the accepted commit.

## Failure classifications

At minimum distinguish:

- `FDC02_SOURCE_SCOPE_DRIFT`
- `FDC02_INLINE_CALLBACK_REGRESSION`
- `FDC02_REENTRANCY_DEADLOCK`
- `FDC02_NOTIFICATION_ORDER_FAILURE`
- `FDC02_CALLBACK_EXCEPTION_ISOLATION_FAILURE`
- `FDC02_DISPOSE_RACE_FAILURE`
- `FDC02_CORE_TEST_FAILURE`
- `FDC02_BUILD_FAILURE`
- `FDC02_DOC_RECONCILIATION_FAILURE`

A failure does not authorize Desktop/native-transport/target changes or folding `FDC-03`, `FDC-04` or `FDC-09` into this slice.


## Accepted Gate-2 / Gate-3 record — 2026-10-04

Authoritative evidence is `stm32_os_fdc02_host_validation_dotnet_v2_20261004_162345.evidence.zip`, SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`.

The evidence has clean ZIP CRC and `72/72` exact manifest ownership. It exact-binds pre/post repository state to `HEAD == origin/main == 0f32c7c22a2526e5ce53dd877e5cce25209324e9`, exactly two modified tracked paths, staged `0`, untracked `0`, and unchanged source hashes after validation.

Accepted source/test hashes:

- `DeusDeviceSession.cs` — `6E39B68DE0D82906AEFAFDE376E49AD824BA9959332E2C79EACCA34CCB13F03F`;
- `ClientTests.cs` — `7FD188B31FA9A1C41754CE74F15C9750B3F3F8DDCEA221F1C4A521B2BD6FCF5C`.

Gate 1 static ownership proof PASS:

- `SESSION_LIFECYCLE_OWNER=DeusDeviceSession`;
- public `StateChanged` signature unchanged;
- inline `StateChanged.Invoke` absent;
- notification owner is one serialized per-session task chain;
- callback exceptions isolated;
- `_operationGate` is not disposed;
- post-gate disposed rechecks exist in Connect/Execute/Disconnect;
- FDC-01 correlation, native transport and target source unchanged.

Gate 2 deterministic validation PASS:

- Core `58/58` passed;
- exact connect/recovery notification ordering covered;
- synchronous reentrant Execute and Disconnect covered with bounded deadlines;
- throwing subscriber isolation covers Connect -> automatic recovery -> Disconnect;
- unsubscribe semantics covered;
- Dispose while callback is blocked covered;
- Disconnect/Dispose race bounded and repeated Dispose idempotent;
- `DeusOs.Control.Core` Release build PASS;
- `DeusOs.Control.Desktop` restore + Release build PASS;
- target I/O and Flash mutation NONE.

Gate 3 canonical reconciliation is complete in the local acceptance candidate. Gate 4 commit and Gate 5 publication are still pending; therefore this record must not yet label FDC-02 CLOSED/PUBLISHED.
