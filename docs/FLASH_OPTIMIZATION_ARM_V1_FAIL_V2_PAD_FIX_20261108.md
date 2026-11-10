# Deus OS — Windows ARM Flash optimization v1 FALSE FAIL, native artifact analysis, corrected OFFLINE v2

**Date:** 2026-11-08 Europe/Paris. **Status (LATEST):** Historical original owner Windows v1 run **FAIL** (strict outdated guard only, immutable); separate corrected v2 owner Windows native ARM **PASS**, independently checked from the *original* immutable Evidence. **SOURCE-ONLY / UNSIGNED / NOT INSTALLED / NO PHYSICAL DEVICE ACCEPTANCE.** Historical section §4 below was prepared before the v2 run and is superseded by §0 for v2's acceptance status.

## 0. ACTUAL OWNER WINDOWS v2 NATIVE RESOURCE PASS — accepted 2026-11-08 20:52 Europe/Paris

Owner executed exact canonical OFFLINE source ZIP `stm32_os_esp01_bridge_flash_optimization_arm_v2_20261108.zip` pinned to whole SHA256 `31E7E501BE45C20D0EF43CBA09027378FAC4CC6C8E9F509E275C9E5A246DF1BE` through Windows `deus-run v1.3.2-source-candidate`. Real runner result `RESULT=PASS`, `CLASSIFICATION=FLASH_OPTIMIZATION_NATIVE_ARM_BUDGET_PASS`, application script AST PASS, first original indexed Evidence preserved at `Evidence/stm32_os_esp01_bridge_flash_optimization_arm_v2_20261108_20261108_205229_86a7cd2728c74b2a9d35b756c50a46d9.evidence.zip`, **6,295 bytes**, SHA256 **`A38D0BB8983EA12403B7408F2279BF93996318A818E04469AD26F382FCEF19CA`**.

Independently examined the **original Evidence bytes, not reconstructed text or rerun** through `Tools/scripts/bench/read_flash_opt_original_windows_evidence_v1_20261108.py`, managed task **`task_1794167631009_6e65f49fe1831592` PASS**. It confirms ZIP CRC 7/7, Evidence SHA+size index for 5/5 members, original Windows outcome PASS with `failure_phase=NONE,error=NONE`, child exit 0, Windows Git status fingerprint pre=post, actual ARM `BUILD_OUTCOME=PASS`, native artifact SHA lines for os.bin/os.elf/os.map, 36 distinct `.su` file/hash lines, source-only GPIO/USART/USB reset/Flash untouched marker and correct 0-byte flash-load padding. Reviewer itself had a legacy unconditional `padding==4` test inherited from v1 and required a *local read-only reviewer correction* to honor v2's actual accepted source-bound 0/4-byte geometry; initial reviewer invocation rejected only that stale reviewer assertion, second invocation **PASS**. The Windows operator/Evidence was neither altered nor rerun during reviewer correction.

| Actual source-only ARM metric | Previous accepted ESP Bridge ARM | Optimized Windows v2 | Change |
|---|---:|---:|---:|
| `os.bin` / app Flash | 52,372 B | **51,364 B** | **−1,008 B** |
| Flash headroom to 53,248 B app limit | 876 B | **1,884 B** | +1,008 B |
| Static SRAM to 11,264 B accepted ceiling | 8,480 B | **8,480 B** | 0 B |
| Native source stack usage files | 36 | **36**, unique | no missing profiles |
| Flash load padding | 4 B | **0 B**, linked MAP consistent | accepted geometry change |

No production C module/transport protocol/USB interface/bootloader/persistence/runtime health mechanism was removed or modified during the correction between v1 and v2. The v2 native resource report is the valid **current baseline for the *source-only* ESP Bridge + optimized command service**, distinct from **installed, signed normal STM32 v8** (Flash 53,196 B application; no ESP maintenance bridge). ESP ROM bridge hardware entry remains unconditional DENY and no signed build/installation was performed. The actual next engineering work can proceed with measured 1,884 B remaining app Flash (target >=4KiB remains pending) and explicit NORMAL/BENCH diagnostic refactor/console size reduction, without repeating this exact Windows native test until a new source candidate exists. Gate4 remains NOT_AUTHORIZED.


