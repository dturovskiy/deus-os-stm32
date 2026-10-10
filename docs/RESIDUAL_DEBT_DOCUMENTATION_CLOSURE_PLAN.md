# Deus OS — RDC-08 Residual Debt Documentation Closure Plan

Status: **GATE-0 DESIGN / READ-ONLY AUDIT RECORDED — ACCEPTANCE AND PUBLICATION PENDING**

Boundary ID: `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE`

Program: `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`

Acceptance: `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md`

Immutable Gate-0 prestate: `80ccdf5534ac65a830dcec5cc022e916521d2fb4` (`main == origin/main`; index/worktree clean; ahead/behind `0/0`).

## 1. Purpose and authority

RDC-08 is the **last mandatory residual-debt closure item**, after RDC-01..RDC-07 CLOSED/PUBLISHED. It reconciles documentation and source-of-truth hygiene; it is not a new product feature, source refactor or permission to update a device.

Follow `docs/DOCUMENTATION_MODEL.md` precedence:

1. Live Git bytes/commit/tree/index state are authoritative for repository facts.
2. `docs/CURRENT_STATE.md` alone owns accepted *current* project disposition.
3. `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, canonical scoped plans and protocol documents own their respective stable contracts.
4. Acceptance evidence/logs own what actually passed; Markdown summaries are not evidence substitutes.
5. `CHANGELOG.md`, `docs/MASTER_EXECUTION_CHECKLIST.md`, `docs/PROJECT_HANDOFF.md` and historical plans preserve chronology, not competing live state.

This boundary must preserve validated identities, physical deployment evidence and all prior Gate facts. **An old assertion made at a historical milestone is not stale merely because a later milestone superseded it.**

## 2. Gate-0 read-only baseline / inspected surfaces

The DEUS-MCP Windows-backed WSL repository view is `/home/deus/projects/deus-os-stm32/OS`, mapped to canonical Windows `D:\Projects\STM32\OS`. It is **not** the Ubuntu Mac-mini USB host and does not itself prove Windows ARM toolchain or physical STM32 access.

Measured at `80ccdf5534ac65a830dcec5cc022e916521d2fb4`:

- 250 tracked repository files; 108 `docs/*.md`; 39 `*_ACCEPTANCE_PLAN.md` and 42 other `*_PLAN.md` records under `docs/`; three additional root/scripts Markdown files.
- The remaining 136 tracked non-Markdown/non-license/config hygiene files were read without truncation, including 32 `src/`, two `bootloader/`, 60 `host/`, 30 `include/`, two `linker/`, nine executable `scripts/` files and one CI workflow.
- 30 unchecked Markdown checklist entries in the prestate: one in the RDC-07 acceptance record (RDC-08 pointer), 19 in `MASTER_EXECUTION_CHECKLIST.md` and ten in `ROADMAP.md`. The non-RDC-08 entries are explicitly deferred/consumer-triggered; unchecked does not mean silently active.
- No unresolved relative `.md` Markdown links in the `docs/` inventory; expected historical plan-name exceptions and the OLED historical semantic pair are explained in `DOCUMENTATION_MODEL.md`.
- `git diff --check` and `git diff --cached --check` passed at the clean prestate. The tracked tree and Git-history **path-name** inventory showed no build binaries, ZIP evidence, logs, or obvious key-bearing artifact filenames; `.gitignore` excludes generated build and Host `bin/obj` outputs.
- Heuristic *current tracked-text* token/private-key-signature searches found no obvious matches; source inventory found no `TODO/FIXME/HACK/XXX/TBD` markers and no target dynamic-allocator calls. These checks are not a full secret-history scan, binary audit, proof of all code paths or independent build/runtime verification.
- Linker/application/bootloader/Host firmware-update constants were spot-checked for 8-KiB bootloader, 52-KiB app, dual metadata and dual persistence ownership. This is a static contract check, not new acceptance of firmware, Flash or USB.

Baseline inventory is a **frozen audit observation**, not a perpetually current live count. Adding the two Gate-0 design files changes the on-disk count; a fresh post-publication audit must report the new count explicitly.

## 3. Confirmed current documentation debt

### D-01 — Current-state history overload (confirmed)

`docs/CURRENT_STATE.md` duplicates detailed FDC/RDC gate histories, evidence hashes and chronology although its canonical role is to state *where the project is now*. In particular, prestate line 140 is one 5,864-character historical Gate narrative. This conflicts with the `DOCUMENTATION_MODEL.md` prohibition on full gate-by-gate history in `CURRENT_STATE.md`.

**Required closure:** make `CURRENT_STATE.md` a concise but sufficient snapshot of accepted product and device identities, active maintenance boundary, next gate, blockers and canonical pointers. Move no unique evidence into oblivion: verify it already exists in the relevant scoped acceptance record / historical ledger before removing any repetition. Preserve the accepted physical STM32 firmware revision 5 and explicitly distinguish *product* publication from maintenance commits. Keep the current source-of-truth map unambiguous.

### D-02 — Documentation inventory freshness (confirmed maintenance)

`docs/DOCUMENTATION_MODEL.md` section 8 reports the **correctly labelled historical FDC-10 inventory** (90 docs Markdown / 30 acceptance plans / 33 other plan-named files). The measured pre-RDC-08 inventory is 108 / 39 / 42. Historical numbers must **not** be overwritten as though wrong.

**Required closure:** retain the FDC-10 snapshot as history and add a separately labelled, exact new RDC-08 publication inventory with recomputed counts, same-stem plan/acceptance pairing and a classification of every active unmatched pair. Avoid a fixed count masquerading as timeless truth.

### D-03 — Ambiguous historical-vs-current phrasing (audit/classification required)

Review `docs/ARCHITECTURE.md` sections headed `C3.9 planned` and `C4.0 planned` beside their later accepted records; inspect current/next/active references, including old FDC-10 feature-eligibility text in historical acceptance records. **No defect is presumed solely from the word `planned` in a historical section.** Correct only text that can reasonably be interpreted as a present-tense competing authority; otherwise classify/preserve it.

### D-04 — Final source-of-truth / hygiene sweep (mandatory verification)

Reconcile `CURRENT_STATE`, `ROADMAP`, RDC/FDC program plans and acceptance records, checklist, deferred backlog, relevant architecture/topology documents, README/handoff and `CHANGELOG` against current Git. Classify all unchecked checkboxes and `planned/next/deferred/active` references as current, accepted history or future consumer-driven work. Confirm there is no orphan *active* scope and no unclassified current technical/tooling/documentation debt.

GitHub branch protection/rulesets and exhaustive credential-history scanning are **separate governance/security-policy questions**, not automatically firmware defects or silent RDC-08 implementation mandates. Record an unresolved actionable violation only with a concrete policy or reproducible evidence; otherwise classify it as optional governance enhancement.

## 4. Frozen path and behavior boundary

**Gate-0 document creation:** exactly these two new Markdown paths, no other mutation:

- `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md`
- `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md`

**Later RDC-08 reconciliation:** `docs/*.md` and `CHANGELOG.md`; `README.md` or `scripts/README.md` only if Gate-1 produces a specific conflicting present-tense assertion. Gate-1 must freeze the actual required files before Gate-2 edits. Do not bulk-format historical records or change acceptance facts just to reduce file length.

**Prohibited without a separate explicitly accepted scope amendment:** `src/`, `include/`, `bootloader/`, `host/`, `linker/`, executable `scripts/*.ps1`, `.github/`, Git policies/config, `.gitignore`, firmware, packages, keys, Flash, device I/O, board reset or USB-cable/power manipulation. No new Host Management/Web/network feature promotion until complete RDC-08 closure.

The present 250-file Git state, signed on-board application firmware revision 5, historical hardware PASS, and accepted RDC-07 ARM equivalence remain untouched. No synthetic-key test binary is deployable.

## 5. Gate sequence (definitions, not claims of acceptance)

- **Gate 0 — audit and design freeze:** independently measure current local Git/docs/source status, classify initial debt and exclusions, create only the canonical plan/acceptance pair, check their content and staged path set, then accept/publish the design docs with exact-SHA hosted CI and clean fresh-fetch proof. Gate-0 docs publication alone does **not** close RDC-08.
- **Gate 1 — exhaustive reconciliation matrix:** enumerate all Markdown sources and current/next/active/deferred/unchecked matches; classify each relevant match as `CURRENT_CORRECT`, `CURRENT_DEBT`, `HISTORICAL`, `DEFERRED` or `NOT_APPLICABLE`; freeze the exact Gate-2 Markdown path set.
- **Gate 2 — minimal docs-only implementation:** repair confirmed competing/current assertions, compact `CURRENT_STATE.md`, preserve scoped evidence and historical chronology; add a measured inventory without falsifying FDC-10's historical snapshot.
- **Gate 3 — independent cross-document acceptance:** repeat the matrix and source-of-truth/pairing/link checks on the exact candidate; show every substantive deletion from `CURRENT_STATE` remains traceable to a canonical record; no unclassified current debt.
- **Gate 4 — repository/security hygiene:** current tracked-file and Git-history path/artifact checks, current-text credential-pattern search with explicit limits, ignore/attributes review, empty unintended build/evidence/secret path set and non-vacuous `git diff --check`.
- **Gate 5 — final program closure adjudication:** show RDC-01..08 disposition, active-next transition eligibility, no feature automatically selected and no unclassified confirmed current debt; finalize candidate exact paths and documentation ownership.
- **Gate 6 — docs-only local acceptance commit:** stage *only* approved Markdown paths, review full staged patch and `git diff --cached --check`; normal nonempty commit; verify exact path set and clean index/worktree.
- **Gate 7 — ordinary non-force candidate publication:** fresh fetch / exact expected parent, normal fast-forward, post-push fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean index/worktree, ahead/behind `0/0`, successful exact-commit hosted Host CI. RDC-08 stays closure-pending until the verified acceptance record is itself published.
- **Gate 8 — final documentation closure record/publication:** *after* Gate-7 exact-SHA CI passes, reconcile `CURRENT_STATE`, RDC program/checklist/roadmap and scoped acceptance record to the verified facts; commit docs-only, publish ordinary non-force, repeat fresh-fetch clean `0/0` and exact-SHA CI. Only then mark RDC-08 and the RDC program CLOSED/PUBLISHED and permit **selection**, not automatic implementation, of a separately planned feature.

Gate numbering and evidence requirements are frozen here; a failed Gate is not retroactively reclassified as PASS. If a test requires an environment not owned by the current execution domain, report the limitation and STOP rather than inventing proof.

## 6. Exit and stop criteria

Final PASS requires: every confirmed/current defect corrected or explicitly blocked with evidence; every remaining `[ ]` and future marker classified; one global current-state authority; preserved accepted historical identities/evidence; no source or target side effects; exact docs-only Git publication and CI; no secret/build artifacts tracked.

STOP on dirty/unexplained prestate, unapproved changed path, missing source evidence for a proposed deletion, unclassified active conflict, invented Gate PASS, non-fast-forward push, unverified CI, failed hygiene check or a request for device/key access. This plan and its audit results **do not authorize** firmware flashing or new product-feature work.
