# Deus OS — Host Native Transport Lifetime Hardening Plan

Status: **GATES 0–5 ACCEPTED / PUBLICATION PENDING / `FDC-09`**

Published baseline commit: `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`

## 1. Purpose

Close `FDC-09` before any long-lived Host Management Service/Web implementation is allowed.

The published Windows and Linux adapters execute synchronous native bulk I/O inside `Task.Run(..., cancellationToken)`. Once `WinUsb_ReadPipe` / `WinUsb_WritePipe` or `libusb_bulk_transfer` has started, cancelling that token does not cancel the already-running native call. At the same time, current `DisposeAsync()` closes native handles immediately and does not coordinate with a worker that may still be inside that call.

Native transfer time is already bounded to 2000 ms on both platforms, so FDC-09 does not introduce a new async USB stack. It freezes an explicit lifetime policy that prevents orphan native work and handle-close races while preserving the accepted wire/session architecture.

## 2. Frozen source boundary

Authorized product/test paths are exactly:

- modify `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbTransport.cs`;
- modify `host/src/DeusOs.Control.Transport.Windows/DeusOs.Control.Transport.Windows.csproj` only to expose internals to `DeusOs.Control.Transport.Tests` if required by deterministic lifetime tests;
- modify `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs`;
- modify `host/tests/DeusOs.Control.Transport.Tests/TransportContractTests.cs`.

Canonical documentation and acceptance harness/tooling may be updated for closure evidence.

No `DeusOs.Control.Core` protocol/channel/session source, discovery semantics, CLI/Desktop behavior, firmware, bootloader or target source mutation is authorized.

## 3. Shared runtime/bootloader ownership fact

Runtime and bootloader discovery already reuse the same platform transport owners:

- Windows runtime/bootloader both open `WindowsWinUsbTransport` with different interface/endpoint profiles;
- Linux runtime/bootloader both open `LinuxLibUsbTransport` with different `LinuxUsbProfile` values.

Therefore one lifetime fix per platform transport closes both runtime and firmware-update transport ownership. No separate bootloader transport implementation is authorized.

## 4. Frozen cancellation semantics

The v1 native adapters use **latched bounded cancellation**, not fake immediate cancellation.

Before native I/O starts:

- an already-cancelled token rejects the operation without entering native I/O;
- cancellation while waiting for the per-transport native-operation owner rejects without native I/O.

After synchronous native I/O starts:

- cancellation is latched;
- the managed call must not return while its native worker is still running;
- the adapter waits for the current native transfer to complete under the existing native `2000 ms` transfer timeout;
- after drain, caller cancellation wins and is surfaced as `OperationCanceledException` / the existing Core cancellation mapping;
- no background/orphan native task may remain after the transport method returns.

This boundary does not claim immediate native cancellation and does not add overlapped WinUSB or libusb asynchronous-transfer/event-loop machinery.

## 5. Frozen disposal semantics

Each transport owns one private lifetime/operation gate.

`DisposeAsync()` must:

1. atomically mark the transport as closing so no new native I/O may enter;
2. wait for any already-entered native transfer to complete under the same bounded native timeout contract;
3. only after the active transfer has drained, release the claimed Linux interface / close libusb handle+context or dispose WinUSB/file handles;
4. be idempotent;
5. return only when no native worker still owns those handles.

Do not close raw Linux handles while `libusb_bulk_transfer` can still be executing. Do not rely on cancelling a `Task.Run` scheduling token to terminate native I/O.

The managed synchronization primitive itself need not be disposed if doing so would race queued callers; closed transport state is the ownership boundary.

## 6. Per-device isolation

Lifetime state must be per transport instance. Two independent transports must not share one static gate, cancellation source, active-task slot or disposed state.

One transport's cancellation/disposal must not block, cancel or corrupt another transport instance. Existing locator identity rules remain unchanged.

## 7. Error precedence

