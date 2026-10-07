# Deus OS — Target Dead API Cleanup Acceptance Plan

Status: **ACTIVE — GATES 0–3 ACCEPTED / GATE 4 LOCAL ACCEPTANCE COMMIT PENDING**

Canonical design:

`docs/TARGET_DEAD_API_CLEANUP_PLAN.md`

Residual-debt item:

`RDC-03`

Repository baseline:

`5b0f6b9586ac5397c877efb2a83601a43a080f19`

Published hardware baseline:

- firmware candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`;
- application SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`;
- Flash/SRAM `51972/10968`.

## Gate 0 — exact source/contract freeze

PASS requires:

- nine application-runtime accessor deletions + one OLED apply-wrapper deletion exactly;
- historical proof that runtime consumers disappeared at `fa75307`;
- bridge ownership and retained APIs re-proven;
- `binary_frame_encode()` explicitly retained;
- only four target source/header paths authorized;
- build identity/provenance semantics frozen;
- conditional hardware rule frozen before source mutation.

## Gate 1 — implementation/static review

PASS requires:

- changes confined to the four frozen target files plus boundary docs;
- all ten symbols have zero remaining source declarations/definitions/references;
- retained runtime/bridge/OLED/Binary Framed APIs remain;
- no Host/bootloader/linker/startup/script drift;
- `git diff --check` PASS.

### Gate-1 result — PASS

Static review on baseline `5b0f6b9586ac5397c877efb2a83601a43a080f19` confirms exactly seven WIP paths: this plan pair, `docs/CURRENT_STATE.md`, and the four frozen target source/header files. Product diff is deletion-only: 130 target-source lines removed. All ten frozen dead APIs have zero remaining declaration/definition/reference in `include/src`. Retained bridge status/state/view owners, OLED validate/activate owners and checked `binary_frame_encode()` remain present. No Host/bootloader/linker/startup/build-script source changed. `git diff --check` PASS.

### Gate-2 baseline-oracle correction

The first corrected runner (`v2`) reached a real fresh baseline build and reproduced firmware tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`, application BIN SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446` and Flash `51972` exactly. Its build output was `data=144`, `bss=10824`, therefore SRAM `10968`. The run stopped only because this RDC-03 record/harness had incorrectly reused SRAM `10956` from the older Firmware Update Gate-6 tree `8323c68c931894441ae4db9138ba3838f35bb8b6` (`53212/10956`). Evidence `stm32_os_target_dead_api_cleanup_gate2_build_equivalence_v1_20261007_200637.evidence.zip`, SHA-256 `2DF53E9BE5F9E9D82DA7CDBCE33927E35EE836BAB06AB91661E7C7FA66AE8C9F`, is therefore classified as `HARNESS-BASELINE-RESOURCE-ORACLE-01`, not product drift. It performed no candidate build and no target/Flash operation. Gate-2 rerun must use exact baseline SRAM `10968`; no target source byte is changed by this correction.

### Gate-2 composed-collector forensic recovery

The earlier Gate-2 collectors are diagnostic inputs only and are not accepted product failures:

- `v1` evidence SHA-256 `8E499AF5CF8169172EA1BEC65752C4190144CB6D3D72F980836380204C9528D2` stopped in PRESTATE because a PowerShell helper parameter named `$Args` collided with the automatic `$args` variable, causing `git.exe` to receive no `rev-parse HEAD` arguments. No build or target operation occurred.
- `v2` is the baseline-resource-oracle failure recorded above.
- `v3` evidence `stm32_os_target_dead_api_cleanup_gate2_build_equivalence_v3_20261007_201045.evidence.zip`, SHA-256 `55D1656938C72A4338CCC8BC1B8EF31676F856F58014390868A8A3DFD459BB11`, has ZIP CRC clean and internal evidence hashes `63/63` exact. It fresh-built both images with identical `text/data/bss = 51828/144/10824`, Flash/SRAM `51972/10968`, and candidate BIN SHA-256 `D63128FE656651DC7065A905AE8638F3B748629BDE982962A1C00CB24C1D2E68`. Its apparent `TARGET_DEAD_API_SYMBOL_ADDRESS_DRIFT` is a false product classification: all 208 global defined symbols match name/type/address exactly and all 506 defined entries match address/type; the sole textual `nm` difference is compiler-local `CSWTCH.49 -> CSWTCH.45` at the same address `0x0800D990` after function deletion renumbered GCC's internal switch-table label.

Full forensic review also rejects the remaining unexecuted `v3` equivalence oracles: raw full `objdump -h` equality would gate on `-g3` DWARF/debug sections; `.su` file-count equality is weaker than retained function/frame/kind equivalence; the candidate tree was incorrectly based on `bb99acf...` rather than current published HEAD and candidate source bytes were materialized with `Copy-Item` instead of the accepted Git archive byte domain; build artifacts were deleted before downstream assertions were replayable; and terminal FAIL did not return a nonzero process exit code. These are harness/process defects, not evidence of target regression.

