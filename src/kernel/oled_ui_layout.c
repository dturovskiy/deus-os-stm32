#include "kernel/oled_ui_layout.h"

static const oled_ui_layout_t default_layout =
{
    { 0, 0, 128, 9 },
    { 1, 10, 126, 22 }
};

const oled_ui_layout_t *oled_ui_layout_default(void)
{
    return &default_layout;
}

int oled_ui_layout_validate(
    const oled_ui_layout_t *layout)
{
    if (layout == (const oled_ui_layout_t *)0)
    {
        return 0;
    }

    return
        (layout->status_rect.x == 0) &&
        (layout->status_rect.y == 0) &&
        (layout->status_rect.width == 128) &&
        (layout->status_rect.height == 9) &&
        (layout->console_rect.x == 1) &&
        (layout->console_rect.y == 10) &&
        (layout->console_rect.width == 126) &&
        (layout->console_rect.height == 22);
}

int oled_ui_layout_self_test(void)
{
    return oled_ui_layout_validate(
        &default_layout);
}
