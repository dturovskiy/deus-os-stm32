# Deus OS — Host RPC Timeout / Correlation Hardening Plan

Status: **GATES 0–2 PASS — ACCEPTANCE/PUBLICATION CLOSURE IN PROGRESS / `FDC-01`**

Baseline repository commit: `8bd09ad890ab10bb7fed6ecba21d5ad6a382237b`

## 1. Purpose

Close `FDC-01` before any long-lived Host Management Service/Web boundary.

The published Host Core correctly rejects unknown request-ID mismatches and already supports a one-response stale-ID path for firmware-update DATA retry. The 2026-10-03 audit proved that generic RPC timeout/cancellation has a different response shape: an abandoned request may later emit multiple `RPC_DATA` frames followed by `RPC_END`. The current channel removes a stale request ID after the first discarded frame, so a later frame from the same abandoned RPC can poison the next request.

This boundary is host-only reliability hardening. It does not change STM32 firmware, protocol wire values, USB descriptors, Flash, target state or physical bench bytes.

## 2. Frozen source boundary

Authorized product-source paths:

- `host/src/DeusOs.Control.Core/DeviceProtocolChannel.cs`
- `host/src/DeusOs.Control.Core/DeusRpcClient.cs`
- `host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs`
- `host/src/DeusOs.Control.Core/RequestIdAllocator.cs` only if required to expose/test deterministic wrap semantics without changing wire IDs
- `host/src/DeusOs.Control.Core/DeusDeviceClient.cs` only if a fail-closed exhausted/wrap state must be surfaced without weakening existing error classification

Authorized tests:

- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`
- `host/tests/DeusOs.Control.Core.Tests/FirmwareUpdateTests.cs`

No Windows/Linux native transport source, CLI/Desktop presentation, target firmware, bootloader, linker/startup or scripts are authorized by this slice.

## 3. Root-cause contract

Current facts that implementation must preserve:

1. `DeviceProtocolChannel` is the single owner of decoder bytes, request-ID allocation, queued frames, request correlation and operation serialization.
2. Unknown request IDs that were never explicitly abandoned remain fatal `RequestCorrelation` errors.
3. Generic RPC is multi-frame: zero or more `RPC_DATA`, then one terminal `RPC_END`; `PROTOCOL_ERROR` is terminal for that request.
4. Firmware-update request/response is single-response at the channel level; DATA alone has one exact immediate timeout retry under the already accepted idempotency rule.
5. INFO/BEGIN/AUTHORIZE/END do not gain blind retry semantics.
6. Request ID `0` remains invalid and allocator wire range remains `1..0xFFFF`.

## 4. Selected abandoned-request model

There will be one channel-owned abandoned-request registry. Domain clients may declare an allocated request abandoned, but they may not maintain independent decoder/correlation filters.

Each abandoned request records its response shape:

- `SingleResponse`: discard exactly one matching response frame, then retire the abandoned ID. This preserves the accepted firmware-update delayed-response behavior.
- `RpcUntilTerminal`: discard every matching delayed frame and retain the abandoned ID until a terminal `RPC_END` or `PROTOCOL_ERROR` is consumed.

A frame whose request ID is not the active ID and not in the abandoned registry remains an error; the hardening must not turn arbitrary correlation mismatches into ignored traffic.

## 5. Timeout/cancellation semantics

For generic RPC, once a nonzero request ID has been allocated, timeout or user cancellation during write/read/response aggregation marks that request as `RpcUntilTerminal` before the operation gate is released.

This includes:

- timeout before the first response frame;
- timeout after one or more valid `RPC_DATA` chunks but before `RPC_END`;
- user cancellation in the same states;
- a native transport timeout surfaced as `HostErrorKind.Timeout` after a request ID was allocated.

The externally visible operation still reports `Timeout` or `Cancelled` as today. This boundary does not relabel a timeout as success.

## 6. Request-ID lifetime and wrap

An abandoned ID is never returned by the allocator while it remains registered.

No arbitrary age/time expiry is permitted because dropping an abandoned ID merely because time passed can make a very late old response indistinguishable from a newly reused request ID.

Bounded lifetime is therefore tied to the finite request-ID epoch:

- unresolved abandoned IDs may remain registered while the current `1..0xFFFF` allocation epoch proceeds;
- if allocation reaches a wrap boundary while any abandoned request remains unresolved, the channel fails closed rather than beginning an epoch that could eventually reuse an unresolved ID;
- that fail-closed condition requires construction of a fresh client/transport session before request allocation resumes; in-place `ResetProtocolState()` must not silently make unresolved old wire responses safe for ID reuse.

This rule bounds abandoned-ID lifetime without inventing unsafe timeout-based reuse.

## 7. Firmware-update reconciliation

`FirmwareUpdateClient` continues to use the shared channel owner. Its current stale-ID helper must be replaced/reframed as registration of `SingleResponse` abandonment, not an independent stale-filter mechanism.

DATA retains exactly one same-payload immediate timeout retry.

INFO/BEGIN/AUTHORIZE/END retain no blind retry. Ambiguous response loss remains an adjudication/restart problem: reconnect/re-enter the appropriate device mode, inspect authoritative state where available, and restart from a safe transaction boundary rather than assuming request-level idempotence. This slice must test/preserve the no-blind-retry rule; full deployment-level END adjudication remains governed by the published firmware-update plan and `FDC-08` hardware closure.

## 8. Required deterministic tests

At minimum:

1. RPC timeout before response; delayed `RPC_DATA` + delayed `RPC_END`; next fresh RPC succeeds.
2. RPC cancellation before response; delayed `RPC_DATA` + delayed `RPC_END`; next fresh RPC succeeds.
3. timeout after at least one valid `RPC_DATA`; remaining delayed chunks + `RPC_END` are discarded; next RPC succeeds.
4. abandoned RPC ending in delayed `PROTOCOL_ERROR` retires the abandoned ID and does not poison the next RPC.
5. unrelated unknown request ID still throws `RequestCorrelation`.
6. single-response firmware-update stale response is discarded exactly once and existing DATA retry tests remain PASS.
7. multiple abandoned request IDs remain distinct and cannot be allocated while registered.
8. request-ID wrap with no unresolved abandoned IDs remains `0xFFFF -> 0x0001`.
9. wrap with unresolved abandoned state fails closed and does not reuse an unresolved epoch.
10. protocol/session reset does not falsely claim that a wrap-poisoned transport is safe for request-ID reuse.

## 9. Validation

Gate 1 implementation/static review must prove only the frozen paths changed and one channel remains correlation owner.

Gate 2 requires:

- `dotnet test` for `DeusOs.Control.Core.Tests` PASS;
- Core Release build PASS;
- existing firmware-update tests PASS;
- no firmware/bootloader/build artifact mutation;
- `git diff --check` PASS.

No target I/O or physical hardware mutation is required for this pure host correlation slice unless implementation unexpectedly changes a native transport contract, which is outside the frozen scope and must instead stop/reopen planning.

## 9.1 Gate-2 accepted implementation evidence

Authoritative Gate-2 evidence: `stm32_os_fdc01_host_validation_dotnet_v1_20261004_151712.evidence.zip`, SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`.

