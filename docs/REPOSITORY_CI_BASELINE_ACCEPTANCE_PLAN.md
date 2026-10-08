# Deus OS — Repository CI Baseline Acceptance Plan

Status: **RDC-05 GATES 0–4 ACCEPTED / CI IMPLEMENTATION PUBLISHED `7f75cffdd0c632c5f99310f2c8708d745769aa42`**

Design: `docs/REPOSITORY_CI_BASELINE_PLAN.md`
Boundary ID: `REPOSITORY_CI_BASELINE` / `RDC-05`
Source baseline: `58d4255d137c7c0b126cf647bba0d635b2a1dafe`

## Gate 0 — docs-only design freeze and publication

PASS requires all of the following, on the exact published baseline:

1. Two new scoped documents: `docs/REPOSITORY_CI_BASELINE_PLAN.md` and this acceptance plan; existing `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` changes only to reconcile the obsolete `RDC-04 — ACTIVE` subsection with its already published closure.
2. Gate-0 source/tree prestate: clean `main`, `HEAD == origin/main == 58d4255d137c7c0b126cf647bba0d635b2a1dafe`, no staged or unrelated WIP, neither proposed scoped doc exists, no tracked `.github/` workflow.
3. Plan and acceptance scope are congruent: no target I/O, no firmware/Host source mutation; minimal Linux-hosted CI; real MTP and locked restore; baseline Core 84, Transport 24; remote publication and actual hosted CI success required before RDC-05 closure.
4. After docs installation: exactly three WIP paths (two new docs, one modified program plan), unchanged real staged entries, `git diff --check` PASS, exact SHA-256 evidence and byte-exact rollback on FAIL. No commit, push, or Flash in the docs-install acceptance harness.
5. Gate-0 acceptance occurs only after one separate normal docs-only commit and separate ordinary non-force publication with fresh-fetch clean `HEAD == origin/main` and ahead/behind `0/0`. The existence of these documents alone is not a Gate-0 PASS claim.

## Gate 1 — minimal workflow source/static acceptance

- One new `.github/workflows/ci.yml`, and no other source/test/workflow changes without a documented reopened scope review.
- YAML parses unambiguously; triggers are safe (`push`/`pull_request` on main), immutable audited official action SHAs and `contents: read` least privilege, no `pull_request_target`, secrets, self-hosted runner, write token, elevated permissions or hardware/remote commands.
- The job uses `ubuntu-24.04`, .NET 10 via `host/global.json`, working directory `host`, isolated job-owned restore state, explicit solution locked restore before Release build/tests, and MTP rather than VSTest.
- Static whitespace/ignored-artifacts and expected source-path checks are non-vacuous. Preflight validates workflow language/schema and negative fixture cases where safe.
- Real Git index and target remain unchanged before the later commit gate.

## Gate 2 — pre-publication local primitive/test acceptance

- Use an independently proven local Linux/WSL or equivalent representative environment with .NET 10 and no bench interface. Retain the source tree, OS/SDK report, working-directory/global.json recognition, command argument vectors, restore/build stdout/stderr and executed test summaries. The genuine hosted-run URL/id/runner/conclusion cannot be required until the workflow is published in Gate 4.
- Solution restore uses `--locked-mode` and succeeds before `--no-restore` build/tests. No silent dependency lock edits or inherited unknown caches.
- Release build succeeds with warning count zero for the scoped Host solution.
- `DeusOs.Control.Core.Tests`: **at least 84 executed tests**, failures/errors/skips/not-run zero.
- `DeusOs.Control.Transport.Tests`: **at least 24 executed tests**, failures/errors/skips/not-run zero.
- Each test summary is from an actually executed runner with exit code zero; discovery or static source count alone is invalid. Unexpected MTP/VSTest selection, missing results, nonzero exit, timeouts and unsupported native dependencies cause a classified stop, never an automatic test-count downgrade.
- Static/git hygiene succeeds; no STM32 target, USB cable, UART, ST-LINK, macmini SSH or firmware-Flash operations are run.
- On local primitive or later hosted GitHub runtime failure distinguish CI/HARNESS, dependency/network/runner, and actual host test/product failure; preserve raw logs and retry only after classifying the primary failure.

