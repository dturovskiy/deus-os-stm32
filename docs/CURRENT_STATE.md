# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

Git is authoritative for live repository bytes, commit/tree identity, branch state and cleanliness. This file records accepted product-boundary identities and current project disposition, not a substitute for `git status` / `git rev-parse`.

For architecture, roadmap sequence, scoped boundary contracts, evidence policy and historical records, follow `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` remains the **latest named product-feature boundary**: Gates 0–7 accepted/published at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb` (source tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`). Its originally deployed signed firmware revision 2 and complete publication evidence are historical; **the physical board now runs the later accepted authenticated application revision 5**, documented in §4.

Canonical feature design: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`; acceptance and historical deployment: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`; fixed wire/package ABI: `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`. Later FDC/RDC source changes are accepted maintenance, **not** a newer named product-feature boundary.

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

## 3. Historical FDC-05..08 hardware snapshot

The FDC-05..08 hardening campaign was accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8`. Its 2026-10-07 signed candidate-v3 Flash/USB/physical-display evidence is valid **for that historical build**, not a claim that candidate v3 is still installed. Refer to the scoped `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_ACCEPTANCE_PLAN.md`, `docs/KERNEL_COMPOSITION_ROOT_CONVERGENCE_ACCEPTANCE_PLAN.md`, related FDC acceptance records and `CHANGELOG.md`.

Subsequent RDC-06 hardware acceptance superseded the installed application with signed revision 5; see §4. No old evidence is reinterpreted as a newly executed hardware test.

## 4. Current work disposition

**No new product-feature boundary is active.** `FDC-01..FDC-10` are all CLOSED/PUBLISHED, and `RDC-01..RDC-07` are CLOSED/PUBLISHED. Their exact commits, detailed gate evidence, measured resource/test outcomes and historical order belong to the canonical `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`, the scoped acceptance plans and `CHANGELOG.md`, not a repeated gate-by-gate narrative here.

**RDC-08 final closure adjudication — event-bound:** Gate-0 docs scope was published at `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`, Gate-1 exhaustive 113-Markdown classification at `c646f3e3ac342cbaef025d115e5be96bd49cf140`, and Gates 2–7 scoped documentation reconciliation/acceptance at `faabfe75bbb3b805548092c9f298ac862c38ef1d` with GitHub Actions [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) **SUCCESS**, Core `84/84`, Transport `24/24`, Release 0 warnings/errors and zero failed/skipped. Its final Gate-8 closure record is the publication of this documentation revision. **RDC-08 and the full RDC program are CLOSED/PUBLISHED exactly when the Gate-8 fresh-fetch/non-force/exact-SHA CI conditions specified in `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` have succeeded; until then OPEN.** The documented result is evidence-conditioned, not an unchecked future claim or a hardcoded live HEAD. No product-feature boundary is automatically activated.

**Current accepted physical STM32 state (RDC-06 hardware acceptance, 2026-10-10):** the board runs authenticated **application firmware security revision 5**; application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, accepted application source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. An independent full 64-KiB HOTPLUG Flash readback verified authenticated version floor 5 and unchanged bootloader/persistence ownership. The separately published STM32 USB EPnR CTR race repair is `83e57f609bab4bcc8a61ea2222629bf9066e910d`; the corrected bootloader passed 72/72 INFO USB regression and the signed revision-5 update. USB/RPC, scheduler, IWDG, Windows CH340/USART1 `115200 8N1` command help and physical OLED were accepted, with operator `PHYSICAL_OLED=PASS`. Hardware evidence SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD` is historical accepted proof. The numeral **5 denotes signed firmware-update security revision, not an OS semantic version**. Gate-0/Gate-1/Gate-2 documentation work did **not** reflash, reset, or freshly revalidate the MCU.

RDC-07 bootloader readability normalization is CLOSED/PUBLISHED at `7846fa48429d00233116438821acc0ea2a0b38be`: byte-identical **synthetic-key/nondeployable** ARM build equivalence, no change to production key or installed firmware. Its full Gate-2 evidence and exact equivalence conditions are retained in `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md`.

Host-management presentation remains a non-authorizing future design (`docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`). **Product-feature selection becomes eligible only upon independently verified RDC-08 Gate-8 closure;** until that proof it remains blocked. Even after program closure, no Host Management Service, Web, networking or other feature is active without its own canonical plan/acceptance and a new explicit promotion. The last published product-feature boundary remains `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`.

**Active maintenance: `PUBLIC_REPOSITORY_SANITIZATION` Gate-1 read-only public/private exposure and consumer classification.** Gate-0 bounded design was accepted/published at `558306fe618d1ee69240fa11d24437cd38fb8e2b`, exact-SHA hosted [CI 38080940176](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38080940176) SUCCESS. Previous FDC/RDC hygiene checked obvious build/secret artifacts and documentation drift, **not** which internal operator/evidence material should be publicly distributed or how old Git history should be managed. Canonical contracts: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` and `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`. No public file removal, private-archive transfer, Git-history rewriting, repository visibility change, source/Flash/key/target manipulation or automatic new product feature is authorized.

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

The ten `FDC-01..FDC-10` audit obligations are **closed/published**. They no longer own current work ordering. The mandatory residual-debt ledger is now `RDC-01..RDC-08`; each source/tooling item requires its own frozen maintenance plan/acceptance pair, and no new product feature boundary may be promoted until the entire RDC program closes.

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

Still intentionally deferred **because they are future capabilities or consumer-triggered enhancements, not confirmed current debt**:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- network mutation security;
- richer observability/runtime-statistics framework.

The completed `FDC-01..FDC-10` program remains historical closure provenance; its sequencing and closure criteria are recorded in `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md` and the relevant scoped plans.

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
