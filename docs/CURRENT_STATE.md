# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

Git is authoritative for live repository bytes, commit/tree identity, branch state and cleanliness. This file records accepted product-boundary identities and current project disposition, not a substitute for `git status` / `git rev-parse`.

For architecture, roadmap sequence, scoped boundary contracts, evidence policy and historical records, follow `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED / PHYSICAL DEPLOYMENT VERIFIED**

Boundary publication commit:

`27fb10288ef45dcc9292287603e5ab8a26bf1fcb`

Boundary publication tree:

`0eca476d84eb1f06b633a7780b7883a3fadd8adb`

Gate-7 ordinary non-force publication evidence SHA-256:

`B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`

Gate-7 publication proved:

- ordinary non-force fast-forward of `main`;
- post-push `HEAD == origin/main == FETCH_HEAD == 27fb10288ef45dcc9292287603e5ab8a26bf1fcb`;
- clean worktree/index and ahead/behind `0/0`;
- GitHub `main` at the same commit/tree;
- verified GitHub commit signature;
- zero target I/O.

Post-publication physical deployment is also accepted from `stm32_os_published_deploy_adjudicated_retry_v7_20261003_141252.evidence.zip`, SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706`. It proves the exact signed published-v2 package SHA-256 `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, exact published application SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, bootloader prestate `Ok/RecoveryIdle` with version floor/committed version `1/1`, successful END response `Ok/Committed` with floor/committed version `2/2`, committed metadata B version `2` with marker `0xA55A`, unchanged bootloader/persistence ownership, runtime source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, runtime/HELLO capabilities `0x0000007F`, `PONG`, health OK and `ROLLBACK_ATTEMPTED=False`. Post-deploy whole-Flash SHA-256 is `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`.

The accepted Gate-6 publication candidate is:

- firmware source tree: `8323c68c931894441ae4db9138ba3838f35bb8b6`;
- host source tree: `c95019bedfb6223705e6eba4f4c6d310b1701cdc`;
- firmware BIN SHA-256: `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`;
- application Flash/SRAM: `53212/10956`;
- system capability mask: `0x0000007F`;
- Host Core tests: `41/41`;
- Host Transport tests: `12/12`;
- Core/Windows/Linux Release builds: PASS;
- dummy-key bootloader resource replay: Flash `5904/8192`, `.data+.bss 660/1024`, conventional SRAM `1684/2048`;
- Gate-6 validation evidence SHA-256: `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7`;
- Gate-6 local acceptance continuation SHA-256: `806D999F1F9677A7E3A3C9D6E0D030079612978561C18A69167A0890A2EBC1CC`.

Canonical scoped records:

- design: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`;
- acceptance: `docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`;
- wire/image ABI: `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`.

## 2. Current product architecture

The published firmware-update layout is realized as:

```text
0x08000000..0x08001FFF  bootloader / recovery, pages 0..7, 8 KiB ceiling
0x08002000..0x0800EFFF  relocated executable application, pages 8..59, 52 KiB ceiling
0x0800F000..0x0800F3FF  authenticated firmware metadata A, page 60
0x0800F400..0x0800F7FF  authenticated firmware metadata B, page 61
0x0800F800..0x0800FBFF  persistent configuration slot A, page 62
0x0800FC00..0x0800FFFF  persistent configuration slot B, page 63
```

Stable accepted update properties:

- reset owner is the bootloader at `0x08000000`;
- application vectors/VTOR are at `0x08002000`;
- runtime USB remains private-test `1209:000C`; bootloader recovery/update USB is private-test `1209:000D`;
- package v1 is 48-byte header + 32-byte HMAC-SHA-256 authenticator + relocated application bytes;
- device enforces product/target/version/authentication/digest/vector checks;
- metadata commit marker `0xA55A` is programmed last;
- update DATA is sequential, with only the exact immediately previous DATA retry accepted idempotently;
- interrupted executable programming falls back to recovery and host retry restarts from BEGIN/offset zero;
- bootloader update writes do not own persistence pages 62/63;
- production host entry defaults to `PublishedOnly` and requires system capability bit 6.