## Gate 3 — local workflow acceptance commit

PASS requires exact reviewed workflow/docs path set, no other source/test/firmware change, `git diff --cached --check`, clean post-commit worktree/index, exactly one normal commit and matching candidate tree. No push or physical mutation occurs at this gate.

## Gate 4 — ordinary publication and published-SHA CI proof

Fetch and verify the direct parent, ordinary non-force push, fresh fetch proving `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`, then independently establish a completed GitHub Actions run whose `head_sha` is exactly the published acceptance commit and whose scoped job `conclusion` is `success`. Retain that run URL/id, runner OS, SDK and test/build logs. Gate-2 local proof cannot substitute for this published-SHA hosted proof; evidence must identify any unpublished PR proof separately. No publication/CI success claims from workflow syntax alone.

## Evidence and stop rules

- The operator return artifact for local gates is **one self-contained `.evidence.zip`** containing `run.log`, `outcome.txt`, raw outputs, SHA-256 manifest and Git/candidate identity. No loose duplicate log by default.
- Every operator-facing harness follows `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`: primitive-first, execution-domain ownership, bounded processes, exact pre/post locks, rollback before commit, timestamped sections, truthful colored final RESULT and classification.
- Gate-0 docs-only failure preserves exact existing bytes and a clean index. Gate-2 CI failure must not be disguised as firmware or hardware regression. Never bypass endpoint security, skip locked restore, downgrade test counts, weaken action/permissions policy or push an unreviewed workaround.
- No closure until all Gate 0–4 obligations are independently accepted/published. `CURRENT_STATE` alone authorizes RDC-06 afterward.

## RDC-05 final acceptance adjudication

- Gate-0 plan/acceptance publication `49392d003abf26636a8717a9e27c5520bf4c1af4` — accepted.
- Gate-1 one-workflow installation — accepted.
- Gate-2 Mac-mini Ubuntu controlled SSH evidence SHA-256 `9ABE736EA688352C570B3D0443BABC797A8E240B27FE2107CFF40940305BF651`: real locked restore and Release build, Core `84/84`, Transport `24/24`, failed/skipped `0`; no STM32 access — accepted.
- Initial Gate-3/4 workflow publication `19025ed7d695bf75b43b010ae2e1d0a1ba105d4d`: source/remote publication accepted but hosted run `37821098177` failed before job creation (invalid `runner.temp` expression at job-level env), so it did not satisfy hosted acceptance.
- Exact workflow-only context repair at `7f75cffdd0c632c5f99310f2c8708d745769aa42`, source tree `69cdb0a83b6a5454582343c6d20239761087809d`; accepted non-force publication evidence SHA-256 `9C701DACDD13E5CFF96F22CC79B86C1DC29889BB6D21B474D0A21FFBE59D0276`.
- Hosted run `37829541187` (`https://github.com/dturovskiy/deus-os-stm32/actions/runs/37829541187`) on the exact accepted SHA is `completed/success`, including the real **Core 84/84** and **Transport 24/24** MTP summaries, skipped `0`, warning/error-clean Release, and repository/static checks. `setup-dotnet` selected SDK `10.0.401` in accordance with the existing `host/global.json` roll-forward policy.
- Final Git identity after publication: `HEAD == origin/main == FETCH_HEAD`, clean index/worktree, ahead/behind `0/0`. No target I/O, Flash or firmware mutation.

**Adjudication:** RDC-05 Gates 0–4 **PASS**, boundary **CLOSED/PUBLISHED**. The distinct docs-only reconciliation commit/push is an administrative post-acceptance step; it does not substitute for or invalidate the accepted CI run on `7f75cffdd0c632c5f99310f2c8708d745769aa42`. The current active boundary is advanced exclusively by `docs/CURRENT_STATE.md` after that reconciliation is published.
