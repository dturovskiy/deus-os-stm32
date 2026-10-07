# Deus OS — Firmware Update / Bootloader Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED / POST-PUBLICATION PHYSICAL DEPLOYMENT VERIFIED**

Boundary: FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION

Canonical design: docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md

Canonical wire/image protocol: docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md

Published prerequisite commit: a8f92f83c2ba8917ad183b1a099c9e21199c9463
Published prerequisite tree: 013c1f472eb9404de94befdcc3f1e1e5acfc5831

Published prerequisite Gate-7 evidence:
stm32_os_preboot_resource_gate7_nonforce_publish_v1_20260927_194416.evidence.zip
SHA-256 335013389D8AB6C1B09C4BF720185FED82EC4A0D30A2F3F6237CF8ACCA1DE8CE

## 1. Gate-0 authority

Gate 0 permits documentation, read-only source inspection and temporary acceptance-only feasibility builds.

It forbids product source/linker/startup mutation, application relocation, bootloader installation, target Flash mutation, reset/reconnect acceptance, option-byte/RDP changes, capability publication, commit and push.

## 2. Gate 0A published prerequisite proof

Required exact facts:

- HEAD = origin/main = a8f92f83c2ba8917ad183b1a099c9e21199c9463 before Gate-0 docs WIP;
- published tree = 013c1f472eb9404de94befdcc3f1e1e5acfc5831;
- accepted app source tree = a10182e7d0659b9b161073ad49a8816ecb6e7918;
- BIN SHA = FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2;
- Flash/SRAM = 52908/10920;
- task margins = 304/432;
- MSP margin = 1592;
- capabilities = 0x0000003F, update bit clear.

Gate 0A binds the Gate-7 evidence and independently verifies live Git.

Gate 0A is **PASS** from `stm32_os_bootloader_gate0a_contract_source_audit_v1_20260927_200959.evidence.zip`, SHA-256 `BB3D9210639564D27E0E9FFF57817A10E619991EC3A1960D4D5D29E82E122002`.

Gate 0B is **PASS** from `stm32_os_bootloader_gate0b_linked_feasibility_recomposed_v4_20260927_225243.evidence.zip`, SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`. Exact authoritative measurements: bootloader Flash `5756/8192` (margin `2436`), `.data+.bss` excluding the reserved stack `660/1024`, MSP `1024`, conventional SRAM `1684/2048` (margin `364`), max frame `248/256`, longest linked call path `648/768`, MSP stack margin `376`, undefined symbols `0`, heap absent. Acceptance-only relocated application: Flash `52932/53248` (margin `316`), SRAM `10920/11264` (margin `344`), origin `0x08002000`, end `0x0800EEC4`, metadata pages `0x0800F000/0x0800F400`, persistence unchanged at `0x0800F800/0x0800FC00`. All 57 native processes exited `0`, stderr was empty, repository/index/poststate remained exact, and target I/O/Flash/reset/RDP/commit/push were absent.

Gate 0 is complete. Gate 1 may now perform only the behavior-preserving Host Core ownership split and exact product-source authorization.

## 3. Flash/map invariants

Final published invariants (the historical Gate-0 precondition was the earlier `ORIGIN 0x08000000 / LENGTH 54K` application layout; Gates 2–7 superseded it with the accepted relocated split below):

    bootloader   0x08000000..0x08001FFF   8192 B
    application  0x08002000..0x0800EFFF  53248 B
    metadata A   0x0800F000..0x0800F3FF   1024 B
    metadata B   0x0800F400..0x0800F7FF   1024 B
    persist A    0x0800F800..0x0800FBFF   1024 B
    persist B    0x0800FC00..0x0800FFFF   1024 B

The accepted application linker origin is `0x08002000`, executable length `52K`, with VTOR at the relocated origin. Pages 60/61 are firmware metadata and pages 62/63 remain persistence; neither pair is executable application capacity.

## 4. Source ownership inventory

Evidence enumerates exact current owners for:

- linker;
- startup/vector table;
- SCB_VTOR definition/use;
- Flash erase/program code;
- Asset recovery bundle generation/restoration;
- reset/IWDG/reboot controls;
- USB management device;
- host WinUSB/libusb transport;
- DeusDeviceClient.cs;
- system identity capability bit 6.

Every Asset-era artifact that assumes application reset ownership at 0x08000000 is classified as replaced, historical-only, generalized or unchanged for Bootloader steady state.

## 5. Architecture decision freeze before Gate 0B

The canonical plan now freezes all ten Gate-0A rows. A dedicated docs/read-only decision-freeze audit must prove exact consistency before Gate 0B is authorized:

1. management-IF2 `ENTER_BOOTLOADER` + BKP_DR1/DR2 one-shot token + `SYSRESETREQ`;
2. minimal vendor/bulk USB bootloader on private-test `1209:000D`, WinUSB/libusb, no CDC, Binary Framed envelope `0x04/0x86`;
3. fixed 48-byte header + header HMAC + raw relocated image, with origin `0x08002000` authenticated implicitly and payload bytes verified against the HMAC-authenticated SHA-256 digest;
4. HMAC-SHA-256 with external non-repository 256-bit key and bounded no-SWD-adversary claim;
5. exact `DEUSHDR1 || origin_le32 || header[48]` authenticated-byte definition;
6. product `0x534F4544`, device `0x0410`, origin `0x08002000`, format v1;
7. monotonic uint32 version floor with normal `>` and recovery `>=` semantics;
8. executable pages 8..59 plus firmware metadata A/B pages 60/61; each metadata page is either erased (`0xFFFF`) or has a single final `0xA55A` marker programmed from erased state; old authenticated records remain rollback-floor evidence and are bootable only while their payload digest matches the current application;
9. full-retry in-place update with header authentication before any Flash mutation and commit marker written last;
10. `.data+.bss <=1024` from linker section-bound symbols (excluding `.boot_stack`), MSP 1024 from the dedicated boot-stack symbols, reviewed stack `<=768`, total conventional SRAM `.data+.bss+MSP <=2048`, no heap; GNU `size` aggregate BSS is not an authority for `.data+.bss` because it includes the NOLOAD `.boot_stack` reservation. Stack-oracle frame identity is source `.su` for ordinary linked functions; recognized GCC IPA clones missing a same-name `.su` record may resolve only to the compiler's canonical `.su` clone stem (for example linked `foo.constprop.0` -> `.su` `foo.constprop`). The clone's own balanced final-disassembly prologue frame must equal that canonical `.su` frame. The clone must be reported separately; direct base-function aliasing or suffix stripping without disassembly agreement is forbidden.

Gate 0B cannot begin until that decision-freeze audit passes.

## 6. Forbidden shortcuts

Automatic failure if a proposal:

- uses CRC/hash as authenticity;
- stages firmware in pages 62/63;
- treats a 0x08000000-linked application as relocated-compatible;
- reuses stale Asset recovery scripts after reset ownership changes without reopening them;
- makes CDC the update carrier simply because it exists;
- treats ST-LINK as normal product update;
- adds update transaction orchestration directly to existing DeusDeviceClient.cs;
- publishes capability bit 6 before full acceptance;
- assumes a second full internal application image without reopening layout.

## 7. Gate 0B linked feasibility

Gate 0B uses a temporary uncommitted candidate-bound project and retains all prototype source hashes, toolchain identity and argv.

The linked prototype implements, not stubs:

- reset/vector owner;
- normal/recovery decision;
- selected update transport RX/TX;
- image parser/bounds;
- application erase/program/verify;
- selected integrity/authenticity verification;
- target/version policy;
- structural application validation;
- deterministic handoff;
- interrupted-update recovery/retry path.

Hard acceptance:

    bootloader loadable Flash <= 8192
    bootloader end <= 0x08002000
    undefined symbols = 0
    heap = forbidden

Evidence records text/rodata/data/bss, stack reservation/bound, object/symbol attribution, compiler helpers, libc/runtime imports and exact ELF/MAP/BIN hashes.

Omitting security or transport to fit invalidates the proof.

## 8. Application relocation feasibility

A temporary link experiment must move the accepted application to ORIGIN `0x08002000`, executable LENGTH `52K`, reserve pages 60/61 for firmware metadata, and prove:

- vector table = `0x08002000`;
- loadable end `<= 0x0800F000` and no loadable byte occupies pages 60/61;
- firmware metadata fixed at `0x0800F000` / `0x0800F400`;
- persistence remains fixed at `0x0800F800` / `0x0800FC00`;
- accepted forward resource ceilings retained;
- all absolute Flash assumptions inventoried;
- explicit future VTOR ownership.

This experiment never authorizes target Flash mutation.

## 9. Host decomposition prerequisite

Before update behavior:

- existing public `DeusDeviceSession` remains the single discovery/reconnect lifecycle owner and stays source-unchanged;
- exactly one new internal `DeviceProtocolChannel` owns transport, decoder, request correlation/allocator, queued frames and the single operation gate;
- `DeusRpcClient` owns generic RPC transaction semantics only;
- `AssetTransferClient` owns Asset transaction semantics and transfer IDs only;
- `DeusDeviceClient` remains a public compatibility facade and must not contain a second transport/decoder/request allocator/gate;
- no `FirmwareUpdateClient` or update feature behavior is introduced until this split is accepted;
- existing Core/Transport tests exact-pass before update tests are added.

Gate 1 exact Host Core source boundary is four paths only: modified `DeusDeviceClient.cs` plus new `DeviceProtocolChannel.cs`, `DeusRpcClient.cs`, and `AssetTransferClient.cs`. `DeusDeviceSession.cs`, all transport projects, all existing tests and all firmware/linker/startup/scripts remain source-unchanged.

Gate 1 is **PASS** from `stm32_os_bootloader_gate1_host_ownership_split_acceptance_v3_20260927_233619.evidence.zip`, SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`. Exact acceptance: 22/22 file SHA lock; Core build, Core test-project build and Transport test-project build all `0 warnings / 0 errors`; direct Microsoft Testing Platform/xUnit v3 Core `28/28` and Transport `5/5`; exact pre/post worktree; no firmware/linker/startup/test-source drift; no target/Flash/reset/index/commit/push mutation.

