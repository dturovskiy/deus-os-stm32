# OLED Console Architecture

Status: ACTIVE / RETAINED 21x3 CONSOLE HARDWARE-ACCEPTED

## 1. Goal

Build a small, deterministic OLED kernel console on top of the hardware-proven
128x32 SSD1306-compatible module.

The design must remain:

- heap-free;
- lightweight;
- deterministic;
- easy to test on real hardware;
- independent of SSD1306 page semantics above the display driver;
- suitable for later kernel logging without coupling faults to the OLED.

## 2. Canonical pipeline

```text
kernel / commands / future klog
            |
            v
      UI composition
 status bar + console viewport
            |
            v
      oled_console
 retained 21 x 3 char state
 cursor / wrap / newline / future ring scroll
            |
            v
      text_renderer
 5x7 font / opaque 6x8 cells
 generic path + equivalent aligned fast path
            |
            v
         mono_fb
 512-byte framebuffer
 primitives / clipping / dirty pages
            |
            v
        ssd1306
 128x32 profile / init / present
            |
            v
           I2C
```

Do not add a generic display-backend vtable yet. There is only one display
backend.

## 3. Module responsibilities

### `drivers/ssd1306`

Owns only:

- SSD1306-compatible controller profile;
- I2C address `0x3C`;
- init sequence;
- native 128x32 geometry;
- full-frame/dirty-page presentation;
- controller power/display commands.

It must not own fonts, text layout, console state, status-bar state, or semantic
scrolling.

### `gfx/mono_fb`

Owns:

- 1-bit framebuffer format;
- clipping;
- pixel/line/rectangle primitives;
- dirty-page tracking.

Canonical framebuffer size: `512 bytes`.

### `gfx/font5x7`

Owns immutable font data and metrics only.

Current metrics:

- glyph `5x7`
- advance `6x8`

### `gfx/text_renderer`

Owns opaque glyph/cell rasterization.

Accepted behavior:

- coordinate-based public API;
- generic arbitrary-Y path;
- page-aligned fast path only when byte-for-byte equivalent;
- clipping;
- stale-pixel overwrite;
- preservation of caller-owned pixels outside the clip rectangle.

### `kernel/oled_console`

Owns semantic console state:

```c
typedef struct
{
    char cells[3][21];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;
```

It owns:

- cursor;
- wrap;
- newline;
- carriage return;
- row clearing;
- future circular scroll;
- rendering semantic cells through `text_renderer`.

It must know nothing about SSD1306 pages, I2C, or status-bar content.

### UI composition / `kernel.c` for the current slice

Current orchestration owns:

- outer frame;
- status-bar reservation;
- status separator;
- console viewport;
- explicit present.

Status-bar content must become its own component before it grows beyond the
current reserved geometry.

## 4. Accepted geometry

Hardware-visible display:

```text
x = 0..127
y = 0..31
```

Accepted framed layout:

```text
y=0      top frame
y=1..4   reserved status-bar content
y=5      status-bar separator
y=6      gap
y=7..13  console row 0
y=14     gap
y=15..21 console row 1
y=22     gap
y=23..29 console row 2
y=30     bottom gap
y=31     bottom frame
```

Canonical console viewport:

```text
x=1
y=7
width=126
height=23
columns=21
rows=3
```

The console calculates available rows from `clip + font metrics`; it does not
align UI rows to SSD1306 pages.

## 5. Rendering rule

Cells are opaque.

Rendering a character must overwrite:

- all glyph pixels;
- the blank spacer column;
- the blank spacer row when that row is inside the clip rectangle.

This prevents ghost pixels after character replacement.

The final row's spacer falls outside the console clip, leaving caller-owned
`y=30` untouched.

## 6. Scrolling

Slice 4 intentionally has no scrolling. Writes after the third row are ignored.

Slice 5 will use retained semantic rows with a circular first-row index:

```c
first_row = (uint8_t)((first_row + 1u) % 3u);
```

Do not use SSD1306 hardware scrolling.
Do not `memmove` the framebuffer as the primary scroll mechanism.

## 7. Presentation

`oled_console_write()` and `oled_console_putc()` do not flush the display.

Flow:

```text
update semantic state
        ->
render semantic rows into mono_fb
        ->
explicit present
```

The hardware-proven full-frame 512-byte flush remains the baseline.
Dirty-page transfer optimization is a later slice behind the same API.

## 8. Memory model

Accepted persistent display state:

- framebuffer: `512 bytes`
- console cells: `63 bytes`
- console metadata: `4 bytes`
- total `oled_console_t`: `67 bytes`
- framebuffer metadata: separate small object

This is comfortably below the old 128x64 design.

## 9. Design constraints

- no heap;
- no function-pointer pseudo-OO;
- no hidden global Y remap;
- no 2x rendering workaround;
- no hardcoded 128x64 assumptions;
- no hardware scroll in the baseline console;
- no flush inside character-write functions;
- no SSD1306 page knowledge inside the console;
- no critical fault reporting dependency on OLED;
- physical acceptance before every rendering-related commit.

## 10. Status bar boundary

The status bar is a sibling UI region, not part of console semantics.

Reserved geometry:

```text
content:   x=1..126, y=1..4
separator: x=1..126, y=5
```

Content is not implemented in Slice 4.

A future status-bar component may use a micro-font/icons appropriate for the
four-pixel-high content region, but it must not distort console geometry or
couple the console to SSD1306 pages.
