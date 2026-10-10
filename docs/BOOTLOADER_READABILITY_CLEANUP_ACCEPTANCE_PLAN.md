# Deus OS — RDC-07 Bootloader Readability Cleanup Acceptance Plan

Status: **RDC-07 GATES 0–5 ACCEPTED / CODE PUBLISHED `7846fa48429d00233116438821acc0ea2a0b38be`; DOCS RECONCILIATION RECORDED BELOW**

Canonical design: `docs/BOOTLOADER_READABILITY_CLEANUP_PLAN.md`
Source-of-truth: `docs/CURRENT_STATE.md`

## 1. Gate-0 measured source baseline (read-only)

- [x] Initial clean `main == origin/main` at `4214b375ba4b1bb4d508cca236971bd81aa848e8`; no source/target/Flash modification during audit.
- [x] `bootloader/bootloader.c` Git blob `39bbbc0ed7bb8dbc307f57364c090e9d256dc503`; 37,177 bytes and 716 lines; 35 >130-column lines; 4 >200-column; ~34 one-line static function bodies (heuristic).
- [x] Immutable dependency blobs: startup `afb877367b99c987425a698f40c2196745e400d5`, linker `db69d178bcd89048ce4843efa80a147924465f25`, build script `50113303ed73dedb250265cbd5f7c289f29d2c22`.
- [x] Source risk classified by region: constants/descriptors 1–289; SHA/HMAC 291–360; metadata/Flash/reset 374–479; clock/USB/EPnR 481–611; firmware update 613–692; USB poll/boot 694–715.
- [x] Correct `ep_invariant()` STM32F103 CTR rc_w0 source repair present. Previous real fixed-bootloader INFO 72/72 and signed application revision5/full Flash readback are historical hardware PASS, **not new Gate-0 device testing**.
- [x] Contract excludes real key, protocol/state/Flash layout changes, production firmware re-sign/reflash, and USB resets. Gate-0 target I/O=NONE.
- [x] Gate-2 requires independent same-synthetic-key baseline/candidate build with exact raw BIN byte equality and unchanged resource/stack conditions; no ARM comparison run or result is claimed at Gate-0. The Windows compiler availability/version and pristine separate build outputs were **pending at Gate-0**, and were subsequently **independently measured and accepted by real Windows Gate-2 evidence**. DEUS MCP access through WSL was never treated as Windows compiler proof.
- [x] Gate-0 exact docs-only **two-path commit** `7f5d14890d4909749f4d61009d3d6ab01fb6bd78` (parent `4214b375ba4b1bb4d508cca236971bd81aa848e8`), `git diff --cached --check` PASS. Normal non-force push `4214b37..7f5d148 main->main`, fresh fetch `HEAD == origin/main == FETCH_HEAD == 7f5d14890d4909749f4d61009d3d6ab01fb6bd78`, clean 0/0. Hosted [GitHub Actions run 38062645242](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38062645242) on the exact SHA COMPLETED/SUCCESS: Core 84/84, Transport 24/24, failed 0, skipped 0, Release warnings 0/errors 0. **Gate-0 ACCEPTED/PUBLISHED; Gate-1 formatting-only authorized, not yet executed.**

## 2. Required acceptance by future gate

| Gate | Required test / evidence | Rejection criterion |
| --- | --- | --- |
| 1 | `bootloader.c` is the ONLY changed product source; independently compare diff and significant preprocessed token sequence | changed C semantics, identifiers, constants, ordering, MMIO or second file |
| 2 | Windows preflight of actual `arm-none-eabi-gcc.exe`/`objcopy`/`size`/`nm` paths and GCC 15.3.1 identity; independently materialized pristine baseline and candidate trees, distinct freshly created **initially empty** output directories; verify zero stale `.o/.su/.elf/.bin/.map`, unchanged linker/startup/flags, same non-deployable synthetic 32-byte key; both builds PASS; raw BIN bytes and SHA **identical** | toolchain absent or wrong, mixed/dirty outputs, cross-build cache, BIN/disassembly/section/symbol mismatch or dependency drift |
| 2 | resource and stack evidence: Flash <=8192, `.data+.bss` <=1024, MSP=1024, conventional SRAM <=2048, unchanged frames/call graph; 0 unknown symbols | threshold/regression/stack unknown |
| 3 | source tree and synthetic BIN proof bound, all USB protocol/HMAC/Flash/metadata and rc_w0 invariants unchanged; target unchanged | any behavior uncertainty; **no flash to compensate** |
| 4 | one normal local commit with exactly one approved changed source path; staged `--check`, no WIP/secret | scope breach |
| 5 | non-force push, independent fresh HEAD/origin/FETCH_HEAD equality, clean 0/0 and exact-SHA GitHub Host Core>=84/Transport>=24, 0 failed/skipped | publication/CI mismatch; Host CI cannot replace ARM proof |