Gate 2 authorizes exactly three product paths: `linker/stm32f103c8.ld`, `src/startup.s`, and `scripts/build_firmware.ps1`. It must produce a clean relocated application at `0x08002000`, `52K`, with vector table at origin, VTOR written before runtime initialization, loadable end `<0x0800F000`, Flash `<=53248`, SRAM `<=11264`, undefined symbols `0`, no heap/libc drift, and persistence addresses unchanged. All existing Host Core Gate-1 bytes remain locked. Gate 2 is build/static only and performs no target Flash/reset.

Gate 2 is **PASS** from `stm32_os_bootloader_gate2_relocated_application_build_acceptance_v2_20260928_145108.evidence.zip`, SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`. Candidate tree `637ea07b10cf84882e19cbb8239f31b7f48856a7`; Flash/SRAM `52932/10920`; end `0x0800EEC4`; initial MSP `0x20005000`; reset vector `0x080020D9`; stack-usage `26/26`, records `249`; undefined `0`; exact poststate. Accepted BIN/ELF/MAP SHA-256 values are `82567F621ED393395810DEB40382EE8477B2975BA1824A407CA9CDDD7B721324`, `DDE555984DA6926CE24C3EAC329E6AB4DDDA4F980E33A05C87258A181036179A`, and `0E075E8DC4B3CD64FF89D0B2119FF228CC4834614B15F5006CF3518508DF15B2`.

Gate-2 postmortem note: the 316-byte Flash margin was sufficient to accept relocation itself but was not proof that the mandatory Gate-3 runtime-entry resident code would fit. The first complete Gate-3 implementation increased linked application load image by 400 bytes. Any future gate that freezes a forward resource ceiling must include the next gate's mandatory resident code in feasibility or reserve an explicit measured budget for it.

Gate 3 may now implement the frozen bootloader/update/security design. Gate-2 relocation/resource invariants remain immutable. Gate 2 is explicitly and narrowly reopened only for the measured compiler-profile update in `scripts/build_firmware.ps1`: `src/kernel/usb_management.c` joins the already-existing `$SizeOptimizedSources` set and therefore changes from `-O2` to `-Os`. `linker/stm32f103c8.ld` and `src/startup.s` remain byte-frozen; origin, 52-KiB application limit, metadata/persistence addresses and `53248/11264` ceilings do not change. This narrow reopen is **PASS** from `stm32_os_bootloader_gate2_reopen_gate3_resource_acceptance_recomposed_v1_20260929_134328.evidence.zip`, SHA-256 `891C3075432F5540E521DABA60E18FD7228E8943725BBF1A92A398D118DD852A`. The accepted reopened build-script SHA-256 is `B6BC4E18F73E24A0574DE695027FFABEBCFC964748196FAB4437745916FFC693`; linker/startup remain byte-frozen. Canonical application Flash/SRAM are `53212/10956` with margins `36/308`; load-image end is `0x0800EFDC`; initial MSP/reset are `0x20005000/0x080020D9`; candidate trees are full `9a9646b67a035d16cc1d164fd4ff9a6a7ab3006d` and firmware `c8212ef6de086da50d3f37465273f78d12c329bb`; BIN/ELF/MAP SHA-256 are `32E225DF6705E5F79ABE433FBA92AFF50353D1802D90B67508667B9C070ECB69`, `1BD4A9E4CC3B8AF99351B9FCD89E513CBD987236C46D1F370347C63DE3CA5136`, and `ABB04D59130CEA2E6ECD8FE28F56D338DC2EFE59E510AD896923B129E299D69D`. Exact poststate passed; no target I/O, commit or push was performed.

Gate-3 implementation acceptance locks the 21 product/test paths enumerated in the parent plan. This 21-path set is the authorized change/source boundary, **not** the candidate-tree membership definition: accepted full/firmware/host tree identities are derived by the accepted Gate-3 v12 candidate construction and must never be reconstructed by starting from an empty index and adding only those authorized paths. A later narrow reopen must carry forward untouched identity domains (notably Host) and recompute only identities actually affected by authorized byte changes. The firmware package generator must pass a deterministic host-only preflight using a fixed temporary image/key/version and an independently derived expected package length/SHA before the real application package oracle is run; PowerShell/C# extension-method assumptions such as instance `.AsSpan()` are not accepted as package-format authority. Application artifact identity is candidate-bound: `BUILD_SOURCE_TREE` / `DEUS_FIRMWARE_SOURCE_TREE_HEX` must equal the current firmware-only candidate tree. Because that 40-hex identity is intentionally embedded in `os.bin`, a later authorized change to a non-application firmware-candidate path (for example the bootloader build observer) is expected to change raw BIN/ELF hashes even when application machine code/layout is otherwise identical. In that case acceptance must prove the current identity value and normalized-BIN equivalence to the older accepted application, not require the older raw BIN SHA. The three added application transport owners (`src/kernel/binary_frame.c`, `src/kernel/binary_rpc.c`, `src/kernel/asset_transfer.c`) are authorized only for the measured shared Binary Framed finalizer composition repair. Gate-2 `linker/stm32f103c8.ld` and `src/startup.s` remain byte-frozen; `scripts/build_firmware.ps1` is reopened only for the single measured `usb_management.c` membership addition to the existing `$SizeOptimizedSources` set; that reopen is accepted by the focused clean-build evidence above. It must prove: application build remains within `53248/11264`; bootloader links within Flash `8192`, `.data+.bss <=1024`, conventional SRAM `<=2048`, MSP `1024`, max frame `<=256`, longest linked stack path `<=768` and MSP margin `>=256`; external key is not present in repository/evidence/logs; package generator independently verifies HMAC domain/origin/header/payload digest; runtime update entry uses frame `0x04/0x86`, destructive flag, exact BKP pair and response-before-reset semantics; bootloader USB is `1209:000D`, IF0, OUT01/IN81 with WinUSB GUID `{F08907B7-BEC4-5FCF-BC4C-B446ED345D87}`; Core/Windows/Linux builds are warnings-as-errors; feature + existing Core/Transport tests pass through direct MTP execution. Host locked restore must run in a harness-owned per-run NuGet package domain and must not inherit package-cache authority from the operator environment; the locked direct `xunit.v3.mtp-v2` package must have a real nuspec in that owned cache before no-restore builds/tests. Gate 3 performs no STM32 target I/O.

Gate 3 is **PASS** from `stm32_os_bootloader_gate3_implementation_acceptance_recomposed_v12_20260929_155027.evidence.zip`, SHA-256 `E016B495CECE75F8BD2A4C120EC0E5506EFF31ED984566007EA6363CC6D35846`. Accepted trees: full `34bee09f7e69e3bf55eee3ac6ca274bd70f137d4`, firmware `8a0d64817de2d0eb9a3b465d3cd57c392259178a`, host `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`. Application Flash/SRAM `53212/10956`; application BIN/ELF/MAP SHA-256 `D799F9FCAE1663ADE59F9783A01499E9FBA0FD954E50A09A62A94C6AA2CBB607`, `0002C39AED7FA405797F056E7DEA31F5D0A63D435C345D5802657D7A39F8D9F4`, `36DD69B8CDEC1AEA3F9E2527BBD5D75E349EFC5488334E4E9D0C2D12171E0BCF`; normalized identity matches the accepted resource BIN. Bootloader resource/stack acceptance: Flash `5896/8192`, `.data+.bss 660/1024`, MSP `1024`, conventional SRAM `1684/2048`, max frame `248/256`, longest linked stack `648/768`, MSP margin `376`. Deterministic package-generator preflight PASS, independent HMAC/package oracle PASS, Core/Windows/Linux Release builds `0 warnings / 0 errors`, direct MTP Core `33/33`, Transport `8/8`, exact poststate PASS, no target I/O/Flash/reset/UART/target USB/index/commit/push mutation.

### Gate 4 — fresh revalidation and immutable recovery campaign

Gate 4 is non-target. The accepted Gate-3 27-path product candidate must remain byte/tree exact; Gate 4 may add only two repository-owned recovery-tooling paths: `scripts/create_bootloader_recovery_bundle.ps1` and `scripts/stm32_bootloader_recovery.ps1`. These tooling paths own a separate recovery-tooling tree and are not part of the application `FirmwareSourceTree` identity.

Gate 4 must fresh-rebuild/recheck the accepted application, bootloader, package generator, static/security assertions and Host Core/Windows/Linux + direct MTP tests. Cross-run application identity is exact accepted product source tree + exact loadable BIN + byte-identical debug-stripped ELF plus fresh symbol/alloc-section/vector/layout/resource proof. Raw debug-enabled ELF and linker MAP SHA values are retained as run-local evidence only because GNU debug/MAP output embeds the unique temporary candidate/build path; they are not cross-run product-identity assertions. It then creates one immutable **operational recovery bundle ZIP** plus one evidence ZIP. The recovery bundle is SHA-bound by evidence but is not embedded in evidence because it contains the key-bearing bootloader image; the raw 32-byte acceptance key is transient only, is never copied into the bundle/evidence/repository/logs, and is deleted before result finalization.

The recovery bundle steady state is exactly pages 0..61 (`0x08000000..0x0800F7FF`, 63488 bytes): bootloader pages 0..7; application pages 8..59, padded with `0xFF` to the full 52-KiB executable region; authenticated metadata A on page 60 containing baseline-v1 `header[48] + tag[32] + marker 0xA55A at offset 0x50`; erased metadata B on page 61. Persistence pages 62/63 are excluded from the recovery image. The bundle must also retain signed `baseline_v1.pkg`, signed `update_v2.pkg`, and a valid-HMAC wrong-target-v2 vector so Gate 5 needs no raw signing key.

Recovery tooling must provide `VALIDATE_ONLY`, `PRESERVE_PERSISTENCE`, and `CLEAN_STATE`. Restore uses ST-LINK/CubeProgrammer only, explicitly erases pages (mass erase forbidden), never performs read-unprotect/RDP or option-byte mutation, verifies device/Flash geometry before mutation, performs full 64-KiB post-readback, requires pages 0..61 byte-exact to the immutable recovery region, and either preserves pages 62/63 byte-exact or proves them erased for clean state. CubeProgrammer connect semantics are explicit: all pre-mutation/read-only probes use and verify `mode=HOTPLUG`; authorized erase/program operations use and verify `mode=NORMAL` so the core is halted deliberately; post-program readback and the final software-reset/release path use and verify `mode=HOTPLUG`. No acceptance-relevant CubeProgrammer invocation may rely on the implicit default connect mode. Gate 5 may not perform its first target mutation until it binds this exact Gate-4 recovery-bundle SHA and validates the bundle locally.

Gate 4 is **PASS** from `stm32_os_bootloader_gate4_fresh_revalidation_recovery_acceptance_v2_20260929_190357.evidence.zip`, SHA-256 `974E482D098973FEE1F56DAA9A767E228A7EA7DBA98D747FCF5E5AD632F905B7`. Evidence integrity is `202/202` internal hashes exact with ZIP CRC clean. The exact Gate-3 full/firmware/host trees were reproduced; recovery-tooling tree is `77b8824fb3b83f12c7b7fa082907fbad3bdddc34`; application BIN remained exact `D799F9FCAE1663ADE59F9783A01499E9FBA0FD954E50A09A62A94C6AA2CBB607` at Flash/SRAM `53212/10956`; same-toolchain stripped Gate-3/Gate-4 ELFs were byte-identical at SHA-256 `698D88EF4C09F2C9A4D33F7B0A333128A725AB4A799BF0893368E9EA34743729`; bootloader resource/stack replay remained `5896`, `660`, `1684`, `48/248/648/376`; Core/Transport tests passed `33/33` and `8/8`. The immutable sensitive recovery ZIP is `stm32_os_bootloader_gate4_immutable_recovery_bundle_v1_20260929_190539.zip`, SHA-256 `00992EC3CF8F31243B322CE2BEF0668CE94889BF47F56DFBEE28EBF14A6AC50E`; recovery region SHA-256 `30642D7947B6C776B1550E8EECA185B8A2BB285A4AFBC3F6E7F01ED0CE6C8088`; baseline/update/wrong-target package SHA-256 values are `FD4AF4D085CDB585C13BC332F729EF86300EED54FF69A42F78F3CC46F16B5387`, `17553B21CA295348394726F943E84245CEA69CC6C51019F2121FB84DF44D9520`, `EE58CE64C33A7B4BFEC3EEC15DB36FE14BE733AC9B63E184579495589BBEE581`. Both source-directory and extracted-ZIP `VALIDATE_ONLY` passed; raw acceptance key was not persisted; Gate-4 target I/O was zero. Gate-4 v1 remains frozen as `HARNESS_ARTIFACT_IDENTITY_SCOPE_FALSE_POSITIVE`.

## 10. Security scope

Minimum claim:

- unauthorized firmware delivered through supported update transport is rejected by device verification;
- corruption is rejected independently of transport integrity;
- wrong target/product is rejected;
- malformed length/origin/vector metadata is rejected.

RDP remains disabled at Gate 0. No resistance to a physical attacker with unrestricted SWD may be claimed. Any debug-lock policy requires a separate explicit recovery/security decision.

## 11. Later hardware matrix to freeze

Before any Gate-5 target mutation, a dedicated **Stage 0 read-only preflight** must PASS. It binds the exact Gate-4 recovery ZIP filename/SHA, extracts it to a unique temporary directory and runs bundle `VALIDATE_ONLY`; verifies Windows ST-LINK/CubeProgrammer device ID `0x410`, factory Flash size `64 KiB`, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF` and option-byte display without mutation; captures a full 64-KiB pre-mutation Flash readback/hash; discovers the Windows CH340/UART adapter without transmitting; proves non-interactive SSH to `deus@macmini`, remote `hostname=macmini`, user `deus`, current OS identity and runtime USB presence `1209:000C`; and records all execution-domain ownership tokens. Stage 0 may not issue erase/program/explicit-reset/start/read-unprotect/option-byte-write or USB update commands. Read-only SWD attachment itself may affect live CPU execution state, so the harness must not claim that the CPU was untouched; it must instead prove that no nonvolatile mutation command was requested and that runtime USB `1209:000C` is present again/continues to be present after the read-only SWD sequence. Only its exact PASS evidence authorizes Stage 1 to invoke recovery `CLEAN_STATE` as the first target mutation. **Stage 0 is formally accepted** by `stm32_os_bootloader_gate5_stage0_acceptance_continuation_v1_20260929_230244.evidence.zip`, SHA-256 `31FBFF9ABD04EF343B0617CA5149F0ED076B338993F54817E42C63526F2F1B1D`: ZIP CRC clean, `37/37` hash-owned evidence entries exact, exact replay of the technical v5 evidence (`52/52`), current runtime USB `1209:000C` present before/after HOTPLUG reads, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF`, current full-Flash SHA-256 `17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`, exact repository poststate, and a single structured authorization record `GATE5_STAGE1_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_EVIDENCE_BIND`. The final colored RESULT block is the last operator-visible block. Stage 1 is therefore authorized only when it exact-binds this evidence SHA and the accepted recovery ZIP v2 SHA. Stage-1 v1 evidence `stm32_os_bootloader_gate5_stage1_clean_baseline_v1_20260930_114012.evidence.zip`, SHA-256 `AA10BD268339EF3251D397E761B72375B133DF93590919306EAA8C45631BF4B1`, proves the first destructive CLEAN operation itself: CRC clean, `58/58` evidence hashes exact, pre-recovery whole Flash exact to Stage 0, explicit pages `0..63` erase, recovery-region program/verify, and post-recovery whole Flash exact `B8D61449ED029E3F93869656C94BB85929509F01311686A7ECD5AD110421E69C` with exact regional hashes. It does **not** yet prove a product normal-boot failure because the recovery tool's final `-rst` reconnected with default `NORMAL` mode, which resets/halt-controls the core. Ten subsequent Mac-mini probes saw neither `1209:000C` nor `1209:000D`. Freeze v1 at `HARNESS_STLINK_CONNECT_MODE_SCOPE` / reset-execution attribution gap. The follow-up reset-attribution run `stm32_os_bootloader_gate5_stage1_reset_attribution_diag_v1_20260930_115228.evidence.zip`, SHA-256 `14A44E5290593C316F8AC320E95A92A20030E61E27757F61D2CBCC667F2F2426`, preserves valid raw evidence (`72/72` internal hashes exact): CLEAN Flash stays exact, core is RUNNING before and after explicit HOTPLUG reset, VTOR is application-owned `0x08002000`, and neither runtime nor bootloader USB enumerates. Its own structured outcome is frozen as `HARNESS_POWERSHELL_STATE_VARIABLE_COLLISION_01` because `$outcome` case-collided with `$script:Outcome`, so only raw/native facts are authoritative. Source attribution then closes the causal gap: bootloader `handoff_app()` masked interrupts with `cpsid i`, application reset/startup never restored PRIMASK, and USB runtime depends on IRQ20; the normal-boot handoff fix therefore removes that mask. Recovery tooling separately changes final reset to explicit `mode=HOTPLUG`. These changes narrowly reopened Gate 3/4 bootloader/recovery identity. That reopen is now accepted without a wholesale Gate-3/4 rerun: Gate-3 narrow repair evidence SHA-256 `58E7FA5576DACC5BE636A2ABAD5B82E1BFDA1A463182957E7B039BB697F50292` accepts repair trees full `1972d7d59057eae8e89fda6b0028ccf07eca5bb1`, firmware `b3b6d4136106f6b6a195fb987a0c66a4b7812842`, host unchanged `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`; Gate-4 narrow repair evidence SHA-256 `62596F4066E6C0FEB476FBC412951123A61FED87731C6B48B473B28429BBCF07` accepts recovery-tooling tree `24434b8f19be22251e4fd6129a8d39f06b101b9c` and immutable repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`. Stage-1 normal-boot repair is accepted from `stm32_os_stage1_repair_apply_accepted_bundle_v3_20260930_155157.evidence.zip`, SHA-256 `3D3A28F0B0C949CCCFB817B7DC726B1B9F63104348EF954ABFEDD08A0F9F0EC5`: ZIP CRC clean, `41/41` hash-owned evidence entries exact, `TARGET_MUTATION_STARTED=True`, `TARGET_MUTATION_COMPLETED=True`, `RUNTIME_RESTORED=True`, post-repair whole Flash SHA-256 `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, recovery region `AB23050CE2A5E6CB5FE3638D8F8FAC299A7707F610116891D6BD349AB8A1E6D0`, bootloader BIN `509C40718A45CD64A078DCDAACE6CE3ECD92039CF3DF876B5E689E23277F73AA`, application BIN `C7EB7201906B5CA8DB8767DE6DB8C48F6397DCA3EB1AEB6C94CC6EFA2E95A4F8`, preserved erased persistence, core running, `VTOR=0x08002000`, `VECTACTIVE=0`, fault magic `0`, advancing kernel ticks, runtime USB `1209:000C` present and bootloader USB `1209:000D` absent. Operator physical acceptance is `OLED=PASS` for `DEUS OS / DESKTOP / READY`. This acceptance explicitly preserves `GATE5_STAGE2_AUTHORIZED_BY_THIS_RESULT=NO`; Stage 2 requires its own explicit authorization.
Gate-5 Stage 2 was then explicitly exercised as the narrow explicit-update-entry / bootloader-INFO case. The first v1 attempt is frozen as `HARNESS_DOTNET_WARNINGS_AS_ERRORS_UNUSED_HELPER_01`: the temporary helper failed build with `CS8321` before update entry or reset. The v2 attempt proved `ENTER_STATUS=Ok` / `ENTER_STATE=Resetting` and real bootloader enumeration `1209:000D`, then failed as `HOST_LINUX_BOOTLOADER_LIBUSB_PERMISSION_01` because non-root `libusb_open` returned `LIBUSB_ERROR_ACCESS (-3)`. The v3 attempt is frozen as `HARNESS_PASSWORDLESS_SUDO_ASSUMPTION_01`: it failed in Mac precheck before target prestate because passwordless sudo was incorrectly assumed.

Stage-2 v4 is **TECHNICAL PASS** from `stm32_os_bootloader_gate5_stage2_explicit_update_entry_v4_20260930_180321.evidence.zip`, SHA-256 `80C670DF059ED37D0A7E2E7BBCF48F55FCC330B21381B68C93FE5DA385DDF8BA`. ZIP CRC is clean and all `68/68` hash-owned evidence entries are exact. Prestate whole Flash is exact accepted Stage-1 repair SHA-256 `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`. Runtime source tree is `b3b6d4136106f6b6a195fb987a0c66a4b7812842`; `ENTER_BOOTLOADER` returned `Ok/Resetting`; runtime `1209:000C` disappeared and bootloader `1209:000D` enumerated. The Linux bootloader node `/dev/bus/usb/001/050` was initially `root:root` mode `664`, readable but not writable by `deus`; one operator-visible privileged action changed only that ephemeral node to owner `deus`, mode `600`. No persistent udev rule or host configuration was created, and the bootloader protocol itself ran unprivileged. Bootloader `INFO` then returned `Ok / RecoveryIdle`, expected offset `0`, version floor `1`, committed version `1`. Whole Flash remained byte-exact while bootloader-idle; `BEGIN`, `AUTHORIZE`, `DATA`, and `END` were not sent. Explicit `mode=HOTPLUG -rst` returned the target to runtime, where `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks advanced `4771 -> 6492`, runtime USB `1209:000C` was present and bootloader USB `1209:000D` absent. Final whole Flash remained exact `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`. Operator physical reconfirmation after this run is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`; Stage 2 is fully accepted. Formal continuation `stm32_os_bootloader_gate5_stage2_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `DA28EC5CD24A31DCE74F996F22D5D774EE1C5E8BD69AA2F7B9F3791608FD3676`, exact-binds the v4 evidence (`68/68`), records the operator physical PASS with zero target I/O/mutation, and authorizes Stage 3 as `GATE5_STAGE3_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Stage-3 invalid-application recovery is **TECHNICAL PASS** from `stm32_os_bootloader_gate5_stage3_invalid_application_recovery_v1_20260930_182233.evidence.zip`, SHA-256 `A507F73C628DA735FCF6A5C0A4E58ABB7A984397970ACB7BE3C64CAE9E4F7D50`; ZIP CRC is clean and all `72/72` hash-owned evidence entries are exact. Exact prestate whole Flash was `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`. The harness erased only application page 59 (`0x0800EC00`), then proved exactly page 59 changed and was fully erased, metadata and persistence pages were untouched, and the invalid whole-Flash SHA-256 was `8195EFFFC3C8FE990BB8677EA6A9BC20B52B75A30B57CC3B68DF1B98A03DF4B4`. After explicit HOTPLUG reset, runtime `1209:000C` was absent and recovery bootloader `1209:000D` present. Unprivileged bootloader `INFO` returned `Ok / RecoveryIdle`, expected offset `0`, version floor `1`, committed version `0`: authenticated metadata retained the version floor while the damaged application was not selected as committed bootable firmware. No USB update `BEGIN/AUTHORIZE/DATA/END` command was sent. Accepted `PRESERVE_PERSISTENCE` recovery then restored the exact repair region and final whole Flash back to `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`; runtime returned with `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks `20939 -> 22724`, runtime `1209:000C` present and bootloader `1209:000D` absent. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage3_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `E587AEC687955E4A5B93B1350D1489E4F6451726A454C09ED091A2F4A3539CA3`, exact-binds the `72/72` Stage-3 technical evidence with zero target I/O/mutation, fully accepts Stage 3, and authorizes Stage 4 as `GATE5_STAGE4_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Stage-4 malformed-header rejection is **TECHNICAL PASS** from `stm32_os_bootloader_gate5_stage4_malformed_header_rejection_v1_20260930_184648.evidence.zip`, SHA-256 `BD0ABADA1523C4E213A4F2B46997A62FE55124208939786543577D2DD434F281`; ZIP CRC is clean and all `68/68` hash-owned evidence entries are exact. The run began from exact accepted whole Flash `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, entered the bootloader through the accepted runtime path, and exercised six structurally malformed `BEGIN` headers: format version 2, non-zero reserved byte, image length too small, image length too large, misaligned image length, and firmware version zero. Every case returned `BadHeader / RecoveryIdle / offset 0 / floor 1 / committed 1`; pre- and post-malformed `INFO` remained `Ok / RecoveryIdle / offset 0 / floor 1 / committed 1`. `AUTHORIZE_HEADER`, `DATA`, and `END` were never sent. Whole Flash remained byte-exact while the bootloader was still active after all malformed cases, proving structural `BEGIN` rejection is non-mutating. The Linux bootloader permission fixture remained bounded to one ephemeral device-node ownership/mode change; protocol execution remained unprivileged and no persistent host permission mutation occurred. Explicit HOTPLUG reset returned the target to runtime with `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks `4806 -> 6555`, runtime `1209:000C` present and bootloader `1209:000D` absent, and final whole Flash exact. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage4_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `AD194B3198D19EF14EA726568DF50207DE7B09EBC4EF2C2FBE2E974974121F5D`, exact-binds the `68/68` Stage-4 technical evidence with zero target I/O/mutation, fully accepts Stage 4, and authorizes Stage 5 as `GATE5_STAGE5_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Stage-5 wrong-target rejection is **TECHNICAL PASS** from `stm32_os_bootloader_gate5_stage5_wrong_target_rejection_v1_20260930_190851.evidence.zip`, SHA-256 `2BAADC493338BA31A26F38F33A0488C7F2B7F092F2ABB8B8F420296D2766043F`; ZIP CRC is clean and all `72/72` hash-owned evidence entries are exact. The run began from exact accepted whole Flash `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, validated exact repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`, and bound its frozen signed `wrong_target_v2.pkg` vector SHA-256 `B82A52EF431017A27C0BAA64D346982A93AB6A7D1240C18C14F11A3EDB2CB5FB` for product `0x534F4544`, target `0x0411`, firmware version `2`; the raw acceptance key was not exported. Runtime `ENTER_BOOTLOADER` returned `Ok / Resetting`; pre-case bootloader `INFO` was `Ok / RecoveryIdle / offset 0 / floor 1 / committed 1`. The structurally valid signed wrong-target `BEGIN` passed as `Ok / HeaderStaged`; `AUTHORIZE_HEADER` then returned exactly `TargetMismatch / HeaderStaged / offset 0 / floor 1 / committed 1`, proving target binding rejection occurs after valid authentication material is supplied and before authorization-side Flash erase. `DATA` and `END` were never sent. Whole Flash remained byte-exact at `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` while bootloader-active after rejection. The Linux bootloader permission fixture remained ephemeral/unprivileged at protocol execution, with no persistent host permission mutation. Explicit HOTPLUG reset restored runtime `1209:000C`, removed bootloader `1209:000D`, restored `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks `4732 -> 6451`, and final whole Flash remained exact. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage5_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `C35A3319B806B21BC42B48EAF84822DF0F721AEE5EF5222BFA45C959F0BB90EE`, exact-binds the `72/72` Stage-5 technical evidence with zero target I/O/mutation, fully accepts Stage 5, and authorizes Stage 6 as `GATE5_STAGE6_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.



