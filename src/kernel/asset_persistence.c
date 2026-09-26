#include <stdint.h>
#include "drivers/flash_persistence.h"
#include "kernel/asset_persistence.h"
#include "kernel/oled_ui_layout.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define SCB_AIRCR             REG32(0xE000ED0Cu)
#define SCB_AIRCR_PRIGROUP    (0x7u << 8)
#define SCB_AIRCR_SYSRESETREQ (1u << 2)
#define SCB_AIRCR_VECTKEY     (0x5FAu << 16)

#define ASSET_RECORD_MAGIC              0x53554544u
#define ASSET_RECORD_ENVELOPE_VERSION   1u
#define ASSET_RECORD_HEADER_BYTES       64u

#define ASSET_RECORD_OFFSET_MAGIC       0x00u
#define ASSET_RECORD_OFFSET_VERSION     0x04u
#define ASSET_RECORD_OFFSET_SCHEMA      0x05u
#define ASSET_RECORD_OFFSET_OBJECT_TYPE 0x06u
#define ASSET_RECORD_OFFSET_HEADER_SIZE 0x08u
#define ASSET_RECORD_OFFSET_LENGTH      0x0Au
#define ASSET_RECORD_OFFSET_GENERATION  0x0Cu
#define ASSET_RECORD_OFFSET_PAYLOAD_CRC 0x10u
#define ASSET_RECORD_OFFSET_FLAGS       0x14u
#define ASSET_RECORD_OFFSET_RESERVED0   0x16u
#define ASSET_RECORD_OFFSET_RESERVED    0x18u
#define ASSET_RECORD_RESERVED_BYTES     32u
#define ASSET_RECORD_OFFSET_HEADER_CRC  0x38u
#define ASSET_RECORD_OFFSET_TAIL        0x3Cu
#define ASSET_RECORD_OFFSET_MARKER      0x3Eu
#define ASSET_RECORD_OFFSET_PAYLOAD     0x40u

#define ASSET_FAULT_ARM_MASK  0xFFFFFF00u
#define ASSET_FAULT_ARM_VALUE 0xA55A0000u

typedef struct
{
    uint32_t slot_address;
    uint32_t generation;
    uint32_t payload_crc32;
    uint16_t accepted_bytes;
    uint8_t pending_byte;
    uint8_t pending_valid;
    uint8_t compare_active;
    uint8_t active;
} asset_candidate_state_t;

volatile asset_persistence_diagnostics_t
    asset_persistence_diagnostics;

volatile uint32_t
    asset_persistence_fault_injection_control;

static asset_persistence_record_t active_record;
static asset_candidate_state_t candidate_state;

static uint8_t asset_flash_read_u8(uint32_t address)
{
    return *(const volatile uint8_t *)(uintptr_t)address;
}

static uint16_t asset_flash_read_u16(uint32_t address)
{
    return *(const volatile uint16_t *)(uintptr_t)address;
}

static uint32_t asset_flash_read_u32(uint32_t address)
{
    return *(const volatile uint32_t *)(uintptr_t)address;
}

/*
 * This translation unit is contractually built with -Os. Keep the update
 * primitive noinline and verify .rodata remains zero in the footprint gate.
 */
static __attribute__((noinline)) uint32_t
asset_crc32_update(uint32_t crc, uint8_t value)
{
    uint32_t bit;

    crc ^= (uint32_t)value;

    for (bit = 0u; bit < 8u; ++bit)
    {
        const uint32_t mask = 0u - (crc & 1u);
        crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }

    return crc;
}

static uint32_t asset_crc32_flash(
    uint32_t address,
    uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t index;

    for (index = 0u; index < length; ++index)
    {
        crc = asset_crc32_update(
            crc,
            asset_flash_read_u8(address + index));
    }

    return crc ^ 0xFFFFFFFFu;
}

