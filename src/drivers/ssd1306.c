#include <stdint.h>
#include "drivers/ssd1306.h"

#define SSD1306_ADDRESS          0x3Cu
#define SSD1306_CONTROL_CMD      0x00u
#define SSD1306_CONTROL_DATA     0x40u
#define SSD1306_CMD_NOP          0xE3u
#define SSD1306_PATTERN_DATA     16u
#define SSD1306_FLUSH_CHUNK_DATA 16u

/*
 * Slice-1 transport seam.
 *
 * I2C1 remains in kernel.c in this refactor so controller extraction can be
 * hardware-validated independently from transport extraction. The SSD1306
 * public API does not expose this dependency.
 */
int i2c1_write(uint8_t address, const uint8_t *data, uint32_t length);

static int ssd1306_command(uint8_t command)
{
    uint8_t payload[2];

    payload[0] = SSD1306_CONTROL_CMD;
    payload[1] = command;

    return i2c1_write(SSD1306_ADDRESS, payload, 2u);
}

int ssd1306_init(void)
{
    static const uint8_t init_packet[] =
    {
        SSD1306_CONTROL_CMD,

        0xAEu,       /* display off */
        0xD5u, 0x80u,/* display clock divide / oscillator */
        0xA8u, 0x1Fu,/* multiplex ratio 1/32 */
        0xD3u, 0x00u,/* display offset */
        0x40u,       /* display start line = 0 */
        0x8Du, 0x14u,/* charge pump on */
        0x20u, 0x00u,/* horizontal addressing mode */
        0xA1u,       /* segment remap */
        0xC8u,       /* COM output scan direction remapped */
        0xDAu, 0x02u,/* sequential COM pins for 128x32 panel */
        0x81u, 0xCFu,/* contrast */
        0xD9u, 0xF1u,/* pre-charge period */
        0xDBu, 0x40u,/* VCOMH deselect level */
        0xA4u,       /* display follows RAM */
        0xA6u        /* normal display */
    };

    return i2c1_write(
        SSD1306_ADDRESS,
        init_packet,
        (uint32_t)sizeof(init_packet)
    );
}

static int ssd1306_set_full_window(void)
{
    static const uint8_t window_packet[] =
    {
        SSD1306_CONTROL_CMD,
        0x21u, 0x00u, 0x7Fu, /* columns 0..127 */
        0x22u, 0x00u, 0x03u  /* pages 0..3 */
    };

    return i2c1_write(
        SSD1306_ADDRESS,
        window_packet,
        (uint32_t)sizeof(window_packet)
    );
}

static int ssd1306_fill_checkerboard(void)
{
    static const uint8_t pattern_packet[] =
    {
        SSD1306_CONTROL_DATA,
        0xAAu, 0x55u, 0xAAu, 0x55u,
        0xAAu, 0x55u, 0xAAu, 0x55u,
        0xAAu, 0x55u, 0xAAu, 0x55u,
        0xAAu, 0x55u, 0xAAu, 0x55u
    };

    uint32_t bytes_remaining = SSD1306_FRAMEBUFFER_BYTES;

    while (bytes_remaining != 0u)
    {
        if (i2c1_write(
                SSD1306_ADDRESS,
                pattern_packet,
                (uint32_t)sizeof(pattern_packet)) == 0)
        {
            return 0;
        }

        bytes_remaining -= SSD1306_PATTERN_DATA;
    }

    return 1;
}

int ssd1306_present_full(const uint8_t *framebuffer)
{
    uint8_t packet[1u + SSD1306_FLUSH_CHUNK_DATA];
    uint32_t offset;
    uint32_t i;

    if (framebuffer == (const uint8_t *)0)
    {
        return 0;
    }

    if (ssd1306_set_full_window() == 0)
    {
        return 0;
    }

    packet[0] = SSD1306_CONTROL_DATA;

    for (offset = 0u;
         offset < SSD1306_FRAMEBUFFER_BYTES;
         offset += SSD1306_FLUSH_CHUNK_DATA)
    {
        for (i = 0u; i < SSD1306_FLUSH_CHUNK_DATA; ++i)
        {
            packet[1u + i] = framebuffer[offset + i];
        }

        if (i2c1_write(
                SSD1306_ADDRESS,
                packet,
                (uint32_t)sizeof(packet)) == 0)
        {
            return 0;
        }
    }

    return 1;
}

int ssd1306_display_on(void)
{
    return ssd1306_command(0xAFu);
}

int ssd1306_ping(void)
{
    return ssd1306_command(SSD1306_CMD_NOP);
}

int ssd1306_show_checkerboard(void)
{
    if (ssd1306_init() == 0)
    {
        return 0;
    }

    if (ssd1306_set_full_window() == 0)
    {
        return 0;
    }

    if (ssd1306_fill_checkerboard() == 0)
    {
        return 0;
    }

    return ssd1306_display_on();
}
