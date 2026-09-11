# OLED Console Acceptance Plan

Status: **PLANNED**

This document defines the evidence required before OLED console code is accepted.

## 1. Global regression invariants

Every OLED-console slice must preserve:

```text
I2C address = 0x3C
D3          = 0x00
start line  = 0x40
orientation = A1/C8
frame       = x=0..127, y=3..63
```

After every flash/reset:

```text
BOOT OK
```

must be observed before OLED testing.

When using the accepted baseline path:

```text
oledtext -> OLED_TEXT_OK
```

remains the primary display regression anchor.

## 2. Build acceptance

Required:

- `-Wall -Wextra -Werror`;
- no warning suppression added to hide new defects;
- `git diff --check`;
- initial MSP remains valid;
- binary/ELF produced successfully;
- `oled_framebuffer` remains exactly 1024 bytes;
- `fault_record` is resolved dynamically from ELF and compared with boot output.

No tool may hard-code the historical `fault_record` address after BSS layout changes.

## 3. Architecture acceptance

Before commit, inspect the source boundaries.

Required:

- `oled_console` contains no I2C register access;
- `oled_console` contains no SSD1306 page/window commands;
- `text_renderer` contains no UART command handling;
- `mono_fb` contains no panel init bytes;
- `ssd1306` contains no font table;
- normal `putc/write` paths do not call display present;
- no heap allocation;
- no function-pointer abstraction introduced without a second real backend.

## 4. Text-renderer acceptance

Opaque overwrite test:

1. draw a glyph with many set pixels;
2. render a different sparse glyph into the same cell;
3. present;
4. confirm no pixels from the old glyph remain.

Clip test:

1. render a cell adjacent to the bottom frame;
2. confirm `y=63` frame remains intact.

Alignment test:

1. render page-aligned text;
2. confirm glyphs are regular and readable;
3. compare with generic-path rendering where safe.

## 5. Console no-scroll acceptance

Temporary command:

```text
oledconsole
```

Required semantic content:

```text
7 rows
21-column capacity
wrap/newline behavior exercised
```

Required physical result:

- all four frame sides visible;
- seven text rows readable;
- no vertical corruption;
- no text at `y=0..7`;
- last glyph row occupies at most `y=56..62`;
- bottom frame at `y=63` remains visible;
- left/right frame remains visible;
- no stale pixels from prior screen.

Protocol result:

```text
OLED_CONSOLE_OK
```

is necessary but not sufficient; physical acceptance is mandatory.

## 6. Scroll acceptance

Write at least nine distinguishable logical lines into a seven-row console.

Required final screen:

- exactly seven logical rows shown;
- first two original rows no longer visible;
- row order preserved;
- newest line visible at bottom;
- cursor state remains valid;
- frame unchanged.

The model must use circular text rows rather than destructive framebuffer scrolling as the source of truth.

## 7. Dirty-page acceptance

Before optimization, record full-present visual output.

After dirty-page implementation:

- clear;
- render known console;
- present;
- modify one row;
- present again.

Required:

- visible result matches full-present path;
- only intended pages are marked dirty before transfer;
- dirty state is cleared only after successful transfer;
- failure leaves enough state for retry;
- full-present fallback still works.

## 8. Fault/regression safety

Fault diagnostics must remain operational.

OLED failure must not prevent UART diagnostics.

No fault handler may depend on successful I2C display transfer for correctness.

## 9. Git acceptance

For each slice:

- only intended files are modified/staged;
- hardware test evidence exists;
- working source matches flashed image at acceptance;
- commit happens only after physical PASS;
- push happens only after the accepted commit is reviewed.

## 10. Rejection examples

Reject the slice if:

- UART says success but OLED text is malformed;
- text rendering is visually correct only after unexplained reset sequences;
- frame is partially erased;
- console code contains magic SSD1306 page constants;
- row coordinates are manually enumerated;
- each character causes its own I2C flush;
- scroll is implemented by treating framebuffer pixels as the only text state;
- optimization changes public semantics;
- a workaround contradicts the hardware profile without new calibration evidence.
