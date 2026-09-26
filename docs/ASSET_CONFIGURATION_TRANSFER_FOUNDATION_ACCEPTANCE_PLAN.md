# Deus OS — Asset / Configuration Transfer Foundation Acceptance Plan

Status: **GATES 0–6 ACCEPTED BY THE LOCAL ACCEPTANCE COMMIT CANDIDATE — GATE 7 PUBLICATION AUTHORIZED NEXT**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Canonical design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

## 0. Gate 0 reactivation state — historical record, 2026-09-21

The original Gate 0 deferral remains an accepted historical fail-closed result. The boundary is now reopened because the missing concrete-consumer prerequisite has been satisfied by `OLED_UI_LAYOUT_CONFIG_V1`.

Reactivation repository baseline:

- `HEAD == origin/main == 8a1fe87572a46b50b503ed5008c080705c517036`;
- working tree clean;
- ahead/behind `0/0`;
- `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` already published at that baseline.

Reactivation matrix at that time:

| Reactivation prerequisite | Current result |
| --- | --- |
| Promoted concrete consumer | **PASS** — `OLED_UI_LAYOUT_CONFIG_V1` |
| Frozen target schema + validation | **PASS** — exact 8-byte payload and strict validation |
| Exact maximum serialized consumer payload | **PASS — 8 bytes** |
| Shared Flash partition incl. bootloader/recovery budget | **PASS** — `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` |
| Repository-owned reproducible firmware build | **PASS** — `scripts/build_firmware.ps1` |
| Read-only real-device Flash/revision/protection preflight | **PASS** |
| Host package-lock / SDK reproducibility | **PASS** |
| First-class bounded binary transfer ABI | **PASS** — `ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md` |
| Persistent A/B record envelope + atomic commit/recovery | **PASS** — `ASSET_CONFIGURATION_PERSISTENCE_V1.md` |
| New implementation resource ceilings | **PASS** — `ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md` |
| Wear/timing/watchdog/USB continuity model | **PASS** — `ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md` |
| Deterministic reset/power-loss fault-injection matrix | **PASS** — `ASSET_CONFIGURATION_FAULT_INJECTION_V1.md` |
| Recovery-bundle / ST-LINK restoration procedure | **PASS** — `ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md` |
| Capability bit 5 advertised | **NO — correctly blocked** |
| Gate 1 implementation authorized | **YES — bounded source boundary only; implementation not started** |

Gate 0 is **complete** after the cross-contract closure audit recorded below. This completion authorizes only the bounded Gate-1 source boundary; it is not implementation acceptance.

No source implementation, linker migration, target Flash mutation, capability activation or hardware run is accepted by this reactivation record.

## 0A. Cross-contract closure audit — 2026-09-22

Closure baseline before this documentation change:

- `HEAD == origin/main == FETCH_HEAD == 57ce4d96fc28f948b0c12809bdc54fc2a0cf0608`;
- ahead/behind `0/0`;
- working tree clean before the closure corrections;
- all six Gate-0 design-contract commits already published.

The closure audit reviewed the Flash ownership decision, concrete consumer and all six Gate-0 contracts as one system rather than accepting six isolated documents.

| Cross-contract concern | Closure result |
| --- | --- |
| Flash arithmetic | **PASS** — `54 KiB application + 8 KiB relocation headroom + 2 KiB persistence = 64 KiB` |
| Gate-2 application ceiling | **PASS** — `54272 B` is one 1-KiB page below the 54-KiB linker region |
| Transfer/storage capacity | **PASS** — `64-byte envelope + 960-byte payload = 1024-byte slot` |
| Current consumer | **PASS** — object `0x0001`, schema `1`, exact `8 B`, compiled default frozen |
| Frame/USB bounds | **PASS** — parser max `132 B`, chunk `32 B`, max Asset response wire `62 B <= 64 B` EP4 packet |
| Carrier capability semantics | **PASS AFTER CORRECTION** — CDC HELLO remains `0x3F`; management IF2 alone may advertise Asset bit 6 / `0x7F` |
| System capability semantics | **PASS** — system identity bit 5 remains OFF through Gate 1–5 |
| Persistent authority | **PASS** — final `0xA55A` halfword is the only authority-changing write after readback validation |
| Fault ordering | **PASS** — FI-1..FI-6 resolve previous/default; FI-7..FI-9 may resolve new only through the normal full validator |
| Wear/fault budget | **PASS** — 10,000 successful changed-generation ceiling; fault campaign counts page-62/63 erases explicitly |
| Recovery ownership | **PASS AFTER CORRECTION** — explicit erase set + `--skiperase`; PRESERVE never erases pages 62/63; CLEAN explicitly erases 0..63 without mass erase |
| Reset ownership | **PASS** — Asset keeps standalone reset owner at `0x08000000`; no VTOR/application-origin relocation |
| SRAM architecture | **PASS** — no heap/new task/whole-object target buffer; `12288 B` static ceiling |
| Gate-1 implementation scope | **PASS** — bounded target/Core/CLI/recovery-script surface frozen in the design plan; startup/scheduler/USB-descriptor/transport/Desktop/Web/bootloader/network expansion guarded |

