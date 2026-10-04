# Deus OS — Host Native Transport Lifetime Hardening Acceptance Plan

Status: **GATES 0–7 ACCEPTED / CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73` / `FDC-09`**

Canonical design: `docs/HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md`

Published baseline commit: `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`

Accepted Gates 0–5 evidence summary:

- Gate 2 deterministic host validation: `stm32_os_fdc09_gate2_host_validation_dotnet_v2_20261004_215723.evidence.zip`, SHA-256 `1B4090EEED139BA7B5E7BEB11C530B0CF4B1EEB44706ADCA079430FAD686FFCE`; Transport `24/24`, Core `78/78`, Windows/Linux/Core Release builds PASS, exact four-path pre/post state, target I/O NONE / Flash mutation NONE.
- Gate 4 Linux real-platform validation: `stm32_os_fdc09_gate4_linux_real_platform_dotnet_v1_20261004_222724.evidence.zip`, SHA-256 `F084F76AEA3ED28E4BE4E697FCDF323360ED190810EEE4E3F944AD56930E2900`; IF2-only libusb open, cancel-drain `2021 ms`, dispose `3 ms`, same-locator reopen `15 ms`, fresh negotiation/ping, CDC `cdc_acm` ownership `2 -> 2`, topology unchanged, Flash mutation NONE.
- Gate 3 Windows real-platform validation: `stm32_os_fdc09_gate3_windows_real_platform_dotnet_v5_20261004_225617.evidence.zip`, SHA-256 `B392F3AEA191E0649CBBF2FFEBCE8AA2CA580BE79D25AF4B36FE58FD699728C5`; manifest `93/93` exact, WinUSB cancel-drain `2017 ms`, dispose `3 ms`, physical removal `TransportDisconnected` in `7 ms`, same-locator replug/reopen, fresh negotiation/ping, canonical Mac-mini bench restoration `634 ms`, exact repository poststate and Flash mutation NONE.
- Post-run presentation audit found `HARNESS-TERMINAL-STRUCTURE-01`: the V5 .NET validator emitted an uncolored final RESULT block and used reserved `PASS`/`FAIL` words in intermediate `[OK]` text, contrary to `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`. This does not change the finalized evidence bytes or product/runtime observations above; no repeated physical run is required solely for terminal styling. Future harness packages must fail package-generation lint if final green/red/yellow result structure or reserved-word discipline is absent.
- Gate 6/7 publication: implementation/docs commit `b88a9eee43095665326787cc0345822218c1ba73`, tree `43829e79c4679146aae5156d635680fb3b6dad39`, parent `5bfb0a28815f3712d30482b394a5b44c17a6011c`; ordinary non-force push; fresh fetch `HEAD == origin/main == FETCH_HEAD`; clean ahead/behind `0/0`; independent GitHub verification reports the exact commit/tree/parent and `verification.verified=true` / `reason=valid`.

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
