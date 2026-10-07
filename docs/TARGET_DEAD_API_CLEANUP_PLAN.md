# Deus OS — Target Dead API Cleanup Plan

Status: **CLOSED / PUBLISHED `e516fdc1d8818007db40ee12669d28f3f828c489`**

Boundary ID:

`TARGET_DEAD_API_CLEANUP`

Residual-debt ID:

`RDC-03`

Repository baseline:

`5b0f6b9586ac5397c877efb2a83601a43a080f19`

Current hardware-accepted firmware candidate:

`bb99acf111dfa3a78193b4e5d3376fa077defa1e`

Current hardware-accepted application SHA-256:

`0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`

Fresh-build resource identity for this exact tree under the accepted GNU Arm toolchain is Flash/SRAM `51972/10968`.

## 1. Purpose

Remove only re-proven target C API surface whose consumers disappeared during accepted ownership decomposition, while preserving retained compatibility/public behavior and proving that linker-visible runtime behavior is unchanged.

## 2. Exact deletion set

### Application Runtime — nine accessors

Remove declaration + implementation of exactly:

1. `application_runtime_is_initialized()`;
2. `application_runtime_state_get()`;
3. `application_runtime_active_id()`;
4. `application_runtime_fault_count()`;
5. `application_runtime_event_count()`;
6. `application_runtime_view_revision()`;
7. `application_runtime_last_event_type()`;
8. `application_runtime_last_event_source()`;
9. `application_runtime_view_dirty()`.

Historical proof: immediately before decomposition commit `fa75307fb392718a1d10d52770a6a111c97208e7`, these accessors had real `src/kernel.c` integration call-sites. That commit moved mutable integration ownership into `application_runtime_bridge` and removed all consumers while leaving declarations/implementations behind.

Current bridge ownership is explicit: `application_runtime_bridge.c` owns the concrete `application_runtime_t`, reads the status/view/state fields directly, and exposes narrow bridge snapshots/queries.

### OLED layout — one unused wrapper

Remove declaration + implementation of:

10. `oled_ui_layout_config_v1_apply()`.

It was introduced by the Asset boundary as a validate+activate convenience wrapper but has no current consumer. Current ownership uses `oled_ui_layout_config_v1_validate()` plus `oled_ui_layout_config_v1_activate_validated()` directly.

## 3. Explicit retained surface

Do not remove or alter:

- `application_runtime_reset/initialize/start/stop/service/dispatch_event`;
- registry/find/state-name owners;
- `application_runtime_view_get()` and `application_runtime_view_consumed()`;
- `application_runtime_stop_failure_self_test()`;
- any `application_runtime_bridge_*` API;
- `oled_ui_layout_config_v1_validate()`;
- `oled_ui_layout_config_v1_activate_validated()`;
- `binary_frame_encode()` — frozen checked compatibility API explicitly retained by the Binary Framed protocol;
- any wire/protocol/Flash/update/bootloader behavior.

## 4. Frozen source boundary

Authorized target source only:

- `include/kernel/application_runtime.h`;
- `src/kernel/application_runtime.c`;
- `include/kernel/oled_ui_layout.h`;
- `src/kernel/oled_ui_layout_config_v1.c`.

Authorized documentation:

- this plan;
- `docs/TARGET_DEAD_API_CLEANUP_ACCEPTANCE_PLAN.md`;
- `docs/CURRENT_STATE.md`;
- closure-only RDC governance/changelog records after acceptance.

No linker/startup/build-script/bootloader/Host source mutation is authorized.

## 5. Linker/provenance implications

The accepted build uses `-ffunction-sections` and `--gc-sections`; therefore the unused function bodies are expected already to be absent from linked runtime code.

However, `build_firmware.ps1` embeds the exact firmware-only candidate tree through `DEUS_FIRMWARE_SOURCE_TREE_HEX`. A source cleanup therefore intentionally changes raw BIN identity even if executable behavior is otherwise byte-equivalent.

Acceptance must not demand old raw BIN SHA. It must prove fresh candidate identity plus normalized equivalence.

## 6. Gate-2 build/static equivalence

On Windows with the accepted GNU Arm toolchain:

