# OLED Console Implementation Plan

Status: ACTIVE PLAN / REBASED ON NATIVE 128x32 HARDWARE

## Completed prerequisites

- SSD1306 I2C address `0x3C` proven.
- SSD1306 driver extracted.
- monochrome framebuffer extracted.
- font5x7 extracted.
- native panel geometry proven as `128x32`.
- native framebuffer proven as `512 bytes`.
- controller profile proven:
  - `A8=0x1F`
  - `DA=0x02`
  - `D3=0`
  - start line `0`
  - `A1/C8`
  - pages `0..3`
- clean 1x `DEUS OS` and full frame physically accepted.

The old 128x64 renderer experiment is rejected.

## Slice 3A — commit corrected native hardware baseline

Scope:

- keep native `128x32`;
- keep 512-byte framebuffer;
- keep 1x 5x7 demo;
- update canonical documentation;
- remove every temporary calibration command.

Acceptance:

- build with `-Wall -Wextra -Werror`;
- framebuffer symbol exactly 512 bytes;
- BOOT OK;
- `ping -> PONG`;
- I2C scan reports only `0x3C`;
- `oledping -> OLED_CMD_OK`;
- `oledtest -> OLED_TEST_OK`;
- `oledtext -> OLED_TEXT_OK`;
- physical thin `DEUS OS`;
- full clean border;
- no alternating missing rows.

Commit only after physical PASS.

## Slice 3B — text renderer, generic path first

Create:

```text
src/gfx/text_renderer.c
include/gfx/text_renderer.h
```

Implement only the correct generic opaque cell renderer first.

Requirements:

- cell size 6x8;
- clip rectangle;
- writes both foreground and background pixels;
- no page-aligned optimization yet;
- no retained console state.

Hardware test:

- render known strings at y=8,16,24;
- overwrite a cell with a different glyph and verify no ghost pixels;
- verify bottom frame y=31 survives the row at y=24.

Do not add a fast path in the same acceptance step.

## Slice 3C — page-aligned fast path

Only after the generic renderer is physically accepted:

- add byte-oriented fast path for aligned 6x8 cells;
- compare generated framebuffer bytes against generic rendering;
- verify identical physical output.

The fast path is an optimization, not an architectural dependency.

## Slice 4 — retained 21x3 console without scrolling

Add:

```text
src/kernel/oled_console.c
include/kernel/oled_console.h
```

State:

```c
char cells[3][21];
```

Implement:

- init;
- clear;
- putc;
- write;
- write_line;
- cursor;
- newline;
- wrap;
- render.

Temporary command:

```text
oledconsole
```

Physical acceptance:

- exactly three clean rows;
- 21-column geometry;
- no text on the frame;
- bottom border remains intact.

## Slice 5 — circular scroll

Add logical row rotation:

```c
first_row = (uint8_t)((first_row + 1u) % 3u);
```

Test by writing more than three lines.

Expected screen after five sequential numbered lines:

```text
3
4
5
```

according to the chosen line content and wrapping rules.

No hardware scrolling.

## Slice 6 — dirty-page present optimization

Framebuffer page count is four.

Dirty mask uses bits 0..3.

Requirements:

- full-frame fallback remains available;
- failed I2C transfer must not falsely clear dirty state;
- partial update must preserve controller addressing assumptions;
- compare every optimized result with the known-good full 512-byte flush.

## Slice 7 — kernel log integration

Add an optional OLED sink after console semantics are stable.

UART remains the primary diagnostic path.

Fault handlers must never depend on OLED availability.

## Deferred

- UTF-8;
- proportional fonts;
- widgets;
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
- native 128x32 frame geometry.

Do not suppress warnings to make a slice pass.
