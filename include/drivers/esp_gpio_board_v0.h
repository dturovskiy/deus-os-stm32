#ifndef DRIVERS_ESP_GPIO_BOARD_V0_H
#define DRIVERS_ESP_GPIO_BOARD_V0_H
#include <stdint.h>

/* Proposed physical route only:
 * Blue Pill B0 (PB0) -> ESP GPIO0/BOOT
 * Blue Pill B1 (PB1) -> ESP EN/CH_PD
 * Must use separate 3.3 V pullups and exclusive pin ownership.
 * Not linked into the current STM32 firmware. GPIO output mode is
 * 2 MHz, open-drain, release-high only. NEVER push-pull HIGH.
 */
typedef struct {
    uint8_t qualified; /* remains zero until explicit local board init */
} esp_gpio_board_v0_t;
void esp_gpio_board_v0_reset(esp_gpio_board_v0_t *b);
/* A local privileged board integration MUST verify physical ownership and
 * actual pull-ups before passing owner_qualified=1. Never pass a USB flag. */
int esp_gpio_board_v0_init(esp_gpio_board_v0_t *b,
                           uint8_t owner_qualified);
int esp_gpio_board_v0_gpio0_pull_low(esp_gpio_board_v0_t *b,
                                     uint8_t pull_low);
int esp_gpio_board_v0_en_pull_low(esp_gpio_board_v0_t *b,
                                  uint8_t pull_low);
#endif
