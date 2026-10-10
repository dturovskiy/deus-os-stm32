# Deus OS — RDC-08 Residual Debt Documentation Closure Acceptance Plan

Status: **GATE-0 PRESTATE AUDIT RECORDED — DESIGN VALIDATION / PUBLICATION PENDING; GATES 1–8 NOT STARTED**

Canonical design: `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md`

Program authority: `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md`

Global state authority: `docs/CURRENT_STATE.md`

Frozen Gate-0 parent: `80ccdf5534ac65a830dcec5cc022e916521d2fb4`.

## Gate 0 — independent audit and two-file scope freeze

**Required PASS evidence**

1. Local DEUS-MCP repository resolves to the Windows-backed OS worktree, not Mac-mini USB host. Baseline `HEAD == origin/main == 80ccdf5534ac65a830dcec5cc022e916521d2fb4`, clean index/worktree and ahead/behind `0/0`, recorded *before* mutation.
2. Verify RDC-01..07 CLOSED/PUBLISHED from Git plus canonical contracts, with RDC-08 the sole mandatory remaining RDC item, no current product-feature boundary.
3. Read complete `docs/*.md` inventory and inspect tracked firmware/Host/bootloader/scripts/linker/CI surfaces; classify observed current-state duplication, inventory history, open checkboxes, stale-versus-historical assertions, path/artifact/secret-search limitations.
4. Gate-0 worktree candidate adds exactly two named docs; NO existing document, product, script, CI, Git policy or target state is altered. At acceptance, staged-name-status must equal `A` for the two named files and nothing else.
5. Review both full documents for exact parent identity, evidence-bound findings, explicit anti-goals, source-of-truth precedence, later Gate tests and fail-closed conditions. No baseline audit observation is misstated as a successful new firmware build or hardware test.
6. `git diff --cached --check` PASS on the nonempty actual staged candidate; exact two-path local docs-only commit; ordinary non-force push after fresh expected-parent verification; fresh post-push `HEAD == origin/main == FETCH_HEAD`, clean index/worktree, `0/0`.
7. GitHub Actions hosted Core `>=84` and Transport `>=24` genuine tests, zero failed/skipped, Release warnings/errors zero and tracked-file/non-vacuous hygiene PASS on the **exact Gate-0 publication commit**. Host CI is not a hardware/ARM equivalence substitute.

**Observed pre-mutation audit (read-only; not Gate-0 publication PASS)**

- WSL-DEUS repo view `/home/deus/projects/deus-os-stm32/OS`, canonical Windows `D:\Projects\STM32\OS`; `main == origin/main == 80ccdf5`, clean `0/0`.
- 250 tracked files. `docs/`: 108 Markdown, 39 matching `*_ACCEPTANCE_PLAN.md` names and 42 other `*_PLAN.md` names. All 108 docs and the 136 tracked non-Markdown source/tool/config surfaces were fully read through DEUS-MCP.
- 30 initially unchecked Markdown items: `BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md` one; `MASTER_EXECUTION_CHECKLIST.md` 19; `ROADMAP.md` ten. RDC-08 remains next; generic UI/kernel/networking candidates remain deferred.
- Relative Markdown `.md` links: no unresolved paths found in the audited syntax subset. Historical unmatched plan-name exceptions explicitly described in `DOCUMENTATION_MODEL.md`.
- Baseline `CURRENT_STATE.md` line 140: 5,864 characters of detailed RDC Gate history, conflicting with concise-current-state governance. Section 8 of `DOCUMENTATION_MODEL.md`: correctly marked FDC-10 snapshot `90/30/33`, not today's inventory `108/39/42`. Historical `planned`/FDC-10 references require classification, not indiscriminate rewriting.
- `git diff --check` and staged whitespace check at pristine parent: PASS; current tracked artifact filenames and all-history path-name search: no prohibited firmware/build/ZIP/log/key class. Current-text obvious private-key/token signatures: none. This is **not** exhaustive secret-history scanning, program verification or device testing.
- Baseline board evidence remains historical: authenticated application revision 5 and the previously accepted bootloader; no target SWD, UART, native USB, Flash, reset, signing-key or hardware I/O was performed.

**Gate-0 result:** `PENDING` until the exact two-document staging/commit/publication and exact-SHA hosted CI proofs above are independently verified. No other Gate is authorized by an unaccepted draft.

## Gate 1 — exhaustive source/document claims matrix

**PASS:** every tracked docs Markdown plus root/script reference Markdown is inventoried, with a source-linked classification of current claims, historical gates, open checkboxes, active-next references, future/deferred terms, topology, firmware ownership and plan pairings. Classifications: `CURRENT_CORRECT`, `CURRENT_DEBT`, `HISTORICAL`, `DEFERRED`, `NOT_APPLICABLE`. Exact required Gate-2 docs-only changed-path allowlist frozen.

