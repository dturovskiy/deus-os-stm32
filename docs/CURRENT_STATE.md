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

**CURRENT DESKTOP PRODUCT STATE (2026-10-15): `DESKTOP_PUBLISHED_ASSET_INSPECTOR` — source/Host CI PUBLISHED/PASS; physical UI NOT VERIFIED.** Gate-0 six-doc decision `457be3dc4ff2e642138bd62178d91420ee552570` passed hosted CI `38746868482`. The **actual three-path** Desktop/Core test feature is published as `4729341e732ffd61e7fe2b0e48973f2c98726a5d`, exact-SHA [GitHub CI 38748957900](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38748957900) SUCCESS: Core **87/87** including **three new synthetic tests**, Transport **24/24**, 0 failed/skipped, Desktop solution Release 0 warnings/errors, hygiene 275. New Desktop UI offers a **manual, capability-gated published Asset Status/OLED layout read**, validated CRC/layout and compiled-default fallback, with clearing on lost/recovering sessions. Source uses existing `ManagementServiceOperations` and `DeusDeviceSession`; no `HostManagementRuntime`, network listener, firmware/bootloader, Flash/SRAM or private-material change. Local .NET runner reported `CONTROL_PLANE_OUTCOME_AMBIGUOUS` without compiler output (neither local PASS nor FAIL); hosted CI is the actual source compilation/test proof. **Connected Desktop UI/physical target observation has not been performed**; do not call this end-to-end hardware/UI PASS. Paired scope/evidence: `docs/DESKTOP_PUBLISHED_ASSET_INSPECTOR_PLAN.md` and `_ACCEPTANCE_PLAN.md`. Previous architecture review's speculative-wrapper NO-GO remains in force, and Web/HTTP remains future separately gated work.

**Current product direction: Host Management Service / local management UI.** Architecture re-review C (2026-10-15) independently compared the accepted Core API with Desktop and CLI and classified the proposed unconsumed `HostManagementRuntime` as **`NO_GO_UNCONSUMED_RUNTIME_WRAPPER`**: `DesktopController` already combines discovery, selected session, typed commands and teardown; `ManagementServiceOperations` already supplies the 5 typed ReadOnly and 3 trusted Control operations. The previous nine-file service EXE and subsequent two-file wrapper proposals are **withdrawn from future implementation scope**, not retroactively un-published or mislabeled product successes. **No new Host feature code is implemented.** The next authorized activity is a new, explicitly selected **concrete consumer/user-facing feature** with its own acceptance (existing Desktop enhancement or locally secured Web host), preferably using current Core directly; no speculative runtime or lockfile. Core's disposal cannot guarantee a deadline under hung native I/O, recovery locator is not authenticated board identity, and concurrent discovery/selection needs consumer-level policy. Current canon: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md` and `_ACCEPTANCE_PLAN.md`; original Gate-0 docs `bfb30d5`, Amendment A `0f95c2e`, Correction B `18e5037` and status sync `bdc5cdf` remain truthful historical docs-only PASS. This review has no STM32/Host source/bootloader/key/Flash/SRAM changes; exact publication/CI is separately required.

**RDC-08 final closure adjudication — event-bound:** Gate-0 docs scope was published at `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`, Gate-1 exhaustive 113-Markdown classification at `c646f3e3ac342cbaef025d115e5be96bd49cf140`, and Gates 2–7 scoped documentation reconciliation/acceptance at `faabfe75bbb3b805548092c9f298ac862c38ef1d` with GitHub Actions [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) **SUCCESS**, Core `84/84`, Transport `24/24`, Release 0 warnings/errors and zero failed/skipped. Its final Gate-8 closure record is the publication of this documentation revision. **That Gate-8 predicate has been fulfilled:** final docs-only commit `c309428b03615a9afd514e3c7c8886a82a5112e9` was published non-force, independently fresh-fetched clean `0/0`, and the exact-SHA [GitHub Actions run 38079465787](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079465787) finished SUCCESS. Therefore **RDC-08 and all eight RDC items are CLOSED/PUBLISHED**, as independently recorded in `docs/MASTER_EXECUTION_CHECKLIST.md`. The documented result is evidence-conditioned, not an unchecked future claim or a hardcoded live HEAD. No product-feature boundary is automatically activated.

