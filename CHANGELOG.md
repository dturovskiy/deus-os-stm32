## 2026-10-10

### Public Repository Sanitization — full reachable-main blob audit and current-tree reconciliation (non-authorizing)

Added read-only, aggregate-only `scripts/public_repository_history_audit.py`: successfully read **1,367/1,367 reachable `main` Git blobs** (30,317,374 bytes) at `95d43fb7e5a6cd5c0d024a97c3e2f0e925c8cb84` with no unreadable/truncated objects, 0 selected recognized credential-signature matches, and historic workstation/home path-pattern exposure still present. `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_RECONCILIATION.md` reconciles five later published audit reports and two new scoped paths with the immutable 254-row baseline, for a predicted 261-path candidate inventory. **This closes the scoped blob-read and file-accounting technical audit backlog, not the 98 owner decisions, 15 redaction specs, inaccessible GitHub copies/caches, or Gate-1 final acceptance.** No existing recovery/build PowerShell was modified, no keys/hardware touched and no history rewritten.

### Public Repository Sanitization — Gate-1B independent read-only evidence (partial)

Created `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_EVIDENCE.md` on verified prestate `300747134dbcd9e72687192b6e81fa9a72f995d9`, with 98/98 unresolved owner/consumer review rows and self-reference-filtered counts (14 PRIVATE and 6 RETAIN governance-only). Recorded 143/143 complete public-main patch-text readings, ten individually fetched historical binary-diff versions of `CHANGELOG.md`, 94 resolved local source includes, 19 resolved Host project/solution references and seven PowerShell default-path cases. No source, scripts, keys, hardware, current public content or Git history were sanitized. **Gate-1B and parent Gate-1 remain OPEN; Gate-2 not authorized.** The exact seven-Markdown evidence commit `3c0c5343ab66f2757702a2c41d75c869102c19f7` was published by normal non-force push; fresh fetch clean `HEAD == origin/main == FETCH_HEAD`, `0/0`. Exact-SHA hosted [run 38088050242](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38088050242) succeeded with Core 84/84, Transport 24/24, zero Release warnings/errors and tracked-output hygiene 259. This accepts only the bounded Gate-1B evidence publication, not the unresolved sanitization Gate.

### Public Repository Sanitization — independent Gate-1A re-audit

A read-only independent recheck confirmed the frozen 254-row matrix counts and verified the subsequent 257-path current tree, while identifying an interpretive flaw: **115/115 Markdown names cited elsewhere is not 115 functional dependencies**. Excluding seven governance/ledger documents, **20 of the 115 baseline Markdown sources were cited only within that excluded group**. `PUBLIC_REPOSITORY_SANITIZATION_GATE1_INDEPENDENT_REAUDIT.md` records the methodology, signature-scan limits and unresolved full-history/hosting/dependency coverage. Corrected the cited-count interpretation, stale Gate-1 NEXT status labels and checklist nesting in a docs-only reconciliation. **Gate-1 final acceptance remains OPEN**; no public file deletion, private archive creation, Git rewrite, source/target mutation or security clearance is claimed.

### Public Repository Sanitization — Gate-1A audit snapshot PUBLISHED `536fcac17cf854fb33cf2fcbf693623ff57ecd6a`

The read-only ten-Markdown Gate-1A candidate (including three scoped evidence files) was normally committed and pushed non-force. Fresh fetch proved `HEAD == origin/main == FETCH_HEAD == 536fcac17cf854fb33cf2fcbf693623ff57ecd6a` clean `0/0`. Hosted exact-commit [CI 38084119171](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38084119171) SUCCESS: Release zero warnings/errors, Core 84/84, Transport 24/24, zero failed/skipped, 257 tracked paths hygiene PASS. **This accepts only a bounded current-tree audit snapshot; Gate-1 final acceptance is OPEN with unresolved `RETAIN`, history/cloud proof limitations.** No source/target/data sanitization or repository-history operation.

### Public Repository Sanitization — Gate-1 read-only current-tree, GitHub/history and dependency first pass

From clean published baseline `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`, all **254 current tracked file contents** were read completely, anchored to Git blob IDs and categorized provisionally as `141 PUBLIC / 15 REDACT / 49 PRIVATE / 49 RETAIN`. Separate scoped reports capture bounded **140-main-commit** history/ref review, **22 Actions runs** (0 stored artifacts across 22 queries, 21 readable job logs), and **254-file textual cross-reference** dependencies. Prior current-file path redaction does not erase historical Git blobs. No confirmed production key leakage follows from these bounded checks. Full history blob/secret scanning, certain GitHub/cloud/clone surfaces and the 49 unresolved `RETAIN` consumers block final Gate-1 acceptance. These are three new safe audit artifacts plus narrow status references only; **no files sanitized and no Git-history, source, firmware, test harness, recovery key or physical device mutation**.

### Public Repository Sanitization — independent maintenance Gate-0 design candidate

The next requested maintenance direction is a **public/private release-view and history-exposure audit**, distinct from the completed FDC-10/RDC-08 documentation consistency and tracked-artifact checks. An initial read-only review of all 83 scoped plan/acceptance documents and public GitHub visibility found no pre-existing per-file sanitization boundary. New `PUBLIC_REPOSITORY_SANITIZATION` plan/acceptance documents freeze decision categories, private provenance preservation, history/hosting coverage and approval requirements. **No deletions, Git-history rewrite, repository visibility change, firmware/source mutation or key/target I/O are authorized.** Gate-0 design was subsequently published docs-only at `558306fe618d1ee69240fa11d24437cd38fb8e2b` by ordinary non-force push; independent fresh fetch returned clean `HEAD == origin/main == FETCH_HEAD`, ahead/behind `0/0`. Hosted GitHub Actions [run 38080940176](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38080940176) exact-SHA SUCCESS (Core `84/84`, Transport `24/24`, Release 0 warnings/errors, 0 failed/skipped, hygiene PASS). Gate-1 **read-only** item-by-item exposure/consumer inventory is next; **no file has been sanitized yet**.

### RDC-08 final documentation closure — GATE-8 release predicate; Gates 0–7 ACCEPTED/PUBLISHED

Gate-1 exhaustive 113-Markdown classification and six-path content allowlist were published at `c646f3e3ac342cbaef025d115e5be96bd49cf140` with hosted CI `38078396770` SUCCESS. Gates 2–5 accepted lossless current-state simplification, FDC-10 vs RDC-08 inventory classification, RDC-07 historical marker correction, a 30-checkbox audit, link/pair/hygiene and six exact content paths plus five status mirrors. Gate-6 local docs-only commit `faabfe75bbb3b805548092c9f298ac862c38ef1d` (11 Markdown paths, no executable/target changes) was published non-force in Gate-7; fresh fetch clean `HEAD == origin/main == FETCH_HEAD`, ahead/behind `0/0`. Hosted GitHub Actions [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) on its exact SHA completed SUCCESS with Release zero warnings/errors, Core `84/84`, Transport `24/24`, 0 failed/skipped and tracked/non-vacuous hygiene PASS. Historical signed application revision 5, production bootloader, real keys, Flash and USB physical topology remain untouched.

**Gate-8 outcome is governed by an externally checked predicate, not a premature statement:** the final closure-documentation commit must be published ordinary non-force, independently fresh-fetched clean `0/0`, and its own exact Git SHA must have successful hosted Release/Core/Transport/hygiene CI. With those facts verified, `RDC-08` and the whole `RESIDUAL_DEBT_CLOSURE_PROGRAM` are CLOSED/PUBLISHED; without them they remain OPEN. Feature *selection* then becomes eligible, but no next feature is automatically promoted.

### RDC-08 documentation/source-of-truth closure — GATE-0 DESIGN ACCEPTED / PUBLISHED `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`

The initial clean parent `80ccdf5534ac65a830dcec5cc022e916521d2fb4` was audited through the local Windows-backed DEUS-MCP repository view: 250 tracked files, including all 108 `docs/*.md`, plus tracked source/Host/bootloader/tooling and Git-history path-name hygiene. Confirmed RDC-08 documentation work is limited to current-state history duplication, a new correctly labelled inventory snapshot, historical/current claim classification and final documentation/security hygiene. Exactly two new scoped Gate-0 plan/acceptance Markdown files were staged and committed as `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2` with whitespace PASS; ordinary non-force push/fresh-fetch gave `HEAD == origin/main == FETCH_HEAD`, clean `0/0`. Exact-SHA GitHub-hosted [run 38075603101](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38075603101) completed success: Host Release 0 warnings/errors, Core 84/84, Transport 24/24, 0 failed/skipped, tracked-output and non-vacuous diff hygiene PASS. No source, firmware, private key, Flash or hardware access occurred. Gate-1 exhaustive 113-Markdown claim classification was subsequently accepted/published docs-only at `c646f3e3ac342cbaef025d115e5be96bd49cf140`; GitHub Actions [run 38078396770](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38078396770) SUCCESS (Core `84/84`, Transport `24/24`, zero failed/skipped, warning-clean Release and static hygiene). Its exact six-file Gate-2 content scope is frozen. The later documentation/hygiene candidate is being reviewed without firmware or target mutation. **RDC-08 and the RDC program remain OPEN, and product features blocked**.

### RDC-07 bootloader readability normalization — ACCEPTED / CODE PUBLISHED `7846fa48429d00233116438821acc0ea2a0b38be`

The frozen Gate-0 plan and acceptance were published at `7f5d14890d4909749f4d61009d3d6ab01fb6bd78`; acceptance was recorded at `d824716634e3292c587c438d328d19f5f1867bd6`, and the Windows-local WSL/isolated-output amendment at `10a930cdb75cfc7ba2ac41dbc746109e55080267`. The only product source changed by Gate-1 is `bootloader/bootloader.c`: the existing 716-line, 37,177-byte baseline Git blob `39bbbc0ed7bb8dbc307f57364c090e9d256dc503` was formatted as the 1,244-line, 40,291-byte candidate blob `ed34ad98f6dcf48c5175c3b53a82d8144e653bee`. C significant tokens, preprocessor, string/char literals, comments, MMIO, EPnR CTR preservation, Flash/metadata/authentication/handoff behavior and dependency source blobs remained identical.

The operator-executed Windows ARM GNU GCC 15.3.1 Gate-2 evidence ZIP SHA-256 `9208C02ACCC956188E32CBAD68740AC46F0EB5E486C9849C7372B898366EB768` passed ZIP CRC, all 3 internal SHA entries and 38/38 fact-to-log agreement. Two independently materialized pristine source/build trees and initially empty outputs, both using one randomly generated **synthetic nondeployable** 32-byte key, passed warning-clean ARM builds. Complete raw BIN matched **byte-for-byte**: both 5,996 bytes, SHA-256 `7EDCD55CED5DDA529EFF173CF1ABB676E7892B1A5824B6EE4695A7BA3EC796BE`; 8,267 significant token entries, defined symbols, normalized disassembly, 32 stack-usage records and linked Flash `5996` / BSS `660` / MSP `1024` / conventional SRAM `1684` were identical and within accepted ceilings. These are **synthetic-key test artifacts**, not a production bootloader image, and no key-bearing files were published.

Gate-3 independent source/ABI/resource/no-flash acceptance PASS; **no STM32 target I/O**, SWD/USB reset, production-key access or reflash occurred. Gate-4 one-file local commit `7846fa48429d00233116438821acc0ea2a0b38be` binds the exact accepted candidate blob. Gate-5 ordinary non-force push/fresh fetch produced clean `HEAD == origin/main == FETCH_HEAD == 7846fa48429d00233116438821acc0ea2a0b38be`, ahead/behind `0/0`. Hosted GitHub Actions [run 38067711160](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38067711160) on the exact code SHA completed success, Release `0 warnings / 0 errors`, Core `84/84`, Transport `24/24`, failed/skipped `0/0`. The device remains on its previously hardware-accepted production bootloader and authenticated application firmware revision `5`. RDC-08 final repository documentation/source-of-truth debt audit is next; new product features remain blocked.


### RDC-06 kernel composition-root final cleanup — ACCEPTED / PUBLISHED

Gate-0 Amendment A, already published at `3925e1d1e0a30e49c0694f746c5bbd3d7912d81c`, rejected the earlier `schedprod` ownership cut for its measured stack regression and confined the accepted replacement to `help`/command-registry presentation. Exactly three authorized source/header paths changed. Local acceptance commit `7bc5f27ce3447f0c907d2643e7db67669371210a` has Git tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`, matching the exact firmware source candidate. The original `schedprod` regression remains explicitly rejected.

During authenticated hardware Gate-3 acceptance, a reproducible STM32F103 USB EPnR rc_w0 CTR-event loss was found and independently corrected. The single-file bootloader behavior repair was committed separately as `83e57f609bab4bcc8a61ea2222629bf9066e910d`, not mixed into the three-file cleanup scope. Production key-bearing bootloader replacement passed targeted Flash pages 0–7 program/verify and independent full-Flash readback; the post-repair bootloader INFO matrix passed all 72 paced/burst requests with no timeouts. The previously signed application candidate revision 5 was then successfully installed by authenticated USB protocol. Full SWD readback proved the expected 52,012-byte application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, authenticated version floor 5 and unchanged bootloader/persistence ownership.

Post-update USB/RPC/`help`/application/scheduler/IWDG runtime acceptance passed. The separately uploaded Windows CH340 UART log proves `COM3`, `115200 8N1`, `help help` descriptor and 36-command `help` catalog; the operator explicitly confirmed `PHYSICAL_OLED=PASS`. Gate-3 evidence-only final acceptance collation SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD`.

Both code commits were published by normal non-force fast-forward. Fresh fetch proved clean `HEAD == origin/main == FETCH_HEAD == 83e57f609bab4bcc8a61ea2222629bf9066e910d`, ahead/behind `0/0`. Exact-commit GitHub Actions [run 38011195238](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38011195238) completed success with warning/error-clean Host Release build, Core `84/84`, Transport `24/24`, failed/skipped `0/0`. No key, application image, raw Flash dump or local evidence bundle entered source control. The separately reconciled documentation records RDC-06 closure and activates RDC-07 Gate-0 audit/design as the next mandatory maintenance boundary; RDC-08 remains queued and new product features stay blocked.

## 2026-10-08

### RDC-04 Tooling Output Directory Safety — CLOSED / PUBLISHED `7fda7b8225f05c487f6d5d7154443e8369e74571`

The three caller-controlled recursive OutputDir resets now use one fail-closed `Reset-DeusGeneratedOutputDirectory` helper with canonical-path containment, owner markers, protected-path and reparse-point guards. The Asset-phase generator remains historical-only; bootloader/product source and Flash layout are unchanged.

Gate 1 source integration PASS. Gate 2A disposable PowerShell 7 safety matrix PASS `27/27`. Gate 2B clean candidate `315d1b67194ba128b25afda8b7950eb371e998f4` reproduces Flash/SRAM `51972/10968`, global symbols/alloc sections/stack records and baseline BIN after exactly one 40-byte embedded-provenance substitution; both recovery generators retain payload equivalence. Gate 2C docs-finalization evidence SHA-256 `07489AE13299106DD69DA0004975316A84E1F4ADDC75D27A34D2378F4712AC02`.

Gate 3 accepted ten-path local commit `7fda7b8225f05c487f6d5d7154443e8369e74571`, parent `100801293796a4bee3ebf24152cd57fc3924cf27`, tree `ae7760c58186ed227ce381d68562fac0a117ddd5`, evidence SHA-256 `A1BCC379A55F99B912266422BD58BF64D878D446AAE2D8345FAEFD9D379E24CB`. Gate 4 ordinary non-force GitHub publication and fresh-fetch verified `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`, evidence SHA-256 `39A8EF0098CD313A2645316AFF0D5DD50841D080D68F98A0CE90E04D49AAFC1C`. No target I/O or Flash mutation. `RDC-05` minimal deterministic CI is the active design-only next maintenance boundary; implementation requires its scoped plan/acceptance freeze.

## 2026-10-07

### RDC-03 Target dead API cleanup — CLOSED / PUBLISHED `e516fdc1d8818007db40ee12669d28f3f828c489`

Exactly ten re-proven dead target APIs were removed: nine post-decomposition Application Runtime accessors and unused `oled_ui_layout_config_v1_apply()`. Bridge ownership, OLED validate/activate owners and checked source/protocol compatibility `binary_frame_encode()` remain intact.

Gate-2 deterministic equivalence evidence `stm32_os_target_dead_api_cleanup_gate2_final_validation_v2_20261007_212047.evidence.zip`, SHA-256 `224F04F33A91DAC295D0716ABF52B1457CC7DDA8C98316D17098243CBB05FE33`, proves exact accepted baseline reproduction, candidate tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b`, candidate BIN SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`, Flash/SRAM `51972/10968`, exact global symbols/runtime sections/retained stack records and zero non-provenance BIN differences. The collector's retained-symbol summary failure is adjudicated as a harness oracle defect from CRLF regex handling plus the intentionally GC-elided source compatibility encoder.

Exact-candidate hardware acceptance deployed the signed candidate and passed USB management, application lifecycle, scheduler/IWDG and UART diagnostics. Its final tail-erasure failure is adjudicated as `HARNESS_APPLICATION_TAIL_ERASE_ORACLE_01` because the bootloader contract lazily erases only DATA-touched application pages. Read-only continuation evidence `stm32_os_target_dead_api_cleanup_gate3_readonly_continuation_v2_20261007_232443.evidence.zip`, SHA-256 `046E4F1F30A69CDAE00B6B9A37B2C02E16A05F7A4F4EBBA56AB1F48E23341AB9`, has `39/39` exact internal hashes and proves exact application readback, unchanged bootloader/persistence, correct final-touched-page erase semantics, runtime health before/after SWD and exact repository poststate. Operator review is `PHYSICAL_OLED=PASS`.

Acceptance commit `e516fdc1d8818007db40ee12669d28f3f828c489` was published by ordinary non-force fast-forward from direct parent `5b0f6b9586ac5397c877efb2a83601a43a080f19`; fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`. RDC-04 Tooling Output Directory Safety is now active.

### RDC-02 Host dead-surface cleanup — CLOSED / PUBLISHED `09a432f6c0b73ef2425add5950b6f6d5dee3733d`

The Linux Host transport profile migration is now cleaned of eight re-proven unreachable internal remnants: five duplicated native constants, the runtime-only `ValidateManagementTopology()` wrapper, unused `libusb_get_device_address` P/Invoke, and the one-argument `LinuxLibUsbTransport.Open(string)` shim. Public runtime profile constants, runtime/bootloader `LinuxUsbProfile` ownership, profile-aware `ValidateTopology(...)`/`Open(..., profile)`, raw `SysInfoAsync()` compatibility and active Asset protocol APIs remain unchanged.

Gate-2 evidence `stm32_os_host_dead_surface_cleanup_gate2_host_validation_v1_20261007_193632.evidence.zip`, SHA-256 `44C5DBFA295087F1F2BFD5A453E63C3855EBC0508EEC5D3BEF283F99D4482B84`, has internal hash index `48/48` exact and proves Linux transport/Core Release builds with zero warnings/errors, Transport `24/24`, Core `84/84`, exact candidate/poststate binding, `TARGET_IO=NONE` and `FLASH_MUTATION=NONE`.

Acceptance commit `09a432f6c0b73ef2425add5950b6f6d5dee3733d` was published by ordinary non-force fast-forward from direct parent `bc562f5030990d4b4e69af35d0ad1a3e4cb301bb`; fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`. RDC-03 Target dead API cleanup is next.

