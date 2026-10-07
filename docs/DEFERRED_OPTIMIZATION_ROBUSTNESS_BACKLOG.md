# Deus OS — Deferred optimization, robustness and diagnostics backlog

Status: **CANONICAL DEFERRED POLICY — REVIEW BY MEASUREMENT/TRIGGER, NOT BY SPECULATION**

This document consolidates deferred engineering work identified across accepted boundaries, beginning with `OLED_DIRTY_REGION_OPTIMIZATION` and including the post-publication Firmware Update / Bootloader audit. None of the items below authorizes immediate implementation by itself. Each item requires a real consumer, measurable pressure, or a dedicated independently accepted boundary.

## 1. Optimization triggers

### I2C async / IRQ / DMA

Keep the current synchronous bounded I2C path while dirty-region updates keep transfer cost low. Revisit an IRQ/DMA path only if measured blocking I2C latency or bus occupancy becomes material after additional UI/sensor consumers. Do not introduce a generic DMA framework solely because the peripheral supports DMA.

### Scheduler ready-set scaling

Keep the current bounded O(N) ready scan while production task count remains small; the accepted production topology currently has two tasks. Revisit bitmap/priority-set structures when task count grows materially (roughly 8–16+ is a practical review trigger) and measured scheduler CPU/latency overhead justifies the extra complexity.

### CRC acceleration

Keep bitwise CRC-16/CCITT-FALSE for low-rate control RPC. Consider table-driven or hardware-assisted CRC only if asset/configuration/update streaming benchmarks show CRC CPU cost is material enough to justify extra Flash/data/code.

### WinUSB / streaming copy policy

The published USB-management and Asset/Configuration paths, plus future update streaming, should prefer bounded batches, minimal-copy flow and explicit backpressure. Avoid whole-payload duplication in SRAM. The accepted framed RPC semantics remain above transport.

### Asset / configuration / update streaming

Use bounded chunks, explicit backpressure and incremental validation/storage. Do not buffer complete files/images/packages in SRAM. Minimize copies between transport, validation and storage/update consumers.

### Tickless / low-power

Defer tickless idle and broader low-power work until there is a concrete power requirement and application timing constraints are known. Keep the 1 kHz SysTick deterministic baseline meanwhile.

### Continuous resource budgets

Continue measuring Flash, static SRAM and per-task stack high-water at acceptance gates. Prefer removing unnecessary work/data before adding optimization complexity; do not micro-optimize without a measured bottleneck.

## 2. Robustness and latent-bug verification

As boundaries gain long-lived state or transport/storage complexity, add targeted evidence rather than speculative kernel mechanisms:

- soak/stress runs for long-lived runtime paths;
- I2C NACK/stuck-busy and partial-transfer fault injection;
- transport disconnect/reconnect and reset/recovery fault injection;
- wraparound/boundary tests for time, counters, request IDs and ring arithmetic;
- host-side fuzzing for binary-frame lengths, flags, CRC and malformed sequencing;
- power-loss/interrupted-write recovery once persistence/filesystems/update exist;
- race/order tests around IRQ publication and task wake/block/timeout coincidence.

## 3. Structured observability policy

Normal production logging should remain lightweight and bounded. Preferred future design:

- a static binary event-trace ring, no heap;
- compact records: monotonic tick/timestamp + event ID + bounded numeric arguments;
- no `printf` or string formatting in the event-write hot path;
- nonblocking writes with an explicit overflow/drop counter;
- on-demand read/drain through the management protocol;
- only a bounded critical crash/reset snapshot retained across reboot through a `.noinit`/backup-style record;
- no continuous ordinary logging to internal Flash because of latency and wear.

Existing UART/USB command output, emergency UART, fault state, RX drop/error/high-water counters, scheduler stack high-water and watchdog diagnostics remain engineering observability mechanisms.

## 4. Test / diagnostic build-profile policy

Do not assume every acceptance/reference self-test must live permanently in the production image. Classify tests into three groups:

1. host/build-time tests — never linked into target firmware;
2. acceptance diagnostics — target/hardware verification, eligible for exclusion from a later production profile;
3. production health diagnostics — small bounded mechanisms retained in shipping firmware.

Heavy reference/self-tests should migrate toward host/build-time or acceptance-only profiles when a stable production build profile exists. A production binary must receive its own final hardware smoke/acceptance; do not fully test one image and ship different unverified bytes. Continue GCC stack-usage analysis for target-resident diagnostics and enforce explicit stack ceilings.

