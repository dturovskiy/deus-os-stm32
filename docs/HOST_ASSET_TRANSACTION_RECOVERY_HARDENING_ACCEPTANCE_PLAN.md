# Deus OS — Host Asset Transaction Recovery Hardening Acceptance Plan

Status: **CLOSED / PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b` — GATES 0–5 ACCEPTED**

Canonical design:

`docs/HOST_ASSET_TRANSACTION_RECOVERY_HARDENING_PLAN.md`

Residual-debt item:

`RDC-01`

Baseline:

`7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`

## Gate 0 — contract/source freeze

PASS requires:

- exact Host-only source/test scope;
- no target/native transport/script/wire mutation;
- `sessionMayBeActive` starts before BEGIN can reach target;
- successful COMMIT closes that window;
- independent cleanup deadline = 2 seconds;
- cleanup uses existing `AssetRequestCoreAsync()` + channel-owned single-response abandonment;
- primary transaction failure is always authoritative;
- no mutation retry/resume.

## Gate 1 — implementation/static review

PASS requires:

- only `AssetTransferClient.cs` changes in product code;
- cleanup is attempted only when transfer ID is nonzero and session may be active;
- ABORT uses same transfer ID, object type `OLED_UI_LAYOUT_CONFIG_V1`, flags zero;
- cleanup uses an independent bounded token;
- expected cleanup failures are swallowed;
- pre-BEGIN/pre-write STATUS failure sends no ABORT;
- successful COMMIT prevents later validation/readback failures from sending ABORT;
- no additional correlation owner, decoder, queue, retry loop or timer framework.

### Gate-1 result — PASS

Static review on baseline `7ba9b303b176f8329be98f29f3f1ce1e6ad5e510` confirms exactly five candidate paths: this plan pair, `docs/CURRENT_STATE.md`, `host/src/DeusOs.Control.Core/AssetTransferClient.cs`, and `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`. Product mutation is confined to `AssetTransferClient.cs`. The implementation introduces only one transaction-local `sessionMayBeActive` flag plus one private bounded cleanup helper; it reuses `AssetRequestCoreAsync()` and existing channel-owned `SingleResponse` abandonment. ABORT uses flags zero and the same transfer ID; no retry, decoder, queue, target, protocol, transport or tooling source changed. Four deterministic regressions raise the Core static test inventory from 80 to 84. `git diff --check` is clean.

## Gate 2 — deterministic host validation

Required regressions:

- ambiguous BEGIN timeout + delayed old BEGIN response -> ABORT on fresh request ID -> original timeout preserved;
- WRITE_CHUNK status/protocol failure -> ABORT attempted -> cleanup failure cannot mask primary failure;
- caller cancellation -> independent ABORT attempt -> caller cancellation preserved;
- post-COMMIT verification failure -> no ABORT;
- existing Asset correlation regressions PASS;
- complete Core suite PASS;
- Core Release build PASS with zero warnings/errors;
- `git diff --check` PASS;
- TARGET_IO=NONE / FLASH_MUTATION=NONE.

### Gate-2 result — PASS

Authoritative evidence: `stm32_os_host_asset_transaction_recovery_gate2_host_validation_v1_20261007_190709.evidence.zip`, SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA`.

Accepted facts:

- evidence hash index `39/39` exact;
- exact prestate `HEAD == origin/main == 7ba9b303b176f8329be98f29f3f1ce1e6ad5e510` with empty real index;
- exact five-path candidate and SHA-256 set preserved before/after validation;
- candidate `host/global.json` in scope under .NET SDK `10.0.201` / Microsoft.Testing.Platform;
- isolated locked restore completed in the harness-owned cache;
- Core Release build PASS with `0` warnings and `0` errors;
- executed Core suite `84/84` PASS, failed `0`, skipped `0`;
- temporary-index `git diff --cached --check` PASS;
- `TARGET_IO=NONE`, `FLASH_MUTATION=NONE`.

Evidence bookkeeping note: `outcome.txt` contains a stale literal `CORE_STATIC_TEST_METHODS=80`. This is classified as `HARNESS-EVIDENCE-RECORD-CARDINALITY-01`, not a product/test failure. The same hash-verified bundle independently proves the correct value three ways: package manifest `expected_core_test_methods=84`, `run.log` static inventory `84`, and executed MTP test summary `total: 84 / failed: 0`. No rerun is required because candidate bytes, raw execution outputs and poststate are already complete and self-consistent apart from that one stale summary field.

## Gate 3 — closure reconciliation

Record exact changed paths, test count, source/test SHA-256, evidence SHA-256 and explicit confirmation that no target/wire/retry behavior changed.

### Gate-3 result — PASS

Accepted product/test identity:

- `host/src/DeusOs.Control.Core/AssetTransferClient.cs` SHA-256 `83CCDB46BEDDD84471305F4C47907F17559519F95540B798763512F2463A8DEB`;
- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs` SHA-256 `DEC3093BDC0F911B66A2A17AD351E23AAB10B5F6D7B06AD3F46BA3697D1B8530`;
- Core deterministic suite `84/84` PASS.

RDC-01 closes only bounded Host-side volatile transaction/session cleanup. It changes no Asset opcode/status/flag encoding, no target session timeout/persistence behavior, no native transport, no retry/resume semantics and no target/Flash/hardware state.

## Gate 4 — local acceptance commit

Require exact staged-path review, no unrelated files, staged diff check PASS, one normal local commit and clean post-commit state.

### Gate-4 result — PASS

Exact staged set contained the five accepted boundary paths only. `git diff --cached --check` PASS. Local acceptance commit `699382a58c4c9570cdf36a05693044461e87c58b` (`fix: harden host asset transaction recovery`) is a direct child of baseline `7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`; post-commit worktree/index was clean with ahead/behind `1/0`.

## Gate 5 — ordinary non-force publication

Require fresh fetch/direct-parent proof before push. After push require fresh fetch with `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

### Gate-5 result — PASS / PUBLISHED

Fresh pre-push fetch proved `origin/main == 7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`, exactly the parent of acceptance commit `699382a58c4c9570cdf36a05693044461e87c58b`. Publication used ordinary non-force fast-forward `7ba9b30..699382a  main -> main`. Fresh post-push fetch proved `HEAD == origin/main == 699382a58c4c9570cdf36a05693044461e87c58b`, clean worktree/index and ahead/behind `0/0`.

`RDC-01 / HOST_ASSET_TRANSACTION_RECOVERY_HARDENING` is CLOSED/PUBLISHED. No target/wire/retry behavior was promoted by this closure.

## Failure classes

- `HOST_ASSET_TRANSACTION_SOURCE_SCOPE_DRIFT`
- `HOST_ASSET_TRANSACTION_ABORT_NOT_ATTEMPTED`
- `HOST_ASSET_TRANSACTION_PRIMARY_ERROR_MASKED`
- `HOST_ASSET_TRANSACTION_ABORT_AFTER_COMMIT`
- `HOST_ASSET_TRANSACTION_CORRELATION_REGRESSION`
- `HOST_ASSET_TRANSACTION_CORE_TEST_FAILURE`
- `HOST_ASSET_TRANSACTION_BUILD_FAILURE`
- `HOST_ASSET_TRANSACTION_DOC_FAILURE`
