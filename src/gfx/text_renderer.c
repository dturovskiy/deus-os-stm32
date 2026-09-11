#include <stdint.h>
#include "gfx/text_renderer.h"
#include "gfx/font5x7.h"

static int text_renderer_axis_contains(
    int32_t value,
    int32_t start,
    int32_t length)
{
    if (length <= 0)
    {
        return 0;
    }

    if (value < start)
    {
        return 0;
    }

    return (uint32_t)(value - start) < (uint32_t)length;
}

static int text_renderer_clip_contains(
    mono_rect_t clip,
    int32_t x,
    int32_t y)
{
    return
        text_renderer_axis_contains(x, clip.x, clip.width) &&
        text_renderer_axis_contains(y, clip.y, clip.height);
}

void text_renderer_draw_cell(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    char c)
{
    const mono_font_metrics_t *metrics;
    const uint8_t *glyph;
    uint32_t column;
    uint32_t row;

    if (
        (fb == (mono_fb_t *)0) ||
        (fb->data == (uint8_t *)0) ||
        (clip.width <= 0) ||
        (clip.height <= 0)
    ) {
        return;
    }

    metrics = font5x7_metrics();
    glyph = font5x7_glyph(c);

    if (
        (metrics == (const mono_font_metrics_t *)0) ||
        (glyph == (const uint8_t *)0)
    ) {
        return;
    }

    for (column = 0u; column < metrics->advance_x; ++column)
    {
        for (row = 0u; row < metrics->advance_y; ++row)
        {
            int value = 0;
            int32_t px = x + (int32_t)column;
            int32_t py = y + (int32_t)row;

            if (text_renderer_clip_contains(clip, px, py) == 0)
            {
                continue;
            }

            if (
                (column < metrics->glyph_width) &&
                (row < metrics->glyph_height) &&
                ((glyph[column] & (uint8_t)(1u << row)) != 0u)
            ) {
                value = 1;
            }

            /*
             * This is deliberately the generic path.
             * mono_fb_set_pixel owns page/index math and framebuffer clipping.
             * The blank spacer column/row are written as zero, making cells
             * opaque and deterministic.
             */
            mono_fb_set_pixel(fb, px, py, value);
        }
    }
}
