# Deus OS — Host Management Service Foundation Plan

Status: **ARCHITECTURE CORRECTION B / GATE-0 SCOPE REPLACEMENT — PUBLISHED AND VERIFIED: 18e5037a9a3adf04f1bcb7cd88785ea4365e2b6c, CI 38739187311 SUCCESS.** The previous Gate-0 docs `bfb30d5f23389b54f0c114959640dc2d5a6291f9` (CI `38732869436`) and locked-restore Amendment A `0f95c2e5036652251976a0f7d6d5f62ea419116d` (CI `38733800122`) were indeed published/accepted as historical decisions. They are **not** proofs that their nine-path executable-service architecture was sound. Correction B replaces that *future* architecture; **no Gate-1 source has been created or accepted**.

Product boundary: `HOST_MANAGEMENT_SERVICE_FOUNDATION`. Global authority: `docs/CURRENT_STATE.md`. Gate evidence: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.

## 1. Architecture decision (corrected, minimal)

The actual accepted substrate is `host/src/DeusOs.Control.Core/`: `IDeviceDiscovery`, `DeusDeviceSession`, `ManagementServiceOperations`, strongly typed management results and capability gating. Windows and Linux transports are independent projects; CLI and Avalonia Desktop are consumers. The earlier plan to introduce a long-running `DeusOs.Control.Service` EXE **without defining how clients reach it** was a design mistake: a cross-process consumer would require IPC/auth, explicitly prohibited in Stage A. A runtime in that EXE would also be an awkward dependency for a later Web host. Adding a new library assembly simply to wrap the already existing Core would add another needless layer.

**Stage A is instead one in-process, reusable `HostManagementRuntime` class inside `DeusOs.Control.Core`, tested inside existing `DeusOs.Control.Core.Tests`.** Its unique value is **explicit device discovery/selection + one session's lifecycle + a strictly five-operation typed read-only façade**. It is not another transport, another service RPC, another state machine, another timer, and not an OS/background service. Neither CLI nor Desktop is modified at this stage; future consumers reference the existing Core assembly. No new .NET projects, executables, solutions, packages, package locks, framework registrations, workflow changes, sockets or IPC are justified by Stage A. If the class adds nothing beyond the frozen Core behavior during implementation review, Gate 1 must be stopped rather than introducing a redundant pass-through.

The owner-selected Host-management product direction remains valid. This is an explicit **Gate-0 architecture correction**; never rewrite the actual historical commits or mark the old design as having produced a running service.

### Architecture / dependency direction

```text
Current CLI / Desktop          Future Web/localhost adapter (NOT STAGE A)
          \                  /
          (future consumers, independent activation gates)
                       |
      DeusOs.Control.Core (existing reusable library)
      HostManagementRuntime [Stage A: new in-process API]
          -> DeusDeviceSession [existing serial operation owner]
          -> ManagementServiceOperations [existing typed service façade]
                       |
            injected IDeviceDiscovery
             /                   \
      WinUSB (Windows)       libusb (Linux)
                       |
                      STM32
```

The new Core class receives `IDeviceDiscovery` by injection; its source cannot import Windows/libusb, discover a platform globally, spawn a process or hold native platform handles outside `DeusDeviceSession`. The Core project stays dependency-free. Selection and ownership are **per runtime instance**; Windows/Linux processes are not thereby guaranteed device exclusivity across processes.

## 2. Stage-A exact API and behavior to test

