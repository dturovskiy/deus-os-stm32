> [!IMPORTANT]
> **OLED UI status: ACCEPTED / FROZEN (2026-09-11).**
> The authoritative hardware-accepted geometry and firmware fingerprint are in
> [`OLED_UI_ACCEPTED_BASELINE.md`](OLED_UI_ACCEPTED_BASELINE.md).
> Any configurable-layout, preset, custom-layout, persistence, or alternate-geometry
> material below is deferred planning and must not override the accepted baseline.
# OLED Console API Contract

Status: ACTIVE / NATIVE 128x32 / SLICE 4 ACCEPTED

## 1. `mono_fb`

```c
typedef struct
{
    uint8_t *data;
    uint32_t width;
    uint32_t height;
    uint8_t dirty_pages;
} mono_fb_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} mono_rect_t;
```

Required operations:

```c
void mono_fb_init(
    mono_fb_t *fb,
    uint8_t *storage,
    uint32_t width,
    uint32_t height);

void mono_fb_clear(mono_fb_t *fb);

void mono_fb_set_pixel(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int value);

void mono_fb_hline(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t width,
    int value);

void mono_fb_vline(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t height,
    int value);

void mono_fb_rect(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    int value);

uint8_t mono_fb_dirty_pages(const mono_fb_t *fb);
void mono_fb_mark_all_dirty(mono_fb_t *fb);
void mono_fb_clear_dirty(mono_fb_t *fb, uint8_t mask);
```

For the installed OLED:

```text
width = 128
height = 32
storage = 512 bytes
dirty-page bits = 0..3
```

## 2. `font5x7`

```c
typedef struct
{
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t advance_x;
    uint8_t advance_y;
} mono_font_metrics_t;
```

Current metrics:

```text
glyph_width  = 5
glyph_height = 7
advance_x    = 6
advance_y    = 8
```

## 3. `text_renderer`

Public entry point:

```c
void text_renderer_draw_cell(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    char c);
```

Accepted contract:

- cell is opaque;
- arbitrary-Y generic path is authoritative;
- aligned fast path may be used only when eligible;
- fast output is byte-for-byte equivalent to generic output;
- clipping applies to foreground and background writes;
- writes outside the clip rectangle are forbidden.

The fast-path diagnostic remains available:

```c
int text_renderer_fast_path_self_test(void);
```

## 4. `oled_console`

```c
#define OLED_CONSOLE_COLUMNS 21u
#define OLED_CONSOLE_ROWS     3u

typedef struct
{
    char cells[OLED_CONSOLE_ROWS][OLED_CONSOLE_COLUMNS];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;
```

Accepted size on the current ABI/build:

```text
67 bytes
```

Public API:

```c
void oled_console_init(oled_console_t *console);
void oled_console_clear(oled_console_t *console);
void oled_console_putc(oled_console_t *console, char c);
void oled_console_write(oled_console_t *console, const char *text);
void oled_console_write_line(oled_console_t *console, const char *text);

void oled_console_render(
    const oled_console_t *console,
    mono_fb_t *fb,
    mono_rect_t clip);
```

Current character behavior:

- printable ASCII;
- `\n`;
- `\r`;
- automatic wrap;
- no scrolling yet;
- writes beyond the third row are ignored.

Deferred:

- scrolling;
- tab;
- backspace editing;
- ANSI;
- UTF-8;
- proportional fonts.

## 5. UI composition contract

The console does not own the whole screen.

Accepted caller-owned layout:

```text
outer frame:       x=0..127, y=0..31
status content:    x=1..126, y=1..4
status separator:  x=1..126, y=5
gap:               y=6
console viewport:  x=1, y=7, width=126, height=23
bottom gap:        y=30
```

For the current font metrics the viewport derives:

```text
column x = 1 + column * 6
row y    = 7 + row * 8
rows     = y=7,15,23
capacity = 21 x 3
```

The console must not know or reproduce status-bar geometry internally. It only
consumes the viewport passed by the caller.

## 6. `ssd1306`

Canonical installed values:

```text
i2c_address        = 0x3C
resolution         = 128x32
multiplex          = 0x1F
com_pins           = 0x02
display_offset     = 0
start_line         = 0
segment_remap      = A1
com_scan_direction = C8
visible_rect       = x0,y0,w128,h32
```

Presentation API:

```c
int ssd1306_init(void);
int ssd1306_present_full(const uint8_t *framebuffer);
int ssd1306_present(const mono_fb_t *fb);
```

Initial `ssd1306_present()` may delegate to the full 512-byte transfer.

## 7. Ownership and side effects

`font5x7`:
- immutable data only;
- no hardware access.

`mono_fb`:
- RAM only;
- no hardware access.

`text_renderer`:
- modifies framebuffer only;
- no flush.

`oled_console`:
- modifies semantic state;
- renders only when explicitly requested;
- no I2C;
- no SSD1306 page semantics;
- no status-bar ownership.

`ssd1306`:
- owns OLED controller/I2C presentation;
- must not own semantic text state.

UI composition:
- owns frame/status/console viewport placement;
- currently orchestrated in `kernel.c`;
- may later move to a dedicated UI module.

## 8. Planned UI layout/config API

The next UI slice introduces a validated layout object conceptually equivalent
to:

```c
typedef struct
{
    mono_rect_t status_rect;
    mono_rect_t console_rect;
    int8_t separator_y;
    uint8_t border_flags;
    uint8_t status_enabled;
    uint8_t separator_enabled;
} oled_ui_layout_t;
```

Planned operations:

```c
const oled_ui_layout_t *oled_ui_layout_preset(...);
int oled_ui_layout_validate(const oled_ui_layout_t *layout);
int oled_ui_layout_apply(const oled_ui_layout_t *candidate);
const oled_ui_layout_t *oled_ui_layout_current(void);
```

Exact names may be refined during implementation.

Contract:

- validation before activation;
- invalid candidate leaves current layout unchanged;
- no heap;
- no I2C;
- no SSD1306 page semantics;
- console and status render only inside caller-provided clips;
- runtime command transport is independent from UART/USB.

See `docs/OLED_UI_LAYOUT_PLAN.md`.
