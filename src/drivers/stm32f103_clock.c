#include <stdint.h>
#include "drivers/stm32f103_clock.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define FLASH_ACR        REG32(0x40022000u)
#define FLASH_LATENCY_2  0x2u
#define FLASH_PRFTBE     (1u << 4)

#define RCC_CR           REG32(0x40021000u)
#define RCC_CFGR         REG32(0x40021004u)
#define RCC_CSR          REG32(0x40021024u)

#define RCC_HSEON        (1u << 16)
#define RCC_HSERDY       (1u << 17)
#define RCC_PLLON        (1u << 24)
#define RCC_PLLRDY       (1u << 25)
#define RCC_SW_MASK      (0x3u << 0)
#define RCC_SW_PLL       (0x2u << 0)
#define RCC_SWS_MASK     (0x3u << 2)
#define RCC_SWS_PLL      (0x2u << 2)
#define RCC_PPRE1_DIV2   (0x4u << 8)
#define RCC_ADCPRE_DIV6  (0x2u << 14)
#define RCC_PLLSRC_HSE   (1u << 16)
#define RCC_PLLMUL_X9    (0x7u << 18)
#define RCC_USBPRE       (1u << 22)
#define RCC_CSR_RMVF     (1u << 24)
#define RCC_CSR_IWDGRSTF (1u << 29)

#define STM32F103_CORE_CLOCK_HZ 72000000u
#define CLOCK_READY_SPIN_LIMIT 1000000u

static int wait_mask_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t remaining = CLOCK_READY_SPIN_LIMIT;

    while ((*reg & mask) == 0u)
    {
        if (remaining == 0u)
        {
            return 0;
        }
        --remaining;
    }

    return 1;
}

uint32_t stm32f103_clock_init_72mhz(void)
{
    uint32_t remaining;

    FLASH_ACR = FLASH_PRFTBE | FLASH_LATENCY_2;

    RCC_CR |= RCC_HSEON;
    if (wait_mask_set(&RCC_CR, RCC_HSERDY) == 0)
    {
        return 0u;
    }

    RCC_CFGR =
        RCC_PPRE1_DIV2 |
        RCC_ADCPRE_DIV6 |
        RCC_PLLSRC_HSE |
        RCC_PLLMUL_X9;

    RCC_CR |= RCC_PLLON;
    if (wait_mask_set(&RCC_CR, RCC_PLLRDY) == 0)
    {
        return 0u;
    }

    RCC_CFGR = (RCC_CFGR & ~RCC_SW_MASK) | RCC_SW_PLL;
    remaining = CLOCK_READY_SPIN_LIMIT;
    while ((RCC_CFGR & RCC_SWS_MASK) != RCC_SWS_PLL)
    {
        if (remaining == 0u)
        {
            return 0u;
        }
        --remaining;
    }

    RCC_CFGR &= ~RCC_USBPRE;
    return STM32F103_CORE_CLOCK_HZ;
}

uint32_t stm32f103_reset_flags_capture_and_clear(void)
{
    const uint32_t flags = RCC_CSR;
    RCC_CSR |= RCC_CSR_RMVF;
    return flags;
}

int stm32f103_reset_was_iwdg(uint32_t reset_flags)
{
    return (reset_flags & RCC_CSR_IWDGRSTF) != 0u;
}