Accepted FDC-05..08 logs re-establish the canonical primitive: temporary index `read-tree HEAD`, stage only reopened target paths, `write-tree`, then `git archive`/extract that exact tree. RDC-03 therefore uses current published HEAD plus exactly four target WIP paths for candidate provenance. The physical `bb99acf...` tree remains the executable baseline only. This recovery follows `HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`: the composed collector is frozen until all downstream assertions have been audited against accepted primitives rather than repaired one failure at a time.

The first recomposed final collector stopped before PRESTATE/build because its runner attempted to self-scan for a forbidden literal `Copy-Item -LiteralPath $src -Destination $dst`; that exact literal existed inside the self-check expression itself, so the check necessarily matched its own source. The same block also used `Contains('exit 1')`, which was self-satisfying for the same reason. Evidence `stm32_os_target_dead_api_cleanup_gate2_final_validation_v1_20261007_211555.evidence.zip`, SHA-256 `7DB92FC2A2AB12C05A48948304010322F4E18157219B1948FE4DF77C6831EB3B`, has exact internal hashes and proves `TARGET_IO=NONE / FLASH_MUTATION=NONE`; no build or candidate-tree construction occurred. Classification: `HARNESS-SELF-REFERENTIAL-SELF-CHECK-01`. Runtime self-scanning is removed entirely; package authoring lint owns these source-form invariants, while runtime starts from cryptographic package/prestate verification.

## Gate 2 — deterministic firmware build/equivalence

Require:

- candidate tree derived by the accepted `temporary index from current HEAD -> stage exact four target paths -> write-tree -> git archive -> extract` primitive; no `Copy-Item` working-tree materialization is accepted as candidate identity;
- baseline clean source materialized from current published HEAD; current target/linker/startup/build-script domain is proven byte-unchanged from the physical `bb99acf111dfa3a78193b4e5d3376fa077defa1e` baseline;
- same accepted GNU Arm toolchain and unchanged build script for both builds;
- exact baseline reproduction: BIN SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`, Flash/SRAM `51972/10968`;
- candidate Flash/SRAM exact `51972/10968`, within `53248/11264` ceilings, warning-clean compile/link;
- undefined symbols 0; heap/libc drift 0;
- candidate deleted-symbol inventory 0 and retained contract owners present;
- global named defined-symbol address inventory exact; local compiler-generated labels are diagnostic, not ABI gates;
- runtime alloc/load section geometry and vector/linker boundaries exact; raw debug/DWARF sections and raw debug-enabled ELF/MAP hashes are run-local evidence only;
- `.su` retained function/frame/kind map exact after removing exactly the ten deleted functions, ignoring source-line/path text;
- whole candidate BIN byte-exact to the baseline BIN after replacing only one same-offset 40-byte `bb99acf...` provenance literal with the candidate tree literal;
- baseline/candidate BIN/ELF/MAP, `.su`, symbol/section/vector reports and all hashes persisted before semantic verdict;
- exact candidate pre/post repository path/hash proof;
- TARGET_IO=NONE / FLASH_MUTATION=NONE;
- every terminal FAIL returns a nonzero process exit code.

### Gate-2 result — PASS BY FORENSIC ADJUDICATION

Authoritative deterministic build/equivalence evidence is `stm32_os_target_dead_api_cleanup_gate2_final_validation_v2_20261007_212047.evidence.zip`, SHA-256 `224F04F33A91DAC295D0716ABF52B1457CC7DDA8C98316D17098243CBB05FE33`. ZIP CRC is clean and the internal evidence index is `141/141` exact. The collector's structured FAIL is rejected only for its retained-symbol oracle; all build/equivalence facts are accepted from the hash-owned raw evidence.

Accepted Gate-2 facts:

- RDC-03 candidate tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b` from current published HEAD plus exactly four target paths;
- archived candidate target bytes exactly match the frozen WIP hashes;
- GNU Arm `15.3.Rel1` exact;
- physical baseline BIN SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446` reproduced exactly at Flash/SRAM `51972/10968`;
- candidate BIN SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`, also Flash/SRAM `51972/10968`;
- undefined symbols empty and forbidden heap/libc inventory empty;
- all ten deleted APIs absent from the linked candidate;
- global defined symbols `208/208` exact by name/type/address;
- runtime section geometry exact: `.isr_vector`, `.text`, `.data`, `.bss` size/VMA/LMA unchanged;
- stack proof removes exactly ten deleted function records and retains `269/269` normalized function/frame/kind records exactly;
- BIN length exact `51972`;
- baseline and candidate provenance literals each occur exactly once at offset `51784`;
- replacing only the baseline 40-byte source-tree literal with candidate tree `3bdb90d...` produces the candidate BIN byte-for-byte, SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`;
- non-provenance BIN differences `0`;
- vector-prefix SHA-256 exact `5EDF0E72D47BFD8885352A708F752825AEFC8584D5968E6FAE477F41218B3940`;
- repository/index/WIP poststate exact; target I/O and Flash mutation NONE.

The seven reported `RETAINED_SYMBOL_PRESENT` failures are `HARNESS-RETAINED-SYMBOL-ORACLE-01`, not product failures. Six required linked owners are visibly present in `candidate_nm_all.stdout.txt` (`application_runtime_bridge_status`, `application_runtime_bridge_is_initialized`, `application_runtime_bridge_view_dirty`, `application_runtime_bridge_state_at`, `oled_ui_layout_config_v1_validate`, `oled_ui_layout_config_v1_activate_validated`); the PowerShell regex used `$` against CRLF `nm` output without newline normalization, so every one was falsely reported absent. `binary_frame_encode()` is intentionally retained as a checked **source/protocol compatibility API** and remains declared/implemented, but has no current linked consumer and is correctly removed by `--gc-sections`; linked presence was never a valid acceptance requirement for it.

Gate 2 is therefore ACCEPTED. Exact-candidate Gate 3 hardware runtime acceptance is authorized for tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b` / BIN SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016` only.

## Gate 3 — exact-candidate hardware runtime acceptance

Gate 3 is mandatory after Gate 2 PASS. It must use the exact Gate-2 candidate tree/BIN and the already accepted bootloader/recovery/operator topology to prove:

- exact flashed application identity and candidate source-tree provenance;
- runtime USB management path operational;
- UART diagnostics/liveness operational;
- OLED physical output PASS;
- scheduler/IWDG/application-runtime liveness PASS;
- no unexpected bootloader/recovery regression;
- exact poststate/recovery evidence.

A Gate-2 non-provenance BIN/layout mismatch blocks hardware deployment; hardware success cannot waive deterministic equivalence failure.

### Gate-3 result — PASS / EXACT-CANDIDATE HARDWARE + READ-ONLY CONTINUATION

The exact Gate-2 candidate tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b` / BIN SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016` was deployed through the accepted authenticated bootloader update path. The mutating run completed the signed update and passed candidate runtime source identity, capability `0x0000007F`, USB management, application start/stop/home restore, scheduler/IWDG liveness, UART `ping`, UART `schedprod` and `oledping`, then stopped only on an invalid all-`0xFF` requirement for the untouched final executable page. Parent evidence SHA-256 is `652319CB3ACD6AA11121A61CB7EE0269440270CB7C0A426763B6345762A8EF1F`; its final classification is adjudicated as `HARNESS_APPLICATION_TAIL_ERASE_ORACLE_01`, not product failure.

Read-only continuation `stm32_os_target_dead_api_cleanup_gate3_readonly_continuation_v2_20261007_232443.evidence.zip`, SHA-256 `046E4F1F30A69CDAE00B6B9A37B2C02E16A05F7A4F4EBBA56AB1F48E23341AB9`, has clean ZIP CRC and `39/39` exact hash-owned entries. It proves:

- exact application SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016` on target;
- bootloader region SHA-256 `106949634C0544A9B1817126B21C2B47F8BF24E6C2A52FEC7494CBAD8C9B18B7` unchanged;
- persistence region SHA-256 `D0FF1B294B5288D1AE1421EADF5B2D38A8752B76D472FF30BED9028E25B1C5B8` unchanged;
- metadata region SHA-256 `BF2949E5271BD798614E44CF3160CBAAD9CED2CB49F67D8C829EDB0DB50B50F3` recorded;
- remainder of the final DATA-touched application page erased exactly;
- untouched page 59 has no all-`0xFF` acceptance requirement under the frozen lazy-page-erase contract;
- runtime source tree exact before and after read-only SWD;
- capability `0x0000007F`, active app `0x0001`, app faults `0`, advancing kernel tick and IWDG reload count;
- repository/index/WIP poststate exact;
- continuation `FLASH_MUTATION=NONE / RESET=NONE`.

Operator physical review after the accepted candidate deployment is `PHYSICAL_OLED=PASS`. Gate 3 is ACCEPTED.

## Gate 4 — local acceptance commit

Exact staged path review, staged diff check PASS, one normal commit, clean post-commit state.

## Gate 5 — ordinary non-force publication

Fresh direct-parent proof, ordinary fast-forward push and post-push fresh fetch with `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

## Failure classes

- `TARGET_DEAD_API_SCOPE_DRIFT`
- `TARGET_DEAD_API_RETAINED_API_REMOVED`
- `TARGET_DEAD_API_BUILD_FAILURE`
- `TARGET_DEAD_API_RESOURCE_DRIFT`
- `TARGET_DEAD_API_NORMALIZED_BIN_MISMATCH`
- `TARGET_DEAD_API_HARDWARE_REQUIRED`
- `TARGET_DEAD_API_DOC_FAILURE`
