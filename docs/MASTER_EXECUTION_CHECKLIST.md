> [!IMPORTANT]

<!-- BEGIN STM32_OS_CURRENT_EXECUTION_STATE -->
## Execution ledger summary

This file is a historical execution/checklist ledger. It is **not** the global current-state authority. Current project state and the active next boundary are owned by `docs/CURRENT_STATE.md`.

### Mandatory foundational debt closure program — OPEN 2026-10-03

This section is the executable ledger for the post-publication architecture/code audit. It is intentionally placed ahead of historical accepted-boundary records. `FDC-01..FDC-10` must all be accepted closed before a new Host Management Service/Web/network feature boundary begins. A checkbox may be marked complete only from dedicated build/test/static/hardware evidence appropriate to that item; “code changed” alone is not closure.

#### Slice A — Host long-lived-session hardening

- [x] **FDC-01 — generic RPC timeout/cancellation correlation and abandoned-response cleanup — CLOSED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e` (`HOST_RPC_TIMEOUT_RECOVERY_HARDENING`).**
  - [x] freeze one policy for RPC timeout/cancel aftermath: one channel-owned abandoned-request registry with `SingleResponse` and `RpcUntilTerminal`; unresolved abandoned state at request-ID wrap fails closed and requires a fresh session;
  - [x] delayed `RPC_DATA` and delayed `RPC_END` for an abandoned request cannot poison a later fresh request;
  - [x] unknown mismatched request IDs that are not explicitly abandoned/stale remain fatal correlation errors;
  - [x] unresolved abandoned IDs cannot be reused; clean wrap remains `0xFFFF -> 0x0001`, while wrap with unresolved abandoned state fails closed;
  - [x] existing `FirmwareUpdateClient` stale-ID behavior is reconciled with the same channel registry rather than a second independent mechanism;
  - [x] INFO/BEGIN/AUTHORIZE/END retain no blind retry; response loss remains reconnect/INFO/adjudicate/restart semantics;
  - [x] deterministic Core validation PASS: timeout/cancel/partial delayed streams, stale terminal `PROTOCOL_ERROR`, unknown correlation, multiple abandoned IDs, clean/poisoned wrap, reset safety and firmware retry/no-blind-retry regressions; authoritative evidence SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`, Core `51/51`, Release build PASS, target I/O zero.

- [x] **FDC-02 — `DeusDeviceSession.StateChanged` reentrancy/deadlock closure — CLOSED / PUBLISHED `42245d9d71504482fb189d8351ecce7049542145` (`HOST_SESSION_STATE_EVENT_REENTRANCY_HARDENING`).**
  - [x] arbitrary subscriber callbacks are no longer invoked synchronously while `_operationGate` is held; one per-session serialized asynchronous notification chain owns delivery;
  - [x] state ordering remains deterministic across Connect/recovery/Disconnect/Dispose and exact transition sequences are unit-tested;
  - [x] subscribers may synchronously invoke `ExecuteAsync` or `DisconnectAsync` without deadlocking the lifecycle owner;
  - [x] callback exceptions are isolated, unsubscribe is effective for later dispatch, Dispose does not wait for blocked callbacks, and Disconnect/Dispose race is bounded/idempotent;
  - [x] deterministic Gate-2 host validation PASS: evidence SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`, manifest `72/72`, Core `58/58`, Core/Desktop Release builds PASS, exact two-path pre/post state, target I/O and Flash mutation zero.

- [x] **FDC-03 — service-facing operation allowlist / no raw RPC proxy — CLOSED / PUBLISHED `0d9adfd8d0ed11478194e2268ede3c57379c8294` (`HOST_SERVICE_OPERATION_ALLOWLIST_HARDENING`).**
  - [x] define typed service operations and explicit read/control/destructive exposure classes;
  - [x] firmware `SAFE`/`DIAGNOSTIC` command classes are not reused as HTTP authorization policy;
  - [x] arbitrary `RpcAsync(rpcId, flags)` is not reachable from Web/HTTP/service input;
  - [x] `wdogtrip`, scheduler stress/diagnostic methods, UI mutation/test methods and unlisted RPC IDs are absent unless separately authorized by a future reviewed contract;
  - [x] negative tests prove raw/unlisted/destructive access is rejected/not routed;
  - [x] Gate-2 evidence SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`: manifest `78/78`, Core `66/66`, Core/CLI/Desktop Release builds PASS, exact candidate pre/post state, target I/O/Flash mutation zero;
  - [x] Gate 4 normal acceptance commit + Gate 5 ordinary non-force publication/fresh-remote verification.

