# OLED Console Acceptance Plan

Status: CANONICAL / NATIVE 128x32

## 1. Global hardware invariants

Every OLED-related production slice must preserve:

```text
address      = 0x3C
resolution   = 128x32
framebuffer  = 512 bytes
pages        = 4
A8           = 0x1F
DA           = 0x02
D3           = 0x00
start line   = 0
segment map  = A1
COM scan     = C8
window       = columns 0..127, pages 0..3
```

The previous 128x64 configuration is explicitly rejected.

## 2. Build gate

Required:

- `-Wall -Wextra -Werror`;
- `git diff --check`;
- valid initial MSP;
- exactly one `oled_framebuffer` symbol;
- `oled_framebuffer` size exactly `512`;
- resolve `fault_record` dynamically from ELF;
- no hardcoded RAM addresses in acceptance tooling.

## 3. Boot/UART gate

After every flash:

- `BOOT OK`;
- boot `FAULTREC` address matches current ELF;
- `ping -> PONG`.

## 4. I2C/OLED gate

Required:

- I2C scan sees `ADDR=0x0000003C`;
- count remains one for the current bench setup;
- `oledping -> OLED_CMD_OK`.

After reset/flash, explicitly issue the intended OLED rendering command before
judging physical output.

## 5. Native baseline regression

`oledtext -> OLED_TEXT_OK`.

Physical result:

- full frame `x=0..127`, `y=0..31`;
- clean 1-pixel text;
- no alternating missing rows.

## 6. Generic renderer regression

`oledrender` must preserve:

- opaque 6x8 cells;
- arbitrary-Y generic path;
- clean overwrite without ghost pixels;
- clipping;
- complete frame.

## 7. Fast-path regression

Required:

```text
OLED_RENDER_EQ_OK
OLED_RENDER_OK
```

The aligned fast path must remain byte-for-byte equivalent to generic output
for the built-in target self-test.

## 8. Slice 4 retained-console acceptance

Capacity:

```text
21 columns x 3 rows
```

Retained state:

```text
67 bytes
```

Accepted screen composition:

```text
frame y=0
status content y=1..4
separator y=5
blank gap y=6
row 0 glyph y=7..13
inter-row gap y=14
row 1 glyph y=15..21
inter-row gap y=22
row 2 glyph y=23..29
bottom gap y=30
frame y=31
```

Canonical console viewport:

```text
x=1, y=7, width=126, height=23
```

Must verify:

- `oledconsole -> OLED_CONSOLE_OK`;
- exactly three readable rows;
- 21-column first-row capacity;
- wrap behavior;
- CR/LF behavior;
- overflow past row 3 does not appear;
- no frame corruption;
- separator intact;
- one blank row between separator and first console row;
- one blank row between last console row and bottom frame.

UART success is necessary but not sufficient. Physical inspection is mandatory.

## 9. Scroll acceptance

Write at least five logical lines into the 3-row console.

Verify:

- newest three logical lines remain;
- row order is correct;
- no framebuffer ghosting;
- no hardware scroll mode;
- status region remains untouched;
- separator and outer frame remain intact.

## 10. Status-bar acceptance

Detailed acceptance plan:

```text
docs/OLED_STATUS_BAR_PLAN.md
```

The status bar is a separate UI component.

Reserved region:

```text
content   x=1..126, y=1..4
separator x=1..126, y=5
gap       y=6
```

### 10.1 Slice 4B.1 static proof

Required:

- `font3x4` supports digits `0..9` and `:`;
- COMM icon renders inside a 4x4 left slot;
- fixed `12:34` renders in the right time field;
- `oledstatus -> OLED_STATUS_OK`;
- no pixel outside `x=1..126,y=1..4` changes;
- frame `y=0` survives;
- separator `y=5` survives;
- gap `y=6` remains blank;
- console pixels remain unchanged;
- physical output is readable.

### 10.2 Slice 4B.2 integration

Required:

- right field displays uptime as `HH:MM`;
- display range saturates at `99:59`;
- UART/serial COMM state is real, not a fabricated network state;
- status becomes dirty only when displayed minute or COMM state changes;
- no 1 Hz OLED-flush requirement;
- `oledconsole -> OLED_CONSOLE_OK`;
- `OLED_RENDER_EQ_OK` remains intact;
- accepted console geometry remains unchanged.

The status component must not:

- perform I2C;
- present/flush the display;
- know SSD1306 page layout;
- modify console semantic state;
- draw separator `y=5`;
- write to `y=0`, `y=5`, or `y=6`.

## 11. Dirty-page acceptance

Dirty-page mask uses bits `0..3` only.

Required:

- correct page marking;
- no false clearing after failed transfer;
- successful partial present matches full-frame output;
- full-frame fallback remains functional.

## 12. Safety regression

UART must remain responsive after OLED operations.
Fault diagnostics must remain available even if OLED/I2C fails.

## 13. Git gate

For every hardware-visible change:

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART
 -> physical visual PASS
 -> commit
 -> push
```

Never commit an OLED rendering/layout change before physical acceptance.
