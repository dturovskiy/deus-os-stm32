# Roadmap

Status: **CANONICAL FORWARD SEQUENCING — CURRENT PROJECT STATE IS OWNED BY `docs/CURRENT_STATE.md`**

The roadmap defines ordering and prerequisites. It does not independently define which boundary is active.

<!-- BEGIN STM32_OS_ROADMAP_CHECKPOINT_2026_09_13 -->
## Published boundary chronology and forward order

### Native USB Device core foundation — PUBLISHED

- [x] commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`;
- [x] tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`;
- [x] accepted firmware `43812` bytes / `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

### USB CDC ACM diagnostic/command console — PUBLISHED

- [x] commit `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`;
- [x] tree `bbc6b24e28279075c41410e8abdace89c4b805b7`;
- [x] accepted firmware `58548` bytes / `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- [x] Windows `usbser` CDC, physical reconnect, automatic software-reset/post-IWDG attach and retained regressions accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

### Transport-neutral shell/RPC foundation — PUBLISHED

- [x] commit `0c33304d2db86e54d715905393f49147bb6dd2ea`;
- [x] tree `19ac9b95caca09e991842f6f9963864934b0a334`;
- [x] accepted firmware `37196` bytes / `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- [x] static allocation-free 32-method command service;
- [x] UART/CDC shared semantics, parser/origin isolation and retained hardware regressions accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0` before the next boundary.

### Binary framed transport foundation — PUBLISHED

- [x] commit `2fde9025a51021511e73a76b561f7983ca655e2f`;
- [x] tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- [x] accepted firmware `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- [x] text/binary CDC coexistence, stable RPC IDs, CRC, request correlation and destructive authorization accepted;
- [x] malformed/pressure/reconnect/IWDG recovery hardware accepted;
- [x] ordinary non-force publication complete;
- [x] local/remote ahead-behind `0/0`.

Canonical protocol/design/acceptance:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`

### Published architecture foundation — OS application and UI model

Boundary:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

This documentation-only architecture boundary is fully accepted and published at commit `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`, tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`. The accepted architecture freezes:

- Deus OS product role as an independently operating embedded runtime;
- initial static application registry/lifecycle model;
- boot splash -> desktop/home -> application view lifecycle;
- real SYSTEM/USB/NETWORK indicator semantics;
- initial uptime `HH:MM` status time;
- firmware versus Control Panel responsibility split;
- target apps versus host plugins versus target resource packages;
- deferral of arbitrary native ARM loading, filesystem and update/bootloader work.

Canonical design/acceptance/gap review:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`
`docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

Foundation completeness constraints retained:

- scheduler event bits stay an internal wake primitive; `APPLICATION_RUNTIME_FOUNDATION` defines a separate bounded semantic app-event/service contract;
- host tooling must gain explicit firmware/build/platform/service/application identity/capability discovery rather than treating USB VID/PID or protocol flags as the whole OS identity;
- Flash-resident settings/packages require a bounded versioned persistence contract with integrity, atomic commit/recovery and wear policy;
- bounded previous-boot crash/reset retention and structured observability remain later independently accepted work;
- CRC/destructive-intent flags are not authentication; network mutation and executable Flash update require explicit security/trust review;
- portability preserves separation of arch/platform/drivers from kernel/services/apps/UI/protocol semantics without a speculative heavyweight HAL.

### Published implementation boundary — Boot / desktop UI foundation

`BOOT_DESKTOP_UI_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**.

This slice is intentionally smaller than the application runtime. It implements exactly `BOOT_SPLASH -> DESKTOP_HOME`, a nonblocking 1000 ms minimum visible splash dwell, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK status semantics, monotonic uptime `HH:MM`, redraw only on visible semantic change, and no steady-state SSD1306 reinitialization/display-off during routine refresh. Gate 2/3 accepted tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; Gate 4 is `PHYSICAL_OLED=PASS`.

Canonical design/acceptance:
`docs/BOOT_DESKTOP_UI_PLAN.md`
`docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`

### Published implementation boundary — OLED dirty-region optimization

`OLED_DIRTY_REGION_OPTIMIZATION` — **GATES 0–7 ACCEPTED / PUBLISHED `39690c9ef103cbcf93272df8bad0359a934b7dc1`**.

Accepted candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`, BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`, Flash `44560 / 65536`, SRAM `9848 / 20480`. Hardware accepted exact dirty-region transfers: historical full semantic refresh `572` payload bytes / `36` writes, clean `0`, one-byte narrow `9`, minute `11`, USB indicator `11`. Task0/task1 post-diagnostic margins remain `328 / 424` bytes. Gate 4 is `PHYSICAL_OLED=PASS`.

Evidence: Gate 2 `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`; Gate 3 `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`; Gate 4 `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`.

Publication commit/tree: `39690c9ef103cbcf93272df8bad0359a934b7dc1` / `f195fac5ce733c36a1d955e0fbe687ee6c83b605`; ordinary non-force publication complete; final ahead/behind `0/0`. Gate 6 evidence `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

Canonical design/acceptance/backlog:
`docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`
`docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`
`docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

### Published implementation boundary — Application runtime foundation