Gate-1 primitive review also closed the STM32F1 halfword/chunk-boundary detail before the transfer handler was implemented. WRITE_CHUNK remains `1..32` bytes; target state may hold exactly one pending odd payload byte plus an exact <=32-byte last-chunk retry cache. The pending final byte is padded with erased `0xFF` only when it is the declared end of the object. This remains bounded streaming, not whole-object buffering, and does not change any frozen SRAM ceiling.

Post-closure Gate-1 implementation review found one response-schema omission before any transfer handler was implemented: STATUS promised committed payload CRC/length but the frozen common response prefix had no fields for them and described response data as READ-only. The contract is corrected without changing any frame-size bound: STATUS now carries an exact 8-byte data extension `committed_length:u16, reserved:u16, committed_crc32:u32`. Maximum response remains the READ_CHUNK case at 50-byte payload / 62-byte wire.

Re-audit after this correction:

- common prefix remains 18 bytes;
- STATUS payload becomes `18 + 8 = 26 bytes`;
- READ maximum remains `18 + 32 = 50 bytes`;
- maximum wire remains `10 + 50 + 2 = 62 bytes`;
- no existing frame type, request shape or consumer/persistence contract changes.

Two substantive closure defects were found and corrected before Gate-0 completion:

1. shared HELLO generation would otherwise have advertised Asset support on CDC; the protocol now requires carrier-specific capability flags;
2. canonical ST-LINK recovery must not rely on CubeProgrammer download erase heuristics; recovery now owns explicit page erase and programs with `--skiperase`.

No source, linker, firmware, host executable or target mutation occurred during Gate 0.

Gate-0 closure result:

`GATE_0_OUTCOME=COMPLETE`

`GATE_1_AUTHORIZED=YES`

Meaning of `YES`:

- Gate 1 may now implement only the bounded source surface frozen in Section 17 of the design plan;
- Gate 1 must still pass its own source review before Gate 2 build acceptance;
- capability bit 5 remains OFF;
- no hardware Flash mutation is authorized until later gates;
- no product capability is accepted or published yet.

## 0B. Live acceptance state — 2026-09-25

The Gate-0 reactivation/closure material above remains historical contract evidence. Current execution has advanced beyond it:

- Gates 0–4 are accepted in the active WIP evidence chain;
- Gate 5 fault/recovery acceptance is **ACCEPTED** on firmware source candidate `f945045221adb52e1aa7a13f4e89b3dbd1ddb5de`; the Gate-6 publication activation smoke is also accepted, and only final docs/commit-candidate validation plus the local acceptance commit remain;
- Campaign A is accepted;
- Campaign B is accepted for FI-1..FI-9, including FI-9 response-loss recovery/idempotency;
- cumulative deliberate persistence-page erase attempts after Campaign B were `20/64`;
- accepted Campaign-B final generation is `2`, payload `01100A6016000000`, payload CRC `838D1F51`;
- accepted whole-Flash SHA-256 after Campaign B is `CCE75D6B5C75934F5E4FB5788BB3BB63639B1B5D7D0FDF552085830C1300341D`;
- accepted persistence-region SHA-256 is `F5375013643DD5E8D3C31D44836BE1B2FB06844A9C99D751CCAB594E61922233`;
- the deterministic corruption matrix is now **ACCEPTED 7/7** by `stm32_os_asset_gate5_corruption_matrix_contract_oracle_v1_20260925_203206.evidence.zip`, SHA-256 `51BA4472AE94C1ADDFE552AEF245E8CF7DC39FDE6C855C2377455CBD74838255`;
- that accepted run first proved exact canonical CLEAN live prestate, then proved each corrupt-state fallback through Windows SWD readback + Mac-mini Linux libusb runtime; every case preserved its exact pre-reset persistence bytes across boot, proving zero boot repair writes, while firmware outside pages 62/63 remained exact;
- Case 7 proved the frozen equal-generation conflict rule: two individually valid but conflicting generation-equal records produce no selected record and compiled-default runtime;
- the accepted matrix consumed `16` persistence-page erases, so cumulative Gate-5 wear is now **`40/64`**; this remains inside the frozen `<=64` campaign limit;
- final recovery is independently exact before and after reset at canonical whole-Flash SHA `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957`, persistence erased, generation `0`, compiled-default runtime and healthy Linux/libusb/scheduler/IWDG/fault state;
- earlier failed matrix attempts remain frozen diagnostic history only and are superseded by this accepted contract-oracle run;
- historical Gate-5 final substep was the single physical target USB/VBUS power-removal/power-return retention proof; that requirement is now accepted by the v3 retention evidence below;
- the first PowerShell-monolithic retention delivery was blocked before execution by Microsoft Defender AMSI as `HackTool:PowerShell/ApexToolkit.A` (Threat ID `2147749462`, Defender events `1116/1117`, quarantine successful). This is `HARNESS-AMSI-DELIVERY-01`, not a product result; security controls remain enabled and no bypass/exclusion is authorized;
- `.NET` replacement run `stm32_os_asset_gate5_physical_retention_dotnet_v1_20260925_222446.evidence.zip` (SHA-256 `A496C86EDDE741358C32B12968172E8FF978AF122BC3A3329EEE18C8415915D0`) successfully passed package/build/prestate/commit stages but failed in `PHYSICAL_POWER_CYCLE` as `ENVIRONMENT / USB_PRESENCE_TIMEOUT`. Across the full 90-second removal window, 81 fresh remote helper processes each created a new Linux libusb context and each observed one Deus device at the same physical locator `usb:001:8`; an actual USB data-path disappearance was therefore not observed. The run is not retention evidence;
- failure cleanup restored exact canonical CLEAN after reset. The failed run consumed three persistence-page erases (`+1` commit + `+2` cleanup), raising cumulative Gate-5 wear to **`43/64`**;
- before any further persistence mutation, the retention harness must perform a zero-write physical-path rehearsal with explicit operator synchronization and independent USB-absence plus SWD-power-off corroboration, then re-attest exact CLEAN after reconnection. Only a passed rehearsal may authorize the actual retention commit/power-cycle;
- `.NET` rehearsal package v2 failed during Release compilation before engine startup and therefore before any target I/O. Audit classifies it `HARNESS-DOTNET-COMPILE-01`: six regex literals added for failed-v1 evidence binding contained ordinary-string `\s` escapes, which are invalid C# escape sequences. Zero persistence erases occurred and cumulative wear remains `43/64`; whole-source scan found exactly those six instances and no additional invalid normal-string escapes;
- physical retention is **ACCEPTED** by `stm32_os_asset_gate5_physical_retention_dotnet_v3_20260925_235521.evidence.zip`, SHA-256 `239735C43309870DC820ED0B3FF2C6D0E331BF805FE9B1BCA2B68ED685A249A7`. The zero-write rehearsal independently proved Linux USB disappearance and SWD target loss, then exact CLEAN return. The retained generation `1` / payload `01100A6016000000` / CRC `838D1F51` survived one real VBUS removal/return with exact runtime and byte-identical pre/post whole Flash and persistence;
- final cleanup restored canonical CLEAN SHA `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957` before and after reset. This run consumed `+3` persistence erases, so final Gate-5 cumulative wear is **`46/64`**, within the frozen limit;
- evidence integrity independently closes: ZIP CRC clean, `412/412` hashes exact, 24/24 source locks exact, repository pre/post status identical, real index empty, and the only two native-process timeouts/nonzero results are the intentional SWD-off probes used to corroborate physical power removal;
- all Gate-5 blocking conditions in `ASSET_CONFIGURATION_FAULT_INJECTION_V1` are satisfied. **Gate 5 is ACCEPTED.** Gate-6 v2 later failed before target I/O because of a fixed-column Git-porcelain parser defect plus noncompliant terminal/evidence presentation; zero persistence erases occurred. Gate-6 v3 corrected those defects and produced trustworthy evidence, but then hit `HARNESS-LINE-ENDINGS-PARSER-01`: its CRLF-sensitive build-token regex falsely rejected a successful build (`BUILD_FLASH=54268`, `BUILD_SRAM=11944`). V3 evidence `stm32_os_asset_gate6_publication_activation_smoke_v3_20260926_120207.evidence.zip`, SHA-256 `4C58503D25033BFCEAAE973746489908E0011B21A52EED00F784CDAD21DC2862`, proves no target I/O and unchanged wear `46/64`. Gate-6 v4 normalized line endings, enforced exact token cardinality, replayed the parser against the raw v3 stdout, and then passed the complete activation smoke; only final documentation/commit-candidate validation and the local acceptance commit remain before Gate 7.

