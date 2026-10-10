# Deus OS — Public Repository Sanitization Acceptance Plan

Status: **GATE-0 ACCEPTED/PUBLISHED `558306fe618d1ee69240fa11d24437cd38fb8e2b` — GATE-1 IN PROGRESS, GATE-1A PUBLISHED, GATE-1B OPEN; GATES 2–7 NOT STARTED**

Design owner: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md`

Sole global state owner: `docs/CURRENT_STATE.md`; publication-role governance: `docs/DOCUMENTATION_MODEL.md`.

Initial frozen baseline: `c309428b03615a9afd514e3c7c8886a82a5112e9` (previously published RDC-08 final exact-SHA Git commit, main/origin clean `0/0`).

## 0. Baseline verification and admissibility

Read-only source verification at design start:

- Public GitHub repository; 252 tracked paths, 113 Markdown files (110 `docs/`), 83 scoped plan/acceptance documents (40 acceptance and 43 other plan-named files). All 113 Markdown sources read fully. No existing `*SANITIZ*` plan pair or equivalent per-file public/private policy found.
- FDC-10, RDC-08 and historical commit `9fa772a` cover different issues: stale docs, current-tree artifact/credential-pattern hygiene and one historical host-path correction. Their acceptance **does not** imply an exhaustive historical-content/hosting leak audit or a public/private release selection.
- Baseline narrative evidence: roughly 1.63 million characters of tracked Markdown, 982 64-hex *occurrences* and 102 evidence ZIP name *occurrences*. These are volume/provenance indicators only, **not** independently proven sensitive tokens, unique secrets or a permitted deletion count.
- Files needing item-by-item **risk adjudication**, not yet condemned: `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`, `PROJECT_HANDOFF.md`, `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, `MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`, `CURRENT_STATE.md`, `scripts/README.md` and all 83 plan/acceptance files; inspect code/config/CI as well.
- Historical workstation-path correction committed at `9fa772a` demonstrably did **not** remove old Git history. No concrete production signing-key, password or API-token compromise has been established by this audit; deny any statement that current absence proves all historical/cloud surfaces safe.

**Gate-0 PASS requires** all of the following:

1. Fresh Git baseline: local `HEAD == origin/main == c309428b03615a9afd514e3c7c8886a82a5112e9` (or explicitly record a legitimate published descendant), index/worktree clean and behind/ahead `0/0` before mutation.
2. Independent read of **all 83 plan/acceptance documents**, as well as current-state/model/roadmap/topology/handoff/repository Git policy and public GitHub visibility. All gaps listed in design §§1–4 reconciled to bounded future Gate duties.
3. Exactly six Gate-0 Markdown changed paths, with exactly two new `PUBLIC_REPOSITORY_SANITIZATION_*` files; all others modified at most as bounded non-sensitive pointer/activation/chronology lines. No mutation to source, executable scripts, product ABI, CI, `.gitignore`, Git config, hardware or GitHub visibility.
4. Public design content contains no new raw operator usernames/SSH aliases/paths, production keys, key-bearing firmware images or private evidence; reveal file *names/categories* only. `git diff --cached --check` PASS on real, nonempty stage and full patch reviewed.
5. Normal local docs-only commit, exact staged path set, fresh expected remote parent, ordinary **non-force** push, independently fresh-fetched `HEAD == origin/main == FETCH_HEAD`, clean `0/0`.
6. Exact GitHub-published SHA hosted CI SUCCESS: locked Release 0 warnings/errors, Core `>=84`, Transport `>=24`, 0 failed/skipped, non-vacuous diff/tracked-output hygiene PASS. GitHub Host CI is not privacy-completeness proof, nor ARM/firmware/hardware test.

