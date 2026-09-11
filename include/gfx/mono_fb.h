#ifndef GFX_MONO_FB_H
#define GFX_MONO_FB_H

#include <stdint.h>

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

#endif