- [x] **FDC-04 — typed Host Core management models — CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d` (`HOST_TYPED_MANAGEMENT_MODELS_HARDENING`).**
  - [x] every service-v1 state/control acknowledgement has a typed Core parser/model or is explicitly excluded (`rpcinfo` diagnostic);
  - [x] health state required by service/Web is no longer presentation-parsed from raw `RpcResult.OutputText`;
  - [x] parsers enforce required keys/shape, bounded input, boolean/application-ID ranges and forward-compatible unknown fields where appropriate;
  - [x] CLI/Desktop/service consume shared Core models rather than duplicating protocol text parsing;
  - [x] Gate-2 evidence SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`: manifest `78/78`, Core `78/78`, Core/CLI/Desktop Release PASS, exact candidate pre/post state, target I/O/Flash mutation zero;
  - [x] Gate 4 normal acceptance commit + Gate 5 ordinary non-force publication/fresh-remote verification.

- [x] **FDC-09 — native transport cancellation/disposal for long-lived/multi-device use — CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73` (`HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING`).**
  - [x] Windows WinUSB and Linux libusb I/O have explicit bounded cancellation/disposal semantics;
  - [x] user cancellation and session disposal cannot leave an unbounded native operation that outlives ownership;
  - [x] two simultaneous independent sessions remain isolated;
  - [x] cancellation followed by reopen/recovery produces a clean decoder/channel/session;
  - [x] platform transport tests and real Windows/Linux smoke acceptance accepted: Transport `24/24`, Core `78/78`, Linux cancel-drain/dispose/reopen `2021/3/15 ms`, Windows cancel-drain `2017 ms`, physical disconnect `TransportDisconnected` in `7 ms`, canonical Mac-mini bench ownership restored.

#### Slice B — Target architecture/lifecycle cleanup

- [x] **FDC-05 — composition-root convergence without framework refactor — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.**
  - [x] Gate-0 audit/plan pair frozen: `KERNEL_COMPOSITION_ROOT_CONVERGENCE_PLAN.md` + `_ACCEPTANCE_PLAN.md`;
  - [x] natural RCC/clock, USART1, I2C1 and PC13 owners extracted without framework-only refactor;
  - [x] no universal `kernel_context_t`, service locator, hidden extracted-state `extern`, dependency cycle or line-count-only split;
  - [x] build/resource/stack/public-ABI regression and hardware equivalence PASS;
  - [x] Gate-5 docs/architecture reconciliation records exact candidate `bb99acf111dfa3a78193b4e5d3376fa077defa1e` and v18 hardware evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`;
  - [x] Gate 6/7 accepted at `6aa2df19ab02c14bde38833e738fe825008102e8`; ordinary non-force push + fresh-fetch clean `0/0`.

- [x] **FDC-06 — semantic system/service state upstream of UI — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.**
  - [x] Gate-0 plan pair frozen: `SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md` + `_ACCEPTANCE_PLAN.md`;
  - [x] one bounded semantic state owner supplies health/USB/network/time to application runtime and OLED presentation;
  - [x] `application_service_snapshot_t` is built from semantic state, not `boot_desktop_ui_snapshot_t` indicators;
  - [x] OLED status rendering consumes the same upstream semantic state;
  - [x] semantic-event/no-rerender behavior and physical OLED regression PASS;
  - [x] build/resource/stack + exact hardware Flash identity PASS;
  - [x] Gate 6/7 accepted at `6aa2df19ab02c14bde38833e738fe825008102e8`; ordinary non-force push + fresh-fetch clean `0/0`.

- [x] **FDC-07 — fail-closed application stop failure semantics — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.**
  - [x] Gate-0 plan pair frozen: `APPLICATION_STOP_FAILURE_HARDENING_PLAN.md` + `_ACCEPTANCE_PLAN.md`;
  - [x] failed `stop()` produces `FAILED`, preserves unresolved `active_id`, increments fault count once and starts no replacement/home fallback;
  - [x] deterministic synthetic failing-stop proof PASS with no production runtime residue;
  - [x] existing Home/Device Info start, repeated-start idempotence, normal stop/home fallback and invalid-start regression PASS;
  - [x] Gate-5 docs reconciliation complete on exact candidate;
  - [x] Gate 6/7 accepted at `6aa2df19ab02c14bde38833e738fe825008102e8`; ordinary non-force push + fresh-fetch clean `0/0`.

