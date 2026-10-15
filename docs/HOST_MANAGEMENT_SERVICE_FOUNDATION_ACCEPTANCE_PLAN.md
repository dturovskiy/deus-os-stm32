# Deus OS — Host Management Service Foundation Acceptance Plan

Status: **GATE-0 ORIGINAL DESIGN AND LOCKFILE AMENDMENT A PASS/PUBLISHED** — original SHA `bfb30d5f23389b54f0c114959640dc2d5a6291f9`, hosted CI `38732869436` SUCCESS; Amendment A SHA `0f95c2e5036652251976a0f7d6d5f62ea419116d`, hosted CI `38733800122` SUCCESS. **Stage-A Gate 1 source/test validation pending; no source/target changes authorized beyond the frozen nine-path set.**

Product boundary: `HOST_MANAGEMENT_SERVICE_FOUNDATION`.
Design owner: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`.
Global status authority: `docs/CURRENT_STATE.md`.
Pre-Gate-0 public base: `13889b4f2c509c9b83a1945f9b899add5d0164d2`; its actual Git tree and live CI must be inspected from Git/Actions, not inferred from this text.

## Gate 0 — scoped selection, risk freeze, publication

**PASS requires all:**

1. Existing Git public/private `main` fetched, clean, identical to origin with ahead/behind 0/0; base public exact `13889b4...` and latest real hosted Host CI SUCCESS, Core >=84 and Transport >=24, 0 failed/skipped and clean Release.
2. Owner-selected eligible Host Management Service foundation is represented as a **new candidate**, distinct from the completed FDC/RDC/sanitization programmes. `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` remains a planning reference, not retrospectively marked implemented.
3. New plan/acceptance and canonical `CURRENT_STATE`, `ROADMAP`, `MASTER_EXECUTION_CHECKLIST`, `CHANGELOG` agree on exact Stage-A scope, no HTTP listener and Stage-B browser/network as a separate gated feature. The accepted old FDC-03 allowlist remains 5 ReadOnly / 3 Control / 0 Destructive, whereas the **new process-facing exposure is the five ReadOnly subset only**.
4. Exactly these **six docs-only paths** are written/staged for Gate 0: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`, `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`, `docs/CURRENT_STATE.md`, `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`. Empty real index prestate; no file outside these six, no firmware/Host/CI/tool/script/key/WIP/target I/O changes.
5. Explicit hazards frozen: localhost is not authentication, no network/HTTP/listener in Stage A, no raw/disallowed/destructive/update/control operation, no auto-open/auto-reconnect outside Core, no bypass of PublishedOnly policy, no new source dependency or service autostart; Core and transport error/lifecycle invariants retained.
6. Historical firmware resource evidence and 52 KiB executable / 20 KiB physical SRAM layout are accurately represented without asserting a fresh ARM measurement. New source paths Stage-A are recorded only as **future Gate-1 authorization conditional on this Gate-0 acceptance and fresh fit review**.
7. Independently inspect staged diff, non-vacuous `git diff --cached --check`, actual changed paths and outgoing commit's direct parent; one ordinary non-force docs-only commit/push; fresh fetch `HEAD == origin/main == FETCH_HEAD`, clean 0/0 and **GitHub Actions SUCCESS on that exact Gate-0 commit**. Host Core >=84, Transport >=24, 0 skipped/failed, Release warnings/errors 0, hygiene PASS. A CI run for an earlier SHA cannot close Gate 0.
8. If any predicate fails, **Gate 0 stays OPEN/FAIL**; no Stage-A code is to be written. Preserve exact failure evidence; no speculative retry or widening patch.

Gate 0 must classify final result `DESIGN_GATE0_ACCEPTED_PUBLISHED` or `GATE0_OPEN` with the exact GitHub CI run and accepted SHA. It is **not** a real service test.

## Gate-0 original evidence / mandatory Amendment A — 2026-10-15

The exact six-path docs design Gate-0 source was published by ordinary non-force commit `bfb30d5f23389b54f0c114959640dc2d5a6291f9`; fresh fetch confirmed clean public main ahead/behind 0/0. Hosted Actions [run 38732869436](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38732869436) tied to this exact SHA is **SUCCESS**: Core **84/84**, Transport **24/24**, failed/skipped zero, Release 0 warnings/errors, hygiene **273**. `ORIGINAL_DESIGN_GATE0=PASS_PUBLISHED`.

