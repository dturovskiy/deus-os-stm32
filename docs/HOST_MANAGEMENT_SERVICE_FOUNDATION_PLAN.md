# Deus OS — Host Management Service Foundation Plan

Status: **GATE-0 DESIGN AND LOCKFILE AMENDMENT A BOTH ACCEPTED/PUBLISHED.** Original design `bfb30d5f23389b54f0c114959640dc2d5a6291f9`, CI `38732869436` SUCCESS. Amendments `0f95c2e5036652251976a0f7d6d5f62ea419116d`, CI `38733800122` SUCCESS (84 Core / 24 Transport, zero failed/skipped, warning-free Release, hygiene 273). **Gate-1 source implementation is now permitted within the nine-path frozen scope, but is NOT YET IMPLEMENTED OR TESTED.**

Boundary ID: `HOST_MANAGEMENT_SERVICE_FOUNDATION`.

Global state owner: `docs/CURRENT_STATE.md`; gate/result owner: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.
Baseline Git: public `main` `13889b4f2c509c9b83a1945f9b899add5d0164d2` (2026-10-15 pre-Gate-0); exact baseline bytes/cleanliness must be freshly checked at execution, not inferred from this static statement.

## 1. Why this is the next product boundary

All FDC-01..10 and RDC-01..08 items and the forward-only repository sanitization programme are completed within their stated proof/owner-policy scopes. No newer product-feature boundary was previously active. `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` explicitly lists a local host-management service as an eligible **separately promoted** boundary. The owner chose that progression over firmware/network expansion.

The accepted substrate already consists of `DeusOs.Control.Core` and `DeusDeviceSession` (per-session transport, correlation, recovery and lifecycle); Windows WinUSB / Linux libusb discovery; CLI (first-class automation) and optional Avalonia desktop; typed Ping/Health/Application/Asset models; and one `ManagementServiceOperations` facade with frozen service-v1 catalog **5 ReadOnly, 3 Control, 0 Destructive**. Do **not** create a second protocol/client, request-ID allocator, transport lock, reconnect loop or duplicated registry.

This is **host-only**. The resource-constrained STM32F103C8 application Flash/SRAM, bootloader, linkers, USB descriptors, physical wiring, signed revision 5, key material and firmware update ABI are unchanged.

## 2. Two deliberately separate boundaries

**This foundation (Stage A)** creates a portable headless local *process owner* for a single selected Deus device/session. It provides explicit start/stop/discovery, bounded cancellation/shutdown, typed session-state and read-only diagnostic/status operations via the existing Core facade. Local machine CLI/process control is the sole interface. It must be useful without GUI and must never automatically open/claim an unselected device. Development-test fixtures are synthetic; no target/device access for Gate 0–2.

**Future separately promoted Stage B:** localhost-only HTTP/WebSocket API and browser frontend. **No socket/listener/HTTP route is authorized under this Stage A.** Before an HTTP listener is allowed, separately freeze local auth, CSRF/Origin/Host and DNS-rebinding protection, caller authorization, capability/allowlist enforcement, lifecycle/session ownership, request/response size/time limits, remote binding denial, and exact negative tests. `localhost` does **not** imply operator authentication. Web must consume this service/Core and must never parse STM32 frames itself. LAN/Wi-Fi/ESP/network remote management requires yet another security/trust gate.

## 3. Frozen Stage-A minimum contract

1. One durable host process; single selected session per service instance. Discovery/open/reconnect is delegated to one `DeusDeviceSession`; default is stopped/disconnected; no USB open on startup without an explicit selected candidate.
2. Service consumer obtains only **named typed read-only** operations: `Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout`; pass through the existing `ManagementServiceOperations` and its `PublishedOnly` policy where applicable.
3. Explicitly do **not** expose the already-existing three service-v1 `Control` operations at the Stage-A process boundary. They remain in the accepted Core facade for other trusted local applications; stage-A allowlist is a strict subset, not a change to the frozen service-v1 Core catalog.
4. **No** raw numeric RPC, command string, arbitrary flags/payloads, firmware-update/bootloader, destructive diagnostic, asset prepublication access, credential/key entry, flash writes, target reset, network listener, plugin/installer/auto-start, or privilege elevation.
5. Reuse existing typed Host Core states/capabilities and error kinds; never invent online/connected/peer status on elapsed time, USB VID/PID or a stale negotiation object.
6. Every action has bounded cancellation/timeouts; unplug/replug, stale responses, delayed terminal RPC, disposal and concurrent requests must preserve existing Core FDC-01/FDC-02/FDC-09 invariants. No subscriber callback while holding service-owned serial locks. Shutdown is idempotent and does not abandon a background task or silently recover a failed command.
7. No background polling unless an explicit bounded consumer justifies it; no raw frames, unbounded logs, secrets or operator-local paths in hosted evidence; explicitly bounded diagnostic summary and structured errors only.
8. Keep Windows/Linux and headless support. CLI `deus-cp` and Desktop remain unchanged; no new SDK/runtime distribution or OS-service registration requirement.

## 4. Gate sequence and source-change authority

