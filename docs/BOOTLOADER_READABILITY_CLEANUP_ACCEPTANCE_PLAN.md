# Deus OS — RDC-07 Bootloader Readability Cleanup Acceptance Plan

Status: **GATE-0 AUDIT FACTS VERIFIED; DOCS-ONLY PUBLICATION REQUIRED BEFORE GATE-1**

Canonical design: `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md`
Source-of-truth: `docs/CURRENT_STATE.md`

## 1. Gate-0 measured source baseline (read-only)

- [x] Initial clean `main == origin/main` at `4214b375ba4b1bb4d508cca236971bd81aa848e8`; no source/target/Flash modification during audit.
- [x] `bootloader/bootloader.c` Git blob `39bbbc0ed7bb8dbc307f57364c090e9d256dc503`; 37,177 bytes and 716 lines; 35 >130-column lines; 4 >200-column; ~34 one-line static function bodies (heuristic).
- [x] Immutable dependency blobs: startup `afb877367b99c987425a698f40c2196745e400d5`, linker `db69d178bcd89048ce4843efa80a147924465f25`, build script `50113303ed73dedb250265cbd5f7c289f29d2c22`.
- [x] Source risk classified by region: constants/descriptors 1–289; SHA/HMAC 291–360; metadata/Flash/reset 374–479; clock/USB/EPnR 481–611; firmware update 613–692; USB poll/boot 694–715.
- [x] Correct `ep_invariant()` STM32F103 CTR rc_w0 source repair present. Previous real fixed-bootloader INFO 72/72 and signed application revision5/full Flash readback are historical hardware PASS, **not new Gate-0 device testing**.
- [x] Contract excludes real key, protocol/state/Flash layout changes, production firmware re-sign/reflash, and USB resets. Gate-0 target I/O=NONE.
- [x] Gate-2 requires independent same-synthetic-key baseline/candidate build with exact raw BIN byte equality and unchanged resource/stack conditions; no ARM comparison run or result is claimed at Gate-0.
- [x] Gate-0 exact docs-only **two-path commit** `7f5d14890d4909749f4d61009d3d6ab01fb6bd78` (parent `4214b375ba4b1bb4d508cca236971bd81aa848e8`), `git diff --cached --check` PASS. Normal non-force push `4214b37..7f5d148 main->main`, fresh fetch `HEAD == origin/main == FETCH_HEAD == 7f5d14890d4909749f4d61009d3d6ab01fb6bd78`, clean 0/0. Hosted [GitHub Actions run 38062645242](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38062645242) on the exact SHA COMPLETED/SUCCESS: Core 84/84, Transport 24/24, failed 0, skipped 0, Release warnings 0/errors 0. **Gate-0 ACCEPTED/PUBLISHED; Gate-1 formatting-only authorized, not yet executed.**

## 2. Required acceptance by future gate

| Gate | Required test / evidence | Rejection criterion |
| --- | --- | --- |
| 1 | `bootloader.c` is the ONLY changed product source; independently compare diff and significant preprocessed token sequence | changed C semantics, identifiers, constants, ordering, MMIO or second file |
| 2 | pristine baseline and candidate separately built with exact pinned ARM GNU toolchain and same disposable 32-byte synthetic key; both build PASS; raw BIN bytes and SHA **identical** | any BIN/meaningful disassembly/section/symbol mismatch or dependency/toolchain drift |
| 2 | resource and stack evidence: Flash <=8192, `.data+.bss` <=1024, MSP=1024, conventional SRAM <=2048, unchanged frames/call graph; 0 unknown symbols | threshold/regression/stack unknown |
| 3 | source tree and synthetic BIN proof bound, all USB protocol/HMAC/Flash/metadata and rc_w0 invariants unchanged; target unchanged | any behavior uncertainty; **no flash to compensate** |
| 4 | one normal local commit with exactly one approved changed source path; staged `--check`, no WIP/secret | scope breach |
| 5 | non-force push, independent fresh HEAD/origin/FETCH_HEAD equality, clean 0/0 and exact-SHA GitHub Host Core>=84/Transport>=24, 0 failed/skipped | publication/CI mismatch; Host CI cannot replace ARM proof |

No source change, key export, firmware build or target I/O is authorized by this Gate-0 contract **until docs-only Gate-0 publication passes**. Next product-source action after accepted publication is a carefully bounded formatting-only Gate-1, not an autonomous whole-project refactor.

## 3. Security/ownership exclusions

Do not include private signing key, real device Flash dump, key-bearing BIN/ELF/MAP, production firmware package, staged OS artifacts, or source-control secrets in evidence. A zero or synthetic key can be used **only for deterministic non-deployable equivalence builds**; never flash a dummy-key result. The installed revision-5 signed application, authenticated metadata floor and bootloader remain unchanged throughout RDC-07.