## 5. External storage and memory expansion policy

Treat nonvolatile storage separately from RAM.

Viable STM32F103-class directions:

- SPI NOR Flash for configuration, assets and update staging;
- microSD over SPI for large removable storage;
- FRAM/EEPROM for small frequently written state where appropriate;
- introduce a bounded block-device abstraction before a filesystem;
- if removable-media files become a real requirement, read-only FAT32 is a reasonable first step before writable semantics and power-loss recovery are accepted.

The current STM32F103 USB path is device-only; consuming commodity USB flash drives requires USB Host capability through an external host controller or a future MCU/platform with USB OTG/Host.

Laptop DDR/DDR2/DDR3 SO-DIMM is not a practical direct expansion for STM32F103: the MCU has no DDR controller and cannot satisfy the required bus width, clocks, signaling, training or refresh. If more RAM becomes a real requirement, evaluate SPI/QSPI PSRAM for modest expansion or move to a future MCU with FMC/external SDRAM support. Hundreds of MB/GB imply a different platform class.

## 6. Repository EOL and operator-log hygiene

Repository line endings are policy, not a developer-machine preference. `.gitattributes` owns LF for source/docs/linker/assembly text so Windows `core.autocrlf` does not flood evidence with conversion warnings.

Known LF->CRLF Git warnings may be suppressed from operator-facing logs only when exact raw stderr is still preserved in evidence. Other stderr must remain visible. Perform normalization deliberately; do not mix unrelated EOL churn into firmware changes.

## 7. Post-decomposition architecture debt

The accepted `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` is a material ownership improvement, not a claim that `src/kernel.c` has reached its final composition-root form. The accepted implementation deliberately stopped before creating speculative abstractions or exposing root-private state merely to reduce line count.

This section records residual architecture debt provenance. The 2026-10-03 audit promoted composition-root convergence (`FDC-05`), system/service-state dependency direction (`FDC-06`) and application stop-failure semantics (`FDC-07`) into mandatory closure obligations. As of the 2026-10-07 Gate-5 reconciliation, those three obligations are accepted through Gates 0–5 on exact candidate `bb99acf111dfa3a78193b4e5d3376fa077defa1e`; only their Gate-6/7 publication remains. The other architecture notes remain trigger-driven unless separately promoted:

### Composition-root convergence

FDC-05 has accepted the concrete low-level convergence needed by the mandatory closure: RCC/clock, USART1, I2C1 and PC13 status LED now have natural bounded owners. Remaining possible root work is intentionally trigger-driven rather than an open FDC-05 requirement:

- transport-neutral command execution/domain dispatch that remains rooted in historical `console_*` naming/organization;
- production task/liveness glue only if a future bounded owner removes another concrete reason to change the root.

Do not reopen already-accepted low-level ownership or invent a catch-all context merely to reduce `src/kernel.c` line count.

Do not reopen decomposition merely to reduce line count. Revisit this debt when a new feature would otherwise add another independent reason to change `src/kernel.c`, when host/management work would duplicate domain dispatch, or when low-level platform code blocks clean driver/service layering.

Historical `console_*` naming on transport-neutral dispatch is not itself a functional defect and is not sufficient reason for a rename-only boundary.

### Host Core client decomposition — resolved