**Current accepted physical STM32 state (RDC-06 hardware acceptance, 2026-10-10):** the board runs authenticated **application firmware security revision 5**; application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, accepted application source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. An independent full 64-KiB HOTPLUG Flash readback verified authenticated version floor 5 and unchanged bootloader/persistence ownership. The separately published STM32 USB EPnR CTR race repair is `83e57f609bab4bcc8a61ea2222629bf9066e910d`; the corrected bootloader passed 72/72 INFO USB regression and the signed revision-5 update. USB/RPC, scheduler, IWDG, Windows CH340/USART1 `115200 8N1` command help and physical OLED were accepted, with operator `PHYSICAL_OLED=PASS`. Hardware evidence SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD` is historical accepted proof. The numeral **5 denotes signed firmware-update security revision, not an OS semantic version**. Gate-0/Gate-1/Gate-2 documentation work did **not** reflash, reset, or freshly revalidate the MCU.

RDC-07 bootloader readability normalization is CLOSED/PUBLISHED at `7846fa48429d00233116438821acc0ea2a0b38be`: byte-identical **synthetic-key/nondeployable** ARM build equivalence, no change to production key or installed firmware. Its full Gate-2 evidence and exact equivalence conditions are retained in `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md`.

**Host presentation scope:** `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` remains a non-authorizing **future Web/browser concept**, although the general Host-management product direction has been explicitly selected. The present Architecture Review C records `NO_GO` for the unconsumed Core runtime proposal, not cancellation of the product goal or a completed Web feature. FDC/RDC and forward-only sanitization remain accepted within their recorded limits. The last installed firmware remains signed revision 5 under the earlier hardware evidence; this docs-only review does not rerun hardware.

**Historical sanitization entry checkpoint (before 2026-10-14 final owner disposition): `PUBLIC_REPOSITORY_SANITIZATION` was then active, with Gate-1 exit accepted with limits and Gate-2 preservation OPEN.** The first-pass 254-path read-only audit was accepted as a published evidence snapshot at `536fcac17cf854fb33cf2fcbf693623ff57ecd6a`, with exact-SHA [CI 38084119171](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38084119171) SUCCESS. **This early Gate-1A checkpoint was not final acceptance; the later independently published Gate-1 exit audit establishes the accepted bounded classification result below.** The independent re-audit (`docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_INDEPENDENT_REAUDIT.md`) corrected an overinterpretation of Markdown citation counts; it did not certify active consumers or authorize de-publication. Gate-0 bounded design was accepted/published at `558306fe618d1ee69240fa11d24437cd38fb8e2b`, exact-SHA hosted [CI 38080940176](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38080940176) SUCCESS. Previous FDC/RDC hygiene checked obvious build/secret artifacts and documentation drift, **not** which internal operator/evidence material should be publicly distributed or how old Git history should be managed. Canonical contracts: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` and `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`. The separately authorized archive-first documentation redactions and seven-script portability edits have since been completed and boundedly accepted as described below. **No further** public file removal, unrestricted archive transfer, Git-history rewriting, repository visibility change, firmware/Flash/key/target manipulation or automatic new product feature is authorized.

