# OLED UI Layout and Customization Plan

Status: PLANNED / SUPERSEDES PIXEL-BY-PIXEL MANUAL UI TUNING

This document defines a configurable UI composition layer for the native
128x32 OLED.

The goal is to stop encoding appearance decisions directly in `kernel.c` and
to make layout experiments reproducible, selectable, importable, and later
configurable from a PC.

## 1. Core principle

The display pipeline is split into three independent concerns:

```text
semantic state
  status / console / future widgets
            |
            v
layout configuration
  rectangles / borders / separators / padding
            |
            v
rendering
  mono framebuffer -> SSD1306 present
```

Semantic modules must not own global screen geometry.

The layout system decides where semantic modules render.

## 2. Why this layer is needed

The current hardware experiments proved that one-pixel UI changes matter on a
128x32 display.

Hardcoding those choices directly in command handlers creates several
problems:

- every visual experiment requires source surgery;
- accepted semantic components become coupled to temporary coordinates;
- layout variants are hard to compare;
- user preferences cannot be changed at runtime;
- a future PC configurator would have no stable target API.

The UI layout layer solves this without adding heap allocation or a generic
display-backend vtable.

## 3. Screen coordinate model

Physical screen:

```text
x = 0..127
y = 0..31
```

Every region is represented as an ordinary `mono_rect_t`.

No hidden Y remapping is allowed.

The UI layer may compose:

- status region;
- status separator;
- console region;
- optional outer borders;
- future widget regions.

## 4. Planned layout data structure

Initial target model:

```c
enum
{
    OLED_UI_BORDER_TOP    = 1u << 0,
    OLED_UI_BORDER_LEFT   = 1u << 1,
    OLED_UI_BORDER_RIGHT  = 1u << 2,
    OLED_UI_BORDER_BOTTOM = 1u << 3
};

typedef struct
{
    mono_rect_t status_rect;
    mono_rect_t console_rect;

    int8_t separator_y;
    uint8_t border_flags;

    uint8_t status_enabled;
    uint8_t separator_enabled;
} oled_ui_layout_t;
```

The exact field packing may change during implementation, but the ownership
model is fixed.

No field may contain SSD1306 page numbers.

## 5. Layout validation

Every custom layout must pass validation before activation.

Validation must reject at least:

- rectangles outside `128x32`;
- negative width/height;
- status and console overlap when not explicitly allowed;
- separator outside screen bounds;
- separator intersecting semantic regions;
- console too small for at least one glyph;
- impossible border coordinates;
- layouts that would overwrite reserved regions.

Invalid runtime configuration must not partially apply.

Apply configuration transactionally:

```text
candidate
 -> validate
 -> accept whole layout
```

or reject it unchanged.

## 6. Presets

Initial built-in presets:

### `minimal`

Design goal:

- no left border;
- no right border;
- no bottom border;
- status bar is the primary structural element;
- console uses the remaining screen with minimal padding.

This is the leading candidate for the production UI.

The precise separator/status height is not frozen until a pixel mockup is
physically accepted.

### `boxed`

Compatibility/reference preset:

- full frame;
- status separator;
- conservative console placement.

This preserves the visual style of the accepted Slice 4 baseline for
comparison.

### `compact`

Design goal:

- no outer box;
- minimal status/console gaps;
- maximize useful text area without reducing readability.

Exact geometry is selected by mockup + hardware acceptance.

### `custom`

Runtime-editable layout.

It starts from a valid preset and receives explicit parameter changes.

## 7. Current visual experiment status

The first static status-bar experiment proved:

- 3x4 status font feasibility;
- isolated status rendering;
- `oledstatus` command path;
- a 21-column console can start at `x=2` when horizontal fit uses glyph width;
- protocol and framebuffer isolation can pass.

However, that visual composition is NOT accepted for commit.

Reason:

- full left/right/bottom frame is now considered visually too heavy;
- appearance should be selected through the new layout system rather than more
  hardcoded coordinate edits.

The experiment remains a useful implementation prototype only.

## 8. External layout design workflow

Pixel-perfect mockups should be designed on a literal:

```text
128 x 32
1-bit
1 pixel grid
```

Suitable editors include:

- Aseprite;
- LibreSprite;
- Piskel;
- other pixel-art/image editors that preserve exact pixels.

A general design tool such as Figma may be used for conceptual work, but the
final artifact must be rasterized and verified on the exact 128x32 grid.

## 9. Import model

The STM32 will NOT decode PNG, SVG, Figma files, or editor project files.

Desktop tooling converts design artifacts into one of two target forms.

### 9.1 Parametric layout

For ordinary UI structure:

```text
status rectangle
console rectangle
separator
border flags
padding
preset id
```

This is the preferred format because semantic content remains dynamic.

### 9.2 Static 1-bit assets

For icons/logos/background fragments:

```text
PNG/BMP source on PC
 -> desktop converter
 -> 1-bit packed bitmap
 -> firmware asset or upload payload
```

A whole-screen screenshot must not replace semantic rendering as the normal UI
architecture.

## 10. Desktop interchange format

First desktop-facing format should be human-readable.

Proposed example:

```json
{
  "version": 1,
  "preset": "custom",
  "status": { "x": 1, "y": 0, "w": 126, "h": 6 },
  "separator_y": 6,
  "console": { "x": 2, "y": 8, "w": 126, "h": 24 },
  "borders": {
    "top": false,
    "left": false,
    "right": false,
    "bottom": false
  }
}
```

