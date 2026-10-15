# Deus OS — Host Management Service Foundation Acceptance Plan

Status: **CORRECTION B — ARCHITECTURE/SCOPE ACCEPTANCE PENDING DIRECT-PARENT NON-FORCE PUBLICATION + EXACT-SHA CI.** Gate-1 implementation is **NOT STARTED**. The original six-doc Gate-0 design `bfb30d5f23389b54f0c114959640dc2d5a6291f9` / CI `38732869436`, Amendment A `0f95c2e5036652251976a0f7d6d5f62ea419116d` / CI `38733800122` and documentation-state commit `a3e2237b2c92378050f80413506a209dedb28951` / CI `38734412365` are genuine previously published PASS events, but none validates the revised Stage-A architecture or a built Service.

Canonical scope: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`. Global status: `docs/CURRENT_STATE.md`.

## 0. Architecture re-review: explicit findings and dispositions

| ID | Independently checked fact / mistake | Correction and acceptance requirement |
|---|---|---|
| AR-01 | Proposed long-lived process had no defined interprocess consumer/control interface while all sockets/HTTP/IPC were excluded. | **Drop the process.** Stage A is a passive in-process Core runtime, no EXE/daemon/IPC/listener. A separate Web host adapter is a future boundary. |
| AR-02 | Runtime implementation inside `DeusOs.Control.Service` EXE would require reverse/circular dependencies or migration for later consumers. | Runtime lives in already reusable `DeusOs.Control.Core`; CLI/Desktop/later Web can reference the same assembly, no extra library assembly or project. |
| AR-03 | Draft promised universal bounded shutdown despite existing `DeusDeviceSession.DisposeAsync()` waiting on `_operationGate.WaitAsync()` without timeout and then native disposal. | Prove **cooperative** operation cancellation and idempotent responsive disposal only. A stuck native call can delay close indefinitely. **Never fake a bounded guarantee via abandoned task.** Future hard shutdown needs separately accepted Core/transport design. |
| AR-04 | USB reconnect matches previous locator; topology is not verified board identity. | Record **locator continuity only**, not immutable or cryptographically authenticated identity; test device-missing/changed-locator behavior, do not extrapolate to remote/control permissions. |
| AR-05 | Nine-path Gate-1 scope, two new NuGet locks and CI registration were frozen before deciding whether any new project was warranted. | **Replace with exactly two new existing-Core source/test files**, no project/lockfile/solution/workflow changes. Historical Amendment A stays in Git but is superseded for future implementation. |
| AR-06 | A read-only wrapper alone could be incorrectly described as a security barrier even though public `ManagementServiceOperations` / `DeusDeviceSession` still support other authorized local consumers. | Runtime offers only the five read-only typed methods; **API narrowing, not cross-process authorization**. Future actual security boundaries require separate review. |
| AR-07 | Docs carried contradictory old 'before Gate-0 publication' and 'next Gate-0 pending' language after actually published Gate 0/Amendment A. | Correct current-state/roadmap/master and preserve accurate dated history; new Correction-B status must not say PASS before its own exact-SHA CI. |
| AR-08 | Managed DEUS MCP local `dotnet_project` returned `CONTROL_PLANE_OUTCOME_AMBIGUOUS`, no compiler logs, while green CI was only for docs/baseline code. | **Infrastructure inconclusive**, not C# compiler failure. Require actual independent build + **new** test execution for Gate-1/2; old CI can certify this docs-only change but not an uncommitted runtime. |

## 1. Architecture Correction B gate (documentation-only)

PASS requires:

1. Freshly checked public/private Git HEAD and origin, clean baseline `a3e2237b2c92378050f80413506a209dedb28951`, upstream 0/0, current hosted CI `38734412365` SUCCESS (existing Core **84/84**, Transport **24/24**, Release 0 warnings/errors, 273 tracked-file hygiene PASS).
2. The exact **six** allowed Gate-0 correction paths: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md`, `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`, `docs/CURRENT_STATE.md`, `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`. No Core/Host/firmware/bootloader/scripts/CI/secret/hardware edits.
3. Current plan clearly supersedes the future nine-path EXE design with exactly **two** future Core files and explicitly documents the absence of sockets, HTTP, IPC, process, daemon, background worker, transport rewrite or lower-layer teardown modification.
4. Architecture invariants: injected `IDeviceDiscovery`; explicit locator selection; single `DeusDeviceSession` operation/state owner; one `ManagementServiceOperations` façade; only `Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout`; asset `PublishedOnly`; no raw/control/destructive/updater routes in the runtime API.
5. Limitation statements are precise and testable: serial Core session, no machine-wide exclusive ownership, no synchronous `StateChanged` delivery guarantee, no finite native-dispose deadline, no cryptographic identity proof, no Web/auth claim. Hosted Host CI cannot prove MCU Flash/SRAM or physical USB.
6. Canonical `CURRENT_STATE`, `ROADMAP`, checklist and changelog agree the original Gate-0 + Amendment A were truly published but are **superseded architecturally** by this correction; Gate 1 is BLOCKED until correction's actual publication/CI. No fictitious rollback/delete of previous Git commits.
7. Exact staged paths, nonempty diff, `git diff --cached --check`=0, reviewed direct parent, ordinary **non-force** publication and fetched `HEAD == origin/main == FETCH_HEAD`, clean 0/0. Exact published correction SHA's GitHub CI **SUCCESS** with actual Core >=84, Transport >=24, failed/skipped 0, Release 0 warnings/errors and tracked-hygiene PASS. An older CI run does not satisfy this predicate.

