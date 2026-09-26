#ifndef DRIVERS_FLASH_PERSISTENCE_H
#define DRIVERS_FLASH_PERSISTENCE_H

#include <stdint.h>

#define FLASH_PERSISTENCE_PAGE_SIZE       1024u
#define FLASH_PERSISTENCE_SLOT_A_ADDRESS  0x0800F800u
#define FLASH_PERSISTENCE_SLOT_B_ADDRESS  0x0800FC00u
#define FLASH_PERSISTENCE_END_ADDRESS     0x08010000u

#define FLASH_PERSISTENCE_OP_ERASE        0u
#define FLASH_PERSISTENCE_OP_PROGRAM      1u
#define FLASH_PERSISTENCE_OP_COUNT        2u

typedef struct
{
    uint32_t attempt_count[FLASH_PERSISTENCE_OP_COUNT];
    uint32_t last_bsy_cycles[FLASH_PERSISTENCE_OP_COUNT];
    uint32_t max_bsy_cycles[FLASH_PERSISTENCE_OP_COUNT];
    uint32_t storage_error_count;
} flash_persistence_diagnostics_t;

extern volatile flash_persistence_diagnostics_t
    flash_persistence_diagnostics;

void flash_persistence_init(void);

static inline uint32_t flash_persistence_cycle_count(void)
{
    return *(const volatile uint32_t *)(uintptr_t)0xE0001004u;
}

int flash_persistence_erase_page(uint32_t page_address);

int flash_persistence_program_halfword(
    uint32_t address,
    uint16_t value);

#endif