`APPLICATION_RUNTIME_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `25752fba557b1a1b518265a93bde05d3a6a3f9ad`**.

Accepted candidate: tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`; BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; ELF `77836` bytes / `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`; MAP `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`; Flash `48604/65536`; SRAM `10032/20480`; named runtime static state `184` bytes. Hardware accepted text/binary lifecycle, idempotent/invalid-start behavior, minute semantic-event delivery without rerender, CDC/UART/binary pressure `128/128`, power-cycle USB recovery, IWDG recovery and final exact Flash readback. Minimum observed task0/task1 margins after full application/binary activity are `272/424` bytes. Gate 4 is `PHYSICAL_OLED=PASS`. Evidence: Gate 2 `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`; Gate 3 `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; Gate 4 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`. Publication commit/tree `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`; Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`.

Canonical design/acceptance:
`docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`
`docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`

### Published boundary — Kernel composition-root decomposition

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — **GATES 0–7 ACCEPTED / PUBLISHED `fa75307fb392718a1d10d52770a6a111c97208e7`**.

The accepted decomposition reduces `src/kernel.c` from `5100` to `3405` lines (-33.235%). `application_runtime_bridge` owns mutable application-runtime integration state/event snapshot/view adaptation; `application_commands` owns `rpcinfo/applist/appstart/appstop`; `scheduler_diagnostics` owns diagnostic orchestration. Root-private production scheduler observability remains in the composition root. No universal `kernel_context_t`, service locator, hidden extracted-state extern, heap, new task/SVC/queue/mutex/generic timer/DMA/persistence machinery or dependency cycle was introduced.

Accepted candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` bytes / `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536`, SRAM `10032/20480`, final task margins `384/424`. Gate 2 evidence `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; Gate 3 evidence `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; Gate 4 `PHYSICAL_OLED=PASS`.

Canonical decision/design/acceptance:
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`
`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`

### Published implementation boundary — USB management device foundation

`USB_MANAGEMENT_DEVICE_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `1f88083843c6aae9fd228ad2d677f9252b889a11`**.

Frozen topology is hardware-accepted on candidate tree `46841b52d351277deb134a6f4709619087b477af`, BIN `50172` / `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`: composite private-test `1209:000C` / `Deus OS Device`; CDC interfaces 0–1 retained as secondary diagnostics; vendor interface 2 automatically bound to inbox WinUSB over EP4 OUT/IN bulk64; Microsoft OS 2.0 first-configuration subset selector `0`; stable device-interface GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`; binary RPC v1 / command-service v2 / 35 methods unchanged. Gate 3 composite evidence/log are `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6` / `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`; WinUSB `128/128`, UART `32/32`, management drops `0/0`, task margins `448/424`, physical reconnect and IWDG recovery PASS, final Flash exact. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`. Canonical design/acceptance: `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md` and `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.

### Published implementation boundary — Host control application foundation

`HOST_CONTROL_APPLICATION_FOUNDATION` is accepted through Gate 7 and published at `e0f49f168542fa1cf49bca451e01b0c077aa8d18`, tree `42c77f2cf3d7e9f7f5c1ff24d9d437f61a397be6`. Final candidate: firmware tree `b895955f7738aceb6fca0272d510cc433378c6ab`, host tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`, BIN `50652` / `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F`. Windows WinUSB CLI/hardware, Windows Avalonia Desktop and Ubuntu 26.04.1/libusb hardware runtime all pass. Linux claims IF2 only and leaves CDC IF0/1 on `cdc_acm`; same-port reconnect passes with stable bus+port locator; 128 unique pings pass; final management/CDC drops are zero and final Flash is exact. Canonical design/acceptance: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` and `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`.
### Published implementation boundary — Asset / Configuration transfer foundation

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `562e786ffa734da055c23144ec4256bc8961bbaf`**.

Accepted publication-activation candidate: firmware tree
`12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, host tree
`136687e80c42bd8104ad6c37fbbccb915b60fd08`, BIN SHA-256
`7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`,
Flash/SRAM `54268/11944`, system capabilities `0x0000003F`. Transfer v1,
persistent A/B recovery, deterministic fault matrix, response-loss idempotency,
corruption fallback, physical VBUS retention and production `PublishedOnly` host access
are accepted. Final accepted persistence wear is `46/64`; broader generic
storage/filesystem/package infrastructure remains outside this boundary.

Canonical design/acceptance:
`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`
`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

### Published prerequisite boundary — pre-Bootloader resource / architecture recovery

`PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — **GATES 0–7 ACCEPTED / PUBLISHED `a8f92f83c2ba8917ad183b1a099c9e21199c9463`**.

This behavior-preserving boundary recovered explicit target Flash/SRAM margin and
resolved the measured scheduler-stack ownership waste without widening product scope.
The accepted candidate is tree `a10182e7d0659b9b161073ad49a8816ecb6e7918`, BIN
`52908` / `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`,
Flash/SRAM `52908/10920`. Relative to the published Asset image this recovers `1360`
Flash bytes and `1024` static-SRAM bytes; accepted runtime margins are task0/task1
`304/432` and MSP `1592`. The exact six-file recovery remains behavior-preserving:
`kernel.c -> -Os`, duplicate scheduler default-stack ownership removed, diagnostics
borrow existing production storage, no USB ring cuts or ABI/linker/update changes.
The boundary is published. Bootloader implementation is still forbidden until
`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` Gate 0 freezes and proves its own contract.

Canonical design/acceptance:
`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md`
`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md`

Implementation order:

