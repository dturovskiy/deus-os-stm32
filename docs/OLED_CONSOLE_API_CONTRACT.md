# OLED Console API Contract

Status: PLANNED / NATIVE 128x32

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

Required API:

```c
const mono_font_metrics_t *font5x7_metrics(void);
const uint8_t *font5x7_glyph(char c);
```

Current metrics:

```text
glyph_width  = 5
glyph_height = 7
advance_x    = 6
advance_y    = 8
```

## 3. `text_renderer`

Required public entry point:

```c
void text_renderer_draw_cell(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    char c);
```

Contract:

- cell is opaque;
- generic arbitrary-Y path must be correct first;
- page-aligned fast path is optional optimization;
- fast path must be framebuffer-equivalent to generic output;
- clipping must apply to both on and off pixels;
- writes outside the clip rectangle are forbidden.

Canonical framed console clip:

```text
x=1, y=1, width=126, height=30
```

Canonical cell origins:

```text
x = 1 + column * 6
y = 8 + row * 8
```

For row 2, glyph pixels may touch `y=30`, but `y=31` belongs to the bottom
frame and must not be overwritten.

## 4. `oled_console`

```c
typedef struct
{
    char cells[3][21];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;
```

Required API:

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

Baseline character behavior:

- printable ASCII;
- `\n`;
- `\r`;
- automatic wrap.

Deferred:

- tab;
- backspace editing;
- ANSI;
- UTF-8;
- proportional fonts.

## 5. `ssd1306`

Native panel profile:

```c
typedef struct
{
    uint8_t i2c_address;
    uint8_t segment_remap;
    uint8_t com_scan_direction;
    uint8_t display_offset;
    uint8_t start_line;
    uint8_t multiplex;
    uint8_t com_pins;
    mono_rect_t visible_rect;
} ssd1306_panel_profile_t;
```

Canonical installed values:

```text
i2c_address       = 0x3C
segment_remap     = A1
com_scan_direction= C8
display_offset    = 0
start_line        = 0
multiplex         = 0x1F
com_pins          = 0x02
visible_rect      = x0,y0,w128,h32
```

Required presentation API:

```c
int ssd1306_init(void);
int ssd1306_present_full(const uint8_t *framebuffer);
int ssd1306_present(const mono_fb_t *fb);
```

Initial `ssd1306_present()` may delegate to the full 512-byte transfer.

## 6. Ownership and side effects

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
- no I2C.

`ssd1306`:
- owns OLED controller/I2C presentation;
- must not own semantic text state.

`kernel.c`:
- orchestrates modules;
- command handlers may request explicit present.
