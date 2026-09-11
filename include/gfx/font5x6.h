#ifndef GFX_FONT5X6_H
#define GFX_FONT5X6_H

#include <stdint.h>
#include "gfx/font5x7.h"

#define FONT5X6_WIDTH  5u
#define FONT5X6_HEIGHT 6u

const mono_font_metrics_t *font5x6_metrics(void);
const uint8_t *font5x6_glyph(char c);

#endif