1. `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — Gates 0–7 accepted / published `fa75307fb392718a1d10d52770a6a111c97208e7`;
2. `USB_MANAGEMENT_DEVICE_FOUNDATION` — Gates 0–7 accepted / published `1f88083843c6aae9fd228ad2d677f9252b889a11`;
3. `HOST_CONTROL_APPLICATION_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `e0f49f168542fa1cf49bca451e01b0c077aa8d18`**;
4. `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` — **docs-only shared contract accepted**;
5. `OLED_UI_LAYOUT_CONFIG_V1` — **promoted/frozen concrete consumer**;
6. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `562e786ffa734da055c23144ec4256bc8961bbaf`**;
7. `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — **GATES 0–7 ACCEPTED / PUBLISHED `a8f92f83c2ba8917ad183b1a099c9e21199c9463`**;
8. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `27fb10288ef45dcc9292287603e5ab8a26bf1fcb` / TREE `0eca476d84eb1f06b633a7780b7883a3fadd8adb` / PHYSICAL DEPLOYMENT VERIFIED / CAPABILITY `0x0000007F`**; Gate 0B feasibility SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`; Gate 1 Host split SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`; Gate 2 relocated application SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`; base Gate 3 acceptance SHA-256 `E016B495CECE75F8BD2A4C120EC0E5506EFF31ED984566007EA6363CC6D35846`; narrow repair evidence Gate 3 `58E7FA5576DACC5BE636A2ABAD5B82E1BFDA1A463182957E7B039BB697F50292`, Gate 4 `62596F4066E6C0FEB476FBC412951123A61FED87731C6B48B473B28429BBCF07`; current repair trees full `1972d7d59057eae8e89fda6b0028ccf07eca5bb1`, firmware `b3b6d4136106f6b6a195fb987a0c66a4b7812842`, host `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`, recovery tooling `24434b8f19be22251e4fd6129a8d39f06b101b9c`; immutable repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`; Stage-1 repair evidence SHA-256 `3D3A28F0B0C949CCCFB817B7DC726B1B9F63104348EF954ABFEDD08A0F9F0EC5`; Stage-2 technical evidence SHA-256 `80C670DF059ED37D0A7E2E7BBCF48F55FCC330B21381B68C93FE5DA385DDF8BA`; Stage-2 acceptance continuation SHA-256 `DA28EC5CD24A31DCE74F996F22D5D774EE1C5E8BD69AA2F7B9F3791608FD3676`; Stage-3 technical evidence SHA-256 `A507F73C628DA735FCF6A5C0A4E58ABB7A984397970ACB7BE3C64CAE9E4F7D50`; Stage-3 acceptance continuation SHA-256 `E587AEC687955E4A5B93B1350D1489E4F6451726A454C09ED091A2F4A3539CA3`; Stage-4 technical evidence SHA-256 `BD0ABADA1523C4E213A4F2B46997A62FE55124208939786543577D2DD434F281`; Stage-4 acceptance continuation SHA-256 `AD194B3198D19EF14EA726568DF50207DE7B09EBC4EF2C2FBE2E974974121F5D`; Stage-5 technical evidence SHA-256 `2BAADC493338BA31A26F38F33A0488C7F2B7F092F2ABB8B8F420296D2766043F`; Stage-5 acceptance continuation SHA-256 `C35A3319B806B21BC42B48EAF84822DF0F721AEE5EF5222BFA45C959F0BB90EE`; Stage-6 technical/continuation `0C5FD70A063F6E4BB366ED85A45DD4AB060B8B645C5983AF7D3E0E486C2703DA` / `D726C04A812BDAB3262E7D352A146481E627894586ADA89A00DA026614175717`; Stage-7 `43DF41301E005D6E23DEE7C71D3076F97797AF3419232C46A57F3FAA9F916515` / `EFA420DA03CB0300937B1DBC99C73CBBC66B7627577E82E92AC3F1252B5D7CA9`; Stage-8 v2 `D2006CEE054A85ECA59B96288B3CE25AFCF5BF97B5611D994CF8F7F06B3DB61F` / `57761DE3C23839566983172E403884F85B1A583AB7C5C9ADC512E6096F22B921`; Stage-9 `2775E2565EA10BCEF8A72EDEE052D698A515B347C6E177D7ABB972D7F36C1EAB` / `C0240BA36BD79C5DB8413B50B4885A6ADBBE6FB69BF2907A75153CEE89D333EB`; Stage-10 physical-VBUS `24FA0C42644B7E87F88AAD0B775547C7272A591A366D11A4903274C3F47D6CA3` / `E6EB019B021AB95DB16CA3B95E1C3F7C2C7F44D862ABF2711F6B42089F35D560`; host reliability validation/continuation `61D8AEBCA99E712D893474376CA4FEDD63D17D11021E45C0F7701621C712B108` / `253026C36C346C579AD5C1CE0FD528A7F7943A07CC21D792171C14C3350797FD`; Gate-6 validation `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7`, firmware/host trees `8323c68c931894441ae4db9138ba3838f35bb8b6` / `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, firmware BIN `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`; Gate-6 local continuation `806D999F1F9677A7E3A3C9D6E0D030079612978561C18A69167A0890A2EBC1CC`; Gate-7 publication evidence `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`; post-publication deployment evidence `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`, deployed whole-Flash `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`, runtime source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, capability `0x0000007F`, metadata B committed version `2`, rollback not attempted;
9. networking/service/security extensions.

Deferred optimization/robustness items are trigger-driven, not new immediate roadmap boundaries: I2C IRQ/DMA only after measured bus pressure; scheduler ready-set acceleration only after materially larger task count and measured overhead; CRC acceleration only for measured streaming cost; bounded minimal-copy/backpressure for WinUSB and transfer paths; tickless only for a real power requirement; structured binary event tracing rather than printf-heavy logging; bounded storage/block-device layers when a consumer exists.

