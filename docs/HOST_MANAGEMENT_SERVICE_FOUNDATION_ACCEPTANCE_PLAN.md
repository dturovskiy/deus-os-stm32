# Deus OS — Host Management Service Foundation Acceptance Plan

**Historical 2026-10-15 architecture re-review C result (the wrapper NO-GO remains binding): `WRAPPER_NO_GO / CONSUMER_REQUIRED` at that decision point.** The architecture-C no-go was **accepted/published** in `87639102c938146c4f68d4cb2b52df18ab591374`, actual exact-SHA GitHub Actions `38744214870` SUCCESS and clean fresh-fetch `0/0`. This proves a docs-only architecture decision, not a Host service runtime. The subsequently selected Desktop Published Asset Inspector became the actual Host consumer: source/CI accepted at `4729341` (run `38748957900`), Ubuntu CLI Gate 3b.1 PASS and connected Windows Avalonia **read-only fallback Gate 3b `OPERATOR_ATTESTED_PASS`** on 2026-10-16. Positive physical retrieval of a previously committed OLED layout is **NOT_VERIFIED**. No *additional* Host consumer is currently activated; a future TUI/Web requires its own design/security/acceptance boundary. Source implementation of `HostManagementRuntime` is **NOT AUTHORIZED** by this revision.

