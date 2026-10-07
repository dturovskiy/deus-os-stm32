# Deus OS — Tooling Output Directory Safety Acceptance Plan

Status: **ACTIVE — GATE 0 SCOPE/DESIGN FREEZE**

Canonical design:

`docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md`

Residual-debt item:

`RDC-04`

Audit baseline:

`e516fdc1d8818007db40ee12669d28f3f828c489`

## Gate 0 — scope/design freeze

PASS requires:

- repo-wide tracked PowerShell audit identifies exactly the caller-controlled recursive-delete surface;
- current versus historical recovery tooling is classified explicitly;
- one shared reset primitive is selected before implementation;
- canonical protected/allowed path semantics and ownership-marker semantics are frozen;
- exact initial implementation path set is frozen;
- PowerShell 7 allowed/forbidden test matrix is frozen;
- firmware candidate-identity consequences of changing `build_firmware.ps1` are frozen;
- no script/source/target mutation occurs in Gate 0.

### Gate-0 result — PASS / ACTIVATED

Baseline audit confirms caller-controlled `OutputDir` recursive deletion in exactly:

- `scripts/build_firmware.ps1`;
- `scripts/create_bootloader_recovery_bundle.ps1`;
- historical-only `scripts/create_asset_recovery_bundle.ps1`.

The only other tracked recursive delete is `build_bootloader.ps1` private `$SecretTemp`, which is not caller-controlled and is not part of the confirmed debt surface.

Selected implementation is one `scripts/output_directory_safety.ps1` primitive owning validation + reset. No product/source/tooling script mutation is authorized by this activation commit.

## Gate 1 — implementation/static review

Require:

- only the frozen implementation/documentation paths change;
- all three caller-controlled direct `Remove-Item -Recurse` output resets are removed from call sites;
- recursive output deletion exists only inside the shared primitive;
- all callers use distinct stable owner IDs;
- historical Asset generator remains explicitly historical-only;
- no target/Host/bootloader product source changes;
- `git diff --check` PASS;
- PowerShell AST parse PASS for every changed `.ps1`.

## Gate 2 — deterministic validation

### A. Destructive-safety matrix

Run under PowerShell 7 using disposable directories only.

For every forbidden case:

- helper returns nonzero/throws before deletion;
- sentinel file SHA-256 remains exact;
- no sibling/parent content changes.

For every allowed case:

- only the exact owned output directory is reset;
- sibling and parent sentinels remain exact;
- ownership marker is correct after recreation.

Reparse/junction/symlink cases must prove no traversal/deletion of the target location.

### B. Script integration

Run each affected script in a disposable accepted output path and prove normal output generation still succeeds.

For recovery generators, prove the generated immutable product payloads/bundles remain semantically and byte-wise equivalent where the safety change does not intentionally alter packaging metadata.

### C. Firmware build equivalence

Because `scripts/build_firmware.ps1` participates in firmware candidate identity:

- accepted GNU Arm toolchain exact;
- baseline build reproduces the current accepted application behavior/resource identity;
- candidate Flash/SRAM and runtime section geometry exact;
- global defined-symbol address inventory exact;
- stack/resource ceilings unchanged;
- one embedded 40-byte candidate source-tree provenance literal is the only allowed BIN delta;
- whole-BIN equality after provenance substitution;
- target I/O NONE / Flash mutation NONE.

Any non-provenance load-image delta blocks acceptance and requires investigation before commit.

## Gate 3 — local acceptance commit

Require exact staged name/status, `git diff --cached --check`, one normal commit and clean local poststate.

## Gate 4 — ordinary non-force publication

Require fresh fetch/direct-parent proof, ordinary fast-forward push, then fresh fetch proving:

- `HEAD == origin/main`;
- clean worktree/index;
- ahead/behind `0/0`.

## Failure classes

- `TOOLING_OUTPUT_SAFETY_SCOPE_DRIFT`
- `TOOLING_OUTPUT_SAFETY_VALIDATOR_BYPASS`
- `TOOLING_OUTPUT_SAFETY_PROTECTED_PATH_ACCEPTED`
- `TOOLING_OUTPUT_SAFETY_REPARSE_TRAVERSAL`
- `TOOLING_OUTPUT_SAFETY_SENTINEL_MUTATED`
- `TOOLING_OUTPUT_SAFETY_OWNER_MISMATCH_ACCEPTED`
- `TOOLING_OUTPUT_SAFETY_SCRIPT_REGRESSION`
- `TOOLING_OUTPUT_SAFETY_FIRMWARE_EQUIVALENCE_FAILURE`
- `TOOLING_OUTPUT_SAFETY_RECOVERY_EQUIVALENCE_FAILURE`
- `TOOLING_OUTPUT_SAFETY_DOC_FAILURE`
