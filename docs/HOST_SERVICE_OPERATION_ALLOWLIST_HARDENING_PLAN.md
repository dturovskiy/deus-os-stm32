# Deus OS — Host Service Operation Allowlist Hardening Plan

Status: **ACTIVE — GATE 0 CONTRACT FREEZE / `FDC-03`**

Baseline repository commit: `42245d9d71504482fb189d8351ecce7049542145`

## 1. Purpose

Close `FDC-03` before any Host Management Service/Web implementation is allowed.

The accepted Host Core deliberately exposes a low-level public `DeusDeviceClient.RpcAsync(ushort rpcId, ..., byte flags, ...)` compatibility API. That API is appropriate for trusted local protocol tooling/tests, but it must never become an HTTP/Web/service pass-through where external input selects numeric target RPC IDs or protocol flags.

Target command classes (`SAFE`, `DIAGNOSTIC`, destructive-intent flag) describe target protocol behavior; they are not host-user authorization policy. A future localhost service therefore requires a separate explicit service-facing allowlist with named operations and host-side exposure classes.

This boundary is host-only policy/API hardening. It does not create a Web server, HTTP route, authentication system, network listener, new transport, target RPC, firmware change or hardware mutation.

## 2. Frozen source boundary

Authorized product source:

- new `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs`

Authorized tests:

- new `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`

Canonical documentation may be updated for acceptance/closure.

No existing Core protocol/channel/session source, CLI/Desktop source, native WinUSB/libusb transport source, firmware/bootloader source or scripts are authorized. If implementation cannot satisfy the contract within these two new files, Gate 0 must be reopened before mutation.

## 3. Ownership model

`DeusOs.Control.Core` owns the service-facing operation policy because presentation/service layers must not decide target protocol exposure ad hoc.

The new source file owns exactly:

1. service operation identifiers/descriptors;
2. host exposure classification;
3. the frozen service-v1 allowlist;
4. a typed facade over an existing `DeusDeviceSession`.

It does **not** own transport, framing, request correlation, authentication, HTTP routing, JSON, presentation parsing or target command registry discovery.

`DeusDeviceSession` remains the lifecycle/reconnect owner. `DeusDeviceClient` remains the protocol/application facade. `DeviceProtocolChannel` remains the correlation owner.

## 4. Exposure classes

Freeze this host-side policy enum:

```text
ReadOnly
Control
Destructive
```

Semantics:

- `ReadOnly`: intended to observe negotiated/current target state without intentionally mutating durable or application lifecycle state;
- `Control`: bounded operator mutation such as application lifecycle or published configuration mutation;
- `Destructive`: reset/watchdog/update/recovery/security-sensitive or equivalent operation requiring a later explicit authorization/security boundary.

These classes are independent from firmware `SAFE` / `DIAGNOSTIC` labels and from `RpcFlagAllowDestructive`.

## 5. Frozen service-v1 allowlist

Named service operations are exactly:

### ReadOnly

- `Ping`
- `Health`
- `Applications`
- `AssetStatus`
- `ReadOledUiLayout`

### Control

- `StartApplication`
- `StopApplication`
- `WriteOledUiLayout`

### Destructive

- **none in service v1**

The facade may expose negotiated `State` / `LastNegotiation` as properties because those are existing session models, not target RPC routing.

`SystemInfo` is obtained from `LastNegotiation` rather than accepting an arbitrary `sysinfo` RPC route. FDC-04 will decide which remaining raw-result read surfaces need typed service models.

## 6. Explicitly forbidden service-facing routes

The new service facade/catalog must expose no operation accepting any of:

- numeric `rpcId` / `ushort` command selector;
- caller-controlled protocol `flags`;
- arbitrary argument list forwarded to target RPC;
- `RpcAsync` or equivalent raw command name/string pass-through.

Service v1 must not expose:

- `EnterBootloaderAsync` or firmware-update entry;
- `wdogtrip`;
- scheduler stress/isolation/diagnostic execution methods;
- UI mutation/test methods;
- generic `rpcinfo` registry browsing as an invocation mechanism;
- arbitrary/unlisted target RPC IDs;
- caller-selected destructive-intent flag.