**Gate-1B partial evidence (not acceptance):** `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_EVIDENCE.md` inventories all 98 still-unresolved `PRIVATE`/`RETAIN` candidates against the rechecked 258-path pre-publication baseline, corrects audit-report self-citation effects and records bounded 143-commit patch, 10 historic binary-diff blob, include/project-link and script-default verification. Final per-file owner decisions, external history hosting/replica coverage, private preservation and executable recovery proofs are not complete. A later bounded read-only tool verified all 1,367/1,367 blob contents (30,317,374 bytes, 0 unreadable/truncated) reachable from published `95d43fb7e5a6cd5c0d024a97c3e2f0e925c8cb84`; recognized credential-pattern matches were 0, but historical operator-path occurrences remain. `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_RECONCILIATION.md` reconciles the five additional published audit docs and two subsequently published audit paths with the immutable 254-path matrix (projected 261-path inventory), without owner publication approvals. **Gate-1 and Gate-1B remain OPEN; Gate-2 NOT AUTHORIZED.** The read-only Gate-1B evidence slice was independently published as docs-only commit `3c0c5343ab66f2757702a2c41d75c869102c19f7` and accepted by exact-SHA hosted [CI 38088050242](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38088050242) SUCCESS (Core 84/84, Transport 24/24, Release 0 warnings/errors, hygiene 259). This is evidence acceptance only, not final per-file privacy approval.

**Public repository docs-only redaction candidate (not full sanitization):** The original operator-bearing documents have been preserved in a verified private-visibility, docs-only archive (19/19 original blob identities). Thirteen existing Markdown paths have been sanitized in place for local path/SSH identity, all public link targets remain present, and the 122-file Markdown screen found no remaining instances of the four identified operator-literal signatures. Bounded proof and limitations: `docs/PUBLIC_REPOSITORY_SANITIZATION_DOCS_ONLY_ACCEPTANCE.md`. At this earlier docs-only checkpoint, seven executable PowerShell defaults remained untouched; they were subsequently preserved privately and portabilized under the accepted scoped evidence below. Public Git history/caches still retain earlier versions. **This docs-only evidence slice is now published/technically accepted at `c55c90c4b40605adc2e3da26a7314467c618c512`; hosted exact-SHA [CI 38140063373](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38140063373) SUCCESS (Core 84/84, Transport 24/24, Release 0 warnings/errors, hygiene 263). Gate-1 remains OPEN; no source/target authority was granted.**

**Second documentation-only operator-reference minimization — PUBLISHED/TECHNICALLY ACCEPTED (not full sanitization):** Public commit `3a5ccfa7db2c156e8a4b396f0837e89a492c6f13` changed 26 Markdown files only, retained physical USB/UART/SWD execution-domain contracts, and was accepted by exact-SHA [GitHub Actions run 38145500397](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38145500397) SUCCESS (Core 84/84, Transport 24/24, Release 0 warnings/errors, tracked-output hygiene 263). A subsequent narrowly scoped IWDG documentation correction is supported by its separately published byte-identical private original at `18d02e4fd2bb0d24c47b5f4a027ebc0b5c8e3970`, bringing selected original preservation to 29 Markdown documents. That IWDG follow-up was independently published in docs-only commit `74a998f2148ad28480ed66825e5ef53fe96de78a` and accepted by exact-SHA [CI 38146560688](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38146560688) SUCCESS (Core 84/84, Transport 24/24, Release 0 warnings/errors, hygiene 263). No executable PowerShell/source/CI changes, hardware operations or Git-history rewriting were authorized or performed. The Gate-1/1B owner-policy and history/external replica decisions remain OPEN; Gate-2 remains BLOCKED. See `docs/PUBLIC_REPOSITORY_SANITIZATION_DOCS_ONLY_ACCEPTANCE.md` for the bounded evidence.

**Gate-1B proposed owner decisions — not signed:** `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md` details 113/113 original PRIVATE/RETAIN/REDACT file routes. Proposed outcome preserves all current public filenames, retains 90 existing scoped documents unchanged, and reserves 23 for specific archive-first safety/editorial/portable-script review (including seven PowerShell tools). The historic matrix and signed product evidence are not moved. The decision-proposal publication itself was independently accepted at `760a1177f1d7e304421d2f1b6e70dfd0d02e5f80`, hosted exact-SHA [CI 38137514271](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38137514271) SUCCESS (Core 84/84, Transport 24/24, Release zero warnings/errors, tracked-output hygiene 262). Approval of publication policy, independent private preservation, historical exposure treatment and executable script equivalence is still required; Gate-1B/1 OPEN, Gate-2 BLOCKED.

