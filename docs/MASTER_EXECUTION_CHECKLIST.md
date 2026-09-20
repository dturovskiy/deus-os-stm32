> [!IMPORTANT]

<!-- BEGIN STM32_OS_CURRENT_EXECUTION_STATE -->
## Current execution state

This section is authoritative.

### Published boundary — application runtime foundation — Gates 0–7 accepted

- [x] Gate 0 architecture/source-boundary freeze — PASS / planning parent `5191850ec749c0e7519b24aa53a5cbb9cd477e8c`.
- [x] Gate 1 exact five-path source implementation/static proof — PASS.
- [x] Gate 2 fresh GNU build/link/resource/stack validation — PASS; evidence `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`.
- [x] Gate 3 hardware/runtime acceptance — PASS; evidence `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`, log `703C41A7493D078E456A5721EAFEFF821A152D217FB3F30AB8C809D7452A6B36`.
- [x] Gate 4 physical application-view OLED review — PASS / `PHYSICAL_OLED=PASS`; evidence `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`.
- [x] Gate 5 canonical docs/evidence finalization — PASS; evidence `817D12D74D88F1C0F31C502B0715F038FF356382E56FC0D5421DC237239D78BC`.
- [x] Gate 6 local acceptance commit — PASS / `25752fba557b1a1b518265a93bde05d3a6a3f9ad`; evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`.
- [x] Gate 7 ordinary non-force publication — PASS; evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`.

Published commit/tree: `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`; final `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`. Accepted candidate: tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`; BIN `48604` / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`; ELF `77836` / `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`; MAP `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`; Flash `48604/65536`; SRAM `10032/20480`; minimum observed task0/task1 margins `272/424`.

Physical micro-USB disconnect/reconnect is a power-cycle recovery proof, not a live `USB_STATE_CHANGED` edge proof, because micro-USB is the sole target power source. Dynamic semantic-event delivery/no-rerender is independently proven by the real minute transition.

### Published boundary — kernel composition-root decomposition — Gates 0–7 accepted

- [x] measured god-module trigger recorded: parent `src/kernel.c = 5100` lines, `console_execute_request = 8644` linked bytes, task0 minimum accepted margin `272` bytes;
- [x] behavior-preserving decomposition only; no feature work;
- [x] no universal `kernel_context_t`, service locator or catch-all god object;
- [x] public application/RPC/binary/USB/scheduler/IWDG/OLED/startup behavior frozen;
- [x] exact seven-path source decomposition accepted; `src/kernel.c = 3405` lines (-33.235%);
- [x] Gate 2 fresh GNU build/resource acceptance PASS; candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`, Flash `48636/48656`, SRAM `10032/10104`;
- [x] Gate 3 hardware/runtime equivalence PASS; final task0/task1 margins `384/424`;
- [x] Gate 4 `PHYSICAL_OLED=PASS`; Gate 5 canonical docs/evidence finalization PASS;
- [x] Gate 6 local acceptance commit — PASS / `fa75307fb392718a1d10d52770a6a111c97208e7`;
- [x] Gate 7 ordinary non-force publication — PASS; final `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`.

### Published boundary — USB management device foundation — Gates 0–7 accepted / published

- [x] Gate 0 Windows/USB architecture and exact source boundary frozen;
- [x] private-test identity `1209:000C`, product `Deus OS Device`;
- [x] composite `EF/02/01`, `bcdUSB 0x0210`;
- [x] CDC interfaces 0–1 retained under IAD;
- [x] vendor WinUSB interface 2 with EP4 OUT `0x04` / IN `0x84`, bulk 64;
- [x] PMA `EP4 RX=0x180`, `EP4 TX=0x1C0`, exact end `0x200`;
- [x] MS OS 2.0 BOS/vendor-request binding frozen: Windows floor `0x0A000000`, vendor code `0x20`, exact set length `178`, configuration subset selector `0` for the first configuration while standard USB `bConfigurationValue` remains `1`;
- [x] stable management GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`;
- [x] binary protocol v1 / command service v2 / 35 RPC registry retained;
- [x] Gate 1 exact five-path source implementation/static proof — PASS; IRQ owns EP4/PMA/rings only, task0 owns management parser/RPC execution, fast bus-reset generation resets management protocol state;
- [x] Gate 2 fresh GNU build/link/resource/static candidate — **PASS** on repaired MS OS 2.0 descriptor candidate. Accepted tree `46841b52d351277deb134a6f4709619087b477af`; BIN `50172` / `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`; Flash/SRAM `50172/11728` within frozen ceilings `54780/11824`; task stacks `1024/512`; undefined symbols `0`; stack-usage `21/21`; real Git index untouched; no Flash. Evidence `B6101C2FAD615DC41856BD1DE88C46E92517759EC3AC1F56ACB2030B89252AF3`; log `245F15303355C47473C15CB05838DB495764F97D228DB1113B3C7693765D429F`.
- [x] Gate 3 Windows + target hardware/runtime — **PASS / COMPOSITE V11+V14**. `REV_0102`; interface 2 -> inbox `WINUSB`; WinUSB `128/128`; UART `32/32`; management RX/TX drops `0/0`; physical USB reconnect PASS; IWDG recovery PASS; task margins `448/424`; canaries intact/faults zero; final Flash exact accepted BIN. Evidence `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6`; log `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`.
- [x] Gate 4 OLED disposition — **PASS / `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`**; management boundary does not change OLED/UI/application rendering.
- [x] Gate 5 canonical docs/current-state finalization — **PASS / docs-only**; accepted source hashes unchanged.
- [x] Gate 6 local acceptance commit — **PASS / `1f88083843c6aae9fd228ad2d677f9252b889a11`**, tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`.
- [x] Gate 7 ordinary non-force publication — **PASS**; fresh fetch proved `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`.
- [x] Next boundary selected: `HOST_CONTROL_APPLICATION_FOUNDATION`.

### Published boundary — host control application foundation — Gates 0–7 accepted / published

- [x] Gate 0 host architecture/source-boundary freeze — **PASS**.
- [x] Gate 1 source implementation/static review — **PASS**.
- [x] Gate 2 firmware + host build/unit/resource acceptance — **PASS**; firmware tree `b895955f7738aceb6fca0272d510cc433378c6ab`, host tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`, BIN/ELF/MAP `50652/483488/207952`, Flash/SRAM `50652/11728`, Core `21/21`, Transport `5/5`.
- [x] Gate 3 Windows hardware/CLI acceptance — **PASS**; WinUSB IF2/EP4, lifecycle/error recovery, 128 unique pings and zero management drops.
- [x] Gate 4 Windows Desktop acceptance — **PASS**; shared Core session, capability-driven UI and reconnect/fresh negotiation.
- [x] Gate 5 real Linux hardware/cross-platform acceptance — **PASS / COMPOSITE**; Ubuntu 26.04.1 + libusb, IF2-only claim, CDC retained, 128 unique pings, same-port reconnect, final management/CDC drops `0/0`, final Flash exact.
- [x] Gate 6 docs finalization + one normal local acceptance commit — **PASS** / `e0f49f168542fa1cf49bca451e01b0c077aa8d18`, tree `42c77f2cf3d7e9f7f5c1ff24d9d437f61a397be6`.
- [x] Gate 7 ordinary non-force publication — **PASS**; final `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`.
- [x] host runtime: C# / `net10.0`; Avalonia `12.1.2`; Windows WinUSB + Linux libusb.
- [x] identity/capability contract: HELLO protocol `1`, service `3`, registry `36`; system capabilities `0x0000001F`.
- [x] Gate 7 evidence SHA-256 `638569FE26600013B3B6717C76DEBF3251980B06D0B493B835D96CB4BC0CFD8F`.

