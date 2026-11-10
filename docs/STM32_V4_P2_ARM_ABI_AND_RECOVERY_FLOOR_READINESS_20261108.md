# Deus OS — v8 → optimized v4 cross-version ABI and protected recovery readiness (P2 offline)

**Date:** 2026-11-08 Europe/Paris. **Scope:** source-locked, actual original Windows ARM ELF and frozen *public* bootloader version/header checks. **Result:** `P2_ROM_COMMAND_ABI=PASS`; `P2_PUBLIC_UNSIGNED_IMAGE_FORMAT=PASS`; `P2_SIGNED_NORMAL=NOT_AUTHORIZED`; `P2_RECOVERY_COMPATIBILITY=BLOCKED/NOT_ATTESTED`; `P3_FLASH=NOT_AUTHORIZED`; `V4_PHYSICAL=NOT_ATTESTED`. No hardware, signer, secrets, signed package creation, reset, update, Git stage/commit/push.

**2026-11-09 owner-authorized P2 source-only signer prepared; no native Windows signing yet:** Current previously reported `P2_SIGNED_NORMAL=NOT_AUTHORIZED` was historically accurate before the owner's express new P2 permission. Now the user has authorized **preparation and protected Windows P2 signing only** (no device, reset or Flash). Canonical source-locked OFFLINE P2 operator `Bundle/stm32_os_esp01_bridge_v4_protected_normal9_rescue9_sign_offline_v2_20261109.zip` 17,263B SHA `E80A61249011E38B3F03D0F98537F8B27185F850D410EAB0C19196809889A41C` independently static PASS (source75/75, pinned accepted v4 Windows native ELF/BIN/Evidence, existing signed protected v8 normal/rescue). Owner must run once with existing V10 masked CredUI; **neither rescue9 nor normal9 has been signed or attested yet**, nor have second copy/restore, v10 normal return or physical gates been met. This is not clearance for P3. Active P2 facts and command `docs/STM32_V4_P2_PROTECTED_NORMAL9_RESCUE9_SIGNER_PREP_20261109.md`. Earlier unexecuted v1 signer wrong version-argument source quarantined to Historical; v2 corrected before native.

**2026-11-09 P2 NATIVE WINDOWS SIGNING COMPLETE — earlier P2 NOT_ATTESTED/SIGNING_NOT_AUTHORIZED paragraphs below are historical:** New protected source-locked normal9 `.pkg` original SHA `DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B` 49100B and rev5-content rescue9 `.pkg` SHA `4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928` 52092B actually owner-signed on Windows via once-only source-locked v2 signer, original Windows Evidence ZIP 2614B SHA `DA7AF7BAC95F954A7307C73D041776F329425CE61A2AC97EBF204943F883396C`, independently checked source/CRC5/5 SHA4/4 sizes3/3 and both embedded images, signed header version9 and original file SHA, task `task_1794181186442_547e36b4381df3cd` PASS. **The version-floor9-compatible signed rescue now exists.** However independent recovery copy/restore, rev5 USB `000C` handoff and future normal10+ plan remain NOT_ATTESTED. **No P3 Flash or Gate4 permission**. Consumed operator archived exact SHA, not runnable again. Canonical new `docs/STM32_V4_P2_NATIVE_V9_SIGNER_ORIGINAL_EVIDENCE_ACCEPTANCE_20261109.md`.

## 1. Context and evidence custody

**Installed signed normal v8** (Mac mini P1 read-only physical PASS) USB Management reports SourceTree `2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926`, Service3, Registry39 and PONG. See `docs/STM32_V8_MACMINI_USB_MANAGEMENT_READONLY_BASELINE_20261108.md`. This is physical v8 identity, **not** an on-chip byte readout of all Flash.

**Native signed-v8 original image:** Windows ARM `OS/build/esp010_rx_gate_arm_a1a570e17af74a9ba7899b748ced8c67/build/native/os.bin` 53,196 B SHA `A34F9D37A4DBC707AC19CB91D3C8480B4CD9940CF5911B08043A0792F30466C1`; original unmodified ELF 560,232 B SHA `550E7A3102F30AB9677B06E680479184CEA9AA1FE95EC021F2FE9FB4A78369D0`. Its original 73-file source index SHA `2EFE9374F2FE2AA2FEC47E8C7BC0CA9203E1F92621CB2493FF9673F9D39E4566`.

