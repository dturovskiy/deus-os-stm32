# STM32 Wi-Fi NodeLink optional registry-40 — original native Windows ARM V2 offline acceptance

**2026-11-10 23:17 follow-up:** One separate OFFLINE read-only original ELF/MAP/38 .su stack/floor operator ZIP Bundle/stm32_os_esp01_wifi_arm_v2_stack_floor_ro_v1_20261110.zip, 13260B SHA D8808ECD87CBA020852968938793CD81AEE4355C6A5306448DAD6AFFCA6AB273, canonical Bundle Guard build+validate PASS. It does not repeat V2 compilation. Native Windows stack/floor original Evidence is **pending**. Signing normal10, runtime PSP/WDOG and physical updates remain NOT_ATTESTED/NOT_AUTHORIZED.

**2026-11-10 22:55 Europe/Paris. Status: `STM32_WIFI_ARM_V2_WINDOWS_NATIVE_OFFLINE_ACCEPTED_SCOPED`.** Global project authority is `docs/CURRENT_STATE.md`. Offline compilation/link acceptance is **not** permission to install, sign, reset, operate ESP RF, or change the physical bench. Earlier V1 compiler FAIL is retained in `docs/STM32_WIFI_ARM_V1_WINDOWS_NATIVE_FAIL_V2_CANONICAL_REPAIR_20261110.md`.

## Original operator / Windows execution identity

Owner ran one native Windows `deus-run v1.3.2-source-candidate` command for the existing canonical OFFLINE operator. Console proves native Windows selected matching bundle SHA, V2 manifest verified, PowerShell AST parse PASS, and original result PASS; independently DEUS MCP confirms operator ZIP remains identical:

- `../Bundle/stm32_os_esp01_wifi_arm_offline_v2_20261110.zip` — **160,832 B**, SHA-256 `A7D7380874A826A3AEFB7A09BE7E0A7BFD454246DC41C8D73E2C593AEE8D78E0`, 90 members, canonical V2 build/validate previously PASS
- Original native Windows Evidence at `../Evidence/stm32_os_esp01_wifi_arm_offline_v2_20261110_20261110_225532_2527c645f4e44d028607a97564ccd3d6.evidence.zip` — **6,315 B**, SHA-256 `A83C4000789C19D00503CA4BF75B3901ED940A0A44DC2E53A2AFB24E40DDFCE5`
- Independent read-only WSL audit script: `scripts/bench/audit_wifi_arm_v2_original_windows_evidence_20261110.py`. Managed task `task_1794347967324_34be53ebe13de524` **exit0**; verified exact whole original Evidence SHA/byte count, ZIP **CRC7/7**, **SHA-index5/5**, **size-index5/5**, seven distinct safe entries. First two versions of the auditor were rejected due to an *auditor-only* file-extension rule erroneously disallowing `run.log`; the actual original Evidence was never changed and final audit passed
- Original contents from `outcome.txt`: schema `DEUS_OS_WIFI_ARM_NATIVE_OFFLINE_EVIDENCE_V1`, `result=PASS`, `classification=WIFI_ARM_OFFLINE_NATIVE_BUILD_PASS_SCOPED`, `failure_phase=NONE`, `error=NONE`, `child_exit=0`, native stdout recorded, native stderr **0 B**, `no_usb_uart_reset_flash=true`, `PSP_RUNTIME=NOT_ATTESTED`
- `git_pre == git_post == D37F0ABC970C4D53F2AEAD869D2033950AEF0547DB906C89BF891E69FDE16157`; this is the Windows operator snapshot, not a claim that the existing owner WSL working tree has no uncommitted changes
- Payload helper **5,103 B**, SHA-256 `6D6D2EF10A47D9BEE7476832122190D0936F890437C47D9F629948802CFB7708`; exact source index SHA `D892610A4B842FE84B17C68DA2546BD29C5698B66B9B76D408BB430D93CD1BC5`, **84** fixed inputs, source identifier `d892610a4b842fe84b17c68da2546bd29c5698b6` = index SHA prefix160, **not a Git tree OID or signed release identity**
- Child build `arm-none-eabi-gcc.exe` GCC **15.3.1**, Cortex-M3 Thumb, `-Wall -Wextra -Werror -fstack-usage`, real `link` exit0, undefined-symbol scan `arm-none-eabi-nm -u` exit0, and `BUILD_OUTCOME=PASS` recorded