**Gate-1B technical next-boundary preflight (non-authorizing):** `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_SCRIPT_PORTABILITY_AUDIT.md` fingerprints the seven PowerShell workstation-bound default-path consumers, identifies two additional executable dependencies, and independently reconfirms **29/29 selected private documentary originals** against public pre-edit commits. The static audit does **not** prove portable Windows execution, recovery-bundle hash equivalence, independent remote archive restore or external-history privacy; all owner and history/visibility decisions remain OPEN. No source code/PowerShell/CI/STM32 changes or Gate-2 promotion are authorized by this scoped evidence.

**Gate-1B seven-script portability — SCOPED WINDOWS OFFLINE PASS / FULL PARITY OPEN:** the seven original PowerShell files were privately preserved at `4cf131f1ca15a2c686b6153cbf5cf6399a96a799` alongside 29 Markdown originals. Source candidate `c071e4e17345a4121f7bdebd46cd743d873b17ea` passed exact-SHA Windows PowerShell 7 parser and mock resolver [run 38150031027](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38150031027) and Host [CI 38150031090](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38150031090); follow-up `c01f7d9e76408bb7fb17f98c38dbefc462f82b20` passed Windows offline historical Asset recovery bundle fixture (seven files/hashes, 54-KiB region) [run 38150522872](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38150522872) and Host [CI 38150522887](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38150522887): Core 84/84, Transport 24/24, Release 0 warnings/errors. Independent private local `git clone --no-local` restored and checked **36/36** originals in private verification commit `fd5a3c09f206fd52e5197c8562f2e62b72b071e1`. Actual Windows ARM/bootloader/recovery binary and physical target parity, private GitHub remote restore/denial, owner policy and old history remain NOT ACCEPTED. Gate-1B/1 OPEN, Gate-2 BLOCKED. No hardware/keys/history changes. Detail: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_SCRIPT_PORTABILITY_AUDIT.md`.

**PowerShell tool-path failure diagnostic follow-up — SCOPED ACCEPTED:** public code correction `e44ae91919a9598f5c15c72f10a6abb9a67d63a0` replaced five error strings that could otherwise include absolute operator tool paths in stderr. Exact-SHA [Windows offline 38151414436](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38151414436) and [Host CI 38151414438](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38151414438) both SUCCESS (seven scripts/four wrappers parse, eight resolver mock cases, seven synthetic Asset hashes, Core 84/84, Transport 24/24, Release 0 warnings/errors, hygiene 267). This is not real Windows ARM/bootloader/physical Flash acceptance. Private Git remains separately preserved and Gate-1B/1 OPEN, Gate-2 BLOCKED.

**DEUS Runner v1.2.0 operator evidence — NEW SCOPED PASS (2026-10-11):** independently validated evidence ZIP SHA-256 `5DA72297853B7925D26164268E2992FC53A6EE49532B6615E570029966AC0B22`, 179/179 payload manifest hashes, clean public prestate SHA `cfb176afd0ac333011746ddd74962c1e32dd7446`. Prior nondeployable Windows ARM binary hashes rechecked without recompilation; current-source synthetic bootloader recovery bundle generated and `VALIDATE_ONLY` passed 10/10 file hashes with no hardware. Actual authenticated PRIVATE GitHub fresh clone on Windows restored and matched 36/36 original Git blobs at private `fd5a3c09f206fd52e5197c8562f2e62b72b071e1`. At the time of this scoped operator diagnostic the historic acceptance recovery replay, anonymous-access evidence and grouped owner approval were not newly proven by that operator ZIP; subsequent independently accepted proof and owner decisions are recorded below (not retroactively part of that ZIP). Gate-1B/1 OPEN; Gate-2 BLOCKED. See `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_SCRIPT_PORTABILITY_AUDIT.md`.

**Gate-1B owner historical-path policy decision (2026-10-11):** the operator selected `ACCEPT_RESIDUAL_HISTORY` for the known old workstation/tool-path exposure and approved **strategy A: prospective-only public cleanup while preserving the existing Git history/commit SHA**. This was informed by a read-only current-main audit of 1,520/1,520 reachable blobs with zero matches against eight recognized credential-pattern classes, while 157 Windows-path-bearing and 21 Unix-home-path-bearing historic blobs remain. This is **not** proof that unknown secrets or third-party historic clones do not exist. At this earlier historical decision point the 113 file-disposition decisions remained `POLICY_PENDING`; **the owner has subsequently approved 113/113 as a group**, as recorded in the later Gate-1B paragraph below. The operator-run GitHub REST `200/404/404` anonymous-denial check is a scoped operator-observed PASS. Gate-1 final remains OPEN and Gate-2 BLOCKED. No history rewrite, removal, private artifact publication, firmware or target action is authorized by this policy selection.

**Gate-1B scoped anonymous-access evidence (2026-10-11):** on Windows PowerShell 7, the operator executed a GitHub API REST check with only explicit `Accept` and `User-Agent` headers and no GitHub authentication arguments. Observed `PUBLIC_HTTP=200`, `PRIVATE_REPO_HTTP=404`, `PRIVATE_FILE_HTTP=404`, `RESULT=PASS`. Therefore the concrete PRIVATE GitHub repo/file endpoints satisfy the expected **operator-observed anonymous denial** while the authenticated remote and independent local byte-restore checks remain 36/36 PASS. This is reported operator evidence, not a new direct packet-level/proxy-authentication audit or verification of unrelated clones/caches. **At this earlier HTTP-check checkpoint the 113-path policy was unsigned; its subsequent explicit owner approval is separately recorded below. Gate-1 final OPEN; Gate-2 BLOCKED.** See `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md`.

**Gate-1B grouped owner publication and private-retention policy — APPROVED (2026-10-11):** the owner explicitly approved the existing 113-row disposition register **113/113 `OWNER_APPROVED_2026_10_11`**: 90 historical PRIVATE/RETAIN paths to keep public, 10 archive-first edited Markdown sources, seven technically accepted portable PowerShell tools, and six editorial-reviewed documents without required operator-value substitutions. Every one of the 23 reviewed/edited paths has a preserved original in the separate private archive. The owner approved keeping all **29 Markdown + 7 PowerShell = 36 originals** in the existing authenticated GitHub-`PRIVATE` repository, with provenance intact and no access/retention deletion until a separate explicit decision; authenticated GitHub remote restore 36/36 and independent offline restore 36/36 remain PASS, and the scoped operator-observed anonymous GitHub REST denial is 200/404/404 PASS. **Policy signoff is real, but not a new firmware, production ARM, live recovery, external-cache/clone or complete Gate-2 archive proof.** Historic Git SHA preserved under `ACCEPT_RESIDUAL_HISTORY`. **`GATE1B_GROUPED_POLICY_SIGNOFF=PASS`, Gate-1 final remains OPEN for scoped remaining host/external and preservation acceptance, Gate-2 BLOCKED.** Canonical matrix/limits: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md`.