### Current boundary — asset/configuration transfer foundation — Gate 0 next

- [ ] Freeze the real v1 asset/configuration consumer and bounded transfer semantics.
- [ ] Freeze schema/version, exact maximum size and Flash ownership region.
- [ ] Freeze CRC/integrity, atomic commit and reset/power-loss recovery.
- [ ] Freeze default/previous recovery, erase/program alignment and Flash wear budget.
- [ ] Freeze incompatible-version migration/rejection.
- [ ] Keep non-executable configuration/assets separate from firmware/update state.
- [ ] Do not introduce a general filesystem without a concrete consumer.

### Published baseline — binary framed transport foundation

- [x] `main` / `origin/main` / remote `main` = `2fde9025a51021511e73a76b561f7983ca655e2f`.
- [x] published tree = `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`.
- [x] subject = `feat: add binary framed transport foundation`.
- [x] accepted firmware candidate = `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`.
- [x] tested source candidate tree = `c2c3d9743c23ab02329a9652714862fafb5bb17c`.
- [x] USB CDC text + binary RPC, malformed-frame recovery, pressure, reconnect and post-IWDG recovery accepted.
- [x] final published repo state = clean, ahead/behind `0/0`.

### Published boundary — binary framed transport foundation

Boundary ID:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Gate 0 planning decisions:

- [x] binary v1 rides over the existing USB CDC byte stream; UART remains text-only emergency diagnostics;
- [x] text/binary CDC demux reserves magic `A5 5A` and preserves partial text-shell state across complete binary frames;
- [x] exact versioned envelope uses request IDs, explicit payload length and little-endian integer fields;
- [x] integrity is CRC-16/CCITT-FALSE (`poly 0x1021`, init `0xFFFF`);
- [x] stable public 16-bit RPC IDs `0x0001..0x0020` are separate from internal dispatch enum ordinals;
- [x] max `4` arguments, max `31` bytes each, max RPC request payload `132` bytes;
- [x] response output is chunked; max data chunk `48` bytes gives an exact `64`-byte maximum RPC_DATA wire frame;
- [x] `HELLO_REQUEST/RESPONSE`, `RPC_REQUEST`, `RPC_DATA`, `RPC_END`, `PROTOCOL_ERROR` are the only v1 frame types;
- [x] destructive RPC requires explicit `ALLOW_DESTRUCTIVE` request flag;
- [x] binary frame TX requires nonblocking all-or-none CDC ring enqueue; no indefinite busy wait;
- [x] command semantics remain owned by the published command service; binary adapter must reuse `command_service_execute()`;
- [x] no heap, new task, SVC, IPC/timer subsystem, USB descriptor/PMA redesign, firmware update, bootloader, host GUI or OLED change.

Gate order:

