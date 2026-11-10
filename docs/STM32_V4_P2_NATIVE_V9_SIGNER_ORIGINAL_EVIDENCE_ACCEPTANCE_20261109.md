# Deus OS — P2 owner-native Windows protected normal9 + rescue9 signing: original Evidence acceptance

**2026-11-09 00:37:18–00:37:55 Europe/Paris.** **Result:** `P2_OWNER_WINDOWS_NATIVE_DUAL_SIGNING=PASS`; `P2_ORIGINAL_INDEXED_EVIDENCE_AUDIT=PASS`; `P2_NEW_PROTECTED_NORMAL9=SIGNED_VERIFIED`; `P2_NEW_PROTECTED_RESCUE9=SIGNED_VERIFIED`; `P2_INDEPENDENT_RECOVERY_COPY_AND_RESTORE=NOT_ATTESTED`; `P3_STM32_FLASH_OR_RESET=NOT_AUTHORIZED`; `PHYSICAL_V4_RUNTIME=NOT_ATTESTED`; `Gate4=NOT_AUTHORIZED`.

**2026-11-09 P3 Host source-only post-P2 supplement:** Genuine Mac .NET10 source-only updater v9 isolated Host DLL SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`, original Mac build+tests SHA audit `task_1794263476838_234a1efc11761968` PASS, actual compiled C# 480 floor/committed matrix for normal8/8 and rescue0/8 or0/9 `task_1794263695991_b430b588d665d971` PASS. Signed normal9/rescue9 originals and local same-store backup unchanged; physical install still NOT_AUTHORIZED and v8 remains live. No second disk requirement. Current source-only P3 technical route and exact evidence `docs/STM32_V9_P3_MAC_NATIVE_HOST_SOURCE_ONLY_READINESS_20261109.md`.

## 1. Genuine owner Windows native result, exact source identity

**2026-11-09 BENCH in-store backup resolved (LATEST):** Owner chose `ProtectedRecovery/Backups/20261109_signed_v9/`, created and full SHA-verified **both protected signed v9 copies**, rescue9 52092B SHA `4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928` and optimized v4 normal9 49100B SHA `DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B`, task `task_1794187701710_81083c0acbb146df` PASS. This is a **same-store copied backup with verified bytes**, not off-media catastrophe recovery; under project single-board BENCH rules, different disk should **not** be invented as mandatory. Linux/SSHFS 0777 synthetic mode cannot prove native Windows NTFS ACL, so mark `ACL_WINDOWS_NOT_ATTESTED` without claiming copies insecure. Remaining real technical scope: bootloader version-floor/return sequencing and rescue physical USB recovery handoff, then separately authorized owner hardware P3; no firmware mutation authorized.

Owner executed exactly once under Windows PowerShell 7:

```powershell
deus-run stm32_os_esp01_bridge_v4_protected_normal9_rescue9_sign_offline_v2_20261109 -ExpectedSha256 E80A61249011E38B3F03D0F98537F8B27185F850D410EAB0C19196809889A41C
```

Observed `deus-run 1.3.2-source-candidate`; original ZIP SHA **`E80A61249011E38B3F03D0F98537F8B27185F850D410EAB0C19196809889A41C`**, manifest verified, native PowerShell AST PASS, local masked owner CredUI prompt for **existing V10 escrow** (owner entered passphrase into the native dialog; not captured in reports). Final `RESULT=PASS`, `CLASSIFICATION=BRIDGE_V4_V9_BOTH_PROTECTED_PACKAGES_SIGNED_OFFLINE`, `NORMAL_CREATED=True;RESCUE_CREATED=True;NORMAL_BYTES=49100;RESCUE_BYTES=52092;DEVICE_HARDWARE=NOT_TOUCHED`. Unlike the historical consumed v8 signer, **no PASS/child-exit mismatch**: source uses separate `$runnerReport` and passed native Windows runner result contract.

Original immutable Windows Evidence **`Evidence/stm32_os_esp01_bridge_v4_protected_normal9_rescue9_sign_offline_v2_20261109_20261109_003754_a18182f44b38479c8ddee91e0cc66a5f.evidence.zip`**, **2614B**, whole SHA256 **`DA7AF7BAC95F954A7307C73D041776F329425CE61A2AC97EBF204943F883396C`**, recorded `2026-11-09T00:37:54.6088274+01:00` (Europe/Paris). This is the actual Windows native receipt, not a reconstructed/synthetic ZIP.

## 2. Independent original evidence and signed package audit — PASS

Read-only original reviewer `Tools/scripts/bench/read_p2_v9_signer_original_evidence_20261109.py` managed task **`task_1794181186442_547e36b4381df3cd` exit0 PASS**. Opened exact **original** Evidence ZIP by complete owner-supplied filename/SHA and checked:

- Original archive ZIP **CRC 5/5**, SHA index **4/4**, size index **3/3**, all five exact members and safe names.
- Original `outcome.txt` schema **`DEUS_OS_ESP01_010_NORMAL_AND_RESCUE_V9_SIGNING_V1`** (source-retained legacy schema label, not different firmware), gate exact P2 v2, `result=PASS` and correct classification, failure NONE, `signed_normal_created=true`, `signed_rescue_created=true`, version 9 for both, actual named outputs and expected lengths.
- Source index SHA **`EDC5E08AE8DEE2455994339C4C438E620C84DF1A5975F501CCD431DC9AD5BBC9`** (v4 **75/75** accepted original sources); original full Windows ARM Evidence SHA **`FFC0F6AC1F080E2831C1CC77013154FA7CB665E804055D9BFA4D9F29094F286B`** and exact 49,020B raw optimized v4 Flash image SHA **`6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E`**.
- `native_checks.txt` authenticated **same existing V10 escrow memory-only restore** (native self-attested) and **same production signing key HMAC verification of both original signed v8 packages**, plus **exclusive native Windows protected write, owner-only NTFS ACL and HMAC + SHA re-read for each new signed v9**, rescue first then normal. Post-sign old originals and Git fingerprints unchanged (`5215AD3EA39EDAF7FEF42F9F23CA46CDA8A93866495DDC25B9BC35B9EFF5F6BD` identical pre/post); Evidence explicitly reports key exported NO, new key file NO, raw firmware in Evidence NO, signed package bytes in Evidence NO, device access NONE, live floor NOT_RECHECKED.
- Auditor separately opened both **new original protected `.pkg`** bytes and checked whole SHA/size matched original `outcome.txt`, exact 48B header SHA, product `0x534F4544`/format1/target `0x0410`/version9/payload length, embedded source SHA, SRAM MSP`0x20005000`, Thumb Reset within correct `0x08002000` app Flash. It **did not have/copy/decrypt the production key or independently recompute HMAC**: HMAC verification and restrictive ACL on Windows are original signer authenticated native self-test evidence.
- The **first** reviewer run locally failed on the reviewer author's guessed new schema label; original actual Evidence has the old `DEUS_OS_ESP01_010` prefix that the exact frozen signer source retained. Corrected **only auditor expected string** after reading original `outcome.txt` safely (inspection task `task_1794181153089_34a4eb1675c5c960` PASS); **original signed packages and Evidence untouched**. Final independent auditor task `task_1794181186442_547e36b4381df3cd` PASS. Not a native signer or device failure.

| Exact original protected output | Whole package bytes/SHA256 | Embedded firmware image | Header SHA256 |
|---|---|---|---|
| **FIRST:** `ProtectedRecovery/deus_os_rev5_000c_recovery_future_floor9_v1.pkg` (rescue v9) | **52,092 B**; **`4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928`** | rev5 rescue, 52,012B; SHA **`AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`** | `E0331085E5A17151C1562F4DF07722058B6B154183BD6267CA785EE2B3B83089` |
| **SECOND:** `ProtectedRecovery/deus_os_000e_bridge_optimized_full_normal_v9_20261109.pkg` (optimized full v4 source normal v9) | **49,100 B**; **`DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B`** | full v4 49,020B Flash; SHA **`6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E`** | `30265E605A2DC420F5B0252E62B1CF57F49503592CEA25AD5B0EA47654BE96F3` |

Preserved original signed normal8 **53,276 B** SHA `DDFBB8AB5B829852B9D0245CE779F26B2B274354AD5E8FD7BAA1474DF4B4281D`; original signed rescue8 **52,092B** SHA `CDBD0DE29DBFBEBFD24C49FFF3E1D68B1A0D68C69E34CC134F525C65714F2959`. Both unchanged after native P2 Windows signing, reverified.

## 3. One-shot signer retirement, no duplicate signing

After accepted original Evidence+two new original package byte identities, the **already consumed** operator ZIP was moved intact out of active `deus-run` search, preserving all 17,263B and whole SHA:

`Bundle/Historical/Bench_20261109_Consumed_NativeP2V9SignerPASS/stm32_os_esp01_bridge_v4_protected_normal9_rescue9_sign_offline_v2_20261109.zip`

Source-only custody script `Tools/scripts/bench/archive_consumed_p2_v9_signer_after_native_pass_20261109.py`, **`task_1794181264661_8dd10bc30cd7ef1a` PASS**, independently rechecked original Evidence and both protected v9 package hashes before moving; SHA/size preserved afterward, active `Bundle/` file absent. Historical unexecuted v1 source bug ZIP remains separately archived unchanged. **Do not rerun** the consumed v2 signer. No signing, Flash, Git mutation or key/material transport during retirement.

## 4. Important qualification boundary: paired signed v9 is not an installed v9 or tested rollback

**Closed now:** previously missing future-floor9-version-compatible **signed rescue9 package exists**, sharing the proven Windows signer key and SHA/ACL custody in the original Windows protected store. Normal9 is also exactly signed and byte-matches accepted optimized v4 ARM image. Original Windows result contract and independently checked indexed Evidence PASS. Source-only v4 ARM full 49,020B app payload gives 4,228B app Flash headroom versus ceiling 53,248B; USB command ABI 39/39 exact signed-v8 vs v4 ROM previously PASS.

**Still open:** a **second independent protected/media-separated, restorable and original-SHA-verified recovery copy** (do not count a duplicate on the same drive/store); a *verified usable update/recovery physical topology* for older rev5 rescue `000C` USB versus current normal `000E`, trusted uploader/bootloader access and owner policy for irreversible version-floor changes; an accepted sequencing strategy for returning from committed rescue9 to normal10+ (normal9 would be rejected when a valid rescue9 at floor9 is installed), and future rescue compatibility if floor becomes10. Device-installed normal v8 is unchanged; no fresh authenticated **real** bootloader floor was read. **No physical v4 USB, stack, I2C/OLED, IRQ, NodeLink under load or ESP programming has been tested.** Signing never itself authorizes firmware mutation.

**Decision:** `P2_PROTECTED_SIGNING_AND_NATIVE_EVIDENCE=ACCEPTED_SCOPED`; `P2_INDEPENDENT_RECOVERY_COPY=NOT_ATTESTED`; `P3_FLASH=NOT_AUTHORIZED`; `GATE4=NOT_AUTHORIZED`. Next safely define owner-held physically independent backup vault, verify strict ACL + media separation + exact original SHA restore; produce an *admission-only source plan* for current floor to proposed floor9 and a separately approved post-rescue normal10 path. Do not construct normal10 as though P2 v9 signature authorizes it.