#### Slice C — Target/update robustness closure

- [x] **FDC-08 — bounded target/update robustness — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.**
  - [x] Gate-0 plan pair frozen: `TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md` + `_ACCEPTANCE_PLAN.md`;
  - [x] reset handler constrained to authenticated `[APP_BASE, APP_BASE + image_length)`;
  - [x] bootloader and runtime HSE/PLL/clock-switch waits are bounded;
  - [x] UART TX wait is bounded with explicit failure semantics;
  - [x] runtime `ENTER_BOOTLOADER` reset fallback uses wrap-safe elapsed-time/deadline semantics;
  - [x] INFO/BEGIN/AUTHORIZE/END retain no blind retry; DATA retains exactly one exact timeout retry;
  - [x] Flash/SRAM/stack, bootloader 8-KiB ceiling, persistence ownership, rollback floor and HMAC/digest guarantees remain intact;
  - [x] deterministic Gate-1..3 proof SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` + final hardware Gate-4 evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` PASS;
  - [x] Gate 6/7 accepted at `6aa2df19ab02c14bde38833e738fe825008102e8`; ordinary non-force push + fresh-fetch clean `0/0`.

#### Slice D — Documentation/source-of-truth closure

- [x] **FDC-10 — documentation consistency — FINAL CLOSURE COMPLETE / PUBLICATION OWNED BY THIS DOCS-ONLY REVISION.**
  - [x] Gate-0 plan pair frozen: `DOCUMENTATION_CONSISTENCY_CLOSURE_PLAN.md` + `_ACCEPTANCE_PLAN.md`;
  - [x] obsolete non-historical USB/persistence/update/current-state claims reconciled with published state;
  - [x] narrow `OLED_UI_LAYOUT_CONFIG_V1` persistence remains distinct from broader deferred/custom UI work;
  - [x] `CURRENT_STATE`, `ROADMAP`, backlog, documentation inventory, scoped plans/addenda and changelog reconciled with `FDC-01..FDC-10`;
  - [x] historical chronology preserved; stale matches in historical/scoped records remain explicitly historical rather than rewritten;
  - [x] repo-wide stale/future/deferred/open-checkbox audit leaves no unclassified fundamental documentation tail;
  - [x] `git diff --check` clean; tracked build/evidence/archive/log/secret-like artifact scan and credential-signature scan clean;
  - [x] final documentation-only acceptance records inventory, classifications, changed paths and clean publication poststate.

Program exit criterion: **10/10 FDC items accepted closed**. Only then may `HOST_MANAGEMENT_SERVICE_FOUNDATION`/Web or networking/service/security feature implementation be promoted.

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

### Published boundary — Asset / Configuration transfer — Gates 0–7 accepted / published

- [x] historical fail-closed `DEFERRED_NO_REAL_CONSUMER` Gate-0 result preserved;
- [x] concrete `OLED_UI_LAYOUT_CONFIG_V1` consumer promoted and Gate 0 reopened/frozen;
- [x] bounded transfer, A/B persistence, resource, Flash-operation, fault-injection and ST-LINK recovery contracts accepted;
- [x] Gate 1 bounded source implementation accepted;
- [x] Gate 2 build/unit/resource acceptance accepted;
- [x] Gate 3 hardware transfer/persistence/runtime acceptance accepted;
- [x] Gate 4 consumer physical/semantic OLED acceptance accepted;
- [x] Gate 5 deterministic fault/recovery acceptance accepted: Campaign A/B, FI-9 response-loss idempotency, 7/7 corruption matrix and physical VBUS retention;
- [x] final accepted persistence wear `46/64`;
- [x] Gate 6 publication activation accepted on firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, Flash/SRAM `54268/11944`, system capabilities `0x0000003F`;
- [x] Gate 7 ordinary non-force publication complete at `562e786ffa734da055c23144ec4256bc8961bbaf`, tree `88720a614794d5aef93cf13ac762a4095cb17baf`.

### Published boundary — pre-Bootloader resource / architecture recovery

Boundary ID: `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY`.