- [x] Gate 0 planning + normative protocol/docs synchronization — **PASS**.
- [x] Gate 1 exact source investigation + binary framing/RPC implementation — **PASS / SOURCE IMPLEMENTED**.
- [x] Gate 2 fresh GNU build/link/static validation — **PASS**.
- [x] Gate 3 USB CDC binary + retained text/UART hardware acceptance — **PASS**.
- [x] Gate 4 OLED conditional review — **PASS / `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED `2fde9025a51021511e73a76b561f7983ca655e2f`**.

Accepted Gate 2/3 candidate and evidence:

- [x] tested candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- [x] BIN `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- [x] ELF `65876` bytes / `4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B`;
- [x] Flash `40720 / 65536`, SRAM `9752 / 20480`;
- [x] binary safe surface `21/21`, binary scheduler BUSY `8/8`, binary pressure `128/128` unique IDs;
- [x] retained CDC pressure `128/128`, UART race `128/128`, physical reconnect PASS, post-IWDG text+binary recovery PASS;
- [x] final Flash readback exact;
- [x] Gate 2 evidence `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- [x] Gate 3 log `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- [x] Gate 3 evidence `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`.

### Published boundary — OS application and UI model foundation

Boundary ID:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

Gate order:

- [x] Gate 0 product/application/UI architecture freeze — **PASS**.
- [x] Gate 1 published-source capability inventory — **PASS / READ-ONLY**.
- [x] Gate 2 architecture consistency review — **PASS**.
- [x] Gate 3 hardware acceptance — **PASS / `N/A_DOCS_ONLY`**.
- [x] Gate 4 OLED review — **PASS / `PHYSICAL_OLED=N/A_DOCS_ONLY_NO_FIRMWARE_CHANGE`**.
- [x] Gate 5 documentation finalization — **PASS**.
- [x] Gate 6 local docs acceptance commit — **PASS / `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED**.

Gate 0 freezes:

- Deus OS as an independently operating deterministic embedded device runtime;
- static firmware-linked applications with explicit IDs/lifecycle for v1;
- no arbitrary uploaded ARM executables or one-task-per-app requirement;
- splash -> desktop/home -> application-view UI lifecycle;
- real SYSTEM / USB / NETWORK status indicators;
- initial status time source = uptime `HH:MM`;
- firmware ownership versus future Control Panel management-plane ownership;
- target apps versus host plugins versus transferable non-executable packages;
- scheduler wake bits remain internal kernel notification state, not the public application-event ABI;
- `APPLICATION_RUNTIME_FOUNDATION` must define bounded semantic app events/services;
- future host tooling requires explicit firmware/build/platform/service/application identity/capability discovery;
- persistent Flash state requires version/integrity/atomic-commit/recovery/wear semantics before acceptance;
- current BSS `fault_record` is not reset-persistent; retained previous-boot crash/reset diagnostics remain later observability work;
- CRC/destructive authorization flags are not authentication; trust-sensitive network/update boundaries require explicit security review;
- portability must keep arch/platform/driver details below kernel/services/apps/UI/protocol semantics without speculative universal HAL work;
- timers/queues/synchronization/runtime statistics and heap/filesystem/RTC/DMA/MPU/power frameworks remain consumer-driven/deferred;
- exact next implementation order beginning with `BOOT_DESKTOP_UI_FOUNDATION`.

Canonical design:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`

Canonical acceptance:
`docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`

Canonical foundation gap review:
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

Gates 0–7 are accepted. Gate 6 committed exactly the accepted docs-only path set at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`; Gate 7 published it by ordinary non-force fast-forward.

### Published boundary — Boot / desktop UI foundation

Boundary ID: `BOOT_DESKTOP_UI_FOUNDATION`.

Gate order:

- [x] Gate 0 architecture/source-boundary freeze — **PASS**.
- [x] Gate 1 source implementation/static consistency — **PASS, REVISION 2**.
- [x] Gate 2 fresh GNU build/link/resource validation — **PASS / REVISION-2 CANDIDATE**.
- [x] Gate 3 retained hardware/runtime acceptance — **PASS**.
- [x] Gate 4 physical OLED acceptance — **PASS / `PHYSICAL_OLED=PASS`**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS / `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED**.

Gate 0 freezes:

- exactly `BOOT_SPLASH -> DESKTOP_HOME`; `APPLICATION_VIEW` remains deferred;
- splash `DEUS OS / STARTING / PLEASE WAIT`, home `DEUS OS / DESKTOP / READY`;
- 1000 ms minimum splash dwell without blocking scheduler/IWDG startup;
- task0 250 ms timed UI-service wake using existing scheduler timeout support;
- task0 as the only normal runtime OLED writer after bootstrap;
- SYSTEM=production readiness, USB=actual CDC configured state, NETWORK=inactive;
- monotonic uptime `HH:MM`, saturating at `99:59`;
- panel presentation only on visible semantic change or explicit `uiruntime` restore;
- normal semantic redraw never re-enters SSD1306 initialization/display-off; failed panel transfer may invalidate initialization and trigger bounded recovery on the next service pass;
- no new task/SVC/queue/mutex/generic timer/heap/filesystem/application runtime/command ID/USB redesign.