**Accepted but uninstalled unsigned optimized v4:** canonical source-locked ZIP SHA `9636BF54A53D106713673A6C8E2EBF3C8A9903BDA7ADF30696C26DD145D50568`; indexed 75-source SHA40 `edc5e08ae8dee2455994339c4c438e620c84df1a`; actual Windows ARM `os.bin` **49,020 B** SHA `6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E`, `os.elf` 506,540 B SHA `896957B49F062CD02DA5EDD19C2DC20631540AD4C11D1AE0E41F6F2ACA082C95`; original Windows Evidence SHA `FFC0F6AC1F080E2831C1CC77013154FA7CB665E804055D9BFA4D9F29094F286B` (independent original Evidence PASS). Flash savings **4,176 B** vs signed v8, static SRAM increases **12 B** v8→v4, source preserves 63/73 byte-identical members, changes10 and adds2 without path removal. Direct baseline study `docs/STM32_SIGNED_V8_VS_OPTIMIZED_V4_DIRECT_COMPARISON_20261108.md`.

## 2. Stronger proof: actual linked ROM command registry equality, 39/39

A new independent **read-only ELF32 ARM symbol/ROM decode** validates the actual compiled `command_registry` table in both *real Windows ARM ELF* images. It does **not** assume that a reported registry count means the records are identical. Tool: `Tools/scripts/bench/diff_real_v8_v4_elf_command_abi_20261108.py`, task **`task_1794177222784_aee1b9f1afd33614` PASS**.

- Verified entire original ELF SHA for each; resolved `command_registry` ELF local symbol and read **468 bytes = 39 ×12B** actual ARM ROM descriptors. v8 ROM address `0x0800EC30`; v4 ROM address `0x0800DB18`. Decoded ROM string pointer → exact method name, RPC uint16, Method ID, safety class, min/max argument count and padding on each row.
- All **39/39 compiled descriptor 6-field tuples v8 = v4** in original order, with RPC IDs 1..39, Method ID 0..38, unchanged SAFE/DIAGNOSTIC/DESTRUCTIVE authorization class and min/max args. Independently re-bound v4 ROM descriptors to the exact canonical `command_service.c` source registry. `wdogtrip` remains DESTRUCTIVE, `nodestart/nodecrc` DIAGNOSTIC. No method ID reorder or silent CLI/RPC method disappearance.
- Extra actual v4 ELF machine-code check: `usb_bridge_can_enter()` is exactly **`00 20 70 47`** Thumb bytes (`movs r0,#0; bx lr`), i.e. **ESP maintenance/ROM admission explicitly disabled on the compiled firmware**. This does not activate ESP Flash.
- **Proof limitation:** compiled descriptor equality does **not** establish response payload/wire behavior equality under all runtimes, fault handling, scheduler timing, USB interrupt latency or physical v4 operation. Separate source-regression v4 Binary RPC, exhaustive 65,536 IDs, OLED framebuffer and NodeLink negative tests already PASS. An on-device signed-v4 runtime must still be explicitly qualified.

## 3. Public unsigned firmware image and bootloader-header readiness

Tool: `Tools/scripts/bench/audit_v4_p2_public_boot_header_floor_readonly_20261108.py`, task **`task_1794177459797_6b48c4dd8741ef29` PASS**. It read the exact v4 `os.bin`/75 indexed source files and **public** `OS/bootloader/bootloader.c`, `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`, matching application linker. Never read the protected signer/key or created an HMAC/package.

- Verified source-index SHA, actual v4 image SHA/length **49,020 B** (multiple of 4; below 53,248 B app Flash ceiling). Vector table starts at `0x08002000`, initial MSP word **`0x20005000`** (8-byte aligned, valid SRAM upper bound), Thumb Reset vector **`0x080020DD`** within image.
- Public v1 package schema: `header[48] || HMAC tag[32] || image[49020]`; if legitimately signed under separate owner authorization, expected **total signed package bytes = 49,100**. Header product `0x534F4544`, target `0x0410`, format1, fixed origin `0x08002000`; no inference of valid signature from a computed length. The application remains **UNSIGNED** and no v9 package exists or has been constructed by this task.
- The public bootloader implementation at `bootloader/bootloader.c` enforces version floor in `handle_authorize`: with valid committed app reject `candidate <= floor`; with no valid app reject `candidate < floor`. The monotonic floor is raised when a new candidate is committed. Protocol §9 states the same normal `>floor` vs recovery `>=floor`. A 98-case deterministic offline policy matrix PASS; no real device version floor was freshly read (the signed physical app identity is v8).

