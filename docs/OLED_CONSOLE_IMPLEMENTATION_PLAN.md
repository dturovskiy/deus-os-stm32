> [!IMPORTANT]
> **OLED UI status: ACCEPTED / FROZEN (2026-09-11).**
> The authoritative hardware-accepted geometry and firmware fingerprint are in
> [`OLED_UI_ACCEPTED_BASELINE.md`](OLED_UI_ACCEPTED_BASELINE.md).
> Any configurable-layout, preset, custom-layout, persistence, or alternate-geometry
> material below is deferred planning and must not override the accepted baseline.
# OLED Console Implementation Plan

Status: ACTIVE PLAN / NATIVE 128x32

## Completed and accepted

- SSD1306 address `0x3C`.
- native panel geometry `128x32`.
- framebuffer `512 bytes`.
- SSD1306 driver extraction.
- monochrome framebuffer extraction.
- font5x7 extraction.
- Slice 3A: native 128x32 production baseline.
- Slice 3B: generic opaque text renderer.
- Slice 3C: byte-equivalent aligned fast path.
- Slice 4: retained 21x3 console without scrolling.
- Slice 4 accepted UI geometry:
  - status content `y=1..4`
  - separator `y=5`
  - gap `y=6`
  - console rows `y=7,15,23`
  - bottom gap `y=30`

The old 128x64 renderer experiment is rejected.

## Slice 4 accepted evidence

State:

```c
char cells[3][21];
```

Accepted behavior:

- init;
- clear;
- putc;
- write;
- write_line;
- cursor;
- CR/LF;
- automatic wrap;
- no-scroll overflow ignored;
- render through `text_renderer`;
- no SSD1306/I2C/page knowledge inside `oled_console`.

Temporary/diagnostic command:

```text
oledconsole
```

Accepted physical layout:

```text
frame y=0
status y=1..4
separator y=5
gap y=6
row0 y=7..13
gap y=14
row1 y=15..21
gap y=22
row2 y=23..29
gap y=30
frame y=31
```

Accepted image:

```text
size   = 5680 bytes
SHA256 = C67AEBA137F645F5BECAEB6410382D44257E1B09EA3BE459BD83AC718F3A09AC
```

## Slice 4B — configurable UI layout + status bar

Detailed plans:

```text
docs/OLED_UI_LAYOUT_PLAN.md
docs/OLED_STATUS_BAR_PLAN.md
```

The first hardcoded 4B.1 status experiment passed protocol/isolation checks but
its full outer frame is visually rejected. It remains uncommitted.

The new sequence is:

### Slice 4B.1 — layout engine + presets + static status proof

- add `oled_ui_layout`;
- add validated `minimal`, `boxed`, and `compact` presets;
- move global rectangles/borders/separator out of ad-hoc command code;
- keep 21x3 console semantics;
- use glyph-based horizontal fit so trailing spacer clipping does not discard
  the final glyph;
- keep 3x4 font + real COMM/UART icon + fixed `12:34`;
- add preset switching through the command layer;
- physically compare at least `minimal` and `boxed`;
- no persistence;
- no uptime integration.

### Slice 4B.2 — custom runtime editing

- `ui show`;
- `ui set ...`;
- transactional validation;
- RAM-only custom layout;
- invalid candidate must leave active layout unchanged.

### Slice 4B.3 — uptime integration

- derive `HH:MM` from uptime;
- COMM reflects real UART state;
- status dirty only when displayed minute/COMM state changes.

### Slice 4B.4 — desktop import/configurator protocol

- exact 128x32 preview workflow;
- PC-side layout interchange format;
- PC-side bitmap conversion for icons/assets;
- send config through UART first;
- later USB CDC reuses the same target API.

### Slice 4B.5 — optional persistence

Only after runtime behavior is stable:

- versioned Flash config;
- validation on load;
- integrity check;
- fallback preset.

Do not commit a visual preset until physical acceptance.

## Slice 5 — circular scroll

Add logical row rotation:

```c
first_row = (uint8_t)((first_row + 1u) % 3u);
```

Test by writing more than three lines.

Expected behavior:

- newest three logical lines remain;
- no framebuffer `memmove` as primary scroll mechanism;
- no SSD1306 hardware scroll;
- status bar and separator remain untouched.

## Slice 6 — dirty-page present optimization

Framebuffer page count is four.
Dirty mask uses bits `0..3`.

Requirements:

- full-frame fallback remains available;
- failed I2C transfer must not falsely clear dirty state;
- partial update must preserve controller addressing assumptions;
- compare optimized output with known-good full 512-byte flush.

## Slice 7 — kernel log integration

Add an optional OLED sink after console semantics are stable.

UART remains the primary diagnostic path.
Fault handlers must never depend on OLED availability.

## Deferred

- UTF-8;
- proportional fonts;
- rich widgets;
- hardware scroll;
- animation;
- DMA I2C;
- generic display-backend vtables;
- 128x64 support for hardware not currently installed.

## Stop conditions

Stop immediately if a slice unexpectedly changes:

- I2C address;
- `A8=0x1F`;
- `DA=0x02`;
- `D3=0`;
- start line 0;
- `A1/C8`;
- page count 4;
- framebuffer size 512;
- native 128x32 frame geometry;
- accepted separator/console layout without an explicit UI-layout slice.

Do not suppress warnings to make a slice pass.
