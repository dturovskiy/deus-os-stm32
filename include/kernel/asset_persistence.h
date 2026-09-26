#ifndef KERNEL_ASSET_PERSISTENCE_H
#define KERNEL_ASSET_PERSISTENCE_H

#include <stdint.h>

#define ASSET_PERSISTENCE_ENVELOPE_BYTES 64u
#define ASSET_PERSISTENCE_PAYLOAD_MAX    960u
#define ASSET_PERSISTENCE_COMMIT_MARKER  0xA55Au
#define ASSET_PERSISTENCE_GENERATION_MAX 10000u

typedef enum
{
    ASSET_PERSISTENCE_RESULT_OK = 0,
    ASSET_PERSISTENCE_RESULT_UNCHANGED,
    ASSET_PERSISTENCE_RESULT_BAD_STATE,
    ASSET_PERSISTENCE_RESULT_CRC_MISMATCH,
    ASSET_PERSISTENCE_RESULT_VALIDATION_FAILED,
    ASSET_PERSISTENCE_RESULT_STORAGE_ERROR,
    ASSET_PERSISTENCE_RESULT_WEAR_BUDGET_EXHAUSTED
} asset_persistence_result_t;

typedef struct
{
    uint32_t slot_address;
    uint32_t generation;
    uint32_t payload_crc32;
} asset_persistence_record_t;

typedef struct
{
    uint32_t ambiguity_count;
    uint32_t successful_commit_count;
    uint32_t wear_policy_rejection_count;
    uint32_t last_mutation_cycles;
    uint32_t max_mutation_cycles;
} asset_persistence_diagnostics_t;

extern volatile asset_persistence_diagnostics_t
    asset_persistence_diagnostics;

extern volatile uint32_t
    asset_persistence_fault_injection_control;

void asset_persistence_init(void);

const asset_persistence_record_t *
asset_persistence_active_record(void);

int asset_persistence_read_active(
    uint16_t offset,
    uint8_t *destination,
    uint16_t length);

asset_persistence_result_t
asset_persistence_begin_candidate(uint32_t payload_crc32);

asset_persistence_result_t
asset_persistence_write_candidate(
    const uint8_t *data,
    uint16_t length);

asset_persistence_result_t
asset_persistence_commit_candidate(void);

void asset_persistence_abort_candidate(void);

void asset_persistence_fault_checkpoint(uint8_t checkpoint_id);

#endif