### Phase 4 ordering constraint

Generic timer callbacks remain deferred until a real consumer requires them.
Message queues and synchronization remain deferred until a real cross-task
ownership boundary requires them. Runtime statistics remain a later
observability slice. The scheduler already has internal PRIMASK-protected
critical sections; that does not imply a public mutex/semaphore API. Heap,
filesystem, generic DMA framework, RTC/wall-clock service, MPU isolation and
general power-management framework are also consumer-driven, not missing
prerequisites. The 128x32 OLED geometry/gfx baseline remains frozen. `BOOT_DESKTOP_UI_FOUNDATION` established runtime-owned content/status semantics, and the accepted `OLED_DIRTY_REGION_OPTIMIZATION` changes only differential rendering/transfer behavior without changing that geometry.

## Phase 0 - Boot baseline

- [x] ARM GNU toolchain
- [x] Linker script
- [x] Vector table
- [x] Reset_Handler
- [x] `.data` initialization
- [x] `.bss` initialization
- [x] Minimal `kernel_main()`
- [x] ELF/BIN generation
- [x] Flash first kernel image
- [x] Prove execution on hardware

## Phase 1 - Core hardware

- [x] GPIO driver
- [x] PC13 status LED
- [x] Clock configuration
- [x] SysTick
- [x] Monotonic kernel time

## Phase 2 - Diagnostics

- [x] HardFault diagnostics
- [x] UART logging
- [x] I2C driver
- [x] SSD1306 driver
- [x] OLED kernel console

## Phase 3 - Scheduler

- [x] Task control block foundation
- [x] Separate static task stacks + synthetic initial frames
- [x] Cooperative scheduling
- [x] PendSV context switch
- [x] Command-gated SysTick preemption proof
- [x] Task stack budget / high-water validation for the accepted command-gated frozen OLED workload
- [x] Normal boot task migration
- [x] `sleep()`
- [x] Priorities

## Phase 4 - Kernel services

The unchecked items below are consumer-driven/deferred rather than active prerequisites.

- [ ] Generic timers — deferred until a concrete consumer requires them
- [ ] Message queues — deferred until a real cross-task consumer requires them
- [ ] Synchronization primitives — deferred until a real concurrency requirement exists
- [x] Watchdog integration
- [ ] Runtime statistics — deferred until observability requirements justify them

## Mandatory foundational debt closure before new service/Web/network features

The 2026-10-03 architecture/code audit promoted ten residual obligations into a mandatory pre-feature closure program. These are no longer optional trigger-driven backlog items. They must be closed through small independently accepted slices before `HOST_MANAGEMENT_SERVICE_FOUNDATION` (or an equivalent Web/service/network boundary) may begin.

### Slice A — Host long-lived-session hardening

- [x] `FDC-01` — **CLOSED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`:** generic RPC timeout/cancellation correlation is accepted; delayed abandoned RPC streams are drained through terminal `RPC_END`/`PROTOCOL_ERROR`, unknown correlation remains fatal, unresolved abandonment fails closed at request-ID wrap, firmware DATA retains one exact retry, and INFO/BEGIN/AUTHORIZE/END retain no blind retry. Gate-2 evidence SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`, Core `51/51`, Release build PASS, target I/O zero; Gate-5 ordinary non-force publication/fresh-fetch closure PASS.
- [x] `FDC-02` — **CLOSED / PUBLISHED `42245d9d71504482fb189d8351ecce7049542145` (`HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING`):** one per-session asynchronous notification chain removes subscriber execution from `_operationGate`; Gate-2 evidence SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`, Core `58/58`, Core/Desktop Release builds PASS; ordinary non-force publication/fresh-fetch closure PASS.
- [x] `FDC-03` — **CLOSED / PUBLISHED `0d9adfd8d0ed11478194e2268ede3c57379c8294` (`HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING`):** explicit service-v1 catalog/facade accepted: 5 ReadOnly, 3 Control, 0 Destructive; no raw numeric RPC/flags, bootloader/update or unlisted routing. Gate-2 evidence SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`, Core `66/66`, Core/CLI/Desktop Release PASS; ordinary non-force publication/fresh-fetch closure PASS.
- [x] `FDC-04` — **CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d` (`HOST_TYPED_MANAGEMENT_MODELS_HARDENING`):** bounded Core Ping/Health/app-control models accepted; service/Desktop raw management output removed, CLI raw output confined to excluded `rpcinfo`; Gate-2 evidence SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`, Core `78/78`, Core/CLI/Desktop Release PASS; ordinary non-force publication/fresh-fetch closure PASS.
- [x] `FDC-09` — **CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73` (`HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING`):** per-instance latched bounded cancellation/drain ownership is accepted on shared WinUSB/libusb runtime+bootloader transports; deterministic Transport `24/24` + Core `78/78`, Linux real-platform and Windows WinUSB cancel/disconnect/reopen proofs are accepted; fresh fetch closed at `HEAD == origin/main == FETCH_HEAD == b88a9eee43095665326787cc0345822218c1ba73`. Canonical slice: `HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING_PLAN.md` + `_ACCEPTANCE_PLAN.md`.

### Slice B — Target architecture/lifecycle cleanup

- [x] `FDC-05` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`:** clock/USART1/I2C1/PC13 ownership convergence, build/resource/ABI and hardware equivalence accepted with no framework/god-context substitution.
- [x] `FDC-06` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`:** `system_service_state` is the upstream authority for application/OLED health/USB/network/time semantics; event/no-rerender and physical OLED behavior accepted.
- [x] `FDC-07` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`:** failed stop preserves unresolved owner/`active_id`, marks `FAILED`, increments one fault and starts no replacement; synthetic + hardware lifecycle proof accepted.

