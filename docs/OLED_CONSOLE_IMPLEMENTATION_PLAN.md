# OLED Console Implementation Plan

Status: **PLANNED / execution gate closed until documentation is accepted**

This plan converts the accepted framebuffer/text proof into a maintainable kernel console without introducing unnecessary framework complexity.

Every slice follows:

```text
source
-> build
-> static validation
-> flash
-> boot/UART regression
-> OLED command
-> physical visual acceptance
-> commit
```

No slice is committed before physical acceptance when the slice changes rendered output.

## Slice 0 — Restore accepted baseline

Purpose:

Remove the failed experimental `oledconsole` implementation from the working tree and MCU before architecture work begins.

Actions:

1. restore `src/kernel.c` to the accepted Git baseline;
2. build the accepted framebuffer/text firmware;
3. verify ELF symbols dynamically;
4. flash/verify/reset;
5. wait for `BOOT OK`;
6. run `oledtext`;
7. require `OLED_TEXT_OK`;
8. physically confirm accepted `DEUS OS` + calibrated frame.

Expected Git result:

- no source commit;
- working tree clean after restoration;
- HEAD remains the documentation head.

This slice exists because the current failed console experiment was flashed and must not be silently treated as a production baseline.

## Slice 1 — Extract SSD1306 driver without behavioral change

Target:

```text
src/drivers/ssd1306.c
include/drivers/ssd1306.h
```

Move only:

- I2C-facing SSD1306 command/data logic;
- initialization sequence;
- panel profile;
- full-window/present logic.

Do not change:

- init bytes;
- `D3=0x00`;
- `A1/C8`;
- address `0x3C`;
- frame geometry;
- text rendering behavior.

Acceptance:

- build with `-Werror`;
- UART regressions pass;
- `oledtext -> OLED_TEXT_OK`;
- physical output is identical to accepted baseline.

Commit only after identical visual output.

## Slice 2 — Extract monochrome framebuffer and font

Target:

```text
src/gfx/mono_fb.c
include/gfx/mono_fb.h
src/gfx/font5x7.c
include/gfx/font5x7.h
```

Move existing proven behavior first.

Add:

- clipping;
- dirty-page mask storage;
- dirty marking by drawing primitives.

Presentation still uses full 1024-byte flush.

Acceptance:

- existing `oledtext` visual output remains identical;
- frame remains `x=0..127, y=3..63`;
- framebuffer remains 1024 bytes;
- no buffer overrun;
- fault record address is resolved from ELF rather than assumed.

## Slice 3 — Introduce text renderer

Target:

```text
src/gfx/text_renderer.c
include/gfx/text_renderer.h
```

Implement one opaque 6x8 cell renderer.

Requirements:

- 5x7 glyph;
- transparent behavior is not the default;
- stale pixels are removed;
- clip rectangle is mandatory;
- generic arbitrary-Y path exists;
- page-aligned fast path exists;
- fast path preserves bits outside clip.

Focused hardware test:

- render known glyph sequence in a page-aligned row;
- overwrite it with different glyphs;
- verify no ghost pixels;
- verify bottom-frame preservation when testing row 7 region.

Do not introduce console state yet.

## Slice 4 — Introduce retained console, no scroll

Target:

```text
src/kernel/oled_console.c
include/kernel/oled_console.h
```

Implement:

- `21x7` cells;
- cursor;
- clear;
- putc;
- write;
- write_line;
- newline;
- automatic wrap;
- render.

Console viewport is computed from:

- hardware/graphics interior;
- page alignment;
- font metrics.

Do not hard-code a table of seven Y coordinates.

Temporary acceptance command:

```text
oledconsole
```

Expected display:

- calibrated frame unchanged;
- seven clean rows;
- first/last row readable;
- no overlap with frame;
- no malformed vertical layout;
- one explicit present after rendering.

## Slice 5 — Circular scroll

Implement scrolling only after Slice 4 is physically accepted.

Model:

- circular seven-row character buffer;
- O(1) `first_row` advance;
- clear reused logical last row;
- render from retained text state.

Acceptance command writes more than seven lines.

Expected:

- oldest line disappears;
- all remaining rows move semantically;
- new line appears at bottom;
- frame remains unchanged;
- no framebuffer memcpy is used as the authoritative scroll model.

## Slice 6 — Dirty-page presentation optimization

Only after console semantics are accepted.

Implement:

- `dirty_pages` consumption in SSD1306 present path;
- page-window transfer for changed pages;
- full-present fallback.

Acceptance compares full and dirty presentation results.

Required proof:

- same visible output;
- frame preserved;
- no stale glyph cells;
- full-present path remains available for diagnostics.

## Slice 7 — Kernel log integration

After the console itself is stable:

- add a small kernel log sink;
- UART remains one sink;
- OLED may become an optional second sink;
- output policy must not block fault-critical paths unexpectedly.

Do not tightly couple fault capture to I2C/OLED availability.

## Deferred work

Explicitly deferred until required:

- multiple font sizes;
- UTF-8;
- proportional fonts;
- multiple display backends;
- widgets;
- hardware scrolling;
- animation;
- double buffering;
- DMA I2C;
- LVGL-style object trees.

## Commit policy

Suggested commits are slice-oriented, for example:

```text
refactor: isolate SSD1306 display driver
refactor: isolate monochrome framebuffer and font
feat: add opaque monochrome text renderer
feat: add retained OLED console
feat: add OLED console scrolling
perf: add dirty-page SSD1306 presentation
```

Exact messages may change, but unrelated slices must not be combined into one large commit.

## Stop conditions

Stop the current slice and do not commit when any of the following occurs:

- frame geometry changes unexpectedly;
- `D3`, `A1/C8`, or address changes without explicit plan;
- text overlaps the frame;
- a layer accesses responsibilities belonging to another layer;
- build requires suppressing warnings;
- visual output differs from the slice acceptance target;
- source and flashed firmware become ambiguous.