- [x] Gate 0 documentation/source-boundary freeze + read-only architecture/resource audit — **PASS**.
- [x] Gate 1 measurement-only resource attribution — **PASS**; exact baseline BIN `7EDB650B...23F9F`, Flash/SRAM `54268/11944`, linked `-Os` deltas: `kernel.c -1648`, scheduler core `-1360`, USB device `-1600`, gfx/OLED `-1508`; scheduler diagnostics classified `LINK_BLOCKED_COMPILER_MEMSET`; `RING_RESIZE_AUTHORIZED=NO`.
- [x] Gate 2 behavior-preserving source/profile recovery — **PASS**; exact six-file WIP applies `kernel.c -> -Os`, removes the scheduler-owned duplicate 1-KiB default stack store, and rebinds diagnostics/self-tests to existing production stack storage; no USB ring cuts, command/RPC pruning, diagnostic-profile split, linker relocation or Bootloader change. Evidence SHA-256 `D84AC967C204FC9D09B94EC8AD7B472B173BCDCE4593320F88601B062681E325`.
- [x] Gate 3 fresh build/resource/static acceptance — **PASS**; candidate tree `a10182e7d0659b9b161073ad49a8816ecb6e7918`, BIN `52908` / `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`, ELF `5BF8D5CE66049CCBD7EF2D77D6D569B980CB96B33E0FFDBFDB1A626A6A101247`, MAP `AE45CF669031AC4CB29DB2FFB72161ABA2249D9A6558B80E6B40757EC162C29E`, Flash/SRAM `52908/10920`, stack-usage `26/26`, undefined `0`, public ABI diff `0`; evidence SHA-256 `9FA982530FAB4B703AF6D92936080BD13A0E2FF346D294F6376F5CAFE814EFE2`.
- [x] Gate 4 exact production-image hardware/runtime equivalence — **PASS / COMPOSITE**; exact Gate-3 candidate tree `a10182e7d0659b9b161073ad49a8816ecb6e7918` retained, task0/task1 margins `304/432`, MSP margin `1592`, management IF2 pressure `128/128`, application lifecycle, scheduler/IWDG, Asset CLEAN, zero USB errors/PMA overruns/drops, physical OLED and reconnect all accepted; IWDG reset completion was independently proven at `7575 ms` with `IWDG_RESET=1`, then final read-only closure proved Home/health/ping and exact 64-KiB Flash SHA `17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`. Closure evidence SHA-256 `05A8F13552656D3C2AE5B05F8BC0FC927140BF0CBAC964B12721E46BABB2357B`.
- [x] Gate 5 docs/evidence finalization — **PASS**; exact 28-path WIP set, exact six-path firmware diff `D1EC010F55313DA61DC1366E99E68A619CF4E678EDD2EE443E7D53AB08AD6A06`, host source unchanged, `git diff --check` clean, canonical Gate-4/Gate-5 tokens synchronized. Evidence SHA-256 `FD2ACCA48E8051356668933FBA9E0A5E451E1E2C97ADADCF3BB3302FC87748B4`; normalized docs diff SHA-256 `397E449A64EED8160A21BC484F8217636B55E45819AD1FED59C867CAB67EFCA9`.
- [x] Gate 6 one normal local acceptance commit — **PASS BY THE NORMAL LOCAL COMMIT CONTAINING THIS CHECKLIST**; exact commit/tree identity is recorded by Gate-6 evidence; no amend and no push are part of Gate 6.
- [x] Gate 7 ordinary non-force publication — **PASS**; ordinary `HEAD:refs/heads/main` push, no force, then fetch proof `HEAD == origin/main == FETCH_HEAD == a8f92f83c2ba8917ad183b1a099c9e21199c9463`, clean worktree/index and ahead/behind `0/0`. Evidence SHA-256 `335013389D8AB6C1B09C4BF720185FED82EC4A0D30A2F3F6237CF8ACCA1DE8CE`.
- [x] Recovery boundary — **COMPLETE / PUBLISHED**.

### Published boundary — firmware update / bootloader foundation + deployment closure

Boundary ID: `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`.