1. preserve the accepted candidate-tree derivation semantics from FDC-05..08: temporary index starts from current published `HEAD`, stages exactly the four authorized target WIP paths, writes one target-source candidate tree, and materializes that tree through `git archive`; the real index remains untouched;
2. materialize the clean baseline from current published `HEAD` through `git archive`; target/linker/startup/build-script bytes are already proven unchanged from the physical `bb99acf111dfa3a78193b4e5d3376fa077defa1e` baseline;
3. fresh-build baseline and candidate with the same accepted GNU Arm toolchain and unchanged build script; baseline build receives physical provenance tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`, candidate build receives the newly written current-HEAD-plus-four-target-path tree;
4. baseline reproduction must be exact: BIN SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`, Flash/SRAM `51972/10968`;
5. candidate must remain exact Flash/SRAM `51972/10968` and within Flash `53248` / SRAM `11264` ceilings;
6. undefined symbols = 0 and forbidden heap/libc drift = 0;
7. all ten deleted APIs must be absent from candidate source and final linked symbol inventory while retained contract owners remain present;
8. global/externally named defined-symbol address inventory must be exact; compiler-generated local labels such as `CSWTCH.N`, `.constprop.N`, `.isra.N` and `.part.N` are not ABI identity and are diagnostic only unless a frozen contract names them;
9. compare only alloc/load/runtime section geometry (`.isr_vector`, `.text`, `.data`, `.bss`, reserved MSP stack/linker boundaries), never raw DWARF/debug section tables or raw debug-enabled ELF/MAP hashes;
10. parse GCC `.su` records as function/frame/kind data, ignoring source line numbers/path text; candidate retained records must equal baseline retained records after removing exactly the ten deleted source functions;
11. locate exactly one embedded 40-byte ASCII physical baseline tree literal in the baseline BIN and exactly one candidate-tree literal at the same offset in the candidate BIN; replacing only that baseline literal with the candidate literal must make the entire expected candidate BIN byte-exact to the actual candidate BIN;
12. persist baseline/candidate BIN, ELF, MAP, relevant `.su`, symbol/section/vector reports and hashes into evidence before any semantic verdict so every downstream assertion is replayable offline;
13. any load-image byte difference outside the single provenance literal invalidates the no-hardware-equivalence path.

## 7. Hardware acceptance rule

Gate 3 is mandatory for RDC-03 even when Gate 2 proves exact whole-BIN equality after substituting only the one 40-byte source-tree provenance literal. Exact equivalence remains the deterministic proof that the source cleanup did not alter executable behavior, but this target-source maintenance boundary will also close with one exact-candidate physical runtime acceptance rather than relying solely on inherited board evidence.

Gate 3 must deploy the exact Gate-2 candidate through the already accepted bootloader/recovery path and prove exact post-Flash identity plus normal runtime USB/UART/OLED/scheduler/IWDG/application behavior. No cable-topology improvisation or unrelated target mutation is authorized.

If whole-BIN substitution equivalence fails for any byte outside the provenance literal, Gate 2 is a product/equivalence failure and Gate 3 must not be used to waive it. The source candidate must first be investigated before any hardware deployment.

Gate 3 is accepted for exact candidate tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b` / application SHA-256 `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`. The signed update committed successfully; USB/application/scheduler/IWDG and UART regression passed. The initial final-readback failure was `HARNESS_APPLICATION_TAIL_ERASE_ORACLE_01`: the bootloader contract lazily erases only application pages touched by DATA, so untouched page 59 is not required to be all `0xFF`. Read-only continuation evidence `stm32_os_target_dead_api_cleanup_gate3_readonly_continuation_v2_20261007_232443.evidence.zip`, SHA-256 `046E4F1F30A69CDAE00B6B9A37B2C02E16A05F7A4F4EBBA56AB1F48E23341AB9`, proves exact candidate application readback, unchanged bootloader/persistence regions, correct final-touched-page erase semantics, runtime health before/after SWD and exact repository/WIP poststate with `FLASH_MUTATION=NONE / RESET=NONE`. Operator physical review is `PHYSICAL_OLED=PASS`.

## 8. Gates

- Gate 0 — exact ten-symbol deletion/retained-surface/equivalence freeze.
- Gate 1 — implementation/static source review.
- Gate 2 — fresh baseline/candidate GNU build, map/symbol/resource and normalized-BIN equivalence.
- Gate 3 — exact-candidate hardware/runtime acceptance plus evidence reconciliation.
- Gate 4 — one normal local acceptance commit.
- Gate 5 — ordinary non-force publication + fresh-fetch clean `0/0`.

## 9. Exit criterion

RDC-03 closes only when exactly ten dead APIs are absent, retained compatibility/bridge owners are unchanged, deterministic build/resource/symbol/whole-BIN provenance-substitution proof passes, exact-candidate hardware/runtime acceptance passes, publication is ordinary/non-force and the final repository poststate is clean.
