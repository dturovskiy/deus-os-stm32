# Deus OS — OLED UI Layout Configuration v1 Consumer Contract

Status: **PROMOTED/FROZEN CONCRETE PERSISTENT CONSUMER — CONSUMED BY ACTIVE ASSET WIP CANDIDATE — THIS FILE DOES NOT AUTHORIZE NEW SOURCE CHANGES**

Consumer ID:

`OLED_UI_LAYOUT_CONFIG_V1`

Asset object type:

`0x0001`

This value is in the Asset/Configuration object-type namespace. It is not an application ID, RPC method ID or protocol frame type.

## 1. Purpose

This contract promotes one deliberately small, non-executable configuration object so `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` has a real product consumer.

The object persists only the OLED console clip within the already accepted 128x32 UI.

It does **not** make the entire deferred configurable-layout plan active.

The accepted system status bar remains system-owned and frozen at:

```text
x = 0
y = 0
w = 128
h = 9
```

Pixel row `y=9` remains the reserved blank gap.

The configurable/persisted v1 object controls only the console rectangle below that gap.

## 2. Why this is the v1 consumer

The current firmware already owns console presentation through `oled_ui_layout_t`, and the console renderer accepts a supplied clip.

The current status renderer, however, intentionally requires the accepted `128x9` status geometry.

Therefore v1 does not pretend that arbitrary status geometry, borders, presets, bitmaps or a general UI package are already real consumers.

This object is the smallest useful persistence consumer that:

- has an existing semantic owner;
- changes a visible product behavior;
- can be strictly validated;
- has a deterministic compiled default;
- is non-executable;
- requires persistence to survive reset/power-cycle;
- does not require a filesystem or generic asset registry.

## 3. Exact serialized payload

The **consumer payload is exactly 8 bytes**.

All multibyte ambiguity is avoided: every field is one byte.

| Offset | Field | Type | Required value / meaning |
| ---: | --- | --- | --- |
| 0 | `schema_version` | `uint8_t` | exactly `1` |
| 1 | `console_x` | `uint8_t` | left edge in pixels |
| 2 | `console_y` | `uint8_t` | top edge in pixels |
| 3 | `console_width` | `uint8_t` | width in pixels |
| 4 | `console_height` | `uint8_t` | height in pixels |
| 5 | `flags` | `uint8_t` | exactly `0` in v1 |
| 6 | `reserved0` | `uint8_t` | exactly `0` |
| 7 | `reserved1` | `uint8_t` | exactly `0` |

Exact serialized consumer payload size:

`8 bytes`

No padding, native C struct layout, pointer, SSD1306 page number, JSON text or transport-specific representation is part of the target payload.

The Asset/Configuration persistence envelope is separate and will carry record metadata such as object type, generation, payload length, integrity and commit-validity state.

## 4. Compiled default

The compiled default is the currently accepted console region:

```text
console_x      = 1
console_y      = 10
console_width  = 126
console_height = 22
```

Serialized v1 default:

```text
01 01 0A 7E 16 00 00 00
```

where:

- `01` = schema v1;
- `01` = x 1;
- `0A` = y 10;
- `7E` = width 126;
- `16` = height 22;
- final three bytes are zero.

The compiled default remains authoritative whenever no valid persisted object exists.

## 5. Strict validation

A candidate is valid only if **all** conditions pass before activation:

1. payload length is exactly `8`;
2. `schema_version == 1`;
3. `flags == 0`;
4. `reserved0 == 0`;
5. `reserved1 == 0`;
6. `console_width >= 5` so at least one current 5-pixel console glyph can render;
7. `console_height >= 6` so at least one current 6-pixel console glyph can render;
8. `console_y >= 10`, preserving the frozen status bar and row-9 gap;
9. `console_x + console_width <= 128`;
10. `console_y + console_height <= 32`;
11. arithmetic is evaluated in a widened integer type so byte overflow cannot make an out-of-range rectangle appear valid.

Any failure rejects the entire object.

Partial field application is forbidden.

## 6. Activation semantics