**Gate-0 result — ACCEPTED/PUBLISHED (2026-10-10):** verified clean baseline `c309428b03615a9afd514e3c7c8886a82a5112e9`, six exact staged Markdown paths (two new sanitation plans and four existing state/index/roadmap/changelog references), `git diff --cached --check` PASS and full nontruncated candidate patch reviewed. Normal docs-only commit `558306fe618d1ee69240fa11d24437cd38fb8e2b`; ordinary non-force push `c309428..558306f main -> main`; independent GitHub ref and fresh-fetch `HEAD == origin/main == FETCH_HEAD == 558306fe618d1ee69240fa11d24437cd38fb8e2b`, clean index/worktree, ahead/behind `0/0`. GitHub Actions [run 38080940176](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38080940176) on the **same exact SHA** completed SUCCESS: locked Release `0 warnings/0 errors`, Core `84/84`, Transport `24/24`, failed/skipped `0/0`, tracked output hygiene `254` PASS and non-vacuous whitespace check PASS. No source/firmware/CI/Flash/key/target/remote-visibility or history modification. **Only Gate-1 analysis is authorized; no public deletions, archive moves, Git rewrite, GitHub visibility/fork action, key rotation or target access are authorized.**

### Post-Gate-0 canonical checklist reconciliation — separate docs-only publication check

This addendum is **not** a retrospective expansion of Gate-0, **not** Gate-1 classification and **not** an implementation authorization. The six-path initial design and six-path Gate-0 acceptance commits have already passed exact-SHA hosted CI. The canonical execution ledger was separately audited after those publications and was found to omit `PUBLIC_REPOSITORY_SANITIZATION`; its historic RDC-08 Gate-8 closure was not yet reflected in its text.

**Bounded fix:** exactly `docs/MASTER_EXECUTION_CHECKLIST.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` and this `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`. The ledger must record independently verified FDC/RDC closures, the two accepted Gate-0 sanitization docs commits, Gate-1 NEXT and unchecked Gates 1–7, plus preservation-first and explicit-approval prohibitions. Current Gate-1 inventory is **254 tracked files**, versus the accurately historical 252-file pre-Gate-0 input. No actual file is classified for deletion in the ledger.

**PASS only with** exactly three staged Markdown paths and no unexpected WIP, `git diff --cached --check` PASS and full patch reviewed, normal local commit, ordinary non-force publication on exact expected parent, fresh-fetch `HEAD == origin/main == FETCH_HEAD` clean `0/0`, and successful exact-SHA hosted Release (zero warning/error), Core `>=84`, Transport `>=24`, zero skipped/failed and hygiene checks. **Local content validation or a pending CI run is not publication acceptance.** No target/Flash/key/hardware/Git-history/visibility/CI/source paths touched. Record actual result externally; do not preclaim a future CI outcome.

### Follow-up source-evidence reconciliation after Gate-0 (documentation-only; independent of Gate-1)

At clean pre-addendum Git `1dccdf5208dbdd0c2e4e7aa88403152a231f3f49`, direct source inspection confirmed present-tense public bench and operator-path disclosures in `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md:11–228`, `docs/PROJECT_HANDOFF.md:23–109`, `docs/CURRENT_STATE.md:67–79`, diagnostic/harness ledger material, `scripts/README.md:23–95` and **seven tracked executable PowerShell paths** documented in the design's new source-backed register. Old `CHANGELOG.md:561–565` and commit `9fa772a` are a prior bounded hygiene review, **not** a public-private classification or proof the disclosed pre-change path is absent from Git history.

**Required Gate-1 independent checks:** enumerate exact operator identity/hostnames, Windows/WSL absolute directories, SSH alias, USB node privilege procedure, debug/pin/power attachment details, historical acceptance receipts and sensitive compiled-bootloader recovery warnings **without copying live values to the public report**. For each of the seven scripts, capture line-level default-path ownership, external parameter overrides, wrapper/code generation propagation, affected Windows/ARM build and bootloader recovery consumers. Verify whether a public-safe documented toolchain location can be configurable without altering accepted safety invariants; do **not** mutate scripts at Gate-1. Distinguish public-facing product protocol/safety facts from private current-bench specifics and confirmed leaks from generic operational transparency.

These findings are **risk/disclosure candidates**, not automatic `PRIVATE` or `REDACT` dispositions and **not proof of a compromised production secret**. Historical Git content/refs, GitHub Actions artifacts and forks were not exhaustively reviewed by this follow-up. This paragraph does not mark Gate-1 PASS, authorize source changes, or reopen accepted earlier Gates.

