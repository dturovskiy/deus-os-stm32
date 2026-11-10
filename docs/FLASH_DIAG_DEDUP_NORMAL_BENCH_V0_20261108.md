# Deus OS — full-feature diagnostic size refactor and NORMAL/BENCH contract inventory
**Date:** 2026-11-08. **Status of this scoped v3 optimization: Windows native ARM resource PASS, original Evidence accepted; now superseded as latest *size* baseline by v4 (49,020 B Flash, 4,228 B spare, target 4KiB achieved).** The original v3 evidence and all comparisons below remain immutable historical proof. Latest Windows v4 original Evidence SHA `FFC0F6AC1F080E2831C1CC77013154FA7CB665E804055D9BFA4D9F29094F286B` independently verified PASS task `task_1794172201827_dade7f8bc594b090`; see `docs/FLASH_OPTIMIZATION_SOLID_ASM_V4_20261108.md` §0. **Hardware NOT TOUCHED; unsigned, not installed.**

## 0. Owner Windows v3 native ARM result — original independent Evidence PASS

Owner ran exact canonical `deus-run stm32_os_esp01_bridge_hexline_dedup_arm_v3_20261108 -ExpectedSha256 0D23408136AF24BAA910A3B58EF8FC3324AFB42C11DC3B9B398168A28D0F1D2A` via Windows `deus-run 1.3.2-source-candidate`, which verified ZIP SHA, manifest and apply.ps1 AST. Original Windows result `PASS`, classification `FLASH_OPTIMIZATION_NATIVE_ARM_BUDGET_PASS`, no hardware touched.

- **Immutable original owner Windows Evidence:** `Evidence/stm32_os_esp01_bridge_hexline_dedup_arm_v3_20261108_20261108_213138_f8c380b48d9f4cc582621492ca8669a3.evidence.zip` **6,367 bytes**, SHA256 **`829678FFBE1F561D27A7A6479A877192C112D05C066DBCBC75AF8A150E37DDB0`**. No Evidence reconstruction.
- **Independent original read-only Evidence review:** `Tools/scripts/bench/read_hexline_v3_original_windows_evidence_20261108.py`, task **`task_1794170019436_2b10a9acd452f27b` PASS**. ZIP CRC 7/7, original SHA+size 5/5, original outcome PASS/error NONE/child_exit0, Git fingerprint pre=post, exact v3 source helper ID and policy, original `os.bin/os.elf/os.map` hash records, 36 distinct compiler stack-usage file hash records, Flash geometry padding0, no physical access asserted.
- Actual native ARM Flash `BUILD_FLASH=49,692 B` out of application ceiling `53,248 B`; **3,556 B** spare. Actual previous v2 baseline **51,364 B**, so **−1,672 B** due to the shared-format-code refactor. Initial unsigned ESP Bridge original baseline **52,372 B**, so full combined source-only optimization **−2,680 B**. **4 KiB headroom target not yet met**: `4,096 − 3,556 = 540 B` still needed.
- Static SRAM **8,480 B**, **unchanged** from v2; accepted linker static ceiling **11,264 B**. **36** native stack usage files unchanged in count; counts do not independently prove runtime max stack margins.
- New source preserves 39 command definitions, full scheduler diagnostic code, binary RPC, USB node transports, BOOT/EN hard-DENY, startup and linker ownership. Host actual-source regressions below PASS but **Windows ARM resource PASS is not hardware runtime/OLED/USB/IRQ safety qualification**. No install/sign/Flash/chip resets/Git commit/push or Gate4 permission occurred. Installed normal signed STM32 v8 and ESP v2 remain unchanged.
- **Next:** preserve this v3 immutable source+original Evidence as **latest ARM resource baseline**; check future **>=540 B** gain with a separate source-locked OFFLINE ARM A/B only after one bounded non-feature-cut candidate exists. Later real hardware runtime/USB/NodeLink/OLED regression requires separate explicit owner permission and proper signed/recovery contract, not an unsigned ad hoc device flash.



## Precise answer: do tests reside in STM32 memory?
- Repository `OS/tests/optimization/*.c`, `host/tests/*`, `Tools/scripts/bench/*`, .su/ELF/MAP debug and original Evidence live on **developer hosts**, not stored in installed MCU firmware.
- Compiled C implementation of **live, callable active self-tests**, test-status strings and any referenced test buffers occupy Flash and potentially static SRAM. `-ffunction-sections -fdata-sections -Wl,--gc-sections` already drops genuinely *unreferenced* functions/data, so deleting such dead source gives **zero image savings**.
- Real **runtime protections are not optional development tests**: preserve watchdog reload/reset diagnostics, stack canaries, MSP/PSP guards, HardFault capture, scheduler priority/event correctness, bootloader recovery, USB Management framing, NodeLink/CRC, flash integrity and authenticated update.
- The 36 compiler `.su` files attest that every C unit emitted compiler stack-use metadata **on Windows**, not that 36 separate runtime stacks or test suites live in MCU SRAM. The v2 static SRAM measure is 8,480B against linker acceptance 11,264B; physical STM32 RAM also reserves its real stacks and other regions.

