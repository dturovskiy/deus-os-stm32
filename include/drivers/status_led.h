#ifndef DRIVERS_STATUS_LED_H
#define DRIVERS_STATUS_LED_H

#include <stdint.h>

void status_led_init(void);
void status_led_set(uint32_t on);
uint32_t status_led_raw_level(void);

#endif