### RDC-01 Host Asset transaction recovery — CLOSED / PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b`

Host Core now closes ambiguous Asset write sessions with one bounded best-effort same-transfer `ABORT` after BEGIN may have activated a volatile session and a subsequent BEGIN/WRITE/COMMIT path fails or is cancelled. Cleanup uses an independent two-second deadline, reuses the accepted channel-owned single-response abandonment model, never masks the primary failure and performs no mutation retry. Successful COMMIT closes the cleanup window, so post-COMMIT STATUS/readback failures do not emit ABORT.

Gate-2 evidence `stm32_os_host_asset_transaction_recovery_gate2_host_validation_v1_20261007_190709.evidence.zip`, SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA`, proves exact five-path binding, warning-clean Core Release build and executed Core `84/84` PASS with `TARGET_IO=NONE` / `FLASH_MUTATION=NONE`. Its `outcome.txt` retained one stale `CORE_STATIC_TEST_METHODS=80` literal; this is reconciled as `HARNESS-EVIDENCE-RECORD-CARDINALITY-01` because the hash-verified manifest, run log and executed MTP summary independently prove `84`.

Acceptance commit `699382a58c4c9570cdf36a05693044461e87c58b` was published by ordinary non-force fast-forward from direct parent `7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`; fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`. RDC-02 Host dead-surface cleanup is next.

### Residual debt-first governance — mandatory RDC-01..RDC-08 before new features

A post-FDC10 read-only audit plus accepted Host Asset correlation hardening showed that the completed FDC program did not exhaust all concrete current technical/tooling/maintainability/documentation debt. Project governance is therefore tightened: no new product feature boundary may be promoted until `RESIDUAL_DEBT_CLOSURE_PROGRAM` closes RDC-01..RDC-08 and its final documentation sweep finds no unclassified current debt.

The mandatory sequence covers Asset transaction/session recovery, Host and target dead-surface cleanup, PowerShell destructive-output safety, a minimal deterministic CI baseline, bounded remaining kernel composition-root ownership cleanup, bootloader readability normalization, and a final documentation/source-of-truth sweep. Consumer-driven future capabilities such as generic timers/queues, DMA, filesystem, networking, low-power and richer observability remain deferred rather than being mislabeled as current debt. The active next maintenance boundary is RDC-01; this governance activation is docs-only and authorizes no source/target/Flash/hardware mutation.

Governance commit `2fb687d4bd57e8029e89d450038b712acb6d9dea` was published by ordinary non-force fast-forward from direct parent `d649e133cbe39fa6155520c657136ca99cee61ae`; fresh post-push fetch proved `HEAD == origin/main`, clean worktree/index and ahead/behind `0/0`. The program remains ACTIVE; only its governance freeze is published.

### Host Asset request-correlation hardening — CLOSED / PUBLISHED `40aab02b4e0c04466453be4c31041f8649c13c1b`

A narrow Host Core maintenance candidate closes a confirmed single-response correlation gap in `AssetTransferClient`: once an Asset request owns a nonzero request ID, native timeout or cancellation now registers that ID through the existing channel-owned `AbandonSingleResponse()` model before the original failure propagates. No new decoder, stale queue, retry policy, protocol `ABORT`, wire value, native transport or target behavior is introduced.

Gate-2 evidence `stm32_os_host_asset_request_correlation_gate2_host_validation_v2_20261007_181012.evidence.zip`, SHA-256 `F574489FE68B35D42DEB8BB0E8EE88FB029B8FAAED5C7F926BB7194C256D3285`, proves exact five-path candidate binding, .NET 10/Microsoft.Testing.Platform policy scope, isolated locked restore, Core Release build with zero warnings/errors, executed Core `80/80` PASS, temporary-index whitespace validation and unchanged live poststate with `TARGET_IO=NONE` / `FLASH_MUTATION=NONE`. Volatile Asset transaction/session cleanup via `ABORT` remains outside this boundary.

Acceptance commit `40aab02b4e0c04466453be4c31041f8649c13c1b` was published by ordinary non-force fast-forward after fresh direct-parent proof; fresh post-push fetch proved `HEAD == origin/main`, clean worktree/index and ahead/behind `0/0`. The boundary is closed; Asset `ABORT` transaction/session cleanup remains unpromoted.

### FDC-10 documentation/source-of-truth closure — final foundational-debt reconciliation

Repo-wide documentation reconciliation closes the ten-item `FDC-01..FDC-10` foundational-debt program. The final inventory is `90` Markdown files in `docs/`, `30` acceptance-plan records and `33` non-acceptance plan-named files; unmatched plan names are limited to the explicitly historical/deferred IMPLEMENTATION/OLED set and there is no orphan active boundary. Stale current claims were corrected for FDC-05..08 publication, physical bench/source alignment, FDC-06 semantic-state ownership, Host Management prerequisites and published CDC identity wording while historical chronology and genuinely deferred consumer-driven roadmap items were preserved.

Repository hygiene is clean: product source has no `TODO/FIXME/HACK/XXX/TBD` markers; tracked firmware/build/map/evidence/archive/log/key/certificate-like artifact classes are absent; obvious credential/private-key signatures are absent; `.gitignore` and `.gitattributes` remain appropriate. FDC-10 changes documentation only and perform no target/Flash/hardware mutation. This docs-only acceptance revision is published by ordinary non-force push/fresh-fetch proof; closing the foundational blocker does not automatically activate Host Management Service/Web/network implementation, which still requires a separately promoted plan/acceptance boundary.

### FDC-05..08 consolidated target closure — Gates 0–7 accepted / published `6aa2df19ab02c14bde38833e738fe825008102e8`

The shared exact candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e` has completed FDC-05 composition-root convergence, FDC-06 semantic system/service state, FDC-07 fail-closed application stop semantics and FDC-08 bounded target/update robustness through Gate 5. Application identity is `51972/53248` bytes, SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`.

Consolidated Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` accepts warning-clean application/bootloader builds, Core `78/78`, Transport `24/24`, resource/stack/static ownership checks, deterministic FDC-06/FDC-07/FDC-08 oracles and exact candidate binding. Final real-hardware evidence `stm32_os_fdc05_08_consolidated_hardware_gate4_v18_20261007_115017.evidence.zip`, SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`, accepts runtime/USB/UART/OLED/scheduler/IWDG equivalence, authenticated v3 firmware update/recovery, non-DATA no-blind-retry adjudication, DATA exact retry policy, composite reset-deadline proof, exact final application/tail/bootloader/metadata/persistence identity and final `CANDIDATE_V3` physical target state. Physical OLED is PASS and no rollback was required.

Gate 5 reconciled the scoped FDC plans/acceptance records, `CURRENT_STATE`, roadmap/checklist, architecture/backlog and host-management blocker wording. Gates 6–7 then completed at `6aa2df19ab02c14bde38833e738fe825008102e8` by ordinary non-force publication with fresh-fetch clean `0/0`. `FDC-10` subsequently performed the final documentation consistency closure; no Host Management Service/Web/network implementation was activated by that closure.

## 2026-10-04

### FDC-09 Host native transport lifetime hardening — CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73`

Gate 6/7 publication is complete. Accepted implementation/docs commit `b88a9eee43095665326787cc0345822218c1ba73`, tree `43829e79c4679146aae5156d635680fb3b6dad39`, direct parent `5bfb0a28815f3712d30482b394a5b44c17a6011c`, was published by ordinary non-force fast-forward. Fresh fetch proved `HEAD == origin/main == FETCH_HEAD == b88a9eee43095665326787cc0345822218c1ba73`, clean ahead/behind `0/0`; GitHub independently confirms the same commit/tree/parent and a valid verified signature. Slice A (`FDC-01..04,09`) is complete; FDC-05 Gate-0 planning/audit is next, with no FDC-05 source mutation authorized until its dedicated plan/acceptance pair freezes the boundary.

The V5 Windows physical proof remains accepted despite the separately classified `HARNESS-TERMINAL-STRUCTURE-01` presentation defect. No hardware rerun is required solely to recover terminal coloring. Future operator harness packages must satisfy the canonical green PASS / red FAIL / yellow warning final-result structure and reserved-word lint before handoff.

### FDC-09 Host native transport lifetime hardening — Gates 0–5 accepted / publication pending

The exact four-path `HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING` candidate has completed deterministic and real-platform acceptance. Gate-2 evidence `stm32_os_fdc09_gate2_host_validation_dotnet_v2_20261004_215723.evidence.zip`, SHA-256 `1B4090EEED139BA7B5E7BEB11C530B0CF4B1EEB44706ADCA079430FAD686FFCE`, proves Transport `24/24`, Core `78/78`, Windows/Linux/Core Release builds, exact source/poststate locks and zero target/Flash mutation.

Linux Gate-4 evidence `stm32_os_fdc09_gate4_linux_real_platform_dotnet_v1_20261004_222724.evidence.zip`, SHA-256 `F084F76AEA3ED28E4BE4E697FCDF323360ED190810EEE4E3F944AD56930E2900`, proves IF2-only libusb ownership, bounded cancel-drain `2021 ms`, dispose `3 ms`, same-locator reopen `15 ms`, fresh negotiation/ping and unchanged CDC `cdc_acm` ownership `2 -> 2`. Windows Gate-3 evidence `stm32_os_fdc09_gate3_windows_real_platform_dotnet_v5_20261004_225617.evidence.zip`, SHA-256 `B392F3AEA191E0649CBBF2FFEBCE8AA2CA580BE79D25AF4B36FE58FD699728C5`, proves WinUSB cancel-drain `2017 ms`, dispose `3 ms`, physical removal classification `TransportDisconnected` in `7 ms`, same-locator replug/reopen, fresh negotiation/ping and restoration of canonical Mac-mini USB ownership in `634 ms`; Flash mutation remains NONE.

A post-run audit also recorded `HARNESS-TERMINAL-STRUCTURE-01`: recent .NET validators omitted the canonical green/red/yellow final RESULT rendering and used reserved `PASS`/`FAIL` words in intermediate summary text. The physical/product evidence is complete and is not rerun for presentation-only styling. Future harness package generation must lint and reject missing final color structure or reserved-word discipline before operator handoff.

### FDC-09 Host native transport lifetime hardening — Gate 0 activated

`HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING` is now the active final Slice-A host closure boundary on published FDC-04 baseline `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`. The audit confirmed that both platform adapters execute synchronous native USB calls inside `Task.Run(..., cancellationToken)`: cancellation after native entry does not stop `WinUsb_ReadPipe`/`WinUsb_WritePipe` or `libusb_bulk_transfer`, while current transport disposal may close/release native handles without coordinating with an already-running worker.

Gate 0 freezes per-instance **latched bounded cancellation** under the existing 2000-ms native transfer timeout: pre-entry cancellation prevents native I/O; cancellation after entry waits for the native transfer to drain before surfacing cancellation; Dispose marks closing, prevents new native entry, drains active I/O, then releases handles exactly once. Runtime and bootloader profiles already share the same Windows/Linux transport owners, so no parallel bootloader transport patch is authorized. Frozen product/test scope is Windows transport (+ optional test IVT project metadata), Linux transport, and `TransportContractTests.cs`; Core protocol/session and target source remain out of scope.

### FDC-04 Host typed management models hardening — CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`

Gate 4/5 accepted implementation/docs commit `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`, tree `b47361fbdb4e82a6d3ddebdf55d5d3fd7ce4f300`, direct parent `f0614d4182b5f603f43eb79b5dd011b929de0e8c`. Ordinary non-force push plus fresh fetch proved `HEAD == origin/main == FETCH_HEAD`, clean `0/0`; GitHub independently matched the same valid signed commit. Authoritative Gate-2 evidence remains SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`, manifest `78/78`, Core `78/78`, Core/CLI/Desktop Release builds PASS, target I/O and Flash mutation zero.

### FDC-04 Host typed management models hardening — Gate 2 accepted / publication pending

`HOST_TYPED_MANAGEMENT_MODELS_HARDENING` accepted the exact eight-path candidate on published Gate-0 baseline `f0614d4182b5f603f43eb79b5dd011b929de0e8c`. Authoritative evidence is `stm32_os_fdc04_host_validation_dotnet_v1_20261004_191243.evidence.zip`, SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`: ZIP CRC clean, manifest `78/78`, exact six-modified-plus-two-new pre/post state, Core `78/78`, Core/CLI/Desktop Release builds PASS, target I/O NONE and Flash mutation NONE.

Static ownership acceptance proves `ManagementStateParser` is the sole new Core parser owner; low-level raw compatibility APIs remain present; `ManagementServiceOperations` has zero `RpcResult` return types; Desktop has zero raw `OutputText` uses; CLI has exactly one raw `OutputText` use confined to the explicitly excluded `rpcinfo` diagnostic. Gate 3 documentation reconciliation is complete in the local publication candidate. FDC-04 remains publication-pending until normal Gate-4 commit and Gate-5 non-force push/fresh-remote verification.

### FDC-04 Host typed management models hardening — Gate 0 activated

`HOST_TYPED_MANAGEMENT_MODELS_HARDENING` is now the active host closure slice on published FDC-03 baseline `0d9adfd8d0ed11478194e2268ede3c57379c8294`. Gate 0 freezes an exact eight-path source/test boundary: one new Core management-model/parser file, typed wrapper additions in `DeusDeviceClient`, convergence of `ManagementServiceOperations`, CLI and Desktop onto the typed models, one new parser/model test file, and updates to the FDC-03 service tests.

Low-level `RpcAsync` plus existing raw Ping/Health/app-start/app-stop wrappers remain compatibility/operator APIs. FDC-04 adds typed `PingStatus`, `HealthSnapshot`, application start/stop acknowledgement models and typed wrappers; `Applications`, Asset results and negotiation/system info reuse existing typed models. CLI `rpcinfo` remains an explicitly excluded raw diagnostic under FDC-03 rather than becoming a service state surface. No protocol/channel/session/native transport/firmware/bootloader/target change is authorized.

### FDC-03 Host service operation allowlist hardening — CLOSED / PUBLISHED `0d9adfd8d0ed11478194e2268ede3c57379c8294`

Gate 4/5 accepted implementation/docs commit `0d9adfd8d0ed11478194e2268ede3c57379c8294`, tree `e8b3a2051d6c2e43266b72cdac21ca28084dc4b8`, parent `1fe0315916deaf551dc0249f2dffb424abd5e7dc`. Ordinary non-force push plus fresh fetch proved `HEAD == origin/main == FETCH_HEAD`, clean `0/0`; GitHub independently matched the same valid signed commit. Authoritative Gate-2 evidence remains SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`, Core `66/66`, Core/CLI/Desktop Release builds PASS, target I/O and Flash mutation zero.

### FDC-03 Host service operation allowlist hardening — Gate 2 accepted / publication pending

`HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING` accepted the exact two-file candidate on Gate-0 publication baseline `1fe0315916deaf551dc0249f2dffb424abd5e7dc`. Production `ManagementServiceOperations.cs` SHA-256 is `299F3344EF1BF79736DEB5A9D5ED6D2E1AA63513683DE6711BBD8FAFCD1CE668`; `ManagementServiceOperationTests.cs` SHA-256 is `A7A43385AE9F1BF754BCDD56E2AF17781AAD871584766374C228D49F539259D1`.

Authoritative evidence is `stm32_os_fdc03_host_validation_dotnet_v2_20261004_184243.evidence.zip`, SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`: CRC clean, manifest `78/78`, static service catalog `8 = 5 ReadOnly + 3 Control + 0 Destructive`, no raw RPC/bootloader/flags/dynamic-registry route, exactly three `PublishedOnly` Asset routes, Core `66/66`, Core/CLI/Desktop Release builds PASS, exact pre/post candidate state, target I/O NONE and Flash mutation NONE.

The preceding v1 test run correctly exposed two invalid synthetic `sysinfo` capability fixtures; only the test fixture changed and the production facade remained byte-identical. Gate 3 documentation reconciliation is complete in the local publication candidate. FDC-03 remains publication-pending until normal Gate-4 commit and Gate-5 non-force push/fresh-remote verification.

### FDC-03 Host service operation allowlist hardening — Gate 0 activated

`HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING` is now the active host closure slice on published FDC-02 baseline `42245d9d71504482fb189d8351ecce7049542145`. Gate 0 freezes an exact two-new-file implementation boundary: `ManagementServiceOperations.cs` plus `ManagementServiceOperationTests.cs`.

Service v1 is explicitly compile-time allowlisted in Core: ReadOnly = `Ping`, `Health`, `Applications`, `AssetStatus`, `ReadOledUiLayout`; Control = `StartApplication`, `StopApplication`, `WriteOledUiLayout`; Destructive = none. Numeric RPC IDs, caller-controlled protocol flags, generic argument/string command proxying, bootloader entry, watchdog trip, scheduler stress/diagnostics, UI test/mutation and other unlisted commands are excluded. Firmware command classes are not host authorization policy. FDC-04 remains separate for typed management result models/parsers.

No Web/service host, HTTP route, network listener, native transport, firmware/bootloader or target change is authorized by this activation.

### FDC-02 Host session state-event reentrancy hardening — CLOSED / PUBLISHED `42245d9d71504482fb189d8351ecce7049542145`

Gate 4 accepted commit `42245d9d71504482fb189d8351ecce7049542145`, tree `5b04eabfda05bb63dca347e3b67bd2b431da3880`, direct parent `0f32c7c22a2526e5ce53dd877e5cce25209324e9`. Ordinary non-force push and fresh fetch proved `HEAD == origin/main == FETCH_HEAD`, clean `0/0`; GitHub independently matched the same valid signed commit. Authoritative Gate-2 evidence remains SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`, Core `58/58`, Core/Desktop Release builds PASS, target I/O zero.

### FDC-02 Host session state-event reentrancy hardening — Gate 2 accepted

`HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING` has passed bounded implementation/static review and deterministic host validation. `DeusDeviceSession.SetState()` no longer executes arbitrary `StateChanged` subscribers inline while `_operationGate` is owned; one private per-session asynchronous task chain serializes notifications, callback exceptions are isolated, and Connect/Execute/Disconnect recheck disposed state after gate acquisition without disposing the gate into a queued-operation race.

Authoritative Gate-2 evidence is `stm32_os_fdc02_host_validation_dotnet_v2_20261004_162345.evidence.zip`, SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`. The ZIP CRC is clean and its manifest is `72/72` exact. It proves exact two-path pre/post WIP state at `HEAD == origin/main == 0f32c7c22a2526e5ce53dd877e5cce25209324e9`, staged/untracked `0/0`, Core `58/58` with failed/skipped/errors/not-run all zero, `DeusOs.Control.Core` Release build PASS, `DeusOs.Control.Desktop` restore/build PASS, target I/O NONE and Flash mutation NONE.

