#include <stdint.h>
#include "gfx/mono_fb.h"

#define MONO_FB_SPAN_CLEAN 0xFFFFu

static uint32_t mono_fb_page_count(const mono_fb_t *fb)
{
    if (fb == (const mono_fb_t *)0)
    {
        return 0u;
    }

    return (fb->height + 7u) / 8u;
}

static int mono_fb_valid(const mono_fb_t *fb)
{
    return
        (fb != (const mono_fb_t *)0) &&
        (fb->data != (uint8_t *)0) &&
        (fb->width != 0u) &&
        (fb->height != 0u) &&
        (fb->width <= 0xFFFFu) &&
        (mono_fb_page_count(fb) <= MONO_FB_MAX_PAGES);
}

static void mono_fb_reset_span(mono_fb_t *fb, uint32_t page)
{
    if ((fb == (mono_fb_t *)0) || (page >= MONO_FB_MAX_PAGES))
    {
        return;
    }

    fb->dirty_min_x[page] = MONO_FB_SPAN_CLEAN;
    fb->dirty_max_x[page] = 0u;
}

static void mono_fb_mark_page_x(
    mono_fb_t *fb,
    uint32_t page,
    uint32_t x)
{
    uint8_t page_mask;

    if (
        (fb == (mono_fb_t *)0) ||
        (page >= MONO_FB_MAX_PAGES) ||
        (x >= fb->width) ||
        (x > 0xFFFFu)
    ) {
        return;
    }

    page_mask = (uint8_t)(1u << page);

    if ((fb->dirty_pages & page_mask) == 0u)
    {
        fb->dirty_min_x[page] = (uint16_t)x;
        fb->dirty_max_x[page] = (uint16_t)x;
        fb->dirty_pages |= page_mask;
        return;
    }

    if (x < (uint32_t)fb->dirty_min_x[page])
    {
        fb->dirty_min_x[page] = (uint16_t)x;
    }

    if (x > (uint32_t)fb->dirty_max_x[page])
    {
        fb->dirty_max_x[page] = (uint16_t)x;
    }
}

void mono_fb_init(
    mono_fb_t *fb,
    uint8_t *storage,
    uint32_t width,
    uint32_t height)
{
    uint32_t page;

    if (fb == (mono_fb_t *)0)
    {
        return;
    }

    fb->data = storage;
    fb->width = width;
    fb->height = height;
    fb->dirty_pages = 0u;

    for (page = 0u; page < MONO_FB_MAX_PAGES; ++page)
    {
        mono_fb_reset_span(fb, page);
    }
}

void mono_fb_mark_all_dirty(mono_fb_t *fb)
{
    uint32_t pages;
    uint32_t page;

    if (mono_fb_valid(fb) == 0)
    {
        return;
    }

    pages = mono_fb_page_count(fb);

    if (pages >= MONO_FB_MAX_PAGES)
    {
        fb->dirty_pages = 0xFFu;
    }
    else if (pages == 0u)
    {
        fb->dirty_pages = 0u;
    }
    else
    {
        fb->dirty_pages = (uint8_t)((1u << pages) - 1u);
    }

    for (page = 0u; page < MONO_FB_MAX_PAGES; ++page)
    {
        if (page < pages)
        {
            fb->dirty_min_x[page] = 0u;
            fb->dirty_max_x[page] = (uint16_t)(fb->width - 1u);
        }
        else
        {
            mono_fb_reset_span(fb, page);
        }
    }
}

uint8_t mono_fb_dirty_pages(const mono_fb_t *fb)
{
    if (fb == (const mono_fb_t *)0)
    {
        return 0u;
    }

    return fb->dirty_pages;
}

int mono_fb_dirty_span(
    const mono_fb_t *fb,
    uint32_t page,
    uint32_t *min_x,
    uint32_t *max_x)
{
    uint8_t page_mask;

    if (
        (mono_fb_valid(fb) == 0) ||
        (min_x == (uint32_t *)0) ||
        (max_x == (uint32_t *)0) ||
        (page >= mono_fb_page_count(fb)) ||
        (page >= MONO_FB_MAX_PAGES)
    ) {
        return 0;
    }

    page_mask = (uint8_t)(1u << page);

    if (
        ((fb->dirty_pages & page_mask) == 0u) ||
        (fb->dirty_min_x[page] == MONO_FB_SPAN_CLEAN) ||
        (fb->dirty_min_x[page] > fb->dirty_max_x[page]) ||
        ((uint32_t)fb->dirty_max_x[page] >= fb->width)
    ) {
        return 0;
    }

    *min_x = (uint32_t)fb->dirty_min_x[page];
    *max_x = (uint32_t)fb->dirty_max_x[page];
    return 1;
}

void mono_fb_clear_dirty(mono_fb_t *fb, uint8_t mask)
{
    uint32_t page;

    if (fb == (mono_fb_t *)0)
    {
        return;
    }

    fb->dirty_pages &= (uint8_t)~mask;

    for (page = 0u; page < MONO_FB_MAX_PAGES; ++page)
    {
        if ((mask & (uint8_t)(1u << page)) != 0u)
        {
            mono_fb_reset_span(fb, page);
        }
    }
}