## 1. Comprehensive exposure/source matrix

**PASS:** one immutable table entry for **every baseline tracked path**, along with historical deleted/renamed blobs/refs and independently discovered GitHub hosting surfaces. Each row records original path/commit and bounded reference (kept in private report when identity-sensitive), owner and real consumer, category `PUBLIC|REDACT|PRIVATE|RETAIN`, rationale, threat/exposure class, candidate disposition, exact scope and whether public history is *already* exposed. Record the four classes **without assuming all acceptance documents are removable**. Include public source/CI/build dependencies and preserved release provenance.

**FAIL:** a path without classification; missing active ref/tag; equating Git history filenames with inspected file content; falsely treating SHA-256/evidence hashes as secrets; copying raw private data into GitHub issue/CI/logs; declaring an inaccessible hosting surface clean.

Required evidence surfaces: Git current tree (binary and text); all reachable Git histories and relevant orphan/ref reachability where verifiable; current GitHub workflows/artifacts/logs/caches/releases, issue/PR/attachment/discussion metadata, forks/mirrors and external indexes where visible. For uninspectable surfaces mark `NOT_CHECKED` with risk owner. Maintain false-positive adjudication.

### Gate-1 preliminary inventory and host-surface evidence — NOT FULL ACCEPTANCE

The first read-only audit is bound to the clean published prestate `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`, preserved as three distinct public-safe documents:

- `PUBLIC_REPOSITORY_SANITIZATION_GATE1_MATRIX.md`: **254/254 current tracked contents** read with Git blob anchors; tentative categories `PUBLIC=141`, `REDACT=15`, `PRIVATE=49`, `RETAIN=49`.
- `PUBLIC_REPOSITORY_SANITIZATION_GATE1_SURFACE_AUDIT.md`: **140 main history commits; 143 with local stash reachable**, 254 distinct historic path names; history changes to workstation/SSH patterns demonstrable but full old-blob secret scanning not proven. GitHub: one public main branch, no tags/releases/PRs/issues, metadata forks=0, **22 Actions runs**; no currently stored Actions artifacts in 22 examined run-artifact collections; 21 reachable job logs scanned heuristically with no recognized key/token patterns or current bench-path signature, one failed historic run log unavailable. Caches and external mirrors `NOT_CHECKED`.
- `PUBLIC_REPOSITORY_SANITIZATION_GATE1_DEPENDENCIES.md`: 254-file exact-text reference graph, including cited plans/evidence history and PowerShell tool entry points; not yet build/ABI/call-graph validation.

**Gate-1-A content-inventory evidence: PASS (bounded 254/254 read). Gate-1 final acceptance: OPEN/NOT PASSED**. Exact risk/consumer sign-off is incomplete, 49 `RETAIN` categories unresolved and no full historical/blob, cache/mirror or private preservation proof exists. The presence of completed read-only checks does not satisfy the final Gate-1 consumer/dependency and exposure predicates. **Gate-2 is not authorized** from this evidence.

### Gate-1A read-only audit snapshot publication — externally VERIFIED

The bounded **10-Markdown** publication commit `536fcac17cf854fb33cf2fcbf693623ff57ecd6a` (three new public-safe Gate-1 evidence documents and seven canonical/chronology status owners) was committed normally and pushed **ordinary non-force** from its exact parent `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`. Independent GitHub main ref and local fresh fetch verified `HEAD == origin/main == FETCH_HEAD == 536fcac17cf854fb33cf2fcbf693623ff57ecd6a`, clean index/worktree, ahead/behind `0/0`. Hosted exact-SHA [GitHub Actions run 38084119171](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38084119171) completed SUCCESS: tracked-output hygiene `257` PASS, non-vacuous patch/whitespace PASS, Release `0 warnings / 0 errors`, Core `84/84`, Transport `24/24`, `0 failed / 0 skipped`.

**Result:** the Gate-1A *read-only evidence publication* is ACCEPTED; **Gate-1 final acceptance remains OPEN/NOT PASSED**, 49 `RETAIN` risk/consumer decisions and the historical/private/cloud proof limitations are unchanged. Publication is not deletion, private preservation or authorization to proceed to Gate-2.

