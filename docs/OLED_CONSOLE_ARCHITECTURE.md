# OLED Console Architecture

Status: **PLANNED / execution not started**

This document defines the production architecture for the OLED text console on the STM32F103 project.

The architecture is intentionally small and deterministic. It borrows the useful separation found in mature embedded graphics stacks without importing a general-purpose GUI framework or object-oriented abstraction layer.

The accepted OLED hardware profile remains authoritative:

`docs/OLED_SSD1306_HARDWARE_PROFILE.md`

## 1. Design goals

The OLED console must be:

- deterministic;
- heap-free;
- small enough for STM32F103C8 SRAM and flash;
- fast for the common text path;
- capable of coexisting with arbitrary monochrome graphics;
- independent of SSD1306 page mechanics at the console API level;
- hardware-quirk aware without leaking panel quirks into generic text code;
- testable one layer at a time;
- suitable as a future kernel log sink.

The architecture must preserve the already accepted SSD1306 configuration:

```text
I2C address = 0x3C
resolution  = 128x64
D3          = 0x00
start line  = 0x40
orientation = A1/C8
frame       = x=0..127, y=3..63
```

## 2. Non-goals

The first production console implementation will not include:

- heap allocation;
- a general widget toolkit;
- LVGL-style scene/object trees;
- virtual interfaces or function-pointer polymorphism;
- multiple display backends;
- hardware scrolling;
- proportional fonts;
- UTF-8 text shaping;
- partial-region composition across arbitrary layers.

Those can be introduced later only when a real requirement exists.

## 3. Chosen architecture

The selected design is a hybrid of retained text state and immediate framebuffer graphics.

```text
kernel / commands / future klog
              |
              v
        oled_console
   retained 21x7 character state
   cursor / wrap / newline / scroll
              |
              v
        text_renderer
   font metrics / opaque cell drawing
   generic blit + aligned fast path
              |
              v
           mono_fb
   1024-byte 1-bpp framebuffer
   primitives / clipping / dirty pages
              |
              v
          ssd1306
   panel profile / init / present
              |
              v
             I2C
```

The console owns text semantics.

The framebuffer owns pixels.

The SSD1306 driver owns controller and transfer mechanics.

No upper layer may depend on I2C registers or SSD1306 GDDRAM page commands.

## 4. Module responsibilities

### `drivers/ssd1306`

Responsibilities:

- controller initialization;
- accepted panel profile;
- SSD1306 command/data transfer;
- full-window setup;
- framebuffer presentation;
- later dirty-page presentation optimization.

It must not contain:

- font tables;
- cursor state;
- text wrapping;
- console row buffers;
- application strings.

### `gfx/mono_fb`

Responsibilities:

- own or reference the 1024-byte monochrome framebuffer;
- pixel access;
- clipping;
- horizontal/vertical lines;
- rectangles;
- bitmap/glyph blitting support;
- dirty-page tracking.

It must not know:

- UART commands;
- console semantics;
- SSD1306 initialization sequences;
- kernel logging policy.

### `gfx/font5x7`

Responsibilities:

- immutable glyph data;
- glyph lookup;
- font metrics.

Initial metrics:

```text
glyph width     = 5
glyph height    = 7
horizontal step = 6
vertical step   = 8
```

### `gfx/text_renderer`

Responsibilities:

- render one opaque text cell into a monochrome surface;
- respect clipping;
- preserve pixels outside the requested clip rectangle;
- use a generic path for arbitrary Y alignment;
- use a fast page-aligned path when conditions allow.

It must not:

- advance the console cursor;
- perform I2C;
- flush the OLED;
- know the meaning of newline or scroll.

### `kernel/oled_console`

Responsibilities:

- retained character-cell state;
- cursor column/row;
- newline;
- carriage return policy;
- automatic wrapping;
- scrolling;
- clearing;
- rendering the current character state through `text_renderer`.

It must not:

- manipulate SSD1306 page numbers;
- access I2C registers;
- send display commands;
- contain controller offsets or orientation commands.

### `kernel.c`

`kernel.c` remains orchestration only.

It may:

- initialize modules;
- expose temporary acceptance commands;
- route future kernel log output;
- request render/present.

It must not remain the permanent home for fonts, SSD1306 transfers, framebuffer algorithms, or console internals.

## 5. Memory model

The accepted full framebuffer remains:

```text
128 * 64 / 8 = 1024 bytes
```

The console keeps a small semantic text model:

```c
typedef struct
{
    char cells[7][21];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;
```

Character storage:

```text
7 * 21 = 147 bytes
```

Expected persistent RAM for framebuffer + text model is therefore roughly 1.2 KiB plus small metadata.

This is preferred over saving approximately 147 bytes at the cost of destructive framebuffer-only scrolling and loss of semantic text state.

## 6. Console geometry is computed, not hard-coded per row

The accepted graphics interior is:

```text
x = 1..126
y = 4..62
```

The console chooses the first page-aligned text baseline dynamically:

```text
console_y0 = align_up(graphics_interior_y0, 8)
           = align_up(4, 8)
           = 8
```

Character placement is derived from font metrics:

```text
x = console_x0 + column * horizontal_step
y = console_y0 + row    * vertical_step
```

For the initial 5x7 font:

```text
horizontal_step = 6
vertical_step   = 8
```

The resulting capacity is 21 columns and 7 rows.

The numbers `8,16,24,...` are therefore layout results, not a manually maintained list of row coordinates.

## 7. Graphics viewport and console viewport are different concepts