Stage-0 v4 is frozen as `HARNESS_STLINK_CONNECT_MODE_SCOPE`: evidence `stm32_os_bootloader_gate5_stage0_readonly_preflight_v4_20260929_212350.evidence.zip`, SHA-256 `B16F46E0A43C2790D80F65DB78AC9E1BC2BA6132942DFB6A9EE08B9BD92C6A55`, passed recovery-v2 local validation, Mac-mini runtime USB before SWD, CH340 discovery, UInt32 sentinel, Device ID/64-KiB Flash/OBR/WRPR/option-byte checks and full 64-KiB readback, then lost runtime USB because CubeProgrammer was invoked in default `NORMAL` mode, which resets and halts the core. No erase/program/explicit-reset/update-entry/UART-TX command was issued, poststate was exact, and no destructive Gate-5 authorization resulted. Stage-0 successors must use `mode=HOTPLUG` on every acceptance CubeProgrammer SWD read call and reject output that does not report `Connect mode: Hot Plug`. Stage-0 v5 (`stm32_os_bootloader_gate5_stage0_readonly_preflight_v5_20260929_213433.evidence.zip`, SHA-256 `EA6414953C72A4A0DF7583B2FE599FD0182EC2E4E4B3BCFF77CAD8FC83F939B1`) satisfies the technical Stage-0 assertions with exact `52/52` evidence hashes, but is not the formal Stage-0 acceptance record because its console finalizer violated `HARNESS-TERMINAL-STRUCTURE-01`: RESULT was uncolored and a green `EVIDENCE_ZIP_SELF_TEST=PASS` was emitted after it. A continuation may inherit the exact v5 technical assertions only after binding that evidence, replaying its hashes/schema, re-verifying the current Flash/runtime state through HOTPLUG, and emitting the final colored RESULT block last. Because v4 itself left the core halted through `NORMAL` attach, the immediate successor may perform one explicitly logged software reset **only as a precondition repair** if runtime USB is absent before the acceptance sequence; this reset does not authorize any erase/program/update-entry operation. After runtime returns, the full 64-KiB Flash SHA must equal the frozen v4 pre-mutation SHA-256 `17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`, including erased metadata/persistence region SHA-256 `D0FF1B294B5288D1AE1421EADF5B2D38A8752B76D472FF30BED9028E25B1C5B8`, before Stage 0 may PASS.

