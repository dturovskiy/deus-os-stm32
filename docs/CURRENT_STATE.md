# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

Git is authoritative for live repository bytes, commit/tree identity, branch state and cleanliness. This file records accepted product-boundary identities and current project disposition, not a substitute for `git status` / `git rev-parse`.

For architecture, roadmap sequence, scoped boundary contracts, evidence policy and historical records, follow `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED / PHYSICAL DEPLOYMENT VERIFIED**

Boundary publication commit:

`27fb10288ef45dcc9292287603e5ab8a26bf1fcb`

Boundary publication tree:

`0eca476d84eb1f06b633a7780b7883a3fadd8adb`

Gate-7 ordinary non-force publication evidence SHA-256:

`B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`

Gate-7 publication proved:

- ordinary non-force fast-forward of `main`;
- post-push `HEAD == origin/main == FETCH_HEAD == 27fb10288ef45dcc9292287603e5ab8a26bf1fcb`;
- clean worktree/index and ahead/behind `0/0`;
- GitHub `main` at the same commit/tree;
- verified GitHub commit signature;
- zero target I/O.

Post-publication physical deployment is also accepted from `stm32_os_published_deploy_adjudicated_retry_v7_20261003_141252.evidence.zip`, SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`. It proves the exact signed published-v2 package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, exact published application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, bootloader prestate `Ok/RecoveryIdle` with version floor/committed version `1/1`, successful END response `Ok/Committed` with floor/committed version `2/2`, committed metadata B version `2` with marker `0xA55A`, unchanged bootloader/persistence ownership, runtime source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, runtime/HELLO capabilities `0x0000007F`, `PONG`, health OK and `ROLLBACK_ATTEMPTED=False`. Post-deploy whole-Flash SHA-256 is `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`.

The accepted Gate-6 publication candidate is:

- firmware source tree: `8323c68c931894441ae4db9138ba3838f35bb8b6`;
- host source tree: `c95019bedfb6223705e6eba4f4c6d310b1701cdc`;
- firmware BIN SHA-256: `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`;
- application Flash/SRAM: `53212/10956`;
- system capability mask: `0x0000007F`;
- Host Core tests: `41/41`;
- Host Transport tests: `12/12`;
- Core/Windows/Linux Release builds: PASS;
- dummy-key bootloader resource replay: Flash `5904/8192`, `.data+.bss 660/1024`, conventional SRAM `1684/2048`;
- Gate-6 validation evidence SHA-256: `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7`;
- Gate-6 local acceptance continuation SHA-256: `806D999F1F9677A7E3A3C9D6E0D030079612978561C18A69167A0890A2EBC1CC`.

Canonical scoped records:

- design: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`;
- acceptance: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`;
- wire/image ABI: `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`.

## 2. Current product architecture

The published firmware-update layout is realized as:

```text
0x08000000..0x08001FFF  bootloader / recovery, pages 0..7, 8 KiB ceiling
0x08002000..0x0800EFFF  relocated executable application, pages 8..59, 52 KiB ceiling
0x0800F000..0x0800F3FF  authenticated firmware metadata A, page 60
0x0800F400..0x0800F7FF  authenticated firmware metadata B, page 61
0x0800F800..0x0800FBFF  persistent configuration slot A, page 62
0x0800FC00..0x0800FFFF  persistent configuration slot B, page 63
```

Stable accepted update properties:

- reset owner is the bootloader at `0x08000000`;
- application vectors/VTOR are at `0x08002000`;
- runtime USB remains private-test `1209:000C`; bootloader recovery/update USB is private-test `1209:000D`;
- package v1 is 48-byte header + 32-byte HMAC-SHA-256 authenticator + relocated application bytes;
- device enforces product/target/version/authentication/digest/vector checks;
- metadata commit marker `0xA55A` is programmed last;
- update DATA is sequential, with only the exact immediately previous DATA retry accepted idempotently;
- interrupted executable programming falls back to recovery and host retry restarts from BEGIN/offset zero;
- bootloader update writes do not own persistence pages 62/63;
- production host entry defaults to `PublishedOnly` and requires system capability bit 6.

Gate-5 hardware acceptance covers normal boot, explicit update entry, invalid-application recovery, malformed header, wrong target, bad digest, bad authenticator, rollback rejection, deterministic reset interruption, physical native-USB/VBUS power interruption, full retry, exact application/metadata ownership, unchanged bootloader/persistence ownership and deterministic handoff/VTOR.

The post-Stage-10 host reliability repair is accepted. It handles delayed responses from explicitly timed-out firmware requests without weakening unknown request-ID correlation failures, and maps Windows WinUSB pipe timeout codes `121/1460` to `HostErrorKind.Timeout`. No second firmware-side response-loss mechanism is claimed from the historical INFO/BEGIN/DATA timeout observations.