The panel hardware profile defines the physically accepted visible area.

The graphics layer may use any pixel inside the accepted graphics interior:

```text
x = 1..126
y = 4..62
```

The console deliberately chooses a page-aligned subset for efficient and reliable text rendering:

```text
first glyph row starts at y=8
last glyph row starts at y=56
last glyph occupies y=56..62
```

The lower frame remains at `y=63`.

This distinction is mandatory:

- panel profile = hardware facts;
- graphics viewport = legal pixel area;
- console layout = UI policy computed from font metrics.

## 8. Opaque text cells

Text rendering must be opaque at the cell level.

Writing a new character over an existing character must remove pixels belonging to the old character.

The renderer must not implement normal text output as:

```c
framebuffer_byte |= glyph_byte;
```

The cell background must be deterministically cleared inside the cell clip before or while applying the new glyph.

The initial cell concept is:

```text
5 pixels glyph width
1 pixel horizontal spacing
7 pixels glyph height
1 pixel vertical spacing
```

Clipping must ensure that the last row does not erase the bottom frame at `y=63`.

## 9. Generic glyph path and aligned fast path

The public text renderer API remains pixel-coordinate based.

Internally it may choose between two implementations.

### Generic path

Used when the glyph Y coordinate is not SSD1306-page aligned or when clipping requires arbitrary bit placement.

The glyph may span two framebuffer bytes/pages.

### Page-aligned fast path

Used when:

```text
y % 8 == 0
```

and the glyph fits the selected clip.

The 5x7 column-oriented glyph representation can then be merged directly into framebuffer bytes without invoking `set_pixel()` for every glyph pixel.

This optimization is private to the renderer.

The console must not know which path was selected.

## 10. Bottom-frame preservation

The seventh console row begins at `y=56`.

Its glyph pixels occupy:

```text
y=56..62
```

The bottom frame occupies:

```text
y=63
```

Both are in SSD1306 page 7.

Therefore the aligned renderer must use a mask and preserve bit 7 of framebuffer bytes belonging to the frame.

Conceptually:

```text
glyph area mask = 0x7F
frame bit       = 0x80
```

The implementation must preserve pixels outside the renderer clip rather than overwriting complete bytes blindly.

## 11. Dirty-page tracking

`mono_fb` will carry an 8-bit page-dirty mask:

```text
bit 0 -> framebuffer page 0
...
bit 7 -> framebuffer page 7
```

All framebuffer mutation primitives mark affected pages dirty.

Initial production acceptance may continue using the already proven 1024-byte full-frame transfer.

Dirty-page I2C transfer is a later optimization behind the same `present()` boundary.

Upper layers must not change when presentation is optimized.

## 12. Presentation rule

`putc()` and `write()` never perform I2C transfer.

The required flow is:

```text
console state mutation
-> console render
-> framebuffer updated
-> one explicit display present
```

A multi-character message therefore does not cause one I2C transaction per character.

## 13. Scroll model

Scroll belongs to the retained console model.

The seven logical rows form a circular buffer.

When a newline advances beyond the last visible row:

1. advance `first_row`;
2. clear the newly exposed logical last row;
3. keep the cursor on that row;
4. mark affected text rows dirty;
5. render/present when requested.

The model scroll operation is O(1).

Framebuffer contents are regenerated from semantic text state rather than memcpy-scrolled as the source of truth.

## 14. SOLID interpretation for this C firmware

SOLID is applied to responsibilities, not to class hierarchies.

### Single Responsibility

Each module has one reason to change:

- SSD1306 controller mechanics;
- monochrome pixels;
- font data;
- text rasterization;
- terminal semantics.

### Open/Closed

New fonts, panel profiles, renderer optimizations, and dirty-page presentation can be added without rewriting console semantics.

### Liskov Substitution

No inheritance hierarchy is introduced, so this principle does not drive the current design.

### Interface Segregation

Modules expose small purpose-specific C APIs instead of one global display object with unrelated operations.

### Dependency Inversion

High-level console logic depends on text/surface concepts, not STM32 I2C registers.

Function-pointer interfaces are intentionally deferred until a second real backend requires them.

## 15. Source tree target

The planned source layout is:

```text
src/
    kernel.c

    drivers/
        ssd1306.c

    gfx/
        mono_fb.c
        font5x7.c
        text_renderer.c

    kernel/
        oled_console.c

include/
    drivers/
        ssd1306.h

    gfx/
        mono_fb.h
        font5x7.h
        text_renderer.h

    kernel/
        oled_console.h
```

Exact file extraction occurs incrementally and only after each hardware-safe slice is accepted.

## 16. Failure evidence that motivated this architecture

An experimental `oledconsole` implementation attempted to render seven scale-1 lines using generic per-pixel writes at arbitrary Y positions.

Observed result:

- frame remained visually correct;
- protocol/build/flash checks passed;
- text was visibly malformed/distorted;
- the experiment was not accepted and must not be committed.

The failure does not invalidate the accepted framebuffer or frame calibration.

It proves that console layout/rendering must be treated as a separate architecture concern and hardware-tested explicitly.

## 17. Production invariants

Until superseded by a hardware-validated architecture revision:

```text
no heap
full 1024-byte framebuffer retained
console text retained separately
console does not know SSD1306 pages
renderer owns page-aligned optimization
panel quirks stay in the panel profile
text cells are opaque
frame pixels outside text clip are preserved
putc/write never flush I2C
presentation is explicit
scroll is semantic/circular, not destructive pixel scrolling
no production commit before physical OLED acceptance
```