Current physical execution topology is split-host and is canonical in `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`: target native USB/power and Linux libusb/CDC are on Ubuntu at `deus@macmini`; ST-LINK/SWD and CH340/UART are on Windows. Composed Gate-5 harnesses must coordinate these domains rather than assume all target interfaces are local to Windows.

System capability bit 5 remained OFF throughout Gates 1–5. After Gate 5 closed, Gate 6 activated it exactly once in `SYSTEM_IDENTITY_CAPABILITIES`; the resulting `0x0000003F` production identity is accepted by the Gate-6 publication-activation smoke and is pending only the final local acceptance commit.

## 1. Historical Gate 0 repository prestate

Gate 0 review started from:

- `HEAD == origin/main == dd68659959d3c04b75e90bb6e31e5a956099eb92`;
- working tree clean;
- ahead/behind `0/0`;
- latest accepted product boundary `HOST_CONTROL_APPLICATION_FOUNDATION`;
- accepted firmware candidate tree `b895955f7738aceb6fca0272d510cc433378c6ab`;
- accepted host candidate tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`;
- accepted firmware Flash/SRAM `50652/11728`;
- binary frame protocol v1;
- command service v3 / registry 36;
- system capability mask `0x0000001F`.

Gate 0 is architecture/documentation review only.

No target Flash mutation, linker partition mutation, host transfer implementation or capability activation is part of this Gate 0 result.

## 2. Mandatory consumer test

Gate 0 may authorize Gate 1 only if a real v1 consumer is identified.

Acceptance requires evidence of a concrete target owner that can answer all of:

- what exact object is transferred;
- why it is needed now;
- what exact target schema/representation consumes it;
- maximum object size;
- validation rules;
- activation semantics;
- fallback/default semantics;
- persistence lifetime/update frequency;
- why volatile-only state is insufficient.

A future idea, reserved capability bit, generic package concept or deferred design document is insufficient.

## 3. Historical consumer audit result

Observed current source state:

- no persistent configuration reader in `src/` or `include/`;
- no asset registry in `src/` or `include/`;
- no Flash persistence driver;
- no persistent record/schema owner;
- no configuration package consumer;
- no target asset/config activation path.

Observed deferred design state:

- `docs/OLED_UI_LAYOUT_PLAN.md` is explicitly deferred;
- its exact target field packing is not frozen;
- its custom-layout/persistence/static-asset material is future planning;
- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` describes target packages as later work requiring persistence policy first.

Result:

`DEFERRED_NO_REAL_CONSUMER`

This satisfies the Gate 0 fail-closed rule and blocks Gate 1.

## 4. Historical Gate 0 deferral matrix

| Concern | Result |
| --- | --- |
| Real current v1 consumer exists | **NO** |
| Exact target object/schema frozen | **NO** |
| Exact maximum payload frozen | **NO** |
| Flash partition frozen | **NO — intentionally deferred** |
| Persistent transaction format frozen | **NO — intentionally deferred** |
| Transfer ABI frozen | **NO — intentionally deferred** |
| Capability bit 5 advertised | **NO** |
| Gate 1 source boundary authorized | **NO** |
| Generic storage/filesystem authorized | **NO** |
| Gate 0 fail-closed disposition recorded | **PASS** |