### Independent Gate-1A re-audit — corrected interpretation, not full acceptance

`docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_INDEPENDENT_REAUDIT.md` independently checks the frozen 254-row matrix against a later 257-tracked-file snapshot and confirms exact-SHA CI outcomes. It documents one substantive interpretive correction: **115/115 Markdown basename citations do not prove real product/build dependencies**; after excluding seven named governance/ledger documents from baseline Markdown-only references, **20/115** are cited only by those governance sources. It also reconciles stale Gate-1 status labels and checklist nesting. The original Gate-1A published evidence remains a valid bounded snapshot, **Gate-1 parent stays OPEN** and no history, source, firmware, archive or public/private implementation is authorized.

### Gate-1B independent consumer/history ledger — partial evidence, still OPEN

The docs-only source `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_EVIDENCE.md` inventories 98/98 tentative `PRIVATE`/`RETAIN` dispositions from the original 254-row matrix against the newer 258-path HEAD: 14 PRIVATE and 6 RETAIN have only baseline governance/ledger Markdown citations after excluding later audit reports; nine scoped documents are cited by README/script README. A static include/project check resolved 94/94 local C/header includes and 19/19 Host project/solution references. All 143 public-main commit patch texts were processed without final truncation; 10 historical binary-diff `CHANGELOG.md` object versions were independently fetched. **No one of the 98 has an approved final publication owner decision, and 1,356 historical blobs, private archive, inaccessible GitHub caches/third-party replicas and executable Windows/recovery parity remain unaccepted.** Audit evidence is not permission to remove, redact or migrate any tracked path. Gate-1 final status stays OPEN.

### Gate-1B read-only evidence publication — ACCEPTED/PUBLISHED (not Gate-1B closure)

The bounded seven-Markdown Gate-1B evidence commit `3c0c5343ab66f2757702a2c41d75c869102c19f7` was committed normally and published non-force from clean parent `300747134dbcd9e72687192b6e81fa9a72f995d9`; independent fresh-fetch verified `HEAD == origin/main == FETCH_HEAD == 3c0c5343ab66f2757702a2c41d75c869102c19f7`, clean `0/0`. Hosted exact-SHA [GitHub Actions run 38088050242](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38088050242) completed SUCCESS: Core `84/84`, Transport `24/24`, Release `0 warnings / 0 errors`, failed/skipped `0/0`, tracked-output hygiene `259` PASS. The seven changed paths were Markdown-only. The attempted separate DEUS-managed Host Release build had an ambiguous control-plane result and is **not** counted as local PASS. **Only the Gate-1B evidence-publication slice is accepted; 98 owner decisions, exhaustive blob/hosting coverage, private archival proof and real Windows/recovery behavior remain OPEN.**

### Reachable-main history and current-tree delta — TECHNICALLY VERIFIED, NOT PRIVACY ACCEPTANCE

Read-only script `scripts/public_repository_history_audit.py` independently read **1,367/1,367** blob contents and **30,317,374** bytes from all objects reachable under `95d43fb7e5a6cd5c0d024a97c3e2f0e925c8cb84`, with zero unreadable/truncated blobs and zero selected known credential signature matches. **150 historical blob versions** contained a Windows workstation/project-path pattern and **21** contained a Unix home-path pattern, confirming residual historical exposure. Five additional already published audit reports were screened and classified in the additive ledger `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_RECONCILIATION.md`; proposed final path arithmetic after adding the two new scoped outputs is **261 = 148 PUBLIC + 15 REDACT + 49 PRIVATE + 49 RETAIN**. This is a bounded recommendation and full reachable-main object-read proof, not signed owner privacy decisions, external cloud cache/clones coverage, private archive proof or Gate-1 final PASS. All 98 PRIVATE/RETAIN decisions and 15 REDACT specs remain OPEN; no Gate-2 mutation is authorized.

## 2. Private archive and exact provenance restoration

**PASS:** agreed private archive location, access restrictions, encrypted transport/storage when necessary, explicit ownership; verifiable manifest of accepted legacy documents, original Git commits/trees, firmware source and protocol identities, accepted Gate hashes, signed-recovery constraints and preservation hashes. Independently restore sampled archive content and validate permission denial to an unauthorized public principal. Private backups must be excluded from current and future public remotes.

