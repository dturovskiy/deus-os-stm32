# Deus OS — Host Asset Request Correlation Hardening Plan

Status: **CLOSED / PUBLISHED `40aab02b4e0c04466453be4c31041f8649c13c1b` — GATES 0–5 ACCEPTED**

Baseline repository commit:

`0713da56a3fe7a6b62f4c548d3248b4ed7fb2e57`

Boundary:

`HOST_ASSET_REQUEST_CORRELATION_HARDENING`

## 1. Purpose

Close the confirmed Host Core reliability gap where an Asset request that times out or is cancelled after receiving a nonzero request ID can later emit one delayed `AssetTransferResponse` that is not registered with the channel-owned abandoned-request registry and can therefore poison the next request in the same long-lived session.

This is a host-only maintenance boundary. It does not change target firmware, Asset wire values, USB/native transport, Flash, scripts, recovery tooling or physical target state.

## 2. Confirmed root cause

Published facts:

1. `DeviceProtocolChannel` owns request-ID allocation, queued frames, decoder state, request correlation and abandoned-response filtering.
2. FDC-01 already provides `SingleResponse` abandonment for one-response protocols and `RpcUntilTerminal` for generic RPC streams.
3. `FirmwareUpdateClient` registers a nonzero request ID as `SingleResponse` after native `HostErrorKind.Timeout` or cancellation.
4. `DeusRpcClient` registers a nonzero request ID as `RpcUntilTerminal` after timeout/cancellation.
5. `AssetTransferClient.AssetRequestCoreAsync()` allocates request IDs through the same channel but currently performs no abandonment registration.
6. Every Asset opcode has one protocol response frame per request. Therefore the existing channel-owned `SingleResponse` shape is the correct correlation primitive.
7. `DeusDeviceSession.ExecuteAsync()` does not discard/reopen a READY session merely because an Asset operation timed out or was cancelled. A delayed Asset response can therefore reach a later operation on the same channel.

## 3. Frozen source/test boundary

Authorized product source:

- `host/src/DeusOs.Control.Core/AssetTransferClient.cs`

Authorized deterministic tests:

- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`

Authorized documentation:

- this plan;
- `docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_ACCEPTANCE_PLAN.md`;
- `docs/CURRENT_STATE.md`;
- closure-only documentation required after accepted validation.

No other Host Core source is authorized unless implementation proves the frozen model impossible; in that case this boundary stops and planning is reopened.

## 4. Selected implementation model

`AssetRequestCoreAsync()` remains a thin domain client over `DeviceProtocolChannel`.

After a nonzero request ID has been allocated:

- a native `DeusHostException` whose kind is `HostErrorKind.Timeout` registers that ID with `DeviceProtocolChannel.AbandonSingleResponse()` before propagating the original exception;
- an `OperationCanceledException` registers that ID with `AbandonSingleResponse()` before propagating;
- zero/unallocated request IDs are never registered;
- successful responses do not create abandoned state;
- protocol/CRC/correlation failures are not silently converted into abandonment.

The channel remains the sole stale-response owner. No Asset-local decoder, stale-ID collection, queue, timer or expiry mechanism is permitted.

## 5. Explicit non-goals

This boundary does **not**:

- send protocol `ABORT` after a failed write transaction;
- add automatic Asset retry;
- alter BEGIN/WRITE/COMMIT semantics;
- change the 2 s / 5 s public Asset timeout policy;
- change user-cancellation exception mapping;
- change Asset transfer IDs;
- change wire frame/opcode/status layouts;
- mutate target firmware or persistence;
- change native WinUSB/libusb lifetime behavior;
- perform target/USB/UART/SWD/Flash hardware operations.

Volatile target-session cleanup after ambiguous mutation failure is a separate question and may be promoted only after this correlation boundary is accepted.

## 6. Required deterministic proof

At minimum:

1. Asset STATUS user cancellation after request allocation registers a single-response abandonment; one delayed response for that old request is discarded and a fresh request succeeds.
2. Native Host `Timeout` after Asset request allocation registers the same single-response abandonment; one delayed old response is discarded and a fresh request succeeds.
3. The fresh request uses the next request ID; the abandoned ID is not reused.
4. Unknown non-abandoned request IDs remain fatal through the existing channel contract.
5. Existing RPC abandoned-stream and Firmware Update single-response tests remain unchanged and PASS.
6. No target/native transport/script source changes occur.

## 7. Gates

### Gate 0 — freeze

PASS when baseline, root cause, exact path boundary, non-goals and proof requirements above are recorded.

### Gate 1 — implementation/static ownership

PASS when only the frozen Host Core/test paths plus boundary docs change and `DeviceProtocolChannel` remains the sole correlation owner.

### Gate 2 — deterministic host validation

Require:

- all `DeusOs.Control.Core.Tests` PASS;
- Core Release build PASS;
- existing Firmware Update/RPC correlation regressions PASS;
- `git diff --check` PASS;
- target I/O NONE / Flash mutation NONE.

### Gate 3 — reconciliation

Record exact test counts/source hashes and classify any remaining Asset volatile-session cleanup separately rather than folding it silently into this boundary.

### Gate 4 — local acceptance commit

One normal local commit after exact staged-path review and clean staged diff check.

### Gate 5 — publication

Ordinary non-force publication only after fresh remote/direct-parent proof; post-push fresh fetch must prove `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## 8. Exit criterion

The boundary is closed only when a timed-out/cancelled Asset request can no longer poison a later request through one delayed `AssetTransferResponse`, while unknown correlation remains fatal and no new retry/transaction-cleanup behavior is introduced.
