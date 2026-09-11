#include <stdint.h>
#include "gfx/mono_fb.h"

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
        (fb->height != 0u);
}

static void mono_fb_mark_page(mono_fb_t *fb, uint32_t page)
{
    if ((fb == (mono_fb_t *)0) || (page >= 8u))
    {
        return;
    }

    fb->dirty_pages |= (uint8_t)(1u << page);
}

void mono_fb_init(
    mono_fb_t *fb,
    uint8_t *storage,
    uint32_t width,
    uint32_t height)
{
    if (fb == (mono_fb_t *)0)
    {
        return;
    }

    fb->data = storage;
    fb->width = width;
    fb->height = height;
    fb->dirty_pages = 0u;
}

void mono_fb_mark_all_dirty(mono_fb_t *fb)
{
    uint32_t pages;

    if (fb == (mono_fb_t *)0)
    {
        return;
    }

    pages = mono_fb_page_count(fb);

    if (pages >= 8u)
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
}

uint8_t mono_fb_dirty_pages(const mono_fb_t *fb)
{
    if (fb == (const mono_fb_t *)0)
    {
        return 0u;
    }

    return fb->dirty_pages;
}

void mono_fb_clear_dirty(mono_fb_t *fb, uint8_t mask)
{
    if (fb == (mono_fb_t *)0)
    {
        return;
    }

    fb->dirty_pages &= (uint8_t)~mask;
}

void mono_fb_clear(mono_fb_t *fb)
{
    uint32_t bytes;
    uint32_t i;

    if (mono_fb_valid(fb) == 0)
    {
        return;
    }

    bytes = fb->width * mono_fb_page_count(fb);

    for (i = 0u; i < bytes; ++i)
    {
        fb->data[i] = 0u;
    }

    mono_fb_mark_all_dirty(fb);
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
    uint32_t index;
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
    index = page * fb->width + ux;
    mask = (uint8_t)(1u << (uy & 7u));

    if (value != 0)
    {
        fb->data[index] |= mask;
    }
    else
    {
        fb->data[index] &= (uint8_t)~mask;
    }

    mono_fb_mark_page(fb, page);
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
