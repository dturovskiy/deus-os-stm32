# Deus OS — Semantic System / Service State Plan

Status: **FDC-06 / GATE 0 CONTRACT FROZEN**

Baseline: `21e45b1dd7b1b5b701b087f44d1b29676ce0e6d9`

## Purpose

Make semantic system/service state the single upstream authority for application-runtime and OLED presentation consumers.

## Confirmed audit finding

Current `src/kernel.c` builds `application_service_snapshot_t` by interpreting `boot_desktop_ui_snapshot_t` indicator values. That makes presentation state an authority for system health/USB/network semantics and is the exact dependency inversion identified by FDC-06.

## Frozen design

Introduce one bounded, allocation-free semantic owner:

- `include/kernel/system_service_state.h`
- `src/kernel/system_service_state.c`

The semantic state contains only the currently real shared facts required by consumers:

- uptime milliseconds;
- displayed monotonic minute;
- system-health boolean;
- USB-configured boolean;
- network-online boolean (currently false until a real network service exists).

`src/kernel.c` updates that state directly from authoritative runtime sources. Both consumers are downstream:

`runtime/scheduler/USB/time facts -> system_service_state -> application_service_snapshot + OLED status presentation`

No OLED indicator enumeration/value may be parsed to reconstruct semantic state.

## Frozen source boundary

Authorized new files:

- `include/kernel/system_service_state.h`
- `src/kernel/system_service_state.c`

Authorized modification:

- `src/kernel.c`
- `scripts/build_firmware.ps1`

No public RPC/command/application ABI change is authorized.

## Behavioral invariants

- existing SYSTEM/USB/NETWORK/uptime visible semantics remain unchanged;
- semantic-event delivery remains based on semantic changes, not redraw occurrence;
- a semantically unchanged service pass must not create an application event or force OLED rerender;
- network remains offline rather than fabricated;
- no second renderer abstraction is introduced merely for this closure.

## Gates

0. contract/source boundary freeze;
1. implementation/static dependency-direction proof;
2. fresh build/resource/stack and deterministic semantic-state tests;
3. target runtime semantic-event/no-rerender regression;
4. physical OLED regression;
5. documentation reconciliation;
6. local acceptance commit;
7. ordinary non-force publication.

## Exit criterion

FDC-06 closes only when application and OLED state are independently derived from the same semantic owner, UI state is no longer an authority for service truth, and target/physical behavior is proven equivalent.
