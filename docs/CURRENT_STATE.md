# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

It intentionally does not duplicate live Git `HEAD`/tree identity. Git itself is authoritative for repository bytes, commit identity, branch state and cleanliness. Documentation-only commits may advance `HEAD` without changing the accepted product boundary.

For architecture, roadmap sequence, boundary contracts, acceptance criteria, evidence policy and historical records, follow the precedence in `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED**

Boundary acceptance/publication commit:

`562e786ffa734da055c23144ec4256bc8961bbaf`

Boundary commit tree:

`88720a614794d5aef93cf13ac762a4095cb17baf`

Accepted publication-activation candidate:

- firmware source tree: `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`
- host source tree: `136687e80c42bd8104ad6c37fbbccb915b60fd08`
- BIN SHA-256: `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`
- whole-Flash SHA-256: `CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`
- Flash/SRAM: `54268/11944`
- task stacks remain `1024/512`
- system capability mask: `0x0000003F`
- final accepted persistence wear counter: `46/64`
- final Gate-5 recovery state: `CLEAN_STATE_POST_RESET_BYTE_EXACT`

Published v1 capability is deliberately bounded:

- management-IF2 binary Asset/Configuration transfer;
- object type `0x0001` / `OLED_UI_LAYOUT_CONFIG_V1`;
- exact current consumer payload `8` bytes;
- persistent A/B pages 62/63 with version/integrity/atomic-commit/recovery/wear contracts;
- production `PublishedOnly` host path accepted on Linux/libusb;
- response-loss idempotency, corruption fallback and physical VBUS retention accepted;
- accepted 128x9 status bar remains system-owned; only the console clip is persisted.

Canonical design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

Canonical acceptance:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

The preceding `HOST_CONTROL_APPLICATION_FOUNDATION` remains published at
`e0f49f168542fa1cf49bca451e01b0c077aa8d18`; it is no longer the latest completed
product boundary.

## 2. Current boundary disposition

`PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — **GATES 0–6 ACCEPTED BY THE ONE NORMAL LOCAL ACCEPTANCE COMMIT CONTAINING THIS DOCUMENT / GATE 7 ORDINARY NON-FORCE PUBLICATION NEXT**

Canonical design:

`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md`

Canonical acceptance:

`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md`

This is a behavior-preserving prerequisite before `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`.
It exists because the published Asset image has only `4` bytes remaining under the
frozen Flash acceptance ceiling and `344` bytes remaining under the static-SRAM ceiling.

Gate-0 read-only audit froze these trigger facts:

- `src/kernel.c = 3459` LOC and remains a measured multi-responsibility composition-root hotspot;
- `src/drivers/usb_device.c = 2365` LOC;
- `src/kernel/scheduler.c = 1859` LOC;
- `src/kernel/scheduler_diagnostics.c = 1361` LOC;
- host `DeusDeviceClient.cs = 1004` LOC, but host decomposition is tracked separately;
- the target build already uses function/data sections plus linker GC;
- most firmware still builds at `-O2`; only the four Asset size-sensitive sources use `-Os`;
- production command/RPC reachability retains substantial scheduler/OLED diagnostic/self-test code;
- scheduler core reserves a 1-KiB default task-stack store while normal production binds separate `1024/512` task stacks;
- target heap API use remains absent and driver/gfx dependency direction remains clean.

Gate 1 measurement is accepted. The exact published baseline reproduced at
Flash/SRAM `54268/11944` with BIN SHA-256
`7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`.
Isolated linked Flash deltas were `kernel.c -1648`, scheduler core `-1360`,
USB device `-1600`, and gfx/OLED presentation `-1508`; all left SRAM unchanged.
Scheduler diagnostics `-Os` is not selected because GCC generated an unresolved
`memset` under the current freestanding `-nostdlib` contract.
`RING_RESIZE_AUTHORIZED=NO` remains frozen.

Gate 2 is now authorized only for the smallest measured recovery set:
`src/kernel.c -> -Os` plus removal of the scheduler-owned 1024-byte default
task-stack store. Scheduler diagnostics/self-tests must preserve behavior by borrowing
already-existing production stack storage while the scheduler is inactive; production
boot continues to bind the existing `1024/512` console/heartbeat stacks. No USB-ring
cuts, command/RPC pruning, diagnostic-profile split, libc shim, linker relocation or
Bootloader behavior is authorized.