- Construct with an explicit non-null `IDeviceDiscovery`. Initially `Disconnected`; no discovery, USB opening, polling or background task on construction. The runtime owns exactly one `DeusDeviceSession` and composes exactly one `ManagementServiceOperations` instance, both private; it never publicly returns the raw session, client, transport, general operation delegate or the existing three Control methods.
- `DiscoverAsync(CancellationToken)` delegates to injected discovery only, returns typed `DeviceCandidate` entries; no connect/claim and no implicit single-device selection. `ConnectAsync(string locator, CancellationToken)` requires a **nonempty explicit locator**, enumerates currently visible candidates and rejects absent/ambiguous exact matches instead of silently connecting the first device, then calls **existing** `DeusDeviceSession.ConnectAsync` on the matched candidate. The existing Core owns negotiation, capabilities, state/error classification, serial operation lock and recovery. Different device selection requires another explicit connect.
- Read-only runtime methods: `PingAsync`, `HealthAsync`, `ApplicationsAsync`, `AssetStatusAsync`, `ReadOledUiLayoutAsync`, routed exclusively through existing `ManagementServiceOperations` (its `AssetAccessPolicy.PublishedOnly` applies to assets). They do not reimplement RPC or grant extra capabilities. `State`/`LastNegotiation` are observations from the existing session, not separately cached truth; existing `StateChanged` is an asynchronous best-effort notification, **not** a synchronous delivery or full-drain guarantee.
- `DisconnectAsync(CancellationToken)` invokes existing session disconnect; `DisposeAsync()` is idempotent and delegates session disposal. A disconnected/faulted session must not claim Ready; startup, device absence, cancellation and protocol errors must be deterministically observed. No new cross-process/device-wide singleton or `Task.Run` polling worker.

### Lifecycle truth / correction to the earlier claim

`DeusDeviceSession.DisposeAsync()` currently awaits `_operationGate.WaitAsync()` **without a cancellation token** and can then await native transport close. A new wrapper **cannot truthfully guarantee a finite shutdown deadline** for arbitrarily hung operations. Stage A guarantees cooperative serialization, canceled waits where the existing Core/transport honors cancellation, no fire-and-forget abandonment and idempotent resource ownership in responsive tests. It **does not** claim wall-clock bounded disposal under a stuck native call. Do not synthesize a false PASS using `Task.WhenAny`, `WaitAsync(timeout)`, `Task.Run` or unobserved background disposal while the USB handle remains owned. If a future daemon/Web product requires a hard deadline, freeze a **separate Core transport/session shutdown change with its own cancellation/close semantics and acceptance** before advertising that guarantee. This is an explicit limit, not a loophole.

### Physical USB identity / correction

Current `DeusDeviceSession.RecoverAsync()` reselects by the previous **USB locator** (string equality), which is a connection/topology address, not a cryptographically verified immutable board identity. Stage A inherits this behavior without adding an unproven identity layer. Unplug/replug to a **different physical board at the same locator** is not distinguishable merely through locator equality; Stage A must **not** claim same physical device attestation. Read-only operations remain confined to the enumerated device and its negotiated service capability; promotion to safety-critical/remote/control writes must first define authenticated device identity and authorization separately. Tests cover same-locator recovery and changed/absent locator rejection as observations, not a security proof.

## 3. Stage-A exposure / forbidden behavior

Stage A exposes precisely **five named read-only methods** and lifecycle/discovery. The unchanged Core service-v1 descriptor catalog remains **5 ReadOnly / 3 Control / 0 Destructive**; Stage-A runtime exposes **0 Control / 0 Destructive**, never raw RPC IDs, arbitrary payloads, serial flags, authenticated update/bootloader, flash writes, reset, invasive watchdog tests, arbitrary host process execution, secret capture or unpublished-asset reads. Tests must check negative API/source exposure instead of assuming private fields alone enforce it.

No HTTP/WebSocket/socket, stdio command interpreter, named pipe, network listener, local daemon, service registration, autostart or network auth is included in Stage A. The presentation plan's browser -> HTTP/WebSocket -> host-service architecture is a **future conceptual topology**, not a completed or implicitly authorized transport. `localhost` without authenticated clients is not security. A future local Web adapter will separately freeze process model, caller auth, CSRF/Origin/Host/DNS-rebinding, operation authorization, session ownership, payload limits and error contracts before opening a listener; remote network/ESP remains another independent boundary.

## 4. Frozen next Gate-1 patch boundary (supersedes nine-path Amendment A)

**Exactly two new tracked source/test paths**:
1. `host/src/DeusOs.Control.Core/HostManagementRuntime.cs`
2. `host/tests/DeusOs.Control.Core.Tests/HostManagementRuntimeTests.cs`

