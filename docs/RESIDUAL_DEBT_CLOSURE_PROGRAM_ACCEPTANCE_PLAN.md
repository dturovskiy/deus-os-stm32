# Deus OS — Residual Debt Closure Program Acceptance Plan

Status: **ACTIVE PROGRAM / GOVERNANCE GATES 0–4 ACCEPTED / PUBLISHED `2fb687d4bd57e8029e89d450038b712acb6d9dea`**

Canonical program:

`docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`

Baseline:

`d649e133cbe39fa6155520c657136ca99cee61ae`

## Gate 0 — debt-first governance freeze

PASS requires:

- no active product feature boundary;
- current confirmed debt is distinguished from consumer-driven future work;
- RDC-01..RDC-08 are recorded as mandatory pre-feature obligations;
- every source/tooling RDC item requires its own plan/acceptance pair;
- no source, target, Flash or hardware mutation occurs during program activation.

### Gate-0 result — PASS

Published baseline is `d649e133cbe39fa6155520c657136ca99cee61ae`, with no active product feature boundary. The post-FDC audit findings are split into mandatory confirmed debt (`RDC-01..RDC-08`) versus consumer-driven future capabilities that remain deferred. Active next maintenance boundary is `RDC-01 / HOST_ASSET_TRANSACTION_RECOVERY_HARDENING`; no RDC product source mutation is authorized by this governance commit.

## Gate 1 — source-of-truth synchronization

PASS requires synchronized current policy across:

- `docs/CURRENT_STATE.md`;
- `docs/ROADMAP.md`;
- `docs/MASTER_EXECUTION_CHECKLIST.md`;
- `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`;
- `docs/DOCUMENTATION_MODEL.md`;
- `CHANGELOG.md`.

No historical accepted result is rewritten; stale current/future wording is corrected or explicitly labelled historical.

### Gate-1 result — PASS

Current policy is synchronized across `CURRENT_STATE`, `ROADMAP`, execution checklist, deferred backlog, documentation model, architecture/Flash/Host-management references and changelog. The prior FDC ledger is explicitly historical/closed. Current Host Management/Web/network eligibility now depends on RDC-01..RDC-08 closure rather than FDC closure alone. The historical FDC-10 acceptance statement that feature selection could resume at that time is preserved as history, not current authority.

## Gate 2 — local docs validation

PASS requires:

- docs-only changed path set;
- `git diff --check` clean;
- no new product/source/tooling mutation;
- no statement that future feature work may begin before RDC-01..08 close;
- no consumer-driven future item silently promoted as debt.

### Gate-2 result — PASS

Exact candidate is docs-only: 11 changed paths, zero product/source/tooling/test paths, no target/Flash/hardware operation. `git diff --check` PASS. Governance search found no current-authority statement allowing product-feature promotion before RDC closure; the only remaining “feature selection may resume” wording is inside the historical accepted FDC-10 record and is intentionally preserved.

## Gate 3 — docs-only acceptance commit

One normal local docs-only commit after exact staged-path review.

### Gate-3 result — PASS

Exact staged set contained 11 documentation/governance paths and no source/tooling/test path. `git diff --cached --check` PASS. Local acceptance commit `2fb687d4bd57e8029e89d450038b712acb6d9dea` (`docs: promote residual debt closure program`) is a docs-only child of baseline `d649e133cbe39fa6155520c657136ca99cee61ae`.

## Gate 4 — ordinary non-force publication