Canon: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`. Global state: `docs/CURRENT_STATE.md`.

## 1. Audited primary-source facts and review findings

| ID | Source-grounded observation | Classification / disposition |
|---|---|---|
| ARC-C01 | `DesktopController.cs:7-77` already composes platform discovery + one `DeusDeviceSession`, exposes state, connect, typed calls and teardown. | **High — duplicate responsibility.** Do not introduce another unconsumed lifecycle facade with the same behavior. |
| ARC-C02 | `DeusDeviceSession.cs:45-175,205-242` serializes connects/operations/disconnect and owns recovery/state. | **High — single-owner invariant.** No parallel session/locks/retry scheduler. |
| ARC-C03 | `ManagementServiceOperations.cs:102-173` already exposes 5 read-only methods (assets `PublishedOnly`) plus 3 Control methods; Core's frozen operation catalog is 5/3/0. | **High — double facade/security confusion.** A narrowed wrapper without a real consumer is unnecessary and is **not** authorization. Future Web must enforce explicit route permissions. |
| ARC-C04 | `Cli/Program.cs:9-44,81-111` already provides discovery and explicit locator selection as a CLI composition root. | **Medium — avoid speculative shared selector.** Different consumers legitimately choose different selection UX. |
| ARC-C05 | Previous Stage A described only a library class; neither CLI, Desktop nor a new Web/HTTP host was allowed to consume it. | **High — no user-facing acceptance.** Retire the two-file runtime-only Gate 1; require a concrete consumer with observable acceptance. |
| ARC-C06 | `DeusDeviceSession.DisposeAsync()` awaits gate without cancellation and native disposal; queued callbacks are asynchronous; recovery matches locator equality. | **Medium — bounded contract only.** Hard shutdown/board identity/synchronous delivery are **NOT_PROVEN**, keep as explicit limits. |
| ARC-C07 | Discovery→selection happens outside Core's operation gate and real consumers may race concurrent selection. | **Medium — consumer-level concurrency requirement**, not a reason to add another global lock prematurely. |
| ARC-C08 | `DOCUMENTATION_MODEL.md` and `HOST_MANAGEMENT_PRESENTATION_MODEL.md` still say no Host product boundary is active; `CURRENT_STATE.md:63` and `ROADMAP.md:326` carry obsolete Gate-0 wording despite published Correction B. | **Doc drift.** Reconcile authority, keep dated original snapshots recognizable, do not silently rewrite history. |
| ARC-C09 | Existing .NET solution uses SDK default .cs inclusion, locked NuGet, Host Core 84 / Transport 24 baseline. | **No project/lockfile/CI changes warranted by docs-only review.** Older `CONTROL_PLANE_OUTCOME_AMBIGUOUS` .NET runner attempts do not prove a compiler issue. |
| ARC-C10 | Previous additional source-proposal files do **not** exist in tracked `main`; public delta from original baseline is six Markdown files and Git has no WIP. | **No new product code, no introduced runtime cost.** CI is only baseline Host CI, not new feature acceptance. |

## 2. Architecture/SOLID decision

- **SRP:** keep Core session ownership, typed facade policy and consumer-level presentation/selection as separate existing responsibilities. A one-class wrapper for all three is not demanded by a real consumer.
- **DIP:** retain `IDeviceDiscovery` injection and platform implementations outside Core; no new dependency direction or service-layer assembly needed.
- **ISP:** expose read-only operations only through the concrete actual consumer's contract; do not confuse compile-time class surface with host authorization.
- **OCP/LSP:** do not create speculative polymorphism/inheritance, new shared interfaces or factories without an actual variation point.
- **DRY/YAGNI:** the proposed in-process `HostManagementRuntime` is an **unconsumed duplicate** now. `WRAPPER_NO_GO`; an actual desktop/browser/user-facing feature may justify later extracting **only measured duplicated nontrivial orchestration**, rather than mandating a two-file dummy product.

This **does not** classify the existing DesktopController or CLI as broken/costly hacks. They are established consumer-specific adapters with different application requirements. Separate direct-operation call sites in Desktop can be consolidated through `ManagementServiceOperations` when a real Desktop change is scoped, but **do not force a refactor absent tests or product need**.

## 3. Current gate scope and acceptance

This re-review is a **docs-only decision gate**, not a renamed or silently executed Gate-1 implementation. Expected changed paths: exactly
`docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`,
`docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`,
`docs/CURRENT_STATE.md`,
`docs/ROADMAP.md`,
`docs/DOCUMENTATION_MODEL.md`,
`docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`,
`docs/MASTER_EXECUTION_CHECKLIST.md`,
`CHANGELOG.md`.

Required acceptance predicates: (1) public/private existing Git clean, baseline `bdc5cdfe0faca0bc06f515517d7c7c230474cb83` and previous CI `38740180025` SUCCESS; (2) one source-grounded NO-GO finding trace in the paired plan and this acceptance, no unsupported `SOLID_PASS` or new-code claim; (3) all eight paths docs-only, no executable/firmware/bootloader/CI/STM32/private-material changes; (4) staged non-vacuous `git diff --cached --check` passes; (5) direct-parent ordinary non-force publication; (6) fresh fetch `HEAD == origin/main`, clean `0/0`; (7) **new exact-SHA GitHub CI SUCCESS**, actual Core ≥84 and Transport ≥24, 0 failed/skipped, Release 0 warnings/errors and output hygiene PASS. If any fails, keep `DOC_REVIEW_PUBLISHED=NOT_VERIFIED`, don't claim closure.

Old design commits `bfb30d5`, `0f95c2e`, `18e5037` and status sync `bdc5cdf` remain **historically accepted docs-only**; their proposed code implementation is **not** grandfathered. Do not revert/rewrite Git history merely to erase a mistaken proposal.

## 4. Historical 2026-10-15 next-consumer criteria (Desktop consumer later delivered; future consumers require fresh scope)

**At the 2026-10-15 review the next action was concrete-consumer selection, not an unconsumed Core wrapper.** The small read-only Avalonia Desktop Inspector was later independently scoped, implemented and accepted for the no-committed-record fallback UI slice, as recorded above and in its own plan/acceptance pair. The following requirements still apply to a *different* future consumer such as a TUI or security-reviewed local Web/API host; neither is activated by this historical review. A future promotion must:

1. State missing user workflow, named presentation/host consumer, expected observable result, exact source paths and why existing `DeusDeviceSession` + `ManagementServiceOperations` are insufficient alone.
2. Require new executable feature tests *actually run* on the exact candidate; retain old Core≥84 and Transport≥24 baselines, MTP locked restore, zero failed/skipped, clean release. Any new tests increase actual counts, not merely documentation.
3. Address discovery/connect race, shared USB session ownership, unplug/reconnect, error/cancel/disposal, notification ordering and device-locator limitations; do not assert a hard native shutdown bound or authenticated board identity without new lower-layer evidence.
4. Before local HTTP/browser exposure: freeze process and startup model, caller identity, loopback binding, Origin/Host/CSRF/DNS rebinding, route-level 5-readonly allowlist, disallow raw RPC/control/updater operations, rate/size/time limits and negative security tests. For real device/USB integration state exactly which hardware checks are required.
5. Freeze only source changes needed by that consumer. Extract a shared runtime **only on demonstrated duplicate nontrivial orchestration in at least two actual consumers**. No stand-alone EXE/IPC/lockfile workarounds for unused glue.

**No existing source product, firmware or hardware acceptance is revoked by this documentation review.** For host-only changes STM32 resource delta is zero by exact changed-path proof, not a fresh firmware build. The previously measured 1,276-byte Flash / 296-byte accepted-static-SRAM margins are historical, not free runtime stack measurements.

## 5. Explicit no-go conditions

Do not create speculative `HostManagementRuntime.cs` / `HostManagementRuntimeTests.cs` just to obtain additional CI tests, hide three Core Control methods as if a wrapper enforced security, open HTTP/socket without a separate trust boundary, add an untested second reconnect state machine, implement unsupported disposal timeouts, auto-open an unselected device, assume USB locator authenticates hardware, modify MCU/bootloader/key/signer/private archive, or describe the current Host CI as a new feature test.

**Historical decision at architecture review C (not current Desktop product status):** `DESIGN_DUPLICATION_IDENTIFIED`; `SPECULATIVE_WRAPPER=NO_GO`; `PRODUCT_FEATURE=NOT_IMPLEMENTED` *for this proposed wrapper*; `NEXT=CONCRETE_CONSUMER_BOUNDARY` *at that time*. The separately scoped Desktop consumer was subsequently delivered and accepted for its read-only fallback Gate 3b. The exact published CI SHA must be recorded from live GitHub, not invented in advance.