**PUBLIC_REPOSITORY_SANITIZATION Gate-1 independent exit audit — ACCEPTED/PUBLISHED `53642b0c529925005f3b09a4882b8c8c115bbe54`, exact-SHA [CI 38175961331](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38175961331) SUCCESS:** `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_EXIT_AUDIT.md` reconciles the frozen 254-path matrix plus 13 later published files (267 tracked paths, zero removals), 113/113 explicitly approved source dispositions, 1,542/1,542 reachable `main` blob checks with zero selected credential signatures, all 56 current Actions artifact collections (zero stored artifacts), 55/56 retrievable job logs (zero selected credential patterns), scoped GitHub public/private API denial and 36/36 private original byte restores. One historic failed job log, expired artifacts, caches and third-party clones/mirrors remain `NOT_CHECKED`; known historical operator paths are owner-accepted only within `ACCEPT_RESIDUAL_HISTORY`. **The Gate-1 source/exposure classification verdict `GATE1_EXIT=PASS_WITH_RECORDED_LIMITS` is ACCEPTED/PUBLISHED, not full sanitized-release closure.** The ordinary non-force audit publication at `53642b0c529925005f3b09a4882b8c8c115bbe54` passed hosted exact-SHA [CI 38175961331](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38175961331): Core 84/84, Transport 24/24, Release 0 warnings/errors, hygiene 268 PASS. **Gate-2 not automatically activated**, no full engineering/archive provenance, product binary parity or hardware proof is claimed.

