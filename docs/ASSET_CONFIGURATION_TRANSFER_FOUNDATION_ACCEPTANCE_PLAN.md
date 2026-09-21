# Deus OS — Asset / Configuration Transfer Foundation Acceptance Plan

Status: **GATE 0 REOPENED / IN PROGRESS — OLED_UI_LAYOUT_CONFIG_V1 PROMOTED — GATE 1 BLOCKED**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Canonical design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

## 0. Current Gate 0 reactivation state — 2026-09-21

The original Gate 0 deferral remains an accepted historical fail-closed result. The boundary is now reopened because the missing concrete-consumer prerequisite has been satisfied by `OLED_UI_LAYOUT_CONFIG_V1`.

Reactivation repository baseline:

- `HEAD == origin/main == 8a1fe87572a46b50b503ed5008c080705c517036`;
- working tree clean;
- ahead/behind `0/0`;
- `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` already published at that baseline.

Current reactivation matrix:

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
| Recovery-bundle / ST-LINK restoration procedure | **OPEN — Gate 0 work** |
| Capability bit 5 advertised | **NO — correctly blocked** |
| Gate 1 implementation authorized | **NO** |

Gate 0 is therefore legitimately reopened but **not yet accepted complete**.

No source implementation, linker migration, target Flash mutation, capability activation or hardware run is accepted by this reactivation record.

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

Items 1–11 are now satisfied by the promoted consumer, frozen Flash decision, accepted infrastructure readiness and the five frozen Asset design contracts. Item 12 — recovery bundle / ST-LINK restoration — remains the sole open Gate 0 contract.

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

Before destructive self-programming acceptance, Gate 0 must freeze the exact recovery bundle and ST-LINK restoration procedure using a fresh repository-built known-good image rather than stale ignored `build/` state.

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

While reopened Gate 0 remains incomplete:

- system capability mask stays `0x0000001F`;
- bit 5 remains reserved only;
- no transfer CLI/Desktop surface is considered accepted product functionality;
- pages 62/63 are reserved by the shared Flash decision but no Asset/Configuration runtime erase/program owner exists yet;
- Gate 1 source/linker/Flash implementation remains unauthorized.

## 12. Historical Gate 0 final outcome

The original fail-closed Gate 0 evidence remains:

`HISTORICAL_FINAL_OUTCOME=DEFERRED_NO_REAL_CONSUMER`

Meaning at that time:

- architecture review completed;
- speculative implementation prevented;
- no accepted product regression;
- no Gate 1 implementation authorized.

## 13. Current reactivation outcome

`CURRENT_OUTCOME=GATE_0_REOPENED_IN_PROGRESS`

`GATE_1_AUTHORIZED=NO`

The concrete consumer and prerequisite substrate are now present, but the six OPEN Gate 0 contracts in Section 0 must be frozen and accepted before Gate 1 may begin.
