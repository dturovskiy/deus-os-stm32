# SSD1306 OLED Hardware Profile

Status: **ACCEPTED / hardware validated**

Accepted implementation commit:

`e7587d2ce2f636889cd03e6b1a13d08ce42021ea`

Accepted firmware image:

- size: `3100 bytes`
- SHA-256: `679B9A142E53C15C2EFD21F4714CE7747EB55525887765B8F8415306DC3A762F`

This document is the hardware-specific contract for the OLED currently used by this project. Future display, console, UI, scheduler-status, diagnostics, and application code must preserve this profile unless a replacement panel is explicitly recalibrated.

## 1. Physical module

Display:

- nominal type: SSD1306-compatible monochrome OLED
- nominal resolution: `128 x 64`
- interface: I2C
- hardware-proven I2C address: `0x3C`

Blue Pill wiring:

| OLED | Blue Pill silkscreen |
| --- | --- |
| GND | `G` |
| VCC | `3.3` |
| SCL | `B6` |
| SDA | `B7` |

Do not rewrite these physical wiring labels as MCU port names in wiring instructions. The board silkscreen names are the canonical physical labels.

## 2. Accepted controller initialization

The accepted panel orientation and addressing configuration is:

- multiplex ratio: `0xA8, 0x3F`
- display offset: `0xD3, 0x00`
- display start line: `0x40`
- charge pump: enabled
- memory addressing: horizontal
- segment remap: `0xA1`
- COM scan direction: `0xC8`
- COM pins configuration: `0xDA, 0x12`
- display follows GDDRAM: `0xA4`
- normal display: `0xA6`

`A1/C8` is intentional and hardware accepted. The display was physically oriented to match this configuration. Do not change it just to make a software experiment appear more conventional.

## 3. Framebuffer contract

The framebuffer is exactly:

`128 * 64 / 8 = 1024 bytes`

The logical memory model is standard SSD1306 page layout:

```text
page  = y >> 3
bit   = y & 7
index = page * 128 + x
mask  = 1 << bit
```

Equivalent pixel addressing:

```text
framebuffer[x + (y / 8) * 128] |= 1 << (y % 8)
```

Valid logical framebuffer coordinates remain:

- `x = 0..127`
- `y = 0..63`

The full transfer window is:

- columns `0..127`
- pages `0..7`

Do not introduce an alternative framebuffer packing format unless the entire display stack is intentionally redesigned.

## 4. Hardware-calibrated visible viewport

This specific OLED module has a proven vertical mapping quirk.

The accepted visible frame is:

```text
LEFT   = x 0
RIGHT  = x 127
TOP    = y 3
BOTTOM = y 63
```

The final accepted frame call is equivalent to:

```c
oled_fb_rect(
    0u,
    3u,
    SSD1306_WIDTH,
    SSD1306_HEIGHT - 3u
);
```

This produces all four physically visible one-pixel frame sides on the current module.

For future UI layout, treat the hardware-calibrated viewport as:

```text
visible logical viewport:
x = 0..127
y = 3..63
```

For content that must not overwrite the frame, use the interior:

```text
safe interior:
x = 1..126
y = 4..62
```

Do not place critical UI information in logical rows `y=0..2` on this module.

## 5. Important distinction: framebuffer coordinates vs physical panel rows

Do not assume that changing logical `y` by one always produces the visually expected one-row movement near the top or bottom edge of this module.

During hardware calibration, adjacent logical row changes near the boundaries could:

- disappear,
- reappear,
- clip,
- appear to wrap,
- or fail to move in the visually expected direction.

The framebuffer formula itself was validated and matches conventional SSD1306 libraries. The abnormal behavior is treated as a module/panel integration quirk rather than as a reason to rewrite generic framebuffer bit addressing.

Therefore:

1. Keep normal SSD1306 framebuffer math.
2. Keep `D3=0x00`.
3. Keep `A1/C8`.
4. Apply the calibrated viewport at the UI/frame-layout level.
5. Do not add hidden global Y remapping without a new hardware experiment proving that it is required.

## 6. Why `D3` must remain `0x00`

Display-offset experiments were performed with `D3`.

Tested values included:

- `0x04`
- `0x3C`
- `0x3D`

They changed clipping/wrapping behavior but did not yield a cleaner or more reliable global coordinate mapping than the accepted baseline.

The project therefore standardizes on:

```text
D3 = 0x00
start line = 0x40
```

Do not use `D3` as a compensation for the `TOP=3` module quirk.

If a future panel requires a different `D3`, that panel must receive a new hardware profile and its own acceptance evidence.

## 7. Proven text/demo state

The accepted framebuffer/text demonstration uses:

```text
DEUS OS
```

with the project 5x7 font rendered at scale 2.

The accepted demo placement is:

```text
x = 17
y = 25
```

The text renderer, framebuffer flush, and frame are hardware proven together.

## 8. Reset and OLED controller state

An STM32 reset does not necessarily reset or power-cycle the OLED controller.

This was observed directly during display-orientation and offset experiments: after reflashing/resetting the MCU, the OLED could retain controller state from an earlier test until a new OLED initialization/render command was issued.

