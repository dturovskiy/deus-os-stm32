#ifndef GFX_TEXT_RENDERER_H
#define GFX_TEXT_RENDERER_H

#include <stdint.h>
#include "gfx/mono_fb.h"

void text_renderer_draw_cell(
    mono_fb_t *fb,
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    char c);

/*
 * Diagnostic equivalence test for the aligned fast path.
 * Returns nonzero only when fast-path output is byte-for-byte identical to
 * the generic reference path across the built-in clipping/overwrite cases.
 */
int text_renderer_fast_path_self_test(void);

#endif
