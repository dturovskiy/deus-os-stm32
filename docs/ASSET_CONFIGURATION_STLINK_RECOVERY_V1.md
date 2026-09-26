# Deus OS — Asset / Configuration ST-LINK Recovery Contract v1

Status: **FROZEN CONTRACT — CONSUMED BY ACTIVE ASSET GATE-5 RECOVERY — TARGET RECOVERY EXECUTION STILL REQUIRES A CANDIDATE-BOUND ACCEPTANCE HARNESS**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Recovery contract:

`ASSET_CONFIGURATION_STLINK_RECOVERY_V1`

## 1. Purpose

This contract freezes the recovery bundle and STM32CubeProgrammer/ST-LINK restoration procedure required before destructive Asset/Configuration acceptance.

Recovery is operator/debug tooling outside normal product self-programming ownership.

It exists so a failed Gate-1 candidate, deliberately corrupted persistence state, interrupted fault-injection trial or unbootable application can be restored to a known-good state without guessing historical build flags or relying on stale ignored `build/` contents.

## 2. Recovery authority

The recovery firmware must be built fresh from the exact Gate-2 candidate repository state through the repository-owned firmware build entrypoint.

Recovery must never use:

- a stale ignored local BIN;
- an arbitrary downloaded firmware image;
- a historical BIN from another boundary;
- a hand-reconstructed compiler invocation;
- an image whose Git/source identity is unknown.

The exact Gate-2 candidate BIN, source tree, build-entrypoint hash and toolchain identity are recorded in the recovery manifest.

The bundle generator is intentionally repository-topology independent. Gate-2 owns candidate provenance and passes `base_head`, exact materialized `candidate_tree`, and `firmware_source_tree` explicitly to the generator. The generator validates those identities and MUST NOT call Git or require `.git` inside the clean candidate. This is required because the canonical Gate-2 candidate is materialized as a gitless content tree.

## 3. Asset-phase Flash geometry

Recovery v1 applies only to the Asset-phase layout:

```text
0x08000000..0x0800D7FF  standalone application region, 54 KiB
0x0800D800..0x0800F7FF  relocation headroom, 8 KiB, expected erased
0x0800F800..0x0800FBFF  persistence slot A, page 62
0x0800FC00..0x0800FFFF  persistence slot B, page 63
```

The application remains reset owner at `0x08000000`.

This contract must be replaced/reopened when a bootloader later owns pages 0..7.

## 4. Recovery bundle contents

The Gate-2 acceptance harness must generate one immutable recovery bundle containing at minimum:

```text
manifest.json
os.bin
application_region_54k.bin
hashes.sha256
restore-preserve-persistence.ps1
restore-clean-state.ps1
README.txt
```

Optional evidence/helper files may be added, but the required files above are normative.

The bundle is not committed as a normal repository build artifact unless a later release policy explicitly requires that. Its manifest/hash is retained in acceptance evidence.

## 5. application_region_54k.bin

`application_region_54k.bin` is exactly:

`55296 bytes`

Construction:

1. bytes `0 .. os.bin.length-1` are the exact Gate-2 `os.bin`;
2. all remaining bytes through offset `0xD7FF` are `0xFF`.

The bundle generator must fail if:

- `os.bin.length > 54272`;
- output length is not exactly 55296;
- any padding byte is not `0xFF`.

Why this artifact exists:

- recovery can prove the whole application region, not only the BIN prefix;
- page-boundary state is deterministic;
- stale bytes inside the 54-KiB application allocation cannot survive unnoticed.

## 6. Recovery manifest

`manifest.json` must record at least:

- boundary ID;
- Gate-2 candidate Git commit/tree;
- firmware source-tree identity;
- `scripts/build_firmware.ps1` SHA-256 of the exact script bytes materialized from the recorded candidate tree and used for the Gate-2 clean build; this is a candidate-tree byte identity and must not be compared to a platform-specific live working-tree checkout hash;
- exact compiler/toolchain version;
- `os.bin` size/SHA-256;
- `application_region_54k.bin` size/SHA-256;
- expected MCU DEV_ID `0x410`;
- expected Flash size `65536`;
- expected application origin `0x08000000`;
- application region length `55296`;
- headroom range `0x0800D800..0x0800F7FF`;
- persistence A/B addresses;
- STM32CubeProgrammer version used for acceptance;
- restore-script SHA-256 values.

All manifest hashes for repository-owned scripts refer to the exact bytes present in the materialized candidate that produced the bundle. Live working-tree hashes remain separate source-lock evidence and may legitimately differ because Git text normalization can canonicalize line endings when the candidate tree is written/materialized.

The manifest does not contain secrets.

## 7. Tooling baseline

Recovery uses STM32CubeProgrammer CLI over ST-LINK/SWD.

The accepted host baseline is STM32CubeProgrammer `2.23.x`; the exact installed version used for Gate-3/Gate-5 acceptance is recorded.

