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

## Slice 4B — status-bar component

Detailed plan:

```text
docs/OLED_STATUS_BAR_PLAN.md
```

Fixed decisions:

- status content remains `x=1..126, y=1..4`;
- separator remains `y=5`;
- `y=6` remains blank;
- console remains `21x3` at row origins `y=7,15,23`;
- left field is COMM status, initially UART/serial, not fake network;
- right field is `HH:MM`;
- initial time source is uptime;
- future RTC uses the same `HH:MM` geometry;
- micro-font is `3x4`;
- status component does not flush the OLED.

Implementation is split into two hardware-visible checkpoints:

### Slice 4B.1 — static status proof

- add `font3x4`;
- add `oled_status_bar`;
- render COMM icon + fixed `12:34`;
- add `oledstatus -> OLED_STATUS_OK`;
- prove pixel isolation to `x=1..126,y=1..4`;
- physical acceptance before integration.

### Slice 4B.2 — uptime integration

- derive `HH:MM` from existing uptime;
- expose UART/serial COMM state;
- dirty only when displayed minute or COMM state changes;
- integrate with the accepted framed console;
- preserve every Slice 4 console regression.

Do not begin 4B.2 until 4B.1 is physically accepted.

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