Canonical design: `docs/BOOT_DESKTOP_UI_PLAN.md`.
Canonical acceptance: `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.

Accepted Gate 2/3/4 evidence:

- [x] candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`;
- [x] BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`;
- [x] ELF `70700` bytes / `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`;
- [x] Flash `41520 / 65536`, SRAM `9792 / 20480`;
- [x] task0/task1 stack margins `640` / `424` bytes;
- [x] CDC `128/128` zero drops, UART `128/128` zero drops/errors, binary `128/128` unique IDs;
- [x] physical USB reconnect and authorized IWDG reboot/recovery PASS;
- [x] final Flash readback exact;
- [x] Gate 2 evidence `62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729`;
- [x] Gate 3 log `9481BEFADB5A8F1D17AF6FC0ACDE238FE66B6956940F56DC86A828CBFAA7F900`;
- [x] Gate 3 evidence `2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD`;
- [x] Gate 4 `PHYSICAL_OLED=PASS`: clean digit/text transitions, no unintended redraw, flicker, blank pulse or stale pixels.

Publication commit/tree: `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8` / `d27cf8246fb7563b2327955ffc06428b9d843b2a`; ordinary non-force push complete; final repository ahead/behind `0/0`.

### Published boundary — OLED dirty-region optimization

Boundary ID: `OLED_DIRTY_REGION_OPTIMIZATION`.

Gate order:

- [x] Gate 0 architecture/source-boundary freeze — **PASS**.
- [x] Gate 1 source implementation + deterministic self/static proof — **PASS**.
- [x] Gate 2 fresh GNU build/link/resource + transfer-metric validation — **PASS / V6 CANDIDATE**.
- [x] Gate 3 retained hardware/runtime + optimized-transfer proof — **PASS / V5 EVIDENCE**.
- [x] Gate 4 physical OLED regression — **PASS / `PHYSICAL_OLED=PASS`**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS / `39690c9ef103cbcf93272df8bad0359a934b7dc1`**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED**.

Canonical design: `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`.
Canonical acceptance: `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`.
Canonical deferred backlog: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.

Accepted Gate 2/3/4 evidence:

- [x] exact eight-path source candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- [x] BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- [x] ELF `71308` bytes / `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`;
- [x] MAP `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`;
- [x] Flash `44560 / 65536`, SRAM `9848 / 20480`, persistent optimization metadata increment `55` bytes;
- [x] `oled_status_bar_self_test` GCC stack usage `240` bytes <= `256` frozen diagnostic ceiling;
- [x] historical four-page semantic refresh `572` payload bytes / `36` writes;
- [x] clean present `0` writes / `0` payload bytes;
- [x] one-byte narrow update `9` payload bytes;
- [x] minute `00:00 -> 00:01` `x=123..125`, `11` payload bytes;
- [x] USB indicator transition `x=9..11`, `11` payload bytes;
- [x] task0/task1 margins after `oledstatus` = `328 / 424` bytes;
- [x] task0/task1 margins after `oleddirty` = `328 / 424` bytes;
- [x] CDC/UART/binary pressure `128/128`, malformed-frame recovery, USB reconnect and IWDG recovery PASS;
- [x] final Flash readback exact;
- [x] Gate 2 evidence `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`;
- [x] Gate 3 log `887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6`;
- [x] Gate 3 evidence `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`;
- [x] Gate 4 evidence `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`;
- [x] physical OLED `PHYSICAL_OLED=PASS`: no blank/off pulse, flicker, stale pixels, dirty-span clipping or console corruption;
- [x] Gate 5 evidence `7DAB43E97E238DA35894B767AD1403064905E64D9326F8B4E002AE0816AB6436`;
- [x] Gate 6 evidence `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`;
- [x] Gate 7 evidence `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

Publication commit/tree: `39690c9ef103cbcf93272df8bad0359a934b7dc1` / `f195fac5ce733c36a1d955e0fbe687ee6c83b605`; ordinary non-force push complete; final repository ahead/behind `0/0`.

Gate 5 additionally froze measured/deferred optimization triggers, latent-bug/soak/fault-injection strategy, bounded binary event-trace policy, external-storage/memory policy, diagnostic build-profile policy and repository EOL policy. `.gitattributes` owns LF for source/docs so Windows line-ending warnings do not remain operator-log noise.

### Published boundary — Application runtime foundation

Boundary ID: `APPLICATION_RUNTIME_FOUNDATION`.

Gate order:

- [x] Gate 0 application-runtime architecture/source-boundary freeze — **PASS**.
- [x] Gate 1 source implementation + deterministic/static proof — **PASS**.
- [x] Gate 2 fresh GNU build/link/resource/stack validation — **PASS / V3 CANDIDATE**.
- [x] Gate 3 retained hardware/runtime + application lifecycle/event proof — **PASS / V5 EVIDENCE**.
- [x] Gate 4 physical OLED application-view regression — **PASS / `PHYSICAL_OLED=PASS`**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS / `25752fba557b1a1b518265a93bde05d3a6a3f9ad`**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED**.

