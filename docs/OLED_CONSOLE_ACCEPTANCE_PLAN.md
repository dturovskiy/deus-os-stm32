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

## 10. Configurable UI/status acceptance

Detailed plans:

```text
docs/OLED_UI_LAYOUT_PLAN.md
docs/OLED_STATUS_BAR_PLAN.md
```

The first hardcoded 4B.1 experiment is protocol-valid but visually rejected and
must not be committed as the production layout.

### 10.1 Layout-engine acceptance

Required:

- built-in `minimal`, `boxed`, `compact` layouts validate;
- invalid layouts are rejected atomically;
- layout module has no I2C/SSD1306 page knowledge;
- semantic console remains 21x3 unless separately approved;
- status and console receive clips from the layout layer;
- frame/separator choices are layout data, not semantic-module constants.

### 10.2 Preset physical acceptance

At minimum compare:

```text
minimal
boxed
```

Verify physically:

- status readability;
- console readability;
- no accidental clipping;
- no stale pixels after preset switch;
- expected border behavior;
- existing rendering regressions remain green.

Protocol success alone is not sufficient.

### 10.3 Custom runtime acceptance

Required:

- `ui show`;
- `ui set ...`;
- valid custom one-pixel changes apply;
- invalid changes leave active layout unchanged;
- no heap;
- RAM-only configuration initially.

### 10.4 Status acceptance

Required:

- 3x4 digits/colon;
- real COMM/UART icon;
- `OLED_STATUS_ISOLATION_OK`;
- `OLED_STATUS_OK`;
- status writes only inside its layout-provided clip;
- status component does not draw global separator/borders;
- no I2C/present inside status component.

### 10.5 Transport acceptance

UI configuration commands must call a transport-independent target API.

UART is the current transport.

Future USB CDC must reuse the same command/config semantics rather than
creating a second UI control path.

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
