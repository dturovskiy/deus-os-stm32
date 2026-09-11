#ifndef GFX_FONT3X5_H
#define GFX_FONT3X5_H

#include <stdint.h>

#define FONT3X5_GLYPH_WIDTH  3u
#define FONT3X5_GLYPH_HEIGHT 5u
#define FONT3X5_ADVANCE_X    4u

const uint8_t *font3x5_glyph(char c);

#endif