The Asset boundary originally grew `DeusDeviceClient` into a protocol-ownership hotspot. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` Gate 1 promoted and completed the needed decomposition: one shared `DeviceProtocolChannel` owns transport/decoder/request correlation/gating, while `DeusRpcClient`, `AssetTransferClient` and `FirmwareUpdateClient` own their transaction domains and `DeusDeviceClient` remains the compatibility/application facade.

Do not re-merge these responsibilities into a monolithic client. Any future protocol domain must reuse the shared channel/correlation owner rather than create a second transport decoder, independent request-ID allocator or competing lock.

### System/service state must remain upstream of presentation

FDC-06 has accepted the intended dependency direction on exact candidate `bb99acf111dfa3a78193b4e5d3376fa077defa1e`:

`runtime/time/USB facts -> system_service_state -> application runtime + OLED presentation`.

`application_service_snapshot_t` and OLED status composition now consume the same semantic owner; rendered indicator values are no longer authoritative for system health, USB or network truth. A future second renderer may still justify presentation refactoring, but semantic service-state ownership itself is no longer open debt.

### Application runtime bridge presentation coupling

`application_runtime_bridge` currently owns both runtime/event integration state and adaptation of the application view into `oled_console_t`. This is acceptable while OLED is the only target presentation consumer and avoids a speculative renderer abstraction.

If a second renderer/presentation consumer appears, split runtime/service ownership from OLED-specific presentation adaptation instead of expanding the bridge into a generic UI god object.

### Scheduler diagnostic binding lifetime

`scheduler_diagnostics_execute_diagnostic()` borrows one binding for the synchronous diagnostic execution, including nested diagnostic scheduler tasks. The module-static binding pointer is therefore valid only under the current serialized/non-reentrant diagnostic contract.

Do not reuse this mechanism as a concurrent diagnostic service, retain the borrowed binding beyond the call, or allow unrelated concurrent diagnostic execution. If diagnostics become concurrent/long-lived, introduce an explicit bounded ownership model in a dedicated reviewed boundary.

### Application stop-failure semantics

FDC-07 has frozen and accepted fail-closed stop semantics before any resource-owning application exists. If current-app `stop()` fails, the state becomes `FAILED`, fault count increments once, `active_id` remains the unresolved owner, and replacement/home start does not occur. The deterministic synthetic failure proof and existing built-in lifecycle hardware regression are accepted on the shared candidate. Future resource-owning applications must preserve this ownership rule or reopen it explicitly.

### Firmware Update / Bootloader post-publication robustness

The 2026-10-01 post-publication read-only audit found no acceptance-regression blocker and retained four bounded robustness items. The 2026-10-03 architecture/code audit subsequently **promoted the relevant items into mandatory `FDC-01` / `FDC-08` closure obligations** before the next service/Web/network feature boundary. This promotion still does not authorize a speculative all-at-once patch: each implementation slice requires a frozen source boundary and acceptance proof.

- **Authenticated image-span vector containment — accepted in FDC-08.** Candidate bootloader validates the reset-handler Thumb address inside `[APP_BASE, APP_BASE + image_length)` rather than merely the whole executable region; Gate-1..3 deterministic/static proof and final Flash acceptance are bound to the exact candidate.
- **Boot/runtime clock and UART wait bounds — accepted in FDC-08.** HSE/PLL/switch and USART1 TX waits are finite with explicit owner failure semantics; hardware acceptance proves normal operation while deterministic/static seams own unsafe oscillator-failure proof.
- **Non-DATA firmware-update timeout restart semantics — accepted end-to-end in FDC-08.** INFO/BEGIN/AUTHORIZE/END retain no blind request retry; DATA retains one exact immediate retry. Consolidated hardware evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` proves authenticated update/recovery, ambiguity adjudication and unchanged authenticity/rollback/persistence ownership.
- **Stale request-ID lifetime under repeated cancellation — resolved by `FDC-01`.** Published FDC-01 commit `c863b5ab9d00ab96de7c8f8275f905ed52c8740e` replaced the earlier ad-hoc stale-ID description with one channel-owned abandoned-request registry. Unresolved abandoned IDs cannot be reused; delayed single responses or RPC streams retire through their frozen terminal semantics; request-ID wrap with unresolved abandonment fails closed and requires a fresh session. Deterministic wrap/multi-abandonment tests are part of the accepted `51/51` Core result.

Historical INFO/BEGIN/DATA response timeouts remain observations, not proof of an additional firmware USB defect. Do not introduce a speculative firmware patch without a causal reproducer.

## 8. Promoted mandatory closure obligations — 2026-10-03 audit

The items below are retained as historical provenance. The full `FDC-01..FDC-10` mandatory closure program is now accepted closed; none of these entries remains active work. Future feature selection still requires an explicit new boundary.