Canonical design: `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`.
Canonical acceptance: `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`.
Mandatory next-boundary decision: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`.

Accepted Gate 1–4 evidence:

- [x] exact five-path source candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`;
- [x] BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`;
- [x] ELF `77836` bytes / `E05725E7D1C5EC21619578A2CE8AEA873F6A3CE2D0D5DC4C75588FD4E65D09C6`;
- [x] MAP `43CFA7528043649C7D8A871D9EBC6FC6A5F9137105137A51FE25B9DA8627D7D4`;
- [x] Flash `48604 / 65536`, SRAM `10032 / 20480`, named runtime static state `184` bytes;
- [x] task stacks unchanged at `1024 / 512`; minimum observed task0/task1 margins after full application/binary activity `272 / 424` bytes;
- [x] application ABI v1: `system.home=0x0001`, `device.info=0x0002`, lifecycle values `0..6`, pointer-free 12-byte semantic event, 3x21 content view;
- [x] RPC IDs `0x0001..0x0020` unchanged; `applist/appstart/appstop = 0x0021/0x0022/0x0023`; command-service version `2`, binary frame protocol v1;
- [x] text and binary lifecycle semantics, repeated-start idempotence, invalid-start no-mutation and `uiruntime` active-app restore PASS;
- [x] real minute semantic event increments event state without application rerender;
- [x] physical micro-USB unplug/reconnect accepted as power-cycle recovery plus USB configured/default-home proof; live `USB_STATE_CHANGED` is not asserted across reset because the sole-power unplug destroys volatile previous state;
- [x] CDC/UART/binary pressure `128/128`, malformed-frame recovery and IWDG recovery PASS;
- [x] final Flash readback exact;
- [x] Gate 1 evidence `BC62687D179141D9B578FAFFF448135C71C9A5D223273D03018C87B848CBA2F3`;
- [x] Gate 2 evidence `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`;
- [x] Gate 3 log `703C41A7493D078E456A5721EAFEFF821A152D217FB3F30AB8C809D7452A6B36`;
- [x] Gate 3 evidence `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`;
- [x] Gate 4 evidence `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`;
- [x] physical OLED `PHYSICAL_OLED=PASS`.

Architecture review outcome: `src/kernel.c` is not classified as spaghetti, but its `5100` lines and concentration of composition, command/diagnostic, UI/application integration and low-level glue constitute a god-module risk. No additional feature may extend it before `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` is accepted. Do not replace it with a catch-all god object.

Publication commit/tree: `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`; ordinary non-force publication complete; final repository ahead/behind `0/0`. Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`; Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`.

### Historical boundary record — Kernel composition-root decomposition

Boundary ID: `KERNEL_COMPOSITION_ROOT_DECOMPOSITION`.

Gate order:

- [x] Gate 0 architecture/source-boundary freeze — **PASS**.
- [x] Gate 1 behavior-preserving source decomposition + static dependency proof — **PASS**.
- [x] Gate 2 fresh build/link/resource/structure validation — **PASS**.
- [x] Gate 3 hardware/runtime equivalence — **PASS**.
- [x] Gate 4 physical OLED equivalence — **PASS (`PHYSICAL_OLED=PASS`)**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS / `fa75307fb392718a1d10d52770a6a111c97208e7`**, commit tree `54fe7dc3d059e0d39d0b659f2ab8e7d1c677c01c`.
- [x] Gate 7 ordinary non-force publication — **PASS**; final local/remote state clean at ahead/behind `0/0`.

Canonical decision: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_DECISION.md`.
Canonical design: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`.
Canonical acceptance: `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`.

Gate 0 freeze:

- [x] parent commit/tree `25752fba557b1a1b518265a93bde05d3a6a3f9ad` / `24624db70923bdaa77f134cc956423511785c199`;
- [x] measured prestate `src/kernel.c = 5100` lines;
- [x] linked prestate: `console_execute_request=8644`, `console_execute_scheduler_diagnostic=3284`, `boot_desktop_ui_render=1420`, `kernel_main=1116` bytes;
- [x] refactor-only boundary: no public application/RPC/USB/scheduler/IWDG/OLED behavior change;
- [x] no universal `kernel_context_t`, service locator or catch-all god object;
- [x] no hidden extern access to private `kernel.c` state and no circular module dependency;
- [x] initially authorized new modules: application runtime bridge, application commands, scheduler diagnostics header/source pairs;
- [x] initially authorized modified files: `src/kernel.c`, with command-service header/source only if relocation requires it without public ABI change;
- [x] Flash <= `48656`, SRAM <= `10104`, task stacks exactly `1024 / 512`, runtime margins >= `256` bytes;
- [x] target material `kernel.c` reduction (~30% expected) is subordinate to coherent ownership/dependency quality, not line-count gaming.

Accepted Gates 1–5 record:

- [x] exact seven-source-path boundary: `src/kernel.c` plus application-command, application-runtime-bridge and scheduler-diagnostics header/source pairs;
- [x] `src/kernel.c 5100 -> 3405` lines (-33.235%);
- [x] linked sizes: `console_execute_request 8644 -> 6780`, `boot_desktop_ui_render 1420 -> 804`, `kernel_main 1116 -> 1112`, extracted `scheduler_diagnostics_execute_diagnostic=3312` versus former monolithic diagnostic `3284`;
- [x] candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` / `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; Flash `48636/65536`; SRAM `10032/20480`; final task margins `384/424`;
- [x] Gate 2 evidence `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`;
- [x] Gate 3 evidence `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`, pressure `128/128` on CDC/UART/binary, faults zero and exact Flash readback;
- [x] Gate 4 `PHYSICAL_OLED=PASS`;
- [x] no god object/service locator, hidden extracted-state extern, heap/new task/SVC/queue/mutex/timer/DMA/persistence mechanism.

After decomposition publication, the next feature boundary was `USB_MANAGEMENT_DEVICE_FOUNDATION`, followed by `HOST_CONTROL_APPLICATION_FOUNDATION`; both are now accepted/published. Current next boundary is `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` Gate 0 contract freeze.

Permanent constraints retained:

- direct-register bare metal; no HAL/Arduino/FreeRTOS;
- UART remains text emergency diagnostics;
- ST-LINK remains recovery/debug;
- no dual ST-LINK 3.3 V + micro-USB VBUS powering;
- UART adapter VCC remains disconnected;
- IWDG reload remains Thread/PSP production-progress owned and forbidden from USB/UART IRQ/Handler;
- existing two-task production topology remains authoritative;
- USB IRQ remains bounded hardware/ring/event ownership only;
- generic timers, IPC/queues/synchronization and runtime statistics remain deferred;
- accepted 128x32 OLED geometry/gfx primitives remain frozen; this dedicated UI boundary may change only runtime-owned status/content semantics under its explicit Gate 4 visual acceptance.

Canonical protocol:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

Canonical design:
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`