The original frozen Gate-1 path list omitted two necessary `packages.lock.json` files for the newly introduced Service executable and Service.Tests project under global locked restore. This is a genuine scope omission **not** a reason to bypass `--locked-mode`. The narrow Amendment A is docs-only: add exactly those two project-local lockfiles to the future Gate-1 path allowlist (nine total); no changes to runtime behavior, operation allowlist, target, CI semantics or network exposure. Amendment A **PASSED** after ordinary non-force publication at `0f95c2e5036652251976a0f7d6d5f62ea419116d` and fresh-fetch clean `0/0`. Its actual [GitHub Actions 38733800122](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38733800122) is **SUCCESS** on the exact SHA: Core 84/84, Transport 24/24, zero failed/skipped, Release warning/error zero and hygiene 273. Gate-1 source may begin under the amended scope, but no actual Stage-A build/test or real runtime outcome has been accepted.

## Gate 1 — exact Stage-A source review (future; blocked until amended Gate 0 PASS)

Review the Stage-A **nine-path** source/test/solution/CI + **two mandatory project lockfiles** set frozen by Gate-0 Amendment A in the plan; reject unapproved Core/CLI/Desktop/firmware/source drift and any listening endpoint. Verify one process/session owner over `DeusDeviceSession`, five typed read-only operations using `ManagementServiceOperations`, default disconnected state, no raw RPC selectors/flags, no three Control routes at process boundary. Require actual test fixture injection without physical device.

If the initial runtime needs a dependency/new source file or a platform-specific service registration, freeze and publish a **Gate-0 amendment** before editing. No hidden coupling or packaging under the same implementation commit.

## Gate 2 — synthetic Host execution (future)

PASS: actual compiled Service project plus executed offline tests covering initial disconnected state, selected discovery/connect, typed operation delegation, missing capabilities, repeated cancellation, disconnected target, reconnection, state callbacks, concurrent requests, slow I/O timeout and idempotent shutdown. Synthetic test failures are real failures, not proof of hardware failure. Also execute existing Host Core **>=84**, Transport **>=24**, zero failed/skipped and Release warning/error free with locked restore, actual counts, no new uncontrolled packages. Do not claim ARM/target hardware from these tests.

## Gate 3 — scoped physical/session acceptance (future)

If and only if separately authorized and justified, check the exact accepted host source on Windows/Linux, real selected-device discovery/HELLO/health/reconnect with typed operation semantics and negative policy controls. Preserve physical target revision/Flash/bootloader/USB/IWDG/OLED and avoid unnecessary power cycles. Actual device checks are **NOT ACCEPTED** until executed, regardless of Gate-2 synthetic outcome.

## Gates 4–6 — accepted commit, publication, closure (future)

Gate 4 requires exact staged diff, no extra files and reviewed accepted tests; Gate 5 requires ordinary non-force push/fetch/hosted exact-SHA CI; Gate 6 reconciles all public status/acceptance docs and publishes its own exact-SHA closure proof. `CLOSED/PUBLISHED` cannot be inferred from a plan or a draft. Web frontend/local HTTP security and firmware/network enhancements remain **separate unpromoted features**.

## Failure classifications / invariants

`GATE0_DOC_SCOPE_DRIFT`, `GATE0_PARENT_MISMATCH`, `GATE0_CI_UNVERIFIED`, `SERVICE_PROCESS_EXPOSURE`, `SERVICE_UNAUTHORIZED_CONTROL`, `SERVICE_RAW_RPC_ROUTE`, `SERVICE_SESSION_LIFETIME`, `SERVICE_TEST_FAILURE`, `HOST_TEST_COUNT_DRIFT`, `RESOURCE_CLAIM_UNSUPPORTED`, `UNAUTHORIZED_TARGET_OR_SECRET_ACCESS`.

This acceptance does **not** reopen previous key/custody work, does **not** authorize publication of a private artifact and does **not** require firmware build/hardware reflash for a Host-only change.
