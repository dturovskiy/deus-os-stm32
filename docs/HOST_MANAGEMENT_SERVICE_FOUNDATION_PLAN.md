# Deus OS — Host Management Service Foundation Plan

**Current decision (architecture re-review C, 2026-10-15): `NO_GO_UNCONSUMED_RUNTIME_WRAPPER`.** Do **not** implement `HostManagementRuntime` merely to satisfy the former Gate-1 two-file plan. This is a reviewed **planning decision**, not a tested or released new product feature. This architecture-C no-go decision **was actually accepted/published** in `87639102c938146c4f68d4cb2b52df18ab591374` with exact GitHub Actions `38744214870` SUCCESS. The later concrete Desktop inspector is independently implemented/published (`4729341`, exact CI `38748957900`); neither event authorizes the rejected wrapper or a Web service.

Boundary: `HOST_MANAGEMENT_SERVICE_FOUNDATION`; global current state: `docs/CURRENT_STATE.md`; evidence and gates: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`. The selected **product goal**—usable local Host management with a future browser presentation—remains valid. There is **no active Stage-A implementation authorization** until a *concrete consumer* demonstrates missing behavior, with its own implementation boundary.

## 1. Verified present-state architecture

Read against public `main` `bdc5cdfe0faca0bc06f515517d7c7c230474cb83`:

- `host/src/DeusOs.Control.Core/DeusDeviceSession.cs:16-175,205-242`: existing session lifecycle, serial execution gate, explicit connect/disconnect, transport-disconnect recovery, negotiation, state notifications and disposal. Session is the **sole** operation/reconnect/state owner; do not introduce a second state machine or locking/retry layer.
- `host/src/DeusOs.Control.Core/ManagementServiceOperations.cs:102-173`: existing facade implements all five typed ReadOnly operations—`Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout`—as well as three separately typed Control operations. Asset methods enforce `AssetAccessPolicy.PublishedOnly`. Service-v1 catalog remains exactly **5 ReadOnly / 3 Control / 0 Destructive**.
- `host/src/DeusOs.Control.Desktop/DesktopController.cs:7-77`: **already** owns injected-platform discovery and a `DeusDeviceSession`, proxies state notifications, discovery, connect, several typed reads, two application controls, disconnect and dispose. It uses `_session.ExecuteAsync` rather than `ManagementServiceOperations`; consolidating call sites is an **optional future consumer-owned refactor**, not a mandatory new Core wrapper.
- `host/src/DeusOs.Control.Cli/Program.cs:9-44,64-111`: existing CLI creates platform-specific discovery, chooses a visible candidate (or requires locator on ambiguity), owns a `DeusDeviceSession` and executes typed commands. This is a **process-specific composition root** with its own richer CLI contract, not a duplicate central protocol engine.
- `IDeviceDiscovery` and `DeviceCandidate` already exist in `TransportAbstractions.cs`; .NET solution/locked packages and 84 Core/24 Transport baseline CI are accepted. No `DeusOs.Control.Service` or `HostManagementRuntime` has been implemented.

### Clean Architecture / SOLID adjudication

- **SRP:** current session handles connection/protocol lifecycle; facade handles typed service operations; presentation layers handle device selection/UI. New one-class discovery + selection + session lifecycle + typed facade **would aggregate rather than separate responsibilities** until a real consumer justifies it.
- **DIP:** Core already depends on `IDeviceDiscovery`, not WinUSB/libusb; preserve this direction. Physical transports belong to their platform projects; Desktop/CLI compose them.
- **ISP:** typed read-only route exposure must be restricted at the *actual consumer boundary*. Merely wrapping a publicly available Control-capable Core facade is **not authorization**. Preserve the existing broader trusted-Core API.
- **OCP/LSP:** no new inheritance or extensibility framework is justified; avoid interfaces/factories solely to anticipate hypothetical Web clients.
- **DRY/YAGNI:** new runtime **fails the current incremental-value test** because no real CLI/Desktop/Web consumer is scheduled to use it in the frozen Stage A and equivalent composition exists. Do not build unconsumed abstraction, second protocol client, second reconnect loop, service process or package.

## 2. Active decision: consumer-first, minimal implementation

**Withdraw the *future* nine-path standalone EXE/lockfile implementation and the later two-file in-process `HostManagementRuntime` implementation.** Their previously published design proposals and CI evidence are **historical facts**, not executable code or active authorization. They remain auditable in Git history; no history rewrite or source removal.

The most direct eligible **next tangible user feature** is a local Host read-only management view/API consumed by a real frontend, using the existing Core session and facade. At its own **new, separately selected** feature gate, first settle:

1. Exactly which consumer exists (Desktop extension, local Web UI/host, or another actual client), which user workflows are missing and whether direct Core composition is sufficient. Favor the **existing Desktop presentation** for local display-only changes and no additional process/security surface; do not create a framework before a use case.
2. If the owner proceeds specifically with a browser/HTTP host: process model and single device/session owner; server-side route-level allowlist for five readonly operations; local authentication and caller isolation; CSRF/Origin/Host/DNS rebinding defenses; actual loopback binding and rejection of LAN exposure; request sizes, concurrency, disconnect, cancellation, shutdown, structured error handling, logging/data protection and negative tests. **Even `localhost` is not authentication.** HTTP/Web is **not authorized** by this decision.
3. Define acceptance for *that consumer*: user-observable behavior, fake/disconnected/multiple-device cases, actual current Core + Transport tests, new feature-specific tests and real physical connection only if the selected acceptance requires it. A docs-only baseline CI **does not** prove the as-yet-unwritten feature.
4. Extract a shared Host runtime **only if ≥2 real consumers need the same nontrivial orchestration and an evidenced consumer-level test proves direct composition duplicates logic**. At that point freeze its exact responsibilities, concurrency rules and changed-path budget before writing it. Otherwise prefer direct `DeusDeviceSession` + `ManagementServiceOperations`.

**Gate-1 NO-GO does not make a finished product or close the broader Host Management Service product goal.** It closes only the abandoned speculative *runtime wrapper* candidate. The next concrete feature implementation is **NOT STARTED**; explicit scoping is required. No new dummy Gate, empty library, two-file proof-only runtime or unexplained lockfile churn.

## 3. Lifecycle, safety and concurrency caveats

- `DeusDeviceSession.ConnectAsync` serializes actual session connects; any **consumer-side Discover → Select → Connect** sequence can race with another selection. A real long-lived consumer must choose a policy (single-flight selection, reject-busy, or explicit last-wins), test concurrent selection and prevent silent wrong-board switching; do not add a second session owner to simulate seriality.
- `DisposeAsync()` waits on `_operationGate.WaitAsync()` without a timeout and may await native close; it provides **no unconditional wall-clock shutdown bound** for hung I/O. Existing cancellation is cooperative where honored. Never claim hard deadlines via abandoned `Task.Run` or `Task.WhenAny`; if required by a future daemon, scope a genuine lower-layer teardown change and prove it.
- Recovery uses USB **locator equality**, not a stable authenticated MCU identity; same-locator replacement may be a different board. Do not imply device attestation or broad remote/control permission.
- `StateChanged` uses queued callbacks; it is not a synchronous completion/notification-drain contract. Do not cache independent stale Ready state or invoke external callbacks while holding new management locks.
- The trusted Core façade contains three Control operations. A future Web/read-only application must enforce **explicit server route authorization** and not export the Core object or raw `ExecuteAsync` to untrusted callers.

## 4. History, resources, change authorization and evidence

The original six-doc Gate-0 selection `bfb30d5f23389b54f0c114959640dc2d5a6291f9` (CI `38732869436`), lockfile scope Amendment A `0f95c2e5036652251976a0f7d6d5f62ea419116d` (CI `38733800122`), previous in-process Correction B `18e5037a9a3adf04f1bcb7cd88785ea4365e2b6c` (CI `38739187311`) and status sync `bdc5cdfe0faca0bc06f515517d7c7c230474cb83` (CI `38740180025`) are **genuine successful docs-only publications**. The designs were refined; those CI results did **not** build a new service.

No new MCU code/bootloader, host executable/core source, project/lockfile, CI workflow, key, signer, Flash or device operation is authorized by this doc-only review. ARM physical split remains 8-KiB bootloader / 52-KiB application / 2-KiB metadata / 2-KiB config; historical measured application 51,972/53,248 B and accepted static RAM 10,968/11,264 B are **not** current-HEAD ARM measurements or live task-stack headroom. Before target-resident features, demand fresh ELF/BIN/MAP/.su and stack accounting.

To accept **this documentation correction**, review exact allowed docs paths, `git diff --cached --check`, normal non-force commit/push, fresh fetch clean `HEAD == origin/main`, and actual exact-SHA GitHub Host CI (Core ≥84; Transport ≥24; failed/skipped=0; clean Release; output hygiene PASS). **The next code feature needs its own implementation tests, not another design-only CI.** Do not reopen historical key/custody operations.
