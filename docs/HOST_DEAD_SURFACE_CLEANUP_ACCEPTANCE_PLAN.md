# Deus OS — Host Dead-Surface Cleanup Acceptance Plan

Status: **ACTIVE — GATES 0–3 ACCEPTED / GATE 4 COMMIT PENDING**

Canonical design:

`docs/HOST_DEAD_SURFACE_CLEANUP_PLAN.md`

Residual-debt item:

`RDC-02`

Baseline:

`bc562f5030990d4b4e69af35d0ad1a3e4cb301bb`

## Gate 0 — exact dead/retained freeze

PASS requires:

- exact eight-symbol deletion list from the plan;
- both current runtime/bootloader Linux paths proven to use profile-aware owners directly;
- containing `LinuxLibUsbTransport` type confirmed internal;
- public runtime profile constants retained;
- raw compatibility/protocol APIs retained;
- no target/native behavior/wire/script/hardware mutation authorized.

## Gate 1 — implementation/static review

PASS requires:

- product changes only in the two frozen Linux transport files;
- all eight dead symbols absent after deletion;
- `RuntimeProfile`, bootloader profile, `ValidateTopology(...)` and `Open(..., profile)` retained;
- public runtime USB constants retained;
- no Core/Windows/target source drift;
- `git diff --check` PASS.

### Gate-1 result — PASS

Static review on baseline `bc562f5030990d4b4e69af35d0ad1a3e4cb301bb` confirms exactly five candidate paths: this plan pair, `docs/CURRENT_STATE.md`, `LinuxLibUsbDiscovery.cs`, and `LinuxLibUsbTransport.cs`. Product diff is deletion-only: 14 Linux transport lines removed. All eight frozen remnants are absent; runtime/bootloader profiles, profile-aware `ValidateTopology(...)`, profile-aware `Open(..., profile)` and public discovery constants remain. Core/Windows/target/script source is unchanged. `git diff --check` PASS. Static inventory is Core 84 methods and Transport 22 methods; the two Transport theories each have two InlineData cases, preserving expected executed Transport total `24`.

## Gate 2 — deterministic Host validation

Require:

- Transport `24/24` PASS;
- Core `84/84` PASS;
- Linux transport Release build PASS with zero warnings/errors;
- Core Release build PASS with zero warnings/errors;
- exact candidate pre/post path/hash proof;
- target I/O NONE / Flash mutation NONE.

### Gate-2 result — PASS

Authoritative evidence: `stm32_os_host_dead_surface_cleanup_gate2_host_validation_v1_20261007_193632.evidence.zip`, SHA-256 `44C5DBFA295087F1F2BFD5A453E63C3855EBC0508EEC5D3BEF283F99D4482B84`.

Accepted facts:

- internal evidence hash index `48/48` exact;
- exact prestate `HEAD == origin/main == bc562f5030990d4b4e69af35d0ad1a3e4cb301bb` with empty real index;
- exact five-path candidate and SHA-256 set preserved before/after validation;
- Linux transport Release build PASS with `0` warnings and `0` errors;
- Core Release build PASS with `0` warnings and `0` errors;
- executed Transport suite `24/24` PASS, failed `0`, skipped `0`;
- executed Core suite `84/84` PASS, failed `0`, skipped `0`;
- temporary-index `git diff --cached --check` PASS;
- `TARGET_IO=NONE`, `FLASH_MUTATION=NONE`.

## Gate 3 — closure reconciliation

Record exact changed paths/source SHA-256/evidence SHA-256 and confirm no behavior/API contract beyond unreachable internal surface changed.

### Gate-3 result — PASS

Accepted product identity:

- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbDiscovery.cs` SHA-256 `F9596129B1274E6162561D436B573ECCADD978B78A1FAF737F79F23BA84796FD`;
- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs` SHA-256 `4F76CEE57003B3406D11A250D989906F6CB166B7A2CE712DFFB8CA95D1B0296E`.

RDC-02 removes exactly the eight frozen unreachable internal remnants and changes no runtime/bootloader profile ownership, public runtime USB constants, Core/Windows API, native behavior, wire contract, target firmware, scripts, Flash or hardware state.

## Gate 4 — local acceptance commit

Exact staged-path review, staged diff check PASS, one normal commit, clean post-commit state.

## Gate 5 — ordinary non-force publication

Fresh direct-parent proof, ordinary fast-forward push and post-push fresh fetch with `HEAD == origin/main == FETCH_HEAD`, clean worktree/index, ahead/behind `0/0`.

## Failure classes

- `HOST_DEAD_SURFACE_SCOPE_DRIFT`
- `HOST_DEAD_SURFACE_RETAINED_API_REMOVED`
- `HOST_DEAD_SURFACE_TRANSPORT_TEST_FAILURE`
- `HOST_DEAD_SURFACE_CORE_TEST_FAILURE`
- `HOST_DEAD_SURFACE_BUILD_FAILURE`
- `HOST_DEAD_SURFACE_DOC_FAILURE`
