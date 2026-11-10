# STM32 Wi-Fi optional NodeLink — original Windows ARM V1 FAIL and corrected V2 OFFLINE operator

**2026-11-10 22:55 Europe/Paris — V2 NATIVE ARM PASS CLOSURE:** The V2 operator now genuinely ran once on owner Windows. Original 6315B Evidence SHA `A83C4000789C19D00503CA4BF75B3901ED940A0A44DC2E53A2AFB24E40DDFCE5` passed independent CRC7/7 SHA5/5 size5/5 audit; GCC15.3.1 ARM compile/link + nm-undefined exit0, Flash52052/53248 (1196 margin), SRAM8584/11264 (2680 margin), 38 .su indexed, outcome child0 and Git pre/post equal. Prior statements below saying **V2 native NOT_RUN** are historical and explicitly **superseded** by this accepted original result; see `docs/STM32_WIFI_ARM_V2_NATIVE_WINDOWS_OFFLINE_ACCEPTANCE_20261110.md` and global `docs/CURRENT_STATE.md`. V1 FAIL is still valid historical evidence, physical PSP/Flash/Radio remain untested.

**Date:** 2026-11-10, Europe/Paris. **Authority:** `docs/CURRENT_STATE.md`; original operator/evidence custody is outside Git. **Classification:** V1 `ORIGINAL_NATIVE_FAIL_ACCEPTED_DIAGNOSTIC`; corrected V2 `CANONICAL_STATIC_PASS_NATIVE_WINDOWS_PENDING`. This report does not change Gate4 or authorize firmware update.

## Original V1 Windows failure — independently recovered (immutable)

Owner ran exactly one Windows native `deus-run v1.3.2-source-candidate` invocation for `stm32_os_esp01_wifi_arm_offline_v1_20261110`. Runner selected the expected exact operator ZIP **160040 B / SHA-256 `4ED70C1E9A9F00C3E011F003344C9B774606C4BD9B2902FFAF4F8D6464A6CA11`**, verified its V2 manifest, and parsed `apply.ps1` with Windows PowerShell AST **PASS**. Actual result: `RESULT=FAIL`, `CLASSIFICATION=WIFI_ARM_OFFLINE_NATIVE_BUILD_FAIL`, `FAILURE_PHASE=WIFI_ARM_NATIVE_WINDOWS`, `ERROR=ARM_CHILD_NONZERO`, child exit1, no timeout and no device action.

Original owner Windows evidence basename: `stm32_os_esp01_wifi_arm_offline_v1_20261110_20261110_222040_cee7e375ba2d4e07953d4aa566485447.evidence.zip`. Windows source: sibling `D:\Projects\STM32\Evidence`; actual matching WSL visible original `../Evidence/<basename>` independently read in full, **3101 B**, whole SHA **`34D5FD35FBDFFB3D01B154F519B53AD349ED3A9ED6687A9EF23E4AB668593CA3`**. Original archive **CRC 7/7**, index SHA **5/5**, size **5/5** PASS. `outcome.txt` gives `git_pre == git_post == 0C44D89F6A99720117AC66CF098D22D34915A2003C7A42522252F9C704063609`. Captured `native_stderr.txt` retains the actionable first failure and terminal command error, not merely a parent exit code:

```text
wifi_node_extension_v0.c:37:28: error:
comparison is always true due to limited range of data type
[-Werror=type-limits]
return s!=0 && s->state>=DEUS_WIFI_OFF &&
cc1.exe: all warnings being treated as errors
compile-src_drivers_wifi_node_extension_v0.o failed with exit 1
```

ARM GNU 15.3.1 had started genuine native compilation. Successful earlier objects were logged; the build stopped at the first failing Wi-Fi translation unit. **Actual target Flash/SRAM, linked image, static stack .su total, watchdog and runtime PSP = NOT_ATTESTED.** Do not replay V1.

## Exact source repair

`nodes/esp8266/firmware/wifi_lab/wifi_node_extension_v0.c`, function `snapshot_valid`: removed **only the redundant** lower-bound check `s->state>=DEUS_WIFI_OFF` (`DEUS_WIFI_OFF=0`; GCC15 says always true) and documented why. Preserved `s!=0`, the meaningful upper-bound `s->state<=DEUS_WIFI_ERROR`, `count<=MAX_APS`, and `truncated<=1`. No changes to DNL REV2, ESP installed app v2, STM32 installed normal v9, USB, USART2 wiring, or ESP SDK service.