The deterministic matrix covers exact connect order, synchronous reentrant Execute and Disconnect from `StateChanged`, exact recovery ordering, throwing-subscriber isolation through Connect/recovery/Disconnect, effective unsubscribe, Dispose completion while a callback is blocked, and bounded/idempotent Disconnect/Dispose race behavior. FDC-01 correlation/native transport/target source remain unchanged.

Gate 3 canonical reconciliation is complete in the local acceptance candidate. FDC-02 remains publication-pending until its normal Gate-4 local commit and Gate-5 ordinary non-force push/fresh-remote verification complete; FDC-03 source mutation is not yet authorized.

### FDC-01 Host RPC timeout/correlation hardening — Gate 2 accepted

`HOST_RPC_TIMEOUT_RECOVERY_HARDENING` has passed implementation/static review and deterministic host validation. The accepted implementation keeps `DeviceProtocolChannel` as the single decoder/request-ID/correlation owner, adds one channel-owned abandoned-request registry with `SingleResponse` and `RpcUntilTerminal` shapes, marks generic RPC requests abandoned on timeout/cancel, drains delayed `RPC_DATA...RPC_END` or terminal `PROTOCOL_ERROR`, and fails closed if request-ID wrap is reached while unresolved abandoned state remains.

`FirmwareUpdateClient` now uses the same channel abandonment owner for single-response request loss. DATA retains exactly one same-payload immediate timeout retry; INFO/BEGIN/AUTHORIZE/END retain no blind retry. Unknown non-abandoned request IDs remain fatal correlation errors.

Authoritative Gate-2 evidence is `stm32_os_fdc01_host_validation_dotnet_v1_20261004_151712.evidence.zip`, SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`. The evidence ZIP has clean CRC and `65/65` exact manifest entries, verifies exact pre/post five-path WIP state at `HEAD == origin/main == 953621bda0e4dd530e9e3a0f149fd9316d811c8d`, runs Core tests `51/51` with failed/skipped/errors/not-run all zero, and passes `DeusOs.Control.Core` Release build. Target I/O and Flash mutation are zero.

The repeated PowerShell validation failures preceding this PASS were harness failures, not FDC-01 product-test failures. The final accepted validator is a typed .NET 10 file-based utility using fresh tracked-only scratch materialization, isolated NuGet/.NET state and structured xUnit XML result counters, eliminating PowerShell pipeline/cardinality ambiguity from the acceptance path.

FDC-01 Gates 3–5 are now closed: accepted implementation/docs were committed as `c863b5ab9d00ab96de7c8f8275f905ed52c8740e` (tree `146704385bcaf0b7196b3519b8fe17488200fe63`, parent `953621bda0e4dd530e9e3a0f149fd9316d811c8d`) and published by ordinary non-force fast-forward `953621b..c863b5a`. Fresh fetch proved `HEAD == origin/main == FETCH_HEAD == c863b5ab9d00ab96de7c8f8275f905ed52c8740e`, clean `0/0`; GitHub independently reports the same commit with a valid verified signature. `FDC-01` is CLOSED/PUBLISHED.

`FDC-02` is now activated as `HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING`, baseline `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`. Its Gate-0 contract freezes `DeusDeviceSession` as the sole lifecycle owner while moving public `StateChanged` subscriber execution outside the `_operationGate` critical section through one serialized per-session asynchronous notification mechanism. Required closure includes deterministic transition order, callback exception isolation, reentrant session-operation tests, bounded Disconnect/Dispose behavior, Core tests and Core/Desktop Release-build regression. Only `DeusDeviceSession.cs` and `ClientTests.cs` product/test mutation is authorized; `FDC-03+`, native transports and target code remain blocked.

## 2026-10-03

### FDC-01 Host RPC timeout/correlation hardening — Gate 0 activated

The first mandatory closure slice is now frozen as `HOST_RPC_TIMEOUT_RECOVERY_HARDENING`, baseline `8bd09ad890ab10bb7fed6ecba21d5ad6a382237b`. The root cause is the mismatch between single-response stale filtering and generic multi-frame RPC: after timeout/cancel an abandoned request may still emit delayed `RPC_DATA...RPC_END`, while the published channel currently retires a stale request ID after discarding only one frame.

Gate 0 selects one channel-owned abandoned-request registry with two response shapes: `SingleResponse` for the accepted firmware-update delayed-response retry case, and `RpcUntilTerminal` for generic RPC until `RPC_END` or `PROTOCOL_ERROR`. Unknown non-abandoned request IDs remain fatal; arbitrary time-based stale expiry is forbidden; unresolved abandoned state reaching request-ID wrap must fail closed and require a fresh client/transport session rather than unsafe ID reuse. INFO/BEGIN/AUTHORIZE/END retain the no-blind-retry rule.

This activation is documentation-only. The frozen source boundary is limited to Host Core correlation/RPC/firmware-update/request-ID ownership plus the two Core test files; STM32 firmware, bootloader, native WinUSB/libusb adapters, CLI/Desktop, scripts, Flash and hardware state are unchanged. Canonical plan/acceptance are `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_PLAN.md` and `docs/HOST_RPC_TIMEOUT_RECOVERY_HARDENING_ACCEPTANCE_PLAN.md`.

### Foundational debt closure program promoted after post-publication architecture/code audit

A fresh architecture/code audit after physical deployment found no forgotten accepted-boundary Gate, but identified ten residual obligations that must be closed before the next Host Management Service/Web/network feature sequence. Canonical IDs are `FDC-01..FDC-10`: generic RPC timeout/cancel correlation; session state-event reentrancy; service operation allowlisting; typed Host Core management models; remaining composition-root convergence; semantic system/service state upstream of presentation; fail-closed application stop failure; bounded boot/runtime/update waits and recovery; bounded native transport cancellation/disposal; and documentation/source-of-truth reconciliation.

`docs/CURRENT_STATE.md` and `docs/ROADMAP.md` promote these items from optional/trigger-driven debt into a mandatory pre-feature closure program. `docs/MASTER_EXECUTION_CHECKLIST.md` carries the executable per-item criteria, and the relevant historical Host Control, application runtime/model, composition-root and firmware-update plans/acceptance plans carry clearly labelled post-publication addenda without rewriting their accepted Gates 0–7 history.

The documentation reconciliation also corrects two concrete current-state drifts discovered during the audit: the final firmware-update Flash map is explicitly `8 KiB bootloader + 52 KiB executable application + metadata A/B + persistence A/B`, rather than the superseded Gate-0 54-KiB application umbrella; and old non-historical `USB CDC later` / broad `UI persistence deferred` wording is reconciled with the already published CDC and narrow `OLED_UI_LAYOUT_CONFIG_V1` persistence foundations. Historical Asset-era 54-KiB contracts and the 2026-10-01 deferred-debt chronology remain preserved as historical facts.

No product source, target, Flash, remote host or hardware state is changed by this documentation promotion. New service/Web/network implementation remains blocked until `FDC-01..FDC-10` are accepted closed through their dedicated slices.

### Published Firmware Update / Bootloader v2 physically deployed and runtime verified

The published `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` image is now physically installed and verified on the STM32 bench. Authoritative evidence is `stm32_os_published_deploy_adjudicated_retry_v7_20261003_141252.evidence.zip`, SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`.

The accepted deployment starts from the historical Stage-10 baseline whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`, uses exact signed package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, verifies bootloader `INFO=Ok/RecoveryIdle` with floor/committed version `1/1`, and receives exact update completion `Ok/Committed` with floor/committed version `2/2`.

Independent post-deploy Flash readback proves exact application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, zero non-`0xFF` bytes in the application tail, metadata A retained as historical version `1`, metadata B committed as version `2` with marker `0xA55A` and matching digest, bootloader unchanged and persistence unchanged. Current whole-Flash SHA-256 is `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`.

Runtime verification proves source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, runtime and HELLO capability masks `0x0000007F`, `PING=PONG`, health OK and no rollback. The board therefore now matches the published v2 product identity.

The preceding END-timeout forensic is closed without a firmware patch. Retained chronology proved the earlier failed published-deploy smoke executed an exact post-failure restore to the old version-1 baseline; therefore the old `C7EB7201...` Flash state observed afterward did not prove that the published package had failed to transfer or commit. The controlled adjudicated retry then completed with a normal END response and independent Flash/runtime verification.

Harness policy is also tightened from this campaign: authoritative evidence is one self-contained `.evidence.zip` with manifest-owned `run.log`; loose duplicate logs are no longer emitted by default. New failure-pattern guidance records PowerShell automatic-variable parameter collisions, Bash leading-zero USB identifiers, and operator-visible interactive privilege boundaries.

## 2026-10-01

### Firmware Update / Bootloader Foundation — Gate-7 published and post-publication audit

Gate 7 published the Gate-6 acceptance commit `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`, tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`, by ordinary non-force fast-forward. Gate-7 evidence SHA-256 `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95` proves post-push `HEAD == origin/main == FETCH_HEAD`, clean worktree/index, ahead/behind `0/0`, matching GitHub `main`, and zero target I/O.

The subsequent read-only post-publication audit found no source-level acceptance blocker and no repository secret/artifact leak. Tracked Git state and repository history contain no firmware binaries/maps/evidence archives/logs/key-file artifact types; product source has no outstanding TODO/FIXME/HACK markers requiring closure; the private recovery bundle is correctly documented as sensitive because it embeds the key-bearing bootloader even though it stores no standalone raw key.

The audit did find documentation drift from the pre-push Gate-6 state. Canonical current/governance/roadmap/architecture/USB/Flash records are reconciled so Firmware Update / Bootloader is a completed published boundary and no new product feature boundary is implicitly active.

At the time of the 2026-10-01 audit, Gate 6/7 were zero-target steps, so the then-last hardware-proven board image was the Stage-10 restored whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`, while published source advertised `SYSTEM_IDENTITY_CAPABILITIES=0x0000007F`. That publication/bench distinction was historically correct on 2026-10-01 and is superseded by the 2026-10-03 accepted physical deployment/runtime verification recorded above.

Four non-blocking robustness items are recorded in the deferred backlog rather than patched speculatively: reset-handler containment within authenticated `image_length`, bounded bootloader HSE/PLL startup failure behavior, explicit non-DATA firmware-update timeout/restart semantics, and stale request-ID lifetime under repeated cancellation.

### Firmware Update / Bootloader Foundation — Gate-6 locally accepted

After Gate-5 hardware/reliability acceptance, Gate 6 reconciled the canonical current-state/design/acceptance/protocol/roadmap records and activated `SYSTEM_IDENTITY_CAP_FIRMWARE_UPDATE` in the runtime system-identity mask. The Gate-6 publication candidate therefore advertises system capabilities `0x0000007F`.

Host publication policy was activated symmetrically: `DeusDeviceClient` no longer treats `SystemCapability.FirmwareUpdate` as forbidden during system-info negotiation, while `NetworkServices` remains forbidden and `EnterBootloaderAsync(PublishedOnly)` still requires the firmware-update bit. Core tests now explicitly cover both the published-capability entry path and rejection when the bit is absent.

Gate-6 validation evidence SHA-256 `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7` is accepted: ZIP CRC clean, `92/92` evidence hashes exact, firmware candidate tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, host candidate tree `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, validated pre-commit full candidate tree `8860661c9bcd5e4424fa5c36b37d48815b8ea7ef`, firmware BIN SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, Flash/SRAM `53212/10956`, Core `41/41`, Transport `12/12`, all Core/Windows/Linux Release builds and zero target/remote/real-index/ref mutation. Gate 6 is accepted by the normal local acceptance commit containing this entry; Gate 7 ordinary non-force publication remains pending and no push is part of Gate 6.

### Firmware Update / Bootloader Foundation — Gate-5 hardware matrix complete

Gate-5 Stages 6–10 are accepted through exact technical evidence plus operator OLED continuations. The campaign covers bad digest, bad authenticator, rollback/version rejection, deterministic reset interruption, physical Mac-mini native-USB/VBUS interruption, recovery and full retry from offset zero, exact successful application bytes, metadata commit ordering, unchanged bootloader pages, byte-identical persistence and deterministic runtime handoff/VTOR.

Key final evidence:

- Stage-6 technical v4 `0C5FD70A063F6E4BB366ED85A45DD4AB060B8B645C5983AF7D3E0E486C2703DA`; continuation `D726C04A812BDAB3262E7D352A146481E627894586ADA89A00DA026614175717`;
- Stage-7 technical `43DF41301E005D6E23DEE7C71D3076F97797AF3419232C46A57F3FAA9F916515`; continuation `EFA420DA03CB0300937B1DBC99C73CBBC66B7627577E82E92AC3F1252B5D7CA9`;
- Stage-8 technical v2 `D2006CEE054A85ECA59B96288B3CE25AFCF5BF97B5611D994CF8F7F06B3DB61F`; continuation `57761DE3C23839566983172E403884F85B1A583AB7C5C9ADC512E6096F22B921`;
- Stage-9 technical `2775E2565EA10BCEF8A72EDEE052D698A515B347C6E177D7ABB972D7F36C1EAB`; continuation `C0240BA36BD79C5DB8413B50B4885A6ADBBE6FB69BF2907A75153CEE89D333EB`;
- Stage-10 physical-VBUS technical `24FA0C42644B7E87F88AAD0B775547C7272A591A366D11A4903274C3F47D6CA3`; continuation `E6EB019B021AB95DB16CA3B95E1C3F7C2C7F44D862ABF2711F6B42089F35D560`.

The final bench was restored byte-exact to whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`.

### Firmware Update / Bootloader Foundation — host reliability repair accepted

The post-Stage-10 architecture/reliability audit identified two host-side defects: delayed responses from timed-out firmware requests could poison a fresh-ID retry/following request, and Windows WinUSB native timeout codes 121/1460 were classified as disconnects rather than timeouts.

The accepted repair keeps one shared `DeviceProtocolChannel` correlation authority, centrally discards one response only for an explicitly stale timed-out request ID, preserves fatal handling for unknown request-ID mismatches, retains exactly one immediate exact-payload DATA retry, and maps only WinUSB pipe-I/O timeout errors to `HostErrorKind.Timeout`.

Validation evidence SHA-256 `61D8AEBCA99E712D893474376CA4FEDD63D17D11021E45C0F7701621C712B108` passed Core `39/39`, Transport `12/12`, Core/Windows/Linux Release builds, exact source/poststate locks and zero target/remote/Git mutation. Formal repair continuation SHA-256 is `253026C36C346C579AD5C1CE0FD528A7F7943A07CC21D792171C14C3350797FD`.

No second firmware-side response-loss mechanism is claimed from the historical INFO/BEGIN/DATA timeout observations; no speculative bootloader patch is introduced. Gate 6 is now the documentation/capability publication finalization step.

## 2026-09-30

### Firmware Update / Bootloader Foundation — Gate-5 Stage-5 technical PASS

`stm32_os_bootloader_gate5_stage5_wrong_target_rejection_v1_20260930_190851.evidence.zip`
(SHA-256 `2BAADC493338BA31A26F38F33A0488C7F2B7F092F2ABB8B8F420296D2766043F`) passed
the signed wrong-target fail-closed hardware case with ZIP CRC clean and `72/72` hash-owned
evidence entries exact.

The run started from exact accepted whole Flash
`A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`,
validated accepted repair bundle SHA-256
`8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`,
and used its frozen signed `wrong_target_v2.pkg` vector SHA-256
`B82A52EF431017A27C0BAA64D346982A93AB6A7D1240C18C14F11A3EDB2CB5FB`
for product `0x534F4544`, target `0x0411`, firmware version `2`.

Pre-case bootloader `INFO` was `Ok/RecoveryIdle` with offset `0`, floor `1`, committed version `1`.
The structurally valid signed wrong-target `BEGIN` passed `Ok/HeaderStaged`; `AUTHORIZE_HEADER`
then returned exactly `TargetMismatch/HeaderStaged`, still with offset `0`, floor `1`, committed
version `1`. `DATA` and `END` were never sent. Whole Flash remained byte-exact while bootloader
was still active after rejection, proving target rejection occurred before authorization-side
metadata erase/application mutation.

Explicit HOTPLUG reset restored runtime `1209:000C`, removed bootloader `1209:000D`, restored
`VTOR=0x08002000`, left `VECTACTIVE=0`, and kernel ticks advanced `4732 -> 6451`; final whole
Flash remained exact. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`.
Formal continuation `stm32_os_bootloader_gate5_stage5_acceptance_continuation_v1_20260930.evidence.zip`,
SHA-256 `C35A3319B806B21BC42B48EAF84822DF0F721AEE5EF5222BFA45C959F0BB90EE`, exact-binds the `72/72` technical evidence with zero target
I/O/mutation, fully accepts Stage 5, and authorizes Stage 6.


### Firmware Update / Bootloader Foundation — Gate-5 Stage-4 technical PASS

`stm32_os_bootloader_gate5_stage4_malformed_header_rejection_v1_20260930_184648.evidence.zip`
(SHA-256 `BD0ABADA1523C4E213A4F2B46997A62FE55124208939786543577D2DD434F281`) passed
the malformed-header fail-closed hardware case with ZIP CRC clean and `68/68` hash-owned
evidence entries exact.

The run entered the accepted bootloader path from exact whole Flash
`A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`
and exercised six structurally invalid `BEGIN` headers: wrong format version, non-zero reserved
byte, image length below minimum, above maximum, misaligned image length, and firmware version
zero. Every request returned `BadHeader/RecoveryIdle` with offset `0`, version floor `1`,
committed version `1`. `AUTHORIZE_HEADER`, `DATA`, and `END` were never sent.

Whole Flash remained byte-exact while the bootloader was still active after all malformed
requests, proving structural `BEGIN` rejection is non-mutating. An explicit HOTPLUG reset then
returned runtime `1209:000C`, removed bootloader `1209:000D`, restored `VTOR=0x08002000`,
left `VECTACTIVE=0`, and kernel ticks advanced `4806 -> 6555`; final whole Flash remained exact.
Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage4_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `AD194B3198D19EF14EA726568DF50207DE7B09EBC4EF2C2FBE2E974974121F5D`, exact-binds the technical evidence with zero target I/O/mutation, fully accepts Stage 4, and authorizes Stage 5.


