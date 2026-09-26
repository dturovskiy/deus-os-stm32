#ifndef KERNEL_OLED_UI_LAYOUT_H
#define KERNEL_OLED_UI_LAYOUT_H

#include <stdint.h>
#include "gfx/mono_fb.h"

#define OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE 0x0001u
#define OLED_UI_LAYOUT_CONFIG_V1_SCHEMA      1u
#define OLED_UI_LAYOUT_CONFIG_V1_BYTES       8u

typedef struct
{
    mono_rect_t status_rect;
    mono_rect_t console_rect;
} oled_ui_layout_t;

const oled_ui_layout_t *oled_ui_layout_default(void);
const oled_ui_layout_t *oled_ui_layout_active(void);
uint32_t oled_ui_layout_revision(void);

int oled_ui_layout_validate(
    const oled_ui_layout_t *layout);

int oled_ui_layout_config_v1_validate(
    const uint8_t *payload,
    uint32_t length);

int oled_ui_layout_config_v1_apply(
    const uint8_t *payload,
    uint32_t length);

void oled_ui_layout_config_v1_activate_validated(
    const uint8_t *payload);

int oled_ui_layout_self_test(void);

#endif