Canonical acceptance:
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`
<!-- END STM32_OS_CURRENT_EXECUTION_STATE -->

> **OLED geometry/rendering baseline: ACCEPTED / FROZEN (2026-09-11).**
> The authoritative hardware-accepted geometry and firmware fingerprint are in
> [`OLED_UI_ACCEPTED_BASELINE.md`](OLED_UI_ACCEPTED_BASELINE.md).
> `BOOT_DESKTOP_UI_FOUNDATION` is the dedicated boundary allowed to evolve runtime-owned content/status semantics while preserving that geometry. Configurable-layout, preset, custom-layout, persistence, or alternate-geometry material remains deferred and must not override the accepted baseline.
# Master Execution Checklist

This file is the canonical execution gate for the STM32 OS project.

## Current hardware baseline

MCU:

- STM32F103C8T6 / medium-density Cortex-M3
- 64 KiB flash
- 20 KiB SRAM

OLED:

- native `128x32`
- SSD1306-compatible
- I2C address `0x3C`
- framebuffer `512 bytes`
- four pages
- `A8=0x1F`
- `DA=0x02`
- `D3=0`
- start line `0`
- `A1/C8`
- full window columns `0..127`, pages `0..3`

The former 128x64 assumption is obsolete.

## Accepted OLED milestones

- I2C scan / address proof.
- SSD1306 command proof.
- visible output proof.
- SSD1306 driver extraction.
- framebuffer/font extraction.
- raw mapping calibration.
- native 128x32 geometry proof.
- 1x font/native full-frame baseline.
- generic opaque renderer.
- byte-equivalent aligned renderer fast path.
- retained 21x3 console without scrolling.
- accepted status-bar reservation/layout.

Current accepted Slice 4 evidence:

- image size `5680 bytes`
- SHA-256
  `C67AEBA137F645F5BECAEB6410382D44257E1B09EA3BE459BD83AC718F3A09AC`
- framebuffer `512 bytes`
- retained console state `67 bytes`
- physical layout accepted

## OLED console canonical plan

<!-- OLED_CONSOLE_ARCHITECTURE_PLAN_CANONICAL -->

Read together:

- `docs/OLED_SSD1306_HARDWARE_PROFILE.md`
- `docs/OLED_CONSOLE_ARCHITECTURE.md`
- `docs/OLED_CONSOLE_API_CONTRACT.md`
- `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md`
- `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md`
- `docs/OLED_STATUS_BAR_PLAN.md`
- `docs/OLED_UI_LAYOUT_PLAN.md`

The accepted Slice 4 framed geometry is now a compatibility/reference preset,
not a permanently hardcoded production appearance.

Canonical framed UI geometry:

```text
frame:             y=0 and y=31
status content:    y=1..4
status separator:  y=5
status/console gap:y=6
console viewport:  x=1, y=7, width=126, height=23
console rows:      y=7,15,23
bottom gap:        y=30
font:              5x7
cell advance:      6x8
capacity:          21x3
```

No hidden Y remap.
No 2x text workaround.
No 128x64 assumptions.
Console UI is not aligned to SSD1306 pages.

## OLED implementation / deferred sequence summary

- [x] Slice 0: restore known-good baseline.
- [x] Slice 1: isolate SSD1306 driver.
- [x] Slice 2: isolate monochrome framebuffer and font.
- [x] Hardware correction: identify and prove native 128x32 panel profile.
- [x] Slice 3A: native 128x32 source + canonical documentation.
- [x] Slice 3B: generic opaque text renderer.
- [x] Slice 3C: framebuffer-equivalent aligned fast path.
- [x] Slice 4: retained 21x3 console without scrolling + accepted reference UI.
- [ ] Slice 4B: configurable UI layout + status bar.
  - [x] 4B.0: initial status architecture/acceptance plan.
  - [x] 4B.0a: static status prototype protocol/isolation proof.
  - [x] 4B.0b: visually reject hardcoded full-frame composition; do not commit it.
  - [x] 4B.0c: define configurable layout/preset/customization plan.
  - [ ] 4B.1: add validated `oled_ui_layout` module.
  - [ ] 4B.1: add `minimal`, `boxed`, `compact` presets.
  - [ ] 4B.1: move borders/separator/regions into layout data.
  - [ ] 4B.1: keep 21x3 console with glyph-based horizontal fit.
  - [ ] 4B.1: retain 3x4 status font + COMM/UART + static `12:34`.
  - [ ] 4B.1: add runtime preset selection.
  - [ ] 4B.1: physically compare at least minimal vs boxed.
  - [ ] 4B.1: accept one or more presets, then commit/push.
  - [ ] 4B.2: add `ui show` and validated RAM-only `ui set`.
  - [ ] 4B.2: prove atomic rejection of invalid custom layouts.
  - [x] 4B.3 uptime `HH:MM` objective — implemented and hardware-accepted in `BOOT_DESKTOP_UI_FOUNDATION`.
  - [x] 4B.3 visible-change update objective — implemented and later optimized in `OLED_DIRTY_REGION_OPTIMIZATION`; accepted semantics use real SYSTEM/USB state rather than the old prototype COMM label.
  - [ ] 4B.4: define PC interchange/converter/configurator protocol.
  - [ ] 4B.4: send layout through UART first.
  - [ ] 4B.4: later reuse the same API through USB CDC.
  - [ ] 4B.5: optional versioned Flash persistence after runtime semantics stabilize.
- [x] Slice 5: circular 3-row scroll — accepted; detailed proof retained below.
- [x] Slice 6: dirty-page presentation optimization — accepted and later superseded by the published `OLED_DIRTY_REGION_OPTIMIZATION` boundary.
- [ ] Optional kernel-log integration — deferred; not part of the accepted numbered Slice 7 dirty-page UI-integration record below.

## Development loop

For every hardware-visible change:

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART
 -> physical visual
 -> PASS/FAIL
 -> evidence
```