## 3. Published source and physical bench state

Gate 6 and Gate 7 were deliberately zero-target publication steps, so the board initially remained on the Stage-10 restored pre-publication baseline whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`. That historical distinction is now closed by the accepted post-publication deployment proof.

Current physical bench state is aligned with the published source/product identity:

- deployed signed package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`;
- deployed application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`;
- post-deploy whole-Flash SHA-256 `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`;
- metadata A retains the historical version-`1` committed record / marker `0xA55A`, but its digest does not match the current v2 application; metadata B is the single matching current commit, version `2` / marker `0xA55A`;
- bootloader pages and persistence pages are unchanged from prestate;
- runtime source tree is `8323c68c931894441ae4db9138ba3838f35bb8b6`;
- runtime and HELLO capability masks are both `0x0000007F`;
- runtime `PING=PONG` and health are accepted;
- deployment rollback was not attempted.

Authoritative deployment evidence SHA-256 is `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`. The earlier failed published-deploy smoke is historical forensic evidence only: its post-failure harness restored the old version-1 baseline, so that old static Flash state was not evidence that the published package had never transferred or committed.

## 4. Current work disposition

There is **no active product feature boundary** after publication and physical deployment verification of `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`. `FDC-01 / HOST_RPC_TIMEOUT_RECOVERY_HARDENING` is now **CLOSED / PUBLISHED** at `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`, tree `146704385bcaf0b7196b3519b8fe17488200fe63`. The active engineering boundary is now **`HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING` / `FDC-02`, Gate 0 contract freeze**.

The 2026-10-03 post-publication architecture/code audit promoted a mandatory **FOUNDATIONAL_DEBT_CLOSURE_PROGRAM** before any new Host Management Service/Web/network feature boundary. FDC-01 authoritative Gate-2 evidence is `stm32_os_fdc01_host_validation_dotnet_v1_20261004_151712.evidence.zip`, SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`: manifest `65/65` exact, Core `51/51`, Core Release build PASS, exact five-path WIP pre/post state, zero target I/O and zero Flash mutation. Gate 4 accepted local commit `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`; Gate 5 ordinary non-force publication and fresh fetch proved `HEAD == origin/main == FETCH_HEAD == c863b5ab9d00ab96de7c8f8275f905ed52c8740e`, and GitHub `main` independently matched the same verified commit. Canonical active FDC-02 plan/acceptance: `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_PLAN.md` and `docs/HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING_ACCEPTANCE_PLAN.md`. No `FDC-03+` source mutation is authorized by this activation.

The mandatory closure ledger uses IDs `FDC-01..FDC-10`:

1. `FDC-01` — **CLOSED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`** — generic RPC timeout/cancellation delayed-response correlation, multi-frame abandoned-RPC cleanup, bounded request-ID reuse/wrap behavior and firmware-update request-level no-blind-retry policy;
2. `FDC-02` — **ACTIVE / Gate 0** — `DeusDeviceSession.StateChanged` delivery must not permit reentrant deadlock through callbacks invoked while the session operation gate is held;
3. `FDC-03` — future host-management service/Web APIs must expose an explicit operation allowlist/authorization surface and must never proxy arbitrary raw `RpcAsync(id, flags)` calls;
4. `FDC-04` — host-management state required by service/Web must be represented by typed `DeusOs.Control.Core` models/parsers rather than presentation-specific parsing of `RpcResult.OutputText`;
5. `FDC-05` — composition-root convergence must remove remaining multi-owner reasons to change `src/kernel.c` when natural owners exist, without line-count-only framework refactoring;
6. `FDC-06` — semantic system/service state must become upstream of OLED/presentation state instead of reconstructing `application_service_snapshot_t` from UI indicator state;
7. `FDC-07` — application `stop()` failure semantics must be fail-closed before resource-owning applications are accepted;
8. `FDC-08` — boot/runtime robustness waits and update-entry recovery must become explicitly bounded: authenticated reset-vector span containment, bootloader and runtime HSE/PLL/switch waits, UART TX wait, elapsed-time bootloader-entry reset fallback, and non-DATA update ambiguity handling;
9. `FDC-09` — Windows/Linux native transport cancellation/shutdown semantics must be bounded and suitable for long-lived/multi-device host service use;
10. `FDC-10` — documentation drift and obsolete future-tense/deferred claims must be reconciled against the published implementation and canonical source-of-truth model.

The closure program is sequenced as: host long-lived-session hardening (`FDC-01..04,09`), target architecture/lifecycle cleanup (`FDC-05..07`), target/update robustness closure (`FDC-08`), then final documentation/source-of-truth reconciliation (`FDC-10`). `HOST_MANAGEMENT_SERVICE_FOUNDATION` or any equivalent Web/service boundary is blocked until all ten IDs are accepted closed.

The board remains on the exact published v2 image with capability `0x0000007F`; no speculative firmware-update forensic patch is authorized by this audit. The next broad networking/service/security work remains downstream of the closure program.

Host-management presentation evolution remains a non-authorizing direction in `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`.

## 5. Current development/acceptance topology

Canonical topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

Current bench facts:

- canonical repository/build host: Windows, `D:\Projects\STM32\OS`;
- target native USB + normal VBUS/power owner: Ubuntu on Mac mini;
- Windows owns ST-LINK/SWD and CH340/UART;
- accepted SWD speed: 950 kHz; read-only fallback ladder `950/480/240/125`;
- ST-LINK 3.3 V and UART-adapter VCC are disconnected;
- physical target power cycle is native-USB VBUS removal/restore on the Mac-mini connection;
- OLED: SSD1306-class 128x32 at `0x3C`.

A topology change must be reflected in the topology contract before topology-dependent hardware acceptance.

## 6. Completed/published foundation sequence

The following major foundations are complete/published and are **not active work**:

- scheduler timed blocking;
- scheduler fixed priority;
- production heartbeat task;
- IWDG production liveness;
- normal-boot production task ownership;
- native USB device core;
- USB CDC ACM diagnostics/console;
- transport-neutral shell/RPC;
- binary framed transport v1;
- OS application/UI model foundation;
- boot/desktop UI foundation;
- OLED dirty-region optimization;
- application runtime foundation;
- kernel composition-root decomposition;
- USB management device foundation;
- Host control application foundation;
- Asset / Configuration transfer foundation;
- pre-Bootloader resource architecture recovery;
- firmware update / bootloader foundation.

Their scoped plan/acceptance/protocol records remain engineering history/contracts. They do not override this file for current project disposition.

## 7. Deferred / trigger-driven work

Canonical deferred policy: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.

Not-active examples include:

- I2C IRQ/DMA until measured bus pressure;
- scheduler ready-set acceleration until measured scale/CPU pressure;
- CRC acceleration until measured streaming cost;
- tickless/low-power until a concrete power requirement;
- generic filesystem/storage layers without a concrete consumer;
- stable physical unit identity until a real multi-unit/privacy requirement;
- richer structured observability/runtime statistics;
- UI layout/preset/configurator expansion;
- networking/remote-management security until that boundary is explicitly promoted.

The ten `FDC-01..FDC-10` audit obligations are **not deferred/optional anymore**: they are promoted closure prerequisites for the next service/Web/network feature sequence. Promotion does not authorize an undisciplined patch; source mutation still requires a dedicated frozen closure slice and acceptance proof.

The pre-existing firmware-update robustness items are consumed by that program rather than left indefinitely trigger-driven: authenticated `image_length` reset-handler containment, bounded bootloader HSE/PLL startup, explicit non-DATA timeout/restart/adjudication semantics, and bounded stale request-ID lifetime under repeated cancellation/wrap. `FDC-08` additionally covers the newly confirmed normal-runtime HSE/PLL/switch waits, UART TX wait and poll-count-based bootloader-entry reset fallback.

## 8. Repository/security hygiene state

The post-publication read-only audit found:

- no tracked build outputs, firmware binaries, maps, evidence archives or logs;
- no tracked key/certificate/private-key artifact types;
- no such suspicious artifact filenames in Git history;
- no product-source TODO/FIXME/HACK markers requiring closure;
- no raw acceptance key in repository or shareable evidence;
- the immutable bootloader recovery bundle is intentionally **private/sensitive** because its recovery region contains the compiled key-bearing bootloader, even though no standalone raw key file is stored.

The repository-owned `.gitignore` covers build outputs, archives/logs, .NET outputs, common local secret files and editor/temp noise.

## 9. Known architecture state

Resolved/current:

- semantic application events are separate from scheduler wake bits;
- system identity/capability discovery is provided through `sysinfo`;
- USB management and host transport are separated from CDC diagnostics;
- Windows/Linux host control share one protocol/Core model;
- Host Core update/RPC/Asset responsibilities share one correlation/channel owner rather than duplicate decoders/locks;
- firmware-update authenticity, target binding, rollback floor, interruption recovery and publication capability are accepted.

Still intentionally unresolved/trigger-driven outside the mandatory closure program:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- network mutation security;
- richer observability/runtime-statistics framework.

Separately, `FDC-01..FDC-10` are now mandatory closure obligations rather than consumer-triggered debt; their sequencing and closure criteria are recorded in `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md` and the relevant scoped plans.

## 10. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- development/acceptance topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- forward ordering: `docs/ROADMAP.md`
- scoped boundary design: matching `*_PLAN.md`
- scoped boundary proof: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements and promoted debt provenance: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- chronology: `CHANGELOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