**Stronger compiled-C version-floor verification (2026-11-08):** The exact C branch between `cand=load_le32(staged_header+12);` and metadata target selection was extracted **verbatim** from current public `OS/bootloader/bootloader.c`, source SHA256 `A7A52D4962F48D7C95506FB97613BC6A5F7DA661BC07CF3CE850D86247EE25E3`, and compiled/executed with GCC `-O0`, `-O2`, `-Os`, and UBSan: **98 explicit version-state cases in each profile, 4/4 PASS**. No Flash, real HMAC, signer or device was invoked. Tool `Tools/scripts/bench/test_bootloader_exact_authorize_floor_branch_20261108.py`, task `task_1794177712569_4ab7986000296e44` PASS. This is source-executed admission logic rather than merely copying the expected version inequality into Python. It strengthens the floor9/rescue8 safety block, not the live bootloader signature or installed state.

## 4. Material fail-closed recovery blocker — v8 rescue not compatible with floor9

The only currently *documented, SHA-attested protected rescue* is **signed rev5-content rescue v8**, 52,092 B, package SHA **`CDBD0DE29DBFBEBFD24C49FFF3E1D68B1A0D68C69E34CC134F525C65714F2959`** in the protected Windows store; signed normal v8 53,276 B SHA **`DDFBB8AB5B829852B9D0245CE779F26B2B274354AD5E8FD7BAA1474DF4B4281D`**. Both share a single verified protected store; a second independent, restorable copy and live restore drill are **NOT_ATTESTED**. Do not claim a v9 rescue absent merely from no search result; its **verified custody and suitability are NOT_ATTESTED**.

| Pure public version-rule scenario | Verdict |
|---|---|
| current normal8, valid app, floor8 → new normal8 | REJECT |
| current normal8, valid app, floor8 → new signed normal9 | Permitted by version rule only |
| new floor9 with invalid app → old signed rescue8 | **REJECT** |
| new floor9 with invalid app → separately signed rescue9 | Permitted by version rule only |
| new floor9 with valid normal9 → rescue9 | REJECT |
| new floor9 with valid rescue9 → normal10 | Permitted by version rule only |

**Consequences:** installing a signed normal9 can make the existing rescue8 unusable for future recovery after floor9. Even if independently signed rescue9 is prepared, committing rescue9 at floor9 can make a same-version normal9 impossible to reinstall via *normal* update; a recovery-to-normal transition would require normal10+ or another **separately reviewed** future-floor strategy. This sequencing is a true bootloader contract, not an optimization or hypothetical loss of Flash. **Do not commit a new version floor, sign, enter bootloader, reset, or flash until protected recovery compatibility/custody and future normal-return sequence are independently verified and explicitly approved.**

## 5. Status table / next permitted action

| Predicate | Evidence classification |
|---|---|
| v8↔v4 39-command compiled descriptor ABI | **ORIGINAL_WINDOWS_ARM_ELF_READONLY_PASS** |
| v4 source index/origin/vectors/header geometry | **PUBLIC_FORMAT_STATIC_PASS** |
| v4 SRAM/Flash/native stack inventory | **WINDOWS_ARM_RESOURCE_PASS** |
| installed-v8 physical Management/watchdog/stack baseline | **REAL_MAC_PHYSICAL_READONLY_PASS_SCOPED** |
| legitimate protected signed normal v9 package and signer continuity | **NOT_ATTESTED / NOT_AUTHORIZED** |
| future floor9-compatible signed rescue custody + independent copy + restore/return strategy | **BLOCKED / NOT_ATTESTED** |
| any v4 on-device USB/OLED/NodeLink/runtime-stack behavior | **NOT_ATTESTED** |
| hardware-mutating Gate4 | **NOT_AUTHORIZED** |

**Next:** retain real v4 as unchanged source-locked unsigned ARM baseline; before requesting signing, prepare an owner-accepted versioning/rollback design for *both* normal+rescue (including post-rescue return version), establish independent off-Git protected recovery custody and an operator preflight that can verify those exact facts without reading/revealing keys. Only subsequently, and with **new explicit owner scope**, may sign/publish a package; separate **hardware-mutating** owner GO and Gate4 admission are still needed for installation. Re-running unchanged v4 Windows native build or v8 Mac smoke is unnecessary and would not close missing recovery proof.
