# Deus OS — Asset / Configuration Transfer Foundation Plan

Status: **GATE 0 REOPENED — OLED_UI_LAYOUT_CONFIG_V1 PROMOTED — GATE 1 NOT AUTHORIZED**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Historical initial Gate 0 repository baseline:

- repository `HEAD == origin/main == dd68659959d3c04b75e90bb6e31e5a956099eb92`;
- working tree was clean and ahead/behind `0/0` before this Gate 0 documentation work;
- latest completed product boundary: `HOST_CONTROL_APPLICATION_FOUNDATION`, published at `e0f49f168542fa1cf49bca451e01b0c077aa8d18`;
- accepted firmware candidate tree: `b895955f7738aceb6fca0272d510cc433378c6ab`;
- accepted host candidate tree: `2c5afd9914851300aed15e321cf69c3a2c3daeed`;
- accepted firmware resource state: Flash `50652`, SRAM `11728`, task0/task1 minimum margins `424/424`;
- binary frame protocol remains v1;
- command service remains v3 / registry 36;
- system capability mask remains `0x0000001F`; bit 5 is reserved for Asset/Configuration transfer but is not advertised.

Canonical predecessors:

- `docs/CURRENT_STATE.md`;
- `docs/ARCHITECTURE.md`;
- `docs/ROADMAP.md`;
- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`;
- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`;
- `docs/OLED_UI_LAYOUT_PLAN.md`;
- `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`;
- `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`;
- `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`;
- `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`.

## 0. Gate 0 reactivation — 2026-09-21

The historical Gate 0 deferral remains preserved below, but its blocking condition has now changed.

Reactivation repository baseline:

- `HEAD == origin/main == 8a1fe87572a46b50b503ed5008c080705c517036`;
- working tree clean;
- ahead/behind `0/0`;
- `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` already published at that baseline.

A concrete persistent consumer has been promoted:

- consumer ID: `OLED_UI_LAYOUT_CONFIG_V1`;
- canonical consumer contract: `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`;
- object type: `0x0001`;
- exact consumer payload: `8 bytes`;
- semantic owner: OLED console layout/clip;
- compiled default: console rectangle `x=1, y=10, w=126, h=22`;
- status bar remains frozen/system-owned at `128x9`;
- persistence is required so operator-selected console layout survives reset and power removal.

The shared future Flash map is also frozen by `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`:

- bootloader/recovery pages `0..7`, 8 KiB ceiling;
- future application pages `8..61`, origin `0x08002000`, 54 KiB maximum;
- persistent slot A/B pages `62/63`, 1 KiB each.

Infrastructure prerequisites already accepted before this reactivation:

- repository-owned firmware build entrypoint `scripts/build_firmware.ps1`;
- read-only current-board MCU/Flash/protection preflight;
- seven project-local host package locks and accepted Windows/Linux SDK restore/build/test reproducibility.

Therefore `DEFERRED_NO_REAL_CONSUMER` is no longer the current disposition. Gate 0 is reopened for the remaining architecture/protocol/persistence acceptance work.

**Gate 1 remains blocked.**

Before Gate 1 may be authorized, reopened Gate 0 must still freeze and accept:

1. first-class bounded binary transfer ABI;
2. persistent A/B record envelope and atomic commit/selection rules;
3. new resource ceilings against the 54 KiB future application region;
4. Flash wear/timing/watchdog/USB continuity policy;
5. deterministic reset/power-loss fault-injection matrix;
6. recovery-bundle/ST-LINK restoration procedure for destructive acceptance.

This reactivation is documentation/design work only. It performs no source implementation, linker migration, vector relocation, Flash erase/program, capability advertisement or target I/O.

## 1. Purpose

This boundary exists to add bounded transfer and persistent non-executable configuration/assets only when a concrete target consumer exists.

It is not permission to create a generic storage framework, filesystem, package manager, dynamic loader, executable upload path, firmware updater or speculative persistence abstraction.

The first Gate 0 question is therefore mandatory:

**What exact v1 object is the current firmware going to receive, validate, persist and consume?**

If that question cannot be answered with a concrete current product consumer and a bounded schema, implementation is deferred.

## 2. Historical Gate 0 consumer audit

The repository was audited for current target consumers.

Current firmware/source provides:

- two statically linked system applications;
- local OLED boot/desktop/application presentation;
- system identity/capability reporting;
- binary RPC over CDC/management transports;
- host application lifecycle/diagnostic control.

Current `src/` and `include/` do not contain:

- a persistent settings/configuration reader;
- an asset registry;
- a Flash persistence driver;
- a persistent record/schema owner;
- a configuration package consumer;
- a runtime asset/configuration activation path.

Existing future-consumer documentation does not satisfy the implementation trigger:

- `docs/OLED_UI_LAYOUT_PLAN.md` is explicitly marked **DEFERRED FUTURE UI/CONFIGURATION CONSUMER**;
- that document states that exact target field packing may change during implementation;
- its custom layout, preset persistence and static bitmap upload concepts are future planning and must not override the accepted OLED baseline;
- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md` describes target packages as later USB-transferable non-executable resources and explicitly requires persistence policy before implementation.

Therefore Gate 0 does not identify a current v1 target object whose persistence is required now.

## 3. Historical Gate 0 disposition

Gate 0 disposition is:

`DEFERRED_NO_REAL_CONSUMER`

Consequences:

- no Gate 1 implementation is authorized;
- capability bit 5 remains reserved but unadvertised;
- no target Flash erase/program code may be added under this boundary;
- no persistence Flash region may be consumed merely to reserve future storage;
- no transfer RPC/frame extension may be implemented yet;
- no host upload/download UI or CLI command may be added yet;
- no linker partition change is made solely for this deferred boundary;
- no generic storage/filesystem/package framework is authorized.

This is a fail-closed architectural result, not a product failure.

This historical disposition is superseded for current work by the Gate 0 reactivation record in Section 0; it remains the reason no implementation was started before a concrete consumer existed.

## 4. Reactivation trigger

The boundary may be reopened only when a concrete product boundary promotes a real consumer.

That trigger is now satisfied specifically by `OLED_UI_LAYOUT_CONFIG_V1`; broader OLED customization, arbitrary assets and generic settings remain deferred.

A qualifying consumer must define all of:

1. semantic owner;
2. exact object purpose;
3. exact target representation or frozen target schema;
4. exact validation rules;
5. exact maximum serialized size;
6. when the object is read;
7. when it becomes active;
8. default/fallback behavior when no valid object exists;
9. whether the object is replaceable, append-only or version-migrated;
10. why volatile-only storage is insufficient.

Possible existing ideas such as configurable OLED layout, monochrome assets, device profiles or application data do not qualify merely because they appear in deferred documents.

## 5. Flash partition contract — architecture map accepted / implementation reservation pending

Before the first persistent target write, the reopened Gate 0 must freeze the physical Flash map.

The contract must state:

- exact firmware application region;
- exact persistence region start/end;
- exact erase-page ownership;
- exact future bootloader/recovery budget;
- whether persistence uses one slot, two slots or another bounded scheme;
- linker-visible reservation so normal firmware cannot grow into persistence;
- linker assertions that fail the build on overlap;
- migration consequences if the future bootloader layout changes.

The current linker gives the firmware the full 64 KiB Flash region. That is acceptable for the current accepted baseline but is not acceptable once persistent Flash pages are owned separately.

The architecture-level partition requirement is now satisfied by `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`.

Implementation is explicitly phased:

- Asset/Configuration phase: standalone application remains reset owner at `0x08000000`, but its linker Flash ceiling becomes 54 KiB; pages 54..61 remain unused relocation headroom; pages 62/63 become persistent A/B slots;
- Bootloader phase: bootloader becomes reset owner in pages 0..7 and the same 54 KiB application budget relocates to `0x08002000..0x0800F7FF`.

Asset Gate 1 must **not** relocate the application to `0x08002000` before a bootloader exists.

## 6. Mandatory real-device read-only preflight

Before any destructive Flash test, the reopened boundary must capture a read-only hardware preflight from the actual board.

At minimum record:

- exact DBGMCU device ID;
- exact numeric silicon revision ID;
- factory Flash-size value;
- option-byte state relevant to read/write protection;
- RDP state;
- write-protection state;
- any board-specific condition that changes self-programming safety.

The preflight must use authoritative STM32 documentation for the discovered silicon revision and Flash geometry.

No erase/program operation is authorized until the captured device state is compatible with the frozen design.

## 7. Transfer protocol requirement

Arbitrary object bytes must not be tunneled through the current UTF-8 string-argument RPC representation using hex, Base64 or equivalent text encoding.

Current RPC semantics remain appropriate for bounded control requests, but asset/configuration transfer requires a first-class bounded binary transfer contract.

When reactivated, Gate 0 must freeze an explicit equivalent of:

- begin;
- chunk;
- commit;
- abort;
- status;
- readback/query.

The exact representation may use additive frame types, a dedicated transfer message family, or another explicitly versioned binary mechanism, but it must preserve the existing accepted protocol ABI.

Required semantics include:

- object/type identifier;
- transfer/session identifier;
- total length;
- offset or sequence;
- duplicate/retry behavior;
- idempotency rules;
- out-of-order rejection or explicit support;
- maximum chunk size;
- reconnect/disconnect behavior;
- timeout/abandon behavior;
- error/status model;
- host readback/verification contract.

## 8. Persistent transaction model

Persistent configuration must be fail-closed under reset and power loss.

The reopened Gate 0 must freeze:

- record magic/type;
- schema/version;
- serialized length;
- generation/sequence;
- integrity field;
- commit-validity state;
- inactive/active selection rule;
- validation order;
- boot-time selection rule;
- incompatible-version handling;
- previous-valid fallback;
- default fallback.

A partially received or partially programmed candidate must never become authoritative.

After reset at any point in the transaction, firmware must select exactly one of:

- previous valid object;
- newly committed valid object;
- compiled/default object when no valid persistent object exists.

Ambiguous/half-valid state is forbidden.

## 9. Resource model

The current accepted resource point is Flash `50652`, SRAM `11728`.

Historical boundary ceilings `54780/11824` leave only 96 bytes of SRAM headroom under the previous ceiling, so whole-object buffering is not an assumed design.

Future implementation must prefer bounded streaming/small scratch unless a new SRAM ceiling is explicitly justified and stack/resource proof is repeated.

The reopened Gate 0 must define new resource ceilings that include:

- firmware code growth;
- transfer parser/state;
- persistence metadata;
- scratch buffers;
- reserved Flash pages;
- future bootloader/recovery budget.

## 10. Flash wear and timing

The future design must state:

- maximum erase frequency expected from the real consumer;
- wear budget and expected lifetime;
- whether unchanged data suppresses writes;
- whether generation rotation/wear distribution is needed;
- maximum bounded erase/program transaction length;
- watchdog servicing policy;
- USB/management continuity expectations during erase/program;
- scheduler/task progress expectations.

These must be measured on real hardware during acceptance rather than justified only by nominal timing arithmetic.

## 11. Fault-injection acceptance requirement

Before implementation, acceptance must define deterministic reset/power-loss injection points including at least:

- before erase;
- immediately after erase;
- during programming;
- after payload programming but before final commit-valid marker;
- immediately after final commit-valid marker.

For each point, post-reset state must be deterministic and valid.

Physical power removal is useful as final proof but must not replace deterministic reset/fault injection for systematic coverage.

## 12. Recovery prerequisite

Before any destructive self-programming test:

- produce a fresh firmware build from a repository-owned reproducible build entrypoint;
- preserve BIN/ELF/MAP identities;
- preserve a known-good recovery BIN outside ignored transient build state;
- preserve SHA-256 identities in evidence;
- prove ST-LINK recovery procedure remains functional.

Ignored stale `build/` contents are not an authoritative recovery source.

## 13. Build reproducibility prerequisite — accepted

The prerequisite is accepted. The versioned repository-owned entrypoint is `scripts/build_firmware.ps1`; independent reproducibility proof reproduced the accepted 50652-byte firmware BIN exactly.

Before Gate 1 implementation, that entrypoint remains responsible for:

- exact source list;
- startup assembly;
- compiler/assembler/linker flags;
- linker script;
- source-tree identity injection if required by the accepted protocol;
- ELF/BIN/MAP generation;
- undefined-symbol rejection;
- size/resource checks;
- clean/reproducible output path.

External acceptance harnesses may invoke that entrypoint but must not remain the only source of the firmware build recipe.

## 14. Host reproducibility prerequisite — accepted

The prerequisite is accepted:

- all seven host projects have committed `packages.lock.json` files;
- locked restore/build/tests pass on accepted Windows and Linux SDKs;
- `host/global.json` retains the accepted .NET 10 roll-forward policy;
- Core transfer logic must continue to remain independent of Avalonia and OS-specific transport layers.

## 15. Security boundary

Configuration/asset mutation is destructive state mutation and must reuse the accepted explicit destructive-authorization model or an equally explicit successor frozen by the boundary.

CRC/integrity is not authentication.

This boundary does not authorize firmware authenticity, remote/network trust or executable code upload. Firmware update and network mutation retain separate security/authenticity boundaries.

## 16. Capability activation

`SYSTEM_IDENTITY_CAP_ASSET_CONFIGURATION_TRANSFER` / host `AssetConfigurationTransfer` remains reserved at bit 5.

Bit 5 may become advertised only after the complete implementation boundary passes acceptance and the feature is actually usable.

Advertising a placeholder, partial implementation or host-only implementation is forbidden.

## 17. Non-goals

This deferred boundary does not authorize:

- generic filesystem;
- heap;
- generic block-device framework;
- dynamic application installation;
- arbitrary uploaded ARM code;
- firmware update;
- bootloader implementation;
- network services;
- cryptographic framework added without a concrete trust requirement;
- generic persistence API designed for hypothetical future consumers.

## 18. Gate sequence when reactivated

A future reactivation must explicitly reopen Gate 0 and then use a fresh gate sequence:

- Gate 0 — concrete consumer + architecture/protocol/persistence/partition contract freeze;
- Gate 1 — bounded source implementation;
- Gate 2 — clean build/unit/static/resource acceptance;
- Gate 3 — hardware transfer/persistence/runtime acceptance including read-only preflight;
- Gate 4 — consumer-specific physical/semantic acceptance when applicable;
- Gate 5 — reset/power-loss/reconnect/recovery/fault-injection acceptance;
- Gate 6 — docs finalization + one normal local acceptance commit;
- Gate 7 — ordinary non-force publication and remote verification.

Gate 0 is now active as a documentation/design freeze. Gate 1 and every later implementation gate remain blocked until all open Gate 0 reactivation criteria are accepted.
