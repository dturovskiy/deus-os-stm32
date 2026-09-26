#include <stdint.h>
#include "kernel/oled_ui_layout.h"

static oled_ui_layout_t active_layout =
{
    { 0, 0, 128, 9 },
    { 1, 10, 126, 22 }
};

static uint32_t active_layout_revision;

static void oled_ui_layout_set_console(
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height)
{
    if ((active_layout.console_rect.x != x) ||
        (active_layout.console_rect.y != y) ||
        (active_layout.console_rect.width != width) ||
        (active_layout.console_rect.height != height))
    {
        active_layout.console_rect.x = x;
        active_layout.console_rect.y = y;
        active_layout.console_rect.width = width;
        active_layout.console_rect.height = height;
        ++active_layout_revision;
    }
}

const oled_ui_layout_t *oled_ui_layout_active(void)
{
    return &active_layout;
}

uint32_t oled_ui_layout_revision(void)
{
    return active_layout_revision;
}

int oled_ui_layout_config_v1_validate(
    const uint8_t *payload,
    uint32_t length)
{
    if ((payload == (const uint8_t *)0) ||
        (length != OLED_UI_LAYOUT_CONFIG_V1_BYTES))
    {
        return 0;
    }

    return
        (payload[0] == OLED_UI_LAYOUT_CONFIG_V1_SCHEMA) &&
        ((uint8_t)(payload[5] | payload[6] | payload[7]) == 0u) &&
        (payload[3] >= 5u) &&
        (payload[4] >= 6u) &&
        (payload[2] >= 10u) &&
        ((uint16_t)payload[1] + (uint16_t)payload[3] <= 128u) &&
        ((uint16_t)payload[2] + (uint16_t)payload[4] <= 32u);
}

void oled_ui_layout_config_v1_activate_validated(
    const uint8_t *payload)
{
    oled_ui_layout_set_console(
        (int32_t)payload[1],
        (int32_t)payload[2],
        (int32_t)payload[3],
        (int32_t)payload[4]);
}

int oled_ui_layout_config_v1_apply(
    const uint8_t *payload,
    uint32_t length)
{
    if (oled_ui_layout_config_v1_validate(
            payload,
            length) == 0)
    {
        return 0;
    }

    oled_ui_layout_config_v1_activate_validated(payload);
    return 1;
}