The required Gate-0 docs-only publication **has passed** (`7f5d148` and follow-up acceptance record `d824716`, both published with successful exact-SHA CI). **Historical Gate-0 authorization (subsequently satisfied):** Gate-1 was restricted to formatting-only `bootloader/bootloader.c` and is now accepted in exact code commit `7846fa48429d00233116438821acc0ea2a0b38be`, not an autonomous refactor. No key export, firmware deployment, target I/O or Flash mutation is authorized. Gate-2 subsequently independently proved the genuine Windows toolchain/version and clean isolated build inputs; DEUS MCP's WSL execution was not used as a substitute for Windows compiler evidence.

## 3. Security/ownership exclusions

Do not include private signing key, real device Flash dump, key-bearing BIN/ELF/MAP, production firmware package, staged OS artifacts, or source-control secrets in evidence. A zero or synthetic key can be used **only for deterministic non-deployable equivalence builds**; never flash a dummy-key result. The installed revision-5 signed application, authenticated metadata floor and bootloader remain unchanged throughout RDC-07.


## 4. Final execution/evidence adjudication — 2026-10-10

- [x] **Gate-1 exact source:** only `bootloader/bootloader.c`, baseline blob `39bbbc0ed7bb8dbc307f57364c090e9d256dc503` -> candidate blob `ed34ad98f6dcf48c5175c3b53a82d8144e653bee`. 8267 exact significant tokens, all 174 preprocessor directives, 108 quoted literals and both comments unchanged; 716 -> 1244 lines, no changed C behavior/ordering.
- [x] **Gate-2 real Windows:** original operator evidence `stm32_os_rdc07_gate2_arm_bin_equivalence_v1_20261010_162149_aaa9c73101a6.evidence.zip` SHA256 `9208C02ACCC956188E32CBAD68740AC46F0EB5E486C9849C7372B898366EB768`, self-contained CRC/3 hashes PASS and 38/38 identical logged facts. Windows GCC15.3.1/objcopy/size/nm/objdump proof, independently materialized pristine source and initially absent output dirs, same random synthetic NONDEPLOYABLE key, two successful ARM builds, raw BIN `5996` bytes bytewise equal with identical SHA-256 `7EDCD55CED5DDA529EFF173CF1ABB676E7892B1A5824B6EE4695A7BA3EC796BE`; nm/symbol and normalized disassembly exact, 32 stack frames exact, Flash5996, BSS660, MSP1024, SRAM1684. Real firmware key and hardware never accessed. Raw synthetic BIN, signing key, Flash dumps remain excluded from public Git/evidence.
- [x] **Gate-3 no-flash acceptance:** exact candidate/dependencies and clean one-file Git WIP prestate independently checked, unchanged USB/CTR/HMAC/Flash/metadata/handoff invariants; no new physical runtime test necessary for byte-exact formatting-only artifact. Production STM32 retained previously proven bootloader and signed application revision 5. No production BIN SHA equality implied.
- [x] **Gate-4 exact source commit:** `7846fa48429d00233116438821acc0ea2a0b38be`, parent `10a930cdb75cfc7ba2ac41dbc746109e55080267`; ONLY `bootloader/bootloader.c` staged/committed, staged blob `ed34ad98f6dcf48c5175c3b53a82d8144e653bee`, whitespace check PASS and unstaged/index clean.
- [x] **Gate-5 publication:** normal non-force fast-forward `10a930c..7846fa4`, fresh `HEAD == origin/main == FETCH_HEAD == 7846fa48429d00233116438821acc0ea2a0b38be`, worktree/index clean 0/0. GitHub-hosted CI [run 38067711160](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38067711160) exact SHA success: Release 0 warnings / 0 errors, Core `84/84`, Transport `24/24`, 0 failed/skipped.
- [ ] **RDC-08 — historical RDC-07 close-out pointer, not an unfinished RDC-07 gate:** Gate-0 design was still required when this RDC-07 record was first completed. It was subsequently accepted/published at `86abbc59fd0f26d3a5ec5f279b3e0c35d16f62c2`; Gate-1 classification followed at `c646f3e3ac342cbaef025d115e5be96bd49cf140`. RDC-08 itself remains open until all its later gates pass; current gate ownership is in `docs/CURRENT_STATE.md`. RDC-07 does not authorize key sanitization, firmware update or new product features.

**Disposition: RDC-07 IMPLEMENTATION/ACCEPTANCE/PUBLICATION PASS.** The above original Gate-0 checkboxes are preserved as historical Gate-0 facts, not outstanding requirements. This docs-only reconciliation publishes the completed status to the canonical plan, current state, roadmap, checklist and changelog without altering the accepted C blob.
