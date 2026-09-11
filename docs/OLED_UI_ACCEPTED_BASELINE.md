# OLED UI accepted baseline

Status: **ACCEPTED / FROZEN**  
Hardware acceptance date: **2026-09-11**

This document is the authoritative visual and geometry baseline for the current
128x32 OLED UI. Earlier layout experiments and configurable-layout plans are retained
as historical/future planning only.

## Hardware profile

- OLED native geometry: 128x32.
- SSD1306-compatible controller.
- I2C address: `0x3C`.
- Framebuffer: 512 bytes.
- Native panel profile remains the accepted 128x32 profile.

## Accepted status bar

The status bar is a fixed 128x9 framed region.

- Top border: `y=0`, `x=0..127`.
- Side borders: `x=0` and `x=127`, `y=1..7`.
- Bottom border: `y=8`, `x=0..127`.
- Status content band: `y=2..6`.
- First filled indicator: `x=2..6`.
- Second filled indicator: `x=8..12`.
- Ring indicator: `x=14..18`.
- Clock glyph area ends at `x=125`.
- `x=126` is a mandatory one-pixel blank right inset.
- `x=127` is the right status-frame side.
- Current accepted proof value is `00:00`.
- Status digits/colon use the compact `3x5` font.

### Reserved notification field

The currently unused center of the frozen status bar is reserved for future
notifications without changing status-bar geometry:

```text
left indicators : x=2..18
left guard      : x=19
notification    : x=20..107, y=2..6
right guard     : x=108
clock           : x=109..125
right inset     : x=126
frame side      : x=127
```

`x=20..107` is a semantic reservation only. Slice 5 does not render notifications.

On the monochrome panel, notification severity may later be represented without
changing geometry:

- normal notice: white glyphs on black;
- emphasized/urgent notice: inverse field, white fill with black glyphs.

The exact notification policy, text/icon format, lifetime, queueing, and priority
rules are deferred to a dedicated notification subsystem slice.

The status bar bitmap is protected by an exact reference self-test:
`OLED_STATUS_REFERENCE_OK`.

## Gap below status bar

`y=9` is completely blank.

This one-pixel row is mandatory and visually separates the status frame from the
console. No console glyph or frame pixel may occupy this row.

## Accepted console viewport

Console clip:

```text
x=1
y=10
width=126
height=22
```

The retained semantic console remains **21 columns x 3 rows**.

The console uses the dedicated compact `5x6` glyph font with a `6x7` cell:

```text
row 0 glyph : y=10..15
gap         : y=16
row 1 glyph : y=17..22
gap         : y=23
row 2 glyph : y=24..29
gap         : y=30
bottom blank: y=31
```

Horizontal geometry remains 5 glyph pixels plus one spacer pixel, preserving all
21 columns.

There is no left, right, or bottom outer frame below the status bar.

## Renderer structure

- Existing `5x7` renderer behavior and aligned fast path remain available for
  diagnostics/regression.
- The accepted console uses `font5x6`.
- Custom glyph rendering goes through the generic opaque renderer path.
- The retained console state remains 67 bytes.
- The framebuffer remains 512 bytes.

A minor visual asymmetry in the compact `B` glyph is explicitly accepted as
non-blocking. Do not reopen UI work solely for that cosmetic detail.

## Accepted firmware fingerprint

The hardware-accepted binary is:

```text
Image size    : 7396 bytes
SHA-256       : 4DDA68DA96215F6FC2960007B37AB5F8808BA9284EC8A726AEFFFC4B97FCF9C4
.text         : 7396 bytes
.data         : 0 bytes
.bss          : 716 bytes
framebuffer   : 512 bytes
console state : 67 bytes
fault_record  : 0x20000270 in the accepted build
```

The commit acceptance script must reproduce the image size and SHA-256 before the
source is committed.

## Accepted protocol regression

The final hardware run passed:

```text
BOOT OK
PONG
ADDR=0x0000003C
COUNT=0x00000001
OLED_CMD_OK
OLED_TEXT_OK
OLED_RENDER_EQ_OK
OLED_RENDER_OK
OLED_UI_LAYOUT_OK
OLED_STATUS_REFERENCE_OK
OLED_STATUS_OK
OLED_CONSOLE_OK
```

The display was then physically accepted.

## Diagnostic screens versus product UI

`oledtext`, `oledrender`, `oledstatus`, and `oledconsole` are diagnostic UART
commands. Acceptance scripts intentionally call them in sequence, so temporary test
patterns may appear while a regression script is running.

Those intermediate diagnostic images are not the product UI and are not a visual
acceptance target. In normal product behavior, diagnostics must only appear when
explicitly requested.

The accepted product-facing visual sequence is the boot splash (`DEUS OS`) followed
by the frozen UI.

## Deferred UI work

The following work is intentionally deferred and is not part of the accepted UI
baseline:

- alternate presets (`minimal`, `boxed`, `compact`);
- runtime custom geometry editing;
- PC layout configurator/import;
- layout persistence;
- uptime-driven `HH:MM`;
- RTC-backed time.

If any of these are resumed later, they must begin from this accepted baseline and
must not silently change its geometry.

## Next active OLED console work

UI styling is closed. Continue with the console/system roadmap, beginning with the
next non-UI behavior slice (retained circular scrolling / `first_row` behavior).
## Slice 5 circular retained console scroll — ACCEPTED 2026-09-11

Circular retained scrolling is now hardware accepted.

Behavior:

- scrolling is implemented by rotating `first_row`;
- the old logical top physical row is cleared and reused as the new bottom row;
- framebuffer `memmove` is not used as the scrolling mechanism;
- SSD1306 hardware scrolling is not used;
- the retained console remains 21x3 and 67 bytes;
- the frozen UI geometry, status bar, fonts, and renderer geometry remain unchanged;
- the reserved notification field remains `x=20..107, y=2..6` and is still visually empty.

The physical Slice 5 proof displayed exactly:

```text
SCROLL TWO
SCROLL THREE
SCROLL FOUR
```

Protocol regression passed:

```text
OLED_SCROLL_STATE_OK
OLED_SCROLL_OK
OLED_UI_LAYOUT_OK
OLED_STATUS_REFERENCE_OK
OLED_STATUS_OK
OLED_CONSOLE_OK
OLED_RENDER_EQ_OK
OLED_RENDER_OK
```

Accepted Slice 5 firmware fingerprint:

```text
Image size    : 8188 bytes
SHA-256       : C9ACF7A77A76C3B70A2EE323A6AE6BF16837DFD40BC2F38F28A83F4DB3CD8463
.text         : 8188 bytes
.data         : 0 bytes
.bss          : 716 bytes
framebuffer   : 512 bytes
console state : 67 bytes
```

Next active OLED slice: dirty-page present optimization.