static asset_persistence_result_t asset_validate_record(
    uint32_t slot_address,
    uint16_t required_marker,
    asset_persistence_record_t *record_out)
{
    asset_persistence_record_t record;
    uint32_t index;

    if ((asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_MAGIC) != ASSET_RECORD_MAGIC) ||
        (asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_VERSION) !=
            ((uint32_t)ASSET_RECORD_ENVELOPE_VERSION |
             ((uint32_t)OLED_UI_LAYOUT_CONFIG_V1_SCHEMA << 8) |
             ((uint32_t)OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE << 16))) ||
        (asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_HEADER_SIZE) !=
            ((uint32_t)ASSET_RECORD_HEADER_BYTES |
             ((uint32_t)OLED_UI_LAYOUT_CONFIG_V1_BYTES << 16))) ||
        (asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_FLAGS) != 0u) ||
        (asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_TAIL) !=
            (0x0000FFFFu | ((uint32_t)required_marker << 16))))
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    for (index = 0u;
        index < (ASSET_RECORD_RESERVED_BYTES / 4u);
        ++index)
    {
        if (asset_flash_read_u32(
                slot_address +
                ASSET_RECORD_OFFSET_RESERVED +
                (index * 4u)) != 0xFFFFFFFFu)
        {
            return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
        }
    }

    record.slot_address = slot_address;
    record.generation =
        asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_GENERATION);
    record.payload_crc32 =
        asset_flash_read_u32(
            slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD_CRC);

    if ((record.generation == 0u) ||
        (record.generation > ASSET_PERSISTENCE_GENERATION_MAX) ||
        (asset_crc32_flash(slot_address, 24u) !=
            asset_flash_read_u32(
                slot_address +
                ASSET_RECORD_OFFSET_HEADER_CRC)))
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    if (record.payload_crc32 !=
        asset_crc32_flash(
            slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD,
            OLED_UI_LAYOUT_CONFIG_V1_BYTES))
    {
        return ASSET_PERSISTENCE_RESULT_CRC_MISMATCH;
    }

    if (oled_ui_layout_config_v1_validate(
            (const uint8_t *)(uintptr_t)(
                slot_address +
                ASSET_RECORD_OFFSET_PAYLOAD),
            OLED_UI_LAYOUT_CONFIG_V1_BYTES) == 0)
    {
        return ASSET_PERSISTENCE_RESULT_VALIDATION_FAILED;
    }

    if (record_out != (asset_persistence_record_t *)0)
    {
        *record_out = record;
    }

    return ASSET_PERSISTENCE_RESULT_OK;
}

static int asset_records_identical(
    const asset_persistence_record_t *left,
    const asset_persistence_record_t *right)
{
    if ((left->generation != right->generation) ||
        (left->payload_crc32 != right->payload_crc32))
    {
        return 0;
    }

    return
        (asset_flash_read_u32(
            left->slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD) ==
         asset_flash_read_u32(
            right->slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD)) &&
        (asset_flash_read_u32(
            left->slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD + 4u) ==
         asset_flash_read_u32(
            right->slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD + 4u));
}

static void asset_clear_active(void)
{
    active_record.slot_address = 0u;
}

static void asset_candidate_reset(void)
{
    candidate_state.active = 0u;
}

static void asset_select_boot_record(void)
{
    asset_persistence_record_t slot_a;
    asset_persistence_record_t slot_b;
    const int valid_a =
        asset_validate_record(
            FLASH_PERSISTENCE_SLOT_A_ADDRESS,
            ASSET_PERSISTENCE_COMMIT_MARKER,
            &slot_a) == ASSET_PERSISTENCE_RESULT_OK;
    const int valid_b =
        asset_validate_record(
            FLASH_PERSISTENCE_SLOT_B_ADDRESS,
            ASSET_PERSISTENCE_COMMIT_MARKER,
            &slot_b) == ASSET_PERSISTENCE_RESULT_OK;

    asset_clear_active();

    if ((valid_a == 0) && (valid_b == 0))
    {
        return;
    }

    if ((valid_a != 0) && (valid_b == 0))
    {
        active_record = slot_a;
    }
    else if ((valid_a == 0) && (valid_b != 0))
    {
        active_record = slot_b;
    }
    else if (slot_a.generation > slot_b.generation)
    {
        active_record = slot_a;
    }
    else if (slot_b.generation > slot_a.generation)
    {
        active_record = slot_b;
    }
    else
    {
        ++asset_persistence_diagnostics.ambiguity_count;

        if (asset_records_identical(&slot_a, &slot_b) == 0)
        {
            return;
        }

        active_record = slot_a;
    }

    oled_ui_layout_config_v1_activate_validated(
        (const uint8_t *)(uintptr_t)(
            active_record.slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD));

}

