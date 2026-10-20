# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

Git is authoritative for live repository bytes, commit/tree identity, branch state and cleanliness. This file records accepted product-boundary identities and current project disposition, not a substitute for `git status` / `git rev-parse`.

For architecture, roadmap sequence, scoped boundary contracts, evidence policy and historical records, follow `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` remains the **latest fully closed target-resident product-feature boundary**: Gates 0–7 accepted/published at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb` (source tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`). Its originally deployed signed firmware revision 2 and complete publication evidence are historical; **the physical board now runs the later accepted authenticated application revision 5**, documented in §4.

Canonical feature design: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`; acceptance and historical deployment: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`; fixed wire/package ABI: `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`. Later FDC/RDC source changes are accepted maintenance, **not** a newer target-resident feature. The newer Host-only `DESKTOP_PUBLISHED_ASSET_INSPECTOR` has its source/CI published and its Ubuntu USB/CLI Gate 3b.1 accepted, but **its connected read-only Avalonia fallback UI Gate 3b is now operator-attested PASS (2026-10-16)**; see §4. The operator-attested UI closure covers the no-committed-record fallback, **not** physical positive retrieval of an already committed OLED layout.

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

**2026-10-16 — `DESKTOP_PUBLISHED_ASSET_INSPECTOR` CLOSED for the approved read-only fallback slice.** Gates 0–3b accepted: source/Host CI published (implementation `4729341e732ffd61e7fe2b0e48973f2c98726a5d`, Core 87/87, Transport 24/24), Ubuntu read-only USB/CLI Gate 3b.1 PASS, and connected Windows Avalonia Gate 3b **`PHYSICAL_DESKTOP_UI=OPERATOR_ATTESTED_PASS`**. A temporary owner-directed USB attachment to Windows enabled actual WinUSB/CLI success and a visually observed `READY` session and manually refreshed published OLED asset displaying the correct `CONFIG_COMMITTED=NO` / compiled-default fallback. The physical evidence ZIP passed CRC and 32/32 indexed SHA-256 checks; the UI confirmation combines operator screenshots and checkbox attestation, **not an automatically recorded screenshot in the ZIP**. The positive physical read of a previously committed OLED layout remains **`NOT_VERIFIED`** and is outside the accepted fallback slice; no configuration write, firmware flash, driver change or Git mutation was performed. See `docs/DESKTOP_PUBLISHED_ASSET_INSPECTOR_ACCEPTANCE_PLAN.md` for scoped proof and the PRIVATE evidence registry for artifact identifiers. **No new feature or hardware gate is activated by this closure.**

**Earlier Gate 3b checkpoints (historical only):** the 2026-10-15 Ubuntu headless/Windows no-device preflight and 2026-10-16 Ubuntu CLI-only outcome correctly recorded `PHYSICAL_DESKTOP_UI=NOT_VERIFIED` *before* the later temporary Windows connected-UI execution. The original acceptance and chronological records remain in the scoped plan, checklist and changelog; they are **not** outstanding work. Canonical bench topology remains Ubuntu Mac mini USB / Windows UART+ST-LINK; the later **2026-10-19 live Management and CDC observations on `macmini`** now positively verify return of the native-USB link to Ubuntu for that date, without changing the earlier Windows UI acceptance or proving any future cable state. The topology document owns host/cable detail.

**Historical Host-management architecture decision (2026-10-15; no new boundary currently activated).** Architecture re-review C (2026-10-15) independently compared the accepted Core API with Desktop and CLI and classified the proposed unconsumed `HostManagementRuntime` as **`NO_GO_UNCONSUMED_RUNTIME_WRAPPER`**: `DesktopController` already combines discovery, selected session, typed commands and teardown; `ManagementServiceOperations` already supplies the 5 typed ReadOnly and 3 trusted Control operations. The previous nine-file service EXE and subsequent two-file wrapper proposals are **withdrawn from future implementation scope**, not retroactively un-published or mislabeled product successes. **This was the architecture-C pre-Desktop snapshot, not the current feature status:** the concrete Desktop published-asset inspector was subsequently implemented and published as source commit `4729341e732ffd61e7fe2b0e48973f2c98726a5d` with Core 87/87 and Transport 24/24 exact-SHA CI PASS (see the current Desktop entry above). No speculative `HostManagementRuntime` or lockfile has been implemented, and a later Web consumer still requires independent authorization. Core's disposal cannot guarantee a deadline under hung native I/O, recovery locator is not authenticated board identity, and concurrent discovery/selection needs consumer-level policy. Current canon: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md` and `_ACCEPTANCE_PLAN.md`; original Gate-0 docs `bfb30d5`, Amendment A `0f95c2e`, Correction B `18e5037` and status sync `bdc5cdf` remain truthful historical docs-only PASS. This review has no STM32/Host source/bootloader/key/Flash/SRAM changes; exact publication/CI is separately required.

