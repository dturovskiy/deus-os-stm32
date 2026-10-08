# Deus OS — Repository CI Baseline Plan

Status: **RDC-05 GATES 0–4 ACCEPTED / CI IMPLEMENTATION PUBLISHED `7f75cffdd0c632c5f99310f2c8708d745769aa42`**

Boundary ID: `REPOSITORY_CI_BASELINE`
Residual-debt item: `RDC-05`
Published source baseline: `58d4255d137c7c0b126cf647bba0d635b2a1dafe`
Canonical current-state owner: `docs/CURRENT_STATE.md`
Acceptance owner: `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md`

## 1. Problem, authority and constraints

At the RDC-05 Gate-0 baseline the repository had no tracked `.github/` workflow, so deterministic Host Core/Transport builds and tests depended on manual acceptance runs. RDC-05 adds a small, reproducible **repository CI gate**, not a product feature or a replacement for the accepted Windows/Mac-mini/STM32 hardware evidence. `RESIDUAL_DEBT_CLOSURE_PROGRAM` owns sequencing; `CURRENT_STATE` alone owns activation.

Gate 0 was docs-only and did **not** authorize `.github` workflow mutation until this design/acceptance pair was independently checked, accepted and published. No changes to firmware sources, linker, bootloader, protocol, Host production/test code, or physical target are required by the design.

## 2. Bounded selected implementation

Initial implementation is **exactly one tracked workflow**: `.github/workflows/ci.yml`. The change is not a general CI framework and must not add another runner abstraction, reusable workflow, helper script, dependency, or test fixture unless a separately reviewed Gate-0 amendment proves it necessary.

- Triggers: `pull_request` targeting `main` and `push` to `main`; a manual `workflow_dispatch` is optional only if it adds no bypass. No `pull_request_target`, scheduled hardware probing, repository writes or auto-commit.
- One GitHub-hosted `ubuntu-24.04` job, bounded timeout, repository `contents: read` permissions, no external secrets or privileged tokens, no self-hosted runner and no target-device access.
- `actions/checkout` and `actions/setup-dotnet` must be from official publishers and fixed to audited **full immutable action commit SHAs** at implementation time, not floating tags. `setup-dotnet` must use `host/global.json` with the existing `.NET 10.0.100`, `latestFeature`, `allowPrerelease=false` policy; the installed SDK and active `global.json` must be visible in job output.
- `host/global.json` already selects `test.runner=Microsoft.Testing.Platform`; the test projects use `xunit.v3.mtp-v2` and `host/Directory.Build.props` treats warnings as errors and enables deterministic compilation. **Never fall back to VSTest.**
- Commands execute with **working directory `host/`**; pass solution and test paths relative to `host/`. Prove `dotnet --info` recognizes `host/global.json` before restore/build/tests.
- Set a job-scoped owned `DOTNET_CLI_HOME` and `NUGET_PACKAGES` under runner temporary storage. Do not depend on residual `obj/project.assets.json`; execute explicit `dotnet restore DeusOs.Control.sln --locked-mode` before any `--no-restore` work. Initial implementation may omit dependency caching rather than introduce another trust surface.
- Build the Host solution in `Release` with `--no-restore` and warnings-as-errors. Run both existing **Core** and **Transport** test applications via .NET 10 **MTP mode** (`dotnet test --project tests/...csproj ...` or directly launched prebuilt MTP executables), after successful restore and build. Record real execution counts; test discovery or source `[Fact]` counting is not acceptance.
- Baseline regression floor: at least **84 executed Core tests** and **24 executed Transport tests** based on published accepted host evidence; zero failures, errors, skipped or unexecuted tests. A larger count is acceptable only with corresponding real run evidence. The run must fail closed on a wrong runner, missing results, absent project, command timeout or nonzero exit.
- Static/repository hygiene must check patch whitespace (`git diff --check` for the relevant change range), no tracked generated build outputs/secrets/evidence, expected workflow syntax and exact no-target-I/O scope; the check must not be a no-op against an always-clean checkout. Worktree cleanliness at the end is measured after generated `host/bin` and `host/obj` remain ignored.