Gate 2 is accepted from
`stm32_os_preboot_resource_gate2_recovery_recomposed_v1_20260926_210150.evidence.zip`
with SHA-256 `D84AC967C204FC9D09B94EC8AD7B472B173BCDCE4593320F88601B062681E325`;
the exact normalized six-file source diff is
`D1EC010F55313DA61DC1366E99E68A619CF4E678EDD2EE443E7D53AB08AD6A06`.
The real index remained empty and there was no target or Flash mutation.

Gate 3 is accepted from
`stm32_os_preboot_resource_gate3_build_static_v1_20260926_212215.evidence.zip`
with SHA-256 `9FA982530FAB4B703AF6D92936080BD13A0E2FF346D294F6376F5CAFE814EFE2`.
The locked firmware candidate tree is `a10182e7d0659b9b161073ad49a8816ecb6e7918`;
BIN `52908` / `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`,
ELF `5BF8D5CE66049CCBD7EF2D77D6D569B980CB96B33E0FFDBFDB1A626A6A101247`, MAP
`AE45CF669031AC4CB29DB2FFB72161ABA2249D9A6558B80E6B40757EC162C29E`, Flash/SRAM
`52908/10920`, stack-usage `26/26`, undefined `0`, forbidden heap/libc drift `0`, public
ABI diff `0`. Gate 4 is accepted as a bound composite hardware/runtime result. The
exact Gate-3 production candidate retained task0/task1 margins `304/432`, MSP margin
`1592`, management IF2 pressure `128/128`, application lifecycle, scheduler/IWDG,
canonical CLEAN persistence, zero USB errors/PMA overruns/drops, physical OLED and
physical reconnect recovery. The final read-only closure
`stm32_os_preboot_resource_gate4_readonly_closure_v1_20260927_190303.evidence.zip`
(SHA-256 `05A8F13552656D3C2AE5B05F8BC0FC927140BF0CBAC964B12721E46BABB2357B`)
proved post-IWDG UART `IWDG_RESET=1`, exact source tree/Home/health/ping over primary
management IF2/libusb, and exact final 64-KiB Flash SHA-256
`17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`.

After this boundary is accepted and published, exact next product boundary is
`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` Gate 0.

### Published Asset acceptance record

The original `DEFERRED_NO_REAL_CONSUMER` Gate 0 result remains historical proof that implementation was correctly blocked when no real consumer existed.

That blocking condition was resolved by the promoted narrow consumer:

- consumer: `OLED_UI_LAYOUT_CONFIG_V1`;
- canonical consumer contract: `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`;
- target object type: `0x0001`;
- exact consumer payload: `8 bytes`;
- scope: persisted OLED **console clip only**;
- accepted status bar remains frozen/system-owned at `128x9`;
- compiled default remains console `x=1, y=10, w=126, h=22`.

Gates 0–7 are accepted and the boundary is published at
`562e786ffa734da055c23144ec4256bc8961bbaf`.

Frozen Gate-0 contracts:

- first-class bounded binary transfer ABI — `docs/ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md`;
- persistent A/B record envelope + atomic commit/recovery — `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md`;
- resource budget — `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md`: Flash `<=54272`, SRAM `<=12288`, task margins `>=256/256`, MSP margin `>=1024`;
- Flash wear/timing/watchdog/USB continuity — `docs/ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md`;
- deterministic reset/power-loss fault-injection matrix — `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md`;
- recovery bundle / ST-LINK restoration — `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md`.

All six Gate-0 design contracts are frozen and the 2026-09-22 cross-contract closure audit is **PASS**.

Closure found and corrected two issues before authorization:

1. HELLO Asset bit 6 is carrier-specific: CDC remains `0x3F`, management IF2 candidate becomes `0x7F`;
2. ST-LINK recovery uses explicit page erases followed by CubeProgrammer `--skiperase`; PRESERVE never erases pages 62/63.

Current accepted Gate-5 candidate source tree is `f945045221adb52e1aa7a13f4e89b3dbd1ddb5de`.

Gate-5 status:

- deterministic Campaign A — **ACCEPTED**;
- deterministic Campaign B with previous valid record — **ACCEPTED**, including FI-1..FI-9 and the FI-9 response-loss/idempotency proof;
- deliberate persistence-page erase attempts accumulated through Campaign B: `20/64`;
- accepted Campaign-B final generation: `2`, payload `01100A6016000000`, payload CRC `838D1F51`;
- accepted Campaign-B whole-Flash SHA-256: `CCE75D6B5C75934F5E4FB5788BB3BB63639B1B5D7D0FDF552085830C1300341D`;
- accepted Campaign-B persistence-region SHA-256: `F5375013643DD5E8D3C31D44836BE1B2FB06844A9C99D751CCAB594E61922233`;
- corruption matrix — **ACCEPTED 7/7** by `stm32_os_asset_gate5_corruption_matrix_contract_oracle_v1_20260925_203206.evidence.zip`, SHA-256 `51BA4472AE94C1ADDFE552AEF245E8CF7DC39FDE6C855C2377455CBD74838255`;
- the accepted matrix independently proved all seven deterministic corrupt-state fallbacks, exact compiled-default runtime and frozen status bar, Linux/libusb management health, zero boot repair mutation by exact pre-reset/post-boot persistence byte equality, and exact firmware bytes outside pages 62/63. Case 7 additionally proved conflicting equal-generation valid A/B records resolve to no record/default as required;
- accepted matrix preflight first proved the previously uncertain post-v2 live target state was already canonical CLEAN, then consumed `16` deliberate persistence-page erases (14 for seven cases + 2 final CLEAN), bringing the Gate-5 cumulative counter to **40/64**;
- final matrix recovery is **CLEAN_STATE_POST_RESET_BYTE_EXACT**: whole-Flash SHA-256 before and after reset is `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957`, persistence pages are erased, Asset status is generation `0`/no-record, runtime is exact compiled default, and scheduler/IWDG/USB/fault health passes;
- historical failed corruption-matrix attempts remain diagnostic history only: v3 restore-state/obsolete Windows-USB composition, cross-host v1 PowerShell argument-cardinality, and cross-host v2 semantic-oracle revision misuse are superseded by the accepted contract-oracle evidence above;
- historical Gate-5 final substep was the physical target USB/VBUS power-removal/power-return retention proof; it is now accepted by the v3 evidence below;
- first retention package `stm32_os_asset_gate5_physical_retention_v1_20260925.zip` was blocked by Microsoft Defender AMSI while `apply.ps1` was being parsed, before any harness code or target I/O executed. Defender Operational events `1116/1117` identify `HackTool:PowerShell/ApexToolkit.A`, Threat ID `2147749462`, detection source `AMSI`, process `pwsh.exe`, action `Quarantine`, action success `True`, security-intelligence version `1.459.398.0`, engine `1.1.26080.3`. This is `HARNESS-AMSI-DELIVERY-01`: zero target mutation, zero additional persistence erases. The blocked package is frozen; no Defender exclusion, AMSI bypass, obfuscation or signature-avoidance rewrite is authorized;
- replacement `.NET` retention run `stm32_os_asset_gate5_physical_retention_dotnet_v1_20260925_222446.evidence.zip`, SHA-256 `A496C86EDDE741358C32B12968172E8FF978AF122BC3A3329EEE18C8415915D0`, proved the executable/AMSI architecture works and reached the physical-power phase. It committed generation `1`, then timed out waiting for USB disappearance: 81 fresh remote helper processes each created a new Linux libusb discovery context and all reported exactly one Deus device at stable locator `usb:001:8`; therefore no actual Linux USB data-path disappearance was observed. Classification `ENVIRONMENT / USB_PRESENCE_TIMEOUT`, not PRODUCT. Failure cleanup restored `CLEAN_STATE_POST_RESET_BYTE_EXACT`;
- that failed retention run consumed `+3` persistence-page erases (`+1` retention commit, `+2` cleanup), so cumulative Gate-5 wear reached **`43/64`** at that checkpoint. The subsequent composition sequence introduced the required zero-write physical-path rehearsal with operator-synchronized USB removal, fresh Linux libusb absence, SWD target-unavailable corroboration, USB/SWD return and exact CLEAN re-attestation before the retained commit; the accepted v3 run below satisfied that requirement;
- `stm32_os_asset_gate5_physical_retention_dotnet_v2_20260925.zip` failed safely during `DOTNET_BUILD_RELEASE` before the .NET engine started. Root cause is `HARNESS-DOTNET-COMPILE-01`: six newly added failed-v1 evidence regexes used `\s` inside ordinary C# string literals, producing Roslyn invalid-escape compile errors. Bootstrap ordering proved zero SSH/ST-LINK/USB/UART/target I/O and zero persistence erases; cumulative wear therefore remained **`43/64`** at that checkpoint. Full-source audit found exactly those six invalid normal-string escapes and no others;
- physical retention — **ACCEPTED** by `stm32_os_asset_gate5_physical_retention_dotnet_v3_20260925_235521.evidence.zip`, SHA-256 `239735C43309870DC820ED0B3FF2C6D0E331BF805FE9B1BCA2B68ED685A249A7`. The zero-write rehearsal proved real Mac-mini Linux USB disappearance plus independent SWD target loss, then exact CLEAN return. The retained test committed generation `1`, payload `01100A6016000000`, CRC `838D1F51`; after one real VBUS removal/return the same generation/payload/runtime returned and the entire 64-KiB Flash plus 2-KiB persistence region were byte-identical pre/post power cycle. Final persistence-only cleanup restored canonical CLEAN whole-Flash SHA `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957` before and after reset;
- accepted retention evidence integrity: ZIP CRC clean, `412/412` evidence hashes exact, all 24 source locks exact, repository pre/post status identical, real index empty, no commit/push/mass erase/option-byte mutation. Of 121 bounded native processes, the only two nonzero/timeouts are the intentional SWD power-off probes during rehearsal and retained power removal;
- the accepted retention run consumed `+3` persistence-page erases (`+1` retained commit + `+2` final CLEAN), bringing the final Gate-5 wear counter to **`46/64`**. This is within the frozen `<=64` campaign budget;
- **GATE 5 = ACCEPTED**: Campaign A/B, FI-9 response-loss idempotency, 7/7 corruption matrix, one physical retention power cycle, wear budget and known-good recovery are all satisfied. Final Gate-5 target state was `CLEAN_STATE_POST_RESET_BYTE_EXACT`. Gate 6 then performed the single capability-publication activation described below;
- Gate-6 harness v2 reached clean locked restore/build but failed before target I/O because its `git status --porcelain=v1` parser trimmed the fixed status prefix before slicing the path, turning tracked names such as `README.md` into `EADME.md`; its operator/evidence presentation also violated the canonical terminal-structure contract. This is diagnostic HARNESS history only and consumed zero persistence erases;
- Gate-6 harness v3 corrected the porcelain/presentation lifecycle and produced trustworthy classified evidence, but falsely reported `BUILD_TOKEN: BUILD_FLASH` after a successful firmware build because its multiline regex was CRLF-sensitive. Evidence `stm32_os_asset_gate6_publication_activation_smoke_v3_20260926_120207.evidence.zip`, SHA-256 `4C58503D25033BFCEAAE973746489908E0011B21A52EED00F784CDAD21DC2862`, proves `BUILD_OUTCOME=PASS`, Flash `54268`, SRAM `11944`, no target I/O and unchanged wear `46/64`; failure class `HARNESS-LINE-ENDINGS-PARSER-01`;
- Gate-6 v4 replayed the corrected normalized/cardinality parser against the exact v3 raw build stdout before execution, then passed the complete activation smoke. This v4 result supersedes the failed Gate-6 harness attempts.

