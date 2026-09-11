#ifndef KERNEL_OLED_STATUS_BAR_H
#define KERNEL_OLED_STATUS_BAR_H

#include <stdint.h>
#include "gfx/mono_fb.h"

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t dirty;
} oled_status_bar_t;

void oled_status_bar_init(
    oled_status_bar_t *status);

void oled_status_bar_set_time(
    oled_status_bar_t *status,
    uint32_t hours,
    uint32_t minutes);

void oled_status_bar_render(
    const oled_status_bar_t *status,
    mono_fb_t *fb,
    mono_rect_t clip);

int oled_status_bar_self_test(void);

#endif