Do not skip physical acceptance for OLED rendering/layout changes.

## Build discipline

- `-Wall -Wextra -Werror`
- `git diff --check`
- no warning suppression
- no implicit package installation
- no unrelated Git changes
- dynamic ELF symbol resolution
- fail closed on unexpected tree state

## Flash discipline

Close STM32CubeProgrammer GUI before CLI access.

SWD fallback order:

```text
4000 KHz
1000 KHz
400 KHz
```

Once flashing begins, source must remain matched to the flashed image.

## UART regression

Minimum:

```text
BOOT OK
ping -> PONG
```

For OLED work also require:

```text
i2cscan -> 0x3C
oledping -> OLED_CMD_OK
```

Rendering regressions:

```text
oledtext    -> OLED_TEXT_OK
oledrender  -> OLED_RENDER_EQ_OK + OLED_RENDER_OK
oledconsole -> OLED_CONSOLE_OK
```

## Fault diagnostics

`fault_record` resides in BSS and is cleared on reset.

Its address is build-dependent. Tooling must resolve it from the current ELF.

OLED must never be the only fault-reporting sink.

## OLED UI customization workflow

Visual layouts should be designed at exact `128x32` resolution with a
one-pixel grid.

Recommended workflow:

```text
Aseprite / LibreSprite / Piskel / equivalent
 -> 128x32 mockup
 -> select geometry/assets
 -> encode as preset/custom config
 -> target validation
 -> hardware preview
 -> physical accept/reject
```

The STM32 does not decode PNG/SVG/Figma files directly.

PC tooling converts layouts/assets into target configuration or packed 1-bit
bitmaps.

Runtime configuration is transport-independent:

```text
UART now
USB CDC later
```

No keyboard or mouse needs to be physically connected to the STM32 for normal
configuration.

## OLED UI freeze checkpoint — ACCEPTED 2026-09-11

This checkpoint supersedes earlier open OLED UI-layout planning entries.

- [x] Native 128x32 panel profile accepted.
- [x] Retained semantic console accepted.
- [x] Status bar exact reference accepted.
- [x] One-pixel clock right inset accepted.
- [x] One-pixel blank row below status bar accepted.
- [x] Three console rows accepted with one-pixel inter-row gaps.
- [x] Console remains 21x3 using compact `5x6` glyphs / `6x7` cells.
- [x] No side/bottom frame below the status bar.
- [x] Exact accepted binary reproduced before commit:
  `7396 bytes`,
  `4DDA68DA96215F6FC2960007B37AB5F8808BA9284EC8A726AEFFFC4B97FCF9C4`.
- [x] Full UART/OLED regression passed.
- [x] Physical OLED appearance accepted.
- [x] UI styling frozen.

Deferred, not blocking the current roadmap:

- [ ] Runtime/custom layout editing — deferred.
- [ ] PC configurator/import — deferred.
- [ ] UI persistence — deferred.
- [x] Uptime `HH:MM` behavior — accepted in `BOOT_DESKTOP_UI_FOUNDATION`.
- [ ] RTC wall-clock source — deferred.

- [x] Center status-bar field reserved for notifications:
  `x=20..107, y=2..6`, with guard columns `x=19` and `x=108`.
- [ ] Notification rendering/queue semantics — deferred to dedicated subsystem.

### Slice 5 — circular retained console scroll

- [x] Rotate retained rows through `first_row`.
- [x] Reuse/clear the old physical top row as the new bottom row.
- [x] Preserve pending-wrap / pending-next-line semantics.
- [x] No framebuffer `memmove` as the scrolling mechanism.
- [x] No SSD1306 hardware scroll.
- [x] `OLED_SCROLL_STATE_OK`.
- [x] `OLED_SCROLL_OK`.
- [x] Physical proof accepted: `SCROLL TWO / SCROLL THREE / SCROLL FOUR`.
- [x] Frozen UI implementation remains unchanged.

### Slice 6 — dirty-page SSD1306 present

- [x] Add `ssd1306_present(mono_fb_t *fb)`.
- [x] Send only pages selected by `mono_fb_dirty_pages()`.
- [x] Treat zero dirty pages as a successful no-op.
- [x] Clear each dirty bit only after that page transfers successfully.
- [x] Preserve failed/later pages for retry.
- [x] Keep `ssd1306_present_full()` for full-refresh/diagnostic use.
- [x] Physical proof accepted: only `y=16..23` became white while clean RAM pages also contained `0xFF`.
- [x] `OLED_DIRTY_MASK_OK`.
- [x] `OLED_DIRTY_CLEAR_OK`.
- [x] `OLED_DIRTY_IDLE_OK`.
- [x] `OLED_DIRTY_OK`.
- [x] Frozen UI implementation remains unchanged.