The existing `host/DeusOs.Control.sln`, project files and global .NET package/lockfiles and `.github/workflows/ci.yml` **already discover and build new .cs files by default** and require no edits for this minimal class. The prior nine future paths (`DeusOs.Control.Service` EXE + tests + two locks + solution + CI) are now **withdrawn as authorized Gate-1 scope**, not silently marked implemented. Do not remove the original amendment from history. If it becomes necessary to modify Core session/transport, CI test thresholds, other projects or lockfiles, stop and obtain one evidence-backed scope extension; never create an adapter or project simply to satisfy the former path count.

Gate 1 is a local *source candidate*, **not** an automatically acceptable commit. New tests must prove nonopening initialization, explicit discovery/selection, zero/one/multiple candidates, 5 typed delegations and asset PublishedOnly policy, capability/error propagation, same-locator recovery limitation, concurrent requests through existing serial gate, cooperative cancellation, in-flight dispose waiting, and no raw/control exposure. Verify the new tests actually ran (at least six new cases; exact test method evidence / expected Core count), existing Core >=84 and Transport >=24 unchanged, all failed/skipped=0 and Release warning/error=0. Do not claim native/hardware parity from synthetics. Build/test gate does not silently pass on the prior 84/24 CI; **real new test execution is mandatory**.

## 5. Gate workflow — minimal and evidence-conditioned

- **Architecture correction B (now)**: docs-only patch to this plan, acceptance, `CURRENT_STATE`, `ROADMAP`, master checklist and changelog. Exact staged paths, staged check, parent proof; normal non-force push then exact new-SHA hosted CI SUCCESS. It is **not** new service implementation. Earlier published Gate-0 and Amendment-A design artifacts remain historically PASS.
- **Gate 1 / Gate 2**: scoped two-source-file candidate plus existing synthetic Core test suite. Prefer the existing build/test runner with actual local .NET proof. The DEUS MCP `dotnet_project` executor twice returned `CONTROL_PLANE_OUTCOME_AMBIGUOUS` without a compiler log, which is **infrastructure indeterminate**, not a build result. Do not replay blind or call it compiler FAIL. If local deterministic build/test is unavailable, stop source acceptance and obtain an authorized real Windows/Linux test log or supported isolated PR CI proof; never claim old hosted CI validated uncommitted new files.
- **Acceptance/publication**: require confirmed new Core tests, unchanged Transport tests, clean Release, non-vacuous diff, no extra tracked outputs, direct-parent non-force publication and exact-SHA hosted CI on implemented source. Canonical status updates are part of subsequent accepted documentation closure, not preliminary PASS.
- **Separate future host adapter / device test**: actual physical Windows/Linux session acceptance only if a promoted client requires it. No MCU Flash/bootloader/secret/physical action is required for this host-only pure-synthetic library foundation. Do not introduce empty Gate-3 hardware ceremony merely to claim a library is useful; distinguish library acceptance from later device/session integration and independent Web/security gates.

## 6. STM32 memory and hard stops

No STM32 source/build/flash change in this Host-only contract. The physical partition remains 8 KiB bootloader, 52 KiB application, 2 KiB metadata, 2 KiB config. Existing application build ceilings Flash **53,248 B**, accepted static SRAM **11,264 B**; historical measured candidate **51,972 / 10,968 B** leaves **1,276 B / 296 B within those ceilings**, not a current ARM proof or free task-stack claim. A future MCU feature requires fresh ELF/BIN/MAP/.su and documented MSP/task headroom. Do not relax accepted ceilings to fit features.

Stop on unexpected source WIP, unrelated files, raw transport/RPC/control exposure, new OS process/listener, fabricated forced timeouts, unproven physical identity, auto USB open, inconsistent Core policy, unrun test cases, .NET/CI inconclusive result, or Git parent/CI mismatch. **No force push, Git-history rewrite or secret/key/target I/O**. None of these restrictions reopens the owner-closed earlier custody programme.
