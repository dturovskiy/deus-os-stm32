#include <stdint.h>
#include "drivers/status_led.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_APB2ENR REG32(0x40021018u)
#define RCC_IOPCEN  (1u << 4)
#define GPIOC_CRH   REG32(0x40011004u)
#define GPIOC_ODR   REG32(0x4001100Cu)
#define GPIOC_BSRR  REG32(0x40011010u)
#define GPIO_PIN_13   (1u << 13)
#define GPIO_RESET_13 (1u << 29)

void status_led_init(void)
{
    RCC_APB2ENR |= RCC_IOPCEN;
    GPIOC_CRH &= ~(0xFu << 20);
    GPIOC_CRH |= (0x2u << 20);
    status_led_set(0u);
}

void status_led_set(uint32_t on)
{
    GPIOC_BSRR = (on != 0u) ? GPIO_RESET_13 : GPIO_PIN_13;
}

uint32_t status_led_raw_level(void)
{
    return ((GPIOC_ODR & GPIO_PIN_13) != 0u) ? 1u : 0u;
}