**FAIL:** unexamined file, active orphan plan, silent promotion of optional features, conflation of WSL worktree with Mac mini USB host, or inferred current device state from old logs alone.

## Gate 2 — scoped documentation reconciliation

**PASS:** only Gate-1-approved `docs/*.md` and if proven necessary `CHANGELOG.md`, `README.md` or `scripts/README.md` change. `CURRENT_STATE` retains a concise complete snapshot and current physical firmware identity, strips duplicated history *only after* proving canonical scoped retention. Fresh RDC-08 inventory added while historical FDC-10 inventory remains labelled and intact. Corrections target only actually competing present-tense assertions.

**FAIL:** losing a unique gate/evidence fact, rewriting accepted history, introducing a second global source of truth, changing a current product contract, or touching executable files.

## Gate 3 — independent source-of-truth and regression review

**PASS:** independent reread/grep after Gate-2 produces a complete classification of all `[ ]`, `planned`, `next`, `active`, `future`, `deferred` and RDC/FDC status references. Confirm actual relative links and textual filename references, correct plan pairing and explicit supersession of historical claims. Review removed paragraphs against source documents; check no stale requirement reappears.

**FAIL:** any unclassified current contradiction, unreferenced evidence loss, broken active contract reference, stale current gate or false `CLOSED/PUBLISHED` assertion.

## Gate 4 — Git/artifact/security hygiene

**PASS:** exact docs-only diff, real nonempty `git diff --check`, gitignore/attributes policy unchanged and functioning, no tracked generated outputs/archive/keys, current-tree credential-pattern audit, Git-history path-name review and disclosure that content/history scan coverage is bounded. Current source/protocol/linker/test/build paths unchanged from Gate-0 parent.

**FAIL:** forbidden path, private key/credential match not safely adjudicated, tracked binary/evidence artifact, empty/vacuous check or any worktree/index drift.

## Gate 5 — RDC program closure readiness

**PASS:** RDC-01..07 remain historical CLOSED/PUBLISHED; RDC-08 candidate passes Gate 1–4; no unclassified confirmed current technical/tooling/documentation debt; deferred ideas stay deferred; `CURRENT_STATE`/roadmap/program/checklist/changelog are consistent about the *pending publication* until Gate 8 actually passes. Product features remain unpromoted.

**FAIL:** premature claim that RDC-08 or program is CLOSED before commit/push/CI, or automatic feature activation.

## Gate 6 — normal docs-only acceptance commit

**PASS:** nonempty exact stage allowlist, independent full patch review, `git diff --cached --check` PASS, normal local commit, exact resulting changed-path set and clean post-commit worktree/index. No force, amend of published history or hidden artifacts.

**FAIL:** scope/content drift or missing proofs.

## Gate 7 — ordinary candidate publication and CI proof

**PASS:** fresh-fetch upstream equal to expected direct parent, normal non-force fast-forward, fresh-fetch `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`, exact-published-SHA hosted Host Release/Core/Transport/hygiene PASS. This accepts the candidate but does **not** prematurely assert the later closure-record publication.

**FAIL:** rejected/force push, remote drift, CI failure/pending, missing poststate or premature closure wording.

## Gate 8 — published final closure record

**PASS:** only after Gate-7 acceptance, synchronize `CURRENT_STATE`, `ROADMAP`, the RDC program/checklist, this acceptance record and chronology to record the verified conclusion. Review exact docs-only staged paths, `git diff --cached --check`, commit normally and publish through non-force fresh-fetch equality/clean `0/0`; confirm exact-SHA hosted CI success. At this point `RDC-08 CLOSED/PUBLISHED` and the entire RDC program closure are factual; any next product boundary still requires its **own** authorization.

**FAIL:** claim of final closure before publication, source/tooling path change, or CI/poststate failure.

## Mandatory rejection and evidence discipline

Classify failures precisely as `RDC08_PRESTATE_DRIFT`, `RDC08_SCOPE_VIOLATION`, `RDC08_UNCLASSIFIED_CURRENT_DEBT`, `RDC08_HISTORY_LOSS`, `RDC08_SOURCE_OF_TRUTH_CONFLICT`, `RDC08_ARTIFACT_OR_SECRET_HYGIENE`, `RDC08_WHITESPACE_OR_LINK_FAILURE`, `RDC08_PUBLICATION_DRIFT`, `RDC08_CI_FAILURE` or `RDC08_ENVIRONMENT_PROOF_UNAVAILABLE`.

A reported read-only audit is **not** itself an accepted published Gate. Do not claim `PHYSICAL_OLED` was newly tested, reflash firmware, generate synthetic deployable packages, export production keys, change USB power ownership or create gratuitous ZIP/evidence bundles. Record only facts actually verified.
