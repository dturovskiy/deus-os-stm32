# OLED Status Bar Plan

Status: PLANNED / SLICE 4B

This document defines the implementation and acceptance plan for the small
status region above the accepted 21x3 OLED console.

The accepted console geometry is frozen for this slice.

## 1. Fixed screen geometry

The current hardware-accepted layout is:

```text
y=0      top frame
y=1..4   status content
y=5      status separator
y=6      one-pixel gap
y=7..13  console row 0
y=14     inter-row gap
y=15..21 console row 1
y=22     inter-row gap
y=23..29 console row 2
y=30     bottom gap
y=31     bottom frame
```

Slice 4B must not change:

- outer frame;
- separator at `y=5`;
- clear row `y=6`;
- console viewport `x=1,y=7,w=126,h=23`;
- console row origins `y=7,15,23`;
- 21x3 console capacity.

Status content owns only:

```text
x=1..126
y=1..4
```

## 2. Information model

The status bar has two semantic fields.

Left field:

```text
COMM
```

This is a communications-status slot, not a fake network indicator.

Initial supported state:

- no communication indicator;
- UART/serial available indicator.

Future network hardware may reuse this slot, but Slice 4B must not imply that
network hardware exists.

Right field:

```text
HH:MM
```

Initial time source is system uptime.

For the first implementation:

- hours = uptime / 3600;
- minutes = (uptime / 60) % 60;
- display range is `00:00` through `99:59`;
- values above `99:59` saturate at `99:59`.

Using `HH:MM` now deliberately matches the eventual RTC wall-clock layout.
Replacing uptime with RTC later must not require a status-bar geometry change.

## 3. Why not `MM:SS`

`MM:SS` would force a visible update every second and would later change
semantics when an RTC is introduced.

`HH:MM` has two advantages:

- stable final field geometry for both uptime and future RTC time;
- normal updates only when the displayed minute changes.

The status bar therefore does not create a permanent 1 Hz OLED refresh load.

## 4. Micro-font

The existing 5x7 font does not fit in the four-pixel-high status region.

Slice 4B introduces a minimal immutable micro-font:

```text
glyph size: 3x4
advance:    4 pixels
required glyphs:
  0 1 2 3 4 5 6 7 8 9 :
```

Proposed files:

```text
include/gfx/font3x4.h
src/gfx/font3x4.c
```

Do not add letters unless a real status field requires them.

The COMM symbol is an icon and does not need to be encoded as a text glyph.

## 5. Status-bar component

Proposed files:

```text
include/kernel/oled_status_bar.h
src/kernel/oled_status_bar.c
```

The component owns semantic status state and rendering of the content region.

Proposed state:

```c
typedef enum
{
    OLED_STATUS_COMM_NONE = 0,
    OLED_STATUS_COMM_UART = 1
} oled_status_comm_t;

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    oled_status_comm_t comm;
    uint8_t dirty;
} oled_status_bar_t;
```

The exact representation may be refined during implementation, but these
semantics are fixed:

- no heap;
- no I2C;
- no SSD1306 page knowledge;
- no display flush;
- no console state ownership.

## 6. Rendering contract

The status-bar renderer receives a caller-owned clip:

```text
x=1, y=1, width=126, height=4
```

It may modify pixels only inside that clip.

Suggested API:

```c
void oled_status_bar_init(oled_status_bar_t *status);

void oled_status_bar_set_time(
    oled_status_bar_t *status,
    uint32_t hours,
    uint32_t minutes);

void oled_status_bar_set_comm(
    oled_status_bar_t *status,
    oled_status_comm_t comm);

void oled_status_bar_render(
    const oled_status_bar_t *status,
    mono_fb_t *fb,
    mono_rect_t clip);
```

The separator at `y=5` remains UI-composition ownership and is not drawn by the
status component.

## 7. Horizontal layout

Initial fixed layout:

```text
x=2..5      COMM icon slot (4x4)
x=107..126  HH:MM field (5 cells x 4 pixels)
```

The right field is fixed-width and right-aligned.

For a 3x4 glyph with 4-pixel advance:

```text
5 cells * 4 pixels = 20 pixels
126 - 20 + 1 = x=107
```

The middle region remains unused and reserved for future status information.

## 8. Slice 4B.1 — micro-font + static status renderer

Scope:

- add `font3x4`;
- add `oled_status_bar`;
- render COMM icon;
- render a fixed `12:34`;
- add temporary command:

```text
oledstatus
```

Expected UART response:

```text
OLED_STATUS_OK
```

Physical acceptance:

- COMM icon clean at the left;
- `12:34` readable at the right;
- no pixel touches frame `y=0`;
- separator `y=5` unchanged;
- row `y=6` remains blank;
- console area remains unchanged.

This slice must not introduce uptime integration yet.

## 9. Slice 4B.2 — semantic state + uptime integration

Only after 4B.1 physical acceptance:

- derive initial `HH:MM` from existing kernel uptime;
- set COMM state to UART when the UART subsystem is available;
- update status framebuffer only when displayed minute or COMM state changes;
- keep explicit OLED present orchestration outside the status component.

Regression display command:

```text
oledconsole
```

must show:

- status bar;
- separator;
- unchanged 21x3 console geometry.

## 10. Pixel-isolation acceptance

Because status pixels share SSD1306 page 0 with frame/separator/gap pixels,
page-level equality is not sufficient.

The on-target self-test must verify at pixel/bit-mask level that status
rendering does not alter any pixel outside:

```text
x=1..126
y=1..4
```

In particular these pixels are caller-owned and must survive:

```text
y=0
y=5
y=6
y=7
```

## 11. Refresh policy

Status setters mark state dirty only when the displayed value changes.

Baseline policy:

- time dirty when displayed minute changes;
- COMM dirty when state changes;
- rendering modifies framebuffer only;
- explicit present remains caller-controlled.

No periodic I2C transfer belongs inside the component.

## 12. Stop conditions

Stop and reject the slice if any implementation:

- moves the accepted console;
- moves separator `y=5`;
- writes status pixels into `y=0`, `y=5`, or `y=6`;
- adds SSD1306/I2C knowledge to the status component;
- adds a 1 Hz display flush requirement;
- invents network connectivity that the hardware does not have;
- breaks `OLED_RENDER_EQ_OK`;
- breaks `OLED_CONSOLE_OK`.

## 13. Commit discipline

Development sequence:

```text
plan
 -> implementation
 -> build
 -> validate
 -> flash
 -> UART regression
 -> physical status-bar inspection
 -> console regression
 -> PASS
 -> commit
 -> push
```

4B.1 and 4B.2 should be independently accepted checkpoints if 4B.2 changes
hardware-visible behavior beyond the static proof.