### Slice C — Target/update robustness closure

- [x] `FDC-08` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`:** bounded clock/UART waits, authenticated image-span vectors, wrap-safe update-entry deadline and non-DATA adjudication accepted. Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788`; hardware evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`.

### Slice D — Documentation/source-of-truth closure

- [x] `FDC-10` — **CLOSED / PUBLISHED `9c02e27f50349c9a0240ab24d34720580c3f3269`:** repo-wide inventory, stale/current claim reconciliation, open-checkbox classification, documentation inventory, artifact/security hygiene and source-of-truth synchronization accepted; the ten-item program is complete.

All ten foundational-debt checkboxes above are accepted closed. That removes the FDC-specific blocker, but a subsequent audit promoted the mandatory `RESIDUAL_DEBT_CLOSURE_PROGRAM` below. At that historical FDC checkpoint the subsequent RDC program still blocked promotion; the later RDC-08 Gate-8 completion independently lifted that specific blocker, without automatically promoting any feature.

## Historical mandatory residual-debt closure — all eight RDC items completed

A subsequent repo-wide audit found additional **confirmed current debt** after FDC-10. These were mandatory pre-feature obligations under `RESIDUAL_DEBT_CLOSURE_PROGRAM` when first identified. **All eight items are now CLOSED/PUBLISHED**, including independently accepted RDC-08 Gate-8; that earlier prerequisite no longer blocks *consideration* of a separately scoped new feature.

- [x] `RDC-01` — Asset transaction/session recovery hardening — CLOSED/PUBLISHED `699382a58c4c9570cdf36a05693044461e87c58b`; bounded post-failure cleanup uses the published idempotent `ABORT` contract without masking the primary failure or adding blind retry.
- [x] `RDC-02` — Host dead-surface cleanup — CLOSED/PUBLISHED `09a432f6c0b73ef2425add5950b6f6d5dee3733d`; eight re-proven unreachable Linux transport remnants removed while published compatibility/profile ownership remains intact.
- [x] `RDC-03` — Target dead API cleanup — CLOSED/PUBLISHED `e516fdc1d8818007db40ee12669d28f3f828c489`; ten dead target APIs removed, retained compatibility owners preserved, deterministic BIN equivalence and exact-candidate hardware/runtime acceptance PASS.
- [x] `RDC-04` — **CLOSED/PUBLISHED `7fda7b8225f05c487f6d5d7154443e8369e74571`** — one fail-closed output reset for three caller-controlled generators; disposable safety 27/27, deterministic firmware/recovery equivalence, Gate-2C docs, local ten-path commit and normal non-force fresh-fetch publication PASS.
- [x] `RDC-05` — **CLOSED/PUBLISHED `7f75cffdd0c632c5f99310f2c8708d745769aa42`** — one pinned GitHub Actions Ubuntu workflow, locked .NET 10 restore, warning-clean Release build, hosted Core 84/84 and Transport 24/24 (run `37829541187`), non-force publication, no physical bench ownership.
- [x] `RDC-06` — **CLOSED/PUBLISHED `7bc5f27ce3447f0c907d2643e7db67669371210a`** — exact three-path `help` ownership relocation, candidate Git tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`, physical signed firmware revision 5 accepted with Windows UART/physical OLED, separate USB EPnR CTR hardware fix committed at `83e57f609bab4bcc8a61ea2222629bf9066e910d`; non-force push/fresh fetch and hosted CI run `38011195238` (Core 84/84, Transport 24/24, no skips) PASS.
- [x] `RDC-07` — **CLOSED/PUBLISHED `7846fa48429d00233116438821acc0ea2a0b38be`**: formatting-only `bootloader.c` with unchanged 8267 significant tokens and byte-exact 5996-byte synthetic-key ARM BIN (`7EDCD55CED5DDA529EFF173CF1ABB676E7892B1A5824B6EE4695A7BA3EC796BE`); symbols/disassembly/stack/resource PASS, no real key/Flash/target changes, non-force fetch and hosted CI `38067711160` Core84/84 Transport24/24 PASS.
- [x] `RDC-08` — documentation/source-of-truth closure work accepted in Gate-0/1 and Gates 2–7 docs-only commit `faabfe75bbb3b805548092c9f298ac862c38ef1d` (exact-SHA CI [run 38079050326](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079050326) SUCCESS). **Final Gate-8 publication predicate is fulfilled:** ordinary non-force commit `c309428b03615a9afd514e3c7c8886a82a5112e9`, independently fresh-fetched clean `0/0`, exact-SHA [hosted CI 38079465787](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38079465787) SUCCESS. RDC-08 and the RDC program are CLOSED/PUBLISHED; this does not independently authorize a next product feature. No new feature boundary is activated.

Canonical program: `docs/RESIDUAL_DEBT_CLOSURE_PROGRAM_PLAN.md` + `_ACCEPTANCE_PLAN.md`.

Consumer-driven future capabilities (generic timers/queues, DMA, filesystem, low-power, networking, richer observability, etc.) are not silently promoted by this program merely because they are deferred.


## Priority maintenance after RDC-08 — public repository sanitization

