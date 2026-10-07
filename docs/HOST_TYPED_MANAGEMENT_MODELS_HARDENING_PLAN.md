# Deus OS — Host Typed Management Models Hardening Plan

Status: **CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d` / `FDC-04`**

Published baseline commit: `0d9adfd8d0ed11478194e2268ede3c57379c8294`

## 1. Purpose

Close `FDC-04` before any Host Management Service/Web implementation is allowed.

The accepted Core already types negotiation/system identity, application snapshots and Asset/Configuration results. The remaining service-v1 management paths still expose raw `RpcResult` text for `Ping`, `Health`, `StartApplication` and `StopApplication`; Desktop directly renders `HealthAsync().OutputText`, and CLI directly interprets raw RPC output. A future service must not create a second presentation-specific parser for those target strings.

This boundary adds typed Core management models/parsers while preserving the accepted low-level `RpcAsync`/raw wrapper APIs for compatibility, diagnostics and FDC-01 correlation tests.

## 2. Frozen source boundary

Authorized product source paths are exactly:

- new `host/src/DeusOs.Control.Core/ManagementStateModels.cs`;
- modify `host/src/DeusOs.Control.Core/DeusDeviceClient.cs`;
- modify `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs`;
- modify `host/src/DeusOs.Control.Cli/Program.cs`;
- modify `host/src/DeusOs.Control.Desktop/DesktopController.cs`;
- modify `host/src/DeusOs.Control.Desktop/MainWindow.cs`.

Authorized tests are exactly:

- new `host/tests/DeusOs.Control.Core.Tests/ManagementStateModelTests.cs`;
- modify `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`.

Canonical documentation may be updated for acceptance/closure. No protocol/channel/session/native transport/firmware/bootloader/target source is authorized.

## 3. Compatibility rule

Existing low-level APIs remain available and unchanged:

- `DeusDeviceClient.RpcAsync(...)`;
- raw `PingAsync`, `HealthAsync`, `StartApplicationAsync`, `StopApplicationAsync` returning `RpcResult`.

FDC-04 adds typed management wrappers rather than redefining request-correlation or raw operator behavior. FDC-01/FDC-03 contracts therefore remain intact.

## 4. Frozen typed models

Core adds typed models for the remaining service-v1 raw surfaces:

- `PingStatus` — carries nonzero request ID after exact `PONG` validation;
- `HealthSnapshot` — `Tick`, `Pc13High`, `WatchdogActive`, `WatchdogReloadCount`, `ResetFlags`, `IwdgReset`;
- `ApplicationStartResult` — request ID, requested application ID and resulting active ID;
- `ApplicationStopResult` — request ID and resulting active ID.

`Applications`, `AssetStatus`, `ReadOledUiLayout`, `WriteOledUiLayout`, `SystemInfo` and `NegotiationResult` remain on their already typed models.

## 5. Parser contracts

### Ping

Input is bounded and must contain exactly one non-empty line equal to `PONG`. Any other payload is protocol failure.

### Health

Firmware shape is one CRLF-terminated line:

`HEALTH TICK=0x........ PC13=0x........ WDOG_ACTIVE=0x........ WDOG_RELOAD_COUNT=0x........ RESET_FLAGS=0x........ IWDG_RESET=0x........`

Parser requirements:

- bounded input length;
- exactly one non-empty line;
- exact `HEALTH` prefix;
- required keys exactly once;
- 32-bit hexadecimal values use frozen `0x` + eight-hex-digit shape;
- `PC13`, `WDOG_ACTIVE`, `IWDG_RESET` are only `0` or `1`;
- unknown additional key/value fields may be ignored for forward compatibility;
- malformed/missing/duplicate required data fails as `HostErrorKind.Protocol`.

### Application start/stop acknowledgement

Accepted firmware shapes:

- `APP_START_OK ID=0x........ ACTIVE_ID=0x........`;
- `APP_STOP_OK ACTIVE_ID=0x........`.

IDs must fit nonzero 16-bit application IDs where required; active ID may be zero after stop. Prefix/required fields are mandatory, unknown additional key/value fields may be ignored, and malformed values fail as protocol errors.

## 6. Typed Core wrappers

`DeusDeviceClient` adds typed wrappers which internally call the accepted raw wrappers, retain existing status/error handling, and parse only after successful RPC completion:

- `PingStatusAsync`;
- `HealthSnapshotAsync`;
- `StartApplicationControlAsync`;
- `StopApplicationControlAsync`.

No new RPC ID, flag, timeout policy or request-correlation path is introduced.

## 7. Consumer convergence

The following management consumers must use the new typed wrappers:

- `ManagementServiceOperations` exposes no `RpcResult` return type;
- CLI ping/health/app-start/app-stop consume typed models and only format presentation text;
- Desktop liveness/health/app-control consume typed models; Desktop must not inspect `RpcResult.OutputText`.

`rpcinfo` remains a CLI-only diagnostic/raw operator command and is explicitly excluded from service v1 by FDC-03. It is not promoted into the typed service state surface by FDC-04.

## 8. Boundedness and ownership

Parsing belongs only to `DeusOs.Control.Core`. CLI/Desktop/service may format typed fields but must not tokenize target output. No regex-heavy/unbounded parser, JSON/HTTP dependency, reflection-driven schema or dynamic firmware registry interpretation is authorized.

## 9. Required deterministic tests

Gate 2 must prove at minimum:

1. valid exact ping parses and preserves request ID;
2. malformed/multiline ping rejected;
3. valid health line maps all frozen fields;
4. unknown health field is tolerated;
5. missing/duplicate/malformed health field rejected;
6. health boolean fields reject values other than 0/1;
7. valid start acknowledgement maps requested/active IDs;
8. valid stop acknowledgement maps active ID including zero;
9. malformed/overflow application IDs rejected;
10. typed `DeusDeviceClient` wrappers route to the same accepted RPC IDs;
11. service facade public methods expose no `RpcResult` return type;
12. CLI/Desktop source contains no management `OutputText` parsing/rendering except explicitly retained CLI `rpcinfo` diagnostic output;
13. all existing Core regressions remain PASS.

## 10. Validation

Gate 1 static review:

- exact eight-path source/test boundary;
- raw compatibility methods remain present;
- typed service facade has zero `RpcResult` return types;
- parser ownership exists only in Core;
- no protocol/channel/session/native transport/target source change.

Gate 2:

- all Core tests PASS with exact count recorded;
- Core Release build PASS;
- CLI Release build PASS;
- Desktop Release build PASS;
- `git diff --check` PASS;
- exact pre/post path hashes;
- target I/O NONE / Flash mutation NONE.

No physical hardware acceptance is required because wire/target behavior is unchanged.

## 11. Gates

- Gate 0 — this documentation/source-boundary freeze.
- Gate 1 — exact implementation/static review.
- Gate 2 — Core tests + Core/CLI/Desktop Release builds + exact pre/post evidence.
- Gate 3 — canonical documentation reconciliation.
- Gate 4 — one normal local acceptance commit.
- Gate 5 — ordinary non-force publication + fresh remote/GitHub verification.

## 12. Exit criterion

`FDC-04` is CLOSED only when every service-v1 management state/control acknowledgement that previously escaped as raw `RpcResult` has an accepted typed Core representation or is explicitly excluded, service/Desktop/CLI consume those models without duplicating target-text parsing, existing low-level compatibility APIs remain intact, full regressions/builds pass and the accepted source is published normally.

## 13. Historical Gate-2/3 accepted candidate — pre-publication state

Gate 2 accepted `stm32_os_fdc04_host_validation_dotnet_v1_20261004_191243.evidence.zip`, SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`. ZIP CRC is clean and manifest verification is `78/78` exact. Core tests are `78/78` with failed/skipped/errors/not-run all zero; Core, CLI and Desktop Release builds PASS; target I/O and Flash mutation are zero.

