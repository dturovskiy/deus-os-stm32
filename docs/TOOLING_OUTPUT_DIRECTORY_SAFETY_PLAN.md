# Deus OS — Tooling Output Directory Safety Plan

Status: **CLOSED / PUBLISHED — GATES 0–4 ACCEPTED AT `7fda7b8225f05c487f6d5d7154443e8369e74571`**

Boundary ID:

`TOOLING_OUTPUT_DIRECTORY_SAFETY`

Residual-debt ID:

`RDC-04`

Audit baseline:

`e516fdc1d8818007db40ee12669d28f3f828c489`

Implementation rule:

No repository-owned destructive script may be mutated until this plan and its acceptance plan are published. The implementation baseline is the publication commit that activates RDC-04.

## 1. Purpose

Close the confirmed PowerShell tooling footgun where a caller-controlled output directory can be recursively removed before regeneration.

This is a tooling/process hardening boundary. It does not authorize firmware, protocol, Flash-layout, bootloader behavior, recovery semantics, Host behavior or target-runtime changes.

## 2. Confirmed current/historical surface

Repo-wide tracked-script audit on the baseline finds exactly three caller-controlled output-directory recursive deletes:

1. `scripts/build_firmware.ps1` — current application build tooling;
2. `scripts/create_bootloader_recovery_bundle.ps1` — current Bootloader-boundary recovery-bundle generator;
3. `scripts/create_asset_recovery_bundle.ps1` — historical Asset-phase recovery-bundle generator retained only for reproducibility.

`scripts/build_bootloader.ps1` also performs a recursive delete, but only for harness-owned/private `$SecretTemp`; it is not caller-controlled `OutputDir` and is outside the product scope unless implementation review proves the shared primitive can be adopted without weakening secret hygiene.

The historical Asset generator remains historical-only. Safety hardening must not re-authorize it for the current relocated Bootloader geometry.

## 3. Selected design

Use one repository-owned shared PowerShell primitive:

`scripts/output_directory_safety.ps1`

It owns validation **and** destructive reset as one operation. Callers must not perform their own `Remove-Item -Recurse` after a separate copied guard.

Frozen public function:

`Reset-DeusGeneratedOutputDirectory`

Required inputs:

- canonical project root;
- requested output directory;
- stable owner ID identifying the calling script.

Required behavior:

1. canonicalize paths before any mutation;
2. require the output path to be a strict descendant of an explicitly safe generated-output root or to carry a valid repository-owned ownership marker for the same owner ID;
3. reject filesystem roots, drive roots, repository root, repository ancestors, user profile/home, the temp root itself, protected repository source/control paths, and ambiguous/empty paths;
4. reject output paths or traversed existing ancestors that are reparse points/junctions/symlinks where canonical ownership cannot be proven;
5. never follow a reparse point during recursive deletion;
6. if an external existing directory is not already owned by the exact owner marker, fail closed rather than deleting it;
7. create/recreate the output directory and write the owner marker only after validation succeeds;
8. return the canonical output path to the caller;
9. emit no target I/O, USB, UART, ST-LINK, reset or Flash action.

Approved generated-output roots:

- strict descendants of `<ProjectRoot>/build`;
- strict descendants of the process temporary directory when the caller owns a unique child directory;
- an external existing directory only when its exact repository-owned marker proves the same owner ID.

The helper must not treat string-prefix matching as path containment authority. Containment must use canonical path semantics with directory-boundary awareness.

## 4. Ownership marker

The marker is tooling metadata only and must not enter firmware/recovery payload identity.

Freeze marker filename:

`.deus-generated-output.json`

Minimum fields:

- schema/version;
- owner ID;
- canonical project-root identity/path;
- explicit generated-output marker.

The helper may overwrite its own valid marker during reset. A malformed marker, wrong owner ID or marker/path mismatch is a fail-closed condition.

## 5. Authorized implementation boundary

Expected implementation paths:

- `scripts/output_directory_safety.ps1` — new shared primitive;
- `scripts/build_firmware.ps1`;
- `scripts/create_bootloader_recovery_bundle.ps1`;
- `scripts/create_asset_recovery_bundle.ps1`;
- `scripts/README.md`;
- this plan + acceptance plan;
- `docs/CURRENT_STATE.md` only for boundary state/evidence;
- closure-only governance/changelog docs after acceptance.

No target C/assembly/linker, bootloader product source, Host source or wire/protocol path is authorized.

If implementation proves a smaller path set is sufficient, reduce scope before staging. Any larger path set requires a docs-only Gate-0 amendment first.

## 6. PowerShell 7 path-safety test matrix

Before any destructive test, use disposable directories only.

Must PASS:

- nonexistent `<ProjectRoot>/build/<unique>`;
- existing owned `<ProjectRoot>/build/<unique>`;
- unique child of process temp;
- existing external disposable directory carrying exact owner marker.

Must FAIL before deletion:

- empty/whitespace path;
- filesystem/drive root;
- project root;
- any ancestor of project root;
- `<ProjectRoot>/src`, `include`, `scripts`, `.git` or another protected non-build subtree;
- user profile/home;
- process temp root itself;
- external existing directory without marker;
- wrong-owner or malformed marker;
- path escaping an approved root through `..`;
- reparse/junction/symlink output or unsafe traversed ancestor;
- path that resolves to the same canonical protected location through alternate spelling.