**Historical note:** the dirty-page runtime-integration boundary was completed before Slice 8; see the current accepted execution state above.
## Slice 7 — dirty-page UI integration — ACCEPTED

Status: **accepted**

Acceptance criteria completed:

- [x] Shared status/console UI path uses `ssd1306_present()`.
- [x] Scroll UI path uses `ssd1306_present()`.
- [x] Retained console dirty rows are consumed after rasterization.
- [x] Clean logical console rows are skipped during incremental render.
- [x] Row-1-only update produces framebuffer dirty mask `0x04`.
- [x] Dirty-page present clears the page mask after successful transfer.
- [x] Frozen status-bar/layout implementation remains unchanged.
- [x] Full UART regression suite passes.
- [x] Flash/verify/reset passes at SWD 4000 KHz.
- [x] Physical OLED result accepted.

Accepted firmware fingerprint:

- binary size: 9828 bytes
- SHA-256: `4015795457F6844EFA768F97E7C59C8170015F147199874B61B524A1289AA5E8`

Next boundary: move dirty-page presentation from acceptance/demo commands into the normal runtime UI lifecycle without reopening frozen geometry.

## Slice 8 — normal runtime OLED boot UI — ACCEPTED 2026-09-12

Status: **accepted**

Acceptance criteria completed:

- [x] Normal reset initializes the frozen product UI automatically after the UART boot banner.
- [x] Runtime composition reuses the accepted `oled_ui_layout_default()`, status renderer, retained console, and `ssd1306_present()` path.
- [x] Frozen status-bar/layout/font implementation files remain unchanged.
- [x] Runtime UI shows the frozen status bar plus `DEUS OS`, `BOOT OK`, and `READY`.
- [x] The proof clock remains `00:00`; uptime/RTC behavior stays deferred.
- [x] `uiruntime` restores the same product UI after explicit diagnostic screens.
- [x] Boot emits `OLED_RUNTIME_UI_OK` after successful OLED initialization/presentation.
- [x] Full UART/OLED regression suite passes.
- [x] Flash/verify/reset passed at SWD 4000 KHz in the acceptance run.
- [x] Final physical OLED appearance accepted.

Accepted firmware fingerprint:

- binary size: 10148 bytes
- SHA-256: `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`
- `.text`: 10148 bytes
- `.data`: 0 bytes
- `.bss`: 716 bytes
- framebuffer: 512 bytes
- retained console state: 67 bytes

Slice 8 closes the transition from acceptance/demo UI commands to the normal
runtime boot lifecycle. Frozen UI geometry and styling remain closed; select the
next non-UI/system slice separately.

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_MASTER_ACCEPTED_20260913 -->
## Production scheduler steady-state wait/wake foundation — ACCEPTED 2026-09-13

Status: **hardware and physical acceptance complete**

Acceptance criteria completed:

- [x] Explicit runnable vs blocked state exists through `SCHEDULER_TASK_BLOCKED`.
- [x] Task wait path uses `scheduler_wait_events()` through SVC #3.
- [x] ISR-side event signalling can wake blocked tasks safely.
- [x] Pending-event semantics close the check-vs-block lost-wakeup race.
- [x] When no task is runnable but blocked work remains, scheduler ownership parks on preserved host MSP with `WFE`; it does not busy-spin and does not falsely finish the run.
- [x] UART RX event publication occurs after ring-buffer byte publication.
- [x] Event consumer re-checks RX FIFO after wake and separates CR/LF protocol framing from payload.
- [x] Four UART IRQ wait/wake hardware rounds pass with raw sentinel `0x57`.
- [x] Positive idle-WFE count is proven in every round.
- [x] Task canaries remain intact; observed diagnostic task high-water stays within 512-byte task stacks.
- [x] Lifecycle isolation blocks all seven invasive scheduler diagnostics while active.
- [x] Existing foundation/cooperative/preemptive/stack/workload/console-probe diagnostics pass before and after wait/wake proof.
- [x] USART1 RX ring reports zero drops and zero errors after the complete regression.
- [x] MSP guard/canary and margin remain healthy.
- [x] Frozen OLED runtime regression passes.
- [x] Final physical OLED appearance is operator-confirmed PASS.
- [x] Final target Flash readback exactly matches the accepted candidate.

Accepted firmware fingerprint:

- path: `build\scheduler_wait_wake_foundation_v2\os.bin`
- bytes: `19932`
- SHA-256: `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`

Accepted evidence:

- source/build log SHA-256: `220316AF92C3328F9B0D4F850B6EBF9F09A345CD673AAD81A301ED0E9ADD0219`
- source/build ZIP SHA-256: `A6A449669CFDD401E569578C40D91169C8FC8E7A5CF557600E945FE92AAF2D3B`
- hardware log SHA-256: `970656981374C7252A96748B5EF16A17600ABAFDFE73624870DE4C73F33C484A`
- hardware ZIP SHA-256: `C4FD9B7774934102A95F4A66C5BF7AC586D0A85C360BA9DF34C7C1315A1B9590`
- physical OLED: `PASS_OPERATOR_CONFIRMED_2026-09-13`

Procedural rulebook for host scripts, evidence and recovery:
`docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`

Publication is handled by separate commit and non-force push gates.

**Next implementation boundary after publication:** normal-boot production task ownership / migration. Do not fold `sleep()` / timer integration or priorities into that migration gate.
<!-- END STM32_OS_SCHED_WAIT_WAKE_MASTER_ACCEPTED_20260913 -->