Before any Gate-5 target mutation, bind the hardware run to the current wiring contract in
`docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` and the target pin map frozen in
`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md` §7.1. At minimum the harness
must record:

- USB execution domain = Mac-mini Ubuntu, native USB on PA11/PA12, target normally powered by that USB VBUS;
- UART execution domain = Windows, CH340 on USART1 PA9/PA10 at 115200 8N1, adapter VCC disconnected;
- ST-LINK execution domain = Windows, SWD on PA13/PA14 with shared ground, ST-LINK 3.3 V disconnected during USB-powered operation;
- OLED/I2C1 remains PB6/PB7 and is not bootloader-owned;
- any cable/power/pin ownership change requires topology update and primitive re-proof before composed hardware acceptance.

The later boundary must cover at least:

- normal boot of accepted authenticated application;
- explicit update entry;
- invalid application -> recovery;
- malformed header;
- wrong target;
- bad digest;
- bad signature/authenticator;
- rollback/version rejection when applicable;
- deterministic reset/power interruption across erase/program boundaries;
- successful retry after interruption;
- persistence pages byte-identical;
- bootloader pages unchanged by application update;
- exact application pages after success;
- deterministic handoff and application VTOR;
- Windows/Linux primary host path as applicable;
- capability bit 6 activation only after complete acceptance.