Gate 0 is accepted only as a **deferral decision**. It is not acceptance of Asset/Configuration implementation.

## 5. Reactivation prerequisites / Gate 1 blockers

Reactivation requires all of:

1. a promoted concrete consumer;
2. frozen target schema and validation;
3. exact maximum serialized object size;
4. exact Flash partition proposal including future bootloader/recovery budget;
5. repository-owned reproducible firmware build entrypoint;
6. accepted read-only real-device Flash/revision/protection preflight baseline for the physical board used by the implementation;
7. first-class binary transfer protocol proposal;
8. persistent atomic-commit/recovery model;
9. resource ceilings;
10. wear/timing/watchdog/USB acceptance model;
11. deterministic reset/power-loss fault-injection matrix;
12. recovery bundle procedure.

If any item is absent, Gate 1 remains blocked.

Items 1–12 are satisfied, all six design contracts are frozen, and the Section 0A closure audit passes. Gate 1 is authorized only within the frozen bounded source boundary.

## 6. Protocol rejection criteria

The reopened boundary must reject:

- hex/Base64 blob tunneling through string RPC arguments as the primary transfer mechanism;
- reinterpretation of accepted v1 frame/RPC meanings;
- unbounded buffering;
- NUL/string escaping tricks used as binary transport;
- a host-only transfer API with no target consumer;
- capability advertisement before full acceptance.

## 7. Persistence rejection criteria

The reopened boundary must reject any design where:

- partial upload can become authoritative;
- a reset can leave two ambiguous active records;
- integrity is checked only after activation;
- there is no previous/default fallback;
- schema mismatch is silently accepted;
- Flash pages are not reserved from normal firmware growth;
- erase/write wear has no bounded consumer-driven budget;
- destructive writes occur before real-device preflight.

## 8. Infrastructure prerequisites

Before future Gate 1:

### Firmware build — accepted

Accepted versioned entrypoint: `scripts/build_firmware.ps1`.

Independent reproducibility acceptance reproduced the canonical 50652-byte `os.bin` byte-for-byte. Asset Gate 1 must update/re-accept the entrypoint for the transitional standalone linker ceiling (`0x08000000`, 54 KiB) while retaining reset ownership at Flash base. Relocation to `0x08002000` is deferred to the later Bootloader boundary.

### Hardware preflight — accepted current-board baseline

The read-only physical-board preflight was captured and evidence-finalized on 2026-09-21 without target or repository mutation.

Accepted values:

- STM32CubeProgrammer device ID `0x410`;
- `DBGMCU_IDCODE=0x20036410`;
- numeric `REV_ID=0x2003`;
- numeric `DEV_ID=0x410`;
- factory Flash-size register `0xFFFF0040`, low halfword `0x0040 = 64 KiB`;
- `FLASH_OBR=0x000003FC`;
- `FLASH_WRPR=0xFFFFFFFF`;
- RDP disabled;
- WRP0..31 inactive;
- accepted SWD frequency `950 kHz`;
- target voltage observed `3.15–3.16 V`;
- six captured native commands, all `EXIT_CODE=0`;
- `NONVOLATILE_MUTATION_REQUESTED=NO`.

Evidence identity:

- immutable source preflight log SHA-256 `B112A754E82ECC01BDC509B2D8FB359D652D74DDC565B3DE046C30C181672C13`;
- evidence-finalization external log SHA-256 `E9196D310B79986477B9255F9D94883D84E9E2AC617086CCC97CB021272D41AF`;
- finalized evidence ZIP SHA-256 `BB1F928A2E716F2C8D0FA6B160FC7F41B3187A75CF869DD139DF0C450D6B42D8`;
- final outcome `PASS`, classification `NONE`.

This satisfies the read-only hardware-preflight prerequisite for the current physical board. It does **not** authorize Flash mutation by itself. Revalidate this prerequisite if the physical MCU/board changes or if protection state is intentionally changed.

### Recovery — Gate 0 procedure still open

