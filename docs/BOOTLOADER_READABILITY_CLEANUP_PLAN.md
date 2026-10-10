# Deus OS — RDC-07 Bootloader Readability Cleanup Plan

Status: **GATE-0 READ-ONLY AUDIT / SCOPE FROZEN; SOURCE EDITS REQUIRE SEPARATE PUBLISHED GATE-0 ACCEPTANCE**

Boundary: `RDC-07 / BOOTLOADER_READABILITY_CLEANUP`
Paired acceptance: `docs/BOOTLOADER_READABILITY_CLEANUP_ACCEPTANCE_PLAN.md`
Project-state authority: `docs/CURRENT_STATE.md`

## 1. Observed current debt and immutable baseline

Read-only audit date: 2026-10-10. Initial clean `main == origin/main` HEAD: `4214b375ba4b1bb4d508cca236971bd81aa848e8`.

| Audited component | Exact identity / observation |
| --- | --- |
| `bootloader/bootloader.c` | blob `39bbbc0ed7bb8dbc307f57364c090e9d256dc503`; 37,177 bytes, 716 source lines |
| Density | 35 lines longer than 130 chars, 4 longer than 200, ~34 single-line static function bodies by textual heuristic |
| `bootloader/startup.s` | blob `afb877367b99c987425a698f40c2196745e400d5` |
| `linker/stm32f103c8_bootloader.ld` | blob `db69d178bcd89048ce4843efa80a147924465f25` |
| `scripts/build_bootloader.ps1` | blob `50113303ed73dedb250265cbd5f7c289f29d2c22` |

Actual readability hotspots: SHA-256/HMAC helpers (around source lines 291–360), Flash FPEC and metadata (374–458), USB PMA/EPnR (494–506), control endpoint (524–590) and update state/response loop (613–705). Dense formatting is **confirmed maintainability debt**, NOT proof of a new functional flaw.

The previously confirmed STM32F103 USB EPnR `CTR_RX/CTR_TX` rc_w0 preservation bug has already been corrected in `ep_invariant()` and committed **separately** as `83e57f609bab4bcc8a61ea2222629bf9066e910d`. That hardware-tested behavior is a nonnegotiable baseline: fixed bootloader INFO 72/72 paced/burst PASS; normal authenticated application update committed firmware revision 5; full 64-KiB Flash readback and USB runtime PASS.

## 2. Authorized scope / explicit anti-goals

**Only Gate-1 product source change:** `bootloader/bootloader.c`. No new headers/modules, helpers/functions, new interfaces or file moves. Startup, linker, build PowerShell, Host, application, protocol, tests and persistent Flash layout remain unchanged.

**Allow:** indentation, line wrapping, grouping of existing C braces/blocks **without adding or deleting significant tokens**, and verified standalone explanatory comments. Keep all significant source/preprocessor tokens, declarations, identifiers, function/global ordering, expression evaluation order, `volatile` reads/writes, inline ASM, attributes and control flow identical. No global mechanical formatter unless its exact diff passes these conditions. Do not add/decrement braces or rewrite a one-line `if`, `for` or nested ternary in a way that changes its token stream.

**Forbidden:** reorganization into components; algorithm optimization; typing/macro changes; any changes to HMAC/SHA-256, constant-time comparison, version-floor checks, reset/VTOR/MSP, interrupt masking, EPnR CTR rc_w0 fix, USB packet/descriptor bytes, polling, Flash erase/write, metadata commit marker, backup token, watchdog/clock waits, retry/timeout behavior, or code size and stack budgeting. No re-signing, no version bump, no new signed package, no device Flash write, no feature. Any discovered actual defect requires a separately justified boundary rather than slipping into RDC-07.

Frozen bootloader map: 0x08000000..0x08001FFF (pages 0–7, <=8 KiB); app `0x08002000..0x0800EFFF` (pages 8–59), metadata 60/61, persistence 62/63. USB boot identity `1209:000d`, IF0 OUT `0x01` IN `0x81`; runtime `1209:000c`. Frozen package/frames and `INFO, BEGIN, AUTHORIZE, DATA, END` per `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`. Metadata `0xA55A` is written last. Trust, HMAC authentication, antirollback floor and all Flash addresses are immutable.

Resource ceilings: Flash <=8192 B, `.data+.bss` <=1024 B, reserved bootloader MSP=1024 B, conventional SRAM <=2048 B; static/nested stack limits cannot grow. Exact **raw BIN byte equality is mandatory**, even when resource sizes fit.

## 3. Deterministic evidence, never device-key disclosure

Use exact unchanged pinned GNU ARM toolchain (15.3.1), `-Os`, existing build script/linker/startup and two **independent isolated** build trees for baseline vs candidate. Build both with the **same 32-byte explicitly synthetic test-only key** in protected disposable workspaces, never real signing material. Build inputs, compiler version, key-test identity (not key bytes), tool flags, baseline/candidate Git blobs, output paths, exit codes and section/symbol/stack measurements must be bound in evidence.

Enforce **byte-for-byte equality of complete bootloader BIN**, not merely size or hashes of source. Hash equality and bytewise comparison must both PASS; compare loadable sections, symbol addresses and normalized disassembly as corroboration. Compiler/preprocessor significant-token identity is an independent fail-closed check. Do not claim full ELF/MAP SHA equality if reproducible build paths differ; interpret ELF/MAP via normalized loadable code/symbol structures. Distinguish synthetic-key BIN from previously installed production-key firmware; the synthetic output is permanently **NON-DEPLOYABLE**. The published private device key, Flash backup and real-key BIN/ELF/MAP are not to be exported, committed or logged.

A candidate BIN mismatch is **STOP**, not an acceptable `format-only` delta. After an exact match, the already installed production bootloader remains valid: source can be published without physical reflash. Neither SWD target write nor physical USB reconnect is required or authorized by this maintainability task.

## 4. Mandatory gates

- **Gate-0:** read-only source/dependency/risk inventory, freeze this design and paired acceptance, docs-only scope, `git diff --check`, normal docs-only commit, ordinary non-force push, fresh fetch clean 0/0 and hosted CI. Gate-1 source mutation forbidden until Gate-0 publication is independently accepted.
- **Gate-1:** one-file `bootloader.c` formatting-only candidate, review exact diff and significant-token identity. No project-source edits elsewhere.
- **Gate-2:** pristine baseline vs exact candidate build with same synthetic key, real pinned GNU ARM `-Os`, strict byte-identical BIN, symbol/disassembly/section/resource/stack equivalence and absence of secrets. Both independent builds PASS. No target I/O.
- **Gate-3:** independent acceptance of protocol/security/MMIO unchanged, source/build candidate binding, regression-policy no-flash adjudication. If mismatch or uncertainty, FAIL/STOP, not reflash.
- **Gate-4:** normal single exact `bootloader/bootloader.c` acceptance commit only, clean staged/index poststate.
- **Gate-5:** normal non-force publication/fetch `HEAD == origin/main == FETCH_HEAD`, clean 0/0, hosted Host CI on published SHA. Host CI does NOT replace Gate-2 ARM equivalence. Final docs reconciliation and promotion of RDC-08 only after full acceptance.

Historical physical revision 5 remains installed. Product feature work continues blocked by RDC-08.

## 5. Explicit Gate-0 stop criteria

No Gate-1 authorization if the source/blob/head differs, baseline already dirty, undocumented sensitive state changes are found, acceptance lacks byte-identical proof, any real signing key would be required outside private operator control, docs changes include executable artifacts, or Gate-0 publication/fresh fetch/CI is not PASS.