The recovery scripts may locate the CLI through an explicit parameter rather than hardcoding one machine path.

The STM32CubeProgrammer GUI must be closed before CLI ownership of ST-LINK is attempted.

The scripts must reuse the accepted read-only preflight logic/assumptions where applicable and fail closed on target mismatch.

## 8. Mandatory preflight before mutation

Before either recovery mode can mutate Flash, prove:

- SWD connection succeeds;
- DEV_ID == `0x410`;
- factory Flash size == `64 KiB`;
- RDP remains disabled;
- WRP for pages 0..63 remains inactive;
- target voltage lies inside the STM32F103 Flash programming range;
- current target can be read;
- CubeProgrammer GUI is not holding ST-LINK;
- recovery bundle hashes match `hashes.sha256`;
- `application_region_54k.bin` matches its manifest hash.

Any mismatch aborts before erase/program.

Recovery does not change option bytes, RDP or WRP.

## 9. Full pre-recovery backup

Before destructive restoration, read the entire 64-KiB internal Flash into:

`pre_recovery_flash.bin`

using CubeProgrammer upload/read semantics.

Required range:

```text
start = 0x08000000
size  = 65536
```

Compute and record SHA-256.

Also extract/record:

- application region bytes;
- relocation-headroom bytes;
- persistence A bytes;
- persistence B bytes.

This backup is evidence and an emergency forensic artifact. It is not automatically written back.

## 10. Recovery mode A — PRESERVE_PERSISTENCE

Purpose:

restore the exact Gate-2 firmware while preserving pages 62/63 byte-for-byte.

Procedure:

1. complete mandatory preflight;
2. capture full 64-KiB pre-recovery backup;
3. save exact 2048-byte persistence backup from `0x0800F800..0x0800FFFF`;
4. record SHA-256 of those 2048 bytes;
5. explicitly erase application + relocation-headroom pages `0..61` and **do not erase pages 62/63**;
6. program `application_region_54k.bin` at `0x08000000` using CubeProgrammer download with `--skiperase` and immediate verification;
7. read full 64-KiB Flash again;
8. prove:
   - bytes `0x0000..0xD7FF` equal `application_region_54k.bin`;
   - bytes `0xD800..0xF7FF` are all `0xFF`;
   - bytes `0xF800..0xFFFF` equal the exact saved persistence backup;
9. perform software reset/start normal application;
10. require normal boot/management health proof.

Canonical PRESERVE recovery never authorizes erase of pages 62/63. The download command is run with `--skiperase`, so persistence safety does not depend on the download command's internal erase policy.

If persistence differs by even one byte, PRESERVE recovery fails.

## 11. Recovery mode B — CLEAN_STATE

Purpose:

restore exact Gate-2 firmware and deliberately return persistence to an erased/no-record state.

Procedure:

1. complete mandatory preflight;
2. capture full 64-KiB pre-recovery backup;
3. explicitly erase pages `0..63` by enumerated page codes; do not use the CubeProgrammer mass-erase primitive;
4. program `application_region_54k.bin` at `0x08000000` using CubeProgrammer download with `--skiperase` and immediate verification;
5. read full 64-KiB Flash;
6. prove exact expected full image:
   - bytes `0x0000..0xD7FF` equal `application_region_54k.bin`;
   - bytes `0xD800..0xFFFF` are all `0xFF`;
7. reset/start application;
8. require runtime falls back to compiled `OLED_UI_LAYOUT_CONFIG_V1` default;
9. require persistence STATUS reports no committed record.

Because erase ownership is explicit and programming uses `--skiperase`, CLEAN_STATE has a deterministic erase set independent of download heuristics.

CLEAN_STATE is the canonical reset point for destructive fault-injection campaign trials.

## 12. Explicit erase lists

Mass erase is not used.

For Asset-phase STM32F103C8 page numbering:

```text
PRESERVE_PERSISTENCE erase set = pages 0..61
CLEAN_STATE erase set          = pages 0..63
```

The recovery scripts expand these ranges to explicit sector/page codes for CubeProgrammer `-e`. They do not rely on shell-specific bracket/range parsing.

PRESERVE_PERSISTENCE must prove that page codes 62 and 63 are absent from its erase invocation.

They must not use:

`-e all`

or read-unprotect/option-byte mutation commands.

## 13. Download mechanism

Recovery uses CubeProgrammer `-w/-d` with `--skiperase` and immediate `-v` verification **after** the exact erase set has already been completed successfully. In the CLI token sequence, `--skiperase` precedes `-w`, and `-v` follows immediately after `-w <file> <address>` so CubeProgrammer binds verification to that write command.

Incremental programming is forbidden for canonical recovery.

Automatic download-time erase is also forbidden for canonical recovery because erase ownership is explicitly controlled by the preceding page-erase step.

Reason:

recovery must deterministically erase exactly the authorized pages, program the required application region without a second hidden erase phase, and then prove the result through independent full readback.