Failure proof must include a sentinel file in the forbidden directory that remains byte-exact after rejection.

## 7. Firmware-candidate identity consequence

Changing `scripts/build_firmware.ps1` changes the firmware candidate tree and therefore the embedded `DEUS_FIRMWARE_SOURCE_TREE_HEX` provenance literal even when compiler/linker behavior is unchanged.

Gate 2 must therefore:

- fresh-build the exact pre-RDC-04 baseline and candidate with the accepted GNU Arm toolchain;
- require identical Flash/SRAM, load/runtime sections, global symbol addresses and retained stack data;
- require candidate BIN equality after substituting only the one 40-byte source-tree provenance literal;
- treat any other load-image delta as a product/tooling regression;
- perform no target I/O when equivalence is exact.

Recovery-bundle generators must also reproduce their payload/content identities except for intentionally changed safety metadata outside immutable product payloads.

## 8. Gates

- Gate 0 — exact script inventory, design/path semantics and test matrix freeze.
- Gate 1 — implementation/static review; destructive operation centralized.
- Gate 2 — disposable PowerShell 7 safety matrix + firmware/recovery deterministic equivalence.
- Gate 3 — local acceptance commit.
- Gate 4 — ordinary non-force publication + fresh-fetch clean `0/0`.

Hardware is not required when Gate 2 proves no load-image/product payload difference outside provenance/tooling metadata.

## Accepted implementation / non-hardware proof — 2026-10-08

- Gate 1 implementation/static review — **PASS**, evidence SHA-256 `F348C1A83311A98AB6E8CCD8F68FC317F692858708BF5021190E61F6D907A7E1`. Exactly four WIP script paths: the new shared `scripts/output_directory_safety.ps1` and three existing callers. All three caller-controlled recursive resets now invoke one owner-specific fail-closed primitive; no target, Host, linker, Flash or Git publication mutation.
- Gate 2A PowerShell 7 destructive-safety matrix — **PASS 27/27**, evidence SHA-256 `3DD73736F772BD42F5534FB517FCA503F0BD768CE27C2C3A8E119ED40D811C50`. Disposable owned/unowned/generated paths, protected roots, sentinels and reparse/junction rejection validated without target I/O.
- Gate 2B firmware/recovery deterministic equivalence — **PASS**, evidence SHA-256 `F3534E3ED6510CA83A6EAE1362CF53E7A0EA5FCA79CCCCCB7B52366C72A49825`. Candidate Git tree `315d1b67194ba128b25afda8b7950eb371e998f4`; accepted RDC-03 baseline BIN `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`; candidate BIN `2BBE8760A281264AA6F359F60B43FB7C86E6F4C3CE982B24CA8098BC1FAA65E4`; normalized candidate BIN equals baseline after substituting only the 40-byte provenance field at offset `51784`. Flash/SRAM `51972/10968`, global symbols, load sections and stack usage are unchanged. Synthetic fixtures prove both Bootloader and historical Asset recovery-generator payload equivalence; historical Asset remains non-operational.
- Gate 2C docs finalization — **PASS**, evidence SHA-256 `07489AE13299106DD69DA0004975316A84E1F4ADDC75D27A34D2378F4712AC02`.
- Gate 3 exact ten-path normal local acceptance commit — **PASS** `7fda7b8225f05c487f6d5d7154443e8369e74571`, parent `100801293796a4bee3ebf24152cd57fc3924cf27`, tree `ae7760c58186ed227ce381d68562fac0a117ddd5`; evidence SHA-256 `A1BCC379A55F99B912266422BD58BF64D878D446AAE2D8345FAEFD9D379E24CB`.
- Gate 4 ordinary non-force publication and fresh-fetch proof — **PASS**; `HEAD == origin/main == FETCH_HEAD`, clean ahead/behind `0/0`; evidence SHA-256 `39A8EF0098CD313A2645316AFF0D5DD50841D080D68F98A0CE90E04D49AAFC1C`.
- RDC-04 is CLOSED/PUBLISHED. The then-next `RDC-05` CI boundary subsequently closed/published at `7f75cffdd0c632c5f99310f2c8708d745769aa42` with GitHub-hosted success run `37829541187`. At that historical publication point, `RDC-06` was the next mandatory boundary; it was subsequently CLOSED/PUBLISHED at `7bc5f27ce3447f0c907d2643e7db67669371210a` with separate bootloader fix `83e57f609bab4bcc8a61ea2222629bf9066e910d` and docs closure `a04006835bdf7194221729a4e15974613156cb30`. Current active-next status is owned only by `docs/CURRENT_STATE.md` (RDC-07 Gate-0 as of 2026-10-10).

## 9. Exit criterion

RDC-04 closes only when all three caller-controlled destructive output paths use the same fail-closed reset primitive, the forbidden-path matrix proves no recursive deletion can occur before ownership/path validation, current/historical tooling distinctions remain intact, firmware/recovery deterministic equivalence passes, and publication is clean/non-force.
