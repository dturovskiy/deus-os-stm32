# Deus OS — Host RPC Timeout / Correlation Hardening Acceptance Plan

Status: **CLOSED — GATES 0–5 ACCEPTED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e` / `FDC-01`**

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

## Gate-2 result — PASS

Authoritative evidence: `stm32_os_fdc01_host_validation_dotnet_v1_20261004_151712.evidence.zip`, SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`.

Accepted proof:

- evidence ZIP CRC clean; manifest `65/65` exact;
- pre/post `HEAD == origin/main == 953621bda0e4dd530e9e3a0f149fd9316d811c8d`;
- exact five modified tracked paths, staged `0`, untracked `0`;
- fresh tracked-only scratch host worktree and exact five WIP hashes reverified;
- isolated .NET/NuGet environment on SDK `10.0.201`;
- xUnit v3 structured result: total `51`, passed `51`, failed `0`, skipped `0`, errors `0`, not-run `0`;
- Core Release build PASS;
- target I/O `NONE`, Flash mutation `NONE`;
- static ownership review confirms one `DeviceProtocolChannel` correlation/abandonment owner, no second decoder/request-ID allocator/stale queue, unknown non-abandoned correlation remains fatal, and firmware non-DATA requests retain no blind retry.

Gate 2 was accepted by the evidence above; the Gate-3/4/5 results recorded below subsequently completed closure/publication.

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

## Gate-3 result — PASS

Canonical reconciliation recorded the exact Gate-2 evidence and preserved the five-path accepted product candidate without further source mutation.

## Gate-4 result — PASS

One normal local acceptance commit was created:

- commit `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`;
- tree `146704385bcaf0b7196b3519b8fe17488200fe63`;
- parent `953621bda0e4dd530e9e3a0f149fd9316d811c8d`;
- message `fix: harden host rpc timeout recovery`;
- exact accepted set: five frozen Host Core/test paths plus six canonical docs;
- staged diff check PASS; post-commit repository clean; local branch ahead `1/0` before publication.

## Gate-5 result — PASS / PUBLISHED

Publication used an ordinary non-force fast-forward only:

`953621b..c863b5a  main -> main`

Fresh synchronization proved:

- `HEAD == origin/main == FETCH_HEAD == c863b5ab9d00ab96de7c8f8275f905ed52c8740e`;
- clean worktree/index;
- ahead/behind `0/0`.

Independent GitHub `main` lookup confirmed the same commit/tree/parent and a verified valid signature.

`FDC-01` is CLOSED. The next active closure boundary is `FDC-02 / HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING`; this acceptance does not authorize `FDC-03+` source work.

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