- **Gate 0 — docs + owner scope**: create this plan and a separate acceptance plan; record `Gate-0 DESIGN CANDIDATE` in current state, roadmap, execution ledger and changelog; require an explicit design/security decision, docs-only staged paths, baseline SHA/CI, direct-parent non-force publication and successful GitHub CI on the published SHA before proceeding. Gate-0 CI is Host CI only, not firmware acceptance.
- **Gate 1 — narrow Stage-A implementation** (only after Gate-0 Amendment A is separately published and accepted): frozen expected path set comprises `host/src/DeusOs.Control.Service/DeusOs.Control.Service.csproj`, `host/src/DeusOs.Control.Service/Program.cs`, `host/src/DeusOs.Control.Service/HostManagementRuntime.cs`, `host/src/DeusOs.Control.Service/packages.lock.json`, `host/tests/DeusOs.Control.Service.Tests/DeusOs.Control.Service.Tests.csproj`, `host/tests/DeusOs.Control.Service.Tests/HostManagementRuntimeTests.cs`, `host/tests/DeusOs.Control.Service.Tests/packages.lock.json`, `host/DeusOs.Control.sln` and the minimum necessary `.github/workflows/ci.yml` test registration. These **nine paths** are all reviewed; adding the two lockfiles is justified by the repository-wide `RestorePackagesWithLockFile=true` and existing CI `--locked-mode` contract. Lockfile generation is only permitted for the reviewed test/Service project, never as an uncontrolled package upgrade. Any extra Core/Transport/CLI/Desktop files, packages, protocol changes, service/network listener or packaging require a further independently published scope amendment **before** code changes. If this set is infeasible, stop and amend rather than create a proxy duplicating Core.
- **Gate 2 — deterministic offline host acceptance**: synthetic discovery/session/transport fixtures, operation allowlist and error/cancel/disconnect/reconnect/shutdown tests plus exact build/test coverage. New Service tests must actually execute; existing Core >=84, Transport >=24, none failed/skipped, warnings/errors zero. Validate Windows/Linux project compilation where toolchains exist; no target I/O.
- **Gate 3 — integrated local host runtime**: exact candidate service process start/stop and bounded real selected-device workflow, Windows/Linux as relevant, only if separately justified by the gate scope. Real device interaction requires an explicit gated operator decision and cannot be inferred from synthetic tests. No physical-target flash/key/reset.
- **Gate 4 — local accepted implementation commit**, after eligible technical acceptance; actual diff/whitespace/path review, no unrelated product changes.
- **Gate 5 — ordinary non-force public publication**, fresh fetch, `HEAD == origin/main == FETCH_HEAD`, clean `0/0`, exact published GitHub CI Core/Transport/Service tests SUCCESS.
- **Gate 6 — final canonical-state and documentation reconciliation**, with separate publication proof. Only then `HOST_MANAGEMENT_SERVICE_FOUNDATION=CLOSED/PUBLISHED`; browser/network remains future.

Do not collapse Gate 0 and Gate 1 into one unproven commit; do not manufacture local Windows, Linux or physical evidence. A synthetic-only release may close its own stage only if this acceptance plan explicitly allows it; never claim real device parity from Host unit tests.

## 5. Resource budget / firmware identity rule

`FLASH_OWNERSHIP_LAYOUT_DECISION_V1` physically reserves 8 KiB bootloader, 52 KiB executable application, 2 KiB firmware metadata, 2 KiB config. `scripts/build_firmware.ps1` enforces application Flash <=53,248 B and **accepted static SRAM** <=11,264 B, which is not the physical 20-KiB SRAM total. Historical measured candidate Flash/SRAM 51,972/10,968 B leaves just 1,276 B Flash and 296 B within accepted static-SRAM ceilings; these are **historical candidate numbers**, not a fresh current-Git ARM build or a measurement of free live stack space.

For this host-only boundary, `STM32_FIRMWARE_CHANGED=NO`, `TARGET_MEMORY_DELTA=0` **by exact changed-path proof**; no new ARM build is necessary to claim that no target source was edited. Before the **next STM32-resident** feature or any acceptance of a new firmware profile, obtain a fresh exact-tree ARM ELF/BIN/MAP + `.su` baseline and preserve existing per-task/MSP limits. Do not weaken build ceilings merely to make a feature fit. Heavy scheduler diagnostics can only be excluded behind an independently evidenced diagnostic-vs-production profile, with fresh production-image hardware acceptance.

## 5.1 Gate-0 original acceptance and lockfile Amendment A (2026-10-15)

The original six-path documentation-only Gate 0 was published as commit `bfb30d5f23389b54f0c114959640dc2d5a6291f9` from direct parent `13889b4f2c509c9b83a1945f9b899add5d0164d2`, verified after fetch clean `0/0`. Exact-SHA [hosted CI `38732869436`](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38732869436) **SUCCESS**: Core `84/84`, Transport `24/24`, zero failed/skipped, Release 0 warnings/errors and hygiene 273/273.

A narrow static Gate-1 preflight identified that adding new .NET test and executable projects to the existing locked-restore solution necessarily creates **two new tracked lockfiles**. The original seven-path Gate-1 scope did not include them, so it is **not** permissible to create them in an unreviewed Gate-1 patch. Amendment A adds the two `packages.lock.json` paths to the frozen future source scope (nine total) and no other feature behavior. Amendment A was published as direct-child docs-only commit `0f95c2e5036652251976a0f7d6d5f62ea419116d`; fresh-fetched clean `0/0`; exact-SHA [hosted CI 38733800122](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38733800122) **SUCCESS** (Core 84/84, Transport 24/24, 0 failed/skipped, Release 0 warnings/errors, hygiene 273). The locked-restore/zero-warning rules remain mandatory and Gate-1 code remains **NOT STARTED**.

## 6. Mandatory stop / rollback conditions

Stop immediately on unknown active WIP, non-direct-parent publication, dirty repository, missing new service tests, downgrade of existing 5/3/0 Core policy, any raw/destructive/updater service route, unintended listening socket, missing shutdown cancellation, unbounded polling/allocations, unauthorized firmware/bootloader/source changes, CI test-count drift, missing exact-SHA hosted CI, or fabricated device/resource claims. Restore only owned new files on local failure and preserve unrelated state. No force push, history rewrite or secret/archive movement.
