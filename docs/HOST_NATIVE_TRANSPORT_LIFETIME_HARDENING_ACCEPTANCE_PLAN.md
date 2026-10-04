# Deus OS — Host Native Transport Lifetime Hardening Acceptance Plan

Status: **ACTIVE — GATE 0 / `FDC-09`**

Canonical design: `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md`

Published baseline commit: `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`

## Gate 0 — contract/source-boundary freeze

Required decisions:

- synchronous WinUSB/libusb native calls remain bounded by the existing `2000 ms` transfer timeout;
- `Task.Run` cancellation is not described as native cancellation once the call has entered unmanaged I/O;
- cancellation after native entry is latched: managed completion waits for native drain, then surfaces cancellation;
- Dispose marks closing before waiting, prevents new native entry, drains active I/O, then closes native resources;
- no transport method may return while an owned native worker still uses that transport's handles;
- lifetime state is per transport instance;
- runtime and bootloader profiles reuse the same fixed transport owners;
- no Core protocol/channel/session or target source mutation is authorized.

## Gate 1 — implementation/static review

Allowed paths are exactly:

- `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbTransport.cs`;
- `host/src/DeusOs.Control.Transport.Windows/DeusOs.Control.Transport.Windows.csproj` only if deterministic tests need `InternalsVisibleTo`;
- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs`;
- `host/tests/DeusOs.Control.Transport.Tests/TransportContractTests.cs`.

Static acceptance requires:

- no `Task.Run(nativeOperation, cancellationToken)` pattern for entered native transfer ownership;
- one per-instance operation/lifetime owner per platform transport;
- no static/global cancellation/active-operation state;
- native resource release occurs after active operation drain;
- repeat Dispose is idempotent;
- runtime/bootloader discoveries still use the same platform transport classes;
- native timeout constants remain 2000 ms unless Gate 0 is explicitly reopened;
- no Core/session/protocol/firmware/target source drift.

## Gate 2 — deterministic host validation

Transport tests must prove:

- pre-cancel prevents native delegate invocation;
- mid-operation cancellation does not allow early managed completion;
- cancellation is surfaced after native drain;
- dispose does not close/release before active operation drain;
- dispose closes/releases exactly once after drain;
- post-dispose operations are rejected without native invocation;
- repeated dispose is bounded/idempotent;
- same-instance operations serialize;
- two owner instances remain independent;
- existing Windows timeout mapping remains exact;
- existing Linux timeout/no-device mapping remains exact;
- runtime and bootloader topology/profile regressions remain PASS.

Build/regression proof requires:

- all Transport tests PASS with exact count recorded;
- all Core tests PASS with exact count recorded;
- Windows transport Release build PASS;
- Linux transport Release build PASS;
- Core Release build PASS;
- exact pre/post path hashes;
- `git diff --check` PASS;
- target I/O NONE / Flash mutation NONE.

## Gate 3 — Windows real-platform acceptance

Using the published target image without reflashing:

- WinUSB management open + fresh HELLO/ping PASS;
- bounded read cancellation/timeout proof completes within the documented transfer/drain bound;
- cancellation/disposal leaves no owned worker using closed WinUSB/file handles;
- dispose/disconnect completes and same locator reopens;
- fresh negotiation/ping after reopen PASS;
- physical removal/disconnect classification remains intact.

Record exact observed cancellation/dispose/reopen timings and native/host error classification.

## Gate 4 — Linux real-platform acceptance

Using the published target image without reflashing:

- libusb IF2-only claim + HELLO/ping PASS; CDC IF0/1 remains untouched;
- bounded read cancellation/timeout proof completes within the documented transfer/drain bound;
- release/close occurs only after active `libusb_bulk_transfer` drain;
- same physical-port locator reopens;
- fresh negotiation/ping PASS after reopen;
- no target Flash mutation.

Record exact observed cancellation/dispose/reopen timings and native/host error classification.

## Gate 5 — documentation reconciliation

After Windows/Linux PASS:

- record exact accepted source/test hashes;
- record Transport/Core test counts and Release-build outcomes;
- record Windows/Linux real-platform timing/reopen evidence identities;
- mark Slice A (`FDC-01..04,09`) complete;
- keep FDC-05..08 and FDC-10 open;
- do not claim Host Management Service/Web unblocked.

## Gate 6 — local acceptance commit

Stage only accepted source/tests/docs. Require `git diff --cached --check` PASS, no unrelated residue, one normal commit, and direct-parent proof against remote `main`.

## Gate 7 — publication

Ordinary non-force push only. Require fresh fetch, `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`, and independent GitHub `main` verification.

## Failure classifications

At minimum:

- `FDC09_SOURCE_SCOPE_DRIFT`
- `FDC09_NATIVE_LIFETIME_POLICY_DRIFT`
- `FDC09_EARLY_CANCEL_ORPHAN`
- `FDC09_CLOSE_BEFORE_DRAIN`
- `FDC09_CROSS_INSTANCE_COUPLING`
- `FDC09_TRANSPORT_TEST_FAILURE`
- `FDC09_WINDOWS_RUNTIME_FAILURE`
- `FDC09_LINUX_RUNTIME_FAILURE`
- `FDC09_REOPEN_FAILURE`
- `FDC09_DOC_RECONCILIATION_FAILURE`

A failure does not authorize Web/service/network or target changes.
