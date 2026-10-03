# Deus OS — Host RPC Timeout / Correlation Hardening Acceptance Plan

Status: **ACTIVE — GATE 0 / `FDC-01`**

Canonical design: `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md`

Baseline commit: `8bd09ad890ab10bb7fed6ecba21d5ad6a382237b`

## Gate 0 — contract/source-boundary freeze

Gate 0 is documentation-only.

Required:

- root cause is frozen as delayed multi-frame RPC responses surviving timeout/cancel;
- one channel-owned abandoned-request registry is selected;
- `SingleResponse` and `RpcUntilTerminal` response shapes are defined;
- unknown non-abandoned request IDs remain fatal;
- no arbitrary time-based stale-ID expiry;
- unresolved abandoned state crossing request-ID wrap fails closed and requires a fresh client/transport session;
- firmware-update DATA retains exactly one immediate exact-payload retry;
- INFO/BEGIN/AUTHORIZE/END receive no blind retry;
- exact authorized source/test paths are frozen;
- target/bootloader/native transport/UI source mutation is explicitly excluded.

Gate 0 passes only when `CURRENT_STATE`, `ROADMAP`, `MASTER_EXECUTION_CHECKLIST` and documentation inventory point to this exact active slice.

## Gate 1 — implementation/static ownership review

Allowed implementation paths are exactly those frozen in the plan.

Static acceptance requires:

- `DeviceProtocolChannel` remains the sole request-correlation/filter owner;
- no second decoder, second request-ID allocator or client-local stale-frame queue is introduced;
- generic RPC registers abandonment after timeout/cancel once a nonzero request ID exists;
- RPC abandonment persists through `RPC_DATA` until `RPC_END`/`PROTOCOL_ERROR`;
- firmware update uses the same registry with single-response retirement;
- arbitrary mismatched request IDs still fail;
- request-ID wrap cannot make an unresolved abandoned ID reusable;
- no source outside the frozen host Core/tests path set changed.

## Gate 2 — deterministic host validation

Required Core tests:

- timeout -> delayed RPC DATA/END -> next RPC PASS;
- cancellation -> delayed RPC DATA/END -> next RPC PASS;
- timeout after partial RPC DATA -> remaining delayed stream discarded -> next RPC PASS;
- delayed stale `PROTOCOL_ERROR` retires only the intended abandoned RPC;
- unknown request-ID mismatch still `RequestCorrelation`;
- firmware delayed timed-out DATA response still discarded and exact one retry retained;
- multiple abandoned IDs stay isolated;
- clean wrap `0xFFFF -> 0x0001` PASS;
- wrap with unresolved abandoned state fails closed;
- in-place protocol reset cannot bypass a wrap-required fresh-session condition.

Validation requirements:

- all `DeusOs.Control.Core.Tests` PASS;
- `DeusOs.Control.Core` Release build PASS;
- existing firmware-update unit suite PASS;
- `git diff --check` PASS;
- no target I/O, Flash, bootloader or firmware mutation;
- no Windows/Linux native transport source mutation.

## Gate 3 — documentation/closure reconciliation

After Gate 2 PASS:

- mark `FDC-01` and all of its sub-checkboxes `[x]` in `MASTER_EXECUTION_CHECKLIST`;
- record exact changed source/test paths and test counts;
- update `CURRENT_STATE` so Slice A proceeds to the remaining open IDs rather than claiming Host Management Service is unblocked;
- update `ROADMAP` `FDC-01` disposition to CLOSED while keeping `FDC-02/03/04/09` open;
- update this acceptance record with exact commit/evidence identity;
- preserve historical Host Control acceptance unchanged.

## Gate 4 — local acceptance commit

Require:

- only accepted FDC-01 source/tests/docs staged;
- no target/firmware/native transport/UI drift;
- staged diff check clean;
- one normal local commit;
- clean worktree/index after commit;
- remote still direct parent before publication.

## Gate 5 — ordinary non-force publication

Require fresh remote-state/direct-parent proof and ordinary non-force push only.

Post-push require:

- `HEAD == origin/main` after fresh synchronization;
- clean worktree/index;
- ahead/behind `0/0`.

## Failure classifications

At minimum distinguish:

- `FDC01_SOURCE_SCOPE_DRIFT`
- `FDC01_RPC_STALE_STREAM_FAILURE`
- `FDC01_UNKNOWN_CORRELATION_WEAKENED`
- `FDC01_REQUEST_ID_WRAP_FAILURE`
- `FDC01_FIRMWARE_RETRY_REGRESSION`
- `FDC01_CORE_TEST_FAILURE`
- `FDC01_BUILD_FAILURE`
- `FDC01_DOC_RECONCILIATION_FAILURE`

A failure does not authorize broad session/native-transport changes or folding `FDC-02/03/04/09` into this slice.
