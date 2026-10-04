# Deus OS — Host Service Operation Allowlist Hardening Acceptance Plan

Status: **GATES 0–3 ACCEPTED / PUBLICATION PENDING / `FDC-03`**

Canonical design: `docs/HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING_PLAN.md`

Baseline commit: `42245d9d71504482fb189d8351ecce7049542145`

## Gate 0 — contract/source-boundary freeze

Gate 0 is docs-only.

Required:

- service-facing policy is owned by Core, not Web/HTTP/presentation code;
- exact host exposure classes are `ReadOnly`, `Control`, `Destructive`;
- exact service-v1 allowlist is frozen to five ReadOnly + three Control operations and zero Destructive operations;
- raw numeric RPC ID, protocol flags, generic argument forwarding and string-command proxy are forbidden from the service-facing API;
- `EnterBootloaderAsync`, `wdogtrip`, scheduler stress/diagnostics, UI mutation/test and unlisted IDs are unavailable;
- firmware `SAFE` / `DIAGNOSTIC` classes are explicitly not host authorization policy;
- future capability bits do not automatically expand the allowlist;
- FDC-04 remains responsible for typed result models/parsers;
- exact two-file source/test boundary is frozen;
- no service host/Web server/network listener/native transport/target mutation is authorized.

## Gate 1 — implementation/static review

Allowed paths are exactly:

- `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs`
- `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`

Static acceptance requires:

- one compile-time descriptor/catalog owner;
- public facade is typed and contains no `rpcId`, RPC `flags`, generic target argument list or raw command string;
- facade source contains no direct `RpcAsync(` call;
- facade source contains no `EnterBootloaderAsync(` call;
- exact descriptor set has no destructive operation and no dynamic registry expansion;
- Asset routes hard-code/use `AssetAccessPolicy.PublishedOnly`;
- Core source outside the new file remains byte-unchanged;
- CLI/Desktop/native transport/target source remains byte-unchanged.

## Gate 2 — deterministic host validation

Required tests:

- exact eight-operation catalog;
- exact 5 ReadOnly / 3 Control / 0 Destructive classification;
- undefined descriptor lookup rejected;
- facade reflection surface has no raw RPC/numeric/flags/generic-command method;
- destructive/update/raw RPC route unavailable;
- every allowed facade route reaches the corresponding existing typed Core operation;
- application validation and published Asset policy remain enforced;
- all existing FDC-01/FDC-02 and previous Core regressions remain PASS.

Validation requires:

- all `DeusOs.Control.Core.Tests` PASS, exact count recorded;
- `DeusOs.Control.Core` Release build PASS;
- `DeusOs.Control.Cli` Release build PASS;
- `DeusOs.Control.Desktop` Release build PASS;
- exact two-path pre/post WIP hashes;
- `git diff --check` PASS;
- target I/O NONE / Flash mutation NONE.

## Gate 3 — documentation reconciliation

After Gate 2 PASS:

- mark FDC-03 implementation/test checklist criteria complete;
- record exact source hashes/test count/build results/evidence identity;
- update CURRENT_STATE/ROADMAP without claiming Host Management Service unblocked;
- preserve FDC-01/FDC-02 published history;
- keep FDC-04/FDC-09 and all target FDC items open.

## Gate 4 — local acceptance commit

Require exact accepted source/tests/docs staged, `git diff --cached --check` PASS, no unrelated source/untracked residue, one normal local commit and direct-parent proof against remote main.

## Gate 5 — publication

Ordinary non-force push only. Post-push require fresh fetch, `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`, and independent GitHub main verification.

## Failure classifications

At minimum distinguish:

- `FDC03_SOURCE_SCOPE_DRIFT`
- `FDC03_RAW_RPC_EXPOSURE`
- `FDC03_ALLOWLIST_DRIFT`
- `FDC03_DESTRUCTIVE_EXPOSURE`
- `FDC03_ROUTE_MAPPING_FAILURE`
- `FDC03_CORE_TEST_FAILURE`
- `FDC03_BUILD_FAILURE`
- `FDC03_DOC_RECONCILIATION_FAILURE`

A failure does not authorize Web/service implementation, target changes, native transport changes or folding FDC-04/FDC-09 into this slice.

## Accepted Gates 1–3 record — 2026-10-04

Gate 1 candidate is exactly two new files on Gate-0 publication baseline `1fe0315916deaf551dc0249f2dffb424abd5e7dc`:

- `ManagementServiceOperations.cs` SHA-256 `299F3344EF1BF79736DEB5A9D5ED6D2E1AA63513683DE6711BBD8FAFCD1CE668`;
- `ManagementServiceOperationTests.cs` SHA-256 `A7A43385AE9F1BF754BCDD56E2AF17781AAD871584766374C228D49F539259D1`.

Authoritative Gate-2 evidence: `stm32_os_fdc03_host_validation_dotnet_v2_20261004_184243.evidence.zip`, SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`.

Accepted proof:

- ZIP CRC PASS; manifest `78/78` exact;
- prestate/poststate `HEAD == origin/main == 1fe0315916deaf551dc0249f2dffb424abd5e7dc`;
- tracked diff `0`, staged `0`, untracked set exactly the two frozen candidate files;
- static catalog proof: total `8`, ReadOnly `5`, Control `3`, Destructive `0`;
- raw RPC call absent; bootloader route absent; caller protocol flags absent; dynamic registry expansion absent;
- exactly three Asset routes use `AssetAccessPolicy.PublishedOnly`;
- Core tests `66/66`, failed/skipped/errors/not-run all `0`;
- Core Release build PASS;
- CLI restore + Release build PASS;
- Desktop restore + Release build PASS;
- target I/O NONE; Flash mutation NONE.

The earlier validator-v1 run correctly reported `66 total / 64 passed / 2 failed`; both failures were invalid synthetic `sysinfo` capability fixtures that violated the already frozen mandatory base capability mask before facade execution. The fixture-only correction was revalidated by the full v2 matrix above.

Gate 3 documentation reconciliation is accepted. Gate 4/5 commit/publication remain pending; therefore FDC-03 is not yet CLOSED/PUBLISHED and FDC-04 source mutation remains unauthorized.