Mandatory validation rule after a flash/reset:

```text
flash
-> verify
-> MCU reset
-> wait for BOOT OK
-> send oledtext
-> require OLED_TEXT_OK
-> only then judge the physical OLED image
```

Never judge a newly flashed OLED configuration before explicitly running `oledtext`.

If the image looks inconsistent with the source immediately after reset, first reinitialize the OLED with `oledtext` before changing code.

## 9. Known-good I2C/display facts

Hardware-proven facts:

- I2C bus works on the selected pins.
- device responds at `0x3C`.
- SSD1306 NOP command succeeds.
- controller initialization succeeds.
- full 128x64 checkerboard transfer succeeds.
- full 1024-byte framebuffer transfer succeeds.
- pixel primitives work.
- rectangle drawing works.
- 5x7 scaled text rendering works.
- `DEUS OS` framebuffer demo is physically accepted.
- calibrated frame `x=0..127, y=3..63` is physically accepted.

These facts should be treated as regression anchors.

## 10. Rules for future OLED code

Future OLED/UI work must follow these rules:

1. Use the existing 1024-byte framebuffer as the canonical render target.
2. Draw in logical framebuffer coordinates; do not directly stream ad-hoc pixel layouts unless performing an explicit diagnostic.
3. Use the calibrated visible viewport `x=0..127, y=3..63`.
4. Keep critical content inside `x=1..126, y=4..62`.
5. Keep `D3=0x00`.
6. Keep `A1/C8`.
7. Keep the full transfer window columns `0..127`, pages `0..7`.
8. Reinitialize with `oledtext` after every flash/reset before visual acceptance.
9. Do not resurrect temporary page-band, single-bit, D3-offset, or live-calibration diagnostics in production code.
10. If partial updates are introduced later, compare their output against the known-good full-frame flush before accepting them.
11. If scrolling is introduced later, remember that SSD1306 hardware scroll/start-line commands can change the physical interpretation of GDDRAM; reset those modes before returning to the normal console framebuffer.
12. Any future clipping/layout layer should clip against the calibrated viewport, not blindly against `y=0`.
13. Keep display hardware quirks out of generic font and primitive algorithms where possible.

## 11. Replacement-module procedure

The `TOP=3` calibration belongs to the current physical module. Do not assume another 0.96-inch OLED behaves identically.

When replacing the display:

1. run I2C scan and confirm the address;
2. validate NOP command;
3. validate the standard checkerboard;
4. validate `A1/C8` orientation;
5. keep baseline `D3=0x00` initially;
6. render a one-pixel full frame;
7. calibrate visible top/bottom rows if necessary;
8. validate text;
9. record the new profile before accepting the replacement.

A replacement module may legitimately use different boundary calibration or even a different compatible controller.

## 12. Datasheet vs module behavior

The SSD1306 datasheet describes controller GDDRAM, SEG, COM, scan, offset, and addressing behavior.

It does not guarantee that every low-cost module sold as "SSD1306-compatible" connects the controller outputs to the OLED glass in exactly the same physical arrangement as the reference configuration.

The current evidence therefore supports this engineering conclusion:

- standard SSD1306 framebuffer organization is retained;
- standard baseline initialization is retained;
- the current module has a hardware-specific visible-boundary quirk;
- the quirk is handled by the calibrated logical viewport rather than by distorting generic driver semantics.

This is a hardware profile, not a claim that all SSD1306 panels require `TOP=3`.

## 13. Accepted evidence summary

The calibration sequence established the following:

- `y=63` is physically associated with the visible bottom edge.
- middle framebuffer pages render and flush correctly.
- direct page writes proved page data reaches the panel.
- direct page0 bit experiments showed unusual top-edge visibility.
- changing `D3` caused clipping/wrapping changes rather than a reliable global fix.
- live UART calibration allowed the frame boundaries to be moved without reflashing.
- physical acceptance was reached at `TOP=3`, `BOTTOM=63`.
- final clean firmware removed the calibration/debug commands and retained the fixed accepted frame.

Accepted final UART validation:

```text
BOOT OK
ping      -> PONG
oledping  -> OLED_CMD_OK
oledtext  -> OLED_TEXT_OK
```

Accepted framebuffer placement:

```text
oled_framebuffer = 0x2000001C
size             = 1024 bytes
fault_record     = 0x2000041C
```

These addresses are evidence for the accepted build only. Tooling must continue to resolve symbols dynamically from the ELF/boot banner after future BSS changes.

## 14. Production invariant

Until this hardware profile is intentionally superseded:

```text
OLED address       = 0x3C
resolution         = 128x64
framebuffer        = 1024 bytes
orientation        = A1/C8
display offset D3  = 0x00
start line         = 0x40
visible X          = 0..127
visible Y          = 3..63
safe interior X    = 1..126
safe interior Y    = 4..62
post-flash visual  = only after oledtext / OLED_TEXT_OK
```

If future code violates one of these invariants, the change must explicitly explain why and must be hardware revalidated.