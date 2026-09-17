#ifndef DRIVERS_SSD1306_H
#define DRIVERS_SSD1306_H

#include <stdint.h>
#include "gfx/mono_fb.h"

#define SSD1306_WIDTH             128u
#define SSD1306_HEIGHT            32u
#define SSD1306_PAGES             (SSD1306_HEIGHT / 8u)
#define SSD1306_FRAMEBUFFER_BYTES (SSD1306_WIDTH * SSD1306_PAGES)

typedef struct
{
    uint32_t window_payload_bytes;
    uint32_t data_payload_bytes;
    uint32_t i2c_write_count;
    uint32_t presented_pages;
} ssd1306_present_stats_t;

int ssd1306_init(void);
int ssd1306_display_on(void);
int ssd1306_ping(void);
int ssd1306_present_full(const uint8_t *framebuffer);
int ssd1306_present(mono_fb_t *fb);
void ssd1306_present_stats_get(ssd1306_present_stats_t *stats);
int ssd1306_show_checkerboard(void);

#endif