### Firmware Update / Bootloader Foundation — Gate-5 Stage-3 technical PASS

`stm32_os_bootloader_gate5_stage3_invalid_application_recovery_v1_20260930_182233.evidence.zip`
(SHA-256 `A507F73C628DA735FCF6A5C0A4E58ABB7A984397970ACB7BE3C64CAE9E4F7D50`) passed
the bounded invalid-application -> recovery hardware case with ZIP CRC clean and `72/72`
hash-owned evidence entries exact. The run started from exact accepted whole Flash
`A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, erased
only application page 59 at `0x0800EC00`, and proved exactly that one page changed while
metadata and persistence pages remained unchanged.

After HOTPLUG reset the runtime `1209:000C` device was absent and recovery bootloader
`1209:000D` present. Bootloader `INFO` returned `Ok/RecoveryIdle`, expected offset `0`,
version floor `1`, committed version `0`, proving the retained authenticated floor did not
make the damaged application bootable. No USB update `BEGIN/AUTHORIZE/DATA/END` command
was sent.

Accepted `PRESERVE_PERSISTENCE` recovery then returned whole Flash byte-exact to
`A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`.
Runtime proof passed with `VTOR=0x08002000`, `VECTACTIVE=0`, kernel ticks
`20939 -> 22724`, runtime `1209:000C` present and bootloader `1209:000D` absent.
Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage3_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `E587AEC687955E4A5B93B1350D1489E4F6451726A454C09ED091A2F4A3539CA3`, exact-binds the technical evidence with zero target I/O/mutation, fully accepts Stage 3, and authorizes Stage 4.


### Firmware Update / Bootloader Foundation — Gate-5 Stage-2 technical PASS

`stm32_os_bootloader_gate5_stage2_explicit_update_entry_v4_20260930_180321.evidence.zip`
(SHA-256 `80C670DF059ED37D0A7E2E7BBCF48F55FCC330B21381B68C93FE5DA385DDF8BA`) passed
the explicit update-entry / bootloader-INFO hardware case with ZIP CRC clean and `68/68`
hash-owned evidence entries exact. Runtime `ENTER_BOOTLOADER` returned `Ok/Resetting`,
`1209:000D` enumerated, bootloader `INFO` returned `Ok/RecoveryIdle` with expected offset `0`,
version floor `1`, and committed version `1`. No `BEGIN/AUTHORIZE/DATA/END` command was sent;
whole Flash remained exact at `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`
before, during bootloader idle, and after the return to runtime.

Linux permission handling was constrained to one operator-visible privileged metadata change on
the ephemeral `1209:000D` `/dev/bus/usb/...` node; the firmware protocol itself ran as user `deus`
and no persistent udev/configuration change was made. Explicit `mode=HOTPLUG -rst` restored runtime;
`VTOR=0x08002000`, `VECTACTIVE=0`, kernel ticks advanced `4771 -> 6492`, runtime `1209:000C`
returned and bootloader `1209:000D` disappeared. Operator physical reconfirmation is now
`OLED=PASS` for `DEUS OS / DESKTOP / READY`; Stage 2 is fully accepted. Stage 3 is not yet authorized.


### Firmware Update / Bootloader Foundation — Stage-1 normal-boot repair accepted

The Gate-5 Stage-1 normal-boot repair is accepted after the original CLEAN run exposed a
boot handoff failure. Hardware fault capture and direct Flash instruction readback proved
the causal chain: the old bootloader executed `cpsid i` before application handoff,
leaking `PRIMASK=1`; the application's first scheduler `svc #0` at `0x0800B720`
therefore escalated to a FORCED HardFault with stacked PC `0x0800B722`. The narrow
product repair removes that interrupt mask from normal handoff, while recovery tooling
uses explicit HOTPLUG final-reset semantics.

The narrow reopen was accepted without rerunning unchanged Host product bytes. Gate-3
repair evidence SHA-256 is
`58E7FA5576DACC5BE636A2ABAD5B82E1BFDA1A463182957E7B039BB697F50292`;
Gate-4 repair evidence SHA-256 is
`62596F4066E6C0FEB476FBC412951123A61FED87731C6B48B473B28429BBCF07`.
Accepted repair trees are full
`1972d7d59057eae8e89fda6b0028ccf07eca5bb1`, firmware
`b3b6d4136106f6b6a195fb987a0c66a4b7812842`, unchanged Host
`96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`, and recovery tooling
`24434b8f19be22251e4fd6129a8d39f06b101b9c`. The accepted immutable repair
bundle SHA-256 is
`8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`.

Stage-1 acceptance evidence is
`stm32_os_stage1_repair_apply_accepted_bundle_v3_20260930_155157.evidence.zip`,
SHA-256 `3D3A28F0B0C949CCCFB817B7DC726B1B9F63104348EF954ABFEDD08A0F9F0EC5`.
Its ZIP CRC and all `41/41` hash-owned evidence entries are exact. Post-repair whole
Flash SHA-256 is
`A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`;
runtime proof shows core running, application `VTOR=0x08002000`, no active HardFault,
cleared fault record, advancing kernel ticks, runtime USB `1209:000C` present and
bootloader USB `1209:000D` absent. Physical OLED acceptance is
`DEUS OS / DESKTOP / READY` = PASS.

Gate-5 Stage 2 is not authorized by this result and requires a separate explicit
authorization/acceptance step.

## 2026-09-27

### Published — pre-Bootloader resource / architecture recovery — Gates 0–7 accepted

`PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` is complete and published at commit
`a8f92f83c2ba8917ad183b1a099c9e21199c9463`, tree
`013c1f472eb9404de94befdcc3f1e1e5acfc5831`. The accepted production firmware
candidate remains source tree `a10182e7d0659b9b161073ad49a8816ecb6e7918`, BIN
SHA-256 `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`,
Flash/SRAM `52908/10920`, task margins `304/432` and MSP margin `1592`.
Gate 7 used an ordinary non-force push and final fetch proved
`HEAD == origin/main == FETCH_HEAD == a8f92f83c2ba8917ad183b1a099c9e21199c9463`,
clean worktree/index and ahead/behind `0/0`. Gate-7 evidence SHA-256 is
`335013389D8AB6C1B09C4BF720185FED82EC4A0D30A2F3F6237CF8ACCA1DE8CE`.

### Firmware Update / Bootloader Foundation Gate 0A accepted

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` is now the active boundary. New canonical
design/acceptance records are
`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` and
`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`; exact v1 wire/image ABI is frozen in `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`.

Gate 0A read-only/source-ownership audit passed from
`stm32_os_bootloader_gate0a_contract_source_audit_v1_20260927_200959.evidence.zip`,
SHA-256 `BB3D9210639564D27E0E9FFF57817A10E619991EC3A1960D4D5D29E82E122002`.
It binds the published recovery state and proves the current standalone
`0x08000000/54K` linker, no application VTOR relocation, persistence-only Flash writer,
Asset-era recovery geometry, reserved-but-clear firmware-update capability bit 6, and
the active Host Core decomposition trigger. No build, target I/O, Flash/reset/option-byte
mutation, index mutation, commit or push occurred.

The ten Gate-0 architecture decisions are frozen: management-IF2/BKP one-shot update entry, minimal single-interface vendor/bulk WinUSB+libusb bootloader on private-test `1209:000D`, fixed 48-byte image header, HMAC-SHA-256 with external non-repository key material, explicit product/device/origin binding, monotonic rollback floor, executable pages 8..59 (`52K`), firmware metadata A/B pages 60/61, full-retry recoverable in-place programming and bounded bootloader RAM/stack. The corrected STM32F1 metadata rule writes `0xA55A` once from erased `0xFFFF`; no in-place `0xA500` retirement exists.

### Firmware Update / Bootloader Foundation Gate 0 complete

Gate 0B linked feasibility passed from `stm32_os_bootloader_gate0b_linked_feasibility_recomposed_v4_20260927_225243.evidence.zip`, SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`. ARM GNU 15.3.1 linked the complete acceptance-only bootloader at Flash `5756/8192`, conventional SRAM `1684/2048`, max frame `248/256`, longest linked stack path `648/768`, MSP margin `376`, undefined symbols `0`. The exact current application relocated acceptance-only to `0x08002000/52K` at Flash/SRAM `52932/10920`, end `0x0800EEC4`, leaving metadata and persistence untouched. All 57 native processes exited `0`; stderr was empty; repository/index/poststate stayed exact; target I/O/Flash/reset/RDP/commit/push were absent.

Gate 0 is complete.

### Firmware Update / Bootloader Foundation Gate 1 accepted

Gate 1 Host Core ownership split passed from `stm32_os_bootloader_gate1_host_ownership_split_acceptance_v3_20260927_233619.evidence.zip`, SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`. The exact 22-path byte lock passed; `DeusDeviceClient.cs` is reduced to a 377-line facade while the new `DeviceProtocolChannel`, `DeusRpcClient` and `AssetTransferClient` own the shared protocol channel, generic RPC and Asset transactions respectively. Core, Core.Tests and Transport.Tests builds passed with zero warnings/errors; direct MTP tests passed Core `28/28` and Transport `5/5`; pre/post repository state matched and firmware/linker/startup/test source was unchanged. No target/Flash/reset/index/commit/push mutation occurred.

### Firmware Update / Bootloader Foundation Gate 2 accepted

Gate 2 relocated application build / VTOR / handoff foundation passed from `stm32_os_bootloader_gate2_relocated_application_build_acceptance_v2_20260928_145108.evidence.zip`, SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`. Exact firmware-only tree `637ea07b10cf84882e19cbb8239f31b7f48856a7`; ARM GNU 15.3.1 Flash/SRAM `52932/10920`, BIN end `0x0800EEC4`, vector table and `g_pfnVectors` at `0x08002000`, metadata/persistence symbols exact, stack-usage files `26/26` with `249` records, undefined symbols `0`, exact poststate. Accepted artifacts: BIN `82567F621ED393395810DEB40382EE8477B2975BA1824A407CA9CDDD7B721324`, ELF `DDE555984DA6926CE24C3EAC329E6AB4DDDA4F980E33A05C87258A181036179A`, MAP `0E075E8DC4B3CD64FF89D0B2119FF228CC4834614B15F5006CF3518508DF15B2`. No target/Flash/reset/index/commit/push mutation occurred.

Gate 3 bootloader/update transport/security implementation is current.

## 2026-09-26

### Published — Asset / Configuration transfer foundation — Gates 0–7 accepted

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` is published at commit
`562e786ffa734da055c23144ec4256bc8961bbaf`, tree
`88720a614794d5aef93cf13ac762a4095cb17baf`. The accepted publication-activation
candidate uses firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, host tree
`136687e80c42bd8104ad6c37fbbccb915b60fd08`, BIN SHA-256
`7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`,
Flash/SRAM `54268/11944`, whole-Flash SHA-256
`CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`, and system
capabilities `0x0000003F`. The boundary accepts bounded management-IF2 transfer,
persistent A/B configuration, deterministic fault/corruption recovery, lost-response
idempotency and physical VBUS retention for the exact 8-byte
`OLED_UI_LAYOUT_CONFIG_V1` consumer. Final accepted persistence wear is `46/64`.

### Post-publication documentation reconciliation and pre-Bootloader recovery Gate 0

A repository-wide documentation/code audit found that Asset publication had completed
while several canonical status headers still described Gate 7 as pending. Current-state,
roadmap, documentation-model, architecture, Asset contract status, consumer, gap-review,
deferred-backlog, README, changelog and execution-ledger text are reconciled without
changing historical evidence bodies.

The same audit freezes `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` as the exact
behavior-preserving prerequisite before `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`. The
published Asset image has only 4 bytes under its Flash acceptance ceiling and 344 bytes
under its static-SRAM ceiling. Gate 1 is measurement-only: exact map/object/symbol/stack
attribution and controlled per-family `-Os` variants, with no target I/O or product
source mutation. The current audit also records the 1-KiB default scheduler stack store
as a concrete SRAM-recovery candidate and keeps host `DeusDeviceClient` decomposition
as a separate host-only trigger rather than inflating the target cleanup boundary.

## 2026-09-22

### Asset transfer halfword/chunk staging clarified before handler implementation

Gate-1 primitive review reconciled the protocol's arbitrary 1..32-byte chunk lengths with STM32F1 16-bit Flash programming. The target may keep exactly one pending payload byte across chunk boundaries and an exact <=32-byte cache of the immediately preceding accepted chunk for stop-and-wait retry idempotency. Sequential bytes are still streamed into the inactive slot; the only odd final byte is programmed with erased `0xFF` high-byte padding at object completion. This remains bounded state and does not permit a whole-object SRAM buffer or change any resource/USB limit. No transfer handler or target mutation existed before this clarification.

### Asset STATUS response metadata omission corrected before Gate-1 transfer implementation

Gate-1 implementation review found that the frozen STATUS semantics promised committed payload length/CRC while the common response prefix had no corresponding fields and the response data field was described as READ-only. `ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1` now defines an exact 8-byte STATUS data extension containing committed length and CRC-32. The correction does not change any maximum: STATUS payload is 26 bytes, READ_CHUNK remains the 50-byte maximum response payload, and maximum wire remains 62 bytes inside one 64-byte management packet. No source implementation or target mutation occurred before this contract correction.

### Asset/Configuration Gate 0 closed; bounded Gate 1 authorized

Completed the cross-contract closure audit across Flash ownership, `OLED_UI_LAYOUT_CONFIG_V1`, transfer ABI, A/B persistence, resource budget, Flash-operation policy, deterministic fault injection and ST-LINK recovery. The audit found and corrected two real integration defects before implementation authorization: Asset HELLO capability bit 6 must be carrier-specific (CDC remains `0x3F`, management IF2 candidate `0x7F`), and canonical recovery must own explicit erase sets and program with CubeProgrammer `--skiperase` so PRESERVE mode never depends on hidden download erase behavior and never erases pages 62/63. Numeric invariants pass: `54K + 8K + 2K = 64K`, `64 + 960 = 1024`, Gate-2 Flash ceiling `54272 < 55296`, and max Asset response wire `62 <= 64`. Gate 1 is authorized only for the bounded target/Core/CLI/recovery-script source surface in the canonical plan; startup/vector relocation, scheduler, USB descriptors/PMA, transport adapters, Desktop/Web, bootloader/network/security and generic storage expansion remain outside scope. System capability bit 5 remains unadvertised until full implementation/hardware/fault acceptance.

### Asset/Configuration Gate 0 ST-LINK recovery contract frozen

Added `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` as the sixth and final reopened Gate-0 design contract. Recovery is candidate-bound and generated from the fresh repository-owned Gate-2 build, with a deterministic 54-KiB application-region image, full 64-KiB pre/post readback and two explicit modes: `PRESERVE_PERSISTENCE` and `CLEAN_STATE`. Explicit page erase is used; mass erase, read-unprotect, option-byte mutation and incremental programming are forbidden. CubeProgrammer verify is required but independent full readback remains authoritative. All six Gate-0 design contracts are now frozen; Gate 1 remains blocked pending the final cross-contract closure audit.

## 2026-09-21

### Asset/Configuration Gate 0 deterministic fault matrix frozen

Added `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md`. The mandatory campaign uses a SWD-armed volatile one-shot checkpoint selector rather than adding a product wire command. Nine exact checkpoints span pre-erase through post-runtime-activation; pre-marker reset must select previous/default, post-marker reset selects the new record only through the ordinary validator. The matrix also covers corrupt-record fallback, lost-COMMIT-response idempotency, zero boot-time repair writes and an explicit <=64 deliberate-erase budget. Repeated manual cable pulling is not used for systematic coverage; after deterministic injection, exactly one normal physical power-removal/return retention proof is required. No source/target mutation occurred; only the recovery/ST-LINK contract remains open in Gate 0.

### Asset/Configuration Gate 0 Flash-operation policy frozen

Added `docs/ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md` from ST DS5319/PM0075 constraints. v1 uses page erase plus aligned halfword programming only, requires HSI on, allows bounded same-Flash CPU/IRQ stalls, forbids mass erase/option-byte mutation and never reloads IWDG from Flash code. Worst-case datasheet arithmetic is 74.65 ms BUSY for a maximum 960-byte record and 41.33 ms for the current 8-byte consumer; product ceilings are 80 ms total BSY, 150 ms mutation phase, 500 ms COMMIT response and >=2 s host timeout. Successful changed generations are capped at 10,000; A/B rotation limits that to at most 5,000 erases/page, half the 10k minimum endurance, while unchanged writes are strictly suppressed. No source/target mutation occurred; two Gate 0 contracts remain open.

### Asset/Configuration Gate 0 resource budget frozen

Added `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md`. Asset-phase linker geometry remains standalone at `0x08000000` with physical `54K` length, but Gate-2 Flash acceptance is capped at `54272` bytes (53 KiB), retaining one 1-KiB application page as post-Asset reserve; from the current 50652-byte baseline this permits at most 3620 bytes growth. Static SRAM is capped at `12288` bytes (12 KiB), leaving a 6-KiB address gap below the fixed 2-KiB MSP reservation and allowing only 560 bytes growth from the current 11728-byte baseline. Existing 1024/512 task stacks remain fixed with >=256-byte hardware margins; MSP measurable margin must remain >=1024. Whole-object/page buffering and new tasks remain forbidden. No source/target mutation occurred; three Gate 0 contracts remain open.

### Asset/Configuration Gate 0 persistent A/B contract frozen

Added `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md`. Pages 62/63 remain the only persistence pages; each 1-KiB slot is frozen as a 64-byte envelope plus up to 960 payload bytes. Record validity requires exact metadata/reserved state, header CRC-32, payload CRC-32, consumer validation and final `0xA55A` halfword commit marker. Replacement always targets the inactive page, verifies readback before programming the commit marker, retains the previous committed slot, suppresses unchanged writes, and falls back deterministically to previous valid or compiled default across reset/power loss. No Flash code or target mutation occurred; four Gate 0 contracts remain open.

### Asset/Configuration Gate 0 transfer ABI frozen

Added `docs/ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md` as the first closed design contract of the reopened Gate 0. It additively reserves frame types `0x03/0x85` on Binary Framed Transport v1, uses management IF2 only, fixes one active session, 32-byte chunks, a 960-byte transport ceiling, 30-second abandonment, whole-object CRC-32, strict sequential/idempotent retry semantics, readback generation binding and explicit destructive intent. Initial accepted object type remains only `0x0001` / `OLED_UI_LAYOUT_CONFIG_V1` at exactly 8 bytes. No firmware/host source or target mutation occurred; Gate 1 remains blocked on the remaining five Gate 0 contracts.

### Asset / Configuration Gate 0 reopened for a concrete OLED console-layout consumer

Promoted `OLED_UI_LAYOUT_CONFIG_V1` as the first real persistent consumer and reopened `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` Gate 0. The promoted object is deliberately narrow: an exact 8-byte, non-executable console-clip configuration with compiled default `x=1,y=10,w=126,h=22`; the accepted 128x9 status bar remains frozen/system-owned. The broader configurable-layout/preset/asset plan remains deferred.

The original `DEFERRED_NO_REAL_CONSUMER` result remains historical proof, but it is no longer the current disposition. Gate 0 is now **REOPENED / IN PROGRESS** and Gate 1 remains blocked. Already-satisfied reactivation prerequisites are the promoted consumer/schema/size, frozen Flash ownership map, repository-owned reproducible firmware build, read-only hardware preflight and host package-lock/SDK reproducibility. Remaining Gate 0 work is the first-class binary transfer ABI, persistent A/B record envelope/atomic recovery, new resource ceilings, wear/timing/watchdog/USB continuity model, deterministic reset/power-loss fault-injection matrix and recovery-bundle/ST-LINK restoration procedure. No source/linker/Flash/target mutation is authorized by this documentation step. The Flash decision is also clarified as a two-phase realization: Asset/Configuration keeps the standalone application as reset owner at `0x08000000` with a 54 KiB linker ceiling and leaves pages 54..61 unused; only the later Bootloader boundary moves the application to `0x08002000` and takes reset ownership in pages 0..7.

### Shared Flash ownership / memory map decision frozen

Added `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md` as a docs-only shared contract before persistent Asset/Configuration or bootloader implementation. For the accepted 64 KiB STM32F103C8 target, the future map reserves pages 0..7 (`0x08000000..0x08001FFF`, 8 KiB ceiling) for the reset-owning bootloader/recovery path, pages 8..61 for a relocated application at `0x08002000` with 54 KiB maximum, and pages 62/63 as independent 1 KiB persistent A/B slots. The 8 KiB bootloader ceiling is derived from 64-page capacity, the current 50-page rounded application footprint, two persistence pages and four explicit application-growth pages; it is not a claim that an unimplemented bootloader already fits. The future Bootloader Gate 0 must prove linked-size feasibility or explicitly reopen this decision. No linker/startup/vector/Flash mutation is performed by this documentation decision.

### Host management presentation model formalized

Added `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` as a non-authorizing planning/architecture reference above the accepted Host Control foundation. The documented direction makes `DeusOs.Control.Cli` the first-class automation/headless/acceptance surface, retains the existing Avalonia Desktop as an optional workstation frontend, and reserves a future Web UI for presentation over the shared Core or a bounded host-management service rather than a duplicate browser-side STM32 protocol implementation. A local Web surface is explicitly separated from STM32 networking; LAN/Wi-Fi/remote exposure still requires a dedicated security/trust boundary. No product implementation boundary is activated by this documentation change.

### Firmware build reproducibility readiness accepted

The final pre-feature repository-readiness prerequisite is accepted. Historical Host Control Gate-2 evidence recovered the exact firmware C/startup/link/objcopy invocation and the generated `deus_build_identity.h`; 20/20 surviving header copies are identical at SHA-256 `F6EAA98172FEE67338BFD978F208BD95731063A1160B0C9C1282306A5D6658A5`. Two independent temporary builds with Arm GNU Toolchain `15.3.1` then reproduced the accepted firmware BIN byte-for-byte: `50652` bytes, SHA-256 `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F`, `text/data/bss = 50540/112/11616`, Flash/SRAM `50652/11728`. The previously documented BIN hash differed by one hexadecimal nibble (`...B61FAF9F...` -> `...B61FAC9F...`); all canonical references are corrected by this finalization. ELF/MAP files carry path-dependent debug/map metadata and therefore are not byte-reproducible identity artifacts, while the firmware BIN is exact.

The proven candidate is now versioned as `scripts/build_firmware.ps1` with SHA-256 `BC7B91825B9BB80394C28643848EE3AEF75A4B62017432BB1070190088B8CDFB`. The script retains the recovered source order, compiler/linker flags and accepted source-tree binding, fails closed on source-list drift, and performs build-only work; it does not invoke STM32CubeProgrammer, ST-LINK, USB, UART, reset or Flash programming. Final reproducibility evidence ZIP SHA-256 is `1FDB3C647ACCF4B9A67FDC02514009495F60D1F3E3B4E00C551FC0B4A3F4F847`.

### Host package-lock / SDK readiness accepted

The post-Host-Control repository readiness task for repeatable package restore is accepted. Seven project-local `packages.lock.json` files are now the reviewed dependency graph while `host/global.json` intentionally retains the accepted `.NET 10` major-line policy (`10.0.100`, `rollForward=latestFeature`, prerelease disabled), which selects Windows SDK `10.0.201` and Linux SDK `10.0.112` in the current environments. Locked restore, Release build and direct Core/Transport test applications pass on both Windows (`21/21`, `5/5`) and Ubuntu/Linux (`21/21`, `5/5`); WSL confirms the DEUS bind path and `D:` path are the same checkout; all seven normalized lock hashes match across platforms. Final acceptance evidence ZIP SHA-256 is `CE7E3BB09A24913D5374A875CCE49556A2A186F114E36128E42AD3BB26F42165`. The acceptance performed no STM32 target I/O, Flash mutation, reset or reconnect.

### Asset / Configuration Gate 0 consumer audit — deferred before implementation

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` completed its initial Gate 0 architecture/consumer review. The live target source contains no persistent settings reader, asset registry, Flash persistence owner or current configuration-package consumer; the closest documented candidate, configurable OLED layout/assets, remains explicitly deferred and does not yet freeze exact target packing. Gate 0 therefore records `FINAL_OUTCOME=DEFERRED_NO_REAL_CONSUMER`: Gate 1 is not authorized, capability bit 5 remains unadvertised, and no linker partition, transfer ABI or Flash-writing implementation is introduced. Canonical records are `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md` and `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`.

