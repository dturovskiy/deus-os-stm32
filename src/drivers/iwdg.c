#include <stdint.h>
#include "drivers/iwdg.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

#define RCC_CSR              REG32(0x40021024u)
#define RCC_CSR_LSION        (1u << 0)
#define RCC_CSR_LSIRDY       (1u << 1)

#define IWDG_KR              REG32(0x40003000u)
#define IWDG_PR              REG32(0x40003004u)
#define IWDG_RLR             REG32(0x40003008u)
#define IWDG_SR              REG32(0x4000300Cu)

#define IWDG_KR_ENABLE_WRITE 0x5555u
#define IWDG_KR_RELOAD       0xAAAAu
#define IWDG_KR_START        0xCCCCu

#define IWDG_SR_PVU          (1u << 0)
#define IWDG_SR_RVU          (1u << 1)
#define IWDG_SR_UPDATE_MASK  (IWDG_SR_PVU | IWDG_SR_RVU)

#define IWDG_PRESCALER_MAX_CODE 6u
#define IWDG_RELOAD_MAX         0x0FFFu

#define DBGMCU_CR             REG32(0xE0042004u)
#define DBGMCU_CR_IWDG_STOP   (1u << 8)

int iwdg_start(
    uint32_t prescaler_code,
    uint32_t reload_value,
    uint32_t spin_limit)
{
    uint32_t spins;

    if (
        (prescaler_code > IWDG_PRESCALER_MAX_CODE) ||
        (reload_value > IWDG_RELOAD_MAX) ||
        (spin_limit == 0u)
    ) {
        return 0;
    }

    RCC_CSR |= RCC_CSR_LSION;

    spins = spin_limit;
    while ((RCC_CSR & RCC_CSR_LSIRDY) == 0u)
    {
        --spins;

        if (spins == 0u)
        {
            return 0;
        }
    }

    /*
     * STM32F1: start IWDG before programming PR/RLR.
     * PVU/RVU synchronization is driven by the running IWDG/LSI domain.
     */
    IWDG_KR = IWDG_KR_START;
    IWDG_KR = IWDG_KR_ENABLE_WRITE;
    IWDG_PR = prescaler_code;
    IWDG_RLR = reload_value;

    spins = spin_limit;
    while ((IWDG_SR & IWDG_SR_UPDATE_MASK) != 0u)
    {
        --spins;

        if (spins == 0u)
        {
            return 0;
        }
    }

    IWDG_KR = IWDG_KR_RELOAD;

    return 1;
}

void iwdg_reload(void)
{
    IWDG_KR = IWDG_KR_RELOAD;
}

void iwdg_debug_freeze_enable(void)
{
    DBGMCU_CR |= DBGMCU_CR_IWDG_STOP;
}
