# Deus OS — Host Asset Request Correlation Hardening Acceptance Plan

Status: **ACTIVE — GATES 0–3 ACCEPTED / GATE 4 COMMIT PENDING**

Canonical design:

`docs/HOST_ASSET_REQUEST_CORRELATION_HARDENING_PLAN.md`

Baseline:

`0713da56a3fe7a6b62f4c548d3248b4ed7fb2e57`

## Gate 0 — contract/source freeze

PASS requires:

- confirmed defect is limited to Asset single-response correlation after timeout/cancel;
- existing `DeviceProtocolChannel.AbandonSingleResponse()` is reused;
- exact implementation path is `AssetTransferClient.cs`;
- deterministic tests are confined to `ClientTests.cs`;
- target/native transport/scripts/wire ABI are excluded;
- Asset protocol `ABORT` cleanup and retries are explicitly excluded.

## Gate 1 — implementation/static review

PASS requires:

- once a nonzero Asset request ID exists, native `HostErrorKind.Timeout` registers `SingleResponse` abandonment before propagation;
- once a nonzero Asset request ID exists, `OperationCanceledException` registers the same abandonment before propagation;
- successful requests create no abandoned state;
- zero request ID is never registered;
- protocol/CRC/request-correlation failures remain failures and are not ignored;
- no second decoder, request-ID allocator, stale queue or timeout-expiry owner is introduced.

### Gate-1 result — PASS

Static review on baseline `0713da56a3fe7a6b62f4c548d3248b4ed7fb2e57` confirms exactly five current candidate paths: this plan pair, `docs/CURRENT_STATE.md`, `host/src/DeusOs.Control.Core/AssetTransferClient.cs`, and `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs`. Product mutation is limited to `AssetTransferClient.cs`; `DeviceProtocolChannel`, RPC, Firmware Update, native transports, target firmware, scripts and wire definitions are unchanged. `AssetRequestCoreAsync()` reuses channel-owned `AbandonSingleResponse()` only for a nonzero request ID on native `HostErrorKind.Timeout` or `OperationCanceledException`; success and non-timeout protocol failures are unchanged. `git diff --check` is clean. Core test inventory is 80 static `[Fact]/[Theory]` methods after adding exactly two Asset correlation regressions.

## Gate 2 — deterministic host validation

Required regressions:

- Asset cancellation -> delayed old Asset response -> fresh operation PASS;
- Asset native timeout -> delayed old Asset response -> fresh operation PASS;
- fresh request uses next request ID;
- existing unknown request-ID rejection PASS;
- existing abandoned RPC stream tests PASS;
- existing Firmware Update single-response/retry tests PASS;
- complete Core test suite PASS;
- Core Release build PASS;
- `git diff --check` PASS.

No target I/O, Flash mutation or physical hardware operation is authorized.

### Gate-2 result — PASS

Authoritative evidence: `stm32_os_host_asset_request_correlation_gate2_host_validation_v2_20261007_181012.evidence.zip`, SHA-256 `F574489FE68B35D42DEB8BB0E8EE88FB029B8FAAED5C7F926BB7194C256D3285`.

Accepted facts:

- evidence hash index `39/39` exact;
- exact baseline/prestate `HEAD == origin/main == 0713da56a3fe7a6b62f4c548d3248b4ed7fb2e57` with empty real index;
- exact five-path candidate and SHA-256 set preserved before/after validation;
- candidate `host/global.json` in scope under .NET SDK `10.0.201` with Microsoft.Testing.Platform;
- isolated locked restore completed in the harness-owned NuGet cache;
- `DeusOs.Control.Core` Release build PASS with `0` warnings and `0` errors;
- executed Core suite `80/80` PASS, failed `0`, skipped `0`;
- full candidate temporary-index `git diff --cached --check` PASS;
- `TARGET_IO=NONE`, `FLASH_MUTATION=NONE`.

The superseded v1 harness is not product evidence: it used legacy positional project syntax under .NET 10/Microsoft.Testing.Platform and terminated before test execution. V2 corrected only the harness invocation and revalidated the unchanged five-path candidate.

## Gate 3 — closure reconciliation

Record:

- exact changed paths;
- exact Core test count/result;
- exact source/test hashes or accepted candidate identity;
- explicit disposition of the separate volatile Asset-session cleanup question.

Gate 3 must not claim that request-correlation hardening implicitly closes transaction `ABORT`/session cleanup.

### Gate-3 result — PASS

Accepted candidate source/test identity remains:

- `host/src/DeusOs.Control.Core/AssetTransferClient.cs` SHA-256 `7A38398D72496F25C34888EF9DC4802E3A57D932C963078E03B6B070139FB7D5`;
- `host/tests/DeusOs.Control.Core.Tests/ClientTests.cs` SHA-256 `309CF7D6FDF3BC02766AE409C55C22F30D62F403EE6A4BF6C563829072051559`.

Closure documentation adds only this plan/acceptance record, `docs/CURRENT_STATE.md` and `CHANGELOG.md`; no additional product source is reopened. The remaining volatile target-session question is explicitly **not closed** here: best-effort Asset `ABORT` after ambiguous BEGIN/WRITE/COMMIT failure remains a separate possible maintenance boundary and must not be inferred from this correlation acceptance.

## Gate 4 — local acceptance commit

Require exact staged-path review, no unrelated files, staged diff check PASS, one normal local commit and clean post-commit state.

## Gate 5 — ordinary non-force publication

Require fresh fetch/direct-parent proof before push. After push require fresh fetch with `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## Failure classes

- `HOST_ASSET_CORRELATION_SOURCE_SCOPE_DRIFT`
- `HOST_ASSET_CORRELATION_STALE_RESPONSE_FAILURE`
- `HOST_ASSET_CORRELATION_UNKNOWN_ID_WEAKENED`
- `HOST_ASSET_CORRELATION_RPC_OR_UPDATE_REGRESSION`
- `HOST_ASSET_CORRELATION_CORE_TEST_FAILURE`
- `HOST_ASSET_CORRELATION_BUILD_FAILURE`
- `HOST_ASSET_CORRELATION_DOC_FAILURE`
