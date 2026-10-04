# Deus OS — Host Service Operation Allowlist Hardening Acceptance Plan

Status: **ACTIVE — GATE 0 / `FDC-03`**

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
