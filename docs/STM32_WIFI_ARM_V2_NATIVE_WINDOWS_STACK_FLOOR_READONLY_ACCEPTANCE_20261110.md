# STM32 Wi-Fi ARM V2 — native Windows read-only original ELF/MAP/stack and memory-floor audit

**2026-11-10 23:50 Europe/Paris. Status: `ARMV2_STACK_FLOOR_NATIVE_WINDOWS_READONLY_ACCEPTED_SCOPED`.** Sole global project-state authority: `docs/CURRENT_STATE.md`. This audit does **not** create or sign firmware, measure the physical Cortex-M3 stacks, change the protected bootloader/Flash, or authorize Gate4.

## Original operator and evidence custody

Owner performed **one** Windows native `deus-run 1.3.2-source-candidate` of canonical OFFLINE `stm32_os_esp01_wifi_arm_v2_stack_floor_ro_v1_20261110`. Runner matched exact whole ZIP SHA, verified Bundle V2 manifest and parsed `apply.ps1` with PowerShell AST. Original result `PASS`; classification `ARMV2_STACK_FLOOR_READONLY_ACCEPTED_SCOPED`.

- Operator: `../Bundle/stm32_os_esp01_wifi_arm_v2_stack_floor_ro_v1_20261110.zip`; **13,260 B**, full SHA-256 `D8808ECD87CBA020852968938793CD81AEE4355C6A5306448DAD6AFFCA6AB273`. Previously canonical Bundle Guard build + validate PASS, 6 members; independent stat still matches.
- Original Windows Evidence: `../Evidence/stm32_os_esp01_wifi_arm_v2_stack_floor_ro_v1_20261110_20261110_235037_e660d59c534c45ea8f781bb9b9a95589.evidence.zip`; **3,399 B**, full SHA-256 `CC532554802A6C7BAB588441406FE1613F1C725FE07AB6910B6F114C6C093231`. Owner's source was `D:\Projects\STM32\Evidence`; independent DEUS MCP read-only WSL inspection found exact shared bytes and matching SHA/length. No inference from WSL POSIX `0777` modes to Windows native NTFS ACLs.
- Evidence integrity independently verified: exact **7 safe unique ZIP entries**, **CRC 7/7**, member **SHA-256 index 5/5**, **size index 5/5**. Members: `outcome.txt`, `run.log`, `native_checks.txt`, `native_stdout.txt`, `native_stderr.txt`, `hashes.sha256`, `sizes.txt`. The generic archive summary's `manifest_missing` is expected for this evidence dialect and does not invalidate the explicit ZIP/index/CRC audit.
- Read-only original reviewer: `scripts/bench/audit_arm_v2_stack_floor_original_windows_evidence_20261110.py` executed via managed DEUS MCP; final strict original Evidence audit **`task_1794351237642_f952c94521d86228` exit 0**, after independently checking outcome schema/classification/payload/Git pre/post/child status/metrics and 10 ELF layout + 20 top-frame records. An interim strict auditor run failed only because its regex did not tolerate Windows CRLF; reviewer fixed and reran **against identical immutable original Evidence**. This was not an operator replay or target-build failure.
- `outcome.txt`: schema `DEUS_OS_ARMV2_EXISTING_STACK_FLOOR_EVIDENCE_V1`; `result=PASS`; `classification=ARMV2_STACK_FLOOR_READONLY_ACCEPTED_SCOPED`; `failure_phase=NONE`, `error=NONE`; `child_exit=0`; `native_stderr=0 B`, `TIMED_OUT=False`; `no_usb_uart_reset_flash=true`. Native `git_pre == git_post == 27E4A2348978DCA3EC2FC5B12CEAF5E510165521A79B05F9D79226A0A3637477`; this fingerprint is not a source Git commit or a signed package identity.
- Audit helper `payload/arm_v2_existing_artifacts_stack_floor_readonly.ps1` **7,368 B**, SHA-256 `C514AF75E63C43A66223103E2669E4E521A5F4677F3F48D677707565C0C07CA4`. It read the **existing** original Windows native ARM V2 output directory rather than rebuilding. The original V2 compile/link source-only acceptance from 22:55 remains separately grounded in `docs/STM32_WIFI_ARM_V2_NATIVE_WINDOWS_OFFLINE_ACCEPTANCE_20261110.md`.

## Byte-locked original ARM artifacts and static stack audit

The native Windows read-only child reported SHA checks **PASS** on the existing original files; independently audited original Evidence contains the bound names, lengths and hashes. **The WSL Evidence auditor verified these original log claims and their index integrity, not a second independent download of the ELF/BIN/MAP bytes.**