## Executed now — no test removal; deterministic source-only A/B candidate

- `src/kernel.c`: 64 report patterns replaced with shared `command_service_write_hex_line(context,label,value)`; source SHA256 `E09806607BC1ED9050934D6CF27B8F6D12143485C0C0FBB8F86F110A7F9569AF`.
- `src/kernel/scheduler_diagnostics.c`: 54 patterns replaced; removed only **two dead wrapper functions** now unused, not any diagnostic method; SHA256 `863621FC38053ACCF0D1CFE390B0DDDAF6B129AFA9F0D3EAAA5B9CDACA699EAD`.
- `src/kernel/command_service.c`: one shared three-call primitive (writes label, 32-bit uppercase hex, CRLF), **all three operations attempted even after an earlier write failure**, matching original ignoring-return semantics; SHA256 `32C03D55EE728D3E87D0496FEE5713657A675BECC4B228845D4D5D81353C8BA9`.
- `include/kernel/command_service.h`: one additive function declaration (no method/struct/enum/protocol/wire changes), SHA256 `01C85F72E7CFE8646B07C3220D40D1B4850CFA7146E51DAC893EFDAC1254F33C`.
- Full 39-command registry, names, rpc IDs, `SAFE/DIAGNOSTIC/DESTRUCTIVE` class, arguments, scheduler 8-test dispatch, asset/firmware update, USB and memory map all identical to accepted ARM v2. The startup assembly, compiler flags, `--gc-sections` and strict 36 production C list were **not changed**.
- Host GCC `-Os` original vs new per-object `.text + .rodata`: `scheduler_diagnostics.c` 8,854→7,606 (−1,248 B), `command_service.c` 1,582→1,627 (+45 B), **sum −1,203 host object bytes**, plus `kernel.c` not measurable in host x86 object due Cortex-M3 architecture / pointer width. **No native STM32 Flash savings should be claimed until one Windows v3 arm build**.

## Source-linked OFFLINE regression results (no hardware)

- Original accepted-v2 ZIP (whole SHA `31E7E501BE45C20D0EF43CBA09027378FAC4CC6C8E9F509E275C9E5A246DF1BE`) is immutable read-only oracle for report labels/value expressions. Exact source-order + expressions 118/118 MATCH; 117 unique labels. `Tools/scripts/bench/test_lossless_hexline_host_actual_source_v0_20261108.py`, task `task_1794168768631_74597d68db87cc32`: actual `command_service.c` with 22,740 sink-error/32-bit scenarios per GCC `-O0/-O2/-Os/UBSan` 4/4 PASS.
- `Tools/scripts/bench/test_scheduler_diag_wire_equivalence_v0_20261108.py`, task `task_1794169062136_e265945af72cf609`: actual `scheduler_diagnostics.c` + `command_service.c` old-v2 vs new, controlled scheduler mocks; `schedprio`, `schedtimed` plus sink error bytes exactly **625/625** at O0/Os/UBSan (3/3 PASS). This is a **mock scheduler** proof, not IRQ timing/physical concurrency.
- `Tools/scripts/bench/run_command_service_lookup_v0_20261108.py`, task `task_1794168803581_57365a50d9edc544`: all 39 entries / **all 65,536 uint16 RPC IDs**, parse/help/denials O0/O2/Os/UBSan 4/4 PASS.
- `Tools/scripts/bench/run_binary_rpc_source_v0_20261108.py`, task `task_1794168807373_a0c2ea587fdf9493`: actual C HELLO/frame/CRC, chunks, malformed, unauthorized destructive deny, TX failure O0/O2/Os/UBSan 4/4 PASS.
- NodeLink/ESP offline `task_1794168811472_f0850bf1cf9e6131` (3/3); Management USB dispatch hard-DENY `task_1794168822850_023897719abf4c05` (3/3); strict C source whitelist **36/36** `task_1794168816287_695b783318c4fbf2` PASS.
- Original v2 operator/Evidence untouched. New v3 OFFLINE ZIP was authored only after these checks, canonical BundleGuard build+validate task `task_1794168933385_825d72bdeef86f86` PASS. Independently validated ZIP81/81 CRC, source index SHA/size75/75, byte-identical unchanged source 71/71, exact 4 changed members, unchanged memory map, preserved full 39-command registry and ESP ROM hard DENY, WSL Windows-mount `D:\Projects\STM32\Bundle` exact ZIP SHA task `task_1794168977324_532378b325f852f2` PASS.

