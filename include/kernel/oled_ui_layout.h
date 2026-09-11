#ifndef KERNEL_OLED_UI_LAYOUT_H
#define KERNEL_OLED_UI_LAYOUT_H

#include "gfx/mono_fb.h"

typedef struct
{
    mono_rect_t status_rect;
    mono_rect_t console_rect;
} oled_ui_layout_t;

const oled_ui_layout_t *oled_ui_layout_default(void);

int oled_ui_layout_validate(
    const oled_ui_layout_t *layout);

int oled_ui_layout_self_test(void);

#endif
