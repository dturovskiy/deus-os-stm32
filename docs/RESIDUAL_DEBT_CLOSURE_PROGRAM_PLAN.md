# Deus OS — Residual Debt Closure Program Plan

Status: **RDC-01..07 CLOSED/PUBLISHED / RDC-08 GATES 0–7 PUBLISHED / FINAL PROGRAM EXIT CONDITIONAL ON GATE-8 EXACT-COMMIT CI + CLEAN FRESH FETCH**

Program ID:

`RESIDUAL_DEBT_CLOSURE_PROGRAM`

Baseline:

`d649e133cbe39fa6155520c657136ca99cee61ae`

## 1. Policy

Before any new product feature boundary is promoted, Deus OS must close every **confirmed current technical, tooling/process, maintainability and documentation debt** identified by accepted audits or by reproducible repository evidence.

This program is debt-first maintenance, not feature work.

A future idea is **not** promoted merely because it appears in a backlog. Generic timers, queues, DMA, filesystem, networking, richer observability, low-power/tickless operation and similar consumer-driven capabilities remain deferred until a concrete consumer/problem exists.

A current deficiency is debt when all of the following are true:

1. it exists in the current published repository or operating workflow;
2. it is independently observable/reproducible or already confirmed by an accepted audit;
3. it has a bounded closure criterion;
4. closing it does not require inventing a speculative framework or product feature.

No new Host Management Service/Web/networking or other product-feature boundary may begin until every mandatory RDC item below is CLOSED/PUBLISHED and the final documentation sweep confirms no unclassified current debt remains.

## 2. Mandatory residual-debt ledger

### RDC-01 — CLOSED / PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b` — Asset transaction/session recovery hardening

Confirmed debt:

- request-correlation poisoning is already closed by published `HOST_ASSET_REQUEST_CORRELATION_HARDENING`;
- after BEGIN may have created a volatile transfer session, a WRITE/COMMIT failure can still leave that session alive until target reset or idle timeout;
- the published Asset protocol already provides idempotent `ABORT`, but Host Core does not currently use it for bounded transaction cleanup.

Required boundary:

`HOST_ASSET_TRANSACTION_RECOVERY_HARDENING`

