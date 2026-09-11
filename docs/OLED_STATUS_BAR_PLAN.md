# OLED Status Bar Plan

Status: PLANNED / OWNED BY CONFIGURABLE UI LAYOUT

The status bar remains a semantic component, but its screen placement is now
owned by `docs/OLED_UI_LAYOUT_PLAN.md`.

## 1. Semantic responsibility

The status component owns:

- COMM state;
- time state;
- 3x4 digit/colon rendering;
- COMM icon rendering;
- dirty semantic state.

It does not own global coordinates.

## 2. Information model

Left field:

```text
COMM
```

Initial real state:

- none;
- UART/serial available.

Do not display a fake network/Wi-Fi state.

Right field:

```text
HH:MM
```

Initial source after layout acceptance:

- uptime.

Future source:

- RTC wall clock using the same geometry.

## 3. Micro-font

Required immutable font:

```text
glyph size: 3x4
required glyphs:
0 1 2 3 4 5 6 7 8 9 :
```

The COMM mark is an icon, not a text glyph.

## 4. Rendering contract

The status renderer receives a clip from the UI layout layer.

It may modify pixels only inside that clip.

It must not:

- draw outer borders;
- draw the global separator;
- move the console;
- perform I2C;
- flush/present the OLED;
- know SSD1306 page layout.

## 5. Current prototype

The uncommitted Slice 4B.1 prototype proved:

- 3x4 rendering;
- `12:34`;
- real COMM/UART icon;
- pixel isolation;
- `OLED_STATUS_OK`.

The protocol proof is useful.

The visual composition is rejected as a final layout because the full outer
frame is considered too heavy.

Do not commit that prototype as the final UI.

## 6. Next acceptance

The next status-bar acceptance occurs through the configurable layout engine.

At least these visual styles must be compared physically:

```text
minimal
boxed
```

`compact` should also be included if it differs meaningfully.

Only after a preset is physically accepted should uptime integration begin.

## 7. Refresh policy

Status state becomes dirty only when its displayed value changes.

For `HH:MM` uptime:

- minute change -> dirty;
- COMM state change -> dirty.

No 1 Hz OLED flush is required.

Presentation remains caller-controlled.

## 8. External customization

Pixel mockups may be created on PC at exact `128x32` resolution.

The board receives validated layout parameters and preconverted assets, not
PNG/SVG editor files directly.

See:

```text
docs/OLED_UI_LAYOUT_PLAN.md
```
