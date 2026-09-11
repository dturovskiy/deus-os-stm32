#include <stdint.h>
#include "gfx/font3x5.h"

/*
 * Each byte is one 3-pixel row.
 * Bit 2 is left, bit 0 is right.
 */
static const uint8_t glyph_0[5] = { 0x7u, 0x5u, 0x5u, 0x5u, 0x7u };
static const uint8_t glyph_1[5] = { 0x2u, 0x6u, 0x2u, 0x2u, 0x7u };
static const uint8_t glyph_2[5] = { 0x7u, 0x1u, 0x7u, 0x4u, 0x7u };
static const uint8_t glyph_3[5] = { 0x7u, 0x1u, 0x7u, 0x1u, 0x7u };
static const uint8_t glyph_4[5] = { 0x5u, 0x5u, 0x7u, 0x1u, 0x1u };
static const uint8_t glyph_5[5] = { 0x7u, 0x4u, 0x7u, 0x1u, 0x7u };
static const uint8_t glyph_6[5] = { 0x7u, 0x4u, 0x7u, 0x5u, 0x7u };
static const uint8_t glyph_7[5] = { 0x7u, 0x1u, 0x2u, 0x2u, 0x2u };
static const uint8_t glyph_8[5] = { 0x7u, 0x5u, 0x7u, 0x5u, 0x7u };
static const uint8_t glyph_9[5] = { 0x7u, 0x5u, 0x7u, 0x1u, 0x7u };
static const uint8_t glyph_colon[5] = { 0x0u, 0x2u, 0x0u, 0x2u, 0x0u };

const uint8_t *font3x5_glyph(char c)
{
    switch (c)
    {
        case '0': return glyph_0;
        case '1': return glyph_1;
        case '2': return glyph_2;
        case '3': return glyph_3;
        case '4': return glyph_4;
        case '5': return glyph_5;
        case '6': return glyph_6;
        case '7': return glyph_7;
        case '8': return glyph_8;
        case '9': return glyph_9;
        case ':': return glyph_colon;
        default: return (const uint8_t *)0;
    }
}
