# Deus OS — Public Repository Documentation-Only Sanitization Evidence

> **STATUS: SCOPED DOCS-ONLY SANITIZATION PUBLISHED/TECHNICALLY ACCEPTED at `c55c90c4b40605adc2e3da26a7314467c618c512`, hosted exact-SHA [CI 38140063373](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38140063373) SUCCESS; full Gate-1 sanitization NOT ACCEPTED.** This is scoped technical evidence subordinate to `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`, not a replacement for the sole project-state source `docs/CURRENT_STATE.md`. No statement in this file authorizes code edits, Git history rewrites or hardware work.

## Frozen publication basis

- Accepted pre-edit public Git commit: `f728e49486d7ff0dba4bb1e45d9b064d6e6c6d7d`, `main == origin/main`, clean `0/0`, 262 tracked paths. Hosted exact-SHA CI `38137758573` SUCCESS (Core 84/84, Transport 24/24, Release 0 warnings/errors; hygiene 262).
- Historic baseline remains the immutable 254-path initial matrix. Later addenda account for the 262-path snapshot. This document adds no competing classification authority.
- Authorized execution scope for this slice: **existing Markdown content and public-safe evidence only**, never PowerShell executable files, C/ASM, C#, scripts, firmware, bootloader, CI, signed packages, ST-LINK, USB, UART, flash or device reset.

## Archive-first evidence — 19 source Markdown files

- An **owner-controlled private GitHub repository** was initialized **outside the public OS working tree**, without any firmware/program source. An authenticated read-only GitHub CLI check returned repository visibility `PRIVATE`.
- Private original snapshot first committed at `476bb6266b434eaf95e13e1bf043c7b7852d6aab` (16 docs). Three additional original Markdown sources were appended non-force at private commit `3f68e4fd3a704854bc19b327ea818b2a1b6bc00a` (19 total). The private repository's manifest contains full source-path-to-Git-blob identity and original public `f728e494...` SHA binding. The private repository identifier and original sensitive values are intentionally absent from this public report.
- **Byte identity proof:** the 19 staged private original Git blob OIDs match 19/19 original public Git blob OIDs at pre-edit HEAD. For the first 16, an additional remote-fetched readback of every file yielded 16/16 exact content equality with the public pre-edit source. For the final three, staged blob equality and successful non-force private remote publication were independently checked. No original was deleted.
- **Bounded limitation:** `PRIVATE` visibility and authenticated Git push/fetch do not independently prove anonymous-access denial, encryption policy, restore of the full Git history or completeness of unknown/unreachable GitHub backups. Gate-2 final acceptance is NOT claimed by these tests.

## Current public documentation edits — 13 paths, all in-place

These are the only existing Markdown documents authorized for safe operator-value substitutions in this slice:

| Current public path | Scope |
|---|---|
| `docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md` | Replace local SSH endpoint/alias literal; preserve acceptance facts |
| `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md` | Replace absolute workstation and WSL directory literals |
| `docs/CURRENT_STATE.md` | Replace absolute workstation root; preserve global state authority |
| `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` | Substitute operator paths, SSH endpoint/alias and username; retain public interface ownership/pin map, label safe public view |
| `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md` | Substitute local SSH endpoint/alias/username; preserve hashes, test gates and bootloader contract |
| `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` | Substitute local paths/SSH identity, label public-safe operational view; preserve safety and oracle rules |
| `docs/PROJECT_HANDOFF.md` | Substitute local paths/SSH identity, label public-safe handoff view |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_ACCEPTANCE_PLAN.md` | Substitute workstation/WSL paths without changing closure proofs |
| `docs/RESIDUAL_DEBT_DOCUMENTATION_CLOSURE_PLAN.md` | Substitute workstation/WSL paths without changing architecture or closure claims |
| `scripts/README.md` | Replace example operator path and SSH endpoint/alias; no script execution |
| `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md` | Use generic headless-Linux host terminology |
| `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md` | Use generic cross-host SSH terminology |
| `docs/REPOSITORY_CI_BASELINE_PLAN.md` | Use generic cross-host SSH/SCP terminology |

Six older baseline REDACT documentation rows did not require an actual operator literal change under the bounded current-pattern inventory and remain untouched in their substantive content: `CHANGELOG.md`, `README.md`, `docs/ARCHITECTURE.md`, `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`, `docs/DOCUMENTATION_MODEL.md`, `docs/ROADMAP.md`. Later changes to governance/status files are bookkeeping rather than operator-value redaction.

## Actual post-edit documentation proof (before evidence document creation)

- All **122/122** then-tracked `.md` paths read, no truncated or missing file. After authoring this evidence document and the current-state/roadmap updates, **123/123** current candidate Markdown paths were independently reread, with zero matches against the four bounded operator-literal patterns, zero matched typical PEM/GitHub token headers, and zero missing simple relative Markdown link targets.
- All four bounded operator-literal screens returned **0** in the then-current Markdown tree: absolute Windows `Users/Projects/Tools` path prefixes; Unix `/home` or `/Users` home-prefix paths; literal concrete SSH account-and-host addresses; and the particular prior operator SSH alias. Screening catches named forms only and is **not** proof that no other confidential semantic information exists.
- The nine README-dependent previous PRIVATE/RETAIN paths remain present, and the tested seven relative Markdown links all target existing paths: **0 broken targets** within the scanner's simple relative-link grammar. Full anchors/semantic dependency proof is not claimed.
- All changed existing files are Markdown. **No `.ps1`, `.py`, `.c`, `.h`, `.s`, `.cs`, `.csproj`, build source or CI config changed.** Current published main still retains every historic public commit and all previously public filenames, with no force, delete or rename.
- The seven legacy PowerShell files with workstation-specific tool/programmer defaults are **intentionally unchanged**. Altering these executable defaults requires a distinct owner-approved code/recovery equivalence lane; this documentation-only request does not authorize it.
- External clones/mirrors, GitHub caches and public prior-commit blobs can retain original workstation/operator values. No future-only public document change can make those copies disappear.

## Published exact-SHA verification

- The 21-path **Markdown-only** package was committed and published by ordinary non-force push at `c55c90c4b40605adc2e3da26a7314467c618c512`; independent fresh fetch confirmed `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`; **263 tracked paths**. No executable source, PowerShell, CI, ROM/Flash image or target-related path changed.
- Hosted exact-SHA [CI 38140063373](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38140063373) **SUCCESS**; independently read job log confirmed Core **84/84**, Transport **24/24**, Release **0 warnings/0 errors**, zero test failures/skips, output hygiene **263 PASS**.
- Independently reran the published read-only Git history scanner on exactly `c55c90c4b40605adc2e3da26a7314467c618c512`: **1,414/1,414** reachable blobs, **32,384,087 bytes**, zero unreadable/truncated, zero selected recognized credential-format matches. Historical Windows path indicators still present in **154** blob versions and Unix home path indicators in **21** blob versions; these are not erased by this future-only redaction.
- Private documentation archive stayed separate, `PRIVATE`, clean and synchronized at `3f68e4fd3a704854bc19b327ea818b2a1b6bc00a`. The archive contains **19 original Markdown blob identities** and no firmware/code.
- This is **accepted proof of documentary publication and scoped operator-literal redaction only**, not security clearance for all past public history, caches, seven unchanged PowerShell executable defaults, or Gate-1 / Gate-2 full acceptance.

## Gate decision

- **PASS candidate**: archive-first 19-document preservation at a verified private-visibility repository, minimal content-only operator-value cleanup, stable public paths and docs link targets, bounded current-tree screening.
- **REQUIRES POST-PUBLICATION PROOF**: scoped staged diff/whitespace check, exact public commit SHA, hosted Core 84/84 + Transport 24/24 + Release clean, tracked-file hygiene and independently refreshed `HEAD == origin/main == FETCH_HEAD` clean `0/0`.
- **OPEN by explicit agreement**: no PowerShell code changes, no historic public Git object rewriting, no archive of code, no third-party cache deletion. Gate-1 / overall sanitization remains OPEN until separately authorized code handling and historical exposure/residual risk decisions are independently accepted. Do not call the complete repository secret-free or sanitized based on this docs-only slice.
