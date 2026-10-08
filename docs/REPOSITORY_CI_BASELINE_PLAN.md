# Deus OS — Repository CI Baseline Plan

Status: **RDC-05 GATE-0 DESIGN FROZEN FOR REVIEW — NOT YET ACCEPTED/PUBLISHED**

Boundary ID: `REPOSITORY_CI_BASELINE`
Residual-debt item: `RDC-05`
Published source baseline: `58d4255d137c7c0b126cf647bba0d635b2a1dafe`
Canonical current-state owner: `docs/CURRENT_STATE.md`
Acceptance owner: `docs/REPOSITORY_CI_BASELINE_ACCEPTANCE_PLAN.md`

## 1. Problem, authority and constraints

The published repository has no tracked `.github/` workflow; deterministic Host Core/Transport builds and tests presently depend on manual acceptance runs. RDC-05 adds a small, reproducible **repository CI gate**, not a product feature or a replacement for the accepted Windows/Mac-mini/STM32 hardware evidence. `RESIDUAL_DEBT_CLOSURE_PROGRAM` owns sequencing; `CURRENT_STATE` alone owns activation.

Gate 0 is docs-only. It does **not** authorize `.github` workflow mutation until this design/acceptance pair is independently checked, accepted and published. No changes to firmware sources, linker, bootloader, protocol, Host production/test code, or physical target are required by the design.

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

CI is accepted only when a fresh hosted run proves both Host test suites executed successfully, deterministic build/static checks passed, no sensitive or generated artifacts were committed, the workflow cannot use physical bench interfaces or elevated write permissions, and the exact accepted source/docs change is published non-force. RDC-06 remains queued until `CURRENT_STATE` explicitly advances after RDC-05 publication.
