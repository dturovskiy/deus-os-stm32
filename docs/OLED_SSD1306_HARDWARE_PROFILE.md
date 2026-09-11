# OLED SSD1306 Hardware Profile

Status: CANONICAL / HARDWARE-VALIDATED

This document is the source of truth for the OLED module currently attached to
the STM32F103 system.

## 1. Proven module identity

The installed module is a native 128x32 monochrome OLED using an
SSD1306-compatible controller over I2C.

Hardware-proven properties:

- I2C address: `0x3C`
- logical resolution: `128 x 32`
- physical scan height: `32 COM rows`
- framebuffer: `512 bytes`
- pages: `4`
- page format:
  - `page = y >> 3`
  - `bit = y & 7`
  - `index = page * 128 + x`
  - `mask = 1 << bit`
- segment remap: `A1`
- COM scan direction: `C8`
- display offset: `D3 = 0x00`
- start line: `0x40`
- addressing mode: horizontal
- full transfer window:
  - columns `0..127`
  - pages `0..3`
- multiplex ratio:
  - command `A8`
  - value `0x1F` (1/32)
- COM pin hardware configuration:
  - command `DA`
  - value `0x02`
- current contrast: `0xCF`

Do not replace the 128x32 geometry with a 128x64 profile unless a replacement
panel is independently identified and hardware-validated.

## 2. Why the previous 128x64 profile was rejected

The module had previously been configured as 128x64:

- `A8 = 0x3F`
- `DA = 0x12`
- pages `0..7`
- framebuffer `1024 bytes`

Raw framebuffer calibration showed that under that profile:

- alternating logical Y rows disappeared;
- `BIT 0,2,4,6` were effectively invisible;
- `BIT 1,3,5,7` were visible;
- X mapping remained sequential and correct;
- pages remained sequential.

That behavior explained why the old 2x vertical font appeared acceptable:
each source pixel was duplicated into two adjacent logical Y rows, so one of
the pair remained visible. The 2x rendering was masking the wrong panel
geometry rather than proving a 128x64 display.

A controlled 128x32 probe changed only the geometry-related controller state:

- height `64 -> 32`
- framebuffer `1024 -> 512`
- `A8: 0x3F -> 0x1F`
- `DA: 0x12 -> 0x02`
- page window `0..7 -> 0..3`

With this profile all eight bit positions and sequential physical rows became
visible and the native 1-pixel font rendered correctly.

The 128x64 profile and the old module-specific `y=3..63` viewport are
therefore obsolete and must not be resurrected.

## 3. Current visible geometry

The native 128x32 profile exposes the full logical display cleanly:

- visible display: `x=0..127`, `y=0..31`
- full-frame border: `x=0..127`, `y=0..31`
- safe graphics interior: `x=1..126`, `y=1..30`

No hidden global Y remapping is permitted.

## 4. Native framebuffer

Canonical storage requirement:

```c
#define SSD1306_WIDTH  128u
#define SSD1306_HEIGHT 32u
#define SSD1306_PAGES  (SSD1306_HEIGHT / 8u)
#define SSD1306_FRAMEBUFFER_BYTES (SSD1306_WIDTH * SSD1306_PAGES)
```

Therefore:

```text
128 * 32 / 8 = 512 bytes
```

The driver must transfer exactly four pages for a full-frame present.

## 5. Accepted framed UI geometry

Current font metrics:

- glyph: `5 x 7`
- cell advance: `6 x 8`
- normal production rendering: 1 logical pixel per display pixel

The accepted UI layout uses every vertical region intentionally:

```text
y=0      top frame
y=1..4   reserved status-bar content
y=5      status-bar separator
y=6      one-pixel gap
y=7..13  console row 0 glyph
y=14     one-pixel inter-row gap
y=15..21 console row 1 glyph
y=22     one-pixel inter-row gap
y=23..29 console row 2 glyph
y=30     one-pixel bottom gap
y=31     bottom frame
```

Horizontal geometry:

- frame: `x=0..127`
- console/status usable width: `x=1..126`
- text advance: `6`
- console capacity: `21 x 3`

Canonical console viewport:

```text
x=1, y=7, width=126, height=23
```

The console module receives this viewport from its caller and derives row
origins from the viewport and font metrics. It does not know SSD1306 page
geometry.

Status-bar content is reserved but not implemented yet. The accepted separator
is at `y=5`.

## 6. Reset and presentation behavior

MCU reset does not guarantee an OLED power-cycle.

After flash/reset, explicitly execute the intended OLED command before judging
the panel.

Regression anchors:

```text
oledtext    -> OLED_TEXT_OK
oledrender  -> OLED_RENDER_EQ_OK + OLED_RENDER_OK
oledconsole -> OLED_CONSOLE_OK
```

Then inspect the physical display.

## 7. Accepted production evidence

Native 128x32 baseline:

- image size: `3412 bytes`
- image SHA-256:
  `0758C49D3EA10447C684987D74481C8B6D7F7002F7CF79B6E804D18A9399F4F0`
- framebuffer size: `512 bytes`

Accepted retained-console Slice 4 image:

- image size: `5680 bytes`
- image SHA-256:
  `C67AEBA137F645F5BECAEB6410382D44257E1B09EA3BE459BD83AC718F3A09AC`
- framebuffer: `512 bytes`
- retained console state: `67 bytes`
- console capacity: `21 x 3`
- separator: `y=5`
- console row glyph origins: `y=7,15,23`

Symbol addresses are build-dependent and must always be resolved dynamically.
Do not hardcode framebuffer, console-state, or `fault_record` addresses.

## 8. Replacement panel rule

A replacement OLED must be identified and calibrated independently. Do not
assume that another visually similar module has the same native height,
multiplex ratio, COM pin configuration, orientation, or address.