## 1. Original failed run — immutable evidence

Owner invoked:

```powershell
deus-run stm32_os_esp01_bridge_flash_optimization_arm_v1_20261108 -ExpectedSha256 43C927EF2EC42FA0245AC3D0F37C4D3771CFFA977AA67B0DBD4B97054ABB7758
```

Runner v1.3.2-source-candidate verified bundle SHA, manifest and `apply.ps1` AST; returned `RESULT=FAIL`, `CLASSIFICATION=FLASH_OPTIMIZATION_NATIVE_ARM_BUDGET_FAIL`, `FAILURE_PHASE=FLASH_OPT_ARM_NATIVE_WINDOWS`, `ERROR=ARM_CHILD_NONZERO`, `OBSERVATIONS=NOT_ATTESTED`.

**Original, not reconstructed or retried, owner Windows Evidence**:
`Evidence/stm32_os_esp01_bridge_flash_optimization_arm_v1_20261108_20261108_203726_b549164293344c4a8c5cb92c57b7fb02.evidence.zip` — **3,931 B**, SHA256 **`CB1080F32FCB4EFC225A88942E74EC042C1B0643171386111E5066820A0618AA`**. The generic, independently prepared original Evidence reader `Tools/scripts/bench/read_flash_opt_original_windows_evidence_v1_20261108.py` original DEUS task **`task_1794166700422_2587cdd3e053099a` PASS**: CRC 7/7, internally indexed SHA+size 5/5, original Windows outcome FAIL faithfully preserved, Git pre=post, no hardware actions claimed. Native stderr identifies actual cause:

```text
scripts/build_firmware.ps1:304
unexpected Flash load padding 0 bytes; expected IRQ38 vector alignment=4
```

**All 36 C compiles, startup assembly, link, binary objcopy, size and undefined-symbol nm succeeded**. The child script falsely rejected valid zero alignment, before printing the final `BUILD_FLASH` and stack hashes. Do **not** rewrite the original Evidence as PASS.

## 2. Independent READ-ONLY examination of the actual native files left by Windows

Original owner artifact directory (already generated before failure):
`OS/build/esp010_rx_gate_arm_dda7a1dd7c7d46b08f1f78f838fff4c0/build/native/`.

Read-only actual Windows artifact inspection task **`task_1794166797185_c58d3b985bc58dc2` PASS**:

- `os.bin` **51,364 B**, SHA256 `C897D303EC4BCB4F9501C8FABAF92A5D88A82AF4DE13F76FDA75F7300A3506E1`.
- `os.elf` 551,624 B, SHA256 `4D0E9594DE9807634452EA8D490F9BD7B85D940038755EF3D90083E69FB21D4E`.
- `os.map` 292,515 B, SHA256 `205A94F9DB9560502CA93413D07C8084DDB791E5AFC94469738AC42006429CF2`.
- Original native arm-none-eabi-size stdout: `.text=51,280`, `.data=84`, `.bss=8,396`; loaded Flash section bytes=51,364; static SRAM=8,480.
- Exact MAP OUTPUT: `.isr_vector` origin **0x08002000**, bytes **0xDC (220)**; output `.text` origin **0x080020DC**, bytes **0xC774 (51,060)**; `.data` RAM origin 0x20000000, bytes **0x54 (84)**, Flash LMA **0x0800E850**. Geometry: `.text` starts immediately after 220-byte vector, gap **0**, and `.data` LMA equals output text end. Flash image ends **0x0800E8A4** (from LMA+0x54) and stays below `0x0800F000` metadata boundary.
- Physical original `os.bin` is **1,008 B smaller** than the previous independently accepted **52,372 B** source-only ESP Bridge ARM candidate. Available application Flash becomes **1,884 B** to the 53,248-B ceiling, based on the actual leftover image. **However Windows v1 operator result remains FAIL and 36 stack files/final acceptance were not attested.** Native v2 revalidation is therefore required; do NOT label original owner v1 result PASS.

## 3. Technical defect and bounded fix

The previous build script mistakenly assumed its original linker padding was *invariantly* four bytes for every `-Os` object combination. Cortex-M3 toolchain legitimately emits zero padding when `.text` section placement aligns at 4 bytes and needs no extra gap. Previously accepted other source profile had gap 4; source optimization changed code/object layout, not the memory partition.