**Evidence audit readiness:** `Tools/scripts/bench/read_hexline_v3_original_windows_evidence_20261108.py` SHA256 `3A4095245CE5882754110C6EA67DDA4011D00CB646AE71D513437700BE13C3C6` is a separate read-only v3-aware auditor. It was regression-tested against the **actual original accepted v2 Windows Evidence** task `task_1794169202038_8e08d0d9ef559eee` PASS, proving existing original ZIP/CRC/member index/36 stack detection remains recognized. Once the v3 owner `deus-run` creates first original PASS/FAIL Evidence, invoke the same auditor on that exact original basename and whole SHA; compare `FLASH_DELTA_VS_ACCEPTED_OPT_V2` and source-specified helper SHA, never silently recalculate/replace an original FAIL. This does not assert new Windows v3 acceptance.

## Historical pre-acceptance Windows OFFLINE ARM command (already executed, PASS; do not rerun unchanged v3)

Package: `Bundle/stm32_os_esp01_bridge_hexline_dedup_arm_v3_20261108.zip` 142,225 B; SHA256 **`0D23408136AF24BAA910A3B58EF8FC3324AFB42C11DC3B9B398168A28D0F1D2A`**; synthetic source SHA index 40 `c226095ba7a4693c95d2419ad25d99e2559a6831`. Exactly 4/75 source changes from accepted ARM v2, other 71 unchanged.

Owner Windows PowerShell accepted `deus-run` command (source-only, no board):

```powershell
deus-run stm32_os_esp01_bridge_hexline_dedup_arm_v3_20261108 -ExpectedSha256 0D23408136AF24BAA910A3B58EF8FC3324AFB42C11DC3B9B398168A28D0F1D2A
```

v3 reports actual `BUILD_FLASH` / `BUILD_SRAM`, load padding verified against MAP, 36 unique C stack-use SHA, `os.bin/elf/map` hashes, exact-source bundle ID, `FLASH_DELTA_VS_ACCEPTED_OPT_V2=(BUILD_FLASH-51364)` and `SRAM_DELTA` from v2 8480. **Result PASS proves resource fit, not size savings or physical correctness**. Actual latest accepted source-only ARM baseline remains **51,364/53,248B Flash**, 8,480/11,264B static SRAM, 1,884B spare until original v3 Evidence is accepted.

## NORMAL / BENCH split — classification and no-silent-feature-loss contract

**Do NOT remove or conditionalize diagnostics in current v3:** user prioritizes no lost functionality. Establish a clear versioned profile/capability contract *before* disabling any diagnostic in future NORMAL.

1. **Always NORMAL:** system boot, startup/reset capture, scheduler itself, task priorities/events/timers, watchdog & stack health, exception/fault/crash state, USB Management+Binary RPC, authentication/integrity/flash update/persistence/recovery, NodeLink HELLO/CRC, `nodestart` and `nodecrc` despite their DIAGNOSTIC command class (these are real node-management functions), application commands, OLED normal runtime, `health`, `mspstat`, `schedprod`, `sysinfo`, `nodestat`, `fault`, desktop and all run-time invariants.
2. **BENCH-only *candidates* only after user accepts changed diagnostics availability:** eight active scheduler tests `schedtest`, `schedcoop`, `schedpreempt`, `schedstack`, `schedworkload`, `schedconsoleprobe`, `schedwaitwake`, `schedisolate`. Two *SAFE*-class commands `schedtimed` and `schedprio` actually execute diagnostic scenarios too; cannot remove on class name alone. `oledtest`, `oledrender` and renderer/layout self-tests are interactive UI validation and must be assessed per specific caller; simple `oledping` and normal status rendering should remain. Explicit owner approval needed for moving any installed command to BENCH; no hidden downgrade.
3. **Special destructive test:** `wdogtrip` stays in existing DESTRUCTIVE protocol class with authorization/confirmation semantics intact. Do not use to save Flash by silently removing reset/Watchdog recovery or exposing unsafe new paths.
4. Future profile may preserve 39 method IDs, name lookups and command argument contracts while advertising a **versioned capability bit** and returning an explicit, version-compatible "unavailable in NORMAL" status rather than disappearing or claiming success. Requires host CLI + Binary RPC + USB + firmware compatibility/negative tests and an **A/B ARM image** proving exact source/feature deltas. Never claim NORMAL/BENCH images feature-identical once tests are excluded.
5. Keep all C#/.NET/Python/host mock tests outside the 36 production compilation sources and available to CI; they do not cost target Flash when not linked.

## Limits / next acceptance

**Remaining unmet proof:** owner Windows v3 actual ARM resource result and later appropriate runtime/USB/NodeLink/OLED/hardware behavior tests (never auto-flash). No signing, programming, board reset, cable/power change, Git commit/push or Gate4 authorization was performed. After v3 original Evidence is independently reviewed, compare actual image with **51,364B v2**, decide whether formatting-only optimization is retained. If ARM saving is insignificant or negative, revert only this bounded refactor from accepted v2 bytes, then evaluate BENCH-only capability contract separately.
