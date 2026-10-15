# Deus OS — Desktop Published Asset Inspector Plan

**Boundary:** `DESKTOP_PUBLISHED_ASSET_INSPECTOR` — Gate 0 design PUBLISHED `457be3dc4ff2e642138bd62178d91420ee552570` (CI `38746868482` SUCCESS); Gate 1/2 implementation PUBLISHED `4729341e732ffd61e7fe2b0e48973f2c98726a5d` (CI `38748957900` SUCCESS, Core 87/87, Transport 24/24, Desktop Release zero warnings/errors). **Physical Desktop/UI and on-device behavior NOT VERIFIED**; this is source/CI acceptance only, not full end-to-end acceptance. Global state authority: `docs/CURRENT_STATE.md`; acceptance: `docs/DESKTOP_PUBLISHED_ASSET_INSPECTOR_ACCEPTANCE_PLAN.md`.

## Need and reuse

Current Avalonia Desktop shows system identity, health and applications but **does not show published asset status or OLED console layout**. CLI already has `config-status` / `config-read`; Core `ManagementServiceOperations.AssetStatusAsync` and `ReadOledUiLayoutAsync` already enforce `AssetAccessPolicy.PublishedOnly`. This is a concrete UI consumer and does **not** justify a new `HostManagementRuntime`, service executable, duplicate USB client, transport, parser or RPC contract.

**Historical Gate 0** froze scope before implementation; actual docs-only commit `457be3d` passed its own exact-SHA CI. The later three-file Desktop code commit `4729341` passed a separate source build/test CI. Physical on-device Desktop acceptance remains NOT_VERIFIED. Freeze exactly six documentation paths: this plan, paired acceptance, `CURRENT_STATE.md`, `ROADMAP.md`, `MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`. Publish non-force with exact-SHA Host CI PASS before changing source.

## Frozen implementation — Gate 1 source accepted/published

Only **three** existing tracked source/test files may change:

1. `host/src/DeusOs.Control.Desktop/DesktopController.cs`: compose existing `ManagementServiceOperations` over the existing private `DeusDeviceSession`, exposing only `AssetStatusAsync` and `ReadOledUiLayoutAsync` for the new UI. No new ownership, discovery or protocol logic.
2. `host/src/DeusOs.Control.Desktop/MainWindow.cs`: one explicitly triggered **Refresh published asset** button and read-only status/layout text. Display asset committed generation, payload size and CRC32; when a committed layout exists display validated console rectangle through `OledUiLayoutConfigV1.Deserialize`; otherwise clearly show compiled-default fallback. Never write asset/config or change existing application controls.
3. `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`: new **actually executed** synthetic Core test cases for the published asset status/read facade, including no-record fallback and/or capability rejection. Existing 84 Core and 24 Transport tests remain; test-count increase is verified from actual run, not assumed.

No `.csproj`, `packages.lock.json`, `host/DeusOs.Control.sln`, CI workflow, Core implementation, bootloader, target firmware, network listener or private material changes.

## Interaction, safety and lifecycle

- A separate Refresh button is disabled until `ConnectionState.Ready` and `SystemCapability.AssetConfigurationTransfer`; no asset polling or automatic requests on startup/connection. No transport I/O from constructor. Read runs only on explicit user action.
- Use existing session serialization; do **not** add a second lock or independent reconnect loop. Avoid stale result display after session changes by checking `ConnectionState.Ready` and the original `NegotiationResult` identity upon completion. Clear asset text on fault, disconnect and recovery; refresh only on explicit click after recovery.
- A device with no asset-transfer capability must present `Unavailable` and issue **zero** asset reads. If there is no committed record, show fallback rather than fabricating persisted data. Malformed payload, CRC, timeout and disconnected status must surface existing Core typed failures; do not hide them or retry writes.
- Do not expose `WriteOledUiLayoutAsync`, `PrepublicationAcceptance`, raw RPC selector or destructive firmware actions. Existing accepted Core operation catalog **5 ReadOnly / 3 Control / 0 Destructive** remains unchanged. One UI button does not confer a new security boundary.
- USB locator is not authenticated board identity; `DisposeAsync` has no guaranteed hard deadline when native transport hangs. Do not claim either is solved.
- Accessibility/observability: text values legible in ScrollViewer, button labels explicit, disabled state visible; no new network endpoints.

## Sequence and acceptance evidence (historical gates; physical UI remains open)

**Gate 0:** approved docs-only scope, staged diff/whitespace, direct-parent non-force push, fresh fetch clean 0/0 and exact-SHA hosted Core >=84, Transport >=24, zero failed/skipped, Release zero warnings/errors.

**Gate 1/2:** exactly the three frozen source/test paths, strict local code review and real compiler/test evidence for new synthetic cases. A previous `dotnet_project` task returned `CONTROL_PLANE_OUTCOME_AMBIGUOUS`; it is not an accepted compiler result. If local execution cannot provide proof, accept only a genuine candidate-specific hosted build/test and keep human UI/hardware acceptance separate. No automatic declaration of UI physically verified.

**Gate 3:** ordinary non-force feature publication only after actual new tests, candidate-diff review and exact-SHA hosted CI; reconcile status/checklists with truth. A real connected Desktop usability/USB test requires operator evidence only if we decide to claim physical UI acceptance. Do not modify firmware or ask for secret artifacts.

This Host-only change does not consume STM32 Flash/SRAM. Historical 51,972/53,248 Flash and 10,968/11,264 accepted static-SRAM measurements are not fresh target-build results. Next MCU-resident feature needs its own exact ARM resource baseline.