The deferred boundary now freezes the prerequisites for later reactivation: concrete consumer/schema, exact Flash partition including future bootloader/recovery budget, first-class binary transfer semantics, atomic reset/power-loss recovery, resource/wear/timing acceptance, deterministic fault injection, repository-owned firmware build reproducibility, read-only real-device Flash/revision/protection preflight and a known-good recovery bundle. A versioned `scripts/stm32_readonly_flash_preflight.ps1` was added for the non-mutating hardware discovery step; it explicitly blocks erase/download/write/read-unprotect and permits option-byte access only as `-ob displ`. The firmware build entrypoint remains intentionally unresolved because the full historical compiler/linker invocation is not preserved in Git and must not be guessed.

The read-only physical-board preflight was then accepted and evidence-finalized without hardware or repository mutation: `DEV_ID=0x410`, `DBGMCU_IDCODE=0x20036410`, numeric `REV_ID=0x2003`, factory Flash size `64 KiB`, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF`, RDP disabled and WRP0..31 inactive. All six captured native operations returned `EXIT_CODE=0`. Immutable source-log SHA-256 is `B112A754E82ECC01BDC509B2D8FB359D652D74DDC565B3DE046C30C181672C13`; finalized evidence ZIP SHA-256 is `BB1F928A2E716F2C8D0FA6B160FC7F41B3187A75CF869DD139DF0C450D6B42D8`. This completes the current-board read-only preflight prerequisite but does not authorize Flash writes.

### Pre-feature readiness documentation audit

A follow-up readiness review corrected three residual documentation inconsistencies before the next product boundary: README no longer lists a non-existent `scripts/` directory; the deferred backlog no longer claims stale current/next roadmap authority and explicitly keeps Asset/Configuration implementation deferred if Gate 0 cannot identify a real consumer; and the canonical binary protocol-v1 document now reflects its transport-neutral reuse over the dedicated management interface plus the accepted additive command-service v3 / registry 36 / RPC IDs `0x0021..0x0024`, without changing the v1 wire envelope.

### Repository hygiene / public GitHub audit

The public repository tree and full 63-commit history were audited for generated build/evidence paths and high-confidence secret patterns. No tracked or historical `build/`, `bin/obj/TestResults`, firmware image/object outputs (`.bin/.elf/.map/.hex/.o`), logs/ZIP evidence, `.env`, private-key/certificate bundles or credential/secrets files were found. Content-history scans found no private-key PEM markers, GitHub/OpenAI/AWS/Slack/Google token patterns, literal password/API-key assignments or credential-bearing URLs.

`.gitignore` was hardened to cover firmware intermediates/images, .NET test/coverage/package outputs, common IDE/user files, local `.env` files and private key/certificate/credential filenames. The public handoff no longer exposes the local Windows account name in its Downloads path; it uses `$env:USERPROFILE\Downloads\`.

## 2026-09-20

### Published — Host control application foundation — Gates 0–7 accepted

Boundary: `HOST_CONTROL_APPLICATION_FOUNDATION`.

The accepted firmware candidate is tree `b895955f7738aceb6fca0272d510cc433378c6ab`; BIN `50652` bytes / SHA-256 `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F`; Flash/SRAM `50652/11728`. The accepted host candidate is tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`, targeting `net10.0` with transport-neutral Core, Windows WinUSB, Linux libusb and Avalonia `12.1.2`; Core/Transport tests are `21/21` and `5/5`.

Windows CLI/hardware, Windows Desktop and real Ubuntu 26.04.1/libusb acceptance all pass. Linux claims management IF2 only and preserves CDC IF0/1; physical reconnect changed USB enumeration address `007 -> 008` while stable locator remained `usb:001:8`; fresh HELLO + `sysinfo` negotiation recovered the session. Final diagnostics show USB errors/PMA overruns `0/0`, management packets RX/TX `13/113`, management drops `0/0`, CDC drops `0/0`, and final Flash exactly matches the accepted BIN.

Gate 6 created one normal local acceptance commit `e0f49f168542fa1cf49bca451e01b0c077aa8d18`, tree `42c77f2cf3d7e9f7f5c1ff24d9d437f61a397be6`, direct parent `61d8e138e7ca44aefbad546e0a4ad5561848a260`. Gate 7 used one ordinary non-force push; fresh post-push fetch proved `HEAD == origin/main == FETCH_HEAD`, clean repository and ahead/behind `0/0`. Gate 7 evidence/log SHA-256 are `638569FE26600013B3B6717C76DEBF3251980B06D0B493B835D96CB4BC0CFD8F` / `5BCA12F1E58C2749DD41C81469EA9875DE1877EEF75F306FD622399F569FE5E7`.

Exact next boundary: `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` Gate 0 contract freeze.

### Documentation structure reconciliation

A post-publication audit found that `README.md`, the top execution-checklist block, roadmap status, architecture status and current handoff still mixed historical snapshots with current state. This reconciliation makes `README.md` a short project landing page, keeps dated history in `CHANGELOG.md`, keeps gate history in `docs/MASTER_EXECUTION_CHECKLIST.md`, and keeps detailed live engineering state in `docs/PROJECT_HANDOFF.md`. A follow-up full-document audit also corrected stale decomposition/USB-management status headers, stale Host Gate-1 wording in architecture/implementation docs, resolved-vs-pending entries in the foundation gap review, and stale OLED checklist summary items. Exact live repository HEAD is intentionally no longer hard-coded into current-state docs, preventing every docs-only commit from immediately making those docs stale. No firmware, host source, protocol, hardware or accepted candidate bytes are changed.

### Documentation source-of-truth consolidation

A full audit of all plans, acceptance plans, checklists, handoff material, deferred UI plans and engineering playbook established `docs/CURRENT_STATE.md` as the sole global project-state source and `docs/DOCUMENTATION_MODEL.md` as canonical documentation governance. `ARCHITECTURE.md`, `ROADMAP.md`, boundary plans/acceptance plans, evidence, backlog, changelog, execution ledger and handoff now have non-overlapping roles. Historical umbrella/OLED plans were explicitly classified as historical or deferred; stale `COMM/UART`, static-time, UART-first configurator and UI-specific Flash persistence assumptions were reconciled with the accepted SYSTEM/USB/NETWORK/uptime and host Core/RPC/WinUSB architecture. The harness playbook now prefers direct `@DEUS MCP` repository operations for repo-only work and reserves operator-run ZIP packages for local Windows/hardware/toolchain execution or operations unavailable through the connected repository tool.

A post-consolidation loss/open-checkbox audit then corrected stale Phase 0–2 and Host/USB roadmap state, leaving only genuinely deferred/future unchecked items. It also restored the small set of operator facts that remained useful after handoff compaction: target Device ID `0x410`, ST-LINK firmware `V2J48S7`, the CubeProgrammer-GUI `DEV_CONNECT_ERR` hazard, accepted OLED `128x32 @ 0x3C`, and provenance that an original full-Flash backup had historically been captured.

## 2026-09-19

### Published — USB management device foundation — Gates 0–7 accepted

Boundary: `USB_MANAGEMENT_DEVICE_FOUNDATION`.

Accepted hardware candidate tree `46841b52d351277deb134a6f4709619087b477af`; BIN `50172` bytes / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`; Flash/SRAM `50172/11728`; task stacks remain `1024/512`. The accepted composite identity is private-test `1209:000C` / `Deus OS Device`, CDC interfaces 0–1 under IAD plus management interface 2 `FF/00/00` over EP4 OUT/IN `0x04/0x84` bulk64. Microsoft OS 2.0 uses vendor code `0x20`, set length `178`, first-configuration subset selector `0`, inbox `WINUSB` on IF2 and stable interface GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`.

Gate 3 is PASS as composite `v11+v14`: full WinUSB RPC/application/error-recovery coverage, physical USB reconnect, authorized IWDG reset/recovery with bounded return to `system.home`, WinUSB pressure `128/128`, UART `32/32`, management RX/TX drops `0/0`, task0/task1 margins `448/424`, intact canaries/zero scheduler faults and final exact Flash readback. Gate 3 evidence/log SHA-256: `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6` / `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Gate 5 finalized canonical docs without source mutation. Gate 6 committed the exact accepted source/docs set as `1f88083843c6aae9fd228ad2d677f9252b889a11`, commit tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`. Gate 7 used an ordinary non-force push; fresh fetch proved `HEAD == origin/main == FETCH_HEAD` and clean ahead/behind `0/0`. Exact next boundary: `HOST_CONTROL_APPLICATION_FOUNDATION` Gate 0 contract freeze.

### Post-publication documentation reconciliation

A post-publication audit found stale status-only documentation: several canonical files still said Gate 6 was next, `README.md` still described USB Gate 0/1 as current, and an older kernel-decomposition checklist still left its already-published Gate 6/7 unchecked. This reconciliation updates status/checklist/history text only; no firmware/source/header/linker behavior changes.

### Gate 0 accepted — Host control application foundation

Boundary: `HOST_CONTROL_APPLICATION_FOUNDATION` — Gate 0 architecture/source contract accepted; Gate 1 source implementation next.

The frozen host baseline is C# / .NET 10 with a transport-neutral `DeusOs.Control.Core`, CLI reference client, direct Windows WinUSB adapter, Linux libusb-1.0 adapter and Avalonia 12.x desktop shell. GUI code cannot own framing/CRC/request correlation. The first target identity addition is safe zero-argument `sysinfo=0x0024`; binary frame protocol stays v1, command-service version advances `2 -> 3`, registry count `35 -> 36`, application-runtime ABI stays v1. `sysinfo` freezes stable OS/platform/architecture/source-tree/capability fields and explicitly reports no stable physical unit identity in v1.

Canonical design/acceptance: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` and `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`. Gate 1 target mutation is limited to new `include/kernel/system_identity.h`, new `src/kernel/system_identity.c`, and modifications to `include/kernel/command_service.h`, `src/kernel/command_service.c`, `src/kernel.c`, plus the exact new `host/` solution tree. Transfer/update/network/plugin/security expansion remains out of scope.

## 2026-09-18

### Published — Application runtime foundation — Gates 0–7 accepted

Boundary: `APPLICATION_RUNTIME_FOUNDATION`.

Accepted candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`; BIN `48604` bytes / SHA-256 `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; ELF `77836` bytes / SHA-256 `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`; MAP SHA-256 `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`; Flash `48604 / 65536`; SRAM `10032 / 20480`; named application-runtime static state `184` bytes. Gate 2 evidence SHA-256 is `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`.

The accepted runtime adds static `system.home=0x0001` and `device.info=0x0002`, explicit seven-state lifecycle ABI, task0-only synchronous pointer-free semantic events, a bounded 3x21 application view, and transport-neutral `applist/appstart/appstop` at RPC IDs `0x0021..0x0023` while retaining existing IDs `0x0001..0x0020` and binary frame protocol v1.

Gate 3 hardware/runtime acceptance passed on the exact candidate: text/binary lifecycle, repeated-start idempotence, invalid-start no-mutation, active-app `uiruntime` restore, real minute semantic-event delivery without app rerender, CDC/UART/binary pressure `128/128`, malformed-frame recovery, physical micro-USB power-cycle recovery, IWDG recovery and final exact Flash readback. Minimum observed task0/task1 margins after full application/binary activity were `272 / 424` bytes. Gate 3 evidence SHA-256 is `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; log SHA-256 `703C41A7493D078E456A5721EAFEFF821A152D217FB3F30AB8C809D7452A6B36`.

Gate 4 is `PHYSICAL_OLED=PASS`; evidence SHA-256 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`.

Acceptance-model clarification: with micro-USB as the sole target power source, physical unplug/reconnect is a reset/power-cycle recovery test, not a live `USB_STATE_CHANGED` edge test because the previous volatile USB snapshot does not survive reset. Dynamic semantic-event/no-rerender behavior is independently proven by the real minute transition.

Gate 5 evidence SHA-256 `817D12D74D88F1C0F31C502B0715F038FF356382E56FC0D5421DC237239D78BC`. Gate 6 committed the exact accepted source/docs set as `25752fba557b1a1b518265a93bde05d3a6a3f9ad`, tree `24624db70923bdaa77f134cc956423511785c199`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`. Gate 7 published by ordinary non-force fast-forward; Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`; final ahead/behind `0/0`.

### Gate 0 accepted — USB management device foundation

Boundary: `USB_MANAGEMENT_DEVICE_FOUNDATION` — Gate 0 architecture/source boundary frozen; Gate 1 source implementation next.

Frozen private-test topology is composite `1209:000C` / `Deus OS Device`: CDC interfaces 0–1 remain secondary diagnostics, vendor interface 2 is the primary WinUSB management path over EP4 bulk OUT/IN 64, PMA consumes the remaining `0x180..0x1FF`, and Microsoft OS 2.0 descriptors register stable management GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`. Binary framed RPC v1 and command service v2 / 35-method registry are reused unchanged. Initial Gate 1 source boundary is exactly new `include/kernel/usb_management.h`, new `src/kernel/usb_management.c`, plus `include/drivers/usb_device.h`, `src/drivers/usb_device.c`, and `src/kernel.c`.