## 3. Explicit exclusions

No firmware build/toolchain provisioning, flashing, STM32CubeProgrammer, physical OLED, ST-LINK, CH340/UART, USB management, Linux `libusb` device access, macmini SSH/SCP, remote secrets, signed firmware packages, key-bearing bootloader or physical reset/reconnect. These remain governed by their own acceptance plans and operating topology. A CI-green Host build is **not** a statement of firmware equivalence or physical USB acceptance.

Do not silently alter `host/global.json`, package lock files, SDK policy, compiler warnings, test projects, `.gitignore`, accepted firmware/recovery scripts, or repository governance to accommodate an immature workflow. Investigate CI environment/test semantics first; any unavoidable scope expansion needs an explicitly re-frozen design.

## 4. Sequenced gates

- **Gate 0:** this plan and scoped acceptance plan; docs-only correction of stale RDC-04 governance wording; source, workflow, index and remote unchanged; docs-only acceptance commit and non-force publication before Gate 1.
- **Gate 1:** implement one minimal workflow with pinned actions and least privileges; local YAML/static/security checks; exact workflow-only WIP scope.
- **Gate 2:** validate deterministic workflow/test primitives before publication: locked restore, Release builds, MTP test execution in an accepted local Linux/WSL or equivalent representative environment, plus syntax/security/static and fail-closed negative cases. Local proof cannot be reported as GitHub-hosted CI; authoritative hosted acceptance occurs after publication in Gate 4.
- **Gate 3:** one local acceptance commit of the frozen workflow/docs candidate after exact staged-path and clean-index checks.
- **Gate 4:** ordinary non-force publication, fresh-fetch `HEAD == origin/main` clean `0/0`, and a completed successful GitHub Actions run tied to the **published commit SHA**. If the workflow first runs in a PR, that pre-publication proof may supplement but not replace final published-SHA proof.

## 5. Acceptance and exit

CI is accepted only when a fresh hosted run proves both Host test suites executed successfully, deterministic build/static checks passed, no sensitive or generated artifacts were committed, the workflow cannot use physical bench interfaces or elevated write permissions, and the exact accepted source/docs change is published non-force. RDC-06 is activated only after `CURRENT_STATE` is updated and the separate RDC-05 docs-closure commit is published.

## 6. Accepted implementation and hosted proof

- Gate-0 docs-only publication: `49392d003abf26636a8717a9e27c5520bf4c1af4`.
- Gate-1 candidate: one workflow `.github/workflows/ci.yml`; no firmware, Host source or target mutation.
- Gate-2 Mac-mini Ubuntu evidence SHA-256: `9ABE736EA688352C570B3D0443BABC797A8E240B27FE2107CFF40940305BF651`; locked restore, warning-clean Release, Core `84/84`, Transport `24/24`, skipped `0`.
- Initial Gate-3/4 workflow commit `19025ed7d695bf75b43b010ae2e1d0a1ba105d4d`: published non-force; hosted run `37821098177` failed before job creation because `runner.temp` was used at invalid job-level context.
- Scoped CI-only repair commit `7f75cffdd0c632c5f99310f2c8708d745769aa42`, tree `69cdb0a83b6a5454582343c6d20239761087809d`, was published non-force; accepted publication evidence SHA-256 `9C701DACDD13E5CFF96F22CC79B86C1DC29889BB6D21B474D0A21FFBE59D0276`.
- GitHub-hosted authoritative [run `37829541187`](https://github.com/dturovskiy/deus-os-stm32/actions/runs/37829541187) is `completed/success` with `head_sha=7f75cffdd0c632c5f99310f2c8708d745769aa42`: Ubuntu 24.04, .NET SDK `10.0.401`, locked restore PASS, Release build `0 warnings / 0 errors`, MTP Core `84/84`, Transport `24/24`, zero skipped, non-vacuous patch/tracked-file hygiene PASS.

RDC-05 is **CLOSED/PUBLISHED** at the accepted CI implementation commit. This documentation reconciliation records the disposition and authorizes only RDC-06 Gate-0 planning after ordinary publication; no hardware acceptance is implied.
