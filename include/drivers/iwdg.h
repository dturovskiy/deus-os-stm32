#ifndef DRIVERS_IWDG_H
#define DRIVERS_IWDG_H

#include <stdint.h>

#define IWDG_PRESCALER_DIV256 6u

int iwdg_start(
    uint32_t prescaler_code,
    uint32_t reload_value,
    uint32_t spin_limit);

void iwdg_reload(void);
void iwdg_debug_freeze_enable(void);

#endif