**RDC-08 final closure adjudication — event-bound:** Gate-0 docs scope was published at `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`, Gate-1 exhaustive 113-Markdown classification at `c646f3e3ac342cbaef025d115e5be96bd49cf140`, and Gates 2–7 scoped documentation reconciliation/acceptance at `faabfe75bbb3b805548092c9f298ac862c38ef1d` with GitHub Actions [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) **SUCCESS**, Core `84/84`, Transport `24/24`, Release 0 warnings/errors and zero failed/skipped. Its final Gate-8 closure record is the publication of this documentation revision. **That Gate-8 predicate has been fulfilled:** final docs-only commit `c309428b03615a9afd514e3c7c8886a82a5112e9` was published non-force, independently fresh-fetched clean `0/0`, and the exact-SHA [GitHub Actions run 38079465787](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079465787) finished SUCCESS. Therefore **RDC-08 and all eight RDC items are CLOSED/PUBLISHED**, as independently recorded in `docs/MASTER_EXECUTION_CHECKLIST.md`. The documented result is evidence-conditioned, not an unchecked future claim or a hardcoded live HEAD. No product-feature boundary is automatically activated.

**Current accepted physical STM32 state (RDC-06 hardware acceptance, 2026-10-10):** the board runs authenticated **application firmware security revision 5**; application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, accepted application source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. An independent full 64-KiB HOTPLUG Flash readback verified authenticated version floor 5 and unchanged bootloader/persistence ownership. The separately published STM32 USB EPnR CTR race repair is `83e57f609bab4bcc8a61ea2222629bf9066e910d`; the corrected bootloader passed 72/72 INFO USB regression and the signed revision-5 update. USB/RPC, scheduler, IWDG, Windows CH340/USART1 `115200 8N1` command help and physical OLED were accepted, with operator `PHYSICAL_OLED=PASS`. Hardware evidence SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD` is historical accepted proof. The numeral **5 denotes signed firmware-update security revision, not an OS semantic version**. Gate-0/Gate-1/Gate-2 documentation work did **not** reflash, reset, or freshly revalidate the MCU.

RDC-07 bootloader readability normalization is CLOSED/PUBLISHED at `7846fa48429d00233116438821acc0ea2a0b38be`: byte-identical **synthetic-key/nondeployable** ARM build equivalence, no change to production key or installed firmware. Its full Gate-2 evidence and exact equivalence conditions are retained in `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md`.

**Host presentation scope:** `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` remains a non-authorizing **future Web/browser concept**, although the general Host-management product direction has been explicitly selected. The present Architecture Review C records `NO_GO` for the unconsumed Core runtime proposal, not cancellation of the product goal or a completed Web feature. FDC/RDC and forward-only sanitization remain accepted within their recorded limits. The last installed firmware remains signed revision 5 under the earlier hardware evidence; this docs-only review does not rerun hardware.

**Public-repository sanitization — `CLOSED/PUBLISHED_WITH_RECORDED_LIMITS` (2026-10-14).** The owner approved **113/113** scoped public/private publication routes, preserved **36/36** privately archived original documents/scripts with independent source restoration, and selected a prospective-only publication strategy preserving the existing public Git history. Gate-7 closure was published normally at `9819f3188d08a9aff03d3284f15be4d0dd6d9139` with exact-SHA CI SUCCESS. The deleted 58 historical evidence ZIPs are **provenance references, not pending recovery tasks**. Historical workstation-path exposure and third-party replicas remain accepted limitations; this is not a claim that earlier public history was purged. The local Windows encrypted escrow/reopen had a scoped PASS, while independent off-host protected recovery/trust custody remains **`NOT_VERIFIED / OWNER_ACCEPTED_DEFERRED_RISK`**, not a technical PASS. No new signing-key, ZIP-hunt or escrow action is authorized by this closure.

**Sanitization evidence and prior Gate-1/1B/2/3–6 history:** retain the immutable scoped records in `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_FINAL_ACCEPTANCE.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md`, `docs/ROADMAP.md`, `CHANGELOG.md`, and the PRIVATE evidence/source archive. Earlier `OPEN`, `POLICY_PENDING`, partial parity, ZIP-search and custody-check statements were true **at their dated checkpoints**, but are not current program gates. They remain historical rather than erased or silently reinterpreted.

**2026-10-20 — `MANAGEMENT_ONLY_RUNTIME_MIGRATION` Gate 0 ACCEPTED/PUBLISHED; ACTIVE_GATE=GATE1_OFFLINE.** After the owner's conditional approval and a fresh **20/20** cross-document/source design consistency check, the scoped USB ABI/CDC-removal/Host/RPC/stack STOP policy is **OWNER_DESIGN_APPROVED**. The paired `docs/MANAGEMENT_ONLY_RUNTIME_MIGRATION_PLAN.md` and `_ACCEPTANCE_PLAN.md` are **tracked and published scoped contracts** at commit `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09`; exact-SHA [GitHub Host deterministic CI run 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) is **SUCCESS**. Formal `GATE0=ACCEPTED_PUBLISHED`, `ACTIVE_GATE=GATE1_OFFLINE`. Only bounded **offline Gate 1** source/descriptor/Host-negative checks are newly authorized; Gate 2 prior offline proof retains its dated scope. The proposed **private-test** `1209:000E/IF0` firmware is **NOT flashed**, while the board continues to run authenticated rev5 `1209:000C/IF2` composite. The isolated same-source `000E` candidate has actual offline ARM v6 evidence (48,992 B Flash, 7516 B engineering static SRAM, candidate tree `4df48389940e9f821dc7c94045e7473340ef80f4`) and Host v5 Core 87/87 + Transport 45/45. These establish **offline candidate evidence only** and are not physical `000E` USB/stack acceptance or Gate-4 write authorization; the **Gate-0 acceptance itself is the separate exact-SHA docs/owner proof recorded above**.

**2026-10-20 18:37 — Gate-1 offline exact-`000E` descriptor re-audit PASS (scoped; Gate 1 still PARTIAL).** A real Windows operator re-opened the pinned historical ARM v6 and Host v5 evidence archives through a read-only v2 re-audit; independent submitted-evidence SHA-256 `AF831EFE356A6F4FCD4F0CD80601A0C58342F78ED17ECFFA5D0085822CF94BE5`/CRC and 3/3 manifest hashes PASS, reported **22/22** exact-archive/embedded-BIN descriptor checks PASS. Candidate `os.bin` SHA-256 `436B4CDEA1C91DCF4FEF382BCF8BF6546C26ABAF28786AAC300D7BD0DE87EC79` confirms one `1209:000E` vendor IF0 `0x04/0x84` 32-byte configuration, no CDC/IAD in that configuration, BOS 33 B, MS OS 2.0 178 B, WINUSB and Management GUID. The original v5/v6 legacy checksum-index format was **not reparsed**; original whole-ZIP SHA/CRC were checked. **Gate-1 descriptor subcriterion accepted; other Gate-1 source/ELF/Host negative criteria remain OPEN.** Installed physical STM32 is unchanged rev5 `000C/IF2`; `ACTIVE_GATE=GATE1_OFFLINE`, `GATE4=NOT_AUTHORIZED`. Full scope and the original v1 harness failure boundary are in `MANAGEMENT_ONLY_RUNTIME_MIGRATION_ACCEPTANCE_PLAN.md`.

**2026-10-20 20:19 — exact candidate `000E` source recovered; Gate 1 source-contract subproof PASS (still PARTIAL).** Private operator evidence ZIP SHA-256 `0E0BB89696D7DA034780CF5F9D7A527465741FA571ACDCA6AA9DDD31B49BE622` has ZIP CRC PASS and 12/12 file SHA/size matches; nine ARM-v6/Host-v5 candidate source files restored with their exact bytes. Independently recomputed nine Git blob IDs, the 275-file baseline tree and resulting nine-file-patched candidate Git tree **`4df48389940e9f821dc7c94045e7473340ef80f4` PASS**. Candidate-specific source review **18/18 scoped static contract checks PASS** for EP0/PMA/IRQ, CDC-class rejection, legacy 15-field `cdcstat`, Management/UART separation and Host identity/topology guards. Existing Host test suite explicitly rejects IF1/IF3 and wrong PIDs; three residual Gate-1 obligations remain: **full ELF/MAP CDC linked-section exclusion**, **fixed-ID diagnostic adapter/authorization negatives and runtime semantic proof**, and **executable malformed-native-endpoint/descriptor Host negatives**. This is source/ABI evidence, **not** an installed `000E` physical acceptance or permission to modify the hash-pinned candidate without new qualification. Raw source evidence stays private; `ACTIVE_GATE=GATE1_OFFLINE`, `GATE1=PARTIAL`, `GATE4=NOT_AUTHORIZED`. Full detail is in the scoped Management-only acceptance plan.

**2026-10-20 20:56 — Gate-1 ARM v6 ELF/MAP name/section attribution PASS (bounded; checklist remains 13/16).** Actual Windows offline operator run returned `BOUNDED_ARM_V6_LINKER_OBJECT_AND_ELF_SYMBOL_ATTRIBUTION_PASS`; submitted evidence ZIP SHA-256 `35318470976DDD7391CBA1DC8C055B7429730D5891C67B672DF8BE8A20C44BCC` independently verified, CRC and 3/3 member hashes PASS. Pinned `os.elf` SHA `0E9FCEA2580CA552EC5ABDC7646C962C9D7649EDF7190129604B8E1A1573DC0C` and `os.map` SHA `D323E17F2A7AB6CE9DC19B7CD976EA5A2BEAB9F032EE640AA8C140D1F77953D1` analyzed: 32 named linker inputs, 440 defined function/object symbols, no forbidden CDC-named object/section/symbol matches, with USB/kernel anchors present. **Not a complete absence-of-unnamed-CDC-machine-code proof**; Gate-1 linked-code closure remains OPEN, along with fixed-ID adapter/authorization and malformed-descriptor executable Host negatives. `GATE1=PARTIAL`, `ACTIVE_GATE=GATE1_OFFLINE`, `GATE4=NOT_AUTHORIZED`. No target I/O or Git/source mutation by the evidence runner. Details in `MANAGEMENT_ONLY_RUNTIME_MIGRATION_ACCEPTANCE_PLAN.md`.

**2026-10-20 21:56 — Gate-1 Windows Host-only delta native USB topology and fixed-ID negatives PASS, scoped; checklist 14/16.** Real `.NET SDK 10.0.201` locked restore, Release warning-clean build and MTP/xUnit **110/110 PASS (zero skipped)** completed on **offline successor Host-only delta** tree `7e49803eacd1507278101e5c7a9fd8f0afedeeb3`; independent baseline+candidate+four-overlay Git-tree recomputation matched exactly. Private operator evidence ZIP SHA-256 **`A699794B4ADB05B336BA0D0E9DFEC954B11FDA5C5041E682FFB32202E392987F`**, CRC and **11/11** manifest SHA/size pairs PASS. Two reviewed Host validator replacements and two test-project files exercise Linux/Windows malformed native endpoints/descriptors and test-only fixed six RPC IDs; wrong/state-changing IDs reject before mocked transport. Synthetic `cdcstat` parser checks exact 15-field absent-CDC schema/config 0/1. **Host malformed-descriptor negative-test subcriterion accepted for this delta only**; the original nine-source candidate `4df48389940e9f821dc7c94045e7473340ef80f4` is unchanged, the four-file overlay has **not** been applied to `main` or approved as an expanded release scope. Remaining Gate-1 compound checks: **full CDC linked-code/data exclusion**, and **five remaining diagnostic response schemas/authorization/OLED semantics** (real physical `000E` checks belong to authorized Gate 4). `GATE1=PARTIAL (14/16)`, `ACTIVE_GATE=GATE1_OFFLINE`, `GATE4=NOT_AUTHORIZED`. See scoped acceptance plan; no hardware/Git write in operator run.

**2026-10-20 — Gate-1 offline residual-contract preparation; status UNCHANGED at 14/16.** Following accepted Windows v3 Host-only native malformed-descriptor/fixed-ID negative tests (**110/110 PASS**), examined the exact SHA-pinned `000E` candidate `src/kernel.c` emission contracts: `uptime` double-CRLF HEX, `rxstat` 7 fields + ring status, `mspstat` 7 fields + stack status, `schedprod` 31 HEX fields + COOPERATIVE mode + status, `fault` five line groups, and previously fixed 15-field `cdcstat`. A bounded successor **test-only** adapter/parser matrix for complete schemas and malformed negatives is **prepared but NOT Windows-executed**. Its eventual PASS cannot automatically establish on-target `000E` behavior or read-only semantics; Gate 4 remains separately authorized only after Gate 1–3 prerequisites. The remaining independently OPEN Gate-1 predicates are complete unnamed CDC machine-code/data linkage exclusion and full diagnostic adapter/schema/semantics acceptance. Original `000E` firmware and signed rev5 `000C` installed runtime unmodified; Host v3 successor delta remains **unmerged** and outside the owner-frozen nine-file change scope. **`GATE1=PARTIAL (14/16)`, `ACTIVE_GATE=GATE1_OFFLINE`, `GATE2=PARTIAL`, `FULL_GATE3=NOT_ACCEPTED`, `GATE4=NOT_AUTHORIZED`**. Canonical details in the Management-only plan and acceptance checklist.

**Latest bounded baseline observations:** existing-rev5 `000C/IF2` native Linux Management + Windows CH340 UART without CDC passed an exact 10-byte bracket (+10 accepted UART bytes, +0 new errors/drops; evidence SHA-256 `718BBB4C5EC23275E4CA86EA9543C158B647D726BCE27B80C301F7D49601AC65`). Subsequently real read-only Host Core `mspstat/schedprod/fault` PASS demonstrated MSP/console/heartbeat margins **1556/456/432 B**, intact canaries and no active fault record in the snapshot (evidence SHA-256 `2D3C0F8F33552F81FE1B870908C7B733236989912F57EBFD130A558140E44669`). These are **current `000C` physical preflight**, not deployed `000E` tests. Formal Gate-1/2 compound static/negative predicates and Gate-3 recovery/power/custody readiness remain separately classified in the scoped acceptance plan. `FULL_GATE3=NOT_ACCEPTED`, `GATE4=NOT_AUTHORIZED`, `GATE5=NOT_STARTED`. Rev5 local protected-escrow create/reopen PASS and off-host custody owner-accepted deferred risk must not be misreported as current signed-recovery-package acceptance. No Flash/reset/keys/publication is authorized.

## 5. Current development/acceptance topology

Canonical topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

Current bench facts:

- canonical repository/build host: Windows, `<WINDOWS_OPERATOR_PATH>`;
- target native USB + normal VBUS/power owner: Linux USB host (Ubuntu);
- Windows owns ST-LINK/SWD and CH340/UART;
- accepted SWD speed: 950 kHz; read-only fallback ladder `950/480/240/125`;
- ST-LINK 3.3 V and UART-adapter VCC are disconnected;
- physical target power cycle is native-USB VBUS removal/restore on the Linux USB host connection;
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

Both FDC-01..10 and RDC-01..08 are **closed/published**; neither programme is an active pre-feature blocker. The historical Host Management Service Stage-A EXE and later unconsumed `HostManagementRuntime` wrapper proposals were **superseded / NO-GO**, not pending implementation Gates. Their consumer-first successor, `DESKTOP_PUBLISHED_ASSET_INSPECTOR`, has source/Host CI accepted and Ubuntu read-only Gate 3b.1 PASS; **the connected read-only UI fallback Gate 3b was subsequently accepted by operator attestation on 2026-10-16**. A later Web/IPC or new Host consumer needs its own concrete, separately authorized scope and acceptance. The target-firmware resource/profile checkpoint remains conditional on a future STM32-resident source change, not a prerequisite for the existing Host-only UI gate.

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