**FAIL:** deletion/rewrite first; archive merely on a public branch or publicly accessible artifact; unverifiable manifests; accidental production-key copy; unusable links from build/acceptance after move.

## 3. Release topology decision and explicit authority

**PASS:** compare (A) new sanitized public repo, (B) approved existing-history rewrite, (C) prospective-only cleanup. Discuss Git SHA/tag identity breaks, forks/cached copies/search crawlers, licenses/attribution, CI/artifact/issue migration, developer clone instructions, in-flight refs, residual irreversibility. Freeze actual source/doc allowlist and human owner approval; obtain **explicit separate authorization** before any force/non-fast-forward/visibility/new-remote operation. Unapproved strategy is BLOCKED, not silently selected.

**FAIL:** unreviewed history rewriting or `push --force`, treating history replacement as guaranteed deletion from all clones, or unapproved repo visibility changes.

## 4. Authorized minimal implementation

**PASS:** only Gate-3-reviewed paths changed with exact original-to-public mapping; all `REDACT` originals preserved privately; public documentation remains coherent and usable; acceptance material required for external reproducibility remains accessible in non-sensitive form. CI, firmware, Host, build, scripts and protocol files are untouched unless specifically included in the reviewed list.

**FAIL:** mass deleting `docs/`, license loss, one global current-state authority erased without replacement, acceptance evidence irretrievably lost, new exposure created by logs/patches or speculative source/hardware changes.

## 5. Independent post-change review

**PASS:** public fresh-clone tests of README/build/license/links/CI, exact test count and warning-free Release, source/product ABI parity and deterministic ARM proof when relevant, independent scanning of current public view **and relevant history/hosting surfaces**, private archive restore check and explicit residual exposure disclosures. No claim that links to already-forked material disappeared.

**FAIL:** vacuous scan; skipped/unknown tests described as passing; fresh-clone build fails because necessary file was privatized; undeclared historical exposure; unverified private archive.

## 6. Controlled publication

**PASS:** normal staged diff/`git diff --cached --check`/non-force commit and fresh-fetch exact-SHA CI for the non-destructive strategy. Any approved exceptional history strategy requires its **own** signed-off, separately audited workflow and documented nonrecoverable consequences. Confirm public GitHub project content and private preservation each independently after action.

**FAIL:** hidden force, remote divergence, missing GitHub authority or exact-sha CI, or contaminated public snapshot.

## 7. Final sanitized release acceptance

**PASS:** every `RETAIN` resolved or documented as an approved stop, final public/private inventories/pointers and manifest are reconciled, external disclosure and unavoidable history/fork residuals disclosed, source-of-truth ownership and build/recovery invariants preserved, publication committed and accepted. Mark sanitization CLOSED only after actual hosted/public/archival evidence.

**FAIL:** remaining unclassified risk, secret exposure, broken build/public licenses, lost accepted evidence, or unsupported erase claim.

## Failure classifications

`PUBLIC_SAN_PRESTATE_DRIFT`, `PUBLIC_SAN_SCOPE_BREACH`, `PUBLIC_SAN_UNCLASSIFIED_PATH`, `PUBLIC_SAN_UNCHECKED_HISTORY_OR_HOSTING`, `PUBLIC_SAN_UNSAFE_PUBLIC_EXPOSURE`, `PUBLIC_SAN_PRIVATE_ARCHIVE_MISSING`, `PUBLIC_SAN_PROVENANCE_LOSS`, `PUBLIC_SAN_BUILD_OR_ABI_REGRESSION`, `PUBLIC_SAN_PUBLICATION_UNAUTHORIZED`, `PUBLIC_SAN_CI_FAILURE`, `PUBLIC_SAN_ENVIRONMENT_PROOF_UNAVAILABLE`.

**Privacy note:** write sensitive file-by-file findings only to the later approved private sink, not into this public-facing acceptance plan or a shareable evidence ZIP. GitHub Actions cannot retroactively guarantee that public clones/forks never received earlier content.
