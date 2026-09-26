#include <stdint.h>
#include "drivers/flash_persistence.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_CR            REG32(0x40021000u)
#define RCC_CR_HSIRDY     (1u << 1)

#define FLASH_KEYR        REG32(0x40022004u)
#define FLASH_SR          REG32(0x4002200Cu)
#define FLASH_CR          REG32(0x40022010u)
#define FLASH_AR          REG32(0x40022014u)

#define FLASH_KEY1        0x45670123u
#define FLASH_KEY2        0xCDEF89ABu

#define FLASH_SR_BSY      (1u << 0)
#define FLASH_SR_PGERR    (1u << 2)
#define FLASH_SR_WRPRTERR (1u << 4)
#define FLASH_SR_EOP      (1u << 5)

#define FLASH_CR_PG       (1u << 0)
#define FLASH_CR_PER      (1u << 1)
#define FLASH_CR_MER      (1u << 2)
#define FLASH_CR_STRT     (1u << 6)
#define FLASH_CR_LOCK     (1u << 7)

#define FLASH_READY_SPIN_LIMIT 4000000u
#define COREDEBUG_DEMCR         REG32(0xE000EDFCu)
#define COREDEBUG_DEMCR_TRCENA  (1u << 24)
#define DWT_CTRL                REG32(0xE0001000u)
#define DWT_CTRL_CYCCNTENA      (1u << 0)
#define DWT_CYCCNT              REG32(0xE0001004u)

volatile flash_persistence_diagnostics_t
    flash_persistence_diagnostics;

static int flash_address_owned(uint32_t address)
{
    return
        (address >= FLASH_PERSISTENCE_SLOT_A_ADDRESS) &&
        (address < (FLASH_PERSISTENCE_END_ADDRESS - 1u)) &&
        ((address & 1u) == 0u);
}

static int flash_wait_ready(void)
{
    uint32_t spins = FLASH_READY_SPIN_LIMIT;

    while ((FLASH_SR & FLASH_SR_BSY) != 0u)
    {
        if (spins == 0u)
        {
            return 0;
        }

        --spins;
    }

    return 1;
}

static int flash_execute(
    uint32_t address,
    uint16_t value,
    uint32_t operation)
{
    uint32_t start_cycles;
    uint32_t elapsed_cycles;
    uint32_t status;

    if ((RCC_CR & RCC_CR_HSIRDY) == 0u)
    {
        goto fail;
    }

    if (flash_wait_ready() == 0)
    {
        goto fail;
    }

    if ((FLASH_CR & FLASH_CR_LOCK) != 0u)
    {
        FLASH_KEYR = FLASH_KEY1;
        FLASH_KEYR = FLASH_KEY2;

        if ((FLASH_CR & FLASH_CR_LOCK) != 0u)
        {
            goto fail;
        }
    }

    FLASH_SR =
        FLASH_SR_EOP |
        FLASH_SR_PGERR |
        FLASH_SR_WRPRTERR;
    FLASH_CR &= ~(FLASH_CR_PG | FLASH_CR_PER | FLASH_CR_MER);

    ++flash_persistence_diagnostics.attempt_count[operation];
    start_cycles = DWT_CYCCNT;

    if (operation == FLASH_PERSISTENCE_OP_ERASE)
    {
        FLASH_CR |= FLASH_CR_PER;
        FLASH_AR = address;
        FLASH_CR |= FLASH_CR_STRT;
    }
    else
    {
        FLASH_CR |= FLASH_CR_PG;
        *(volatile uint16_t *)(uintptr_t)address = value;
    }

    if (flash_wait_ready() == 0)
    {
        FLASH_CR =
            (FLASH_CR & ~(FLASH_CR_PG | FLASH_CR_PER)) |
            FLASH_CR_LOCK;
        goto fail;
    }

    elapsed_cycles = DWT_CYCCNT - start_cycles;
    flash_persistence_diagnostics.last_bsy_cycles[operation] =
        elapsed_cycles;

    if (elapsed_cycles >
        flash_persistence_diagnostics.max_bsy_cycles[operation])
    {
        flash_persistence_diagnostics.max_bsy_cycles[operation] =
            elapsed_cycles;
    }

    status = FLASH_SR;
    FLASH_CR =
        (FLASH_CR & ~(FLASH_CR_PG | FLASH_CR_PER)) |
        FLASH_CR_LOCK;

    FLASH_SR =
        FLASH_SR_EOP |
        FLASH_SR_PGERR |
        FLASH_SR_WRPRTERR;

    if ((status &
        (FLASH_SR_EOP |
         FLASH_SR_PGERR |
         FLASH_SR_WRPRTERR)) == FLASH_SR_EOP)
    {
        return 1;
    }

fail:
    ++flash_persistence_diagnostics.storage_error_count;
    return 0;
}

void flash_persistence_init(void)
{
    /*
     * Flash is locked by hardware reset, and flash_execute() re-locks after
     * every operation that can unlock it. Timing telemetry must not depend
     * on an external debugger to provide its cycle-counter source.
     */
    COREDEBUG_DEMCR |= COREDEBUG_DEMCR_TRCENA;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

int flash_persistence_erase_page(uint32_t page_address)
{
    uint32_t offset;

    if ((page_address != FLASH_PERSISTENCE_SLOT_A_ADDRESS) &&
        (page_address != FLASH_PERSISTENCE_SLOT_B_ADDRESS))
    {
        return 0;
    }

    if (flash_execute(
            page_address,
            0u,
            FLASH_PERSISTENCE_OP_ERASE) == 0)
    {
        return 0;
    }

    for (offset = 0u;
        offset < FLASH_PERSISTENCE_PAGE_SIZE;
        offset += 2u)
    {
        if (*(const volatile uint16_t *)(uintptr_t)(
                page_address + offset) != 0xFFFFu)
        {
            ++flash_persistence_diagnostics.storage_error_count;
            return 0;
        }
    }

    return 1;
}

int flash_persistence_program_halfword(
    uint32_t address,
    uint16_t value)
{
    volatile uint16_t *location;

    if (flash_address_owned(address) == 0)
    {
        return 0;
    }

    location = (volatile uint16_t *)(uintptr_t)address;

    if (value == 0xFFFFu)
    {
        return (*location == 0xFFFFu) ? 1 : 0;
    }

    if (*location != 0xFFFFu)
    {
        ++flash_persistence_diagnostics.storage_error_count;
        return 0;
    }

    if (flash_execute(
            address,
            value,
            FLASH_PERSISTENCE_OP_PROGRAM) == 0)
    {
        return 0;
    }

    if (*location != value)
    {
        ++flash_persistence_diagnostics.storage_error_count;
        return 0;
    }

    return 1;
}