### Gate 5 accepted — kernel composition-root decomposition

Boundary: `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — Gates 0–7 accepted and published at `fa75307fb392718a1d10d52770a6a111c97208e7`.

The accepted decomposition reduces `src/kernel.c` from `5100` to `3405` lines (-33.235%). `application_runtime_bridge` owns mutable application-runtime integration state, semantic-event snapshotting and application-view adaptation; `application_commands` owns `rpcinfo/applist/appstart/appstop`; `scheduler_diagnostics` owns diagnostic orchestration. The composition root retains top-level initialization/wiring, production task binding/start, top-level IRQ/exception glue and production scheduler observability that depends on root-private production state. No universal `kernel_context_t`, service locator, hidden extracted-module extern state, heap, new task/SVC/queue/mutex/generic timer/DMA/persistence machinery or dependency cycle was introduced.

Linked before/after measurements: `console_execute_request 8644 -> 6780`, `boot_desktop_ui_render 1420 -> 804`, `kernel_main 1116 -> 1112`; the former `console_execute_scheduler_diagnostic=3284` monolithic responsibility is now owned by `scheduler_diagnostics_execute_diagnostic=3312`. New owner entry points include `application_commands_execute=776`, `application_runtime_bridge_service=316` and `application_runtime_bridge_apply_view=216` bytes.

Accepted candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` bytes / SHA-256 `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF SHA-256 `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP SHA-256 `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536` against ceiling `48656`; SRAM `10032/20480` against ceiling `10104`; final task0/task1 margins `384/424` bytes.

Gate 3 retained boot/scheduler/IWDG/USB behavior, text+binary lifecycle/idempotence/invalid-start-no-mutation, minute-event/no-rerender behavior, text and binary diagnostic BUSY `8/8`, CDC/UART/binary pressure `128/128`, malformed/CRC/oversize/split recovery, physical reconnect, authorized IWDG recovery, zero production/application faults and exact final Flash readback. Gate 2 evidence SHA-256 `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549` (authoritative build log `88A0AEA8569D404FC8F498124F042F2142EA43440CF9D9CF4096A3C3D12585E0`; evidence-repair log `904C5C476D3A4ED29CAB80532B30FE5B1DD757DF770CC8B1ACEA6AEC1C18A25F`); Gate 3 evidence SHA-256 `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42` (log `4F3714F2D8772F032D3D15CFA5C3A61E69B53D69C60E8D0E77101297153D5856`); Gate 4 is `PHYSICAL_OLED=PASS`.

Canonical decision/design/acceptance: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`, `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`, `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`.

## 2026-09-17

### Documentation consistency finalization

Post-publication documentation audit completed after `OS_APPLICATION_AND_UI_MODEL_FOUNDATION` Gate 7. Canonical current-state docs now record Gates 0–7 as published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`, distinguish the accepted firmware/source baseline from the later docs-only architecture publication, and identify `BOOT_DESKTOP_UI_FOUNDATION` as the exact next boundary. Historical C3.6/C3.7/C3.8/C3.9/C4.0 plan/acceptance headers that still claimed implementation/publication was pending were corrected to their actual published commits; historical plan bodies remain unchanged. No firmware/source/header/linker/script/build mutation is part of this cleanup.

### Published — Boot / desktop UI foundation — Gates 0–7 accepted

Boundary: `BOOT_DESKTOP_UI_FOUNDATION`.

The accepted revision-2 implementation delivers `BOOT_SPLASH -> DESKTOP_HOME`: bootstrap renders `DEUS OS / STARTING / PLEASE WAIT`, task0 transitions to `DEUS OS / DESKTOP / READY` only after a minimum 1000 ms visible dwell plus production readiness, and the accepted status bar now carries real SYSTEM/USB/NETWORK plus monotonic uptime semantics. Task0 uses the existing timed wait with a 250 ms service bound; semantic snapshot suppression prevents idle polling redraws. A hardware-observed first-candidate defect was corrected by separating one-time/recovery SSD1306 initialization from steady-state panel refresh, so routine minute/status transitions never issue `display off`.

Accepted Gate 2 candidate: tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`; BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; ELF `70700` bytes / `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`; MAP SHA-256 `D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8`; Flash `41520 / 65536`; SRAM `9792 / 20480`. Gate 2 evidence SHA-256 `62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729`.

Gate 3 hardware acceptance passed on the exact candidate: no reflash was needed because target Flash already matched; boot/scheduler/IWDG startup, task margins (`640` / `424` bytes), CDC text pressure `128/128` with zero drops, UART pressure `128/128` with zero drops/errors, binary pressure `128/128` unique IDs, malformed/CRC/oversize/split-frame recovery, physical USB reconnect, authorized IWDG reboot/recovery, three USB enumerations and final exact Flash readback all passed. Gate 3 log SHA-256 `9481BEFADB5A8F1D17AF6FC0ACDE238FE66B6956940F56DC86A828CBFAA7F900`; Gate 3 evidence SHA-256 `2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD`.

Gate 4 physical OLED review is `PHYSICAL_OLED=PASS`: displayed digits and text update correctly, no unintended content is drawn, and no flicker/blank pulse/stale-pixel artifact is visible. Gate 5 documentation/evidence finalization changed docs only. Gate 6 committed the accepted 12-path source/docs candidate as `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`, tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`, subject `feat: add boot desktop UI foundation`; Gate 7 published it by ordinary non-force fast-forward and final local/remote ahead-behind is `0/0`.

### Published — OLED dirty-region optimization — Gates 0–7 accepted

Boundary: `OLED_DIRTY_REGION_OPTIMIZATION`.

The accepted implementation keeps one 512-byte framebuffer, adds bounded 16-bit dirty X spans, uses change-aware framebuffer writes, sends exact SSD1306 page/column windows, records bounded present telemetry and renders status components independently so ordinary minute/SYSTEM/USB changes do not clear/recompose the whole framebuffer or rerasterize console content. No shadow framebuffer, heap, DMA, new task/SVC/IPC primitive, geometry change, command/RPC renumbering or USB redesign was introduced.

Accepted Gate 2/3 candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`; BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`; ELF `71308` bytes / SHA-256 `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`; MAP SHA-256 `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`; Flash `44560 / 65536`; SRAM `9848 / 20480`; named persistent optimization metadata increment `55` bytes. Gate 2 evidence SHA-256 `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`.

Gate 3 hardware acceptance passed on the exact candidate. The historical full semantic refresh measured `572` I2C payload bytes / `36` writes; clean present measured zero traffic; a single changed byte measured `9` payload bytes; deterministic `00:00 -> 00:01` measured span `x=123..125` / `11` payload bytes; USB indicator transition measured `x=9..11` / `11` payload bytes. Task0 margin remained `328` bytes after both `oledstatus` and `oleddirty`; task1 margin remained `424` bytes. CDC/UART/binary pressure `128/128`, malformed-frame recovery, physical USB reconnect, authorized IWDG reboot/recovery and final exact Flash readback all passed. Gate 3 log SHA-256 `887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6`; Gate 3 evidence SHA-256 `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`.

Gate 4 is `PHYSICAL_OLED=PASS`: splash/home, real minute rollover, USB indicator transition and `uiruntime` restore were visually accepted with no blank/off pulse, flicker, stale pixels, dirty-span clipping or console corruption. Gate 4 evidence SHA-256 `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`.

Gate 5 finalized documentation/evidence, added repository LF policy through `.gitattributes`, and recorded the deferred measured-optimization / robustness / observability / test-profile / storage policy in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`. Gate 5 evidence SHA-256 `7DAB43E97E238DA35894B767AD1403064905E64D9326F8B4E002AE0816AB6436`. Gate 6 committed the exact reviewed source/docs set as `39690c9ef103cbcf93272df8bad0359a934b7dc1`, tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`, subject `feat: optimize OLED dirty region updates`; Gate 6 evidence SHA-256 `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`. Gate 7 published it by one ordinary non-force push; fresh fetch verified `HEAD == origin/main == FETCH_HEAD`, clean repository and ahead/behind `0/0`. Gate 7 evidence SHA-256 `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

### Planned — Application runtime foundation — Gate 0 accepted

Boundary: `APPLICATION_RUNTIME_FOUNDATION`.

Gate 0 freezes the first real application runtime on the accepted two-task substrate: static `system.home=0x0001` and `device.info=0x0002`, one foreground application, explicit REGISTERED/STOPPED/STARTING/RUNNING/BLOCKED/STOPPING/FAILED lifecycle, task0-only callback/event dispatch, fixed pointer-free semantic events separate from scheduler wake bits, a bounded system/time/USB/network snapshot, and exactly three application content rows under the system-owned status bar. `system.home` preserves `DEUS OS / DESKTOP / READY`; `device.info` uses `DEUS OS / DEVICE INFO / STM32F103`.

The existing RPC IDs `0x0001..0x0020` remain frozen. Gate 1 will append only `applist=0x0021`, `appstart=0x0022`, `appstop=0x0023`, advance `COMMAND_SERVICE_FOUNDATION_VERSION` to `2`, and retain binary framing protocol v1. Initial source boundary is new `include/kernel/application_runtime.h`, new `src/kernel/application_runtime.c`, plus `include/kernel/command_service.h`, `src/kernel/command_service.c`, and `src/kernel.c`. No heap, queue, mutex, generic timer, new task/SVC, dynamic loader, filesystem or USB redesign is authorized. Gate 2 budgets are Flash <= `48656`, SRAM <= `10104`, task stacks exactly `1024 / 512`, and hardware runtime margins >= `256` bytes.

## 2026-09-16

### Published — Native STM32F103 USB Device core foundation

Boundary:
`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Accepted candidate:

- source tree `4b798382ef843ef8f488115624d48c0cd1506c75`;
- binary `43812` bytes;
- SHA-256 `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`.

Accepted architecture:

- direct-register STM32F103 USB FS Device on PA11/PA12;
- accepted 72 MHz SYSCLK retained; RCC `USBPRE=0` gives 48 MHz USB clock;
- IRQ20 `USB_LP_CAN1_RX0`;
- explicit BTABLE/PMA/EP0 ownership;
- bounded standard-control request engine with delayed `SET_ADDRESS` semantics;
- development identity `VID 0x1209 / PID 0x000A`, private testing only;
- minimal vendor-specific interface with no non-control endpoints;
- no CDC ACM, new task, SVC, IPC, generic timer subsystem, runtime-statistics subsystem or OLED change.

Hardware proof:

- exact device/configuration descriptors read by Windows through EP0 control transfers;
- initial addressed state `20`, physical reconnect addressed state `21`;
- physical disconnect/reconnect recovered without reflashing;
- post-IWDG USB recovery passed;
- destructive IWDG reboot `8748 ms`, post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- safe surface pre/post `20/20`;
- timed event controlled RX wake `34 ms` pre-IWDG and `35 ms` post-IWDG;
- invasive scheduler diagnostics `8/8 BUSY`;
- UART race `128/128 PONG`, zero RX drops/errors;
- final Flash readback exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Evidence:

- Gate 2 build evidence `5E261F473EA48CAA7ABD2B080E22732D95EEEA24BEA76349A4A10E66088FC81F`;
- Gate 3 hardware log `F612D710A6029ADACD0AE51BF5F85B2F5983F22003A6C09787C99045C520ECEC`;
- Gate 3 hardware evidence `14E7751AFBACEED96DF5816B81969AB33C499FAF182F19BBA89A22A21E2C6B43`;
- Gate 4 disposition `FCC6B971D7CCBDDA7866CBD92B56D33955D328EF1D368690B132E06A6801F676`.

Published commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`, tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`, subject `feat: add native USB device core foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — USB CDC ACM diagnostic/command console

Boundary: `USB_CDC_ACM_CONSOLE_FOUNDATION`.

Accepted candidate:

- tested source tree `d815a8357b9f77850c08ff071f47be8d2b5d4b53`;
- binary `58548` bytes;
- SHA-256 `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- Flash used `58548` bytes; SRAM used `9200` bytes.

Accepted architecture and hardware proof:

- private-test identity `1209:000B`, product `Deus OS CDC Console`;
- Windows inbox `usbser.sys`, no custom INF, configuration `1`;
- CDC Control interface 0 + CDC Data interface 1;
- EP1 `0x81` interrupt IN, EP2 `0x02` bulk OUT, EP3 `0x83` bulk IN;
- bounded EP0 OUT data stage plus `SET_LINE_CODING`, `GET_LINE_CODING`, and `SET_CONTROL_LINE_STATE`;
- CDC RX/TX rings `1024` / `2048` bytes;
- task0 owns UART + CDC command work with independent parser state and origin-bound responses; no third task/SVC/IPC;
- corrected USB init forces a bounded PA12/D+ low disconnect pulse so MCU/IWDG reset produces a fresh host-visible attach;
- automatic CDC reopen after software reset PASS; physical micro-USB reconnect PASS; post-IWDG automatic `usbser` reopen PASS;
- CDC safe surface `20/20` pre-IWDG and `20/20` post-IWDG;
- scheduler diagnostics `8/8 BUSY`, packet-boundary proof `38/38`, CDC pressure `128/128` with zero drops, UART pressure `128/128`, dual-transport parser/response isolation PASS;
- real IWDG reboot `9028 ms`, post-reset UART + CDC recovery PASS;
- final Flash readback exact.

Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- corrected Gate 2 evidence `49AAFBAFEA0B895DFBEBA7311335069EFD512640A13F200D1F3B9D65D6774869`;
- Gate 3 hardware log `D009F0D7E12C10C3FBA40037C6EC485E9FA27674901C5C3D6C4AAE72C0D42F35`;
- Gate 3 hardware evidence `6035C6FD5C0718252B28AAC07CD67C879AA1F14A639DD9A7854D3B74F54D49CB`.

Published commit `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`, tree `bbc6b24e28279075c41410e8abdace89c4b805b7`, subject `feat: add USB CDC ACM console foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — transport-neutral shell/RPC foundation

Boundary: `SHELL_RPC_FOUNDATION`.

Gate 0 establishes a static allocation-free command service above the already accepted UART and USB CDC transports: one deterministic registry, semantic status codes, generic response sink/context, bounded argument tokenization, exact legacy command compatibility, and `help`/`rpcinfo` introspection. Binary framing, host control application, firmware update, bootloader, new tasks/SVC/IPC, and OLED changes remain out of scope.

Canonical planning: `docs/SHELL_RPC_FOUNDATION_PLAN.md` and `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`.

Gates 0–7 are accepted and published. Gate 2 produced candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`, BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`, Flash `37196` bytes and SRAM `9216` bytes. Gate 3 proved the shared command service over both UART and CDC: deterministic 32-method registry, `help`, `help ping`, `rpcinfo`, exact legacy `ERR` behavior, tabs/backspace/DEL/overflow recovery, origin-bound replies, UART+CDC `schedtimed`, retained `20/20` CDC safe surface pre/post IWDG, `8/8` scheduler BUSY, `38/38` packet-boundary PONG, `128/128` CDC pressure, `128/128` UART pressure, physical USB reconnect, real IWDG reboot with automatic CDC reopen, and final exact Flash readback. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`. Published commit `0c33304d2db86e54d715905393f49147bb6dd2ea`, tree `19ac9b95caca09e991842f6f9963864934b0a334`, subject `feat: add transport-neutral shell RPC foundation`; local/remote ahead-behind `0/0`.

### Published — binary framed transport foundation

Boundary: `BINARY_FRAMED_TRANSPORT_FOUNDATION`.

Gates 0–7 are accepted and published. Protocol v1 runs over the existing USB CDC stream with `A5 5A` magic, little-endian fields, request-ID correlation, CRC-16/CCITT-FALSE, stable explicit 16-bit RPC IDs for all 32 command-service methods, bounded four-argument adaptation, 48-byte response-data chunks, structured final status, explicit destructive authorization, deterministic text/binary coexistence, and nonblocking all-or-none CDC frame enqueue.

Accepted tested candidate:

- source candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- ELF `65876` bytes / SHA-256 `4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`.

Gate 3 hardware proof:

- Windows `usbser` and retained CDC class controls PASS;
- binary HELLO/request-ID echo and exact `help` chunking/accounting PASS;
- binary safe core `21/21`, scheduler diagnostics `8/8 BUSY`;
- unknown ID, BAD_ARGS, destructive deny/allow, bad CRC, oversize resync and arbitrary split writes PASS;
- partial text preservation and binary/text parser isolation PASS;
- binary pressure `128/128` unique IDs with zero CDC drops;
- retained CDC pressure `128/128`, UART race `128/128`;
- physical micro-USB reconnect restores text + binary without reflash;
- authorized binary `wdogtrip` produced a real IWDG reboot in `7433 ms`; post-reset UART + CDC text + binary recovered automatically;
- final Flash readback exactly matched the Gate 2 candidate.

Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- Gate 2 evidence `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- Gate 3 hardware log `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- Gate 3 hardware evidence `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`.

Gate 5 documentation/evidence finalization is accepted. Canonical protocol/design/acceptance: `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`, `docs/BINARY_FRAMED_TRANSPORT_PLAN.md`, and `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`.

Published commit `2fde9025a51021511e73a76b561f7983ca655e2f`, tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`, subject `feat: add binary framed transport foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — Deus OS product/application/UI model foundation

Boundary: `OS_APPLICATION_AND_UI_MODEL_FOUNDATION`.

This documentation-only architecture boundary defines Deus OS as an independently operating embedded device runtime, freezes the first static application/lifecycle model, defines boot splash -> desktop/home -> application-view ownership, assigns real SYSTEM/USB/NETWORK and uptime semantics to the current status bar, separates target applications from host Control Panel plugins/packages, and keeps native dynamic ARM loading, filesystem, update protocol and bootloader deferred.

Canonical planning:

- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`;
- `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`.

Gates 0–7 are accepted. The docs-only foundation was committed as `3dac2c4528fc77e87e1374ff47f56223d2b44e2c` (`docs: freeze application and ui model foundation`), tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`, and published by ordinary non-force fast-forward. After this foundation, exact firmware order is `BOOT_DESKTOP_UI_FOUNDATION` -> `APPLICATION_RUNTIME_FOUNDATION` -> `HOST_CONTROL_APPLICATION_FOUNDATION`.

A foundation completeness review is now canonical in `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`. It records that no kernel blocker requires speculative RTOS work before that sequence, while explicitly retaining later contracts for semantic application events, system/build/platform capability identity, bounded persistent-state safety, retained crash/reset observability, structured telemetry, security/trust before network mutation or executable update, controlled update reboot handoff, and portability layering. Generic timers, message queues, synchronization and runtime statistics remain consumer-driven; heap/filesystem/RTC/DMA/MPU/general power-management facilities remain deferred until a real product requirement justifies them.

## 2026-09-15


### Published — C4.0 IWDG production liveness foundation

