# Deus OS — Management-only Runtime USB Migration Plan

**Boundary:** `MANAGEMENT_ONLY_RUNTIME_MIGRATION`

**Status:** **GATE 0 ACCEPTED/PUBLISHED — GATE 1 OFFLINE ACTIVE — GATE 4 NOT AUTHORIZED**
**Prepared:** 2026-10-18, based on source HEAD `1293abf08cf1677c81d99df018afee0ce31d37f1`, tree `ebe8ef6bdc0b26a2d5de937a3caf05a3b9fb84f7`. These are provenance anchors, not declarations of the current live HEAD.

**Documentation authority:** `docs/CURRENT_STATE.md` alone determines activation/current project state; `docs/ROADMAP.md` owns sequencing; `docs/DOCUMENTATION_MODEL.md` owns document roles. The owner-approved Gate-0 contract was **published at `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09`**, and exact-SHA [Host deterministic CI run 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) **SUCCESS** independently confirmed. Gate 0 is accepted; `CURRENT_STATE.md` activates **Gate 1 offline only**. This document does not activate physical candidate operations. Acceptance criteria and the live gate checklist are in `docs/MANAGEMENT_ONLY_RUNTIME_MIGRATION_ACCEPTANCE_PLAN.md`.

## 1. Owner direction and boundary

The owner selected **Management-only USB** and removal of target USB CDC ACM, then **conditionally approved the complete bounded Gate-0 design on 2026-10-20 after a fresh conflict review**. This records owner approval of the ABI and test/STOP criteria **for bounded offline source qualification**. Subsequent docs-only publication and exact-SHA CI **completed Gate 0 and activate Gate 1 offline only**; this is **not** installed firmware, a physical write, recovery operation or production USB identity. Current implementation/activation is owned by `CURRENT_STATE.md`.

