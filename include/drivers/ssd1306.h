#ifndef DRIVERS_SSD1306_H
#define DRIVERS_SSD1306_H

#include <stdint.h>
#include "gfx/mono_fb.h"

#define SSD1306_WIDTH             128u
#define SSD1306_HEIGHT            32u
#define SSD1306_PAGES             (SSD1306_HEIGHT / 8u)
#define SSD1306_FRAMEBUFFER_BYTES (SSD1306_WIDTH * SSD1306_PAGES)

int ssd1306_init(void);
int ssd1306_display_on(void);
int ssd1306_ping(void);
int ssd1306_present_full(const uint8_t *framebuffer);
int ssd1306_present(mono_fb_t *fb);
int ssd1306_show_checkerboard(void);

#endif