- [x] `PUBLIC_REPOSITORY_SANITIZATION` — **CLOSED/PUBLISHED_WITH_RECORDED_LIMITS (2026-10-14, prospective-only).** Gate-0 design and Gate-1 classification accepted, owner-limited Gate-2 disposition explicitly documented, Gate-3 forward-only choice selected, Gate-4 two-path `.gitignore` / CI hygiene committed, Gate-5 independent verification PASS_WITH_RECORDED_LIMITS, Gate-6 ordinary non-force publication plus exact-SHA CI PASS, Gate-7 final authority reconciled and **PUBLISHED** at `9819f3188d08a9aff03d3284f15be4d0dd6d9139` with fresh-fetch clean `0/0` and [exact-SHA CI 38629887654](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38629887654) SUCCESS. The original Git history, public URL, private source provenance and product contracts remain intact; no new product feature is implicitly authorized. Canonical scope/proof: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`, `docs/PUBLIC_REPOSITORY_SANITIZATION_FINAL_ACCEPTANCE.md`.

Gate-1B **owner-approved** disposition policy: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md` retains all 113 original PRIVATE/RETAIN/REDACT classifications with **113/113 `OWNER_APPROVED_2026_10_11`** for the proposed 90 public-keep and 23 archive-first edit/review/no-extra-edit routes, with all nine README-referenced paths retained. Owner also approves keeping 36 original-source snapshots in the existing GitHub-`PRIVATE` repository until separately authorized deletion, and has already accepted known historical-path exposure under forward-only strategy A. Authenticated and independent offline source restores both 36/36 PASS; operator-observed anonymous REST 200/404/404 PASS (scoped endpoints). **This closes grouped policy signing only**, not full Gate-1 historical/external-hosting/complete preservation acceptance. Gate-2 remains BLOCKED.

Docs-only execution slice PUBLISHED/TECHNICALLY ACCEPTED at `c55c90c4b40605adc2e3da26a7314467c618c512` (exact-SHA [CI 38140063373](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38140063373) SUCCESS; 263 tracked paths): a protected original-document archive and 13 public Markdown operator-value replacements are described in `docs/PUBLIC_REPOSITORY_SANITIZATION_DOCS_ONLY_ACCEPTANCE.md`. That **docs-only slice itself** neither authorized nor changed the seven Windows build/recovery scripts, Git history, target firmware or hardware. Seven scripts were subsequently archived and portabilized under a **separate operator-directed, Windows-tested technical sublane**; see `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_SCRIPT_PORTABILITY_AUDIT.md`. Final repository sanitization and historical-risk approval remain OPEN.

**Gate-1 exit audit ACCEPTED/PUBLISHED `53642b0c529925005f3b09a4882b8c8c115bbe54`, exact-SHA [CI 38175961331](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38175961331) SUCCESS (2026-10-11):** `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_EXIT_AUDIT.md` documents a fully reconciled 267-path public prestate (254 frozen + 13 accepted additions), owner-approved 113/113 routes, 1,542 main-reachable blobs screened, 56/56 Action run artifact queries with 0 stored artifacts, 55/56 retrievable job logs without recognized credential signatures, archived-original restore 36/36 PASS, and the scoped accepted old-history/anonymous-private-denial policies. An inaccessible old failed job log, deleted/expired/cached material and external replicas are named `NOT_CHECKED`, not declared safe. `PASS_WITH_RECORDED_LIMITS` is accepted as the bounded Gate-1 exit; full Gate-2 private engineering preservation and Gates 3–7 are not initiated by this result.

**Final maintenance disposition — 2026-10-14:** The formerly active Gate-2 historical ZIP discovery and custody-check workflow is superseded by the owner's accepted recorded-limits policy, not silently passed as a technical test. Gate-5 independently checked the exact public release-view candidate (269 tracked, 1574 reachable blobs, 36/36 private originals, Host Core 84/84 and Transport 24/24). Gate-6 acceptance was published normally at `1772d37d3a706d0d2f64a4e09f5810cee46ef76e` with exact-SHA hosted CI run `38629097010` SUCCESS. Gate-7 documents final closure under explicit current-state authority. **No more historical ZIP searches or custody-escrow work is pending in this maintenance programme.** Older Gate-1B/2 OPEN statements above are retained as historical snapshots only, not current tasks. Additional product work requires separate promotion/acceptance.

## Selected next concrete Host feature — Desktop published asset inspector (2026-10-15)

**Physical Gate 3b is still OPEN (`PHYSICAL_DESKTOP_UI=NOT_VERIFIED`):** the Windows screenshot demonstrates the new Avalonia panel but Windows does not own STM32 native USB on the canonical bench; the cable belongs to Ubuntu Mac mini. The mistaken Windows WinUSB/PnP diagnostic is withdrawn as execution-domain ownership error. **Gate 3b.1 Ubuntu USB/libusb IF2 + PublishedOnly CLI is now runtime PASS** (2026-10-16; source `a0c346d17b89`, operator evidence SHA-256 `F9B28764219E06E710BC4B223F9B53C6E83451573A7A6E03E973E9C976F2B081`, CRC and 62/62 hashes PASS). SSH/SCP, 10/10 CLI payload hashes, physical USB `1209:000C`/vendor IF2 `FF/00/00` and CLI `devices`/`info`/`config status`/`config read` EXIT=0; no committed asset, compiled-default fallback, remote cleanup verified. Ubuntu Server is confirmed by its operator to have no graphical desktop; no Ubuntu Avalonia test is currently possible. Classify this operator-confirmed headless configuration as `ENVIRONMENT_BLOCKED_HEADLESS_GUI`, not a product failure; CLI PASS never implies GUI PASS. No driver changes, rewiring, extra USB cycles, firmware writes, or speculative Web/USB forwarding. The Desktop Inspector acceptance plan owns the detailed criteria; `CURRENT_STATE.md` owns global status.

