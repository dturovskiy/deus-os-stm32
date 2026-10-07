# Deus OS — Host Asset Transaction Recovery Hardening Plan

Status: **ACTIVE — GATES 0–3 ACCEPTED / GATE 4 COMMIT PENDING**

Boundary ID:

`HOST_ASSET_TRANSACTION_RECOVERY_HARDENING`

Residual-debt ID:

`RDC-01`

Baseline:

`7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`

## 1. Purpose

Close the confirmed Host Core debt where a write transaction can leave a volatile Asset session alive after BEGIN may have reached the target but a later BEGIN/WRITE_CHUNK/COMMIT operation fails, times out or is cancelled.

The published Asset protocol already owns an idempotent `ABORT` operation that drops only the volatile session, never invalidates the previously committed record, and is safe when no matching session exists. This boundary wires that existing recovery primitive into Host Core without adding mutation retry or changing target/wire behavior.

## 2. Prerequisite already closed

`HOST_ASSET_REQUEST_CORRELATION_HARDENING` is CLOSED/PUBLISHED at `40aab02b4e0c04466453be4c31041f8649c13c1b`.

Therefore:

- timed-out/cancelled Asset request IDs are registered in the channel-owned `SingleResponse` abandoned-response registry;
- a delayed old response cannot poison the cleanup ABORT or a later request;
- unknown non-abandoned correlation remains fatal.

RDC-01 must reuse that accepted ownership and must not implement another stale-response mechanism.

## 3. Frozen source/test boundary

Authorized product source:

- `host/src/DeusOs.Control.Core/AssetTransferClient.cs`

Authorized deterministic tests:

- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`

Authorized documentation:

- this plan;
- `docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md`;
- `docs/CURRENT_STATE.md`;
- `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`;
- `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_ACCEPTANCE_PLAN.md`;
- closure-only changelog/roadmap/checklist reconciliation after acceptance.

No target firmware, protocol, transport, script, linker, Flash or hardware source is authorized.

## 4. Recovery ownership and active-session window

The Host write transaction owns one nonzero transfer ID.

`sessionMayBeActive` becomes true immediately before issuing BEGIN, because a BEGIN failure can be ambiguous: the request may have reached the target and created/prepared the volatile session even if Host Core did not receive a valid response.

The flag remains true through WRITE_CHUNK and COMMIT ambiguity.

It becomes false only after Host Core has received and parsed a successful COMMIT response. The protocol guarantees successful COMMIT closes the volatile session.

Failures after that point — post-COMMIT STATUS/readback/CRC verification — must not trigger transaction ABORT because there is no longer an active volatile session to clean.

## 5. Best-effort ABORT contract

When the primary write operation fails while `sessionMayBeActive == true`:

1. preserve the exact primary exception classification/semantics;
2. create a cleanup-only cancellation source independent of the caller/5-second write token;
3. bound cleanup by 2 seconds, matching the accepted FDC-09 native transfer bound;
4. send one `ABORT` for the same object type and transfer ID with frame flags zero;
5. use normal `AssetRequestCoreAsync()`, so cleanup timeout/cancellation also registers its request ID through the accepted channel-owned `SingleResponse` model;
6. accept only normal protocol success; any cleanup timeout/cancel/protocol/transport failure is swallowed as secondary cleanup failure and must never replace the primary error;
7. perform no BEGIN/WRITE/COMMIT retry.

This is best-effort session cleanup, not transaction success recovery.

## 6. Error coverage

Cleanup is required for expected Host transaction failures after the session may be active:

- `DeusHostException`, including protocol/status/CRC/native timeout classes;
- `OperationCanceledException`, including caller cancellation and the outer 5-second transaction deadline.

Pre-write STATUS failures occur before a transfer ID/session exists and must not send ABORT.

## 7. Frozen protocol facts

No wire change is authorized:

- ABORT opcode remains `0x04`;
- ABORT payload remains 6 bytes;
- ABORT does not require `ALLOW_DESTRUCTIVE`;
- repeated ABORT with no matching active session remains idempotent OK/no-session;
- successful COMMIT remains the point that closes the volatile session;
- no partial candidate becomes authoritative before COMMIT.

## 8. Deterministic proof

At minimum:

1. ambiguous BEGIN native timeout after the target could have accepted BEGIN:
   - original BEGIN request ID is abandoned;
   - one delayed BEGIN response is discarded;
   - cleanup sends ABORT using the same transfer ID and a fresh request ID;
   - ABORT flags are zero;
   - the primary timeout remains the surfaced failure.

2. WRITE_CHUNK protocol/status failure while a session is active:
   - cleanup sends ABORT;
   - a failing ABORT response does not mask or rewrite the primary WRITE failure.

3. caller cancellation while a session may be active:
   - cleanup still runs using its independent cleanup token;
   - caller cancellation remains the surfaced exception.

4. successful COMMIT closes the cleanup window:
   - later post-COMMIT validation/readback failure does not send ABORT.

5. existing Asset correlation, RPC and Firmware Update regressions remain PASS.

## 9. Non-goals

This boundary does not:

- retry BEGIN, WRITE_CHUNK or COMMIT;
- resume partial uploads;
- change transfer-ID allocation;
- change request-ID allocation/correlation;
- change Asset timeout policy except for the private bounded cleanup deadline;
- expose cleanup as a new public API;
- change target session timeout;
- change persistence, Flash, protocol flags/opcodes/statuses;
- perform target/USB/UART/SWD/Flash hardware operations.

## 10. Gates

### Gate 0 — freeze

PASS when exact baseline, source scope, active-session window, cleanup deadline, primary-error preservation and deterministic proof are frozen.

### Gate 1 — implementation/static ownership

PASS when product mutation is confined to `AssetTransferClient.cs`, tests to `ClientTests.cs`, and no second correlation/retry/session owner is introduced.

### Gate 2 — deterministic host validation

Require:

- complete Core test suite PASS;
- Core Release build PASS with zero warnings/errors;
- exact candidate hash/path proof;
- `git diff --check` PASS;
- TARGET_IO=NONE / FLASH_MUTATION=NONE.

### Gate 3 — reconciliation

Record exact test count, source/test hashes, evidence SHA-256 and confirm RDC-01 closes only transaction/session cleanup.

### Gate 4 — local acceptance commit

Exact staged path review, staged diff check, one normal local commit.

### Gate 5 — publication

Fresh direct-parent proof, ordinary non-force push, fresh-fetch clean `HEAD == origin/main == FETCH_HEAD`, ahead/behind `0/0`.

## 11. Exit criterion

RDC-01 is closed only when any expected Host failure after BEGIN may have activated the target session receives one bounded best-effort ABORT attempt, the primary failure is preserved, successful COMMIT suppresses unnecessary cleanup, and all existing correlation behavior remains intact.