Accepted source/test SHA-256 values are:

- `host/src/DeusOs.Control.Cli/Program.cs` — `D74C59FF6099081A595788978CF6B42D0D55406CCAC7B9145869F819DE860F03`;
- `host/src/DeusOs.Control.Core/DeusDeviceClient.cs` — `CEDB67F7314977BB8F8D6FB6CDEC45A15E88AFA3923C7807DA238FCCD0906CA8`;
- `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs` — `F255EF9AAAC391992E79D18F25223466D3EFA440B8EBB3747ABC36BFFECCE881`;
- `host/src/DeusOs.Control.Core/ManagementStateModels.cs` — `79AB1697CC2E1181B03B93DF4D0C3E30F1F4B44849DA83851DE100FD137D67D8`;
- `host/src/DeusOs.Control.Desktop/DesktopController.cs` — `14E139AAD8C4792AF3E3BD12DCC1EC505BE892BE2EA68558BD0077B982946E85`;
- `host/src/DeusOs.Control.Desktop/MainWindow.cs` — `BC4C81A7D7DBE5B3C0A069AD51C158FEA3EAC0AC674563101DBE770AE0737B73`;
- `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs` — `62B52078BC9B8E72E1A00642B2B88A8E1F59D54AB0A9049E81C13B872E375A4D`;
- `host/tests/DeusOs.Control.Core.Tests/ManagementStateModelTests.cs` — `2AB4F632F38D95E9CB46D9936B540216AEF887226142A2C903476487E787CD87`.

Static acceptance additionally proves `ManagementStateParser` is the Core parser owner, low-level raw compatibility APIs remain present, `ManagementServiceOperations` has zero `RpcResult` return types, Desktop has zero raw `OutputText` uses, and CLI has exactly one raw `OutputText` use confined to `rpcinfo`. Gate 3 canonical reconciliation is complete in the local publication candidate. FDC-04 remains publication-pending until Gates 4–5 complete.
