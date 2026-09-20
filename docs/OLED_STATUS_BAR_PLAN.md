> [!IMPORTANT]
> **Accepted OLED geometry/rendering baseline remains frozen.**
> The hardware-accepted geometry and firmware fingerprint are in
> [`OLED_UI_ACCEPTED_BASELINE.md`](OLED_UI_ACCEPTED_BASELINE.md).
> This file is deferred future design. It does not authorize implementation and
> must not override accepted SYSTEM/USB/NETWORK/uptime semantics.

# OLED Status Bar Plan

Status: **DEFERRED FUTURE UI CONSUMER — NOT AN ACTIVE ROADMAP BOUNDARY; CURRENT STATE IN `docs/CURRENT_STATE.md`**

The purpose of this plan is limited: if configurable UI layout work is promoted
later, status-bar **placement/style** may become configurable without moving
system/service truth into the presentation layer.

## 1. Semantic responsibility

The status renderer may own:

- tiny glyph/icon rendering;
- rendering inside the clip supplied by the layout/composition layer;
- presentation-local dirty tracking derived from changed displayed values.

It must not own:

- scheduler/system health truth;
- USB state truth;
- network-service truth;
- timekeeping truth;
- transport discovery/session state;
- global layout coordinates;
- I2C/panel presentation.

Authoritative system/service state remains upstream of the OLED presentation.

## 2. Accepted information model

Future layout work must preserve the accepted status semantics unless a separate
boundary explicitly changes them.

Current accepted semantic fields are:

- **SYSTEM** — scheduler/task/watchdog readiness;
- **USB** — actual target USB/service state used by the accepted runtime;
- **NETWORK** — inactive until a real network service exists;
- **time** — monotonic uptime `HH:MM`, saturating according to the accepted UI contract.

Do not invent “connected” state for UART or ST-LINK.

Do not display fake Wi-Fi/network state.

A future RTC wall clock may reuse the time field only after a dedicated time/RTC
contract is accepted.

## 3. Rendering contract

The renderer receives a validated clip from the UI layout layer.

It may modify pixels only inside that clip.

It must not:

- draw global borders/separators;
- move application/console content;
- perform I2C;
- flush/present the OLED;
- know SSD1306 page layout;
- infer system/service state from already-rendered pixels.

## 4. Historical prototype note

An early Slice 4B.1 experiment proved:

- 3x4 digit rendering;
- pixel isolation;
- status rendering mechanics.

Its `COMM/UART` icon and static `12:34` semantics are historical prototype material
and are **not** the current product status model.

The visual composition was not accepted as the final configurable layout.

## 5. Future acceptance if this work is promoted

Before implementation, create a dedicated accepted boundary or explicitly include
this work in a promoted UI/configuration boundary.

Acceptance must prove at least:

- accepted SYSTEM/USB/NETWORK/time semantics are preserved;
- layout/style changes do not change authoritative service state;
- at least two materially different presets are physically compared if presets are in scope;
- invalid configuration is rejected atomically;
- no blank pulse/stale pixels/console corruption;
- no new transport-specific UI configuration logic.

Uptime integration is already complete and is **not** a future prerequisite.

## 6. Refresh policy

Status presentation becomes dirty only when a displayed value or its layout changes.

Examples:

- displayed minute changes -> dirty;
- SYSTEM/USB/NETWORK displayed state changes -> dirty;
- layout/preset changes -> dirty.

No 1 Hz OLED flush is required merely because a clock exists.

Presentation remains caller/orchestrator controlled.

## 7. External customization

PC tools may create exact `128x32` previews and validated layout parameters.

The MCU must receive bounded validated configuration/assets, not PNG/SVG editor files
or arbitrary executable content.

Any persistent layout storage must consume the accepted
`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` contract. This plan must not define a
parallel Flash format.

Host transfer should use the existing transport-neutral management/RPC architecture;
UART remains emergency diagnostics and CDC remains secondary diagnostics.

See:

`docs/OLED_UI_LAYOUT_PLAN.md`