Absence from the allowlist means unavailable, not “allowed if target calls it SAFE”.

## 7. Typed facade rule

The new facade must be constructed over `DeusDeviceSession` and implement each allowed operation through the already accepted typed Core method for that exact operation.

Examples:

- `PingAsync` -> `client.PingAsync`;
- `HealthAsync` -> `client.HealthAsync`;
- `ApplicationsAsync` -> `client.ApplicationsAsync`;
- application start/stop -> existing typed wrappers;
- Asset status/read/write -> existing typed published-policy wrappers.

No facade method may call `client.RpcAsync(...)` directly.

The facade must not inspect firmware command registry text to dynamically expand its allowlist. The catalog is compile-time explicit and reviewable.

## 8. Capability / publication behavior

The service facade does not invent capability support. Existing Core operations continue to enforce negotiated/published capability rules.

In particular:

- Asset operations retain `AssetAccessPolicy.PublishedOnly`;
- firmware update remains absent regardless of the published firmware-update capability bit;
- future capability bits do not auto-enable new service operations;
- a new service operation requires an explicit reviewed source change to the allowlist.

## 9. FDC-04 boundary preservation

FDC-03 freezes **which operations may be routed**, not the final typed representation of every result.

Some accepted typed Core wrappers still return `RpcResult`/text for read/control operations. FDC-04 remains independently responsible for typed Core models/parsers required by future service/Web state surfaces. FDC-03 must not grow ad-hoc HTTP/presentation parsing to compensate.

## 10. Required deterministic tests

Gate 2 must prove at minimum:

1. catalog contains exactly the frozen eight service-v1 operations and no duplicates;
2. exact access classification is five ReadOnly, three Control, zero Destructive;
3. descriptor lookup rejects undefined enum/numeric values;
4. service facade public methods contain no raw RPC ID/flags/argument-list entry point;
5. reflection/static API check proves `RpcAsync`, `EnterBootloaderAsync` and generic command routing are absent from the facade;
6. allowed facade methods route to the corresponding typed `DeusDeviceClient` operation on a synthetic session;
7. application ID validation remains enforced by existing typed wrapper behavior;
8. Asset operations use `PublishedOnly` and cannot request private/unpublished access;
9. all existing Core tests remain PASS.

Negative tests must explicitly prove unlisted/destructive/raw invocation is unavailable rather than merely untested.

## 11. Validation

Gate 1 static review:

- changed source/test set equals exactly the two frozen new files;
- no raw numeric/string RPC selector in public service-facing API;
- no protocol flags parameter;
- no direct call to `DeusDeviceClient.RpcAsync` inside the facade;
- one compile-time allowlist owner;
- no HTTP/Avalonia/native transport/target dependencies.

Gate 2:

- all `DeusOs.Control.Core.Tests` PASS with exact count recorded;
- `DeusOs.Control.Core` Release build PASS;
- `DeusOs.Control.Cli` Release build PASS;
- `DeusOs.Control.Desktop` Release build PASS;
- `git diff --check` PASS;
- no target I/O, Flash, firmware, bootloader or native transport mutation.

No physical hardware acceptance is required because this boundary adds no transport or target behavior.

## 12. Gates

- **Gate 0** — this documentation/source contract freeze.
- **Gate 1** — exact two-file implementation + static allowlist review.
- **Gate 2** — deterministic Core tests + Core/CLI/Desktop Release builds + exact pre/post evidence.
- **Gate 3** — canonical docs/checklist reconciliation.
- **Gate 4** — one normal local acceptance commit.
- **Gate 5** — ordinary non-force publication + fresh-fetch/GitHub verification.

## 13. Exit criterion

`FDC-03` is CLOSED only when a future service has a stable Core-owned typed operation surface whose allowlist is explicit, reviewable and incapable of routing caller-selected raw RPC IDs/flags; destructive operations are absent from service v1; negative tests prove raw/unlisted/destructive access is unavailable; regressions build cleanly; and the accepted source is published normally.

Closing FDC-03 does not activate a Web/service feature boundary and does not close FDC-04/FDC-09 or any target-side FDC item.