Closure accepted at `699382a58c4c9570cdf36a05693044461e87c58b`. Gate-2 evidence SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA` proves Core `84/84`, warning-clean Release build and zero target/Flash mutation. Host Core now performs one bounded best-effort same-transfer `ABORT` while a volatile session may be active, preserves the primary failure and performs no blind mutation retry.

### RDC-02 — CLOSED / PUBLISHED `09a432f6c0b73ef2425add5950b6f6d5dee3733d` — Host dead-surface cleanup

Confirmed current dead remnants include:

- Linux `ValidateManagementTopology()` after profile-based `ValidateTopology(...)` ownership;
- unused `libusb_get_device_address` P/Invoke.

Before deletion, exact current references/history/contracts must be rechecked. Compatibility/operator APIs explicitly retained by published contracts — including raw `SysInfoAsync()` and protocol `EncodeAbort()` — are not dead code and must remain.

Required boundary:

`HOST_DEAD_SURFACE_CLEANUP`

Closure accepted at `09a432f6c0b73ef2425add5950b6f6d5dee3733d`. Gate-2 evidence SHA-256 `44C5DBFA295087F1F2BFD5A453E63C3855EBC0508EEC5D3BEF283F99D4482B84` proves Transport `24/24`, Core `84/84`, warning-clean Linux transport/Core Release builds and zero target/Flash mutation. Eight unreachable Linux transport remnants were removed without changing profile-aware runtime/bootloader ownership or retained compatibility APIs.

### RDC-03 — CLOSED / PUBLISHED `e516fdc1d8818007db40ee12669d28f3f828c489` — Target dead API cleanup

Confirmed declaration/implementation-only target surface currently includes:

- nine unused `application_runtime_*` accessors left after composition-root decomposition;
- `oled_ui_layout_config_v1_apply()`, while current consumers use validate/activate ownership directly.

`binary_frame_encode()` is explicitly retained by the frozen Binary Framed protocol as a checked compatibility API and is excluded.

Required boundary:

`TARGET_DEAD_API_CLEANUP`

Closure accepted at `e516fdc1d8818007db40ee12669d28f3f828c489`. Exactly ten dead target APIs were removed. Gate-2 evidence SHA-256 `224F04F33A91DAC295D0716ABF52B1457CC7DDA8C98316D17098243CBB05FE33` plus forensic adjudication prove exact baseline reproduction, candidate Flash/SRAM `51972/10968`, global symbol/layout/stack invariants and zero non-provenance BIN differences. Gate-3 read-only continuation evidence SHA-256 `046E4F1F30A69CDAE00B6B9A37B2C02E16A05F7A4F4EBBA56AB1F48E23341AB9` proves exact candidate application readback, unchanged bootloader/persistence ownership and runtime health; operator physical OLED is PASS.

### RDC-04 — CLOSED / PUBLISHED `7fda7b8225f05c487f6d5d7154443e8369e74571` — Repository PowerShell destructive-output safety

The historical debt was caller-controlled recursive output-directory deletion in the firmware builder, current bootloader recovery generator and historical-only Asset recovery generator. RDC-04 centralized all three resets through the single fail-closed `Reset-DeusGeneratedOutputDirectory` primitive with protected path, owner marker, reparse-point and sentinel checks; the Asset generator remains historical and not an operational recovery authority.

Gate-1 source/static acceptance, Gate-2A PowerShell 7 safety matrix **27/27**, Gate-2B deterministic firmware/recovery equivalence, Gate-2C documentation and Gate-3 local ten-path commit are accepted. Gate-4 normal non-force fast-forward and fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`, at published implementation commit `7fda7b8225f05c487f6d5d7154443e8369e74571`. Separate docs closure was published at `58d4255d137c7c0b126cf647bba0d635b2a1dafe`. This item is closed; the next mandatory boundary is RDC-05.`

### RDC-05 — CLOSED / PUBLISHED `7f75cffdd0c632c5f99310f2c8708d745769aa42` — Minimal continuous-integration baseline

The baseline process debt was the absence of tracked deterministic CI. The frozen `REPOSITORY_CI_BASELINE` design/acceptance pair was published docs-only at `49392d003abf26636a8717a9e27c5520bf4c1af4`. The accepted implementation is exactly one `.github/workflows/ci.yml`, with pinned official actions, `ubuntu-24.04`, least-privilege token, locked .NET 10 restore, Release Host build, real MTP Core/Transport tests and repository hygiene, without target or physical bench access.

Gate-2 Mac-mini Ubuntu proof SHA-256 `9ABE736EA688352C570B3D0443BABC797A8E240B27FE2107CFF40940305BF651` records Core `84/84`, Transport `24/24`, zero skipped and warning-clean Release build. Initial workflow commit `19025ed7d695bf75b43b010ae2e1d0a1ba105d4d` produced hosted run `37821098177` with failure before job creation: unsupported `runner.temp` job-level expression. The exact workflow-only context fix was accepted and non-force published at `7f75cffdd0c632c5f99310f2c8708d745769aa42`, tree `69cdb0a83b6a5454582343c6d20239761087809d`. Authoritative hosted GitHub Actions run `37829541187` (`https://github.com/dturovskiy/deus-os-stm32/actions/runs/37829541187`) on that exact SHA completed **success**, executing Core `84/84` and Transport `24/24`, skips `0`, with locked restore, Release `0 warnings / 0 errors` and static hygiene PASS. The accepted repair-publication evidence SHA-256 is `9C701DACDD13E5CFF96F22CC79B86C1DC29889BB6D21B474D0A21FFBE59D0276`. This closes RDC-05 as Host-only CI; firmware and STM32 hardware acceptance remain separate.

### RDC-06 — Remaining kernel composition-root ownership cleanup

Current `src/kernel.c` is approximately 2961 lines after the accepted decomposition and remains a composition/orchestration/console hotspot.

This is not a line-count exercise. The boundary may extract only coherent ownership with an existing reason-to-change and must preserve the accepted anti-framework rules.

Required boundary:

`KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP`

Gate 0 was required to re-audit current responsibilities and to reduce/reshape scope if concentration proved intentional rather than debt. **RDC-06 is CLOSED/PUBLISHED**: the original `schedprod` extraction was rejected for a measured stack regression, followed by a separately published Gate-0 Amendment A authorizing the coherent three-file `help`/registry presentation move. Accepted implementation commit `7bc5f27ce3447f0c907d2643e7db67669371210a` has exact approved source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`, physically deployed as authenticated application firmware revision 5. Its Gate-3 final acceptance binds full Flash readback, USB/RPC/scheduler/IWDG/Windows UART and `PHYSICAL_OLED=PASS` (evidence SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD`). The independent USB EPnR CTR race fix, excluded from the three-file cleanup commit, is published at `83e57f609bab4bcc8a61ea2222629bf9066e910d` and hardware-tested with 72/72 INFO responses and a successful signed v5 update. GitHub CI on the final source commit `83e57f6`, run `38011195238`, passed executed Core 84/84 and Transport 24/24, zero skips, Release clean. Existing root platform/interrupt/console transport and UI integration responsibilities remain intentionally root-owned in this accepted scope; further UI/console decomposition requires a separately demonstrated consumer or debt and is not silently authorized. **Historical ordering:** RDC-07 was next at RDC-06 closure. Subsequently, RDC-07 completed Gates 0–5 and code publication at `7846fa48429d00233116438821acc0ea2a0b38be`; RDC-08 is now next for Gate-0 documentation/source-of-truth audit.