## 11.1 Gate-5 closure record

The Gate-5 hardware matrix is complete and accepted. The minimum matrix in §11 is satisfied by the accepted Stage-1 through Stage-10 campaign:

- normal boot / deterministic handoff / application VTOR: accepted Stage-1 repair and retained runtime proofs;
- explicit update entry: Stage 2;
- invalid application -> recovery: Stage 3;
- malformed header: Stage 4;
- wrong target: Stage 5;
- bad digest and exact immediate DATA retry behavior: Stage 6;
- bad authenticator: Stage 7;
- rollback/version rejection: Stage 8;
- deterministic reset interruption after AUTHORIZE and after first application DATA, plus full retry from zero: Stage 9;
- physical Mac-mini native-USB/VBUS interruption after first DATA ACK, cold-power recovery, full retry from zero and exact successful-update Flash oracle: Stage 10;
- persistence pages byte-identical, bootloader pages unchanged, exact application bytes after success and metadata commit ordering: Stage 9/10 exact whole-Flash oracles;
- applicable primary hardware path: Linux/libusb on the frozen Mac-mini USB topology. A Windows target-USB run is not required without explicit rewiring/re-proof because §11 and the topology contract bind the target USB cable to the Mac mini.

Final acceptance anchors:

- Stage-6 technical v4 SHA-256 `0C5FD70A063F6E4BB366ED85A45DD4AB060B8B645C5983AF7D3E0E486C2703DA`; continuation `D726C04A812BDAB3262E7D352A146481E627894586ADA89A00DA026614175717`;
- Stage-7 technical SHA-256 `43DF41301E005D6E23DEE7C71D3076F97797AF3419232C46A57F3FAA9F916515`; continuation `EFA420DA03CB0300937B1DBC99C73CBBC66B7627577E82E92AC3F1252B5D7CA9`;
- Stage-8 technical v2 SHA-256 `D2006CEE054A85ECA59B96288B3CE25AFCF5BF97B5611D994CF8F7F06B3DB61F`; continuation `57761DE3C23839566983172E403884F85B1A583AB7C5C9ADC512E6096F22B921`;
- Stage-9 technical SHA-256 `2775E2565EA10BCEF8A72EDEE052D698A515B347C6E177D7ABB972D7F36C1EAB`; continuation `C0240BA36BD79C5DB8413B50B4885A6ADBBE6FB69BF2907A75153CEE89D333EB`;
- post-Stage-9 gap-review SHA-256 `64894D6FF012602DB5A5A06D09797574D26D170BD298C93E23E13D43B18CE6B2`;
- Stage-10 technical SHA-256 `24FA0C42644B7E87F88AAD0B775547C7272A591A366D11A4903274C3F47D6CA3`; continuation `E6EB019B021AB95DB16CA3B95E1C3F7C2C7F44D862ABF2711F6B42089F35D560`;
- historical Stage-10 restored pre-publication whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`;
- post-publication deployment evidence SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`;
- current deployed whole-Flash SHA-256 `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`.

