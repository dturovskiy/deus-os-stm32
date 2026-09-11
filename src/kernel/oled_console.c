#include <stdint.h>
#include "kernel/oled_console.h"
#include "gfx/font5x6.h"
#include "gfx/text_renderer.h"

static uint8_t oled_console_all_rows_mask(void)
{
    return (uint8_t)((1u << OLED_CONSOLE_ROWS) - 1u);
}

static void oled_console_mark_row(
    oled_console_t *console,
    uint32_t row)
{
    if (
        (console == (oled_console_t *)0) ||
        (row >= OLED_CONSOLE_ROWS)
    ) {
        return;
    }

    console->dirty_rows |= (uint8_t)(1u << row);
}

static int oled_console_prepare_printable(
    oled_console_t *console)
{
    if (console == (oled_console_t *)0)
    {
        return 0;
    }

    if (console->cursor_y >= OLED_CONSOLE_ROWS)
    {
        return 0;
    }

    /*
     * cursor_x == OLED_CONSOLE_COLUMNS is the pending-wrap state.
     * Wrapping happens only when another printable character arrives.
     * This prevents an exact-width line followed by LF from skipping a row.
     */
    if (console->cursor_x >= OLED_CONSOLE_COLUMNS)
    {
        console->cursor_x = 0u;
        ++console->cursor_y;
    }

    return console->cursor_y < OLED_CONSOLE_ROWS;
}

void oled_console_init(oled_console_t *console)
{
    oled_console_clear(console);
}

void oled_console_clear(oled_console_t *console)
{
    uint32_t row;
    uint32_t column;

    if (console == (oled_console_t *)0)
    {
        return;
    }

    for (row = 0u; row < OLED_CONSOLE_ROWS; ++row)
    {
        for (column = 0u; column < OLED_CONSOLE_COLUMNS; ++column)
        {
            console->cells[row][column] = ' ';
        }
    }

    console->first_row = 0u;
    console->cursor_x = 0u;
    console->cursor_y = 0u;
    console->dirty_rows = oled_console_all_rows_mask();
}

void oled_console_putc(oled_console_t *console, char c)
{
    uint32_t row;

    if (console == (oled_console_t *)0)
    {
        return;
    }

    if (c == '\r')
    {
        console->cursor_x = 0u;
        return;
    }

    if (c == '\n')
    {
        console->cursor_x = 0u;

        if (console->cursor_y < OLED_CONSOLE_ROWS)
        {
            ++console->cursor_y;
        }

        return;
    }

    if (((uint8_t)c < 0x20u) || ((uint8_t)c > 0x7Eu))
    {
        return;
    }

    if (oled_console_prepare_printable(console) == 0)
    {
        return;
    }

    row = console->cursor_y;
    console->cells[row][console->cursor_x] = c;
    oled_console_mark_row(console, row);
    ++console->cursor_x;
}

void oled_console_write(
    oled_console_t *console,
    const char *text)
{
    if (
        (console == (oled_console_t *)0) ||
        (text == (const char *)0)
    ) {
        return;
    }

    while (*text != '\0')
    {
        oled_console_putc(console, *text);
        ++text;
    }
}

void oled_console_write_line(
    oled_console_t *console,
    const char *text)
{
    oled_console_write(console, text);
    oled_console_putc(console, '\n');
}

void oled_console_render(
    const oled_console_t *console,
    mono_fb_t *fb,
    mono_rect_t clip)
{
    const mono_font_metrics_t *metrics;
    uint32_t available_columns;
    uint32_t available_rows;
    uint32_t row;
    uint32_t column;

    if (
        (console == (const oled_console_t *)0) ||
        (fb == (mono_fb_t *)0) ||
        (fb->data == (uint8_t *)0) ||
        (clip.width <= 0) ||
        (clip.height <= 0)
    ) {
        return;
    }

    metrics = font5x6_metrics();

    if (
        (metrics == (const mono_font_metrics_t *)0) ||
        (metrics->glyph_width == 0u) ||
        (metrics->glyph_height == 0u) ||
        (metrics->advance_x == 0u) ||
        (metrics->advance_y == 0u)
    ) {
        return;
    }

    /*
     * Horizontal contract:
     * 5-pixel glyph + 1-pixel spacer.
     * x=1,w=126 keeps all 21 columns.
     */
    if ((uint32_t)clip.width < (uint32_t)metrics->glyph_width)
    {
        available_columns = 0u;
    }
    else
    {
        available_columns =
            1u +
            (
                ((uint32_t)clip.width -
                 (uint32_t)metrics->glyph_width) /
                (uint32_t)metrics->advance_x
            );
    }

    if (available_columns > OLED_CONSOLE_COLUMNS)
    {
        available_columns = OLED_CONSOLE_COLUMNS;
    }

    /*
     * Frozen final vertical contract:
     *
     * status bar   y=0..8
     * blank gap    y=9
     * console clip y=10..31
     *
     * Compact cell = 6-pixel glyph + 1-pixel spacer:
     *
     * row 0 glyph  y=10..15
     * gap          y=16
     * row 1 glyph  y=17..22
     * gap          y=23
     * row 2 glyph  y=24..29
     * gap          y=30
     * bottom blank y=31
     */
    if ((uint32_t)clip.height < (uint32_t)metrics->glyph_height)
    {
        available_rows = 0u;
    }
    else
    {
        available_rows =
            1u +
            (
                ((uint32_t)clip.height -
                 (uint32_t)metrics->glyph_height) /
                (uint32_t)metrics->advance_y
            );
    }

    if (available_rows > OLED_CONSOLE_ROWS)
    {
        available_rows = OLED_CONSOLE_ROWS;
    }

    for (row = 0u; row < available_rows; ++row)
    {
        uint32_t logical_row =
            (console->first_row + row) % OLED_CONSOLE_ROWS;

        for (column = 0u; column < available_columns; ++column)
        {
            int32_t x =
                clip.x +
                (int32_t)(
                    column *
                    (uint32_t)metrics->advance_x);

            int32_t y =
                clip.y +
                (int32_t)(
                    row *
                    (uint32_t)metrics->advance_y);

            const uint8_t *glyph =
                font5x6_glyph(
                    console->cells[logical_row][column]);

            text_renderer_draw_glyph_cell(
                fb,
                clip,
                x,
                y,
                glyph,
                metrics);
        }
    }
}
