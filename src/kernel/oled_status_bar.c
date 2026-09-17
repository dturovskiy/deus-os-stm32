#include <stdint.h>
#include "kernel/oled_status_bar.h"
#include "gfx/font3x5.h"

#define OLED_STATUS_REFERENCE_WIDTH  128
#define OLED_STATUS_REFERENCE_HEIGHT 9
#define OLED_STATUS_TIME_CHARS       5u
#define OLED_STATUS_TIME_DIRTY_ALL   0x1Fu

static const uint8_t indicator_filled[5] =
{
    0x0Eu,
    0x1Fu,
    0x1Fu,
    0x1Fu,
    0x0Eu
};

static const uint8_t indicator_ring[5] =
{
    0x0Eu,
    0x1Bu,
    0x11u,
    0x1Bu,
    0x0Eu
};

static void draw_bitmap_5x5(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    const uint8_t bitmap[5])
{
    uint32_t row;
    uint32_t column;

    for (row = 0u; row < 5u; ++row)
    {
        for (column = 0u; column < 5u; ++column)
        {
            uint8_t mask =
                (uint8_t)(1u << (4u - column));

            mono_fb_set_pixel(
                fb,
                x + (int32_t)column,
                y + (int32_t)row,
                (bitmap[row] & mask) != 0u);
        }
    }
}

static void draw_font3x5(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    char c)
{
    const uint8_t *glyph;
    uint32_t row;
    uint32_t column;

    glyph = font3x5_glyph(c);

    if (glyph == (const uint8_t *)0)
    {
        return;
    }

    for (row = 0u; row < FONT3X5_GLYPH_HEIGHT; ++row)
    {
        for (column = 0u; column < FONT3X5_GLYPH_WIDTH; ++column)
        {
            uint8_t mask =
                (uint8_t)(
                    1u <<
                    (FONT3X5_GLYPH_WIDTH - 1u - column));

            mono_fb_set_pixel(
                fb,
                x + (int32_t)column,
                y + (int32_t)row,
                (glyph[row] & mask) != 0u);
        }
    }
}

static void clear_reference_rect(
    mono_fb_t *fb,
    mono_rect_t clip)
{
    int32_t y;
    int32_t x;

    for (y = clip.y; y < (clip.y + clip.height); ++y)
    {
        for (x = clip.x; x < (clip.x + clip.width); ++x)
        {
            mono_fb_set_pixel(fb, x, y, 0);
        }
    }
}

static void oled_status_time_text(
    uint8_t hours,
    uint8_t minutes,
    char text[OLED_STATUS_TIME_CHARS])
{
    text[0] = (char)('0' + (hours / 10u));
    text[1] = (char)('0' + (hours % 10u));
    text[2] = ':';
    text[3] = (char)('0' + (minutes / 10u));
    text[4] = (char)('0' + (minutes % 10u));
}

void oled_status_bar_init(
    oled_status_bar_t *status)
{
    if (status == (oled_status_bar_t *)0)
    {
        return;
    }

    status->hours = 0u;
    status->minutes = 0u;
    status->system_indicator = OLED_STATUS_INDICATOR_FILLED;
    status->usb_indicator = OLED_STATUS_INDICATOR_FILLED;
    status->network_indicator = OLED_STATUS_INDICATOR_RING;
    status->dirty = OLED_STATUS_DIRTY_ALL;
    status->time_dirty = OLED_STATUS_TIME_DIRTY_ALL;
}

void oled_status_bar_mark_all_dirty(
    oled_status_bar_t *status)
{
    if (status == (oled_status_bar_t *)0)
    {
        return;
    }

    status->dirty = OLED_STATUS_DIRTY_ALL;
    status->time_dirty = OLED_STATUS_TIME_DIRTY_ALL;
}

void oled_status_bar_set_time(
    oled_status_bar_t *status,
    uint32_t hours,
    uint32_t minutes)
{
    char old_text[OLED_STATUS_TIME_CHARS];
    char new_text[OLED_STATUS_TIME_CHARS];
    uint8_t new_hours;
    uint8_t new_minutes;
    uint32_t i;

    if (status == (oled_status_bar_t *)0)
    {
        return;
    }

    new_hours = (uint8_t)((hours > 99u) ? 99u : hours);
    new_minutes = (uint8_t)((minutes > 59u) ? 59u : minutes);

    if (
        (status->hours == new_hours) &&
        (status->minutes == new_minutes)
    ) {
        return;
    }

    oled_status_time_text(
        status->hours,
        status->minutes,
        old_text);
    oled_status_time_text(
        new_hours,
        new_minutes,
        new_text);

    for (i = 0u; i < OLED_STATUS_TIME_CHARS; ++i)
    {
        if (old_text[i] != new_text[i])
        {
            status->time_dirty |= (uint8_t)(1u << i);
        }
    }

    status->hours = new_hours;
    status->minutes = new_minutes;

    if (status->time_dirty != 0u)
    {
        status->dirty |= OLED_STATUS_DIRTY_TIME;
    }
}