Gate-5 hardware acceptance covers normal boot, explicit update entry, invalid-application recovery, malformed header, wrong target, bad digest, bad authenticator, rollback rejection, deterministic reset interruption, physical native-USB/VBUS power interruption, full retry, exact application/metadata ownership, unchanged bootloader/persistence ownership and deterministic handoff/VTOR.

The post-Stage-10 host reliability repair is accepted. It handles delayed responses from explicitly timed-out firmware requests without weakening unknown request-ID correlation failures, and maps Windows WinUSB pipe timeout codes `121/1460` to `HostErrorKind.Timeout`. No second firmware-side response-loss mechanism is claimed from the historical INFO/BEGIN/DATA timeout observations.

## 3. Historical FDC-05..08 published source and 2026-10-07 hardware snapshot

**Historical snapshot, superseded by RDC-06:** as of the 2026-10-07 FDC-05..08 hardware acceptance, the most recent target/source closure was `6aa2df19ab02c14bde38833e738fe825008102e8`. The last named product-feature boundary remains `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` at `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`; FDC-05..08 were post-publication architecture/lifecycle/robustness hardening. The current accepted source and physical STM32 revision **5**, superseding this historical snapshot, are recorded in §4 below under **Current physical STM32 state (RDC-06 accepted 2026-10-10)**.

At that 2026-10-07 acceptance, the physical bench was aligned with the FDC-05..08 target state (historical v3 firmware; **not** the current deployed revision 5):

- accepted source tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`;
- application `51972/53248` bytes, SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`;
- runtime capability mask remains `0x0000007F`;
- metadata version `2` is preserved and version `3` is committed with marker `0xA55A`;
- bootloader private exact compare PASS and the update key remains preserved/not evidenced;
- persistence pages 62/63 are byte-exact unchanged from prestate;
- signed application bytes, erased final touched-page remainder and untouched executable tail are exactly adjudicated;
- final runtime smoke, UART/IWDG health, I2C `0x3C`, OLED command and physical OLED observation PASS;
- final target disposition is the published candidate-v3 bytes; rollback was not attempted.

Authoritative consolidated hardware evidence is `stm32_os_fdc05_08_consolidated_hardware_gate4_v18_20261007_115017.evidence.zip`, SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`. Gate 6 committed the exact accepted source/docs set at `6aa2df19ab02c14bde38833e738fe825008102e8`; Gate 7 used an ordinary non-force push and fresh fetch proved `HEAD == origin/main == FETCH_HEAD` at that commit with clean ahead/behind `0/0`.

## 4. Current work disposition

There is **no active new product feature boundary**. `FDC-01..FDC-10` are CLOSED/PUBLISHED; FDC-05..08 share publication commit `6aa2df19ab02c14bde38833e738fe825008102e8`, and the final docs-only FDC-10 publication is `9c02e27f50349c9a0240ab24d34720580c3f3269`. Consolidated FDC-05..08 Gate-1..3 evidence SHA-256 is `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` and final hardware Gate-4 evidence SHA-256 is `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`. A new mandatory **RESIDUAL_DEBT_CLOSURE_PROGRAM** is now active on published baseline `d649e133cbe39fa6155520c657136ca99cee61ae`; no new product feature may be promoted until all confirmed residual technical/tooling/maintainability/documentation debt in that program is CLOSED/PUBLISHED.
Program governance is published at `2fb687d4bd57e8029e89d450038b712acb6d9dea`; fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`.