The post-Stage-10 architecture/reliability audit identified two host-side defects and no causally proven second firmware-side response-loss mechanism. The accepted host repair centralizes explicit stale request-ID filtering for timed-out firmware requests while keeping unknown request-ID mismatches fatal, and maps Windows WinUSB pipe timeout errors 121/1460 to `HostErrorKind.Timeout` without changing Open/setup classifications. Validation SHA-256 `61D8AEBCA99E712D893474376CA4FEDD63D17D11021E45C0F7701621C712B108` passed Core `39/39`, Transport `12/12`, all Release builds and zero target/remote/Git mutation; formal repair continuation SHA-256 is `253026C36C346C579AD5C1CE0FD528A7F7943A07CC21D792171C14C3350797FD`.

Gate-5 hardware and technical reliability acceptance are therefore complete. Gate 6 activated capability bit 6 (`SYSTEM_IDENTITY_CAPABILITIES = 0x0000007F`) after documentation reconciliation. Gate-6 validation evidence SHA-256 `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7` has ZIP CRC clean, `92/92` manifest verification, firmware tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, host tree `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, Core `41/41`, Transport `12/12`, all Core/Windows/Linux Release builds and zero target/remote/real-index/ref mutation. Gate 6 is accepted by commit `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`, tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`; local acceptance continuation SHA-256 is `806D999F1F9677A7E3A3C9D6E0D030079612978561C18A69167A0890A2EBC1CC`. Gate 7 then published the same commit by ordinary non-force fast-forward; publication evidence SHA-256 `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95` proves post-push `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

The zero-target publication/bench distinction is now closed by post-publication deployment evidence `stm32_os_published_deploy_adjudicated_retry_v7_20261003_141252.evidence.zip`, SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`. It exact-binds the published package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, starts from the historical Stage-10 baseline `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`, verifies bootloader `INFO=Ok/RecoveryIdle` at floor/committed `1/1`, receives exact update completion `Ok/Committed` at floor/committed `2/2`, then independently reads back application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, metadata B version `2` / marker `0xA55A` / matching digest, unchanged bootloader and persistence regions, and post-deploy whole-Flash SHA-256 `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`. Runtime verification then proves source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, runtime and HELLO capability masks `0x0000007F`, `PONG`, health OK, and no rollback. At the time of this accepted deployment proof, the board was physically aligned with the published v2 product identity. This is historical scoped evidence; the current bench/source identity is owned by `CURRENT_STATE.md` and was later advanced by published FDC-05..08 hardening.

