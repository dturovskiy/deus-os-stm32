# OLED Console Architecture

Status: PLANNED / BASELINE GEOMETRY CORRECTED TO NATIVE 128x32

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
      oled_console
 retained 21 x 3 char state
 cursor / wrap / newline / ring scroll
            |
            v
      text_renderer
 5x7 font / opaque 6x8 cells
 generic path + page-aligned fast path
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

It must not own fonts, text layout, console state, or semantic scrolling.

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

Requirements:

- coordinate-based public API;
- generic arbitrary-Y path;
- page-aligned fast path only when equivalent;
- clipping;
- stale-pixel overwrite;
- border preservation when a cell shares a controller page with UI geometry.

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
- circular scroll;
- rendering semantic cells through `text_renderer`.

It must know nothing about SSD1306 pages.

### `kernel.c`

Owns orchestration and UART command dispatch only.

## 4. Geometry

Hardware-visible display:

```text
x = 0..127
y = 0..31
```

Framed graphics interior:

```text
x = 1..126
y = 1..30
```

Page-aligned text layout is derived, not hardcoded as a table:

```text
text_x = 1
text_y = align_up(1, 8) = 8
columns = 126 / 6 = 21
rows = 3
```

Cell origins:

```text
row 0 -> y=8
row 1 -> y=16
row 2 -> y=24
```

The last glyph occupies `y=24..30`; the bottom frame is `y=31`.

## 5. Rendering rule

Cells are opaque.

Rendering a character must overwrite:

- all glyph pixels;
- the blank spacer column;
- the blank spacer row when that row is inside the clip rectangle.

This prevents ghost pixels after character replacement.

For the bottom row, the clip rectangle excludes `y=31`, so the frame must
survive the cell update.

## 6. Scrolling

Do not use SSD1306 hardware scrolling for the kernel console.

Use retained semantic rows with a circular first-row index:

```c
first_row = (uint8_t)((first_row + 1u) % 3u);
```

Clear the reused logical last row and mark affected rows dirty.

Do not `memmove` the framebuffer as the primary scroll mechanism.

## 7. Presentation

`console_write()` and `console_putc()` must not flush the display.

Flow:

```text
update semantic state
        ->
render dirty semantic rows into mono_fb
        ->
explicit present
```

Initial implementation may keep the hardware-proven full-frame 512-byte flush.
Dirty-page transfer optimization is a later slice behind the same API.

## 8. Memory model

Approximate persistent display state:

- framebuffer: `512 bytes`
- console cells: `63 bytes`
- console metadata: a few bytes
- framebuffer metadata: a few bytes

Expected persistent cost is roughly `600 bytes`, not the old ~1.2 KiB
128x64 design.

## 9. Design constraints

- no heap;
- no function-pointer pseudo-OO;
- no hidden global Y remap;
- no 2x rendering workaround;
- no hardcoded 128x64 assumptions;
- no hardware scroll in the baseline console;
- no flush inside character-write functions;
- no critical fault reporting dependency on OLED;
- physical acceptance before every rendering-related commit.

## 10. Rejected historical assumption

The previous 128x64 console plan (`21 x 7`, 1024-byte framebuffer,
`y=3..63`) is invalid for the installed hardware and must not be reused.

The native 128x32 profile is now the architectural baseline.