- [x] Gate 0A contract/source-boundary/source-ownership audit — **PASS**; published recovery bound, exact docs-only WIP, current linker/startup/Flash/recovery/host/capability owners proven, update bit 6 clear, no build/target/product/index/commit/push mutation. Evidence SHA-256 `BB3D9210639564D27E0E9FFF57817A10E619991EC3A1960D4D5D29E82E122002`.
- [x] Gate 0 architecture decision freeze / corrected FPEC reconciliation — **PASS**; 48-byte header, private-test `1209:000D`, HMAC-SHA-256, metadata pages 60/61, `0xA55A` only from erased `0xFFFF`, full-retry in-place update and bounded bootloader RAM/stack frozen.
- [x] Gate 0B static linked-size + relocation feasibility — **PASS**; ARM GNU 15.3.1 bootloader Flash `5756/8192`, conventional SRAM `1684/2048`, longest linked stack `648/768`, MSP margin `376`, undefined `0`; relocated application Flash/SRAM `52932/10920`, end `0x0800EEC4`. Evidence SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`.
- [x] Gate 0 overall — **COMPLETE / ACCEPTED**.
- [x] Gate 1 behavior-preserving Host Core ownership split + exact product source authorization — **PASS**; 22/22 byte lock, clean Core/test builds, Core `28/28`, Transport `5/5`, exact poststate; evidence SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`.
- [x] Gate 2 relocated application build/VTOR/handoff foundation — **PASS**; tree `637ea07b10cf84882e19cbb8239f31b7f48856a7`, Flash/SRAM `52932/10920`, end `0x0800EEC4`, vectors `0x08002000`, stack-usage `26/26`, undefined `0`; evidence SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`.
- [x] Gate 3 bootloader/update transport/security implementation — **PASS**; accepted evidence SHA-256 `E016B495CECE75F8BD2A4C120EC0E5506EFF31ED984566007EA6363CC6D35846`, full/firmware/host trees `34bee09f7e69e3bf55eee3ac6ca274bd70f137d4` / `8a0d64817de2d0eb9a3b465d3cd57c392259178a` / `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`; target mutation absent.
- [x] Gate 4 fresh build/static/host tests + immutable Bootloader-boundary recovery bundle — **PASS**; product revalidation evidence SHA-256 `974E482D098973FEE1F56DAA9A767E228A7EA7DBA98D747FCF5E5AD632F905B7`; recovery-tooling repair-v2 evidence SHA-256 `4FE642BD172ACAE390EB4BCCE1A8ABA7E2C6B5FFCD7D28319A2AAFAC4E9B0E31`; repaired tooling tree `a29b27aebd8f30fcd0448d561a7820a50a4cbb61`; independently byte-audited immutable recovery ZIP v2 SHA-256 `E4D8928E17C809B2A0BE7736379C4ACE3274B0B4AEB47621D9FBFBA0BC2FD3AE`; recovery region and signed package vectors unchanged; old recovery ZIP `00992EC3...` superseded/forbidden.
- [x] Gate 5 hardware update/interruption/authenticity/recovery acceptance — **PASS / HARDWARE FUNCTIONAL MATRIX COMPLETE; STAGES 0–10 ACCEPTED; HOST RELIABILITY REPAIR ACCEPTED**. Stage-2 v4 evidence `stm32_os_bootloader_gate5_stage2_explicit_update_entry_v4_20260930_180321.evidence.zip`, SHA-256 `80C670DF059ED37D0A7E2E7BBCF48F55FCC330B21381B68C93FE5DA385DDF8BA`, has ZIP CRC clean and `68/68` exact evidence hashes. It proves runtime `ENTER_BOOTLOADER=Ok/Resetting`, bootloader `1209:000D`, unprivileged bootloader `INFO=Ok/RecoveryIdle` with expected offset `0`, version floor `1`, committed version `1`, byte-exact Flash `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` before/while/after Stage 2, no `BEGIN/AUTHORIZE/DATA/END`, explicit HOTPLUG reset back to runtime, `VTOR=0x08002000`, `VECTACTIVE=0`, ticks `4771 -> 6492`, runtime USB restored and bootloader USB absent. One operator-visible privileged action changed only the ephemeral Linux `1209:000D` device-node ownership/mode; protocol execution remained unprivileged and no persistent host permission mutation was made. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`; Stage 2 is fully accepted. Formal continuation SHA-256 `DA28EC5CD24A31DCE74F996F22D5D774EE1C5E8BD69AA2F7B9F3791608FD3676` exact-binds Stage-2 v4 technical evidence plus the operator physical PASS with zero target I/O/mutation and authorizes Stage 3. Stage-3 technical evidence `stm32_os_bootloader_gate5_stage3_invalid_application_recovery_v1_20260930_182233.evidence.zip`, SHA-256 `A507F73C628DA735FCF6A5C0A4E58ABB7A984397970ACB7BE3C64CAE9E4F7D50`, has ZIP CRC clean and `72/72` exact evidence hashes. It proves bounded invalidation of only application page 59 (`0x0800EC00`), automatic bootloader recovery (`1209:000D`) with `INFO=Ok/RecoveryIdle`, version floor `1`, committed version `0`, no USB update `BEGIN/AUTHORIZE/DATA/END`, successful accepted `PRESERVE_PERSISTENCE` restore, exact final Flash `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, runtime USB restored, `VTOR=0x08002000`, `VECTACTIVE=0`, and advancing ticks `20939 -> 22724`. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal Stage-3 continuation SHA-256 `E587AEC687955E4A5B93B1350D1489E4F6451726A454C09ED091A2F4A3539CA3` exact-binds the `72/72` technical evidence with zero target I/O/mutation, fully accepts Stage 3, and authorizes Stage 4. Stage-4 technical evidence `stm32_os_bootloader_gate5_stage4_malformed_header_rejection_v1_20260930_184648.evidence.zip`, SHA-256 `BD0ABADA1523C4E213A4F2B46997A62FE55124208939786543577D2DD434F281`, has ZIP CRC clean and `68/68` exact evidence hashes. Six malformed `BEGIN` headers (format version, reserved byte, lower/upper/misaligned length, zero firmware version) all returned `BadHeader/RecoveryIdle` with offset `0`, floor `1`, committed version `1`; `AUTHORIZE_HEADER`, `DATA`, and `END` were not sent; whole Flash remained exact `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` before, immediately after the malformed cases, and after runtime restoration. HOTPLUG reset restored runtime `1209:000C`, `VTOR=0x08002000`, `VECTACTIVE=0`, and ticks `4806 -> 6555`. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal Stage-4 continuation SHA-256 `AD194B3198D19EF14EA726568DF50207DE7B09EBC4EF2C2FBE2E974974121F5D` exact-binds the `68/68` technical evidence with zero target I/O/mutation, fully accepts Stage 4, and authorizes Stage 5. Stage-5 technical evidence `stm32_os_bootloader_gate5_stage5_wrong_target_rejection_v1_20260930_190851.evidence.zip`, SHA-256 `2BAADC493338BA31A26F38F33A0488C7F2B7F092F2ABB8B8F420296D2766043F`, has ZIP CRC clean and `72/72` exact evidence hashes. It binds accepted repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E` and its signed wrong-target vector SHA-256 `B82A52EF431017A27C0BAA64D346982A93AB6A7D1240C18C14F11A3EDB2CB5FB` for target `0x0411`; pre-case `INFO` was `Ok/RecoveryIdle`, `BEGIN` passed `Ok/HeaderStaged`, and `AUTHORIZE_HEADER` rejected exactly as `TargetMismatch/HeaderStaged` with offset `0`, floor `1`, committed version `1`. `DATA` and `END` were not sent, whole Flash remained exact `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` before/after rejection and after runtime restoration, and runtime returned with `VTOR=0x08002000`, `VECTACTIVE=0`, ticks `4732 -> 6451`, runtime USB present and bootloader USB absent. Operator physical reconfirmation is now `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal Stage-5 continuation SHA-256 `C35A3319B806B21BC42B48EAF84822DF0F721AEE5EF5222BFA45C959F0BB90EE` exact-binds the `72/72` technical evidence with zero target I/O/mutation, fully accepts Stage 5, and authorizes Stage 6.
- [x] Gate 6 docs + capability-bit publication finalization + one normal local acceptance commit — **PASS BY THE NORMAL LOCAL COMMIT CONTAINING THIS CHECKLIST**; validation evidence SHA-256 `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7`, firmware/host trees `8323c68c931894441ae4db9138ba3838f35bb8b6` / `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, capability mask `0x0000007F`, Core `41/41`, Transport `12/12`, target I/O zero.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`**, tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb`; evidence SHA-256 `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`; ordinary non-force fast-forward, final `HEAD == origin/main == FETCH_HEAD`, clean, ahead/behind `0/0`.
- [x] Post-publication END-timeout forensic reconciliation — **PASS / CLOSED**; exact historical smoke chronology proves the failed published-deploy run executed an exact post-failure restore to the old version-1 baseline, so the old `C7EB7201...` Flash state was not evidence that published v2 had never transferred/committed; no speculative firmware patch authorized.
- [x] Published-image deployment — **PASS**; exact signed package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, bootloader prestate `RecoveryIdle` floor/committed `1/1`, exact END `Ok/Committed` floor/committed `2/2`, published application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, metadata B version `2` / marker `0xA55A`, bootloader/persistence unchanged.
- [x] Published runtime verification — **PASS**; source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, runtime + HELLO capabilities `0x0000007F`, `PING=PONG`, health OK, rollback not attempted.
- [x] Physical bench == published v2 — **PASS**; post-deploy whole-Flash SHA-256 `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`; authoritative deployment evidence SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`.

Canonical design: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`.