**Gate-0 AND source/Host CI ACCEPTED/PUBLISHED:** `DESKTOP_PUBLISHED_ASSET_INSPECTOR` delivers an explicit manual Refresh published asset control in existing Avalonia Desktop. Gate-0 docs `457be3dc4ff2e642138bd62178d91420ee552570` (CI `38746868482` SUCCESS); source paths `DesktopController.cs`, `MainWindow.cs`, `ManagementServiceOperationTests.cs` committed as `4729341e732ffd61e7fe2b0e48973f2c98726a5d`, [exact-SHA CI 38748957900](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38748957900) SUCCESS: Core 87/87, Transport 24/24, Release 0 warnings/errors. Capability-gated PublishedOnly reads, no implicit polling/writes/network/new Host runtime. **Physical Desktop + STM32 UI acceptance remains NOT_VERIFIED**, a separate follow-up if product parity is required. The paired source/acceptance documents record the evidence and boundaries. Web/HTTP remains a separate future feature; no MCU source or resource change.

## Host Management direction — architecture re-review C (historical decision, still constraining new code)

**CURRENT DECISION: `NO_GO_UNCONSUMED_RUNTIME_WRAPPER` (architecture re-review C).** Exact source comparison established that `DesktopController` already composes discovery, session ownership and typed operations; CLI has device selection; `DeusDeviceSession` handles serialized lifecycle and recovery; `ManagementServiceOperations` implements the accepted typed management facade. The previously proposed long-lived EXE/IPC plan and the newer two-file in-process `HostManagementRuntime` would not serve a real new consumer. **Do not implement either wrapper just to satisfy a historical Gate.** Original docs-only Gate-0 `bfb30d5` (CI `38732869436`), lockfile Amendment A `0f95c2e` (CI `38733800122`), Correction B `18e5037` (CI `38739187311`) and status sync `bdc5cdf` (CI `38740180025`) remain genuine publication history, but **none is a delivered Host service**. **Historical next-step decision at review C (now fulfilled for Desktop):** select a concrete user-facing consumer; the independent Desktop Published Asset Inspector was subsequently selected and source/CI-accepted at `4729341`, while physical Gate 3b remains open. Any *additional* Web consumer still requires its own UX/API/security/lifecycle gate and real new tests. A Web/HTTP listener, remote management, device authorization, physical MCU work and any speculative new Core abstraction remain **NOT AUTHORIZED**. Proof and future gate: `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md` + `_ACCEPTANCE_PLAN.md`.

### Frozen original Gate-0/Amendment-A proposal (historical; not current implementation scope)

**2026-10-15 owner selection:** `HOST_MANAGEMENT_SERVICE_FOUNDATION` is the next product direction, initially a bounded headless process/runtime consuming the accepted `DeusDeviceSession`, `ManagementServiceOperations` and existing Windows/Linux transports. **Original Gate-0 DESIGN ACCEPTED/PUBLISHED:** `bfb30d5f23389b54f0c114959640dc2d5a6291f9`, exact-SHA hosted CI `38732869436` SUCCESS; a narrow nine-path (two `packages.lock.json` additions) **Gate-0 Amendment A was separately published** at `0f95c2e5036652251976a0f7d6d5f62ea419116d` with [exact-SHA hosted CI 38733800122](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38733800122) SUCCESS, clean fresh-fetch `0/0`. **FROZEN HISTORICAL GATE-0 TEXT (superseded):** the proposed Standalone Service Stage-A implementation never began; the later unconsumed Core wrapper was rejected by architecture review C. Instead the independent Desktop Inspector feature was implemented and published at `4729341e732ffd61e7fe2b0e48973f2c98726a5d` (Core 87/87, Transport 24/24 CI PASS), with physical Gate 3b still open. The plan pair `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_PLAN.md` and `docs/HOST_MANAGEMENT_SERVICE_FOUNDATION_ACCEPTANCE_PLAN.md` freezes Stage-A at only five typed ReadOnly operations; no automatic USB device open, no new target RPC, no firmware/bootloader/Flash changes, no HTTP/socket listener. First prove a separate docs-only non-force publication plus exact-SHA Hosted CI; then separately validate synthetic source/test and real session gates. A local HTTP/Web frontend, its authorization/Origin/Host/security policy, and ESP/network remote management remain **future separately authorized boundaries**. For the first Host-only stage, ARM resource delta is zero by changed-path proof; before a later target-resident feature, run an exact-candidate ARM ELF/BIN/MAP/.su measurement. Completed FDC/RDC and prospective-only sanitization do not reopen.

## Host management presentation evolution — historical design, Web/network future

The accepted Host Control foundation already provides one transport-neutral Core with CLI + Avalonia presentation. Forward presentation policy is:

- CLI remains the first-class automation/headless/acceptance surface;
- Avalonia Desktop remains an optional workstation frontend;
- the `RESIDUAL_DEBT_CLOSURE_PROGRAM` is CLOSED/PUBLISHED; the Host-management direction is selected, but the unused Stage-A runtime proposal is **explicitly NO-GO** after architecture re-review C. Any local Desktop user feature or Web host requires consumer-specific scope and proof before source; a Web/localhost listener needs its own security contract and cannot be inferred from prior docs-only PASS.
- local Web presentation does not require STM32 networking;
- remote/LAN/Wi-Fi Web exposure belongs to the later network/security sequence and requires an explicit trust/authentication/authorization contract.