Fresh fetch/direct-parent proof before push; post-push fresh fetch must prove `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

### Gate-4 result — PASS / PUBLISHED

Fresh pre-push fetch proved `origin/main == d649e133cbe39fa6155520c657136ca99cee61ae`, exactly the parent of governance commit `2fb687d4bd57e8029e89d450038b712acb6d9dea`. Publication used an ordinary non-force fast-forward `d649e13..2fb687d  main -> main`. Fresh post-push fetch then proved `HEAD == origin/main == 2fb687d4bd57e8029e89d450038b712acb6d9dea`, clean worktree/index and ahead/behind `0/0`.

Program governance is accepted/published. The program itself remains ACTIVE until RDC-01..RDC-08 close.

## Program execution acceptance

Each RDC item owns its own gates and evidence. This program-level acceptance record tracks only governance/order and the final all-debt exit criterion.

Current program progress:

- `RDC-01 / HOST_ASSET_TRANSACTION_RECOVERY_HARDENING` — **CLOSED/PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b`**; Core `84/84`, Gate-2 evidence SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA`, target I/O NONE / Flash mutation NONE;
- `RDC-02 / HOST_DEAD_SURFACE_CLEANUP` — **CLOSED/PUBLISHED `09a432f6c0b73ef2425add5950b6f6d5dee3733d`**; Transport `24/24`, Core `84/84`, Gate-2 evidence SHA-256 `44C5DBFA295087F1F2BFD5A453E63C3855EBC0508EEC5D3BEF283F99D4482B84`, target I/O NONE / Flash mutation NONE;
- `RDC-03 / TARGET_DEAD_API_CLEANUP` — **CLOSED/PUBLISHED `e516fdc1d8818007db40ee12669d28f3f828c489`**; Gate-2 deterministic equivalence evidence SHA-256 `224F04F33A91DAC295D0716ABF52B1457CC7DDA8C98316D17098243CBB05FE33`, Gate-3 read-only continuation evidence SHA-256 `046E4F1F30A69CDAE00B6B9A37B2C02E16A05F7A4F4EBBA56AB1F48E23341AB9`, exact target candidate readback and `PHYSICAL_OLED=PASS`;
- `RDC-04 / TOOLING_OUTPUT_DIRECTORY_SAFETY` — **CLOSED/PUBLISHED `7fda7b8225f05c487f6d5d7154443e8369e74571`**; Gate-2A safety matrix 27/27, Gate-2B firmware/recovery equivalence, Gate-2C docs closure, Gate-3 commit and Gate-4 fresh-fetch publication PASS; Gate-4 evidence SHA-256 `39A8EF0098CD313A2645316AFF0D5DD50841D080D68F98A0CE90E04D49AAFC1C`;
- `RDC-05 / REPOSITORY_CI_BASELINE` — **CLOSED/PUBLISHED `7f75cffdd0c632c5f99310f2c8708d745769aa42`**; Gate-2 Linux proof SHA-256 `9ABE736EA688352C570B3D0443BABC797A8E240B27FE2107CFF40940305BF651`; hosted GitHub Actions run `37829541187` on the accepted exact SHA PASS with Core `84/84`, Transport `24/24`, zero skipped, locked restore, clean Release and static hygiene;
- `RDC-06 / KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP` — **CLOSED/PUBLISHED `7bc5f27ce3447f0c907d2643e7db67669371210a`**, candidate source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`; hardware Gate-3 signed revision 5 + full Flash readback + UART/physical OLED accepted (final evidence SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD`), separate EPnR CTR hardware repair at `83e57f609bab4bcc8a61ea2222629bf9066e910d`, non-force push/fresh fetch clean, CI run `38011195238` PASS (Core 84/84, Transport 24/24, 0 skips);
- `RDC-07 / BOOTLOADER_READABILITY_CLEANUP` — **CLOSED/PUBLISHED code commit `7846fa48429d00233116438821acc0ea2a0b38be`** (only `bootloader/bootloader.c`, candidate Git blob `ed34ad98f6dcf48c5175c3b53a82d8144e653bee`); independent real Windows GCC 15.3.1 synthetic-key Gate-2 evidence SHA-256 `9208C02ACCC956188E32CBAD68740AC46F0EB5E486C9849C7372B898366EB768` verifies full 5996-byte raw BIN identity SHA-256 `7EDCD55CED5DDA529EFF173CF1ABB676E7892B1A5824B6EE4695A7BA3EC796BE`, source token/linked symbol/disassembly/32 stack-frame/resource equivalence. Gate-3 no-flash acceptance PASS; non-force push, fresh fetch clean 0/0; hosted CI `38067711160` PASS (84/84 Core, 24/24 Transport, zero skips/errors). No production key or target I/O;
- `RDC-08 / RESIDUAL_DEBT_DOCUMENTATION_CLOSURE` — **NEXT: Gate-0 read-only audit and design**, no feature authorization.

Program exit requires RDC-01..RDC-08 CLOSED/PUBLISHED plus final RDC-08 audit with no unclassified current debt.