1. **`FDC-01` — generic RPC timeout/cancellation correlation — CLOSED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`.** One channel-owned abandoned-request policy now drains delayed multi-frame `RPC_DATA...RPC_END`, bounds request-ID reuse by failing closed at unresolved wrap, preserves unknown non-stale mismatch failures, reconciles firmware-update abandonment and preserves explicit no-blind-retry semantics for non-DATA operations. Gate-2 evidence SHA-256 `992A3C38908BC6F5E0E40EA844612A235F7DF2A5960184B6CA19A2F1B1DDEEE6`, Core `51/51`.
2. **`FDC-02` — session event reentrancy — CLOSED / PUBLISHED `42245d9d71504482fb189d8351ecce7049542145`.** One per-session serialized asynchronous notification chain removes public callbacks from the lifecycle critical section; callback/recovery/dispose/unsubscribe regressions pass in Core `58/58` with evidence SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`.
3. **`FDC-03` — management service operation allowlist — CLOSED / PUBLISHED `0d9adfd8d0ed11478194e2268ede3c57379c8294`.** The Core-owned typed service facade/catalog contains exactly eight operations (5 ReadOnly / 3 Control / 0 Destructive); numeric RPC IDs/flags/unlisted commands and bootloader/update routing are absent. Gate-2 evidence SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`, Core `66/66`, Core/CLI/Desktop Release PASS.
4. **`FDC-04` — typed Core management models — CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`.** Core owns bounded typed Ping/Health/app-control parsing; service/Desktop consume those models without raw `RpcResult`/`OutputText`, CLI raw output is confined to excluded `rpcinfo`, and compatibility/operator raw APIs remain. Gate-2 evidence SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`, Core `78/78`, Core/CLI/Desktop Release PASS.
5. **`FDC-05` — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.** Natural clock/USART1/I2C1/PC13 ownership convergence is accepted without universal context/service-locator/hidden-state coupling.
6. **`FDC-06` — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.** `system_service_state` is the accepted semantic authority upstream of both application-runtime and OLED consumers; UI indicators no longer reconstruct system truth.
7. **`FDC-07` — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.** Failed stop preserves unresolved ownership/`active_id`, marks `FAILED`, increments fault count once and starts no replacement; synthetic + hardware lifecycle proof accepted.
8. **`FDC-08` — CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`.** Authenticated image-span vectors, bounded boot/runtime/UART waits, wrap-safe reset fallback and explicit non-DATA adjudication are accepted; Gate-4 final hardware evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` proves update/recovery and exact trust/persistence ownership.
9. **`FDC-09` — native transport cancellation/disposal — CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73`.** `HOST_NATIVE_TRANSPORT_LIFETIME_HARDENING` implements per-instance latched bounded cancellation for synchronous WinUSB/libusb calls: an entered native call drains under the existing 2000-ms transfer timeout before cancellation completes, Dispose blocks new I/O and closes handles only after drain, runtime/bootloader reuse the same transport owners, deterministic isolation tests pass, and Linux/Windows real-platform reopen proof is accepted.
10. **`FDC-10` — documentation/source-of-truth reconciliation.** Remove obsolete future-tense/deferred statements contradicted by published USB CDC, persistence, host management and firmware-update work; preserve clearly labelled historical chronology; reconcile current-state/roadmap/backlog/scoped addenda; run repo-wide stale-token/open-checkbox and `git diff --check` audits.

After all ten are accepted closed, this section remains as historical provenance and the active disposition moves back to future feature selection.

### Anti-goals for all follow-up work

None of the debt above authorizes a speculative generic HAL, universal `kernel_context_t`, service locator, heap, dynamic allocation, generic queue/mutex/timer framework, new task, or framework-only refactor. Ownership must move only with a concrete reason-to-change and bounded state.

These items did **not** block the subsequently published `USB_MANAGEMENT_DEVICE_FOUNDATION`, `HOST_CONTROL_APPLICATION_FOUNDATION`, `ASSET_CONFIGURATION_TRANSFER_FOUNDATION`, `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` or `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`. The Host Core decomposition trigger was resolved inside the Firmware Update / Bootloader boundary. As of 2026-10-07, `FDC-01..04` and `FDC-09` are CLOSED/PUBLISHED; `FDC-05..08` are accepted through Gate 5 on the exact shared candidate and await only local commit/publication; `FDC-10` remains the final documentation closure after that publication. All other backlog material remains trigger-driven merely by appearing here.

## 9. Roadmap placement

This backlog does not own current roadmap state or activation. `docs/CURRENT_STATE.md` is authoritative for the active boundary and `docs/ROADMAP.md` owns forward sequencing.

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` is published at `562e786ffa734da055c23144ec4256bc8961bbaf`; `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` at `a8f92f83c2ba8917ad183b1a099c9e21199c9463`; and `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`. No new product feature boundary is active. `FDC-01..04,09` are CLOSED/PUBLISHED; `FDC-05..08` have accepted Gates 0–5 and are at commit/publication; `FDC-10` is next after that publication. Unrelated storage, networking/security, optimization and observability debt remains consumer-driven until separately promoted by `CURRENT_STATE.md` plus a dedicated plan/acceptance contract.

Optimization rule: remove unnecessary work first, measure next, add complexity only against an observed bottleneck.