Canonical acceptance: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`.

### Project-state pointer

This historical ledger does not own current project disposition. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` is published and its physical deployment closure is complete; there is currently no active product feature boundary. See `docs/CURRENT_STATE.md` for the authoritative state. When a new boundary is explicitly promoted, its dedicated `*_PLAN.md` / `*_ACCEPTANCE_PLAN.md` pair owns the gate checklist.

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

After decomposition publication, the next feature boundary was `USB_MANAGEMENT_DEVICE_FOUNDATION`, followed by `HOST_CONTROL_APPLICATION_FOUNDATION`; both are now accepted/published. The subsequently selected Asset/Configuration boundary is tracked as active/current only in `docs/CURRENT_STATE.md`.

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

This file is the historical execution/checklist ledger for the STM32 OS project. Global current state is owned by `docs/CURRENT_STATE.md`; unchecked historical items are not automatically active work.

## Accepted hardware baseline

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
- [ ] Deferred candidate: Slice 4B configurable UI layout + status bar — the narrow console-clip configuration/persistence subset was promoted and published by `ASSET_CONFIGURATION_TRANSFER_FOUNDATION`; broader presets/style/status-layout customization remains deferred unless separately promoted.
  - [x] 4B.0: initial status architecture/acceptance plan.
  - [x] 4B.0a: static status prototype protocol/isolation proof.
  - [x] 4B.0b: visually reject hardcoded full-frame composition; do not commit it.
  - [x] 4B.0c: define configurable layout/preset/customization plan.
  - [x] 4B.1 narrow substrate: validated `oled_ui_layout` module exists and is consumed by the published v1 console-clip configuration; this does not activate the broader preset/style plan.
  - [ ] 4B.1: add `minimal`, `boxed`, `compact` presets.
  - [ ] 4B.1: move borders/separator/regions into layout data.
  - [ ] 4B.1: keep 21x3 console with glyph-based horizontal fit.
  - [ ] 4B.1: preserve accepted SYSTEM/USB/NETWORK/uptime semantics while allowing layout/style changes.
  - [ ] 4B.1: add runtime preset selection.
  - [ ] 4B.1: physically compare at least minimal vs boxed.
  - [ ] 4B.1: accept one or more presets, then commit/push.
  - [ ] 4B.2: add `ui show` and validated RAM-only `ui set`.
  - [ ] 4B.2: prove atomic rejection of invalid custom layouts.
  - [x] 4B.3 uptime `HH:MM` objective — implemented and hardware-accepted in `BOOT_DESKTOP_UI_FOUNDATION`.
  - [x] 4B.3 visible-change update objective — implemented and later optimized in `OLED_DIRTY_REGION_OPTIMIZATION`; accepted semantics use real SYSTEM/USB state rather than the old prototype COMM label.
  - [ ] 4B.4: define bounded host interchange/converter/configurator protocol if this deferred work is promoted.
  - [ ] 4B.4: use shared host Core/RPC over the accepted management transport; do not define a UART-first UI contract.
  - [ ] 4B.4: keep UART/CDC diagnostic transports independent of UI configuration semantics.
  - [x] 4B.5 narrow v1 persistence rule: published console-clip persistence consumes the accepted Asset/Configuration A/B contract; no UI-specific parallel Flash format was introduced.
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

Runtime configuration is transport-independent. Historical UART-first planning has been superseded by the published USB CDC and USB management foundations; UI/config semantics must remain independent of whether the accepted carrier is UART, CDC or the primary management transport:

```text
UART / USB CDC diagnostics / USB management
        -> shared target configuration semantics
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
- [x] Narrow `OLED_UI_LAYOUT_CONFIG_V1` persistence — published by `ASSET_CONFIGURATION_TRANSFER_FOUNDATION`; broader runtime/custom layout persistence remains deferred with the broader UI customization work.
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