void asset_persistence_fault_checkpoint(uint8_t checkpoint_id)
{
    const uint32_t control =
        asset_persistence_fault_injection_control;

    if (((control & ASSET_FAULT_ARM_MASK) !=
            ASSET_FAULT_ARM_VALUE) ||
        ((uint8_t)(control & 0xFFu) != checkpoint_id))
    {
        return;
    }

    asset_persistence_fault_injection_control = 0u;
    __asm volatile ("dsb" ::: "memory");

    SCB_AIRCR =
        SCB_AIRCR_VECTKEY |
        (SCB_AIRCR & SCB_AIRCR_PRIGROUP) |
        SCB_AIRCR_SYSRESETREQ;

    __asm volatile ("dsb" ::: "memory");

    for (;;)
    {
    }
}

void asset_persistence_init(void)
{
    flash_persistence_init();
    asset_select_boot_record();
}

const asset_persistence_record_t *
asset_persistence_active_record(void)
{
    return
        (active_record.slot_address != 0u) ?
        &active_record :
        (const asset_persistence_record_t *)0;
}

int asset_persistence_read_active(
    uint16_t offset,
    uint8_t *destination,
    uint16_t length)
{
    uint16_t index;

    if ((active_record.slot_address == 0u) ||
        ((destination == (uint8_t *)0) && (length != 0u)) ||
        (offset > OLED_UI_LAYOUT_CONFIG_V1_BYTES) ||
        (length >
            (uint16_t)(OLED_UI_LAYOUT_CONFIG_V1_BYTES - offset)))
    {
        return 0;
    }

    for (index = 0u; index < length; ++index)
    {
        destination[index] =
            asset_flash_read_u8(
                active_record.slot_address +
                ASSET_RECORD_OFFSET_PAYLOAD +
                offset +
                index);
    }

    return 1;
}

static asset_persistence_result_t asset_program_metadata(void)
{
    uint16_t words[12];
    uint32_t index;

    words[0] = 0x4544u;
    words[1] = 0x5355u;
    words[2] =
        (uint16_t)(
            ASSET_RECORD_ENVELOPE_VERSION |
            ((uint16_t)OLED_UI_LAYOUT_CONFIG_V1_SCHEMA << 8));
    words[3] = OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE;
    words[4] = ASSET_RECORD_HEADER_BYTES;
    words[5] = OLED_UI_LAYOUT_CONFIG_V1_BYTES;
    words[6] = (uint16_t)(candidate_state.generation & 0xFFFFu);
    words[7] = (uint16_t)(candidate_state.generation >> 16);
    words[8] = (uint16_t)(candidate_state.payload_crc32 & 0xFFFFu);
    words[9] = (uint16_t)(candidate_state.payload_crc32 >> 16);
    words[10] = 0u;
    words[11] = 0u;

    for (index = 0u; index < 12u; ++index)
    {
        if (flash_persistence_program_halfword(
                candidate_state.slot_address +
                (index * 2u),
                words[index]) == 0)
        {
            return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
        }
    }

    return ASSET_PERSISTENCE_RESULT_OK;
}