- Original V1 C: 10985 B SHA `63E0B38D256404B2ACAC42DEB6BC83470553D99CD50FF32A3EE77279F56637FD`.
- Patched V2 C: 11222 B SHA `D1BA5AD91FF93C80E40FB0B4A3B772AA88D3FC08151E68732BC67CC3149740C2`.
- Genuine native Mac mini GCC15.2 C11 `-Wall -Wextra -Wpedantic -Werror -Werror=type-limits`, O0/Os/UBSan: **3/3 PASS** for actual frozen DNL REV2 + patched codec + full native C test. Original worker `wifi-arm-gcc15-failfix-source-only-native-mac-1110-a2`: `completed/exit0`, queue SHA `28855584102E13FBA916AADCCF73B0CC2AF6F70DD752B485E5366292DE3D59F4`, receipt SHA `1850F6D7C021CB44CB5CB09C1C3BE94B3432E0E0FC2A5260183B437F5FBBEB6`, full log SHA `004709EB10ACEAF8182EB6A9DD7EAAB74DFCB94EDE65D0292EF6ED803D7EB1ED`. The earlier worker a1 was a **source-preflight SHA literal typo**, exited1 before compilation, corrected in the same exact-pinned script and preserved; do not misclassify as code test failure.
- Independently managed WSL Python native source tests O0,Os,UBSan **3/3 PASS**, task `task_1794347058619_4b96a2a9b0b87091` exit0.

## ONE corrected OFFLINE operator V2

Prepared exactly one next stage `../Bundle/_stage_stm32_os_esp01_wifi_arm_offline_v2_20261110/`: all **84** source inputs explicitly size/SHA locked; comparing V1 to V2 input manifests shows just the patched C source differs. New `payload/source_sha_index.tsv` SHA **`D892610A4B842FE84B17C68DA2546BD29C5698B66B9B76D408BB430D93CD1BC5`**, `payload/wifi_arm_native_offline_build_v2.ps1` SHA **`6D6D2EF10A47D9BEE7476832122190D0936F890437C47D9F629948802CFB7708`**, top-level `apply.ps1` SHA **`62F08F3AE1B59B0CD44810C66B78F953F1854F8203E035C6E3A866174A519F1F`**, 89 stage entries including five control members.

**Canonical Bundle Guard** `build` exit0 **and** `validate --sha256` exit0 using the official `Tools/scripts/operator_bundle_guard/bundle_guard.py`, invoked by its reviewed OS-repo-local Python managed-runner wrapper, single completed task `task_1794347169790_af1a82ce477e44cf`. Exact final **`../Bundle/stm32_os_esp01_wifi_arm_offline_v2_20261110.zip` = 160832 B, SHA `A7D7380874A826A3AEFB7A09BE7E0A7BFD454246DC41C8D73E2C593AEE8D78E0`**, 90 unique ZIP members, `manifest.json`, `hashes.sha256`, independent full-file SHA/size reread, no unsafe names or duplicates, CRC test pass. `profile=OFFLINE`, `qualification=STATIC_ONLY_NOT_NATIVE_OR_HARDWARE`. V1 original operator/evidence SHA stayed unchanged; no global MCP profile change, no handwritten manifest, no other operator variant.

## Native Windows follow-up — still pending

Before a **single** next native Windows execution, owner must verify that the *Windows* sibling `D:\Projects\STM32\Bundle\stm32_os_esp01_wifi_arm_offline_v2_20261110.zip` has exact full SHA above (WSL existence alone does not prove Windows identity). Accepted runner command:

```powershell
deus-run stm32_os_esp01_wifi_arm_offline_v2_20261110 -ExpectedSha256 A7D7380874A826A3AEFB7A09BE7E0A7BFD454246DC41C8D73E2C593AEE8D78E0
```

**No automatic rerun of V1**, no manual `apply.ps1`, no parallel ZIP. Stop at the first Windows V2 PASS or classified FAIL; preserve actual new sibling original Evidence and independently audit exact ZIP/index, `child_exit`, first terminal compiler/PowerShell error, Git pre/post, linked MAP origin `0x08002000`, Flash<=53248 B, SRAM<=11264 B, 38 .su files; differentiate `NATIVE_SOURCE_PASS` from still unmeasured live PSP and watchdog margin. Do not Flash, sign, RESET, open CH340/USB, change power/GPIO or start radio without a fresh **specific** hardware authorization. `ESP01-010=PARTIAL`; `Gate4=NOT_AUTHORIZED`.

`git diff --check` passed after patch, staging remained empty. This is one scoped repair, not a claim that all future target compiler errors or ARM linked resource ceilings are already solved.