Gate 0 has frozen the recovery requirement in `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md`. Gate 1/2 must materialize a candidate-bound recovery bundle using a fresh repository-built image; stale ignored `build/` state remains forbidden.

### Host restore — accepted

Seven project-local `packages.lock.json` files are committed and locked restore/build/tests are accepted on the Windows/Linux SDK matrix. The previous package-lock ambiguity is closed.

## 9. Resource acceptance when reactivated

A future Gate 2 must not silently inherit old ceilings.

It must freeze and prove new ceilings covering:

- firmware code growth;
- transfer state/parser;
- persistence metadata;
- scratch buffers;
- task-stack margins;
- persistence pages removed from application Flash;
- future bootloader/recovery budget.

Whole-object RAM buffering is not assumed acceptable.

## 10. Hardware acceptance when reactivated

Future hardware acceptance must prove at minimum:

- exact real-device preflight values recorded before destructive work;
- normal management/CDC behavior remains bounded;
- transfer begin/chunk/retry/commit/abort/status/readback semantics;
- duplicate/reconnect behavior;
- no partial activation;
- previous/default fallback;
- reset at deterministic transaction points;
- watchdog survival;
- scheduler progress;
- bounded Flash operation latency;
- zero unexpected USB/PMA/management/CDC regressions;
- final persistent object readback/integrity;
- final firmware Flash image remains exact outside the explicitly owned persistence region.

## 11. Publication rule

No publication may claim Asset/Configuration capability until all implementation gates pass.

Publication state is now split explicitly:

- through Gates 1–5, the published system capability mask stayed `0x0000001F` and bit 5 remained reserved only;
- the Gate-6 activation candidate advertises `0x0000003F` and its production `PublishedOnly` transfer path is hardware-accepted, but this remains unpublished WIP state until the Gate-6 local acceptance commit exists and Gate 7 completes ordinary non-force publication;
- pages 62/63 remain owned by the accepted Asset A/B persistence implementation; Gate-6 activation did not mutate them and persistence wear remains `46/64`;
- no broader generic storage/filesystem/package surface is authorized by this activation.

## 12. Historical Gate 0 final outcome

The original fail-closed Gate 0 evidence remains:

`HISTORICAL_FINAL_OUTCOME=DEFERRED_NO_REAL_CONSUMER`

Meaning at that time:

- architecture review completed;
- speculative implementation prevented;
- no accepted product regression;
- no Gate 1 implementation authorized.

## 13. Current boundary outcome

`CURRENT_OUTCOME=GATE_6_ACCEPTED_GATE_7_AUTHORIZED`

`GATES_0_TO_5_ACTIVE_WIP_ACCEPTANCE=PASS`

`GATE_5_CAMPAIGN_A=PASS`

`GATE_5_CAMPAIGN_B=PASS`

`GATE_5_CORRUPTION_MATRIX=PASS`

`GATE_5_PHYSICAL_RETENTION=PASS`

`GATE_5_PERSISTENCE_ERASE_ATTEMPTS=46/64`

`GATE_5_FINAL_STATE=CLEAN_STATE_POST_RESET_BYTE_EXACT`

`GATE_6_AUTHORIZED=YES`

`GATE_7_AUTHORIZED=YES`

The boundary remains unpublished. Gate 6 owns exactly one post-Gate-5 product-activation source change: add system capability bit 5 to the advertised mask so the accepted production `PublishedOnly` host path becomes usable. That changed firmware candidate passed fresh build/resource checks, Core `28/28`, Transport `5/5`, exact Flash readback and production-policy hardware smoke in `stm32_os_asset_gate6_publication_activation_smoke_v4_20260926_134106.evidence.zip`, SHA-256 `7A65CA036E29FE06E39C5ADA2B70461292DBFD3DC584D28C3563A7E498322C60`. Accepted identities: firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, host tree `136687e80c42bd8104ad6c37fbbccb915b60fd08`, BIN SHA-256 `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`, whole-Flash SHA-256 `CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`, Flash `54268/54272`, SRAM `11944/12288`, system capabilities `0x0000003F`, persistence wear unchanged `46/64`. The exact 42-path local commit candidate is the Gate-6 acceptance boundary; after that commit exists, Gate 7 is authorized to perform only the ordinary non-force publication proof.