int mono_fb_write_masked_byte(
    mono_fb_t *fb,
    uint32_t page,
    uint32_t x,
    uint8_t value,
    uint8_t mask)
{
    uint32_t index;
    uint8_t old_value;
    uint8_t new_value;

    if (
        (mono_fb_valid(fb) == 0) ||
        (page >= mono_fb_page_count(fb)) ||
        (page >= MONO_FB_MAX_PAGES) ||
        (x >= fb->width) ||
        (mask == 0u)
    ) {
        return 0;
    }

    index = (page * fb->width) + x;
    old_value = fb->data[index];
    new_value =
        (uint8_t)(
            (old_value & (uint8_t)~mask) |
            (value & mask));

    if (new_value == old_value)
    {
        return 0;
    }

    fb->data[index] = new_value;
    mono_fb_mark_page_x(fb, page, x);
    return 1;
}

void mono_fb_clear(mono_fb_t *fb)
{
    uint32_t pages;
    uint32_t page;
    uint32_t x;

    if (mono_fb_valid(fb) == 0)
    {
        return;
    }

    pages = mono_fb_page_count(fb);

    for (page = 0u; page < pages; ++page)
    {
        for (x = 0u; x < fb->width; ++x)
        {
            (void)mono_fb_write_masked_byte(
                fb,
                page,
                x,
                0u,
                0xFFu);
        }
    }
}

void mono_fb_set_pixel(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int value)
{
    uint32_t ux;
    uint32_t uy;
    uint32_t page;
    uint8_t mask;

    if (mono_fb_valid(fb) == 0)
    {
        return;
    }

    if ((x < 0) || (y < 0))
    {
        return;
    }

    ux = (uint32_t)x;
    uy = (uint32_t)y;

    if ((ux >= fb->width) || (uy >= fb->height))
    {
        return;
    }

    page = uy / 8u;
    mask = (uint8_t)(1u << (uy & 7u));

    if (value != 0)
    {
        (void)mono_fb_write_masked_byte(
            fb,
            page,
            ux,
            mask,
            mask);
    }
    else
    {
        (void)mono_fb_write_masked_byte(
            fb,
            page,
            ux,
            0u,
            mask);
    }
}

void mono_fb_hline(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t width,
    int value)
{
    int32_t i;

    if (width <= 0)
    {
        return;
    }

    for (i = 0; i < width; ++i)
    {
        mono_fb_set_pixel(fb, x + i, y, value);
    }
}

void mono_fb_vline(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t height,
    int value)
{
    int32_t i;

    if (height <= 0)
    {
        return;
    }

    for (i = 0; i < height; ++i)
    {
        mono_fb_set_pixel(fb, x, y + i, value);
    }
}

void mono_fb_rect(
    mono_fb_t *fb,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    int value)
{
    if ((width <= 0) || (height <= 0))
    {
        return;
    }

    mono_fb_hline(fb, x, y, width, value);
    mono_fb_hline(fb, x, y + height - 1, width, value);
    mono_fb_vline(fb, x, y, height, value);
    mono_fb_vline(fb, x + width - 1, y, height, value);
}

int mono_fb_dirty_region_self_test(void)
{
    uint8_t storage[32];
    volatile uint8_t *storage_init = storage;
    mono_fb_t fb;
    uint32_t min_x;
    uint32_t max_x;
    uint32_t storage_index;

    for (storage_index = 0u;
         storage_index < (uint32_t)sizeof(storage);
         ++storage_index)
    {
        storage_init[storage_index] = 0u;
    }

    mono_fb_init(&fb, storage, 16u, 16u);

    mono_fb_set_pixel(&fb, 3, 2, 0);
    if (mono_fb_dirty_pages(&fb) != 0u)
    {
        return 0;
    }

    mono_fb_set_pixel(&fb, 3, 2, 1);
    if (
        (mono_fb_dirty_pages(&fb) != 0x01u) ||
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x != 3u) ||
        (max_x != 3u)
    ) {
        return 0;
    }

    mono_fb_set_pixel(&fb, 9, 7, 1);
    if (
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x != 3u) ||
        (max_x != 9u)
    ) {
        return 0;
    }

    mono_fb_clear_dirty(&fb, 0x01u);
    if (
        (mono_fb_dirty_pages(&fb) != 0u) ||
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) != 0)
    ) {
        return 0;
    }

    mono_fb_clear(&fb);
    if (
        (mono_fb_dirty_pages(&fb) != 0x01u) ||
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x != 3u) ||
        (max_x != 9u)
    ) {
        return 0;
    }

    mono_fb_clear_dirty(&fb, 0x01u);
    mono_fb_clear(&fb);
    if (mono_fb_dirty_pages(&fb) != 0u)
    {
        return 0;
    }

    mono_fb_mark_all_dirty(&fb);
    if (mono_fb_dirty_pages(&fb) != 0x03u)
    {
        return 0;
    }

    if (
        (mono_fb_dirty_span(&fb, 0u, &min_x, &max_x) == 0) ||
        (min_x != 0u) ||
        (max_x != 15u) ||
        (mono_fb_dirty_span(&fb, 1u, &min_x, &max_x) == 0) ||
        (min_x != 0u) ||
        (max_x != 15u)
    ) {
        return 0;
    }

    return 1;
}