The current bench is split-host: target native USB/power is owned by Ubuntu on the Mac mini and exercised through Linux libusb via `deus@macmini`; ST-LINK/SWD and CH340/UART are owned by Windows. The canonical operational contract is `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

The Asset Flash erase/program implementation and 54-KiB linker boundary are active in the published Asset boundary. Application reset origin remains `0x08000000`; bootloader relocation/VTOR migration is still deferred. Gate 5 accepted the implementation with system capability bit 5 intentionally off. Gate 6 added the single publication-activation source change `SYSTEM_IDENTITY_CAP_ASSET_CONFIGURATION_TRANSFER`, changing the published system capability mask from `0x0000001F` to `0x0000003F`; that activation is now hardware-accepted by `stm32_os_asset_gate6_publication_activation_smoke_v4_20260926_134106.evidence.zip`, SHA-256 `7A65CA036E29FE06E39C5ADA2B70461292DBFD3DC584D28C3563A7E498322C60`. Accepted activation firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, host tree `136687e80c42bd8104ad6c37fbbccb915b60fd08`, BIN SHA-256 `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`, whole-Flash SHA-256 `CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`. Fresh build/tests pass at Flash `54268/54272` (4-byte headroom) and SRAM `11944/12288`; production `PublishedOnly` `config status/read`, ping8 and health pass on Mac-mini Linux libusb. Persistence remains erased and Gate-5 wear remains `46/64`. Generic storage/filesystem/package infrastructure remains unauthorized.

Canonical Asset design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

Canonical Asset acceptance:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

The broader configurable OLED layout/preset/assets roadmap remains deferred. Only `OLED_UI_LAYOUT_CONFIG_V1` has been promoted as the concrete persistence consumer.

Historical repository-readiness prerequisites that supported the reopened Asset Gate 0:

1. repository-owned firmware build entrypoint — **ACCEPTED**: the exact historical Host Control Gate-2 compiler/startup/link/objcopy invocation was recovered from acceptance evidence rather than guessed, the generated `deus_build_identity.h` was recovered identically from 20/20 surviving copies (SHA-256 `F6EAA98172FEE67338BFD978F208BD95731063A1160B0C9C1282306A5D6658A5`), and two independent temporary builds with Arm GNU Toolchain `15.3.1` reproduced the accepted 50652-byte firmware BIN byte-for-byte at SHA-256 `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F` with exact `text/data/bss = 50540/112/11616` and Flash/SRAM `50652/11728`; the versioned entrypoint is `scripts/build_firmware.ps1` (accepted script SHA-256 `BC7B91825B9BB80394C28643848EE3AEF75A4B62017432BB1070190088B8CDFB`), final reproducibility evidence ZIP SHA-256 `1FDB3C647ACCF4B9A67FDC02514009495F60D1F3E3B4E00C551FC0B4A3F4F847`; ELF/MAP debug artifacts remain path-dependent and are not the firmware identity; no target I/O or Flash mutation occurred;
2. read-only real-device MCU/Flash preflight — **ACCEPTED** on the current physical board: `DEV_ID=0x410`, numeric `REV_ID=0x2003`, factory Flash size `64 KiB`, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF`, RDP disabled, WRP0..31 inactive, SWD accepted at 950 kHz; immutable source-log SHA-256 `B112A754E82ECC01BDC509B2D8FB359D652D74DDC565B3DE046C30C181672C13`, finalized evidence ZIP SHA-256 `BB1F928A2E716F2C8D0FA6B160FC7F41B3187A75CF869DD139DF0C450D6B42D8`;
3. host package-lock/SDK restore reproducibility — **ACCEPTED**: seven project-local `packages.lock.json` files are now the reviewed dependency graph; `host/global.json` remains the accepted `.NET 10` policy (`10.0.100`, `rollForward=latestFeature`, prerelease disabled), selecting Windows SDK `10.0.201` and Linux SDK `10.0.112`; locked restore, Release build and direct Core/Transport tests pass on both Windows (`21/21`, `5/5`) and Ubuntu/Linux (`21/21`, `5/5`), WSL proves the DEUS path and `D:` path are the same checkout, and all seven normalized lock hashes are identical across Windows/Linux. Final acceptance evidence ZIP SHA-256 `CE7E3BB09A24913D5374A875CCE49556A2A186F114E36128E42AD3BB26F42165`; no target I/O, Flash mutation, reset or reconnect occurred.