Canonical reference: `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`.

## Phase 5 - Networking

Future roadmap only. Asset/Configuration, firmware-update safety, `FDC-01..FDC-10` and `RDC-01..RDC-08` are already CLOSED/PUBLISHED. Networking is **not active**: its remaining blockers are a concrete consumer, explicit new feature promotion, and an accepted network-security/trust boundary; historical debt closure itself is no longer an outstanding prerequisite.

- [ ] ESP-01 / ESP8266 UART transport
- [ ] Framed STM32 <-> ESP protocol
- [ ] Network service boundary
- [ ] Remote control API with explicit security/trust contract
- [ ] Network-exposed Web control panel over the shared host/service model

<!-- BEGIN STM32_OS_USB_STRATEGY -->
## Native USB strategy

The Blue Pill micro-USB connector is an accepted first-class OS transport, not only a power connector.

Target progression:

1. Minimal STM32F103 USB Device core on PA11/PA12, implemented without HAL — published.
2. USB CDC ACM diagnostic/command console — published.
3. Transport-neutral shell/RPC service layer — published.
4. Binary framed RPC transport with CRC/request correlation — published; v1 is control/RPC only, not file transfer or firmware update.
5. Deus OS product/application/UI model freeze — published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`.
6. Boot/desktop UI foundation — published at `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`.
7. OLED dirty-region optimization — published at `39690c9ef103cbcf93272df8bad0359a934b7dc1`; exact changed page/column spans retained with one 512-byte framebuffer.
8. Static application runtime foundation — published at `25752fba557b1a1b518265a93bde05d3a6a3f9ad`.
9. Kernel composition-root decomposition — published at `fa75307fb392718a1d10d52770a6a111c97208e7`.
10. Production USB management-device foundation — published at `1f88083843c6aae9fd228ad2d677f9252b889a11`; vendor-specific WinUSB `Deus OS Device`, Microsoft OS descriptors, stable interface GUID and accepted binary RPC above transport; CDC is debug/recovery rather than the primary production API.
11. Cross-platform Windows/Linux host application — **published through Gate 7** at `e0f49f168542fa1cf49bca451e01b0c077aa8d18`; C#/.NET 10 Core/CLI + Avalonia desktop, Windows WinUSB/Linux libusb adapters and system identity/capability discovery are accepted.
12. Freeze shared internal-Flash ownership: reset-owning bootloader/recovery lower region, relocated application region, and top-page A/B persistence reservation — docs-only decision accepted in `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`.
13. Promote/freeze `OLED_UI_LAYOUT_CONFIG_V1` as the exact non-executable persistence consumer — complete.
14. Reopen Asset/Configuration Gate 0 for that object and freeze transfer/persistence/resource/fault-recovery contracts — complete; cross-contract closure PASS.
15. Implement bounded versioned Asset/Configuration transfer + atomic A/B persistence — Gate 1 authorized, then Gates 2–7 acceptance.
16. Recoverable USB firmware-update path + small bootloader — Gates 0–7 accepted/published at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`; capability mask `0x0000007F` active.
17. Close the mandatory `FDC-01..FDC-10` foundational-debt program through independently accepted host/target/docs slices.
18. Close the mandatory `RDC-01..RDC-08` residual-debt program and final documentation sweep.
19. Only after both debt-closure programs are complete, add networking/service/security extensions over the same application/service model.
20. Keep UART as the low-level emergency console and ST-LINK as recovery/GDB access even after USB becomes the primary management transport.

Production Windows integration uses the accepted inbox WinUSB stack without a custom kernel-mode driver or COM-port-first product identity. `USB_MANAGEMENT_DEVICE_FOUNDATION` supplies the firmware device/interface metadata; CDC remains an explicit development/debug/recovery profile, not the primary management surface. `HOST_CONTROL_APPLICATION_FOUNDATION` now consumes that accepted interface.

Constraint: STM32F103C8 is a USB Device target here, not a general USB Host platform. Keyboard/mouse emulation is possible as USB HID device behavior; directly hosting commodity USB peripherals is outside the baseline architecture.
<!-- END STM32_OS_USB_STRATEGY -->

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_ROADMAP_ACCEPTED_20260913 -->
## Scheduler steady-state wait/wake foundation — accepted 2026-09-13

Completed:

- explicit blocked task state;
- SVC event wait;
- ISR-safe event signal/wake;
- pending-event lost-wakeup protection;
- non-busy scheduler idle on preserved host MSP using `WFE`;
- PSP resume after event-driven wake;
- UART RX event integration;
- repeated hardware proof and full scheduler/RX/MSP/OLED regression.

Accepted candidate: `19932` bytes, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`.

### Historical next steps from this 2026-09-13 checkpoint

All items below were subsequently implemented through later accepted/published boundaries; they are retained only as chronology.

1. **Normal-boot production task ownership / migration**
   - move from the current MSP-owned normal boot to deliberate production task ownership;
   - preserve accepted event wait/wake and interrupt-driven idle semantics;
   - preserve UART ring ownership and frozen OLED behavior.

2. **Timer-backed sleep integration**
   - build `sleep()` / timed blocking on the accepted wait/wake model;
   - do not create a parallel blocking mechanism.

3. **Scheduler priorities**
   - add only after production ownership and timer blocking semantics are independently accepted.

Harness/evidence/recovery rules live in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.
<!-- END STM32_OS_SCHED_WAIT_WAKE_ROADMAP_ACCEPTED_20260913 -->