Published commit:
`3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`

Published tree:
`e024d425e97f878c170ccfc41f0505dce79277a3`

Subject:
`feat: add IWDG liveness foundation`

Accepted firmware:
`25192` bytes /
`4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D`.

Publication:
ordinary non-force fast-forward; local/remote ahead-behind `0/0`.

### Planned — Native STM32F103 USB Device core foundation

Boundary:
`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Roadmap basis:

- minimal STM32F103 USB Device core on PA11/PA12 comes before CDC ACM;
- CDC ACM remains the next class/transport slice after the core;
- generic timer callbacks remain deferred until a real consumer;
- queues/synchronization remain deferred until a real shared-ownership boundary;
- runtime statistics remain a later observability slice;
- OLED/status-bar remains frozen.

Initial core scope:

- direct-register USB FS device support, no HAL;
- preserve 72 MHz SYSCLK and derive the required USB 48 MHz clock;
- USB reset + low-priority interrupt ownership;
- PMA/BTABLE foundation;
- endpoint 0 control-transfer state machine;
- standard control requests needed for minimal enumeration;
- centralized USB descriptor identity with no arbitrary third-party VID/PID;
- UART/ST-LINK recovery paths retained.

Power constraint:
during native USB hardware testing, do not power the board simultaneously from
ST-LINK 3.3 V and micro-USB VBUS.

No source implementation is authorized by this planning gate.

### Published — C4.0 IWDG production liveness foundation

Boundary:
`IWDG_LIVENESS_FOUNDATION_C4_0`

Published parent:
`39ea5b3d1b72fca8d15e22e7544870ab0704c274` (`feat: add production heartbeat task`)

Accepted candidate:

```text
build\iwdg_liveness_foundation_v2\os.bin
25192 bytes
4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D
```

Accepted architecture:

- register-level STM32F103 IWDG on independent LSI;
- prescaler `/256` (code `6`), reload `1249`, nominal approximately `8 s`;
- repaired sequence `START -> unlock -> PR/RLR -> wait PVU/RVU -> reload`;
- reset cause captured before reset flags are cleared;
- watchdog starts immediately before normal scheduler ownership;
- reload only from concrete Thread/PSP production progress;
- SysTick/USART IRQ/Handler/fault paths never reload;
- destructive `wdogtrip` never reloads;
- no scheduler-core change, no new SVC, no IPC, no OLED/gfx/status-bar edit.

Hardware proof:

- normal reload count `39 -> 50`;
- real IWDG reboot after `7294 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- post-reset heartbeat delta `3`;
- safe `20/20`, timed `4/4`, diagnostics `8/8 BUSY`;
- UART race `128/128 PONG`;
- task0 stack `604 used / 420 margin`;
- task1 stack `80 used / 432 margin`;
- MSP `348 used / 1636 margin`;
- RX drop/error/depth `0/0/0`;
- final Flash exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Evidence:

- build log `D7034BB4597EEF3C5CF1CA6025BAA148BCAD1F21D9534009EF242EAD334A0421`;
- build evidence `96773CADE1440BA844FE1455E049109C4099318F57A8F906D7F28148471E5709`;
- hardware log `6C613D31AEF61968021288F41EED3D28EE8B14DBB8C7A129B21636858676B2D9`;
- hardware evidence `08410EF7BC5F330CF2D18BD7CEDF5E85D83FCD4A825C3D5BD0D0C1E4F7D24F66`;
- Gate 4 disposition `3B69B950D80AB8402D873E02567BEF4976E846CC585EE680CCD852B4E96778AE`.

Gates 0–7 are accepted. Published at `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca` by ordinary non-force fast-forward.

### Published — C3.9 production heartbeat task ownership

Status: Gates 0–7 accepted and published at `39ea5b3d1b72fca8d15e22e7544870ab0704c274`.

Accepted candidate:

```text
build\production_heartbeat_task_v1\os.bin
24648 bytes
4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86
```

Accepted architecture:

- task0 remains console/runtime, priority `128`, stack `1024 B`;
- task1 owns normal-runtime PC13 heartbeat, priority `255`, stack `512 B`;
- heartbeat blocks with `scheduler_sleep_ms(500)`;
- SysTick retains only kernel time + `scheduler_tick()`;
- fatal-fault PC13 blink remains an out-of-band diagnostic exception;
- no IPC, no new SVC, cooperative production retained.

Hardware proof:

- heartbeat count `3 -> 10`, delta `7`;
- PC13: `5` transitions, both ODR states observed;
- heartbeat stack `80 used / 432 margin`;
- console stack `616 used / 408 margin`;
- fixed-priority self-test `0x0000003F`;
- task0 `128 -> 128`, task1 `255 -> 255`;
- safe surface `20/20`;
- timed blocking `4/4`;
- invasive diagnostics `8/8 BUSY`;
- retained `4 x 32 = 128/128 PONG`;
- RX `drop/error/depth = 0/0/0`;
- MSP `340 used / 1644 margin`;
- `OLED_RUNTIME_UI_OK`;
- final Flash identity exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Publication completed at `39ea5b3d1b72fca8d15e22e7544870ab0704c274` by ordinary non-force fast-forward.
### Accepted — C3.8 static fixed-priority scheduling

Accepted candidate:

- `build\scheduler_fixed_priority_v1\os.bin`
- `23792` bytes
- SHA-256 `492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F`
- `src/kernel.c` `235813864C83E7E813A31288DBE45635AF9948213CC3352A039FC4AC31D82A8D`
- `src/kernel/scheduler.c` `B59B08373662B841C2CC077C92DE18D7FA21DA6DCE4E1DEE435F86AB58566C88`
- `include/kernel/scheduler.h` `A48DCBB90D08FAD03F2D426AA2A129A8D8858204380D4A518D7094A318F5D841`

Architecture accepted:

- static priority range `0..255`, default `128`;
- lower value = higher priority;
- inactive-only `scheduler_task_priority_set()`;
- one shared priority-aware READY selector;
- equal priorities retain round-robin tie behavior;
- cooperative production remains cooperative;
- task0 DEFAULT, task1 UNUSED;
- no new SVC;
- timed/event/WFE/PRIMASK architecture retained.

Hardware accepted:

- `schedprio` `1/1`;
- selector self-test `0x0000003F`;
- active mutation rejected and task0 remains `128`;
- safe surface `20/20`;
- timed blocking `4/4`;
- `8/8` invasive diagnostics BUSY;
- retained `4 x 32`, `128/128 PONG`;
- RX drops/errors/depth `0/0/0`;
- production stack `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact.

OLED Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- build log `306AC5D97125F5BEFB4B2DB95E602ED8F6541E32CF364AA61ACD3D93A0E946D0`;
- build evidence `33E699282A456B79A0F15C8B2B6DF756BAD5979867091863599575A851CEA000`;
- hardware log `005D646FD613506896BC1A3961DDA752D9D424AD7F4AD0CFAAEE377C50E05222`;
- hardware evidence `41665A892477FB195D00DD722A093F69B09AB2A66669EB872DDB7A3518EACC6C`.

Published as `0312bb376c365c0235b9cafe22e927254528f2c4` (`feat: add fixed-priority scheduler policy`).
## 2026-09-14

### Accepted — C3.7 SysTick-backed timed blocking / sleep foundation

Accepted candidate:

- `build\scheduler_timed_blocking_v1\os.bin`
- `22900` bytes
- SHA-256 `366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A`
- `src/kernel.c` `44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC`
- `src/kernel/scheduler.c` `90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618`
- `include/kernel/scheduler.h` `F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547`

Architecture accepted:

- one existing `BLOCKED` state with deadline metadata; no sleeping state;
- `scheduler_sleep_ms()` and `scheduler_wait_events_timeout()`;
- SVC 4 timed blocking;
- `kernel_ticks` remains time authority;
- wrap-safe signed-delta expiry with maximum timeout `0x7FFFFFFF ms`;
- event/timeout arbitration is first atomic `BLOCKED -> READY` transition wins;
- timeout wake publishes READY before `SEV`;
- production UART wait remains untimed;
- ring is payload; event is notification.

Hardware accepted:

- `schedtimed` `4/4` phases;
- sleep `50 ms`, timeout `50 ms`;
- external UART event wake `164 ms`, event `0x00000001`;
- timed RX delta `26`, host idle delta `2904`;
- no double wake; injected `ping` survives in ring and yields `PONG`;
- safe surface `19/19`;
- invasive scheduler diagnostics `8/8` BUSY;
- retained `4 x 32` burst regression, `128/128 PONG`;
- RX drops/errors/depth `0/0/0`;
- production stack `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact;
- OLED runtime restore PASS;
- physical OLED PASS.

Evidence:

- build log `6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84`;
- build evidence `431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199`;
- hardware log `3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769`;
- hardware evidence `ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4`.

Published as `90df6a690230c9800c0d8497d5597f87a5ae0409` (`feat: add scheduler timed blocking foundation`).
### Accepted — normal-boot production task ownership + atomic host-idle race closure

Accepted C3.6 candidate:

- path: `build\normal_boot_production_ownership_atomic_racefix_v1\os.bin`
- bytes: `21832`
- SHA-256: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`
- scheduler core: `src/kernel/scheduler.c` SHA-256 `FA649EF24569AEE653A1CA022778237F76FF236A3F0B1B38FDDA8FB06F867649`
- production ownership source: `src/kernel.c` SHA-256 `FE046521F7A017B3DE204E984ECC392CA471C82824530B00EFCBFDFA433658F1`

- Normal boot now automatically starts one cooperative production console/runtime task on PSP.
- Slot 0 owns production console/runtime work; slot 1 remains `UNUSED`.
- MSP is bootstrap-complete scheduler host/idle + exception stack and uses `WFE` when no task is READY.
- The hardware-discovered READY/BLOCKED terminal-classification race was closed by PRIMASK-atomic host classification in `scheduler_start_mode()`.
- Full safe production command surface: `18/18` PASS.
- Invasive scheduler commands: `8/8` exact `SCHED_DIAG_BUSY`.
- Atomic regression: `4 x 32` unpaced `ping`, `128/128 PONG`, no scheduler return/fatal path.
- RX drops/errors/final depth = `0/0/0`; burst high-water = `4`.
- Production stack = `580 used / 444 free`, canary intact.
- MSP = `320 used / 1664 free`, canary intact.
- Final Flash readback exactly matched the accepted candidate.
- Automated OLED regression + `uiruntime` PASS.
- Physical frozen OLED `DEUS OS / BOOT OK / READY`: `OLED PASS`.
- Local acceptance commit and non-force publication are the only remaining gates.

## 2026-09-13

### Published — scheduler steady-state wait/wake foundation

- Published commit `bfed76e0de1c52bf65e60f66c029cc41710fabd0` (`feat: add scheduler steady-state wait/wake foundation`) by normal non-force fast-forward.
- Published candidate `19932 bytes`, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`.
- Added explicit BLOCKED task state, SVC event wait, ISR-safe event wake, pending-event race closure and host-MSP WFE idle.
- UART RX event publication occurs only after ring insertion; event consumers re-check FIFO/condition after wake.
- Hardware acceptance: 4/4 UART IRQ wait/wake rounds PASS; lifecycle/RX/MSP/OLED regression PASS; physical OLED PASS.
- Worktree/index clean after publication.

### Hardware discovery / scope reclassification — normal-boot production ownership

- Gate 1 source implementation completed in `src/kernel.c`; Gate 2 fresh GNU validation PASS.
- Initial production candidate: `21804 bytes`, SHA-256 `609BFB2216B178C59E6BC7D0F04F4986571465A24C820E3A34E575DB768858E3`.
- Hardware v1 proved automatic cooperative production ownership, PSP execution, host-MSP WFE idle, all 18 safe commands, and all 8 invasive diagnostics blocked with `SCHED_DIAG_BUSY`.
- Before burst pressure: production stack high-water `580 / 1024`, free margin `444`, canary intact; RX drops/errors were `0`.
- UART burst then exposed a scheduler host-idle TOCTOU race: a task can transition `BLOCKED -> READY` between the host's first READY scan and its BLOCKED scan, causing a false `scheduler_abort_run()` and `scheduler_start()` return `0`.
- Observed failure signature: 14 `PONG`, then `SCHED_PROD_RETURN=0x00000000`, `SCHED_PROD_UNEXPECTED_RETURN`, `SCHED_PROD_FATAL`.
- This is a real scheduler-core correctness blocker, not a harness failure.
- C3.6 source scope is therefore reclassified from `src/kernel.c` only to `src/kernel.c` plus a minimal `src/kernel/scheduler.c` host-idle race fix.
- Gate 2B linked the first-pass second READY scan, but static review found a smaller remaining IRQ window before `scheduler_abort_run()`. Final closure requires atomic READY/BLOCKED terminal classification under `PRIMASK`, with terminal abort committed before interrupt restore.
- `include/kernel/scheduler.h`, startup, linker and frozen OLED implementation remain guards.
- Regression hardware acceptance must repeat burst rounds and prove no unexpected scheduler return/fatal path.

### Planning baseline — normal-boot production task ownership migration

- Canonical next boundary is `NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_MIGRATION`.
- Added detailed ownership design and acceptance plans.
- Chosen first production topology: one cooperative console/runtime task on PSP, slot 1 UNUSED.
- Reuse accepted 1024-byte console PSP allocation; migration must re-prove >=256-byte free margin.
- Preserve USART1 IRQ sole-DR-reader/ring ownership and drain-first event consumer semantics.
- MSP becomes scheduler host/idle + exception stack after bootstrap; no dummy idle task.
- Initial OLED UI remains bootstrap work; runtime OLED/I2C application calls move with console ownership to PSP.
- All eight console-visible invasive scheduler diagnostics remain blocked with `SCHED_DIAG_BUSY` while production scheduler is active.
- `sleep()`/timed waits, priorities, second production task, OLED task and IPC remain deferred.
- Original target source boundary was `src/kernel.c` only; hardware v1 reclassified C3.6 to `src/kernel.c` plus a minimal `src/kernel/scheduler.c` host-idle race fix. Scheduler header, startup/linker and frozen OLED modules remain guards.

### Hardware + physical accepted — production scheduler lifecycle / diagnostic isolation

- Published baseline before this delta: `8bc1510265d0377adafda663d320c1e55f5b2c3a` (`feat: add console PSP stack-budget probe`).
- `scheduler_init()` is now status-returning and rejects reset while active before any scheduler-global mutation.
- Added read-only `scheduler_is_active()`.
- Centralized scheduler diagnostics behind an active-lifecycle gate; six published invasive diagnostics return exact `SCHED_DIAG_BUSY` while active and preserve idle behavior.
- Added offline `schedisolate` real-preemption diagnostic.
- Accepted source delta remains exactly `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- Candidate `18324 bytes`, SHA-256 `9AFDE9AC5AF196E98A2896BAC0DD7AE610414FB5A2888F1D499D2A5E0A12794B`.
- `.bss=5248 bytes`, `_ebss=0x20000C80`, RAM gap below MSP `15232 bytes`, `fault_record=0x20000764`, probe stack `0x20000060 / 1024 bytes`.
- `4/4` isolation rounds PASS; every round had six busy lines, `INIT_REJECT=1`, `BLOCKED_DIAGNOSTICS=6`, `ACTIVE_PRESERVED=1`, `OVERLAP=1`, both canaries intact.
- Isolation stacks: `160 / 512` and `88 / 512`; switches `16`.
- All six published scheduler diagnostics PASS before and after isolation.
- Console PSP regression: standalone `2/2`; composite `4/4`; exact `+105` IRQ / `+105` byte deltas in every composite.
- Console PSP high-water `560 / 1024`, minimum margin `464`; peer `88 / 512`; maximum switches `693`.
- RX high-water `80 / 128`, zero drops/errors, depth zero after stress.
- MSP high-water `320 / 1984`, minimum margin `1664`, canary intact.
- Fresh reset isolation + console probe PASS; final exact flash identity PASS; runtime failures `0`.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Normal boot remains MSP-owned and production scheduler remains inactive during normal boot.
- Next active scheduler gate: steady-state wait/wake foundation; normal-boot migration remains later.

### Hardware + physical accepted — console PSP stack-budget foundation

- Added inactive-only external task-stack binding while preserving the legacy internal scheduler stack pool at `2 x 512 bytes`.
- Added a dedicated aligned `1024-byte` console PSP measurement stack.
- Split safe named console dispatch from scheduler diagnostics; the accepted PSP surface is exactly `17` non-scheduler commands.
- Increased `UART_COMMAND_CAPACITY` from `16` to `32` after hardware v1 proved the 17-character `schedconsoleprobe` command could not reach dispatch with the old parser capacity.
- Accepted source delta: `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- Candidate `17028 bytes`, SHA-256 `13539E4F0C422FF0E3C373EF4167F3238FFA6D9A1B09F309C1A07787F2596C7F`.
- `.bss=5224 bytes`, `_ebss=0x20000C68`, RAM gap below MSP reservation `15256 bytes`, `fault_record=0x2000074C`, probe stack `0x20000048 / 1024 bytes`.
- Hardware v3: `4/4` standalone and `8/8` composite probes passed.
- PSP high-water `600 / 1024 bytes`; minimum margin `424 bytes` >= `256-byte` acceptance floor.
- Peer high-water `88 / 512 bytes`; maximum switches `693`; overlap `1`; both canaries intact.
- Exact safe console surface `17/17` completed.
- RX high-water `80 / 128`; drops/errors `0`; depth returned to zero; all `8/8` composites produced exact `+105` IRQ / `+105` byte deltas.
- MSP high-water `360 / 1984`; minimum margin `1624`; canary intact.
- Fresh-reset probe and exact fresh `+105` delta passed.
- Legacy scheduler, health, I2C/OLED regressions and final exact flash identity passed.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Accepted sizing decision: `1024 bytes` for the exact tested 17-command safe PSP surface; 512-byte full-console size remains rejected.
- Normal boot remains MSP-owned; lifecycle/diagnostic isolation is now accepted above; steady-state wait/wake is the next scheduler gate.

### Planned host application naming

- Native USB direction remains STM32F103 USB Device -> CDC ACM -> shell/RPC -> binary transport -> host control/update tooling.
- Provisional cross-platform Windows/Linux host application name: **Deus OS CP** (`Deus OS Control Panel`).
- The name may be changed later without changing the protocol/transport architecture.
- UART remains the emergency console and ST-LINK remains recovery/debug access.

### Hardware + physical accepted — MSP runtime high-water / guard foundation

- Reserved the upper `2048 bytes` of SRAM as a dedicated MSP region:
  - bottom `64 bytes` are the guard/canary area
  - upper `1984 bytes` are the measurable watermark capacity
  - `_smsp_stack=0x20004800`
  - `_emsp_guard=0x20004840`
  - `_estack=0x20005000`.