Existing accepted runtime: private-test `1209:000C`, composite CDC ACM IF0/IF1 + vendor Management IF2. Target private **test** candidate: `1209:000E`, a single vendor-specific Management IF0, class `FF/00/00`, bulk OUT `0x04` and IN `0x84`, 64-byte packets. Device-level class/subclass/protocol `00/00/00`, no CDC IAD. The new PID avoids binding/cache conflicts with existing `000C`. Official pid.codes confirms `1209:000E` is a **private Test PID** (https://pid.codes/1209/000E/); its eligibility does **not** itself approve this scoped Gate-0 proposal and never authorizes redistribution, sale, manufacturing or release identity.

The independent bootloader stays `1209:000D`, vendor IF0, bulk `0x01/0x81`, with its existing distinct discovery GUID and signed-update/recovery protocol. UART/CH340, ST-LINK/SWD, application Flash/metadata/persistence ownership and authenticated update/rollback invariants must remain intact.

Out of scope: bootloader rewrite, signing-key handling or migration, broad new CLI/Web privilege surface, source-wide refactor, production diagnostics split, RTOS/HAL introduction and new hardware transports.

## 2. Gate-0 accepted test-only candidate design — **not physically deployed**

| Contract | Existing composite | Target test candidate |
| --- | --- | --- |
| VID:PID | `1209:000C` | `1209:000E` |
| Runtime Management | IF2 `FF/00/00` | IF0 `FF/00/00` |
| Bulk endpoints | `0x04` OUT / `0x84` IN, 64 B | Same |
| Physical USB controller | STM32F103 USB Full Speed | Same |
| USB class | Device `EF/02/01`, CDC IAD | Device `00/00/00`, no CDC |
| Config interfaces | 3 | 1; expected config length 32 B |
| Management MS OS 2.0 | WINUSB function subset IF2 | WINUSB function subset IF0 |
| Management GUID | `{C8B05EDE-1683-5002-81F0-95636B89CEC6}` | Same |
| Runtime RPC | Shared Binary Frame/RPC, 36 methods | Same ABI |
| Bootloader | `000D/IF0` independent | Unchanged |

BOS and Microsoft OS 2.0 request contract: vendor code `0x20`, request type `0xC0`, `wValue=0`, `wIndex=0x0007`, descriptor-set length 178 B, `WINUSB` compatible ID and matching `DeviceInterfaceGUIDs` REG_MULTI_SZ. Candidate descriptor layout, sizes and actual emitted BIN bytes must be checked, not inferred from source.

EP0 standard/vendor handling, EP4 PMA buffer layout (`0x180` RX and `0x1C0` TX), IRQ routing, SET_CONFIGURATION, reset/re-enumeration and logical configured status remain functional; CDC class setup, notification, EP1/2/3, line coding and state machine must be removed, not silenced via `__attribute__((unused))`.

## 3. RPC, Host and operator compatibility

Preserve Binary Frame and Binary RPC semantics, request IDs, CRC as accidental-corruption detection (not authentication), service version **3**, **36-method** Command Service registry, Management HELLO capabilities `0x7F`, response/status shape and numeric ID `0x0005`. Keep `cdcstat` as a deterministic **legacy compatibility method** reporting the absent CDC function; verify the precise response shape against parsers rather than silently dropping the method or changing registry count.

Host Windows and Linux must support **both** existing `000C/IF2` and target `000E/IF0` runtime transports, with strict VID/PID/topology/endpoint checks. Runtime discovery must never mistake `000D` bootloader for the application. WinUSB discovery GUID does not itself identify or authenticate a physical board, and a COM terminal is not automatically replaced by current Host CLI.

**Owner CDC disposition (2026-10-18): REMOVE USB CDC ACM from the target runtime.** The owner explicitly prefers reclaiming firmware Flash/SRAM over preserving the STM32 USB virtual-COM interface and **accepts loss of compatibility for external programs that require that USB COM port**. The tracked Host CLI/Desktop uses Management over WinUSB/libusb; no `SerialPort`, `System.IO.Ports`, or `ttyACM` consumer was found in the tracked Host/scripts scan. This bounded scan is not evidence that no external serial-terminal clients exist. Preserve the physical USART1/UART console through CH340, including emergency commands, and preserve the vendor Management RPC transport. Do not introduce a CDC emulation layer or unused CDC buffers merely for backward compatibility; the `cdcstat` RPC method remains as a deterministic legacy status response, not a USB CDC driver.

### Bounded Management diagnostics without CDC — source audit and owner-approved Gate-0 contract

Source observations (2026-10-18; **not** hardware acceptance): the current `ManagementServiceOperationCatalog` has exactly **five typed ReadOnly operations** (`Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout`) and **three typed Control operations** (`StartApplication`, `StopApplication`, `WriteOledUiLayout`). The existing Host CLI exposes a narrow **read-only observation sequence** through Management: `info` (HELLO + sysinfo), `ping`, `health`, `rpcinfo`, `apps`, `config-status`, `config-read` (the last two use `PublishedOnly`). CLI also has separate explicitly named state-changing commands `app-start`, `app-stop`, `config-set-layout`, which are **excluded** from read-only preflight.

The Command Service registry contains **36 RPC methods**, not 36 read-only methods. The firmware flags `appstart`, `appstop`, several OLED operations and `i2cscan` as `CLASS_SAFE`, although these can modify state, render/update UI or actively probe an I2C bus. In `binary_rpc.c`, the destructive-request flag explicitly gates only `CLASS_DESTRUCTIVE`; `CLASS_SAFE` and `CLASS_DIAGNOSTIC` are not an authentication or read-only boundary. `DeusDeviceClient.RpcAsync` is lower-level generic RPC; the eight-operation Host service allowlist does **not** automatically restrict all local USB frames. Existing local USB Management should therefore be treated as a **physically trusted bench interface**, not a remotely safe, authenticated service. No network/Web exposure or broadened privilege is authorized by this migration.

| Observation surface | Approved-for-proposed-read-only-preflight calls | Not part of read-only preflight |
| --- | --- | --- |
| Existing Host CLI / Management | `info`, `ping` (bounded), `health`, `rpcinfo`, `apps`, `config-status`, `config-read` | `app-start`, `app-stop`, `config-set-layout`, generic caller-selected RPC |
| Typed Host service v1 | `Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout` | `StartApplication`, `StopApplication`, `WriteOledUiLayout` |
| Target UART/CH340 (separate emergency path) | existing `mspstat`, `schedprod`, `fault`, `uptime`, `rxstat` and other explicit diagnostics subject to their own safety classification | USB CDC console does not return |
| **Proposed** bounded Management stack/legacy diagnostics (additional test-only adapter, not currently a Host CLI command) | fixed-ID RPC `uptime=0x0002`, `rxstat=0x0004`, `cdcstat=0x0005`, `mspstat=0x0006`, `schedprod=0x0007`, `fault=0x000A`; no arbitrary ID input | `schedtest`/`schedworkload`/`schedisolate`, `wdogtrip`, `i2cscan`, OLED mutations, firmware-update entry, Asset writes |

**Accepted/published Gate-0 diagnostic allowlist design (candidate behavior still unproven):** existing seven CLI read-only observations remain unchanged. A **test-only fixed-ID noninteractive adapter**, not the production Host CLI, is proposed for `uptime=0x0002`, `rxstat=0x0004`, legacy `cdcstat=0x0005`, `mspstat=0x0006`, `schedprod=0x0007`, `fault=0x000A`. Never add caller-selected `rpc <id>` or expose the `CLASS_SAFE` label as access control. By 2026-10-20, fixed-ID `rxstat`, `mspstat`, `schedprod` and `fault` have real read-only Host Core evidence **only on existing `000C/IF2`**; `uptime`/future `000E`/legacy `cdcstat` remain unproven in this particular adapter and must not be represented as accepted. Freeze per-method response schemas, side-effect classifications and rejection policy before promoting the allowlist; the 36-method target wire ABI remains unchanged.

CDC removal and the **bounded Gate-0 design choices** are owner-approved and **formally Gate-0 accepted following verified docs-only publication/exact-SHA CI**. The only newly active work is Gate-1 offline source/static verification; real-device conformance remains subject to separate later gates. Owner approval of design is not authorization for signing, protected recovery actions, flashing, replugging or reset.

### Legacy `cdcstat` ABI freeze — explicit no-CDC semantics

For Management-only `1209:000E`, preserve method `cdcstat`, RPC ID `0x0005`, zero arguments, successful read-only status, **one** CRLF-terminated line, and **15 fields in this exact order**, formatted as 32-bit hexadecimal values in the existing wire shape:

`USB_CDC_CONFIGURED CONFIGURATION RX_PACKETS RX_BYTES RX_DROPS RX_HIGH_WATER TX_PACKETS TX_BYTES TX_DROPS TX_HIGH_WATER CONTROL BAUD STOP PARITY DATA_BITS`.

- `USB_CDC_CONFIGURED=0` always: there is no runtime CDC function, interface or IAD.
- `CONFIGURATION=0` when the USB device is unconfigured and `CONFIGURATION=1` when **Management-only USB** is configured. This is the *shared device configuration state*, not a CDC-specific value to force to zero; keep GET_CONFIGURATION consistent.
- Every one of the **13 other CDC-only** statistics/control/line-coding values is zero: no CDC rings, endpoint activity, cached line coding or fabricated traffic. No hidden CDC emulation or unused buffers.
- Preserve response order/format and bound execution; offline parser, source and schema negatives belong to Gate 1/2. The configured `CONFIGURATION=1` case on real `000E` can be proved **only after authorized Gate-4 deployment**. Candidate noncompliance is a failure, not permission to silently drop the method.

Tracked baseline `src/kernel.c` prints these fields in order and `src/drivers/usb_device.c` handles `configuration` as a shared 0/1 state. This is a normative *proposed new-candidate ABI*, **not evidence that the isolated `000E` implementation already passes**.

### Gate-0 owner decision package — **accepted/published after exact-SHA CI** (2026-10-20)

**Owner approval record:** the owner authorized this bounded Gate-0 design **on condition that a fresh cross-document/source consistency audit found no material contradictions**. After the explicit `cdcstat` semantics and separate MSP policy were corrected, the 2026-10-20 read-only DEUS MCP audit passed **20/20 design-consistency checks**, verified **9/9** scope paths, preserved 36 registry IDs and found no remaining blocking *design-contract* conflict. This **satisfies the condition for owner design approval only**. It is **not Gate-0 publication**, not confirmation that the unflashed `000E` binary has passed every Gate-1/2 test, and not a physical GO. The following owner-approved review contract freezes the intended architecture; downstream acceptance tests must still prove compliance:

| Gate-0 decision | Proposed frozen contract | Acceptance boundary |
| --- | --- | --- |
| Development USB identity | **Test-only** `1209:000E`, never the current `000C` nor bootloader `000D`; independent release identity remains required | Approval covers *offline development design*, not flashing or distribution |
| Interface and descriptors | one vendor IF0 `FF/00/00`, device class `00/00/00`, bulk `0x04/0x84` at 64 B, configuration 32 B, BOS 33 B, MS OS 2.0 178 B, request code `0x20` and established management GUID | Real enumeration, host binding and EP0/PMA/IRQ/reset behavior are verified **only after authorized Gate 4**; Gate 1–2 retain rigorous offline proof |
| Wire protocol | protocol 1, service 3, **36 unchanged method IDs**, HELLO capability `0x7F`, request-ID correlation, framing/status/CRC; legacy `cdcstat=0x0005` keeps the same output **field names/order** but reports absent USB CDC deterministically | CRC does not authenticate commands; method class SAFE is not permission to perform state changes |
| Diagnostic access | fixed **test-only** read-only `uptime=0x0002`, `rxstat=0x0004`, `cdcstat=0x0005`, `mspstat=0x0006`, `schedprod=0x0007`, `fault=0x000A`; no generic RPC selector | `rxstat/mspstat/schedprod/fault` already observed on old `000C`, **not** `000E`; `uptime/cdcstat` still need scoped adapter schema/runtime proof |
| Host and recovery | dual runtime discovery `000C/IF2` + `000E/IF0`; strict descriptor/endpoints, separate bootloader `000D/IF0`; unchanged CH340 UART and ST-LINK safety | No CDC emulation shim, no remote/network trust escalation, no signing-key transfer, no bootloader/persistence rewrite |
| Exact source scope | four target paths + five Host/test paths listed in §4; Git materialization/hashes must match a single candidate tree | Additional source paths require separately reviewed scope amendment |
| Resource limits | Flash **≤53,248 B**, engineering static SRAM **≤11,264 B**, exact ELF/MAP/BIN/.su static evidence; source tree and Host tests must match | No physical `000E` stack readings until Gate-4 authorized deployment |
| Runtime stack policy (proposal) | In Gate 4, `MSP_STACK_OK` and `SCHED_PROD_OK`, intact all three canaries, nonzero/high-water used below capacity, **≥256 B measured margin** for MSP, console, heartbeat; confirm the candidate's own capacity/guard and natural active scheduler; **MSP threshold must be independently enforced by the Gate-4 Host verifier**, because current firmware's `MSP_STACK_OK` checks bounds/canary but **not** a 256-byte minimum | This is a **proposed policy threshold awaiting Gate-0 acceptance**, not a retroactive accepted budget; an uninstalled candidate has no runtime stack evidence |
| Operator safeguards | Windows owns CH340 and SWD, Ubuntu owns native USB/VBUS when rediscovered; fail on wrong device/host, source drift, malformed replies, correlation errors or lost recovery readiness | Gate 3 remains strictly read-only. Gate 4 requires a **new separate** explicit owner GO before flash/reset/USB-cable/power/signing actions |
| Publication | owner-approved plan/acceptance and policy/global-state docs published in commit `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09` with [exact-SHA CI 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) SUCCESS | **Gate 0 ACCEPTED/PUBLISHED; Gate 1 OFFLINE ACTIVE**. This is not physical Gate-4 authorization, hardware proof or Gate-5 product publication |

**Stack-policy interpretation:** 256 B is a minimum **measured stop threshold**, not a worst-case call-chain/nested-interrupt safety proof. The Gate-4 harness must reject `<256 B` on any of the three stacks even if the target reports `MSP_STACK_OK`, and must independently exercise the agreed bounded real workload/watchdog/USB reconnection cases; no uninstalled-candidate runtime PASS can be predeclared.

**Anti-circular gate boundary:** Gate 1 owns source/descriptor/negative **offline** proof; Gate 2 owns exact-tree ARM+Host builds, static resource/frame inventory and the **approved** candidate-runtime margin policy. Gate 3 is the existing rev5/`000C` **read-only** physical baseline plus currently applicable private recovery/power checks. Gate 4 owns **post-flash physical** `000E` enumeration, Management/legacy RPC, UART/OLED/IWDG and **real candidate MSP/scheduler watermarks** after an independently authorized signed update. Gate 5 owns review/publish only. Earlier rev5 proof is not re-executed to satisfy a future candidate condition.

**Final Gate-0 disposition after conditional approval, 20/20 design review and real docs publication:** `DESIGN_DIRECTION=OWNER_SELECTED`; `CDC_REMOVAL_DECISION=OWNER_ACCEPTED`; `FULL_ABI_AND_STACK_STOP_POLICY=APPROVED`; `GATE0_DOCUMENT_PUBLICATION=PASS_COMMIT_7F73EBD_CI_39452032151`; `FORMAL_GATE0=ACCEPTED_PUBLISHED`; `ACTIVE_GATE=GATE1_OFFLINE`; `GATE4=NOT_AUTHORIZED`. This authorizes **bounded offline source qualification only**, never signing, package recovery, target writes/reset/replugging or release.

## 4. Exact source/change scope

Proposed target firmware modifications:

- `src/drivers/usb_device.c`
- `src/kernel.c`
- `include/drivers/usb_device.h`
- `include/kernel/binary_rpc.h`

Proposed Host modifications/tests:

- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbDiscovery.cs`
- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs`
- `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbTransport.cs`
- `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbDiscovery.cs`
- `host/tests/DeusOs.Control.Transport.Tests/TransportContractTests.cs`

Required documentation integration **after Gate 0 review**: this scoped plan and acceptance pair, `docs/USB_IDENTITY_POLICY.md`, `docs/CURRENT_STATE.md`, `docs/ROADMAP.md`, and historical execution ledger as needed. Earlier accepted CDC/USB Management design and acceptance records remain untouched and historically valid.

A path-scope expansion, release PID, update-authority change or hardware write is a separate owner decision.

## 5. Evidence already obtained — correctly limited

- Pinned composite baseline: Flash **52,012 B**, engineering static-SRAM accounting **10,968 B** (includes 2,048 B MSP); BIN SHA-256 `A30E1DDA387C02D3C01774466586A64C4519D02B2A23415F3BA53A46A37F9D8D`.
- Earlier clean-source, old-PID `000C` offline build: Flash **48,992 B**, SRAM **7,516 B**, candidate tree `5b64a7b929cffe6280e19c01fc5233638021aa19`, BIN SHA-256 `EEFFE49F3E5FE5F0E2EE911AAC218A8A1886E6B71B6BD9ECD952CFE7070D1CCB`. This demonstrates linked resource savings of **3,020 B Flash / 3,452 B SRAM** for that experiment, **not** a deployable `000E` image.
- Isolated clean-source and dual-profile Host code exists in the DEUS MCP workspace; the **v5 Windows Host-only** Release and test proof passed (Core 87/87, Transport 45/45). Source has **not** been merged, published or physically accepted.
- Historical first Host+ARM collector of 2026-10-18 stopped **before either compiler** at `SOURCE_SIZE_OR_LF_MISMATCH` in LinuxLibUsbDiscovery.cs. Subsequent byte-domain proof confirmed Git-archive CRLF transformation (original 14,779 B LF vs TAR 15,221 B CRLF), and v4/v5 verified 275/275 raw Git blobs and original Git tree. **This harness defect is resolved**, without changing the original FAIL history.
- Earlier v4 **PID 000E** ARM compilation on source tree `9562332f2e44e35ce686d0df3ee60019d7550a25`: Flash **48,992 B**, static SRAM **7,516 B**, BIN SHA-256 `021DD33281CC87408714B7C3BFF7E823C5D57A2167495B2DCA10ACD820A45AE8`. v4's original overall verdict remains FAIL due to WinError 32 cleanup; a separate evidence-bound cleanup-only reconciliation proved CLEANED.
- **v5 Host-only** Windows identity hardening on a different source tree `4df48389940e9f821dc7c94045e7473340ef80f4`: locked restore PASS, Release warning-clean, Core **87/87 PASS** and Transport **45/45 PASS**, no skipped/failed; evidence ZIP SHA-256 `378218AE0D64C812457FDDC1EA47C66EA0B25D769413C61E1DDB336722A03E51` with CRC/67 of 67 indexed SHA-256 PASS, cleanup PASS. Nine final changed files match patch-stage SHA-256 (ten sequential edits because the Transport test file was patched twice).
- Historical v4 ARM and v5 Host have different source trees. **That exact-tree gap has now been closed by v6 ARM** on tree `4df48389940e9f821dc7c94045e7473340ef80f4`, which exactly matches the v5 Host proof. Runtime stack high-water, real USB enumeration and hardware acceptance remain **unverified for the future, unflashed `000E/IF0` candidate**. The separate 2026-10-20 MSP/scheduler read-only PASS pertains only to **currently installed `000C/IF2` rev5**.
- **v6 final exact-v5-tree ARM proof:** source candidate tree `4df48389940e9f821dc7c94045e7473340ef80f4` matches Host v5; fresh ARM build PASS, Flash **48,992 B**, static SRAM **7,516 B**, BIN SHA-256 `436B4CDEA1C91DCF4FEF382BCF8BF6546C26ABAF28786AAC300D7BD0DE87EC79`. Evidence ZIP SHA-256 `A25DD9CF8034FC9016AEC87E1E424232F6D5D4281DAD27056B4A709DDE17BE0C`, CRC PASS, **94/94** SHA files PASS, 31 .su files, ELF/MAP/BIN present, Git pre/poststate and cleanup PASS. **This remains an accepted dated offline proof, not a claimed re-read of unavailable ZIP bytes in the 2026-10-20 reconciliation.**
- All such offline evidence is private evidence/provenance, not a reason to commit ZIP/ELF/BIN into public Git. Static `.su` information does **not** replace a measured scheduler/MSP runtime stack high-water or physical USB/RPC acceptance.

## 6. Gate sequence and STOP conditions

### Gate 0 — ACCEPTED/PUBLISHED (owner approval + docs commit + exact-SHA CI)

The owner **approved** the bounded design contract and gate/STOP criteria (test PID, IF0/EP4/BOS/MS OS 2.0, Host compatibility, exact legacy RPC/`cdcstat`, loss of USB COM, nine-source scope, resource/stack stop thresholds, Host evidence, protected update/recovery policy). The eight-document docs-only publication at `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09` was a normal non-force push; exact-SHA [GitHub Actions run 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) completed **SUCCESS**. **Gate 0 is ACCEPTED/PUBLISHED; Gate 1 offline becomes active**, subject to its own full acceptance predicates. None of this authorizes Gate 4.

### Gate 1 — isolated source and static/descriptor tests (PARTIAL)

Recreate the exact candidate from pinned Git tree with full before/after source SHA-256. Reject unreviewed paths, broken descriptor topology, stale CDC symbols, wrong PID/IF/endpoint binding, unsafe RPC registry changes and references to the bootloader as a runtime device. Keep original main clean.

### Gate 2 — fresh `000E` ARM + Host proof (STATIC ARM/HOST SAME-TREE PASS; REMAINING OFFLINE PREDICATES OPEN)

Official ARM builder against the exact candidate source tree; capture raw build/link output, BIN/ELF/MAP/.su and their SHA-256, ensure Flash/SRAM/stack budgets and no linked CDC. Decode VID/PID, device/configuration/BOS/MS OS 2.0 from the actual candidate BIN.

Restore the .NET Host solution in locked mode; build Release with warnings as errors; run nonvacuous Core and Transport tests, including new positive/negative runtime profile tests. Verify no regression for `000C` or the separate `000D` bootloader.

The source byte-domain mismatch and Python `return`-in-`finally` runner defect have been resolved in v4/v5. **The exact same-tree ARM-v6 + Host-v5 proof is already PASS**, with 48,992 B Flash, 7516 B static engineering SRAM, 31 `.su` files, Core 87/87 and Transport 45/45. Do **not** request another basic compile merely because an older checklist says 'still required'. Remaining Gate-1/2 obligations are *specific* descriptor/EP0 negative/source-link/registry clauses and a **preapproved runtime margin policy**, not measurements from uninstalled firmware. Live `000E` stack high-water/canary/margins are a **Gate-4 post-installation** predicate, with a safe stop/recovery plan; never make Gate 2 depend on unauthorized flashing.

### Gate 3 — physical *read-only* preflight (PARTIAL SCOPED EVIDENCE / NOT ACCEPTED)

The dated acceptance plan records successful physical *read-only* preflight **on currently installed composite `1209:000C/IF2`, security revision 5 only**: Ubuntu Host Core `HELLO/sysinfo/rxstat`, two distinct Management `rxstat` snapshots bracketing one exact Windows CH340 10-byte UART request with **zero new RX errors/drops and no CDC opened** (SHA-256 `718BBB4C5EC23275E4CA86EA9543C158B647D726BCE27B80C301F7D49601AC65`); and 2026-10-20 real `mspstat=0x0006`, `schedprod=0x0007`, `fault=0x000A` read-only Host Core PASS (evidence SHA-256 `2D3C0F8F33552F81FE1B870908C7B733236989912F57EBFD130A558140E44669`). Current rev5 MSP margin 1556 B, console 456 B and heartbeat 432 B, canaries intact; scheduler/fault snapshot policy PASS. Do not repeat these measurements. This **does not** establish any physical behavior or stack margin for **unflashed `000E/IF0`**. Gate 3 remains PARTIAL pending specifically applicable power/host-ownership/3.3-V isolation and verifiable authenticated rev5 recovery/bootloader/revision-floor readiness; historical accepted rev5 HOTPLUG evidence must be reused within its scope. Owner-accepted *deferred* independent off-host escrow custody is separate from a rev5 rollback-safe recovery package. No Flash, reset, replugging or protected-key read is authorized by read-only evidence.

### Gate 4 — controlled physical acceptance (NOT AUTHORIZED YET)

Require a separate owner go/no-go after proof from prior gates and a valid, authenticated, rollback-safe update package. Validate real Windows/Linux enumeration, `000E/IF0`, WinUSB/libusb and descriptors; HELLO, RPC/health/assets/`cdcstat`, UART, OLED, watchdog, reset, recovery and verified Flash ownership. Reject wrong endpoints, class requests and malformed requests; do not perform destructive fault injection without established rollback.

### Gate 5 — publication / close-out (NOT RUN)

Bind final source tree, signed candidate, exact tested Host artifacts, physical attestation and hashes; reconcile only necessary global/policy docs and historical ledger. Review public-safe changed files, `git diff --check`, scoped commit, fresh-fetch/non-force push, exact-SHA CI and clean poststate. Never substitute offline ARM PASS for physical acceptance.

## 7. Public/private evidence boundary

This public planning contract contains no private host addresses, account identities, HMAC keys, DPAPI blobs, raw Flash or operational credentials. Real operator signing/recovery steps and evidence custody stay inside the private operational domain/repository. A separate approved release VID/PID is required for distribution.

## 8. Current classification

```text
TARGET_ARCHITECTURE=MANAGEMENT_ONLY_SELECTED
GATE0=ACCEPTED_PUBLISHED_DOCS_CI_39452032151
GATE0_OWNER_DESIGN=APPROVED_CONDITION_MET_2026_10_20
CLEAN_OLD_PID_ARM=PASS_OFFLINE_ONLY
NEW_PID_000E_ARM_V4=PASS_OFFLINE_ONLY_TREE_9562332F2E44E35CE686D0DF3EE60019D7550A25
FINAL_V5_CANDIDATE_ARM_V6=PASS_OFFLINE_TREE_4DF48389940E9F821DC7C94045E7473340EF80F4
FINAL_V5_BIN_SHA256=436B4CDEA1C91DCF4FEF382BCF8BF6546C26ABAF28786AAC300D7BD0DE87EC79
ARM_AND_HOST_SAME_CANDIDATE_TREE=PASS_OFFLINE_ONLY
HOST_DUAL_PROFILE_V5=PASS_CORE_87_OF_87_TRANSPORT_45_OF_45
NEW_000E_HARDWARE=NOT_FLASHED_NOT_TESTED
EXISTING_REV5_000C_READONLY=PASS_SCOPED_STACK_UART_MANAGEMENT
GATE1=PARTIAL
GATE2=PARTIAL_OFFLINE_SAME_TREE_PROOFS
FULL_GATE3=NOT_ACCEPTED
GATE4=NOT_AUTHORIZED
ACTIVE_GATE=GATE1_OFFLINE
GATE0_DOC_PUBLICATION=PASS_7F73EBD_CI_39452032151
GATE5_PRODUCT_PUBLICATION=NOT_STARTED
```