static asset_persistence_result_t asset_start_changed(void)
{
    asset_persistence_result_t result;

    if ((active_record.slot_address != 0u) &&
        (active_record.generation >=
            ASSET_PERSISTENCE_GENERATION_MAX))
    {
        ++asset_persistence_diagnostics.wear_policy_rejection_count;
        return ASSET_PERSISTENCE_RESULT_WEAR_BUDGET_EXHAUSTED;
    }

    candidate_state.slot_address =
        ((active_record.slot_address != 0u) &&
         (active_record.slot_address ==
            FLASH_PERSISTENCE_SLOT_A_ADDRESS)) ?
        FLASH_PERSISTENCE_SLOT_B_ADDRESS :
        FLASH_PERSISTENCE_SLOT_A_ADDRESS;
    candidate_state.generation =
        (active_record.slot_address != 0u) ?
        active_record.generation + 1u :
        1u;
    candidate_state.accepted_bytes = 0u;
    candidate_state.pending_valid = 0u;
    candidate_state.compare_active = 0u;
    asset_persistence_diagnostics.last_mutation_cycles =
        flash_persistence_cycle_count();

    asset_persistence_fault_checkpoint(1u);

    if (flash_persistence_erase_page(
            candidate_state.slot_address) == 0)
    {
        asset_candidate_reset();
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    asset_persistence_fault_checkpoint(2u);

    result = asset_program_metadata();
    if (result != ASSET_PERSISTENCE_RESULT_OK)
    {
        asset_candidate_reset();
        return result;
    }

    asset_persistence_fault_checkpoint(3u);
    return ASSET_PERSISTENCE_RESULT_OK;
}

asset_persistence_result_t
asset_persistence_begin_candidate(uint32_t payload_crc32)
{
    if (candidate_state.active != 0u)
    {
        return ASSET_PERSISTENCE_RESULT_BAD_STATE;
    }

    candidate_state.payload_crc32 = payload_crc32;
    candidate_state.active = 1u;

    if ((active_record.slot_address != 0u) &&
        (active_record.payload_crc32 == payload_crc32))
    {
        candidate_state.accepted_bytes = 0u;
        candidate_state.pending_valid = 0u;
        candidate_state.compare_active = 1u;
        return ASSET_PERSISTENCE_RESULT_OK;
    }

    return asset_start_changed();
}

static asset_persistence_result_t
asset_program_payload_halfword(
    uint16_t offset,
    uint16_t value)
{
    if (flash_persistence_program_halfword(
            candidate_state.slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD +
            offset,
            value) == 0)
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    if (offset == 0u)
    {
        asset_persistence_fault_checkpoint(4u);
    }

    return ASSET_PERSISTENCE_RESULT_OK;
}

static asset_persistence_result_t asset_write_payload_bytes(
    const uint8_t *data,
    uint16_t length)
{
    uint16_t index = 0u;

    if ((candidate_state.pending_valid != 0u) && (length != 0u))
    {
        const uint16_t offset =
            (uint16_t)(candidate_state.accepted_bytes - 1u);
        const asset_persistence_result_t result =
            asset_program_payload_halfword(
                offset,
                (uint16_t)candidate_state.pending_byte |
                (uint16_t)((uint16_t)data[0] << 8));

        if (result != ASSET_PERSISTENCE_RESULT_OK)
        {
            return result;
        }

        candidate_state.pending_valid = 0u;
        ++candidate_state.accepted_bytes;
        index = 1u;
    }

    while ((uint16_t)(index + 1u) < length)
    {
        const uint16_t offset = candidate_state.accepted_bytes;
        const asset_persistence_result_t result =
            asset_program_payload_halfword(
                offset,
                (uint16_t)data[index] |
                (uint16_t)((uint16_t)data[index + 1u] << 8));

        if (result != ASSET_PERSISTENCE_RESULT_OK)
        {
            return result;
        }

        candidate_state.accepted_bytes =
            (uint16_t)(candidate_state.accepted_bytes + 2u);
        index = (uint16_t)(index + 2u);
    }

    if (index < length)
    {
        candidate_state.pending_byte = data[index];
        candidate_state.pending_valid = 1u;
        ++candidate_state.accepted_bytes;
    }

    return ASSET_PERSISTENCE_RESULT_OK;
}

asset_persistence_result_t
asset_persistence_write_candidate(
    const uint8_t *data,
    uint16_t length)
{
    uint16_t index;

    if ((candidate_state.active == 0u) ||
        ((data == (const uint8_t *)0) && (length != 0u)) ||
        ((uint32_t)candidate_state.accepted_bytes + length >
            OLED_UI_LAYOUT_CONFIG_V1_BYTES))
    {
        return ASSET_PERSISTENCE_RESULT_BAD_STATE;
    }

    if (candidate_state.compare_active != 0u)
    {
        for (index = 0u; index < length; ++index)
        {
            const uint16_t logical_offset =
                (uint16_t)(candidate_state.accepted_bytes + index);

            if (asset_flash_read_u8(
                    active_record.slot_address +
                    ASSET_RECORD_OFFSET_PAYLOAD +
                    logical_offset) != data[index])
            {
                uint16_t prefix = 0u;
                asset_persistence_result_t result =
                    asset_start_changed();

                if (result != ASSET_PERSISTENCE_RESULT_OK)
                {
                    return result;
                }

                while ((uint16_t)(prefix + 1u) < logical_offset)
                {
                    result = asset_program_payload_halfword(
                        prefix,
                        asset_flash_read_u16(
                            active_record.slot_address +
                            ASSET_RECORD_OFFSET_PAYLOAD +
                            prefix));

                    if (result != ASSET_PERSISTENCE_RESULT_OK)
                    {
                        asset_candidate_reset();
                        return result;
                    }

                    candidate_state.accepted_bytes =
                        (uint16_t)(
                            candidate_state.accepted_bytes + 2u);
                    prefix = (uint16_t)(prefix + 2u);
                }

                if (prefix < logical_offset)
                {
                    candidate_state.pending_byte =
                        asset_flash_read_u8(
                            active_record.slot_address +
                            ASSET_RECORD_OFFSET_PAYLOAD +
                            prefix);
                    candidate_state.pending_valid = 1u;
                    ++candidate_state.accepted_bytes;
                }

                return asset_write_payload_bytes(
                    &data[index],
                    (uint16_t)(length - index));
            }
        }

        candidate_state.accepted_bytes =
            (uint16_t)(candidate_state.accepted_bytes + length);
        return ASSET_PERSISTENCE_RESULT_OK;
    }

    return asset_write_payload_bytes(data, length);
}

static asset_persistence_result_t asset_program_header_crc(void)
{
    const uint32_t header_crc =
        asset_crc32_flash(
            candidate_state.slot_address,
            24u);

    if ((flash_persistence_program_halfword(
            candidate_state.slot_address +
                ASSET_RECORD_OFFSET_HEADER_CRC,
            (uint16_t)(header_crc & 0xFFFFu)) == 0) ||
        (flash_persistence_program_halfword(
            candidate_state.slot_address +
                ASSET_RECORD_OFFSET_HEADER_CRC +
                2u,
            (uint16_t)(header_crc >> 16)) == 0))
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    return ASSET_PERSISTENCE_RESULT_OK;
}

asset_persistence_result_t
asset_persistence_commit_candidate(void)
{
    asset_persistence_record_t committed;
    asset_persistence_result_t result;
    uint32_t elapsed_cycles;

    if ((candidate_state.active == 0u) ||
        (candidate_state.accepted_bytes !=
            OLED_UI_LAYOUT_CONFIG_V1_BYTES))
    {
        return ASSET_PERSISTENCE_RESULT_BAD_STATE;
    }

    if (candidate_state.compare_active != 0u)
    {
        asset_candidate_reset();
        return ASSET_PERSISTENCE_RESULT_UNCHANGED;
    }

    if (candidate_state.pending_valid != 0u)
    {
        return ASSET_PERSISTENCE_RESULT_BAD_STATE;
    }

    asset_persistence_fault_checkpoint(5u);

    result = asset_program_header_crc();
    if (result != ASSET_PERSISTENCE_RESULT_OK)
    {
        return result;
    }

    result = asset_validate_record(
        candidate_state.slot_address,
        0xFFFFu,
        (asset_persistence_record_t *)0);
    if (result != ASSET_PERSISTENCE_RESULT_OK)
    {
        return result;
    }

    asset_persistence_fault_checkpoint(6u);

    if (flash_persistence_program_halfword(
            candidate_state.slot_address +
                ASSET_RECORD_OFFSET_MARKER,
            ASSET_PERSISTENCE_COMMIT_MARKER) == 0)
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    if (asset_flash_read_u16(
            candidate_state.slot_address +
                ASSET_RECORD_OFFSET_MARKER) !=
        ASSET_PERSISTENCE_COMMIT_MARKER)
    {
        return ASSET_PERSISTENCE_RESULT_STORAGE_ERROR;
    }

    asset_persistence_fault_checkpoint(7u);

    result = asset_validate_record(
        candidate_state.slot_address,
        ASSET_PERSISTENCE_COMMIT_MARKER,
        &committed);
    if (result != ASSET_PERSISTENCE_RESULT_OK)
    {
        return result;
    }

    asset_persistence_fault_checkpoint(8u);

    active_record = committed;

    oled_ui_layout_config_v1_activate_validated(
        (const uint8_t *)(uintptr_t)(
            active_record.slot_address +
            ASSET_RECORD_OFFSET_PAYLOAD));

    elapsed_cycles =
        flash_persistence_cycle_count() -
        asset_persistence_diagnostics.last_mutation_cycles;
    asset_persistence_diagnostics.last_mutation_cycles =
        elapsed_cycles;
    if (elapsed_cycles >
        asset_persistence_diagnostics.max_mutation_cycles)
    {
        asset_persistence_diagnostics.max_mutation_cycles =
            elapsed_cycles;
    }
    ++asset_persistence_diagnostics.successful_commit_count;
    asset_candidate_reset();

    asset_persistence_fault_checkpoint(9u);
    return ASSET_PERSISTENCE_RESULT_OK;
}

void asset_persistence_abort_candidate(void)
{
    asset_candidate_reset();
}