- `Reset_Handler` initializes the MSP guard and watermark before its first `BL kernel_main`, so boot/runtime Thread-mode MSP usage is included in the measurement.
- Added read-only `mspstat -> MSP_STACK_OK` telemetry:
  - reserved size
  - guard size
  - usable capacity
  - high-water used bytes
  - remaining margin
  - current MSP use
  - canary state.
- Accepted source delta is exactly `src/kernel.c` + `src/startup.s` + `linker/stm32f103c8.ld`.
- Candidate:
  - `15620 bytes`
  - SHA-256 `C15634184CAB7BA3C5CE503773EB7BA4BB45DDB4B32A3D399F6BE587C518D907`
  - MSP reserved `2048 bytes`
  - MSP usable capacity `1984 bytes`
  - SRAM gap below MSP reservation `16320 bytes`.
- Hardware proof:
  - initial MSP high-water `400 bytes`; margin `1584 bytes`
  - OLED/I2C regression raised high-water to `568 bytes`
  - accepted maximum high-water `596 bytes`
  - minimum observed margin `1388 bytes`
  - canary remained intact
  - `12/12` composite nested-pressure rounds passed
  - pressure classes: `oledstatus`, `schedpreempt`, `schedworkload`
  - each composite included `16 x ping`
  - RX ring high-water reached `80 / 128 bytes`
  - RX drops `0`, errors `0`, depth `0` after accepted composites
  - fresh-reset MSP pressure accepted at `572 bytes` used / `1412 bytes` margin.
- Scheduler/workload/health/I2C/OLED regressions remained PASS.
- Final exact flash readback matched the candidate.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Console ownership is still MSP; the production scheduler is still not started during normal boot.

### Hardware + physical accepted — USART1 RX IRQ / 128-byte ring-buffer foundation

- Replaced direct polling reads of `USART1_DR` with IRQ-driven receive ownership:
  - `USART1_IRQHandler` is the sole `USART1_DR` reader
  - external IRQ37 is wired in the real vector table
  - RXNE interrupt enabled; USART1 NVIC priority `0x80`.
- Added a 128-byte single-producer/single-consumer RX ring.
- Kept the existing `uart_try_getc()` API as the consumer path, so the console remains MSP-owned in this slice.
- Restored `WFI` idle in `kernel_main()`; SysTick and USART1 interrupts wake the core.
- Added read-only `rxstat -> RX_IRQ_RING_OK` telemetry:
  - capacity
  - IRQ count
  - received byte count
  - drop count
  - error count
  - high-water
  - current depth.
- Accepted source delta is exactly `src/kernel.c` + `src/startup.s`.
- Candidate:
  - `15084 bytes`
  - SHA-256 `E25DC54C149EB9DA7B26F5378868DAA790847A1C4C7B4CFB970F294CFB738EFB`
  - `.bss=2112 bytes`
  - `_ebss=0x20000840`
  - SRAM headroom `18368 bytes`
  - linked `fault_record=0x20000320`
  - USART1 IRQ own static frame `12 bytes`.
- Hardware burst proof:
  - `4 x 32` `ping` commands: every round returned exact `32/32` `PONG`
  - each burst plus its `rxstat` snapshot produced exact `+167` IRQ and `+167` byte deltas
  - observed RX ring high-water `29 / 128 bytes`
  - zero drops
  - zero RX errors
  - depth returned to zero after every accepted burst
  - fresh reset repeated exact `167` IRQ / `167` bytes with high-water `29`.
- Scheduler, substantive PSP workload, health, I2C and full OLED regressions remained PASS.
- Final flash readback exactly matched the accepted candidate.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- This slice does **not** start the production scheduler or migrate the normal console to PSP.

### Architecture decision after substantive PSP workload

- Substantive PSP workload is published as `8f6b922a7d2e55abc3133702e7571057f995da5d`.
- Direct normal-boot migration was rejected until prerequisite ownership/stack issues are separated.
- The full current console linked feasibility estimate is `524 bytes`; with the 64-byte context reserve it is `588 bytes`, so a 512-byte console PSP stack is not accepted.
- Scheduler self-tests currently reinitialize global scheduler state and cannot safely run nested inside an active production scheduler.
- Kernel/MSP runtime stack budget was unproven at the decision-audit boundary.
- USART1 polling RX was selected as the first prerequisite and is closed by the IRQ/ring-buffer hardware acceptance above.
- The separate MSP runtime budget prerequisite is now closed by the measured guard/high-water acceptance above.

### Next

- Prove the console PSP workload stack budget explicitly; the prior `524 + 64 = 588-byte` feasibility result still rejects a 512-byte full-console PSP stack.
- Keep console ownership on MSP until that runtime workload-sizing gate passes.
- Keep production scheduler lifecycle/diagnostic isolation and normal-boot task migration as separate later gates.

### Accepted / published — substantive preemptive PSP OLED workload

- Added command-gated `schedworkload -> SCHED_WORKLOAD_OK`.
- Added public `scheduler_start_preemptive()` wrapper and read-only PendSV switch-count telemetry.
- Workload task 0 executes the frozen OLED runtime full render/present path on PSP.
- Workload task 1 is CPU-only, performs no UART/I2C/OLED access, and does not voluntarily yield.
- Task stacks remain exactly `2 x 512 bytes`.
- Static feasibility evidence:
  - planning estimate for `oled_runtime_ui_show()`: `212 + 64 = 276 bytes`; margin `236 bytes`
  - source-build workload task 0 estimate: `220 + 64 = 284 bytes`; margin `228 bytes`
  - source-build workload task 1 estimate: `12 + 64 = 76 bytes`; margin `436 bytes`.
- Real hardware high-water:
  - workload task 0: `328 bytes`; measured free margin `184 bytes`
  - workload task 1: `80 bytes`; measured free margin `432 bytes`
  - capacity: `512 bytes` per task
  - canaries intact for both tasks in every accepted run.
- Real preemption proof:
  - `WORKLOAD_UI_RESULT=1`
  - `WORKLOAD_PEER_OVERLAP=1`
  - PendSV switches observed `124..126`
  - first workload PASS
  - 32/32 workload stress commands PASS
  - 4/4 return-to-kernel checkpoint pings PASS
  - final workload PASS
  - total accepted workload commands: `34`.
- Full scheduler/UART/I2C/OLED regression remained valid and final exact flash identity matched the candidate.
- Hardware v1 boot checks were false negatives caused by stale harness expectation `FAULTREC=0x20000270`.
- Linked `fault_record` is `0x20000284`; recovery v2:
  - proved the only two v1 false results were the boot matcher
  - revalidated both captured v1 boot frames
  - passed two fresh corrected boot checks
  - revalidated `schedworkload`, scheduler regressions, I2C, health, runtime UI, and exact target readback without reflashing.
- Accepted candidate binary: `14292 bytes`.
- SHA-256: `8C124B0954D65E0F698AD1C62525E72FC4F569D5133EF8F0CE67A8297A80A2CF`.
- `.bss=1952 bytes`; `_ebss=0x200007A0`; SRAM headroom `18528 bytes`.
- Physical frozen OLED output confirmed unchanged: `DEUS OS / BOOT OK / READY`.

The runtime `328-byte` task-0 high-water exceeds the `.su`-based `284-byte` source-build estimate. Therefore that static call-chain method is a feasibility estimate, not a conservative stack upper bound. Runtime watermark/canary evidence is authoritative for sizing.

For the exact tested frozen OLED render/present workload, the current 512-byte PSP stack has `184 bytes` (35.9%) measured margin. Console-task and MSP/kernel stack requirements remain separate open proofs.

### Next

- Finalize the production task ownership/stack budget from the accepted runtime evidence.
- Prove any console-task stack and MSP/kernel stack budgets separately.
- Keep normal-boot scheduler ownership migration deferred until those budgets and the migration design are explicit.
- Preserve `schedtest`, `schedcoop`, `schedpreempt`, `schedstack`, and `schedworkload` as regression gates.

## 2026-09-12


### Accepted — OLED runtime lifecycle (Slice 8)

- Frozen 128x32 OLED UI is initialized as part of the normal boot/runtime lifecycle.
- Boot UART contract includes `OLED_RUNTIME_UI_OK`.
- Full UART/OLED regression passed.
- Physical OLED result accepted.
- Accepted binary: `10148 bytes`.
- SHA-256: `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`.
- Acceptance commit: `4217865403d9725707f4c17e572317caf7fe733f`.

### Accepted — scheduler foundation (Slice 9A)

- Added `include/kernel/scheduler.h` and `src/kernel/scheduler.c`.
- Added two static TCBs, two 512-byte static task stacks, and synthetic initial Cortex-M task frames.
- Added deterministic foundation self-test:
  - `schedtest -> SCHED_FOUNDATION_OK`.
- Accepted binary: `10752 bytes`.
- SHA-256: `29CA6F248B94A861497D2A97C723B4E945208FC4002C363956753599AE38BFBC`.
- Acceptance commit: `1a57f79cda42674219e774900ce07a0da8fedaf4`.

### Accepted / published — cooperative scheduler activation (Slice 9B)

- Corrected initial PC / stacked LR Thumb semantics.
- Activated task Thread-mode execution on PSP.
- SVC `#0/#1/#2` provide start / voluntary yield / task-return exit.
- Added kernel/MSP parking and restoration.
- `schedcoop -> SCHED_COOP_OK` validates `0x10 -> 0x20 -> 0x11 -> 0x21`.
- Real normal task-return path passed across `34` complete cooperative runs.
- Candidate binary: `11792 bytes`.
- SHA-256: `27C5327125BFAC97526F80F83248620152893F7AD92B46F6C56542843F29B885`.
- Acceptance commit: `1114621e9a6bc57d5471cf51a922c216b76bebe2`.

### Accepted / published — PendSV timer-driven preemption

- Activated `PendSV_Handler`.
- `SysTick_Handler()` calls `scheduler_tick()`.
- Scheduler tick requests PendSV only during the command-gated preemptive run.
- PendSV priority is lowest.
- PendSV saves/restores `r4-r11` on PSP.
- EXC_RETURN/SPSEL guard prevents MSP-origin PendSV from touching PSP.
- Abort and final-exit paths clear stale pending PendSV before kernel/MSP restoration.
- Added `schedpreempt -> SCHED_PREEMPT_OK`.
- Preemption acceptance tasks are CPU-bound and contain no voluntary yield.
- Expected sequence: `0x30 -> 0x40 -> 0x31 -> 0x41 -> 0x42 -> 0x32`.
- Hardware path passed across `34` complete preemptive runs with full regression.
- Accepted binary: `12740 bytes`.
- SHA-256: `E1D02C22AF7739DB3EE71E9CB9FF65D0A5F78F8C0ED61D632EFC1444040A7E4A`.
- `.bss=1920 bytes`; `_ebss=0x20000780`; SRAM headroom `18560 bytes`.
- Acceptance commit: `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.

### Accepted / published — scheduler stack canary / high-water instrumentation

- Added command-gated `schedstack -> SCHED_STACK_WATER_OK`.
- Existing task stacks remain exactly `2 x 512 bytes`.
- Canary/high-water telemetry is reset at the start of each scheduler run.
- High-water is recorded on SVC yield, SVC exit, and PendSV switch paths.
- Stack scanning executes in Handler mode/MSP, so the measurement does not consume the measured PSP stack.
- `schedstack` runs the cooperative and preemptive self-tests and reports per-task usage.
- Real hardware measurements:
  - cooperative task 0: `72 bytes`
  - cooperative task 1: `72 bytes`
  - preemptive task 0: `72 bytes`
  - preemptive task 1: `72 bytes`
  - capacity: `512 bytes`
  - observed free margin: `440 bytes`.
- Static worst-case for the current synthetic test tasks was also `72 bytes`, matching the hardware high-water result.
- Canary remained intact in every accepted run.
- Hardware acceptance:
  - exact flash program/verify/readback PASS
  - first real stack-water command PASS
  - 32/32 stack-water stress commands PASS
  - 4/4 return-to-kernel ping checkpoints PASS
  - post-stress SysTick health PASS
  - `schedtest`, `schedcoop`, and `schedpreempt` regressions PASS
  - full I2C/OLED legacy regression PASS
  - final reset + `schedstack` + cooperative + preemptive + health PASS
  - final exact flash identity PASS
  - total accepted stack-water commands: `34`
  - total underlying scheduler runs: `68`
  - physical frozen OLED output confirmed unchanged.
- Accepted candidate binary: `13408 bytes`.
- SHA-256: `4A57F4559AAC3BDAE8FEF5FD3B51F3DEA9033FC917DA19032754796459959D42`.
- `.bss=1936 bytes`; `_ebss=0x20000790`; SRAM headroom `18544 bytes`.
- This acceptance proves the current synthetic scheduler test tasks fit comfortably in 512-byte stacks; workload-specific proof remains required for substantive production paths.
- Acceptance commit: `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.

### Transition

- The next gate after this milestone is a representative substantive PSP workload with real high-water/canary measurement.
- Normal boot, console, and OLED remain on the current kernel/MSP path until that workload-specific proof is complete.
<!-- END STM32_OS_CHANGELOG_2026_09_13 -->

All notable project milestones are recorded here.

This project is currently pre-release; entries are milestone-oriented rather than semantic-version release notes.

## [Unreleased]

<!-- BEGIN STM32_OS_CHANGELOG_2026_09_10 -->
## 2026-09-10

### Added
- Custom Cortex-M3 startup, vector table, .data initialization, and .bss clearing.
- Direct-register 72 MHz clock configuration and SysTick timebase.
- Fault diagnostics for HardFault, MemManage, BusFault, and UsageFault with stacked-context capture.
- Hardware-proven controlled UsageFault diagnostic path and LED panic signaling.
- USART1 polling TX console on PA9 at 115200 8N1.
- UART boot banner and 32-bit hexadecimal output helper.
- Stable kernel time API: kernel_time_now().
- Wraparound-safe deadline and elapsed-time helpers.
- Automated PowerShell hardware loop for build/preflight, ST-LINK flash/verify/reset, UART capture, and acceptance logging.

### Verified
- ST-LINK SWD programming and verification.
- SysTick-driven PC13 heartbeat.
- UART boot output through external USB-UART adapter.
- Time API build and hardware regression image SHA-256: E672398FACBB3BA83E8F05DF4D7165ACFC1849D34BF3589EA81E114ED2DC72E9.
- Dynamic fault_record ELF symbol resolution after its RAM address moved to 0x20000004.

- Time-API hardware regression passed on the 1000-byte image; UART banner matched dynamic `fault_record=0x20000004`, and the PC13 SysTick heartbeat was physically reconfirmed.

- USART1 bidirectional console hardware milestone passed: PA10 RX + PA9 TX at 115200 8N1, automated `ping` -> `PONG`, image SHA-256 `13715B1E206AAD6A8F2EE82E96584623954905C7CAB03611E87514E1D37BE954`, PC13 heartbeat reconfirmed.

- Console `uptime` hardware milestone passed: live `kernel_time_now()` values advanced from `1936 ms` to `2938 ms` (delta `1002 ms`), `ping` regression passed, dynamic `fault_record` matched ELF, image SHA-256 `67C08D182D7CF152BD01F449420C9C5445F0D40F3A49D43B6E1A82BBBFCCC988`.

- Console `health` hardware milestone passed: 10 automated samples showed `3399 ms` SysTick progression, both PC13 output-latch states, and 7 state transitions; image SHA-256 `5BA9FED465FD9F7988B3156CA612EFB88021B1071C6137ABF1D7D1B82042C203`.

- Read-only `fault` console milestone passed: clean fault_record/CFSR/HFSR, SHCSR `0x00070000`, dynamic ELF-matched FAULTREC, and full ping/uptime/health regression; image SHA-256 `185899CE66C3B8683088284939BED267C9E473EFC9317386913595EE15271072`.

- Phase 6 I2C1 hardware scan passed: B6/B7 at 100 kHz, one device ACKed at 7-bit address `0x3C`, with full console/fault/health regression; image SHA-256 `B8F894631076F34EA259E36C0A6744D4FAA6A3B69657F58CEFED72893D25945B`.

- First SSD1306 command transaction passed: `oledping` sent control `0x00` + NOP `0xE3` to address `0x3C`, returned `OLED_CMD_OK`, and the device remained visible to `i2cscan`; image SHA-256 `AA5F092F9C25E6ED07B2F23DEAFF40AF358F07F89B8544EB1BAE2D47605CC9E6`.

- First visible SSD1306 output passed: `oledtest` initialized the 128x64 panel at `0x3C`, wrote a full 1024-byte `0xAA/0x55` checkerboard, returned `OLED_TEST_OK`, remained responsive, and the user physically confirmed the visible pattern; image SHA-256 `46D97925518CD8924278D6A27709E2BEF96AB89E2C49E98E01E88BA40625A5D7`.

### Planned
- USART1 RX and bidirectional kernel command console.
- Native USB Device support on PA11/PA12.
- USB CDC console, binary/RPC transport, host-rendered UI path, and later USB firmware update/bootloader.
- OLED SSD1306 system/status console.
- Scheduler/tasks and PendSV context switching.
<!-- END STM32_OS_CHANGELOG_2026_09_10 -->

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_CHANGELOG_20260913 -->
## 2026-09-13 — production scheduler steady-state wait/wake foundation accepted

### Added

- `SCHEDULER_TASK_BLOCKED` lifecycle state and per-task wait/wake event metadata.
- `scheduler_wait_events()` SVC wait path and ISR-safe `scheduler_event_signal()`.
- Host-MSP `WFE` scheduler idle ownership when all incomplete tasks are blocked.
- `schedwaitwake` UART-IRQ hardware diagnostic.
- `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` for harness/evidence/failure classification and recovery rules.

### Corrected

- Wait/wake diagnostic now treats UART events as notifications to re-check the RX FIFO instead of assuming the event identifies the next payload byte.
- CR/LF console framing is filtered from the wait/wake payload proof.
- `SCHED_WAIT_WAKE_ARMED` is emitted from the active scheduler task so the host can prove blocked -> WFE idle -> IRQ wake -> PSP resume.

### Accepted evidence

- source/build candidate: `19932` bytes, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`;
- four real UART IRQ wait/wake rounds passed with payload `0x57`, positive idle-WFE counts, and intact PSP canaries;
- lifecycle isolation passed with seven invasive diagnostics blocked while the scheduler was active;
- all six existing scheduler diagnostics passed before and after wait/wake proof;
- USART1 RX ring remained at zero drops and zero errors;
- MSP canary and margin remained healthy;
- OLED runtime regression and final physical OLED appearance passed.

Normal boot remains MSP-owned. Production scheduler normal-boot ownership/migration is the next implementation boundary; timer sleep semantics and priorities remain deferred.
<!-- END STM32_OS_SCHED_WAIT_WAKE_CHANGELOG_20260913 -->