This JSON is a PC-side interchange format only.

The MCU does not need a JSON parser.

The PC tool translates it into CLI commands or a future compact binary
protocol.

## 11. Runtime command model

The UI configuration API must be transport-independent.

Planned commands:

```text
ui show
ui preset list
ui preset minimal
ui preset boxed
ui preset compact

ui set status_x <n>
ui set status_y <n>
ui set status_w <n>
ui set status_h <n>

ui set separator_y <n>
ui set separator on|off

ui set console_x <n>
ui set console_y <n>
ui set console_w <n>
ui set console_h <n>

ui set border_top on|off
ui set border_left on|off
ui set border_right on|off
ui set border_bottom on|off

ui reset
```

Initial configuration lives in RAM only.

Later:

```text
ui save
ui load
```

may persist a validated configuration in Flash.

## 12. Transport independence

Today:

```text
PC keyboard
 -> terminal
 -> HW-193 UART
 -> command parser
 -> UI config API
```

Later:

```text
PC keyboard / configurator
 -> USB CDC
 -> same command parser
 -> same UI config API
```

Therefore the UI command parser must not depend directly on UART registers.

UART and USB CDC are transports, not UI semantics.

No physical keyboard or mouse needs to be connected to the STM32 for normal
configuration.

## 13. Future PC configurator

A dedicated PC configurator is planned after the target-side layout API is
stable.

Minimum features:

- exact 128x32 preview;
- 1-pixel grid;
- drag/resize status and console regions;
- border toggles;
- preset selector;
- live validation;
- export/import layout file;
- send active configuration to the board;
- later save/load board configuration.

The configurator may use mouse and keyboard on the PC.

It communicates with the same logical UI configuration API used by the text
CLI.

## 14. Status bar relationship

`oled_status_bar` remains a semantic renderer.

It owns:

- COMM state;
- time state;
- micro-font/icons;
- rendering inside the clip it receives.

It must NOT own:

- global status rectangle coordinates;
- separator position;
- outer borders;
- console geometry;
- display present/flush.

The UI composition layer owns those decisions.

## 15. Console relationship

`oled_console` remains a semantic 21x3 retained console until an explicit
capacity change is accepted.

The console renderer receives a clip from the layout layer.

Horizontal fit must be glyph-based, not cell-advance-only, so a trailing
spacer may be clipped without losing the final glyph.

The console does not own outer borders or status spacing.

## 16. Planned modules

Target files:

```text
include/kernel/oled_ui_layout.h
src/kernel/oled_ui_layout.c
```

Potential later PC-side tools:

```text
tools/oled_ui_preview/
tools/oled_ui_convert/
```

Do not build desktop tooling before the target layout contract is accepted.

## 17. Slice sequence

### Slice 4B.1 — layout engine + presets + static status proof

Scope:

- add validated `oled_ui_layout_t`;
- add `minimal`, `boxed`, `compact` presets;
- move frame/separator/region placement out of ad-hoc command code;
- retain 3x4 micro-font + static `12:34`;
- retain real COMM/UART icon;
- render status and console from selected layout;
- add runtime preset switching;
- no persistence;
- no uptime integration yet.

Acceptance includes physical comparison of at least two presets.

### Slice 4B.2 — custom runtime editing

Scope:

- `ui show`;
- `ui set ...`;
- transactional validation;
- runtime `custom` layout;
- no Flash persistence yet.

Acceptance:

- invalid layout is rejected without changing current layout;
- valid one-pixel changes are visible immediately after explicit redraw;
- console/status semantic state survives layout changes.

### Slice 4B.3 — uptime integration

Scope:

- status `HH:MM` from uptime;
- update status state only when displayed minute changes;
- COMM state reflects real UART availability;
- layout remains independent.

### Slice 4B.4 — desktop import/configurator protocol

Only after target-side config API is stable:

- define PC interchange format;
- implement converter/sender;
- use UART first;
- USB CDC later uses the same API.

### Slice 4B.5 — optional persistence

Only when runtime configuration semantics are stable:

- versioned Flash record;
- validation on load;
- CRC/check value;
- safe fallback to built-in preset.

## 18. Acceptance invariants

Every UI-layout slice must preserve:

```text
OLED native geometry = 128x32
framebuffer           = 512 bytes
console semantic size = 21x3 unless separately approved
status component      = no I2C
console component     = no I2C
layout component      = no I2C
present               = explicit orchestration
```

Existing regressions remain mandatory:

```text
BOOT OK
PONG
0x3C
OLED_CMD_OK
OLED_TEXT_OK
OLED_RENDER_EQ_OK
OLED_RENDER_OK
OLED_CONSOLE_OK
```

Status slices additionally require:

```text
OLED_STATUS_ISOLATION_OK
OLED_STATUS_OK
```

## 19. Design-review workflow

For visual work:

```text
mockup variants
 -> compare at 128x32
 -> encode as preset/custom layout
 -> build/flash
 -> physical inspection
 -> accept/reject
```

Do not commit a visual preset merely because its protocol test passes.

## 20. Stop conditions

Reject a change if it:

- pushes layout coordinates back into semantic modules;
- requires PNG/SVG decoding on STM32;
- requires heap allocation;
- couples UI configuration directly to UART;
- breaks existing console/status isolation;
- changes panel geometry;
- introduces a persistent configuration format before runtime semantics are
  stable;
- commits a visual layout before physical acceptance.
