#ifndef DRIVERS_STM32F103_CLOCK_H
#define DRIVERS_STM32F103_CLOCK_H

#include <stdint.h>

uint32_t stm32f103_clock_init_72mhz(void);
uint32_t stm32f103_reset_flags_capture_and_clear(void);
int stm32f103_reset_was_iwdg(uint32_t reset_flags);

#endif
