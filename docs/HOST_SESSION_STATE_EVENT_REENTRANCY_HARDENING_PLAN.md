# Deus OS — Host Session State-Event Reentrancy Hardening Plan

Status: **GATES 0–3 ACCEPTED / GATE 4 LOCAL ACCEPTANCE COMMIT PENDING / `FDC-02`**

Baseline repository commit: `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`

## 1. Purpose

Close `FDC-02` before any long-lived Host Management Service/Web boundary.

`DeusDeviceSession` correctly serializes Connect/Execute/recovery/Disconnect/Dispose with one `_operationGate`, but its current `SetState()` mutates `State` and invokes public `StateChanged` subscribers synchronously on the same call stack. Every production state transition currently occurs while the session operation gate is owned. An arbitrary subscriber can therefore synchronously call another session operation and wait on the same gate while the owner is waiting for the callback to return, creating a reentrant deadlock.

The current Avalonia subscriber happens to be safe because it posts work to the UI dispatcher, but the public Core event contract cannot rely on every future consumer doing that. A long-lived service must be able to subscribe without inheriting this lock-coupling hazard.

This boundary is host-only lifecycle hardening. It does not change STM32 firmware, protocol frames, request correlation, Windows/Linux native USB transports, target Flash or physical bench bytes.

## 2. Frozen source boundary

Authorized product-source path:

- `host/src/DeusOs.Control.Core/DeusDeviceSession.cs`

Authorized tests:

- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`

Canonical documentation may be updated for acceptance/closure.

Explicitly outside this slice:

- `DeusDeviceClient` and protocol/channel/RPC ownership;
- Windows WinUSB transport;
- Linux libusb transport;
- CLI and Avalonia/Desktop source;
- target firmware, bootloader, linker/startup and scripts.

If implementation requires any excluded source path, Gate 0 must be reopened before mutation.

## 3. Preserved ownership model

The following accepted ownership remains unchanged:

1. `DeusDeviceSession` is the single owner of host session lifecycle state.
2. `_operationGate` continues to serialize Connect/Execute/recovery/Disconnect/Dispose state mutation and client ownership.
3. `State` remains synchronously updated by the lifecycle owner before a notification is scheduled.
4. `LastNegotiation`, `_client` and `_candidate` remain session-owned and are not moved into presentation code.
5. Automatic same-locator reconnect still performs fresh open + HELLO + `sysinfo` negotiation.
6. FDC-01 request-correlation ownership remains entirely in `DeviceProtocolChannel`; this slice must not reopen it.

## 4. Selected notification model

`StateChanged` becomes a **serialized asynchronous observational notification**, not an inline extension of the session critical section.

Required properties:

- `SetState()` may update `State` while `_operationGate` is held, but it must only enqueue/schedule notification work; it must never execute arbitrary subscriber code inline.
- one private per-session dispatcher/queue/task-chain owns notification serialization;
- notification dispatch occurs on a thread-pool/default scheduler path independent of the session operation gate;
- state values are dispatched in the exact order in which accepted state transitions were scheduled;
- no coalescing or dropping of distinct state transitions is permitted;
- callback execution is observational: Connect/Execute/recovery/Disconnect/Dispose do not wait for arbitrary subscriber completion;
- a callback may synchronously or asynchronously invoke another public session operation; it will serialize normally through `_operationGate` after the current owner releases it rather than deadlocking the owner;
- one throwing subscriber must not fail a session operation, fault the notification dispatcher, or prevent later subscribers/later state notifications from being delivered;
- the implementation must not create a second public event framework, global dispatcher, synchronization-context dependency or presentation-specific Core dependency.

## 5. Subscription and delivery semantics

The event remains:

```csharp
public event Action<ConnectionState>? StateChanged;
```

No public API signature change is authorized.

Because delivery is asynchronous:

- the `ConnectionState` event argument is the authoritative transition being reported;
- by callback execution time, `session.State` may already have advanced to a later state;
- consumers must not infer that callback execution itself holds the lifecycle gate;
- unsubscribe must prevent future dispatch from invoking that subscriber; already scheduled state values may still be drained for other currently subscribed handlers;
- callback exceptions are isolated from Core lifecycle state and are not converted into session `Faulted` state.

The existing Desktop behavior remains valid: `MainWindow` already posts state rendering onto the Avalonia UI dispatcher. No Desktop source change is required by this boundary.

## 6. Dispose and shutdown semantics

Disposal must remain bounded by session/transport ownership, not by arbitrary external callback execution.

Therefore:

- `DisposeAsync()` must not wait for subscriber callbacks to finish;
- the final transition to `Disconnected` is scheduled using the same ordered notification mechanism when it is a real state change;
- `_operationGate` is not disposed while a reentrant operation can still require it in a way that turns a formerly safe callback into an `ObjectDisposedException` race; implementation must define a safe disposal ordering within `DeusDeviceSession`;
- repeated `DisposeAsync()` remains idempotent;
- no callback may resurrect a disposed session; public operations after disposal continue to fail through the existing disposed contract.

A consumer that requires no callbacks after its own teardown is responsible for unsubscribing before/while tearing down its presentation object; the Core must make unsubscribe effective for subsequent dispatch.

## 7. Required deterministic tests

At minimum Gate 2 must cover:

1. connect transition order remains exactly `Discovered -> Opening -> Negotiating -> Ready`.
2. a `Ready` subscriber that synchronously calls another session operation does not deadlock; the nested operation completes after the outer operation releases the gate.
3. a subscriber that synchronously requests disconnect does not deadlock and final session state is `Disconnected`.
4. recovery ordering preserves `Recovering -> Discovered -> Opening -> Negotiating -> Ready` in-order around the existing reconnect flow.
5. a throwing subscriber does not fail Connect/recovery/Disconnect and does not prevent a second subscriber or later state event from being delivered.
6. concurrent/near-concurrent Disconnect and Dispose complete without deadlock; disposal remains idempotent and final state is `Disconnected`.
7. unsubscribe prevents subsequent queued/delivered notifications from invoking the removed handler according to the frozen delivery contract.
8. existing FDC-01 RPC/correlation tests and all pre-existing Core tests remain PASS.

Tests must use bounded deadlines/timeouts so a reentrancy regression is reported as a deterministic test failure rather than hanging the suite.

## 8. Validation

Gate 1 static review must prove:

- only the frozen Core/session test paths changed;
- no public event signature change;
- no subscriber invocation occurs directly inside `SetState()` or while `_operationGate` ownership is synchronously waiting for that callback;
- notification serialization has one per-session owner;
- callback exceptions are isolated;
- no Desktop/native transport/target source drift.

Gate 2 requires:

- all `DeusOs.Control.Core.Tests` PASS, including the reentrancy/deadlock matrix above;
- `DeusOs.Control.Core` Release build PASS;
- `DeusOs.Control.Desktop` Release build PASS as a compatibility regression because it consumes `StateChanged`;
- `git diff --check` PASS;
- no target I/O, Flash, firmware/bootloader or native transport mutation.

No physical hardware acceptance is required for this host-only event-delivery slice unless implementation unexpectedly changes a native transport or target contract, which is outside scope and must stop/reopen planning.

## 9. Gates

- **Gate 0** — this contract/source boundary frozen and canonical pointers activated.
- **Gate 1** — bounded implementation + static ownership/reentrancy review.
- **Gate 2** — deterministic Core tests + Core/Desktop Release builds + exact pre/post state evidence.
- **Gate 3** — canonical docs/checklist reconciliation.
- **Gate 4** — one normal local acceptance commit.
- **Gate 5** — ordinary non-force publication + fresh-fetch/GitHub verification.

## 10. Exit criterion

`FDC-02` is CLOSED only when arbitrary `StateChanged` subscriber code can no longer participate inline in the session critical section, notification ordering remains deterministic, callback failure is isolated, reentrant session operations are proven deadlock-free, Desktop compatibility remains intact, and the accepted source is published normally.

Closing `FDC-02` does not authorize or close `FDC-03`, `FDC-04`, `FDC-09` or any target-side FDC item.


## 11. Gate-2 / Gate-3 accepted result — 2026-10-04

Implementation and deterministic host validation are accepted for the exact frozen two-path candidate:

- `host/src/DeusOs.Control.Core/DeusDeviceSession.cs` SHA-256 `6E39B68DE0D82906AEFAFDE376E49AD824BA9959332E2C79EACCA34CCB13F03F`;
- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs` SHA-256 `7FD188B31FA9A1C41754CE74F15C9750B3F3F8DDCEA221F1C4A521B2BD6FCF5C`.

Accepted implementation properties:

- `SetState()` synchronously updates `State` but never executes public subscribers inline;
- one private per-session task chain serializes `StateChanged` notification delivery on the default scheduler;
- callback exceptions are isolated per subscriber and do not fault lifecycle operations or later notifications;
- Connect/Execute/Disconnect recheck disposed state after acquiring `_operationGate`;
- `_operationGate` is not disposed, avoiding a queued/reentrant disposed-semaphore race;
- Dispose remains idempotent and does not wait for arbitrary callback completion;
- public event signature remains unchanged.

Authoritative Gate-2 evidence:

- file: `stm32_os_fdc02_host_validation_dotnet_v2_20261004_162345.evidence.zip`;
- SHA-256: `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`;
- manifest: `72/72` exact;
- Core tests: `58/58` passed, failed/skipped/errors/not-run all zero;
- `DeusOs.Control.Core` Release build: PASS;
- `DeusOs.Control.Desktop` Release build: PASS;
- exact pre/post changed-path set: two frozen paths, staged `0`, untracked `0`, source hashes unchanged;
- target I/O: NONE;
- Flash mutation: NONE.

Gate 3 documentation reconciliation is accepted by the same closure update. Gate 4 local acceptance commit and Gate 5 ordinary non-force publication remain required before `FDC-02` is described as CLOSED/PUBLISHED or before `FDC-03` source mutation is authorized.