Runtime activation is transactional at the consumer boundary:

```text
candidate payload
 -> exact-length/schema validation
 -> rectangle validation
 -> accept whole consumer object
 -> apply console clip
```

or:

```text
candidate payload
 -> any validation failure
 -> reject unchanged
```

Persistence activation adds the Asset/Configuration transaction layer:

```text
receive candidate
 -> transfer integrity
 -> consumer validation
 -> program inactive persistent slot
 -> verify
 -> atomic commit
 -> select
 -> activate
```

A candidate must never become active merely because some bytes were received or programmed.

## 7. Boot/reset selection semantics

At boot the target must resolve exactly one authoritative configuration:

1. newest committed valid `OLED_UI_LAYOUT_CONFIG_V1` record;
2. otherwise previous committed valid v1 record;
3. otherwise compiled default.

An incompatible schema version, invalid payload, failed integrity check, incomplete record or ambiguous record must not partially affect UI state.

## 8. Why volatile-only state is insufficient

The product requirement for this consumer is:

> an operator-selected console layout survives normal reset, watchdog reset and power removal without requiring the host to resend it.

A RAM-only setting cannot satisfy that requirement.

Persistence is therefore intrinsic to this consumer rather than a speculative storage exercise.

## 9. Expected update frequency / wear intent

This is human/operator configuration, not telemetry.

Design intent:

- writes are explicit operator actions;
- unchanged payload must suppress erase/program activity;
- automatic periodic writes are forbidden;
- boot does not rewrite a valid record;
- normal display refresh does not touch Flash.

The exact accepted erase-frequency/lifetime budget is frozen in reopened Asset Gate 0 before Gate 1.

## 10. Transport position

The consumer has no transport ownership.

The same exact 8-byte payload may arrive through the future accepted Asset/Configuration binary transfer contract over the management plane.

UART, CDC, WinUSB, libusb, a future local Web frontend or later network transport must not redefine the consumer schema.

PC-side JSON may exist as an interchange/editor format, but it must be converted to this exact binary target payload before transfer.

The MCU does not require a JSON parser.

## 11. Flash position

This consumer does not own raw Flash addresses directly.

Persistence must consume the shared A/B pages frozen by:

`docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`

- slot A: page 62, `0x0800F800..0x0800FBFF`;
- slot B: page 63, `0x0800FC00..0x0800FFFF`.

The Asset/Configuration foundation owns the generic record envelope and atomic selection rules inside those pages.

The OLED consumer must not create a parallel UI-specific Flash format.

## 12. Explicit non-goals

v1 does not persist or configure:

- status-bar geometry;
- status semantics or indicators;
- separator/border style;
- fonts;
- bitmaps/icons;
- arbitrary static assets;
- OLED contrast;
- panel geometry;
- application installation;
- executable code;
- filesystem paths;
- generic key/value settings.

Those require later promoted consumers or schema versions.

## 13. Source-boundary implication

When Gate 1 is eventually authorized, consumer-specific source work may extend the current layout validation/selection path only as needed to support this v1 console rectangle.

The accepted `128x9` status renderer contract remains unchanged.

The existing default rectangle remains the fallback and must continue to pass all retained OLED/UI regressions.

## 14. Acceptance implication

Consumer-specific acceptance must prove at minimum:

- compiled default unchanged when no valid persistence exists;
- at least one non-default valid console rectangle is accepted and visibly used;
- invalid width/height/bounds/schema/reserved fields are rejected atomically;
- persisted valid state survives reset/power-cycle;
- corrupted/incomplete newest state falls back to previous valid or compiled default;
- status bar remains physically and semantically unchanged;
- no Flash write occurs for an unchanged payload.

Physical OLED review is required for at least default and one non-default accepted configuration.

## 15. Relationship to broader deferred UI planning

`docs/OLED_UI_LAYOUT_PLAN.md` remains the broader deferred design for presets, custom status placement, borders, PC configurator workflows and future assets.

This v1 contract intentionally promotes only one narrow consumer from that design space.

It must not be used to claim that the broader configurable-layout roadmap is active.