### RDC-07 — Bootloader readability / maintainability normalization

`bootloader/bootloader.c` remains a compact security-sensitive implementation (~715 lines) with readability debt from compressed/minified sections.

Required boundary:

`BOOTLOADER_READABILITY_CLEANUP`

This boundary is **CLOSED/PUBLISHED (source commit `7846fa48429d00233116438821acc0ea2a0b38be`)**. Exactly `bootloader/bootloader.c` was formatted with identical significant C tokens, literals, comments and preprocessor directives. Real Windows ARM GCC 15.3.1 Gate-2 used independent pristine source/output workspaces and one synthetic NON-DEPLOYABLE 32-byte key; both builds produced the same 5,996-byte raw BIN SHA-256 `7EDCD55CED5DDA529EFF173CF1ABB676E7892B1A5824B6EE4695A7BA3EC796BE`, with exact linked symbols, disassembly, stack and resources. External Gate-2 evidence SHA-256 `9208C02ACCC956188E32CBAD68740AC46F0EB5E486C9849C7372B898366EB768` (private operator artifact, never committed). Gate-3 no-flash adjudication PASS; code pushed ordinary non-force, fresh fetch clean 0/0, hosted GitHub Actions run `38067711160` SUCCESS on exact code SHA (Core84/84, Transport24/24, 0 failed/skipped, Release0 warnings/errors). Existing hardware-tested USB/Flash/trust behavior, production key and MCU firmware revision5 remained untouched. No new feature or firmware deployment.

### RDC-08 — Final documentation/source-of-truth debt sweep

Gate-0 design was published at `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`; Gate-1 exhaustive 113-Markdown classification at `c646f3e3ac342cbaef025d115e5be96bd49cf140`; Gate-2–5 documentation/content/hygiene and Gate-6/7 exact docs-only candidate publication at `faabfe75bbb3b805548092c9f298ac862c38ef1d` passed hosted [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) on that exact SHA (Core 84/84, Transport 24/24, 0 failed/skipped, Release clean). Canonical plan/acceptance are `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` and `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md`. **The program reaches CLOSED/PUBLISHED if and only if Gate-8 final docs publication, post-push fresh-fetch clean equality and the Gate-8 commit's own hosted CI all pass. Before that external proof, the program remains OPEN.** All RDC-08 documentation debt identified by accepted audits has been classified and corrected in the released candidate; consumer-driven future work remains deferred.

After RDC-01..07 close, perform one final repo-wide documentation/governance reconciliation:

- current/next/active wording;
- stale gate/publication states;
- historical versus current assertions;
- open checkboxes;
- current topology/tooling ownership;
- dead-code/debt references;
- artifact/security hygiene;
- `git diff --check`.

Required boundary:

`RESIDUAL_DEBT_DOCUMENTATION_CLOSURE`

This final gate must leave no unclassified current technical/documentation debt before product feature selection resumes.

## 3. Mandatory order

Default order:

1. RDC-01 Asset transaction/session recovery;
2. RDC-02 Host dead-surface cleanup;
3. RDC-03 Target dead API cleanup;
4. RDC-04 tooling OutputDir safety;
5. RDC-05 minimal CI baseline;
6. RDC-06 kernel composition-root cleanup;
7. RDC-07 bootloader readability cleanup;
8. RDC-08 final documentation/source-of-truth sweep.

The order may change only through an explicit docs-only governance amendment that records why dependency/risk ordering changed.

## 4. Anti-goals

This program does not authorize:

- new user-facing product features;
- Host Management Service/Web/network implementation;
- speculative HAL/service-locator/universal context;
- heap/dynamic allocation;
- generic queue/mutex/timer framework without a consumer;
- unrelated protocol or Flash-layout redesign;
- refactoring merely to reduce line count;
- deletion of retained compatibility APIs merely because current call count is zero.

## 5. Per-item rule

Every RDC item gets its own canonical plan + acceptance plan before source/tooling mutation.

Each item must:

- freeze exact baseline and path scope;
- classify product vs harness/environment failures;
- reuse proven primitives;
- provide deterministic proof before hardware proof where applicable;
- commit normally;
- publish only by ordinary non-force fast-forward;
- finish with fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## 6. Exit criterion

`RESIDUAL_DEBT_CLOSURE_PROGRAM` is complete only when RDC-01..RDC-08 are CLOSED/PUBLISHED and the RDC-08 final audit finds no unclassified current technical, tooling/process, maintainability or documentation debt.

Only then may `CURRENT_STATE.md` promote a new product feature boundary.