void oled_status_bar_set_indicators(
    oled_status_bar_t *status,
    oled_status_indicator_t system_indicator,
    oled_status_indicator_t usb_indicator,
    oled_status_indicator_t network_indicator)
{
    oled_status_indicator_t new_system;
    oled_status_indicator_t new_usb;
    oled_status_indicator_t new_network;

    if (status == (oled_status_bar_t *)0)
    {
        return;
    }

    new_system =
        (system_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            OLED_STATUS_INDICATOR_FILLED :
            OLED_STATUS_INDICATOR_RING;

    new_usb =
        (usb_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            OLED_STATUS_INDICATOR_FILLED :
            OLED_STATUS_INDICATOR_RING;

    new_network =
        (network_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            OLED_STATUS_INDICATOR_FILLED :
            OLED_STATUS_INDICATOR_RING;

    if (status->system_indicator != new_system)
    {
        status->system_indicator = (uint8_t)new_system;
        status->dirty |= OLED_STATUS_DIRTY_SYSTEM;
    }

    if (status->usb_indicator != new_usb)
    {
        status->usb_indicator = (uint8_t)new_usb;
        status->dirty |= OLED_STATUS_DIRTY_USB;
    }

    if (status->network_indicator != new_network)
    {
        status->network_indicator = (uint8_t)new_network;
        status->dirty |= OLED_STATUS_DIRTY_NETWORK;
    }
}

void oled_status_bar_render(
    oled_status_bar_t *status,
    mono_fb_t *fb,
    mono_rect_t clip)
{
    char time_text[OLED_STATUS_TIME_CHARS];
    static const uint8_t time_x_offsets[OLED_STATUS_TIME_CHARS] =
    {
        0u,
        4u,
        7u,
        10u,
        14u
    };

    uint32_t i;
    int32_t x;
    int32_t y;

    if (
        (status == (oled_status_bar_t *)0) ||
        (fb == (mono_fb_t *)0) ||
        (clip.width != OLED_STATUS_REFERENCE_WIDTH) ||
        (clip.height != OLED_STATUS_REFERENCE_HEIGHT)
    ) {
        return;
    }

    if ((status->dirty & OLED_STATUS_DIRTY_FRAME) != 0u)
    {
        status->dirty |= OLED_STATUS_DIRTY_ALL;
        status->time_dirty |= OLED_STATUS_TIME_DIRTY_ALL;

        clear_reference_rect(fb, clip);

        mono_fb_hline(
            fb,
            clip.x,
            clip.y,
            clip.width,
            1);

        mono_fb_hline(
            fb,
            clip.x,
            clip.y + 8,
            clip.width,
            1);

        for (y = clip.y + 1; y <= (clip.y + 7); ++y)
        {
            mono_fb_set_pixel(
                fb,
                clip.x,
                y,
                1);

            mono_fb_set_pixel(
                fb,
                clip.x + 127,
                y,
                1);
        }
    }

    if ((status->dirty & OLED_STATUS_DIRTY_SYSTEM) != 0u)
    {
        draw_bitmap_5x5(
            fb,
            clip.x + 2,
            clip.y + 2,
            (status->system_indicator == OLED_STATUS_INDICATOR_FILLED) ?
                indicator_filled :
                indicator_ring);
    }

    if ((status->dirty & OLED_STATUS_DIRTY_USB) != 0u)
    {
        draw_bitmap_5x5(
            fb,
            clip.x + 8,
            clip.y + 2,
            (status->usb_indicator == OLED_STATUS_INDICATOR_FILLED) ?
                indicator_filled :
                indicator_ring);
    }

    if ((status->dirty & OLED_STATUS_DIRTY_NETWORK) != 0u)
    {
        draw_bitmap_5x5(
            fb,
            clip.x + 14,
            clip.y + 2,
            (status->network_indicator == OLED_STATUS_INDICATOR_FILLED) ?
                indicator_filled :
                indicator_ring);
    }

    if ((status->dirty & OLED_STATUS_DIRTY_TIME) != 0u)
    {
        oled_status_time_text(
            status->hours,
            status->minutes,
            time_text);

        /*
         * Exact supplied reference:
         * digit starts x=109,113; colon center x=117;
         * final digits start x=119,123 and end at x=125.
         * x=126 is the mandatory blank right inset.
         * x=127 is the status-frame side.
         */
        x = clip.x + 109;

        for (i = 0u; i < OLED_STATUS_TIME_CHARS; ++i)
        {
            if ((status->time_dirty & (uint8_t)(1u << i)) == 0u)
            {
                continue;
            }

            draw_font3x5(
                fb,
                x + (int32_t)time_x_offsets[i],
                clip.y + 2,
                time_text[i]);
        }
    }

    status->dirty = 0u;
    status->time_dirty = 0u;
}

static int framebuffer_pixel(
    const uint8_t *storage,
    uint32_t width,
    uint32_t x,
    uint32_t y)
{
    uint32_t index =
        ((y / 8u) * width) + x;

    uint8_t mask =
        (uint8_t)(1u << (y & 7u));

    return (storage[index] & mask) != 0u;
}

static int expected_reference_pixel(
    uint32_t x,
    uint32_t y)
{
    /*
     * Exact alpha mask of the supplied 128x32 reference image,
     * restricted to its 128x9 status-bar region.
     */
    static const uint8_t rows[9][16] =
    {
        {
            0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,
            0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu
        },
        {
            0x80u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x01u
        },
        {
            0x9Cu,0x71u,0xC0u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x07u,0x71u,0xDDu
        },
        {
            0xBEu,0xFBu,0x60u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x05u,0x55u,0x55u
        },
        {
            0xBEu,0xFAu,0x20u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x05u,0x51u,0x55u
        },
        {
            0xBEu,0xFBu,0x60u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x05u,0x55u,0x55u
        },
        {
            0x9Cu,0x71u,0xC0u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x07u,0x71u,0xDDu
        },
        {
            0x80u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,
            0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x01u
        },
        {
            0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,
            0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu,0xFFu
        }
    };

    uint8_t mask;

    if ((x >= 128u) || (y >= 9u))
    {
        return 0;
    }

    mask =
        (uint8_t)(1u << (7u - (x & 7u)));

    return
        (rows[y][x / 8u] & mask) != 0u;
}

int oled_status_bar_self_test(void)
{
    uint8_t storage[128];
    mono_fb_t fb;
    oled_status_bar_t status;
    static const mono_rect_t clip =
    {
        0,
        0,
        128,
        9
    };
    static const mono_rect_t bottom_row_clip =
    {
        0,
        -8,
        128,
        9
    };

    uint32_t i;
    uint32_t y;
    uint32_t x;
    uint32_t min_x;
    uint32_t max_x;

    mono_fb_init(
        &fb,
        storage,
        128u,
        8u);

    for (i = 0u; i < 128u; ++i)
    {
        storage[i] = 0u;
    }

    oled_status_bar_init(&status);
    oled_status_bar_set_time(
        &status,
        0u,
        0u);

    oled_status_bar_render(
        &status,
        &fb,
        clip);

    if ((status.dirty != 0u) || (status.time_dirty != 0u))
    {
        return 0;
    }

    for (y = 0u; y < 8u; ++y)
    {
        for (x = 0u; x < 128u; ++x)
        {
            int actual =
                framebuffer_pixel(
                    storage,
                    128u,
                    x,
                    y);

            int expected =
                expected_reference_pixel(
                    x,
                    y);

            if (actual != expected)
            {
                return 0;
            }
        }
    }

    mono_fb_clear_dirty(&fb, 0xFFu);

    oled_status_bar_set_time(&status, 0u, 1u);

    if (
        (status.dirty != OLED_STATUS_DIRTY_TIME) ||
        (status.time_dirty != (uint8_t)(1u << 4))
    ) {
        return 0;
    }

    oled_status_bar_render(&status, &fb, clip);

    if (
        (mono_fb_dirty_pages(&fb) != 0x01u) ||
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x < 123u) ||
        (max_x > 125u)
    ) {
        return 0;
    }

    mono_fb_clear_dirty(&fb, 0xFFu);

    oled_status_bar_set_indicators(
        &status,
        OLED_STATUS_INDICATOR_FILLED,
        OLED_STATUS_INDICATOR_RING,
        OLED_STATUS_INDICATOR_RING);

    if (status.dirty != OLED_STATUS_DIRTY_USB)
    {
        return 0;
    }

    oled_status_bar_render(&status, &fb, clip);

    if (
        (mono_fb_dirty_pages(&fb) != 0x01u) ||
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x < 8u) ||
        (max_x > 12u)
    ) {
        return 0;
    }

    mono_fb_clear_dirty(&fb, 0xFFu);

    oled_status_bar_set_indicators(
        &status,
        OLED_STATUS_INDICATOR_RING,
        OLED_STATUS_INDICATOR_FILLED,
        OLED_STATUS_INDICATOR_FILLED);

    oled_status_bar_render(
        &status,
        &fb,
        clip);

    if (
        (framebuffer_pixel(storage, 128u, 4u, 4u) != 0) ||
        (framebuffer_pixel(storage, 128u, 10u, 4u) == 0) ||
        (framebuffer_pixel(storage, 128u, 16u, 4u) == 0)
    ) {
        return 0;
    }

    for (i = 0u; i < 128u; ++i)
    {
        storage[i] = 0u;
    }

    mono_fb_init(
        &fb,
        storage,
        128u,
        1u);
    oled_status_bar_init(&status);
    oled_status_bar_render(
        &status,
        &fb,
        bottom_row_clip);

    for (x = 0u; x < 128u; ++x)
    {
        if (
            framebuffer_pixel(storage, 128u, x, 0u) !=
            expected_reference_pixel(x, 8u)
        ) {
            return 0;
        }
    }

    return 1;
}