## Independently checked numeric consistency (original native stdout)

| Resource | Native used | Acceptance ceiling | Remaining |
| --- | ---: | ---: | ---: |
| Flash load / `os.bin` | **52,052 B** | 53,248 B | **1,196 B (2.25%)** |
| SRAM static sections | **8,584 B** | 11,264 B | **2,680 B (23.79%)** |

Section reconciliation from the original child log: `BUILD_TEXT=51968`, `BUILD_DATA=84`, `BUILD_BSS=8500`, `BUILD_FLASH_SECTION_SUM=52052`, `BUILD_FLASH_LOAD_PADDING=0`, `BUILD_FLASH=52052`, `BUILD_SRAM=8584`, `BUILD_BIN_SIZE=52052`. All repeated metrics reconcile with `outcome.txt`. **Flash budget passes, but has only ~2.25% remaining and should be treated as a feature-growth constraint.**

Native original build recorded these linked artifact byte counts and digests:

| Artifact | Byte count | Original native SHA-256 |
| --- | ---: | --- |
| `os.bin` | 52,052 | `6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73` |
| `os.elf` | 544,236 | `C60B803B2A605170B0FCA461A391D8EEC6E90724E778FB37B9118A612F3B14A5` |
| `os.map` | 322,548 | `549E7F6EB4195EF31A8A83D0F41E11EC27CDD1C637E5BB6B9B67799B0737CF28` |

**Evidence scope note:** The full Evidence directly contains the original native stdout with those hashes, the outcome/checks/run logs, and internal SHA/size indices. The actual ELF/BIN/MAP bytes and the 38 individual `.su` files are in the owner's private native build directory, **not embedded in the seven-member Evidence ZIP**; their original native hash values were logged but were not independently recomputed from Windows file bytes by this read-only WSL audit.

Original native stdout enumerates **38 distinct `.su` files** with SHA-256 references, including the new `src_kernel_node_transport.su`, `src_kernel_node_wifi_client_v0.su`, and `src_drivers_wifi_node_extension_v0.su`. This proves 38 stack-usage reports were produced and indexed; it does **not** prove call-chain worst-case, MSP/PSP watermark, runtime canaries, watchdog timing or absence of stack collisions.

## Accepted boundary and next engineering gate

**Accepted:** one exact-source native Windows GCC15.3.1 ARM build + linker MAP/resource ceiling checks, successful source/configuration handoff, no unresolved-symbol scan failures, original evidence integrity, Git source/working documentation unchanged by native child.

**Not accepted:** installed signed normal10 firmware; compatibility with protected bootloader and rollback/source signer identity; real normal9→normal10 STM32 management update; command RPC `nodewifi` on actual USB/USART2; ESP v2→new Wi-Fi dispatcher installation; 3.3V radio power and brownout margin; live scan/AP pagination; task0 runtime PSP, IWDG, scheduler and OLED after update; Gate4. The currently installed STM32 normal9 and ESP node app v2 remain as last hardware-authorized baseline. `ESP01-010=PARTIAL`; `Gate4=NOT_AUTHORIZED`.

**Recommended next safe scoped step:** finish a read-only static analysis of the linked ARM ELF/MAP and `.su` call chains, assess Flash headroom against normal9's protected update/recovery contract and prepare a separate **source/target locked signer and recovery floor preflight**. Before any physical installation, there must be **fresh explicit owner scope** for the specific signed STM32 app and, separately, the ESP app-only candidate; no automatic flash/reset/CH340/ESP RF action from this PASS.

No stage/commit/push was made during this audit. Preserve the original V1 FAIL and V2 PASS ZIP and Evidence immutable.