## 12. Gate 0 evidence

Gate 0A evidence includes exact Git state, Gate-7 binding, map assertions, source-owner inventory, open-decision matrix, host-decomposition trigger proof and zero target/build/product mutation.

Gate 0B additionally includes prototype sources/hashes, toolchain/argv, ELF/MAP/BIN, resource report, symbol/object attribution, relocation experiment and the final selected architecture matrix.

## 13. Terminal outcomes

PASS only after Gate 0A + Gate 0B.

BLOCKED_8K if the minimum safe composition exceeds the frozen bootloader region.
BLOCKED_SECURITY_CONTRACT if trust/authenticity/version semantics are not frozen.
BLOCKED_STORAGE_MODEL if the design requires staging the frozen map cannot supply.
BLOCKED_HOST_OWNERSHIP if update orchestration would be added without the required host split.

No blocked outcome authorizes weakening the missing responsibility.

## 14. Post-publication `FDC-08` acceptance addendum

This addendum is a forward robustness closure contract and does not alter the historical Gates 0–7 acceptance result.

`FDC-08` is accepted only when all of the following are proven:

- **authenticated vector span:** reset vector is rejected if its handler address is below `APP_BASE`, at/above `APP_BASE + image_length`, non-Thumb or otherwise structurally invalid; tests include a signed/authorized shorter-image case whose vector points into stale bytes beyond the authenticated image span;
- **bootloader clock bound:** HSE-ready, PLL-ready and system-clock-switch waits have explicit finite bounds and a deterministic recovery/fail state; no fallthrough to update/boot with an invalid clock configuration;
- **runtime clock bound:** the normal application has equivalent finite startup bounds and a deterministic failure disposition that does not claim normal health;
- **UART TX bound:** a stuck/not-ready USART TX path cannot spin forever; the chosen degraded/error behavior is explicit and does not silently break watchdog/liveness ownership;
- **update-entry reset deadline:** `ENTER_BOOTLOADER` response/reset completion is controlled by a measured elapsed-time/deadline rule. Acceptance demonstrates both normal response-TX completion reset and fallback reset when TX completion is not observed;
- **non-DATA ambiguity:** host tests cover INFO/BEGIN/AUTHORIZE/END timeout or response loss and prove the frozen adjudication/restart behavior. Unknown state must fail closed; no non-idempotent blind retry is accepted;
- **resource invariants:** bootloader <= 8192 bytes, bootloader SRAM/stack ceilings preserved, application Flash/SRAM/stack ceilings preserved, undefined/heap policy clean;
- **ownership invariants:** bootloader pages 0..7, executable app pages 8..59, metadata pages 60..61 and persistence pages 62..63 remain exact; persistence is not used as update scratch;
- **security invariants:** HMAC authenticity, product/target binding, rollback floor, digest validation and commit-marker-last semantics remain intact;
- **hardware acceptance:** physically meaningful oscillator/reset/update-entry failure/recovery paths are exercised or, where a fault cannot be induced safely on the current bench, a dedicated deterministic injection mechanism is accepted without weakening production behavior.

Final `FDC-08` evidence must also retain successful normal boot, recovery entry, signed update, runtime `PONG`/health and exact whole-Flash ownership/readback checks.
