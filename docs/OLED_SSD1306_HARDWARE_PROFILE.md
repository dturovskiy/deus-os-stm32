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

## 5. Text geometry

Current font metrics:

- glyph: `5 x 7`
- cell advance: `6 x 8`
- normal production rendering: 1 logical pixel per display pixel

For a framed console:

- safe interior: `x=1..126`, `y=1..30`
- first page-aligned text row:
  - `align_up(1, 8) = 8`
- cell Y origins:
  - `8`
  - `16`
  - `24`
- glyph pixels:
  - row 0: `y=8..14`
  - row 1: `y=16..22`
  - row 2: `y=24..30`
- bottom frame remains at `y=31`
- columns:
  - `126 / 6 = 21`
- framed console capacity:
  - `21 x 3`

The third text row shares page 3 with the bottom border. Opaque cell rendering
must clip the spacer row at `y=31` so the border bit is preserved.

A borderless full-screen text mode could use four 8-pixel rows, but that is not
the current framed-console contract.

## 6. Reset and presentation behavior

MCU reset does not guarantee an OLED power-cycle.

After flash/reset, explicitly execute the intended OLED command before judging
the panel. For the current production regression anchor:

```text
oledtext
```

Expected UART response:

```text
OLED_TEXT_OK
```

Then inspect the physical display.

## 7. Accepted production evidence

Accepted native 128x32 production candidate:

- image size: `3412 bytes`
- image SHA-256:
  `0758C49D3EA10447C684987D74481C8B6D7F7002F7CF79B6E804D18A9399F4F0`
- framebuffer size: `512 bytes`
- example build framebuffer symbol address: `0x2000002C`
- example build `fault_record` address: `0x2000022C`

Symbol addresses are build-dependent and must always be resolved dynamically.
Do not hardcode either address in tooling.

Physical result accepted:

- clean full 128x32 frame;
- centered `DEUS OS`;
- 1-pixel-thick font;
- no alternating missing rows.

## 8. Replacement panel rule

A replacement OLED must be identified and calibrated independently. Do not
assume that another visually similar module has the same native height,
multiplex ratio, COM pin configuration, orientation, or address.
