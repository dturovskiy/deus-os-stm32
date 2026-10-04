# Deus OS — Host Typed Management Models Hardening Acceptance Plan

Status: **GATES 0–3 ACCEPTED / PUBLICATION PENDING / `FDC-04`**

Canonical design: `docs/HOST_TYPED_MANAGEMENT_MODELS_HARDENING_PLAN.md`

Published baseline commit: `0d9adfd8d0ed11478194e2268ede3c57379c8294`

## Gate 0 — contract/source-boundary freeze

Required:

- low-level raw `RpcAsync` and existing raw wrappers remain compatibility/operator APIs;
- typed Core management models are added for Ping, Health, app-start and app-stop acknowledgement;
- `Applications`, Asset results and negotiation/system info reuse existing typed models;
- service-facing facade exposes no `RpcResult` return type after implementation;
- CLI/Desktop consume typed models for management paths and do not parse/render raw management `OutputText`;
- CLI `rpcinfo` remains explicitly excluded diagnostic/raw operator output under FDC-03 service policy;
- no protocol/channel/session/native transport/target source mutation is authorized.

## Gate 1 — implementation/static review

Allowed paths are exactly:

- `host/src/DeusOs.Control.Core/ManagementStateModels.cs` (new);
- `host/src/DeusOs.Control.Core/DeusDeviceClient.cs`;
- `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs`;
- `host/src/DeusOs.Control.Cli/Program.cs`;
- `host/src/DeusOs.Control.Desktop/DesktopController.cs`;
- `host/src/DeusOs.Control.Desktop/MainWindow.cs`;
- `host/tests/DeusOs.Control.Core.Tests/ManagementStateModelTests.cs` (new);
- `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`.

Static acceptance requires:

- exact eight-path mutation set;
- Core owns all new target-text parsing;
- `ManagementServiceOperations` public async methods return typed models / `Task`, never `RpcResult`;
- raw compatibility methods still exist in `DeusDeviceClient`;
- typed wrappers call accepted raw wrappers rather than raw numeric `RpcAsync` directly;
- CLI/Desktop have no health/ping/app-control `OutputText` parsing/rendering;
- no Core protocol/channel/session/native transport/firmware/target change.

## Gate 2 — deterministic host validation

Parser/model tests must cover:

- exact valid ping + request ID;
- invalid/multiline ping;
- exact health mapping;
- unknown health field compatibility;
- missing/duplicate/bad-hex health rejection;
- boolean health range validation;
- exact app-start/app-stop acknowledgement mapping;
- 16-bit application ID bounds;
- malformed prefix/shape rejection;
- typed client/facade routing to existing RPC IDs.

Regression validation requires:

- all Core tests PASS, exact count recorded;
- Core Release build PASS;
- CLI Release build PASS;
- Desktop Release build PASS;
- exact eight-path pre/post hashes;
- `git diff --check` PASS;
- target I/O NONE / Flash mutation NONE.

## Gate 3 — documentation reconciliation

After Gate 2 PASS:

- record exact source/test hashes, test count/build outcomes and evidence identity;
- mark FDC-04 implementation/parser/consumer criteria complete;
- preserve FDC-01..03 published history;
- keep FDC-09 and target-side FDC items open;
- do not claim Host Management Service/Web unblocked.

## Gate 4 — local acceptance commit

Stage only the accepted source/tests/docs set, require `git diff --cached --check` PASS, no unrelated residue, one normal commit and direct-parent proof against remote main.

## Gate 5 — publication

Ordinary non-force push only. Require fresh fetch, `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0` and independent GitHub main verification.

## Failure classifications

At minimum:

- `FDC04_SOURCE_SCOPE_DRIFT`
- `FDC04_TYPED_SURFACE_DRIFT`
- `FDC04_RAW_PRESENTATION_PARSING`
- `FDC04_PARSER_SHAPE_FAILURE`
- `FDC04_CORE_TEST_FAILURE`
- `FDC04_BUILD_FAILURE`
- `FDC04_DOC_RECONCILIATION_FAILURE`

A failure does not authorize service/Web implementation, transport changes or target changes.

## Gate 2/3 accepted result — publication pending

Authoritative evidence: `stm32_os_fdc04_host_validation_dotnet_v1_20261004_191243.evidence.zip`, SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`.

Accepted proof:

- ZIP CRC PASS; manifest `78/78` exact;
- exact pre/post repository baseline `HEAD == origin/main == f0614d4182b5f603f43eb79b5dd011b929de0e8c`;
- exact candidate shape: six modified tracked paths + two SHA-bound new paths, staged count zero;
- Core `78/78`, failed/skipped/errors/not-run all zero;
- Core Release build PASS;
- CLI Release build PASS;
- Desktop Release build PASS;
- `MODEL_PARSER_OWNER=DeusOs.Control.Core.ManagementStateParser`;
- low-level raw compatibility APIs preserved;
- service-facade `RpcResult` return types `0`;
- Desktop raw `OutputText` uses `0`;
- CLI raw `OutputText` uses `1`, confined to `rpcinfo`;
- protocol/channel/session/native transport/target source unchanged;
- target I/O NONE; Flash mutation NONE;
- live repository poststate exactly matches prestate and source hashes are unchanged.

Gate 3 documentation reconciliation records the evidence and accepted source hashes without widening scope. Gate 4/5 publication remains required before `FDC-04` is CLOSED.
