# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

Git is authoritative for live repository bytes, commit/tree identity, branch state and cleanliness. This file records accepted product-boundary identities and current project disposition, not a substitute for `git status` / `git rev-parse`.

For architecture, roadmap sequence, scoped boundary contracts, evidence policy and historical records, follow `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED**

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

## 3. Published source versus physical bench state

Gate 6 and Gate 7 were deliberately zero-target publication steps. Therefore:

- **published source/product identity** is the Gate-6/Gate-7 state above with capability mask `0x0000007F`;
- **last hardware-proven physical bench image** is the Stage-10 restored pre-publication candidate with whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`;
- that Stage-10 bench state predates the Gate-6 capability-bit activation, so the physical board must not be assumed to contain the published `0x0000007F` image until a separately authorized deployment/smoke installs and verifies it.

This distinction is not a Gate-6 acceptance gap: the frozen Gate-6 contract required documentation reconciliation plus build/test/source-consistency validation after complete Gate-5 hardware acceptance, not another target mutation. Any future task that requires **bench == published source** must explicitly establish that state first.

## 4. Current work disposition

There is **no active product feature boundary** after publication of `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`.

The post-publication documentation/repository hygiene and architecture audit is complete at the current documentation state. No product source repair or target mutation was authorized by that audit. New work requires explicit promotion of a concrete boundary.

The next broad roadmap area remains networking/service/security evolution, but it is **not active** until a concrete consumer/problem is named and a dedicated plan + acceptance plan freeze the boundary.

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

Post-publication firmware-update robustness debt is recorded in the backlog and is **not** implicitly authorized for patching:

- reset-handler containment inside the authenticated `image_length` span;
- bounded HSE/PLL startup failure handling in the bootloader;
- explicit non-DATA firmware-update timeout/restart policy;
- stale request-ID lifetime under repeated user cancellation.

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

Still intentionally unresolved until a consumer or dedicated robustness boundary requires them:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- network mutation security;
- richer observability/runtime-statistics framework;
- the four post-publication firmware-update robustness items listed above.

## 10. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- development/acceptance topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- forward ordering: `docs/ROADMAP.md`
- scoped boundary design: matching `*_PLAN.md`
- scoped boundary proof: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- chronology: `CHANGELOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