| Original artifact | Size | Expected/rechecked SHA-256 |
| --- | ---: | --- |
| `os.bin` | **52,052 B** | `6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73` |
| `os.elf` | **544,236 B** | `C60B803B2A605170B0FCA461A391D8EEC6E90724E778FB37B9118A612F3B14A5` |
| `os.map` | **322,548 B** | `549E7F6EB4195EF31A8A83D0F41E11EC27CDD1C637E5BB6B9B67799B0737CF28` |

All **38 original GCC `.su` files** were individually matched against the SHA-index created directly from original native ARM V2 compiler output: index SHA `F8E85B7960B54F0A2F0D1FF2CA161F5398578EEC1499EBDC00CCD166565AF069`. Original Windows child parsed **334 function records**, `UNPARSED=0`, **`DYNAMIC=0`**, no individual static frames `>=256 B` or `>=512 B`, maximum **240 B**.

| Selected function | Largest reported per-function frame |
| --- | ---: |
| `text_renderer_fast_path_self_test` | 240 B |
| `oled_status_bar_self_test` | 232 B |
| `console_execute_safe_method` | 216 B |
| `node_transport_service` | 144 B |
| `dnl_worker_process` | 128 B |
| `deus_node_wifi_client_v0_receive` | 104 B |
| `deus_node_wifi_client_v0_request` | 88 B |
| `node_transport_wifi_request` | 88 B |

**Interpretation limit:** these values describe *individual GCC-reported C function stack frames* only. They are **not additive call-chain upper bounds**; nested synchronous calls, preemption, ISR exception entry, assembler paths and scheduler context frames can require additional stack. No physical PSP/MSP watermark, canary or watchdog was sampled in this operator.

Source-defined reservations: task0/console PSP **1,024 B**, heartbeat PSP **512 B**, MSP reserved **2,048 B** including **64 B guard**. A single largest frame of 240 B is **not** a proof that either PSP margin is safe under nested Node Wi-Fi requests. Original native link resource ceilings continue to pass but retain tight **Flash 52,052/53,248 B, margin 1,196 B (2.25%)** and **SRAM 8,584/11,264 B, margin 2,680 B (23.79%)**.

## ELF linker layout, bootloader floor and memory protection

The Windows native child inspected SHA-pinned `os.map` plus `arm-none-eabi-nm -n` output of the exact SHA-pinned `os.elf`, **10/10** required absolute symbols and `_ebss <= _smsp_stack` passed:

| Required symbol | Accepted address |
| --- | --- |
| `_application_flash_origin` | `0x08002000` |
| `_application_flash_end` / `_firmware_metadata_slot_a` | `0x0800F000` |
| `_firmware_metadata_slot_b` | `0x0800F400` |
| `_persistence_slot_a` | `0x0800F800` |
| `_persistence_slot_b` | `0x0800FC00` |
| `_persistence_flash_end` | `0x08010000` |
| `_smsp_stack` | `0x20004800` |
| `_emsp_guard` | `0x20004840` |
| `_estack` | `0x20005000` |

Original `_ebss=0x2000198C` is below MSP start `0x20004800`; the linker layout did not collide with the reserved MSP. Original reset entry lay in allowed app Flash range, checked by the child. **This does not attest live bootloader rollback or update authorization, nor imply all bytes between .bss and MSP are unallocated stack headroom.**

## Signer/recovery readiness boundary and next safe work

**Accepted now:** existing exact-byte STM32 Wi-Fi ARM V2 native compiler/linker output rechecked on Windows without rebuild, original 38 `.su` and 334 static frame entries, expected metadata/persistence linker floor and no .bss/MSP collision, original indexed Evidence integrity.

**Not accepted:** complete ARM call graph and task0/heartbeat worst-case stack; live firmware PSP/MSP/IWDG/OLED/NodeLink/USB; new normal10 signature, HMAC/secure-storage compatibility, signed release version/floor and rollback/recovery; real STM32/ESP firmware update or ESP RF scan. Prior protected signed normal9 and rescue9 custody is separately accepted historically, **not accessed, changed or retested here**; independent rescue restore and floor10-compatible normal10 return path are **not attested**.

**Next engineering decision:** offline analyze call paths and scheduling/IRQ nesting for 1,024 B task0 PSP and 512 B heartbeat PSP, separately review the signed normal9/rescue9 production-version floor and signer input identity using only public contracts and original protected package receipts; then prepare a **separately scoped signing/recovery preflight**, without exposing keys or creating a signed normal10 package until specifically authorized. The old signed STM32 normal9 and ESP app v2 remain the installed hardware baseline. `ESP01-010 PARTIAL`, **Gate4 NOT_AUTHORIZED**.

No new firmware build, bundle rerun, USB/CH340 open, STM32 Flash/RESET, ESP GPIO/power or RF operation occurred during this acceptance/audit. No Git staging/commit/push.