The canonical recovery artifact already has fixed 54-KiB scope, so full-region deterministic programming is acceptable.

## 14. Independent final readback is authoritative

CubeProgrammer `-v` is required but not sufficient by itself.

After programming, the scripts must perform a separate 64-KiB upload/readback and compute SHA-256/byte-range comparisons.

Final recovery acceptance is based on that independent readback.

For CLEAN_STATE an expected 64-KiB image may be materialized as:

```text
application_region_54k.bin
+ 10240 bytes of 0xFF
```

and compared byte-for-byte to the final readback.

## 15. Reset/start behavior

After exact final readback proof, recovery may issue:

- CubeProgrammer software reset `-rst`, or
- start at `0x08000000`.

The chosen command is recorded in evidence.

Recovery must not rely on a bootloader.

Expected reset/application origin remains `0x08000000`.

## 16. Runtime recovery proof

After restore, require at minimum:

- target boots normally;
- management IF2 enumerates;
- HELLO negotiation succeeds;
- `sysinfo` reports the exact Gate-2 firmware source identity;
- `ping` succeeds;
- scheduler health/fault state is normal;
- IWDG is active under existing ownership;
- OLED status bar remains accepted;
- CLEAN_STATE -> compiled default console layout;
- PRESERVE_PERSISTENCE -> runtime either activates the preserved valid record or falls back according to the normal validator if preserved bytes were already invalid.

Recovery does not fabricate validity for corrupted preserved data.

## 17. Recovery and acceptance candidate identity

The recovery bundle must correspond to the exact firmware candidate under hardware acceptance.

If source changes after Gate 2:

- old recovery bundle becomes stale;
- destructive Gate 3/5 work stops;
- fresh Gate 2 build and fresh recovery bundle are required.

A bundle built for one candidate cannot be used as acceptance proof for another.

## 18. Failure handling

If recovery fails before any mutation:

- target remains unchanged;
- failure is environment/tooling/preflight.

If failure occurs after mutation:

- retain pre-recovery full-Flash backup and all CubeProgrammer logs;
- do not claim product failure automatically;
- retry only after classifying ST-LINK/tool/power/target state;
- do not enter an unbounded erase/program loop;
- CLEAN_STATE may be retried only under explicit operator control.

If SWD cannot establish a valid session, stop. Do not attempt read-unprotect.

## 19. Fault-campaign integration

Before Gate-5 deterministic persistence fault tests:

1. prove recovery bundle hashes;
2. execute one CLEAN_STATE restore rehearsal;
3. prove exact full-Flash readback and normal boot;
4. record elapsed recovery and tool logs.

During the campaign, CLEAN_STATE may re-establish trial preconditions when needed.

CLEAN_STATE erases of persistence pages 62/63 count toward the fault campaign's `<=64` **persistence-page erase-attempt** budget. PRESERVE_PERSISTENCE must contribute zero page-62/page-63 erase attempts. Erases of application/headroom pages 0..61 are recorded separately and do not consume that persistence-page counter. All recovery erase/program operations remain bounded and explicitly logged.

## 20. Required recovery evidence

Recovery evidence contains:

- recovery bundle manifest + hashes;
- exact CubeProgrammer version/path;
- preflight result;
- full pre-recovery Flash SHA-256;
- exact commands executed;
- native stdout/stderr capture;
- persistence backup SHA-256 for PRESERVE mode;
- final 64-KiB readback SHA-256;
- byte-range comparison result;
- boot/HELLO/sysinfo/ping proof;
- final mode classification;
- PASS/FAIL.

Native blank output lines must remain acceptable under the project harness/evidence playbook.

## 21. Repository script requirement

Gate 1/2 must add versioned recovery-script implementation matching this contract before destructive hardware acceptance.

The script implementation is not added by Gate 0.

It must be reviewed under the same primitive-first harness policy as other hardware tools:

1. prove CubeProgrammer discovery/version/read primitive;
2. prove 64-KiB readback primitive;
3. prove exact page-erase primitive on authorized persistence/headroom pages;
4. prove download+verify primitive;
5. only then compose the full recovery workflow.

No monolithic unproven recovery collector is accepted.

## 22. Security boundary

This is local physical/debug recovery tooling.

It assumes trusted operator possession of ST-LINK/SWD.

It is not a remote recovery/update mechanism and does not define firmware authenticity for field updates.

Executable authenticity remains a future Bootloader/Firmware-Update requirement.

## 23. Gate-0 exit criterion

This contract closes the final design prerequisite only when the canonical Asset plan/acceptance records are synchronized to it.

Gate 0 completion still requires a full cross-contract closure audit proving that:

- transfer ABI;
- persistence format;
- resource budget;
- Flash-operation policy;
- fault matrix;
- recovery procedure;

are mutually consistent.

The 2026-09-22 cross-contract closure audit recorded in the canonical acceptance plan satisfies this exit criterion and authorizes the bounded Gate-1 source boundary.
