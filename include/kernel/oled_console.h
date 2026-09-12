#ifndef KERNEL_OLED_CONSOLE_H
#define KERNEL_OLED_CONSOLE_H

#include <stdint.h>
#include "gfx/mono_fb.h"

#define OLED_CONSOLE_COLUMNS 21u
#define OLED_CONSOLE_ROWS     3u

typedef struct
{
    char cells[OLED_CONSOLE_ROWS][OLED_CONSOLE_COLUMNS];
    uint8_t first_row;
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t dirty_rows;
} oled_console_t;

void oled_console_init(oled_console_t *console);
void oled_console_clear(oled_console_t *console);
void oled_console_putc(oled_console_t *console, char c);
void oled_console_write(oled_console_t *console, const char *text);
void oled_console_write_line(oled_console_t *console, const char *text);

int oled_console_scroll_self_test(void);

void oled_console_render(oled_console_t *console,
    mono_fb_t *fb,
    mono_rect_t clip);

#endif
