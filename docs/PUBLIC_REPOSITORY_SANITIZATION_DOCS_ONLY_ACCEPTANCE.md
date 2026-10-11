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

## 2026-10-11 semantic operator-reference review — second documentation-only slice

**Status: second docs-only slice PUBLISHED / technically accepted at `3a5ccfa7db2c156e8a4b396f0837e89a492c6f13` (GitHub Actions run `38145500397` SUCCESS); NOT full Gate-1 acceptance.** This independent continuation covers semantic workstation/bench identifiers missed by the earlier four literal-pattern screens. It does not replace the previous accepted `c55c90c4b40605adc2e3da26a7314467c618c512` evidence.

- Public pre-edit Git baseline: `4588985d68efc8c4669e05d53c87bf257789e085`. No firmware, bootloader, source, PowerShell executables, workflow, private key, local hardware or Git-history mutation is in scope.
- Full current documentation inventory: **123/123 Markdown files readable**. The prior 19-document original snapshot was extended by five originals at private commit `6fdd4a6a91116f8e719e5d1fbf9ca8dcd614f888` and four at `581c7dba8fba3c89e9064af089a3bce3d48f2eed`: **28 preserved Markdown originals** total in a separately authenticated repository reporting `PRIVATE`. Each newly added file was checked byte-identical to its unmodified current-public pre-edit source; source Git blob identities are recorded only in the private manifest. Do not publish that private source content or assume an anonymous-access-denial test from the private GitHub visibility flag.
- The published candidate replaces machine-model references with the **Linux USB host** execution role, retained as distinct from the Windows build/SWD/UART owner. Absolute workstation/WSL path literals and host-local UART enumeration identifiers are excluded where exact original preservation preceded the edit. Product/protocol/Flash/USB/bootloader safety invariants and recorded acceptance hashes are unchanged. All modified existing paths are Markdown. Each operator-bearing Markdown source edited for this redaction has a private original snapshot; this public-safe acceptance addendum is separately maintained without operator data.
- The second-pass selected literal screen on all 123 current Markdown files has **zero** matches for the prior machine-model spelling variants and the identified original absolute WSL checkout path. At the accepted `3a5ccfa7` snapshot, one historic host-local UART COM-port literal remained in `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md`; that original was subsequently privately preserved and the remaining literal was removed in the follow-on docs-only candidate. The earlier blocked archive attempt is a historical event, not a standing blocker. This explicitly bounded disclosure is **not** a known credential/key compromise. Broad semantic, external replica, historic prior-commit and unknown-token completeness is not established by these results.
- Still out of scope: the seven executable PowerShell default-path consumers, legacy public commits/blobs, caches/clones/mirrors, separately controlled signed firmware/recovery bundles, GitHub repository visibility and target Flash/hardware. The parent Gate-1/1B remains OPEN; Gate-2 and subsequent full sanitization acceptance are not implicitly unlocked.

**Published acceptance for this slice:** `3a5ccfa7db2c156e8a4b396f0837e89a492c6f13` changed exactly 26 Markdown files; normal non-force push, clean `HEAD == origin/main`, ahead/behind `0/0`, and exact-SHA [GitHub Actions run 38145500397](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38145500397) SUCCESS. The hosted job executed Core `84/84`, Transport `24/24`, zero failed/skipped, warning/error-clean Release and tracked-output hygiene `263` PASS. The IWDG residual fix is independently scoped below and is not retroactively included in that earlier SHA.

## Final outstanding IWDG operator-literal closure — 2026-10-11

**Scoped status: documentation-only follow-up candidate, pending its own publication and exact-SHA CI confirmation.** The lone outstanding host-specific UART COM-port literal was in `docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md`. Unlike earlier attempts, the complete original was successfully copied to `deus-os-stm32-private-docs/originals/docs/IWDG_LIVENESS_FOUNDATION_ACCEPTANCE_PLAN.md`, checked byte-identical against the current public pre-edit source at `3a5ccfa7db2c156e8a4b396f0837e89a492c6f13`, and published in the **PRIVATE** archive at `18d02e4fd2bb0d24c47b5f4a027ebc0b5c8e3970` (clean upstream `0/0`). The source Git blob is `50e2a3221d4458ccaff80dbf7b7122804759dd55`. This raises the preserved-doc original count from 28 to **29** without any executable changes.

The only edit to the IWDG acceptance contract replaces a specific Windows COM number with the device-discovered CH340 UART COM port. The original watchdog reset/reopen acceptance requirement is preserved. The 29 originals form a *selected redaction-source snapshot*, not a claim to archive all project documentation, all Git history, or external mirrors.

To accept this final follow-up, independently prove: all 123 current public Markdown paths readable; no selected workstation-model literal, raw operator checkout path, or concrete historical UART COM-number literal remains in the current Markdown; current documentation navigation references still resolve; exact docs-only staged path set; no firmware/host/PowerShell/CI changes; clean non-force Git publication and hosted exact-SHA Core `84/84`, Transport `24/24`, warning-clean Release and repository hygiene. Historic reachable blobs, seven unchanged PowerShell defaults, owner decisions and external replicas remain separate Gate-1 blockers; **Gate-1, Gate-1B and Gate-2 authorization remain OPEN/BLOCKED as previously documented**.