**Gate-2 preservation checkpoint — SCOPED PRIVATE PUBLICATION (2026-10-12):** The original public `main` history is privately archived and independently restored (frozen public `8292bae1ff991630b1f92502b67c77b6e894b6da`: 172 reachable commits, 268 tracked files), together with **36/36** historically anchored original documents/scripts. A separate metadata-only registry covers **58** historical evidence ZIP references and nine provisional document-hash priority anchors; it does **not** contain original ZIP bytes or establish their recoverability. Full source-history archive, manifest, verification scripts and evidence registry have been published to the existing access-controlled PRIVATE source archive; its scoped preservation handoff is recorded at private commit `64efce861f4e2a94a4b6f70ab677d5b9d5f00352` (`evidence_registry/GATE2_PRESERVATION_CHECKPOINT_2026-10-12.md`). **`GATE2=OPEN`:** accepted original evidence ZIP retrieval and independently attested production-recovery/trust-material custody remain unproven; Gate-3–7 not opened. No public source/ABI/CI, Git-history rewrite, signing-key transfer, Flash or STM32 operation was performed by these archival sublanes. Earlier Gate-1B OPEN/BLOCKED statements above are historical checkpoint classifications, superseded for current ordering by the accepted Gate-1 exit and this limited Gate-2 progress.

**Gate-2 current-state scope correction — owner directive (2026-10-14):** The 58 historical evidence ZIP names and nine provisional SHA anchors are retained as **legacy provenance only**. Their original ZIPs were deleted by the owner and their recovery is **not an active preservation requirement**; prior archival-search follow-ups are superseded. Use the latest independently locked Git candidate/source/ABI/acceptance contracts as the normal current-state proof basis and obtain new, separate, candidate-bound test results only when a current acceptance need actually requires them. For present-tense STM32 measurements, use only separately scoped read-only tests on the host that owns each hardware interface; do not equate a new Git `HEAD` with the last installed application revision 5 without a matching hardware observation. Existing source history and 36 original private docs/scripts were already verified as frozen source provenance. Restricted *current* production recovery/trust custody and current-source archive/restore still require their own evidence; no default raw Flash/key-bearing bootloader export. **`GATE2=OPEN`, Gate-3–7 NOT AUTHORIZED.** This paragraph supersedes the former requirement to recover archived historical evidence ZIP bytes in the 2026-10-12 snapshot above, but does not claim that those bytes were ever preserved or independently verified.