The 2026-10-03 post-publication architecture/code audit promoted a mandatory **FOUNDATIONAL_DEBT_CLOSURE_PROGRAM** before any new Host Management Service/Web/network feature boundary. That ten-item program is complete: FDC-01..04 and FDC-09 are CLOSED/PUBLISHED at their accepted commits, FDC-05..08 are CLOSED/PUBLISHED together at `6aa2df19ab02c14bde38833e738fe825008102e8`, and FDC-10 is CLOSED/PUBLISHED docs-only at `9c02e27f50349c9a0240ab24d34720580c3f3269`. A subsequent read-only audit plus the accepted Host Asset correlation hardening identified additional concrete residual debt. Those confirmed items are now promoted into `RESIDUAL_DEBT_CLOSURE_PROGRAM`; feature selection remains blocked until its RDC ledger and final documentation sweep are closed.

The mandatory closure ledger uses IDs `FDC-01..FDC-10`:

1. `FDC-01` — **CLOSED / PUBLISHED `c863b5ab9d00ab96de7c8f8275f905ed52c8740e`** — generic RPC timeout/cancellation delayed-response correlation, multi-frame abandoned-RPC cleanup, bounded request-ID reuse/wrap behavior and firmware-update request-level no-blind-retry policy;
2. `FDC-02` — **CLOSED / PUBLISHED `42245d9d71504482fb189d8351ecce7049542145`** — one serialized per-session asynchronous notification chain removes subscriber execution from the lifecycle critical section; Gate-2 evidence SHA-256 `E01040E5D0C896BA966752C38FEC889AFFC44D64B5FD943BC28067D6C21A62DB`, Core `58/58`, Core/Desktop Release builds PASS;
3. `FDC-03` — **CLOSED / PUBLISHED `0d9adfd8d0ed11478194e2268ede3c57379c8294`** — one Core-owned compile-time service catalog/facade exposes exactly five ReadOnly + three Control + zero Destructive operations; raw numeric RPC/flags, bootloader/update and unlisted routing are absent; Gate-2 evidence SHA-256 `32C57FBA8AE04B9FAA9A4456FA846C3BD58ED9BD05D242FD00280B634A737D25`, Core `66/66`, Core/CLI/Desktop Release PASS;
4. `FDC-04` — **CLOSED / PUBLISHED `3fcd93f3e038323bbcc33c136a3ab4ba1f605e5d`** — bounded Core typed management models are accepted; service/Desktop management paths expose no raw `RpcResult`/`OutputText`, CLI raw `OutputText` is confined to excluded `rpcinfo`, low-level compatibility APIs remain intact; Gate-2 evidence SHA-256 `C6201371B0D302B964F8D24B8A413E5CDEC82FF93EF1056C54A8776B8E5171C4`, Core `78/78`, Core/CLI/Desktop Release PASS;
5. `FDC-05` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`** — natural clock/USART1/I2C1/PC13 owners accepted without universal context/service-locator/hidden-state coupling;
6. `FDC-06` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`** — `system_service_state` is the accepted upstream authority for application and OLED consumers; equal-state/no-event and physical OLED regression accepted;
7. `FDC-07` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`** — failed application `stop()` preserves unresolved ownership/`active_id`, marks `FAILED`, increments one fault and starts no replacement; deterministic + hardware lifecycle proof accepted;
8. `FDC-08` — **CLOSED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`** — bounded boot/runtime clock and UART waits, authenticated image-span vectors, wrap-safe reset deadline and non-DATA adjudication accepted; hardware evidence proves authenticated v3 update/recovery and exact final Flash/metadata/persistence ownership;
9. `FDC-09` — **CLOSED / PUBLISHED `b88a9eee43095665326787cc0345822218c1ba73`** — per-instance latched bounded native cancellation/drain ownership is accepted for WinUSB/libusb; deterministic Transport `24/24` + Core `78/78`, Linux real-platform and Windows WinUSB cancel/disconnect/reopen proofs are accepted;
10. `FDC-10` — **CLOSED / PUBLISHED `9c02e27f50349c9a0240ab24d34720580c3f3269` (DOCS-ONLY)** — repo-wide stale/open-checkbox/source-of-truth/artifact/security hygiene reconciliation completed the ten-item foundational-debt program without product/target mutation.