**Changed exactly one product source file for v2:** `OS/scripts/build_firmware.ps1`, SHA256 **`B9293502BC1728EA147CB7FB995A09005F649A0CF66C9D91AD381E5C05E21B19`**. The script still uses **actual `os.bin` length** to check Flash ceiling; it now parses real `os.map`, enforces exactly one `.isr_vector/.text/.data` output section, verifies vector origin 0x08002000 and length 220, 4-byte text alignment, data LMA continuity, text+vector vs native section bytes, data byte size, computed output image byte count, and permits **only map-proven 0 or 4 Flash bytes of alignment padding**. Any gap, length, section, load-location or ceiling drift fails closed. The v2 `apply.ps1` now allows valid 0/4 padding and retains the metric, stack count 36, source ID, original indexed Evidence-on-PASS/FAIL and Git pre/post logic.

Map-geometry test directly against actual Windows `os.map` plus simulated prior valid 4-byte gap and multiple corrupt configurations: `Tools/scripts/bench/test_flash_opt_native_map_geometry_v2_20261108.py`, original task `task_1794167019195_3f9cd33d0aa5ab68` **PASS**. This host regression is NOT a Windows PowerShell runtime execution of v2.

## 4. Corrected immutable v2 package — ready for ONE owner Windows OFFLINE invocation

Canonical artifact: `Bundle/stm32_os_esp01_bridge_flash_optimization_arm_v2_20261108.zip`, **142,251 B**, SHA256 **`31E7E501BE45C20D0EF43CBA09027378FAC4CC6C8E9F509E275C9E5A246DF1BE`**.

- Base is **the exact v1 operator ZIP**, whole SHA `43C927EF2EC42FA0245AC3D0F37C4D3771CFFA977AA67B0DBD4B97054ABB7758`, preserved unchanged.
- All 75 source index entries valid. **Only `scripts/build_firmware.ps1` changed**, remaining **74/74 source files byte-identical** to v1, including all 36 C modules, linker script, startup.s, USB and other network/host-facing semantics. New synthetic indexed SHA40 `172bc8aff1cd2c66dc73b9924110aa3c875378d3`.
- New helper `payload/esp01_bridge_flash_optimization_arm_v2_build.ps1` SHA `C5BA28399E942BECF615C4BF10AB282DCCBD58DB295404A127692CE889FB4D73`; `apply.ps1` SHA `9023349E0AE9E29A53271D036F75540F9F8A377589585A61FAD0C76D3DD38D77`.
- Canonical Bundle Guard build task **`task_1794167039803_44c574029a881ff8` PASS**, independent canonical guard + direct SHA/source/semantic audit task **`task_1794167098074_859b1fe5344f6081` PASS** (ZIP CRC81/81, source SHA/size75/75, unchanged74/74, ESP ROM admission still hard-DENY, Flash/RAM guard unchanged).
- Read-only WSL proof of **the exact ZIP on Windows-mounted `D:\Projects\STM32\Bundle`** and preservation of original FAIL Evidence, task **`task_1794167141212_fd726fe06305cfa2` PASS**. This proves Windows-mounted file byte identity, not Windows runtime.

**ONE next owner command, Windows PowerShell 7, no device:**

```powershell
deus-run stm32_os_esp01_bridge_flash_optimization_arm_v2_20261108 -ExpectedSha256 31E7E501BE45C20D0EF43CBA09027378FAC4CC6C8E9F509E275C9E5A246DF1BE
```

Use the **first original Evidence and its SHA on PASS or FAIL**. Original native v2 output must attest Flash, SRAM, 36 C `.su` stack hashes and `os.bin/elf/map` hashes, read-only validate with `Tools/scripts/bench/read_flash_opt_original_windows_evidence_v1_20261108.py` (this reader now recognizes v2-specific geometry marker and delta against 52,372 Flash / 8,480 SRAM). Do not run any device commands, erase or repeat baseline hardware acceptance.

**Current state:** v1 Windows owner FAIL diagnosed; v2 static/proven source ready, **v2 Windows native NOT RUN**, no signed/install/STM32 flash/ESP boot/USB/CH340/reset/Git commit/push. Gate4 **NOT_AUTHORIZED**.
