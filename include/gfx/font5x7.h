#ifndef GFX_FONT5X7_H
#define GFX_FONT5X7_H

#include <stdint.h>

typedef struct
{
    uint8_t glyph_width;
    uint8_t glyph_height;
    uint8_t advance_x;
    uint8_t advance_y;
} mono_font_metrics_t;

const mono_font_metrics_t *font5x7_metrics(void);
const uint8_t *font5x7_glyph(char c);

#endif
