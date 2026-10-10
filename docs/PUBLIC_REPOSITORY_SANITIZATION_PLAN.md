# Deus OS — Public Repository Sanitization Plan

Status: **GATE-0 DESIGN CANDIDATE — DOCS-ONLY PUBLICATION AND EXACT-SHA CI REQUIRED; NO SANITIZATION AUTHORIZED**

Boundary: `PUBLIC_REPOSITORY_SANITIZATION`

Acceptance authority: `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`

Project current-state authority: `docs/CURRENT_STATE.md`; documentation-role authority: `docs/DOCUMENTATION_MODEL.md`.

Frozen initial prestate: `c309428b03615a9afd514e3c7c8886a82a5112e9`, local `main == origin/main == FETCH_HEAD` from the previously independently verified RDC-08 publication, clean index/worktree, ahead/behind `0/0`. This is a **new maintenance/privacy/publication-policy boundary after** `RDC-01..08`, not a reopened RDC Gate, feature, security vulnerability finding or firmware update.

## 1. Problem statement and separation from already accepted work

The GitHub repository is **public**. Its engineering source/docs tree contains much more material than a minimal public SDK/firmware project needs, including historical acceptance recipes and physical operator-topology details. The decision to publish, redact, relocate, summarize or keep such material has **not** been made per file.

Existing authorities were reviewed, not negated:

- `DOCUMENTATION_MODEL.md`: defines source-of-truth precedence and historical preservation **within the engineering repository**; it does **not** define an external-publication classification or permitted private storage boundary.
- `DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` and `_ACCEPTANCE_PLAN.md` (**FDC-10**): repaired stale documentation claims, orphan plan pairings, obvious credential signatures and tracked build/evidence filename hygiene; **not** historical Git content minimization or a public/private release split.
- `RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` and `_ACCEPTANCE_PLAN.md` (**RDC-08**): closed remaining documentation source-of-truth debt, inspected 113 Markdown files and checked obvious token signatures/current-and-history path names; explicitly disclaimed a full historical secret-content audit and required evidence to remain preserved; **not** a decision to publish all such preserved evidence.
- Historical Git commit `9fa772a3eed926262367ed072ffc46ed1c43e413` (**2026-09-21**): hardened `.gitignore` and removed one explicit workstation-account path from the then-current handoff text. It did **not** remove the earlier version from Git history or classify other operator/evidence disclosures.
- `PROJECT_HANDOFF.md`, `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`, `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, `MASTER_EXECUTION_CHECKLIST.md`, `CHANGELOG.md`, historic `*_PLAN.md` and `*_ACCEPTANCE_PLAN.md` have narrow legitimate internal-engineering roles. **Presence is not by itself proof of a secret leak or grounds for blind deletion.**

This plan addresses **publication minimization / attack-surface and privacy reduction** while keeping existing firmware/protocol/recovery/evidence contracts intact.

### Cross-plan gap matrix — verified before drafting

This is a **comparison of authority coverage**, not a claim of a detected credential leak. The review included all 83 tracked `docs/*_PLAN.md` and `docs/*_ACCEPTANCE_PLAN.md` records (40 acceptance, 43 other plans), plus all 30 other tracked Markdown references.

| Finding / control | Prior source covering part of it | What was **not** accepted there / new owner |
|---|---|---|
| Multiple competing project-state summaries | `DOCUMENTATION_MODEL`, FDC-10, RDC-08 | Already resolved as documentation governance; **retain**, do not reopen as a new sanitization defect. |
| Public repo contains dense internal plans and evidence chronology | FDC-10/RDC-08 inventory, `MASTER_EXECUTION_CHECKLIST` | Per-file external-publication necessity and public/private placement **absent**; Gate 1 matrix. |
| Local operator and bench identity/topology details in current text | `DEVELOPMENT_ENVIRONMENT_TOPOLOGY`, `PROJECT_HANDOFF`, historical notes | Risk-based `REDACT/PRIVATE` policy and public replacement **absent**; Gates 1–4. |
| Long `CHANGELOG` and accepted plan/acceptance evidence references | FDC-10/RDC-08 historical preservation rule | Short public release notes vs full private acceptance provenance **not selected**; Gates 1–4. |
| Product architecture/protocol/USB/Flash contracts | `ARCHITECTURE`, protocol and source plans | Public consumer/dependency classification **absent**; don't hide ABI/build requirements blindly; Gate 1. |
| Tracked generated build/ZIP/key artifact exclusion | `.gitignore`, commit `9fa772a`, FDC-10/RDC-08 | Existing bounded checks **pass**; verify no public regression, but do not label every retained engineering file unsafe. |
| Real credentials, key-bearing Flash/bootloader materials | Firmware-update/bootloader safety and script policies | No new leak established; verify historical content/cloud surfaces and contain **confirmed** incidents separately; Gates 1/3/5. |
| Historical repository disclosures after current-file redaction | `9fa772a` current-text path fix, Gate-0 audit | Old reachable blobs persist; strategy must address old commits/refs **and known limits**; Gates 1 and 3. |
| GitHub Actions artifacts/logs/cache/releases, issues/PRs, forks and mirrors | CI baseline/hygiene checks | No complete external-surface classification; enumerate `NOT_CHECKED` where inaccessible; Gates 1/5. |
| Accepted evidence/identity continuity after moving files | `DOCUMENTATION_MODEL`, `PROJECT_HANDOFF`, scoped acceptances | Private archive permissions, manifest and **restore proof missing**; Gate 2. |
| Public build, CI, license, links and onboarding after separation | Existing README and CI plans | Public-fresh-clone consumer proof after sanitization **missing**; Gates 3–5. |
| Destructive publication governance (history rewrite/new repo) | Prior ordinary non-force publication rules | No approved history strategy, rewrite authority or risk transfer; explicit human approval at Gate 3. |
| Full acceptance classification and post-release validation | RDC-08 Gate-8 exact-SHA/CI discipline | New risk categories, disclosure-surface checks and public/private poststate required; Gates 5–7. |

The presence of these **gaps** does not retrospectively invalidate previously published FDC/RDC accepted PASS results; they addressed different contractual questions.

## 2. Read-only Gate-0 factual inventory (not a secret-leak verdict)

Measured at `c309428b03615a9afd514e3c7c8886a82a5112e9` through DEUS-MCP + GitHub:

- Public GitHub repository `dturovskiy/deus-os-stm32`, `main`, clean local Git `0/0`; 252 tracked files, including 110 Markdown in `docs/`, 113 Markdown total, 40 acceptance-plan and 43 other plan-named documents within `docs/`. The 83 plan/acceptance documents and all 113 Markdown files were read completely with no truncation.
- Approximate Markdown corpus: **1,634,063 characters / 30,570 lines**. The corpus contains **982 occurrences of 64-hex identifiers** (not 982 unique secrets, and some are public content hashes) and **102 occurrences of `.evidence.zip` names**. Numerous gate-result, topology and command details are public text already.
- Concrete operator/topology exposure **candidates**, requiring row-level adjudication: `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` (split hosts, operator SSH identity/alias, local roots and live USB ownership), `PROJECT_HANDOFF.md`, `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, `scripts/README.md`, `CURRENT_STATE.md`, and historical acceptance records. Do **not** copy their raw machine identities into new public sanitization logs or examples.
- Internal-detail volume candidates: `CHANGELOG.md`, `MASTER_EXECUTION_CHECKLIST.md`, `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`, completed scoped plans and acceptance plans. Protocol, architecture, source and build contracts may still need selective public retention; file-size or `Gate` token count alone is not a disclosure finding.
- Existing `.gitignore` excludes common images, build products, logs, archives, credentials and IDE noise; tracked-tree / Git-history **filename** checks did not identify a forbidden build/secret artifact class. Previous obvious-key/token-signature checks were negative, but are not an exhaustive historical-content, entropy, binary, GitHub-hosted artifact, release, issue, PR, fork, cache or secret-rotation audit.
- The historical Windows-account path affected by `9fa772a` remains accessible in old reachable Git commits unless the Git graph is changed; deletion from `HEAD` cannot undo disclosure. The observed history item **does not prove credential compromise**.
- Source, bootloader, host application, build scripts, CI and physical STM32 were not modified. Installed signed application security revision **5** and published recovery/bootloader constraints retain their original acceptance authority.

**Audit gaps:** exact file-by-file disclosure decisions; whether non-text sources/config contain embedded identities; whole Git-history content coverage; GitHub Actions logs/artifacts/caches/releases/PRs/issues/forks and public mirrors; private preservation target and restoration proof; inter-document/build/CI link impact; allowed history-migration technique and post-migration release verification.

The figures above are a frozen baseline, **not** a claim that future plan-file additions preserve the counts.

## 3. Scope and minimal-disclosure policy

Create a **public release view** that exposes only the material required to understand, build, test, use or contribute to Deus OS and its supported interfaces. Preserve the complete authoritative engineering archive privately with access controls, verifiable content hashes and recoverable repository/acceptance provenance.

In Gate 1 classify **each of the 252 tracked paths, each relevant reachable historical path/content finding, and each GitHub-side publishing surface**. Each row has exactly one disposition:

| Disposition | Meaning and mandatory consequence |
|---|---|
| `PUBLIC` | Reviewed and retained, exact content needed for product/build/API/contributing/license/CI; no known disallowed disclosure in relevant history. |
| `REDACT` | Public version required but only after removing/generalizing identified operator/internal-only content; full original preserved in private archive, line/source provenance recorded privately. |
| `PRIVATE` | Internal engineering/evidence/operator content relocated to access-controlled archive; public replacement/index, if necessary, uses **non-sensitive** summary/link only. |
| `RETAIN` | Publication disposition **blocked pending consumer/dependency/provenance or risk adjudication**. The full source is preserved; no deletion or public exposure expansion is authorized. Gate-2 removal cannot start on an unresolved `RETAIN` row. |

Tag each row additionally with a bounded risk/consumer reason: `IDENTITY_OR_TOPOLOGY`, `GATE_EVIDENCE`, `HISTORICAL_LEDGER`, `BUILD_OR_CI_DEPENDENCY`, `PUBLIC_ABI`, `LICENSE_NOTICE`, `HOSTING_METADATA`, `SECRET_SUSPICION`, or `OTHER`. A confirmed secret requires **separate incident containment and approved rotation**, not a vague `REDACT` label.

Do not assume all 40 acceptance plans should be private; some accepted protocol, ABI, recovery and provenance interfaces may have a legitimate public consumer. Conversely, a README or source comment is not automatically safe just because it is not in `docs/`.

## 4. Mandatory decision axes

1. **Source-of-truth and archive topology:** specify public vs private repositories/directories with independent permission controls. A private worktree on the same public Git history is **not** a private archive. Before relocating anything, prove that scoped source/acceptance records, release evidence, signing/public-key trust chain, hashes, human decision trail and recovery recipe survive with exact identities and are discoverable by authorized maintainers. Define a public-only `CURRENT_STATE` and roadmap view that cannot accidentally expose a private operator state.
2. **Disclosure model:** classify real credentials vs non-secret hashes, IDs, absolute local paths, usernames, SSH aliases, topology/power ownership, serial/USB identities, debug recovery commands, internal reliability failures and firmware-update internals. Treat risk as contextual; preserve public interoperability specs and safety instructions. Avoid recopying exact sensitive strings into the report.
3. **Git-history scope:** inspect every reachable branch/tag/ref and relevant commits (not only current filenames). Track content across deleted/renamed paths and blobs, old path revisions, merge branches and annotated tags. Record actual scan coverage, limits and findings. A private replacement repository **does not erase public clone/fork/cached copies**.
4. **GitHub external surfaces:** enumerate repository metadata, workflow logs, Actions artifacts/caches, release attachments, issues/PRs/discussions, public forks, mirrors and GitHub indexing where observable; classify `NOT_CHECKED` where tools cannot see a surface. Distinguish revocable material from irretrievable exposure, and never assert universal eradication.
5. **Build and contract dependencies:** audit links, relative paths, included files, build scripts and docs referenced by CI, tests, packaging, developer onboarding, license, future maintenance and hardware safety. Create a public skeleton of release/usage/build docs before removing private originals; ensure `git archive`/fresh clone works.
6. **Repository migration strategy (explicit independent decision):** compare (A) sanitized new public repository with preserved private canonical original, (B) selective history rewrite of existing public repository, (C) ordinary future-only public minimization with acknowledged permanent historical exposure. Provide scope, impact on SHAs/tags/issues, GitHub forks/clones/caches, backups and CI. Do not force-push, change visibility, archive/delete a remote, rotate signing keys or create another public remote on your own.
7. **Safety/identity:** never publish an image, backup or trace containing a compiled production key; do not copy private full-Flash recovery bundles into CI or a public evidence ZIP. Historical synthetic-key equivalence does not authorize synthetic firmware flashing. No STM32 flashing, reset, key extraction, VBUS switching, SWD/UART/native USB or physical test is necessary merely to classify publication surfaces.

## 5. Ordered gates / acceptance boundaries

- **Gate 0 — policy/design freeze:** audit all plan/acceptance material and the public Git inventory; record confirmed present vs missing publication controls; add only this plan/acceptance pair and narrow pointers in current-state/roadmap/documentation model/changelog. Review real candidate diff, commit normally, publish only the **sanitized design text** by ordinary non-force push, fresh-fetch clean `0/0`, exact-SHA hosted CI. This is approval to **audit**, not to remove/migrate documents.
- **Gate 1 — comprehensive source + hosting disclosure matrix:** complete itemized 252-path current tree and reachable Git-history/hosting-surface inventory; classify every source by `PUBLIC|REDACT|PRIVATE|RETAIN`, reason and evidence coverage; separately verify source hashes and exact dependence graph. Expose only high-level counts/public-safe summaries to GitHub; keep private findings encrypted/restricted. **Gate-1 acceptance freezes the exact candidate path and exposure categories before content mutation**.
- **Gate 2 — preservation/archive proof:** establish and read back an access-controlled private canonical archive, including history, accepted source/ABI/recovery contracts and evidence. Verify manifest, hashes, restore and permissions. No public deletion before PASS; never push original sensitive content to an unintended public remote.
- **Gate 3 — release-view and migration-strategy selection:** independently adjudicate options A/B/C against actual risks and GitHub history limitations, with the user's explicit approval required for destructive/non-fast-forward/historical/visibility transformations. Freeze exact public `README`/build/CI/license/API/compatibility/documentation replacement and exact changed path set. If user approval is unavailable, STOP rather than assume.
- **Gate 4 — minimal public sanitization implementation (authorized option only):** make only approved file edits/moves/redactions using stable source-of-truth mapping; scrub sensitive text from newly generated logs and CI outputs; never discard the private original. A simple Git deletion is not historical erasure.
- **Gate 5 — independent privacy/functional proof:** fresh/public-clone leak and source/dependency audit, representative Host Core/Transport Release tests, firmware/source/build checks when source/build paths are changed, real ABI/USB/protocol parity as relevant, `git diff --check`, license/attribution continuity, link resolution and documented residual public historical exposure. No fake passing on unavailable Windows/ARM/hardware evidence.
- **Gate 6 — controlled publication:** normal docs/code commits and exact path review for an ordinary fast-forward approach; for approved history rewriting/new-remote actions use a *separate* explicit operator authorization and risk-controlled publication procedure, not the usual non-force Git gate pretending to suffice. Verify actual external state, GitHub CI exact-SHA, public clone and private archive remain intact.
- **Gate 7 — final current-state/source-of-truth adjudication:** update canonical public/private indexes and histories without leaking private evidence; verify current Git identity, independent provenance mapping and successful publication. Only then mark PUBLIC_REPOSITORY_SANITIZATION CLOSED/PUBLISHED.

A rejected or incomplete risk finding blocks the relevant gate, **not** retroactively closed FDC/RDC gates.

## 6. Gate-0 frozen changed-path boundary and hard stops

Allowed in Gate-0 only:

1. `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` (new)
2. `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md` (new)
3. `docs/CURRENT_STATE.md` (brief activation/pointer only)
4. `docs/ROADMAP.md` (one narrowly scoped priority item)
5. `docs/DOCUMENTATION_MODEL.md` (new public/private authority rule, no rewrite of historical inventory)
6. `CHANGELOG.md` (minimal audit-start chronology, no disclosure examples)

No other path mutation permitted at Gate 0. In particular **do not edit/delete/move** any legacy accepted Gate/acceptance/protocol/host/operator file, source, linker, Flash configuration, `.gitignore`, workflow or Git settings at this stage. No Git-history rewrite, force push, remote visibility change or new public repository until selected and specifically authorized.

**STOP** for missing private preservation proof, missing owner approval on irreversible operations, unclassified secret suspicion, unexplained WIP path, broken protocol/build consumer, undisclosed history limitations, uncertain GitHub-surface coverage or target-device/key access. Proof coverage must be declared, not inferred from the public-tree scan.

## 7. Status / required next action

At this design candidate stage, **Gate-1 has not started and no file has been classified for deletion**. The next substantive action after Gate-0 acceptance is the **complete content/exposure/consumer matrix** and strategy comparison; no physical STM32 interaction and no ZIP scripting required.