The completed FDC program was sequenced as: host long-lived-session hardening (`FDC-01..04,09`), target architecture/lifecycle cleanup (`FDC-05..07`), target/update robustness closure (`FDC-08`), then documentation/source-of-truth reconciliation (`FDC-10`). Those FDC prerequisites are satisfied, but they are no longer the complete pre-feature gate: `RESIDUAL_DEBT_CLOSURE_PROGRAM` is now the active mandatory maintenance sequence. `HOST_MANAGEMENT_SERVICE_FOUNDATION` or any equivalent Web/service/network feature remains blocked until RDC-01..RDC-08 are CLOSED/PUBLISHED.

**Current physical STM32 state (RDC-06 accepted 2026-10-10):** the board runs the exact authenticated application firmware revision **5** (application SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, source candidate tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`). Independent full 64-KiB HOTPLUG Flash readback validates the authenticated revision floor `5`, unchanged bootloader/persistence ownership across the application update, and normal USB runtime `1209:000c`. The separate STM32 USB endpoint CTR race repair is published in `83e57f609bab4bcc8a61ea2222629bf9066e910d`; its replacement bootloader passed 72/72 paced/burst INFO hardware regression and the subsequent signed v5 update. USB/RPC/`help`/scheduler/IWDG and Windows CH340/USART1 `115200 8N1` `help` (36 commands) passed; the operator explicitly reported `PHYSICAL_OLED=PASS`. Gate-3 final evidence collation SHA-256 `F38EFC5697A26E93B7381B3ACE5DEE225CE826053B4C2B68C5F08279EEBF13AD` (evidence-only, no new MCU execution). Firmware security revision 5 is **not** an OS semantic version.

Host-management presentation evolution remains a non-authorizing direction in `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`.

The host-only maintenance boundary `HOST_ASSET_REQUEST_CORRELATION_HARDENING` is CLOSED/PUBLISHED at `40aab02b4e0c04466453be4c31041f8649c13c1b` on baseline `0713da56a3fe7a6b62f4c548d3248b4ed7fb2e57`. Gate-2 evidence `stm32_os_host_asset_request_correlation_gate2_host_validation_v2_20261007_181012.evidence.zip`, SHA-256 `F574489FE68B35D42DEB8BB0E8EE88FB029B8FAAED5C7F926BB7194C256D3285`, proves Core `80/80`, warning-clean Core Release build, exact candidate/poststate binding, target I/O NONE and Flash mutation NONE. The accepted fix reuses the existing channel-owned single-response abandonment model for timed-out/cancelled Asset requests and changes no target/native-transport/script/wire contract. The separate Asset transaction/session cleanup question is now promoted as mandatory `RDC-01`, not left as an unowned future possibility.

`RDC-01 / HOST_ASSET_TRANSACTION_RECOVERY_HARDENING` is CLOSED/PUBLISHED at `699382a58c4c9570cdf36a05693044461e87c58b` on baseline `7ba9b303b176f8329be98f29f3f1ce1e6ad5e510`. Gate-2 evidence `stm32_os_host_asset_transaction_recovery_gate2_host_validation_v1_20261007_190709.evidence.zip`, SHA-256 `F63A788E08FEBDD4454A7BF3AD217ABF009B1BD24413EB83F51C7AD6D71E95FA`, proves Core `84/84`, warning-clean Release build, exact candidate/poststate binding, target I/O NONE and Flash mutation NONE. The accepted Host cleanup issues one bounded best-effort same-transfer `ABORT` only while a volatile session may be active, preserves the primary failure, performs no mutation retry and changes no target/native-transport/script/wire contract. The evidence bundle's stale `CORE_STATIC_TEST_METHODS=80` summary literal is reconciled as `HARNESS-EVIDENCE-RECORD-CARDINALITY-01`; manifest, run log and executed MTP output all prove `84`.

`RDC-02 / HOST_DEAD_SURFACE_CLEANUP` is CLOSED/PUBLISHED at `09a432f6c0b73ef2425add5950b6f6d5dee3733d` on baseline `bc562f5030990d4b4e69af35d0ad1a3e4cb301bb`. Gate-2 evidence `stm32_os_host_dead_surface_cleanup_gate2_host_validation_v1_20261007_193632.evidence.zip`, SHA-256 `44C5DBFA295087F1F2BFD5A453E63C3855EBC0508EEC5D3BEF283F99D4482B84`, proves Transport `24/24`, Core `84/84`, warning-clean Linux transport/Core Release builds, exact candidate/poststate binding, target I/O NONE and Flash mutation NONE. Exactly eight unreachable Linux transport remnants were removed; profile-aware ownership, public runtime constants and retained compatibility/protocol APIs remain unchanged.

`RDC-03 / TARGET_DEAD_API_CLEANUP` is CLOSED/PUBLISHED at `e516fdc1d8818007db40ee12669d28f3f828c489` on baseline `5b0f6b9586ac5397c877efb2a83601a43a080f19`. Exactly ten dead target APIs were removed while bridge/OLED retained owners and source/protocol compatibility `binary_frame_encode()` remained. Gate 2 accepted candidate tree `3bdb90d3f9b7e270d32bad58b9c639c40b97150b`, BIN `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016`, Flash/SRAM `51972/10968`, global symbol/layout/stack invariants and non-provenance BIN differences `0`. Gate 3 exact-candidate hardware/runtime acceptance plus read-only continuation prove exact application readback, unchanged bootloader/persistence ownership, USB/UART/scheduler/IWDG/application liveness and `PHYSICAL_OLED=PASS`. Publication was an ordinary non-force fast-forward and fresh fetch proved clean `HEAD == origin/main`, ahead/behind `0/0`.

**Completed maintenance boundary:** `RDC-04 / TOOLING_OUTPUT_DIRECTORY_SAFETY`, audit baseline `e516fdc1d8818007db40ee12669d28f3f828c489`. Confirmed current tooling debt is caller-controlled recursive output-directory deletion in `scripts/build_firmware.ps1` and `scripts/create_bootloader_recovery_bundle.ps1`; historical-only `scripts/create_asset_recovery_bundle.ps1` carries the same footgun and remains in safety scope without being re-promoted as current recovery tooling. Gate 0 design/scope freeze is published. Gate 1 source implementation/static review is **PASS** (evidence SHA-256 `F348C1A83311A98AB6E8CCD8F68FC317F692858708BF5021190E61F6D907A7E1`). Gate 2A PowerShell 7 disposable destructive-safety matrix is **PASS 27/27** (evidence SHA-256 `3DD73736F772BD42F5534FB517FCA503F0BD768CE27C2C3A8E119ED40D811C50`). Gate 2B clean baseline/candidate firmware and both recovery-generator deterministic equivalence is **PASS** (evidence SHA-256 `F3534E3ED6510CA83A6EAE1362CF53E7A0EA5FCA79CCCCCB7B52366C72A49825`); exact candidate tree `315d1b67194ba128b25afda8b7950eb371e998f4`, normalized firmware BIN equals accepted RDC-03 BIN `F0DA0AA44388D181426D9222C955649D573BBAE7BE0A610CDA9439A835C44016` after only the 40-byte provenance substitution; Flash/SRAM unchanged `51972/10968`. Historical Asset tooling is not operationally reauthorized. Gate 2C docs finalization **PASS** (evidence SHA-256 `07489AE13299106DD69DA0004975316A84E1F4ADDC75D27A34D2378F4712AC02`); Gate 3 accepted normal local ten-path commit `7fda7b8225f05c487f6d5d7154443e8369e74571` / tree `ae7760c58186ed227ce381d68562fac0a117ddd5` (evidence SHA-256 `A1BCC379A55F99B912266422BD58BF64D878D446AAE2D8345FAEFD9D379E24CB`); Gate 4 ordinary non-force publication and fresh fetch **PASS** (evidence SHA-256 `39A8EF0098CD313A2645316AFF0D5DD50841D080D68F98A0CE90E04D49AAFC1C`), proving clean `HEAD == origin/main == FETCH_HEAD` and ahead/behind `0/0`. RDC-04 is **CLOSED/PUBLISHED**. Canonical plan/acceptance: `docs/TOOLING_OUTPUT_DIRECTORY_SAFETY_PLAN.md` + `_ACCEPTANCE_PLAN.md`. **Completed maintenance boundary:** `RDC-05 / REPOSITORY_CI_BASELINE` — **GATES 0–4 ACCEPTED / CI IMPLEMENTATION PUBLISHED** at `7f75cffdd0c632c5f99310f2c8708d745769aa42` (tree `69cdb0a83b6a5454582343c6d20239761087809d`). The Gate-0 CI plan/acceptance pair was published docs-only at `49392d003abf26636a8717a9e27c5520bf4c1af4`. Gate-1 installed exactly one `.github/workflows/ci.yml`; Gate-2 Mac-mini Ubuntu SSH acceptance proved locked Host restore, warning-clean Release build, genuine MTP Core `84/84` and Transport `24/24`, no skips and no target I/O (evidence SHA-256 `9ABE736EA688352C570B3D0443BABC797A8E240B27FE2107CFF40940305BF651`). Gate-3 workflow commit `19025ed7d695bf75b43b010ae2e1d0a1ba105d4d` was published non-force but its initial hosted run `37821098177` failed before job creation due to invalid job-level `runner.temp` context. The scoped CI-only fix was accepted and published non-force at `7f75cffdd0c632c5f99310f2c8708d745769aa42`. GitHub Actions run [`37829541187`](https://github.com/dturovskiy/deus-os-stm32/actions/runs/37829541187) on that exact SHA completed **success**: Ubuntu 24.04, .NET SDK `10.0.401`, locked restore, Release build `0 warnings / 0 errors`, executed Core `84/84` and Transport `24/24`, `0 failures / 0 skips`, tracked-file and non-vacuous diff hygiene PASS. The accepted non-force repair-publication evidence SHA-256 is `9C701DACDD13E5CFF96F22CC79B86C1DC29889BB6D21B474D0A21FFBE59D0276`. CI proof is Host-only and does not replace the established STM32/USB/OLED acceptance. **RDC-06 / KERNEL_COMPOSITION_ROOT_FINAL_CLEANUP is CLOSED/PUBLISHED:** exact three-file `help` relocation commit `7bc5f27ce3447f0c907d2643e7db67669371210a`, tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`, and separate bootloader CTR fix `83e57f609bab4bcc8a61ea2222629bf9066e910d` were published by normal fast-forward; fresh fetch was clean `HEAD == origin/main == FETCH_HEAD`, ahead/behind `0/0`. GitHub-hosted CI run [`38011195238`](https://github.com/dturovskiy/deus-os-stm32/actions/runs/38011195238) passed at exact `83e57f6`: Core `84/84`, Transport `24/24`, no failed/skipped tests and Release `0 warnings / 0 errors`. **Next mandatory active maintenance boundary:** `RDC-07 / BOOTLOADER_READABILITY_CLEANUP` Gate-0 read-only audit/design (no source change yet); RDC-08 queued. Feature work remains blocked.

## 5. Current development/acceptance topology

Canonical topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

Current bench facts:

- canonical repository/build host: Windows, `D:\Projects\STM32\OS`;
- target native USB + normal VBUS/power owner: Ubuntu on Mac mini;
- Windows owns ST-LINK/SWD and CH340/UART;
- accepted SWD speed: 950 kHz; read-only fallback ladder `950/480/240/125`;
- ST-LINK 3.3 V and UART-adapter VCC are disconnected;
- physical target power cycle is native-USB VBUS removal/restore on the Mac-mini connection;
- OLED: SSD1306-class 128x32 at `0x3C`.

A topology change must be reflected in the topology contract before topology-dependent hardware acceptance.

## 6. Completed/published foundation sequence

The following major foundations are complete/published and are **not active work**:

- scheduler timed blocking;
- scheduler fixed priority;
- production heartbeat task;
- IWDG production liveness;
- normal-boot production task ownership;
- native USB device core;
- USB CDC ACM diagnostics/console;
- transport-neutral shell/RPC;
- binary framed transport v1;
- OS application/UI model foundation;
- boot/desktop UI foundation;
- OLED dirty-region optimization;
- application runtime foundation;
- kernel composition-root decomposition;
- USB management device foundation;
- Host control application foundation;
- Asset / Configuration transfer foundation;
- pre-Bootloader resource architecture recovery;
- firmware update / bootloader foundation.

Their scoped plan/acceptance/protocol records remain engineering history/contracts. They do not override this file for current project disposition.

## 7. Deferred / trigger-driven work

Canonical deferred policy: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.

Not-active examples include:

- I2C IRQ/DMA until measured bus pressure;
- scheduler ready-set acceleration until measured scale/CPU pressure;
- CRC acceleration until measured streaming cost;
- tickless/low-power until a concrete power requirement;
- generic filesystem/storage layers without a concrete consumer;
- stable physical unit identity until a real multi-unit/privacy requirement;
- richer structured observability/runtime statistics;
- UI layout/preset/configurator expansion;
- networking/remote-management security until that boundary is explicitly promoted.

The ten `FDC-01..FDC-10` audit obligations are **closed/published**. They no longer own current work ordering. The mandatory residual-debt ledger is now `RDC-01..RDC-08`; each source/tooling item requires its own frozen maintenance plan/acceptance pair, and no new product feature boundary may be promoted until the entire RDC program closes.

The pre-existing firmware-update robustness items are consumed by that program rather than left indefinitely trigger-driven: authenticated `image_length` reset-handler containment, bounded bootloader HSE/PLL startup, explicit non-DATA timeout/restart/adjudication semantics, and bounded stale request-ID lifetime under repeated cancellation/wrap. `FDC-08` additionally covers the newly confirmed normal-runtime HSE/PLL/switch waits, UART TX wait and poll-count-based bootloader-entry reset fallback.

## 8. Repository/security hygiene state

The post-publication read-only audit found:

- no tracked build outputs, firmware binaries, maps, evidence archives or logs;
- no tracked key/certificate/private-key artifact types;
- no such suspicious artifact filenames in Git history;
- no product-source TODO/FIXME/HACK markers requiring closure;
- no raw acceptance key in repository or shareable evidence;
- the immutable bootloader recovery bundle is intentionally **private/sensitive** because its recovery region contains the compiled key-bearing bootloader, even though no standalone raw key file is stored.

The repository-owned `.gitignore` covers build outputs, archives/logs, .NET outputs, common local secret files and editor/temp noise.

## 9. Known architecture state

Resolved/current:

- semantic application events are separate from scheduler wake bits;
- system identity/capability discovery is provided through `sysinfo`;
- USB management and host transport are separated from CDC diagnostics;
- Windows/Linux host control share one protocol/Core model;
- Host Core update/RPC/Asset responsibilities share one correlation/channel owner rather than duplicate decoders/locks;
- firmware-update authenticity, target binding, rollback floor, interruption recovery and publication capability are accepted.

Still intentionally deferred **because they are future capabilities or consumer-triggered enhancements, not confirmed current debt**:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- network mutation security;
- richer observability/runtime-statistics framework.

The completed `FDC-01..FDC-10` program remains historical closure provenance; its sequencing and closure criteria are recorded in `docs/ROADMAP.md`, `docs/MASTER_EXECUTION_CHECKLIST.md` and the relevant scoped plans.

## 10. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- development/acceptance topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- forward ordering: `docs/ROADMAP.md`
- scoped boundary design: matching `*_PLAN.md`
- scoped boundary proof: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements and promoted debt provenance: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- chronology: `CHANGELOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