**2026-10-14 owner stop / forward-only continuation (newest maintenance decision):** The owner directed an end to additional recovery-key/escrow work and a return to the existing sanitization sequence. Actual private source-archive/restore proof remains PASS; the existing local Windows escrow has a scoped accepted creation/reopen PASS, while independent off-host custody/restore is **NOT_VERIFIED** and explicitly **OWNER_ACCEPTED_DEFERRED_RISK**, not fabricated as a technical PASS. For **non-destructive prospective-only public hygiene only**, Gate-2 has owner disposition `OWNER_CLOSED_WITH_RECORDED_LIMITS`; the previously approved `ACCEPT_RESIDUAL_HISTORY` choice freezes Gate-3 **option C (forward-only existing public repository)** with the exact next candidate restricted to `.gitignore` and `.github/workflows/ci.yml` plus necessary scoped documentation. Source/bootloader/firmware, keys, device, private archive originals, existing Git history/visibility and force-push are out of scope. This owner exception is not transferable to a later security/release gate. Canonical scope/evidence: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE2_3_FORWARD_ONLY_OWNER_DECISION.md`. The earlier 2026-10-14 paragraph above remains a true *pre-owner-exception* checkpoint; it no longer prohibits the recorded forward-only maintenance path.

**Historical Gate-3/4 forward-only hygiene publication checkpoint (2026-10-14; Gate-5/6/7 completed later):** owner-limited Gate-2/strategy decision was published in docs-only commit `7340c5dfb4cad8e0ae0381bc076f7ad68eee3cdb`, with exact-SHA hosted CI run `38626707694` **SUCCESS**. Gate-4's two-path implementation (`.gitignore`, `.github/workflows/ci.yml`) was separately published by non-force commit `85704c521d247235585304ca264826582b821fdd` and accepted by exact-SHA hosted CI run `38627081404` **SUCCESS**; its CI includes synthetic allow/deny fixtures and checks all tracked path names. Both publications were verified against clean upstream Git. **At this Gate-4 checkpoint, the next step was Gate-5; later Gate-5/6/7 proofs supersede this historical NEXT.** No new firmware feature or physical acceptance is activated.

**Historical Gate-5 independent public-release regression checkpoint (2026-10-14; later Gate-6/7 were accepted):** accepted `PASS_WITH_RECORDED_LIMITS` on published commit `2aae249890a5570fcb4a1dd341ba82c4250dc56e`: independent isolated public/private local Git clones and fsck; 269 tracked, 7/7 local Markdown links, exact eight-path docs/hygiene delta, zero forbidden tracked artifacts, current history scan 1,574/1,574 blobs, 36/36 private originals, retained 175-commit source-bundle restore, and actual hosted exact-SHA CI `38627451883` Core 84/84 + Transport 24/24 with zero failures/skips and Release 0 warnings/errors. Limits: **no fresh anonymous remote clone**, unobservable external history copies remain unverified. This gate leaves firmware/Host product code, hardware and prior owner policy untouched. Public-safe acceptance: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE5_INDEPENDENT_ACCEPTANCE.md`. **At the Gate-5 checkpoint Gate-6/7 were still ahead; both subsequently passed and were published.**

**Final public-repository sanitization reconciliation (2026-10-14):** `PUBLIC_REPOSITORY_SANITIZATION=CLOSED/PUBLISHED_WITH_RECORDED_LIMITS` under the approved prospective-only history-preserving policy. Gate-5 was independently validated; Gate-6 published public Gate-5 proof in normal non-force commit `1772d37d3a706d0d2f64a4e09f5810cee46ef76e`, verified clean against fetched `origin/main`, exact-SHA hosted [CI 38629097010](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38629097010) SUCCESS (Core 84/84, Transport 24/24, 0 failed/skipped, Release 0 warnings/errors, 270 tracked-output hygiene PASS). Final Gate-7 acceptance `docs/PUBLIC_REPOSITORY_SANITIZATION_FINAL_ACCEPTANCE.md` records explicit limited status, non-erasure of old Git history and preserved private provenance; final Gate-7 commit `9819f3188d08a9aff03d3284f15be4d0dd6d9139` **was independently confirmed** after ordinary non-force publication, fresh-fetch clean `0/0` and exact-SHA hosted CI `38629887654` SUCCESS (Core 84/84, Transport 24/24, zero failed/skipped, Release 0 warnings/errors, tracked-output hygiene 271 PASS). Maintenance closure **does not activate a product feature**. Next work requires a separately frozen feature/acceptance choice.

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

Both FDC-01..10 and RDC-01..08 are **closed/published**; neither programme is an active pre-feature blocker. The selected Host Management Service Stage-A Gate-0 candidate is a **new product-feature design**, not a reopening of those debts. Its plan/acceptance and exact-SHA publication predicate govern whether implementation may begin. The target-Firmware resource/profile backlog remains deferred until a real STM32-resident consumer.

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
