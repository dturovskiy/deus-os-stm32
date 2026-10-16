# Deus OS — Desktop Published Asset Inspector Plan

**Accepted follow-up (2026-10-16):** the published implementation's real **read-only compiled-default fallback UI Gate 3b** was accepted through the owner's temporary Windows native-USB connection, actual WinUSB + CLI read results and an operator-attested Avalonia `READY` / manual asset Refresh view. `PHYSICAL_DESKTOP_UI=OPERATOR_ATTESTED_PASS` for this fallback slice; evidence ZIP CRC and 32/32 file hashes independently checked (full artifact ID retained privately). A positive physical read of previously **committed** OLED layout stays `NOT_VERIFIED`. This is neither a write acceptance nor a permanent topology change. Earlier `NOT_VERIFIED` text below is the historical source-publication checkpoint, not the current Gate 3b disposition.

**Boundary:** `DESKTOP_PUBLISHED_ASSET_INSPECTOR` — Gate 0 design PUBLISHED `457be3dc4ff2e642138bd62178d91420ee552570` (CI `38746868482` SUCCESS); Gate 1/2 implementation PUBLISHED `4729341e732ffd61e7fe2b0e48973f2c98726a5d` (CI `38748957900` SUCCESS, Core 87/87, Transport 24/24, Desktop Release zero warnings/errors). **At initial source/CI publication**, physical Desktop/UI behavior was NOT VERIFIED; the later 2026-10-16 read-only fallback UI Gate 3b was separately operator-attested PASS as described above. Global state authority: `docs/CURRENT_STATE.md`; acceptance: `docs/DESKTOP_PUBLISHED_ASSET_INSPECTOR_ACCEPTANCE_PLAN.md`.

## Need and reuse

**Pre-implementation baseline:** Avalonia Desktop showed system identity, health and applications but did not show published asset status or OLED layout. The new inspector was subsequently implemented and hosted-CI accepted at `4729341e732ffd61e7fe2b0e48973f2c98726a5d` (87 Core / 24 Transport PASS). Connected UI acceptance was still unverified at the earlier source-publication checkpoint; the fallback UI path subsequently passed on 2026-10-16. CLI already has `config-status` / `config-read`; Core `ManagementServiceOperations.AssetStatusAsync` and `ReadOledUiLayoutAsync` already enforce `AssetAccessPolicy.PublishedOnly`. This is a concrete UI consumer and does **not** justify a new `HostManagementRuntime`, service executable, duplicate USB client, transport, parser or RPC contract.

**Historical Gate 0** froze scope before implementation; actual docs-only commit `457be3d` passed its own exact-SHA CI. The later three-file Desktop code commit `4729341` passed a separate source build/test CI. Physical on-device Desktop acceptance was NOT_VERIFIED at the earlier source-publication checkpoint; the fallback path subsequently passed by operator attestation. Freeze exactly six documentation paths: this plan, paired acceptance, `CURRENT_STATE.md`, `ROADMAP.md`, `MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`. Publish non-force with exact-SHA Host CI PASS before changing source.

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

## Sequence and acceptance evidence (historical pre-2026-10-16 gates; physical fallback UI now operator-attested PASS)

**Gate 3b: mandatory physical-interface ownership (2026-10-15 correction).** Windows runs the repo/ST-LINK/UART; STM32 native USB/VBUS and the libusb IF2 management interface belong to the Ubuntu Mac mini in the canonical bench. The Windows screenshot shows the new panel but `No management device found` there is an expected host mismatch, **not evidence of a broken firmware, USB descriptor or WinUSB driver**. First validate the accepted Windows→Ubuntu SSH execution primitive; on Ubuntu use bounded read-only native USB/libusb evidence (optionally the existing `deus-cp devices`, `config status`, `config read`, PublishedOnly). **That cannot substitute for Avalonia visual acceptance.** Earlier planning asked to preflight whether Ubuntu actually has a graphical desktop; this is now resolved by the owner's Ubuntu Server description, so classify `ENVIRONMENT_BLOCKED_HEADLESS_GUI` and keep `PHYSICAL_DESKTOP_UI=NOT_VERIFIED`. No forced Windows driver changes, cabling/power cycles, firmware flash, arbitrary USB bridge or unauthorized Web endpoint. No Linux USB/CLI/GUI measurement was executed as part of this plan correction. See `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`, `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` and paired acceptance.

**Gate 0:** approved docs-only scope, staged diff/whitespace, direct-parent non-force push, fresh fetch clean 0/0 and exact-SHA hosted Core >=84, Transport >=24, zero failed/skipped, Release zero warnings/errors.

**Gate 1/2:** exactly the three frozen source/test paths, strict local code review and real compiler/test evidence for new synthetic cases. A previous `dotnet_project` task returned `CONTROL_PLANE_OUTCOME_AMBIGUOUS`; it is not an accepted compiler result. If local execution cannot provide proof, accept only a genuine candidate-specific hosted build/test and keep human UI/hardware acceptance separate. No automatic declaration of UI physically verified.

**Gate 3:** ordinary non-force feature publication only after actual new tests, candidate-diff review and exact-SHA hosted CI; reconcile status/checklists with truth. A real connected Desktop usability/USB test requires operator evidence only if we decide to claim physical UI acceptance. Do not modify firmware or ask for secret artifacts.

This Host-only change does not consume STM32 Flash/SRAM. Historical 51,972/53,248 Flash and 10,968/11,264 accepted static-SRAM measurements are not fresh target-build results. Next MCU-resident feature needs its own exact ARM resource baseline.
