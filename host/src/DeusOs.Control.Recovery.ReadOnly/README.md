# Gate 3 — opt-in recovery read-only Host capability (candidate)

**Status:** integrated into the Host solution as a source-level read-only library and a bounded offline CLI contract inspection. Not a production recovery operator, not a hardware admission or a Gate 3 PASS. The CLI/Desktop have NO wired live `000D` execution path.

This independent project references the **unchanged** production `DeusOs.Control.Core`. The three C# source bodies are byte-for-byte copies of the previously accepted, separately qualified OFFLINE Gate3 shadow sources under `OS/build/gate3_000d_source_qualification/`:

- `ReadOnlyRecoveryInfoProbe.cs`: SHA-256 `528B5DD97BC35051838A842DE4A4FFFC2E86D2F2F7D38BF5DCAE59CC2D9F2E50`
- `RecoveryRouteReadiness.cs`: SHA-256 `26B944FC9CA57D8B9F64F593C325583F124B0AAE24B731B7BC48B87881A70B10`
- `RecoveryRouteCoordinator.cs`: SHA-256 `3F4E5D326AB150B80858F9D85194EA317C340C63CF0BE21369159BA2796681AA`

The acceptance console preserves the original policy and integrated coordinator tests and must execute with no live USB. It is **not** wired to the live bootloader path, Desktop, or firmware-update operations. It does not contain `EnterBootloaderAsync`, `UpdateAsync`, signing or Flash calls. One read-only `INFO` can be issued only when a future authorized application explicitly supplies discovery and handoff claims; a matching result **never** authenticates physical identity and **never** grants destructive access.

For isolated .NET 10 qualification, from `OS/host`:

```powershell
dotnet run --project tests/DeusOs.Control.Recovery.Acceptance/DeusOs.Control.Recovery.Acceptance.csproj --configuration Release
```

The new `deus-cp recovery contract` CLI command prints the **expected** v6/v7 package identities and source-tree pin only; it does **not** open or hash either protected `.pkg`, prove their HMAC, create USB discovery, accept device claims, open `INFO`, or perform firmware operations. It rejects `--device` and additional subcommands. This CLI entry and both recovery projects are included in `host/DeusOs.Control.sln`; this is build-surface adoption, not a complete recovery workflow.

From `OS/host`:

```powershell
dotnet run --project src/DeusOs.Control.Cli/DeusOs.Control.Cli.csproj --configuration Release -- recovery contract
```

The tests use in-memory device transports only. They check valid-app and no-valid-app paths, source and package pins, USB owner/session/transport correlation, errors, anti-rollback floors, and the single-info/no-flash invariant. Historical acceptance of earlier byte-identical shadow tests **does not** substitute for a fresh Windows-native run of this integrated project.

**2026-10-28 — Native Windows source-candidate qualification (owner terminal).** `dotnet run --project tests/DeusOs.Control.Recovery.Acceptance/DeusOs.Control.Recovery.Acceptance.csproj --configuration Release` completed with required process exit zero and `GATE3_READONLY_HOST_NATIVE_TEST=PASS`; policy **34/34 PASS** and coordinator **23/23 PASS**, **57/57** together. These are in-memory, read-only source acceptance results, not live `000D`/USB and not a formal original indexed evidence ZIP. This clears the prior WSL build limitation for the candidate but does NOT publish or adopt it in production. No reset, Flash, signing, USB or owner hardware-release action occurred.

**2026-10-28 — Full Host solution and offline CLI Windows-native PASS (owner terminal).** `dotnet build host/DeusOs.Control.sln -c Release --warnaserror`: all nine projects succeeded, `HOST_SOLUTION_BUILD=PASS`. Built CLI `recovery contract` printed expected combined tree and protected v6/v7 sizes/SHA, and explicitly `LIVE_USB=NOT_QUERIED PHYSICAL_IDENTITY_AUTHENTICATED=NO`, `DESTRUCTIVE_OPERATION_AUTHORIZED=NO`, `GATE3_AUTO_ACCEPTANCE=NO`; `--device usb:001:8 recovery contract` rejected with expected nonzero exit, `CLI_DEVICE_ARGUMENT_REJECT=PASS`, `HOST_SOLUTION_CLI_INTEGRATION=PASS`. This is console-supplied source/CLI test evidence (not original indexed evidence ZIP); `dotnet build` compiles existing Core/Transport tests but does **not** execute them. Post-integration regression execution is separate.

**2026-10-28 — Windows-native post-integration regressions PASS (owner terminal).** MTP Core **87/87 PASS**, Transport **24/24 PASS** (each 0 failed/0 skipped); read-only recovery policy **34/34 PASS** and coordinator **23/23 PASS**; total **168/168 PASS**, `HOST_POST_INTEGRATION_REGRESSION=PASS`. All recovery calls are in-memory/mock; no live `000D`, reset, signing or Flash. This verifies compiled code and exercised regression behavior on Windows but is **not** an original indexed evidence ZIP, independently attested physical USB identity or a usable recovery writer. No new ZIP should be retroactively manufactured to replace the actual terminal-only proof.

Formal Gate3 readiness still needs independently reviewed original evidence for future native operator qualification, a genuine end-to-end recovery operator with signed/native admission, physical-session boundary policy and documented STOP/rollback ownership. The two-source-project solution integration and offline CLI visibility do not, by themselves, provide the hardware recovery operation. `FULL_GATE3=NOT_ACCEPTED`, `GATE4=NOT_AUTHORIZED`. Do not remove any quarantine or run the physical reset helper as a way to qualify this candidate.
