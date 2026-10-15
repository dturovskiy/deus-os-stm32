# Deus OS — Desktop Published Asset Inspector Acceptance Plan

**Candidate Gate 0, 2026-10-15.** New product boundary `DESKTOP_PUBLISHED_ASSET_INSPECTOR`. Parent plan: `docs/DESKTOP_PUBLISHED_ASSET_INSPECTOR_PLAN.md`. Canonical current status: `docs/CURRENT_STATE.md`. It does not reopen or implement rejected `HostManagementRuntime` or a Web/HTTP service.

## Gate-0 contract

- Confirm fetched public/private Git clean 0/0 and exact public baseline `87639102c938146c4f68d4cb2b52df18ab591374` (public Host CI `38744214870` SUCCESS; Core 84/84, Transport 24/24, Release 0 warnings/errors).
- **Exactly six** docs-only paths: plan, this acceptance, `CURRENT_STATE.md`, `ROADMAP.md`, `MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`. No compiler/target actions, secrets, source or CI mutation.
- Accepted concrete user value: Desktop users can manually inspect published OLED asset status and validated layout, using already accepted Core APIs; no new interprocess, transport, RPC or frontend framework.
- Explicit no-write, capability, no-record, stale-connection, error and no-background-poll contract frozen in plan.
- Non-vacuous staged whitespace check, direct parent, ordinary non-force Git publication, fresh `HEAD==origin/main` clean 0/0 and hosted CI **on exact new Gate-0 SHA** with Core >=84, Transport >=24, zero failed/skipped, Release 0 warnings/errors and tracked-output hygiene.
- If Gate-0 publication/CI fails, **no source changes** authorized. A green CI of the prior SHA cannot close this Gate.

## Future Gate-1/2 real code / test acceptance

Freeze exactly `host/src/DeusOs.Control.Desktop/DesktopController.cs`, `host/src/DeusOs.Control.Desktop/MainWindow.cs`, `host/tests/DeusOs.Control.Core.Tests/ManagementServiceOperationTests.cs`. All additions must be necessary; any discovered extra project, script, lockfile, Core or transport dependency requires fresh scoped approval before editing.

- Actual Desktop UI has asset status and validated OLED layout in read-only text, one manual refresh button. Button disabled until Ready + `AssetConfigurationTransfer`; no auto-read on startup. Errors are observable, no fake Connected state. No-record reports compiled-default fallback. Unsupported capability causes no asset RPC.
- Status/read call via existing `ManagementServiceOperations` with `PublishedOnly`; no raw protocol, `PrepublicationAcceptance`, `WriteOledUiLayoutAsync` or destructive access. No duplicate `DeusDeviceSession` lifecycle or locks. No stale content on fault/disconnect/recovery or session change.
- At least two *new* synthetic Core tests that **actually execute** for published-status/read and fallback/negative paths; pre-existing 84 Core and 24 Transport test baselines preserved, 0 failed/skipped. Desktop must compile in Release, warnings/errors 0. Hosted exact candidate SHA CI proves compile/Host tests, **not physical Desktop UI/USB**. If managed local .NET tool returns ambiguous, log `INFRASTRUCTURE_INDETERMINATE`, not compiler FAIL/PASS.
- Accepted source Git diff and standard non-force push, fetched clean 0/0, exact-SHA hosted Core/Transport CI new counts and result, plus an honest `PHYSICAL_DESKTOP_UI=NOT_VERIFIED` unless the owner supplies actual physical observation. Do not claim full physical acceptance from CI.

## Stop criteria

Unexpected source WIP/extra paths, unbounded polling, auto-read without user, new Control/Write route, missing capability guard, fake on-device measurement, swallowed Core error, UI stale-state leak, no executable new tests, unreviewed NuGet/solution change, Git parent/CI mismatch. No Flash, MCU, signing/key, bootloader, SWD or history rewrite.
