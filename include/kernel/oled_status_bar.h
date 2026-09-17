#ifndef KERNEL_OLED_STATUS_BAR_H
#define KERNEL_OLED_STATUS_BAR_H

#include <stdint.h>
#include "gfx/mono_fb.h"

typedef enum
{
    OLED_STATUS_INDICATOR_RING = 0,
    OLED_STATUS_INDICATOR_FILLED = 1
} oled_status_indicator_t;

#define OLED_STATUS_DIRTY_FRAME   (1u << 0)
#define OLED_STATUS_DIRTY_SYSTEM  (1u << 1)
#define OLED_STATUS_DIRTY_USB     (1u << 2)
#define OLED_STATUS_DIRTY_NETWORK (1u << 3)
#define OLED_STATUS_DIRTY_TIME    (1u << 4)
#define OLED_STATUS_DIRTY_ALL     \
    (OLED_STATUS_DIRTY_FRAME | OLED_STATUS_DIRTY_SYSTEM | \
     OLED_STATUS_DIRTY_USB | OLED_STATUS_DIRTY_NETWORK | \
     OLED_STATUS_DIRTY_TIME)

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t system_indicator;
    uint8_t usb_indicator;
    uint8_t network_indicator;
    uint8_t dirty;
    uint8_t time_dirty;
} oled_status_bar_t;

void oled_status_bar_init(
    oled_status_bar_t *status);

void oled_status_bar_mark_all_dirty(
    oled_status_bar_t *status);

void oled_status_bar_set_time(
    oled_status_bar_t *status,
    uint32_t hours,
    uint32_t minutes);

void oled_status_bar_set_indicators(
    oled_status_bar_t *status,
    oled_status_indicator_t system_indicator,
    oled_status_indicator_t usb_indicator,
    oled_status_indicator_t network_indicator);

void oled_status_bar_render(
    oled_status_bar_t *status,
    mono_fb_t *fb,
    mono_rect_t clip);

int oled_status_bar_self_test(void);

#endif