If any predicate fails, `CORRECTION_B=OPEN`, no Stage-A code. If complete, `CORRECTION_B=ACCEPTED/PUBLISHED`; Gate 1 can begin. This is a design-only proof, not actual service evidence.

## 2. New Gate 1 / Gate 2 — source + executed synthetic tests

Gate-1 implementation can add **only**:
- `host/src/DeusOs.Control.Core/HostManagementRuntime.cs`
- `host/tests/DeusOs.Control.Core.Tests/HostManagementRuntimeTests.cs`

These are already included by default SDK `.csproj` compile globs, existing Core.Tests project, solution and CI. No additional project, `packages.lock.json`, extra dependencies, CI redesign, package upgrade, platform adapter, launcher, service registration, timeout wrapper or executable. Unexpected need for an extra file requires an explicit evidence-backed bounded scope decision **before** adding it.

Review runtime API for explicit discovery/connect by present exact locator, no auto-open, zero/multiple candidates fail closed, one serial Core session, 5 read-only methods via `ManagementServiceOperations`, existing asset policy and negotiated capabilities, Core error propagation, state/disconnect and cooperative cancellation. No duplicated reconnect loop, local task scheduler, separate RPC framing or platform-specific type. Public runtime must not reveal `DeusDeviceSession`, `DeusDeviceClient`, raw operation delegate or any Control API. **Do not misstate this as a total security boundary:** other trusted Core callers can still access the accepted broader Core interfaces.

Mandatory new fake-discovery/fake-transport tests (minimum **six distinct actually executed tests**) cover initial zero activity, absent/ambiguous/manual selection, all five named readonly operations and capability failures, asset PublishedOnly, existing per-session serialization/cancellation, unplug/replug locator continuity, idempotent responsive disposal and absence of accidental Control/raw RPC surface. Also explicitly document **not proven**: hard shutdown under stuck native I/O, identical physical board under same USB locator, exclusivity across processes, Windows/Linux USB behavior on physical boards.

Actual local deterministic compiler/test proof or separately accepted isolated candidate CI is required **before treating Gate-1/2 as accepted**. Core minimum 84, Transport minimum 24, but Core total must include **at least six new real tests beyond the exact 84-test baseline** (normally >=90); zero failed/skipped and Release warnings/errors zero; locked restore unchanged. Test-count change must be explained, not guessed. Actual post-publication exact-SHA hosted CI must subsequently validate the implemented source. If the managed local .NET executor again returns `CONTROL_PLANE_OUTCOME_AMBIGUOUS`, preserve the infrastructure error and **do not** label compiler/build PASS or FAIL, do not blindly rerun the same operation. An external authoritative operator test log can be accepted only with candidate SHA, invocation, counts and reproducible paths, without raw secrets.

## 3. Publication, real-device scope and completion

The synthetic Core runtime may be accepted as a **library foundation** without physical STM32 operations, because this gate explicitly has no device/hardware parity claim. A *future* CLI/Desktop/Web integration needs its own process/client-level gate and actual physical session validation when appropriate. Future Web/HTTP (even localhost) requires explicit local security, Origin/Host/CSRF/DNS-rebinding, authorization, size and bounded behavior; the Core runtime by itself does not solve these.

Follow established exact diff/whitespace/commit-parent non-force push, exact-SHA hosted CI and canonical `CURRENT_STATE` reconciliation; these are **acceptance checks, not excuses for empty implementation artifacts**. No source feature automatically follows the documentation correction.

## 4. Resource and non-interference evidence

Stage A source remains entirely under existing Host .NET Core and tests, so **target firmware code delta=0 only after exact path diff evidence**. Physical STM32F103C8 Flash/SRAM unchanged; historical measured margins **1,276 B Flash** and **296 B accepted static SRAM** are not a live ARM stack/headroom measurement. No SWD, firmware, reset, OLED, signing/custody, Git history rewrite or visibility mutation.

Stop on unapproved client process/IPC, unbounded false lifecycle guarantees, security overclaim, hidden WIP, missing new tests, unexpected project/lockfile, hosted CI result for wrong SHA, or any firmware/Host protocol change.