These were infrastructure prerequisites for the now-published Asset boundary; they are retained here as acceptance provenance.

## 3. Planned product sequencing

The shared docs-only Flash ownership decision is now frozen by `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`:

- steady-state bootloader/recovery: pages 0..7, `0x08000000..0x08001FFF`, 8 KiB ceiling;
- steady-state relocated application: pages 8..61, origin `0x08002000`, 54 KiB maximum;
- persistent slot A/B: pages 62/63 at `0x0800F800` / `0x0800FC00`, 1 KiB each;
- Asset/Configuration is implemented first in a transitional standalone phase: application remains reset owner at `0x08000000`, its linker ceiling becomes 54 KiB, pages 54..61 remain unused relocation headroom, and only pages 62/63 become persistent;
- application relocation to `0x08002000`, VTOR/handoff changes and lower-page bootloader ownership occur only in the later Bootloader boundary.

The historical Asset reactivation condition was satisfied by `OLED_UI_LAYOUT_CONFIG_V1`; the full Asset/Configuration boundary is now accepted and published at `562e786ffa734da055c23144ec4256bc8961bbaf`.

Forward dependency order is:

1. `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` — **ACCEPTED DOCS-ONLY SHARED CONTRACT**;
2. `OLED_UI_LAYOUT_CONFIG_V1` — **PROMOTED / FROZEN CONSUMER CONTRACT**;
3. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `562e786ffa734da055c23144ec4256bc8961bbaf`**;
4. `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — **GATE 0 ACCEPTED / GATE 1 MEASUREMENT NEXT**;
5. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — only after the recovery boundary is accepted/published;
6. networking/service/security extensions.

The pre-Bootloader recovery boundary is deliberately behavior-preserving. It does not
implicitly authorize firmware-update logic, linker relocation or public-ABI removal.

Firmware update requires separate authenticity/security, controlled reboot/update handoff and recoverable Flash transaction design. Networking remains a later service boundary and does not authorize speculative TCP/IP expansion on the STM32F103.

Canonical sequence:

`docs/ROADMAP.md`

## 4. Completed/published foundation sequence

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
- Asset / Configuration transfer foundation.

Their `*_PLAN.md` and `*_ACCEPTANCE_PLAN.md` files are retained as immutable scoped design/proof records, not as competing global current-state documents.

## 5. Deferred / trigger-driven work

The following items exist as possible future work but are **not active roadmap boundaries unless explicitly promoted**:

### Performance and kernel/runtime

- I2C async/IRQ/DMA after measured bus pressure;
- scheduler ready-set acceleration after measured task-count/CPU pressure;
- CRC acceleration after measured streaming cost;
- bounded minimal-copy/backpressure refinements for transfer/update streaming;
- tickless/low-power after a real power requirement;
- generic timers/queues/synchronization/runtime statistics only for real consumers;
- further composition-root convergence when ownership pressure warrants it.

### Robustness and observability

- previous-boot crash/reset retention;
- bounded structured binary event tracing;
- soak/fault-injection campaigns;
- explicit application stop-failure semantics before resource-owning apps require them.

### UI/configuration ideas

- alternate OLED layout presets;
- runtime custom layout editing;
- PC layout configurator/import;
- persisted UI layout only through an accepted persistence contract;
- RTC wall-clock;
- notification rendering/queue semantics;
- optional kernel-log presentation.

`docs/OLED_UI_LAYOUT_PLAN.md` and `docs/OLED_STATUS_BAR_PLAN.md` are deferred future-consumer designs, not active implementation plans.

### Host management presentation direction

- `DeusOs.Control.Cli` is the canonical first-class management surface for automation, diagnostics, acceptance and headless Linux operation;
- the existing Avalonia Desktop remains an optional workstation frontend rather than a Linux-server dependency;
- a future Web UI should sit above the same `DeusOs.Control.Core` / bounded host-management service API rather than implement the STM32 binary protocol independently in the browser;
- a local Web/service surface does not imply STM32 networking and may be promoted separately only when a concrete host-management consumer requires it;
- LAN/Wi-Fi/remote exposure remains a later networking/security boundary and requires explicit authentication/authorization/trust design before activation.

Forward presentation reference: `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`.

This direction is **not an active implementation boundary**.

### Storage/platform expansion

- general filesystem;
- external storage/block-device layers without a concrete consumer;
- MPU/user-kernel isolation;
- broad power-management framework;
- stable physical unit identity/MCU UID until a real multi-unit/privacy requirement exists.

Canonical deferred policy:

`docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

## 6. Known architecture state

The initial Asset/Configuration Gate 0 review found no substrate blocker and was historically deferred because no concrete consumer with a frozen persistence schema existed. That blocker was subsequently resolved by promoting `OLED_UI_LAYOUT_CONFIG_V1`; implementation and hardware acceptance have since advanced to Gate 5.

Resolved gaps:

- semantic application events are separate from scheduler wake bits;
- system identity/capability discovery is provided through `sysinfo`;
- USB management and host transport are separated from CDC diagnostics;
- Windows/Linux host control share one protocol/Core model;
- kernel composition-root risk was materially reduced without introducing a god context/service locator.

Still intentionally unresolved until a consumer requires them:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- firmware-update authenticity;
- network mutation security;
- richer observability/runtime-statistics framework.

These remain consumer- or boundary-specific future concerns. None is implicitly activated by the current bounded `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` boundary.

## 7. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- development/acceptance host and physical-interface topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- forward ordering: `docs/ROADMAP.md`
- active/completed boundary design contract: matching `*_PLAN.md`
- boundary proof/acceptance contract: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- historical change chronology: `CHANGELOG.md`
- execution history/checklists: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator/handoff reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella implementation notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
