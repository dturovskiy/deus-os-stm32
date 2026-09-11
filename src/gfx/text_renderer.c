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

static void text_renderer_draw_cell_generic(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    const uint8_t *glyph,
    const mono_font_metrics_t *metrics)
{
    uint32_t column;
    uint32_t row;

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

            mono_fb_set_pixel(fb, px, py, value);
        }
    }
}

static int text_renderer_can_use_aligned_fast_path(
    const mono_fb_t *fb,
    int32_t y,
    const mono_font_metrics_t *metrics)
{
    if (
        (fb == (const mono_fb_t *)0) ||
        (fb->data == (uint8_t *)0) ||
        (metrics == (const mono_font_metrics_t *)0)
    ) {
        return 0;
    }

    if (
        (metrics->glyph_width != 5u) ||
        (metrics->glyph_height != 7u) ||
        (metrics->advance_x != 6u) ||
        (metrics->advance_y != 8u)
    ) {
        return 0;
    }

    if ((y < 0) || ((y & 7) != 0))
    {
        return 0;
    }

    return ((uint32_t)y + 7u) < fb->height;
}

static void text_renderer_draw_cell_aligned(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    const uint8_t *glyph)
{
    uint32_t page = (uint32_t)y >> 3;
    uint32_t column;
    uint32_t row;
    uint8_t vertical_mask = 0u;
    int wrote_any_column = 0;

    for (row = 0u; row < 8u; ++row)
    {
        int32_t py = y + (int32_t)row;

        if (
            text_renderer_axis_contains(py, clip.y, clip.height) &&
            (py >= 0) &&
            ((uint32_t)py < fb->height)
        ) {
            vertical_mask |= (uint8_t)(1u << row);
        }
    }

    if (vertical_mask == 0u)
    {
        return;
    }

    for (column = 0u; column < 6u; ++column)
    {
        int32_t px = x + (int32_t)column;
        uint8_t cell_bits;
        uint32_t index;
        uint8_t old_value;

        if (
            (px < 0) ||
            ((uint32_t)px >= fb->width) ||
            (text_renderer_axis_contains(px, clip.x, clip.width) == 0)
        ) {
            continue;
        }

        cell_bits = (column < 5u) ? glyph[column] : 0u;
        index = page * fb->width + (uint32_t)px;
        old_value = fb->data[index];

        fb->data[index] =
            (uint8_t)(
                (old_value & (uint8_t)~vertical_mask) |
                (cell_bits & vertical_mask));

        wrote_any_column = 1;
    }

    if ((wrote_any_column != 0) && (page < 8u))
    {
        fb->dirty_pages |= (uint8_t)(1u << page);
    }
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

    if (text_renderer_can_use_aligned_fast_path(fb, y, metrics) != 0)
    {
        text_renderer_draw_cell_aligned(
            fb,
            clip,
            x,
            y,
            glyph);
        return;
    }

    text_renderer_draw_cell_generic(
        fb,
        clip,
        x,
        y,
        glyph,
        metrics);
}

typedef struct
{
    mono_rect_t clip;
    int32_t x;
    int32_t y;
    char c;
} text_renderer_self_test_case_t;

static void text_renderer_self_test_seed(
    uint8_t *storage,
    uint32_t length,
    uint8_t salt)
{
    uint32_t i;

    for (i = 0u; i < length; ++i)
    {
        storage[i] =
            (uint8_t)(
                (uint8_t)(i * 37u) ^
                (uint8_t)(0x5Au + salt));
    }
}

static int text_renderer_self_test_equal(
    const uint8_t *left,
    const uint8_t *right,
    uint32_t length)
{
    uint32_t i;

    for (i = 0u; i < length; ++i)
    {
        if (left[i] != right[i])
        {
            return 0;
        }
    }

    return 1;
}

int text_renderer_fast_path_self_test(void)
{
    static const text_renderer_self_test_case_t cases[] =
    {
        {{0, 0, 16, 16},   0, 0, 'A'},
        {{1, 1, 14, 14},   0, 0, 'M'},
        {{1, 1, 14, 14},  10, 8, 'I'},
        {{1, 1, 14, 14},  12, 8, 'R'},
        {{0, 8, 16, 8},    5, 8, 'Z'},
        {{2, 8, 10, 7},    4, 8, ' '},
        {{0, 0, 16, 7},   -2, 0, 'G'}
    };

    uint8_t generic_storage[32];
    uint8_t fast_storage[32];
    mono_fb_t generic_fb;
    mono_fb_t fast_fb;
    const mono_font_metrics_t *metrics = font5x7_metrics();
    uint32_t case_index;

    if (metrics == (const mono_font_metrics_t *)0)
    {
        return 0;
    }

    for (
        case_index = 0u;
        case_index < (uint32_t)(sizeof(cases) / sizeof(cases[0]));
        ++case_index
    ) {
        const text_renderer_self_test_case_t *test = &cases[case_index];
        const uint8_t *glyph = font5x7_glyph(test->c);
        uint8_t salt = (uint8_t)(case_index * 11u);

        if (glyph == (const uint8_t *)0)
        {
            return 0;
        }

        text_renderer_self_test_seed(
            generic_storage,
            (uint32_t)sizeof(generic_storage),
            salt);

        text_renderer_self_test_seed(
            fast_storage,
            (uint32_t)sizeof(fast_storage),
            salt);

        mono_fb_init(
            &generic_fb,
            generic_storage,
            16u,
            16u);

        mono_fb_init(
            &fast_fb,
            fast_storage,
            16u,
            16u);

        if (
            text_renderer_can_use_aligned_fast_path(
                &fast_fb,
                test->y,
                metrics) == 0
        ) {
            return 0;
        }

        text_renderer_draw_cell_generic(
            &generic_fb,
            test->clip,
            test->x,
            test->y,
            glyph,
            metrics);

        text_renderer_draw_cell_aligned(
            &fast_fb,
            test->clip,
            test->x,
            test->y,
            glyph);

        if (
            text_renderer_self_test_equal(
                generic_storage,
                fast_storage,
                (uint32_t)sizeof(generic_storage)) == 0
        ) {
            return 0;
        }

        if (generic_fb.dirty_pages != fast_fb.dirty_pages)
        {
            return 0;
        }
    }

    /*
     * Verify that the public dispatch keeps the generic fallback for
     * arbitrary-Y rendering.
     */
    {
        mono_rect_t clip = {1, 1, 14, 14};
        const uint8_t *glyph = font5x7_glyph('N');

        text_renderer_self_test_seed(
            generic_storage,
            (uint32_t)sizeof(generic_storage),
            0x33u);

        text_renderer_self_test_seed(
            fast_storage,
            (uint32_t)sizeof(fast_storage),
            0x33u);

        mono_fb_init(
            &generic_fb,
            generic_storage,
            16u,
            16u);

        mono_fb_init(
            &fast_fb,
            fast_storage,
            16u,
            16u);

        text_renderer_draw_cell_generic(
            &generic_fb,
            clip,
            4,
            5,
            glyph,
            metrics);

        text_renderer_draw_cell(
            &fast_fb,
            clip,
            4,
            5,
            'N');

        if (
            text_renderer_self_test_equal(
                generic_storage,
                fast_storage,
                (uint32_t)sizeof(generic_storage)) == 0
        ) {
            return 0;
        }

        if (generic_fb.dirty_pages != fast_fb.dirty_pages)
        {
            return 0;
        }
    }

    return 1;
}