Frozen precedence for one in-flight operation:

- if caller cancellation was requested while a native call was active, drain the native call first and then surface cancellation;
- otherwise native timeout remains `HostErrorKind.Timeout`;
- physical removal/no-device remains `HostErrorKind.TransportDisconnected`;
- short transfer remains transport failure as today.

FDC-01 request-correlation semantics remain above this transport boundary and are not changed here.

## 8. Deterministic host tests

Gate 2 must prove with platform-transport lifetime test seams, without hardware:

1. pre-cancelled operation never invokes the native delegate;
2. cancellation during active native work does not complete the managed operation before the native delegate drains;
3. after drain, active-operation cancellation is surfaced as cancellation;
4. Dispose started during active work does not invoke native close/release before that work drains;
5. Dispose completes after drain and close/release occurs exactly once;
6. operations beginning after disposal starts are rejected and never call native I/O;
7. repeated Dispose is idempotent;
8. two independent transport lifetime owners execute independently;
9. same-transport operations are serialized;
10. existing Windows timeout mapping and Linux no-device/timeout mapping remain unchanged;
11. runtime and bootloader profile constants/regressions remain PASS.

## 9. Gate-2 build/static acceptance

Required:

- exact frozen source/test path set;
- all Transport tests PASS with exact count recorded;
- Core tests remain PASS with exact count recorded;
- Windows transport Release build PASS on Windows;
- Linux transport Release build PASS;
- Core Release build PASS;
- no protocol/channel/session source drift;
- `git diff --check` PASS;
- exact pre/post source hashes;
- target I/O NONE / Flash mutation NONE.

## 10. Windows real-platform acceptance

On the real Windows WinUSB management adapter:

- normal open/HELLO/ping remains PASS;
- one read-side cancellation/timeout scenario returns within the frozen bound and leaves no owned background native operation;
- dispose/disconnect after the bounded operation completes cleanly;
- same locator can be reopened and fresh negotiation/ping succeeds;
- ordinary physical/device removal still classifies as transport disconnect rather than cancellation corruption;
- target is not reflashed.

Use the existing 2000 ms pipe timeout; no operator cable thrashing is required beyond a narrowly justified reconnect/removal proof.

## 11. Linux real-platform acceptance

On the real Linux libusb management adapter:

- normal IF2-only open/HELLO/ping remains PASS and CDC IF0/1 stays untouched;
- one read-side cancellation/timeout scenario returns within the frozen bound with no orphan bulk-transfer worker;
- dispose releases IF2 only after active transfer drain;
- same physical-port locator reopens and fresh negotiation/ping succeeds;
- target is not reflashed.

## 12. Gates

- Gate 0 — this documentation/source-boundary freeze.
- Gate 1 — exact implementation/static review.
- Gate 2 — deterministic Transport/Core tests + Release builds + exact pre/post host evidence.
- Gate 3 — Windows real-platform bounded-cancel/dispose/reopen acceptance.
- Gate 4 — Linux real-platform bounded-cancel/dispose/reopen acceptance.
- Gate 5 — canonical documentation reconciliation.
- Gate 6 — one normal local acceptance commit.
- Gate 7 — ordinary non-force publication + fresh remote/GitHub verification.

## 13. Non-goals

Not authorized:

- overlapped WinUSB redesign;
- libusb async-transfer/event-thread framework;
- changing the 2000 ms native transfer timeout without a measured acceptance reason;
- parallel in-flight protocol requests;
- stable physical identity work;
- service/Web/network implementation;
- target firmware/USB descriptor/endpoint change.

## 14. Exit criterion

`FDC-09` is CLOSED only when native I/O cancellation and disposal are explicitly bounded and ownership-safe on both Windows and Linux, deterministic tests prove no early-return orphan worker / close-before-drain race / cross-instance coupling, real-platform reopen succeeds after bounded cancellation/disposal, and the exact accepted source is normally published.