Accepted facts:

- ZIP CRC clean and evidence manifest `65/65` exact;
- exact live pre/post repository state at `HEAD == origin/main == 953621bda0e4dd530e9e3a0f149fd9316d811c8d`;
- exactly five modified tracked paths, all inside the frozen FDC-01 source/test set;
- `DeviceProtocolChannel` remains the sole decoder/request-ID/correlation/abandoned-response owner;
- `DeusRpcClient` registers abandoned multi-frame RPC state on timeout/cancel;
- `FirmwareUpdateClient` uses the same channel registry for single-response abandonment; DATA retains exactly one same-payload timeout retry while INFO/BEGIN/AUTHORIZE/END retain no blind retry;
- fresh tracked-only scratch host worktree with no imported `bin/obj` residue;
- isolated .NET/NuGet environment, .NET SDK `10.0.201`, xUnit.net v3 `4.0.1`;
- Core tests `51/51` PASS, failed/skipped/errors/not-run all zero;
- `DeusOs.Control.Core` Release build PASS;
- `git diff --check` PASS;
- staged/untracked state remained zero;
- target I/O and Flash mutation were both zero.

Gate 2 is accepted. Gates 3–5 remain closure/publication steps; no `FDC-02+` source mutation is authorized until those steps finish.

## 10. Exit criterion

`FDC-01` is CLOSED only after deterministic tests prove abandoned multi-frame RPC traffic cannot poison a later request, unknown correlation mismatches remain fatal, firmware single-response retry semantics remain intact, and unresolved request state cannot survive into unsafe request-ID reuse.

Closing `FDC-01` does not authorize or close `FDC-02`, `FDC-03`, `FDC-04` or `FDC-09`.
