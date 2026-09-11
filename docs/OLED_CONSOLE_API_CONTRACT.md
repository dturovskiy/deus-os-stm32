# OLED Console API Contract

Status: **PLANNED**

This file defines the intended C boundaries before implementation.

The exact names may receive minor mechanical adjustments during implementation, but responsibility boundaries and ownership rules are normative.

## 1. General rules

All APIs are:

- heap-free;
- deterministic;
- usable without libc allocation;
- explicit about ownership;
- free of hidden I2C transfer in graphics/text functions.

Return values should use small integer success/failure conventions consistent with the existing codebase.

## 2. `gfx/mono_fb.h`

Planned types:

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

Planned operations:

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

void mono_fb_mark_all_dirty(mono_fb_t *fb);
uint8_t mono_fb_dirty_pages(const mono_fb_t *fb);
void mono_fb_clear_dirty(mono_fb_t *fb, uint8_t mask);
```

All drawing operations clip safely to the framebuffer.

Out-of-range coordinates must not write outside storage.

## 3. `gfx/font5x7.h`

Planned immutable metrics:

```c
typedef struct
{
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t advance_x;
    uint8_t advance_y;
} mono_font_metrics_t;
```

Planned accessors:

```c
const mono_font_metrics_t *font5x7_metrics(void);
const uint8_t *font5x7_glyph(char c);
```

The glyph representation remains column-oriented and optimized for 5x7 monochrome use.

Unsupported characters map deterministically to a defined fallback glyph.

## 4. `gfx/text_renderer.h`

Planned operation:

```c
void text_renderer_draw_cell(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    char c);
```

Contract:

- draws one opaque 6x8 logical cell using the 5x7 font;
- clips every write to `clip`;
- never modifies pixels outside `clip`;
- clears stale glyph pixels inside the cell;
- may select an optimized aligned path internally;
- does not update cursor state;
- does not perform display transfer.

A later generic font parameter may be introduced only when a second font is actually needed.

## 5. `kernel/oled_console.h`

Planned dimensions for the first implementation:

```text
columns = 21
rows    = 7
```

Planned type:

```c
typedef struct
{
    char cells[7][21];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;
```

Planned API:

```c
void oled_console_init(oled_console_t *console);
void oled_console_clear(oled_console_t *console);

void oled_console_putc(
    oled_console_t *console,
    char c);

void oled_console_write(
    oled_console_t *console,
    const char *text);

void oled_console_write_line(
    oled_console_t *console,
    const char *text);

void oled_console_render(
    const oled_console_t *console,
    mono_fb_t *fb,
    mono_rect_t viewport);
```

Newline behavior:

- `\r` returns to column zero or is ignored according to one documented policy;
- `\n` advances one row;
- reaching column 21 automatically wraps;
- advancing beyond row 6 performs circular scroll.

The console does not flush the display.

## 6. `drivers/ssd1306.h`

Planned panel profile:

```c
typedef struct
{
    uint8_t i2c_address;
    uint8_t segment_remap;
    uint8_t com_scan_direction;
    uint8_t display_offset;
    uint8_t start_line;
    mono_rect_t visible_rect;
} ssd1306_panel_profile_t;
```

The current accepted profile encodes:

```text
address       = 0x3C
segment remap = A1
COM scan      = C8
D3            = 0x00
start line    = 0x40
visible rect  = x=0..127, y=3..63
```

Planned operations:

```c
int ssd1306_init(
    const ssd1306_panel_profile_t *profile);

int ssd1306_present_full(
    const mono_fb_t *fb);

int ssd1306_present(
    mono_fb_t *fb);
```

Initial `ssd1306_present()` may delegate to full presentation.

Later it may consume `dirty_pages` and transfer only dirty pages.

This optimization must not change callers.

## 7. Ownership

Framebuffer storage is statically allocated by the system.

`mono_fb_t` references that storage.

`oled_console_t` owns only character state and cursor metadata.

Font data is immutable static data.

The SSD1306 driver does not own the framebuffer.

No API returns heap-owned memory.

## 8. Side-effect matrix

```text
function family            modifies RAM   modifies OLED/I2C
-----------------------------------------------------------
oled_console_*             yes            no
text_renderer_*            yes            no
mono_fb_*                  yes            no
ssd1306_init               driver state   yes
ssd1306_present*           dirty state    yes
```

This separation is a mandatory acceptance criterion.
