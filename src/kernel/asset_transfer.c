#include <stdint.h>
#include "kernel/asset_persistence.h"
#include "kernel/asset_transfer.h"
#include "kernel/oled_ui_layout.h"
#include "kernel/time.h"

#define ASSET_TRANSFER_FLAG_ALLOW_DESTRUCTIVE 0x01u

#define ASSET_TRANSFER_OPCODE_BEGIN       0x01u
#define ASSET_TRANSFER_OPCODE_WRITE_CHUNK 0x02u
#define ASSET_TRANSFER_OPCODE_COMMIT      0x03u
#define ASSET_TRANSFER_OPCODE_ABORT       0x04u
#define ASSET_TRANSFER_OPCODE_STATUS      0x05u
#define ASSET_TRANSFER_OPCODE_READ_CHUNK  0x06u

#define ASSET_TRANSFER_CURRENT_LENGTH OLED_UI_LAYOUT_CONFIG_V1_BYTES
#define ASSET_TRANSFER_RETRY_CACHE_MAX OLED_UI_LAYOUT_CONFIG_V1_BYTES

typedef struct
{
    kernel_time_ms_t last_activity_ms;
    uint32_t payload_crc32;
    uint16_t transfer_id;
    uint16_t next_offset;
    uint8_t last_chunk_length;
    uint8_t active;
    uint8_t last_chunk[ASSET_TRANSFER_RETRY_CACHE_MAX];
} asset_transfer_state_t;

typedef union
{
    uint8_t bytes[ASSET_TRANSFER_RESPONSE_WIRE_MAX];
    uint16_t halfwords[ASSET_TRANSFER_RESPONSE_WIRE_MAX / 2u];
} asset_transfer_wire_t;

static asset_transfer_state_t asset_transfer_state;
static asset_transfer_wire_t asset_transfer_wire;

_Static_assert(
    ASSET_TRANSFER_RESPONSE_WIRE_MAX <= 64u,
    "asset transfer response must fit one management IN packet");
_Static_assert(
    ASSET_TRANSFER_CURRENT_LENGTH <= ASSET_TRANSFER_CHUNK_MAX,
    "current consumer must fit one max transfer chunk");

static uint16_t asset_get_u16(const uint8_t *source)
{
    return
        (uint16_t)source[0] |
        (uint16_t)((uint16_t)source[1] << 8);
}

static uint32_t asset_get_u32(const uint8_t *source)
{
    return
        (uint32_t)source[0] |
        ((uint32_t)source[1] << 8) |
        ((uint32_t)source[2] << 16) |
        ((uint32_t)source[3] << 24);
}

static asset_transfer_session_state_t asset_session_state(void)
{
    if (asset_transfer_state.active == 0u)
    {
        return ASSET_TRANSFER_SESSION_NONE;
    }

    return
        (asset_transfer_state.next_offset ==
            ASSET_TRANSFER_CURRENT_LENGTH) ?
        ASSET_TRANSFER_SESSION_COMPLETE_UNCOMMITTED :
        ASSET_TRANSFER_SESSION_RECEIVING;
}

static void asset_clear_session(void)
{
    asset_persistence_abort_candidate();
    asset_transfer_state.transfer_id = 0u;
    asset_transfer_state.active = 0u;
}

static void asset_finish_commit(void)
{
    asset_transfer_state.active = 0u;
}

void asset_transfer_init(void)
{
    asset_clear_session();
}

void asset_transfer_reset_session(void)
{
    asset_clear_session();
}

void asset_transfer_poll(void)
{
    if ((asset_transfer_state.active != 0u) &&
        (kernel_time_elapsed(
            asset_transfer_state.last_activity_ms,
            ASSET_TRANSFER_IDLE_TIMEOUT_MS) != 0))
    {
        asset_clear_session();
    }
}

static asset_transfer_status_t
asset_map_persistence_result(asset_persistence_result_t result)
{
    switch (result)
    {
        case ASSET_PERSISTENCE_RESULT_OK:
        case ASSET_PERSISTENCE_RESULT_UNCHANGED:
            return ASSET_TRANSFER_STATUS_OK;
        case ASSET_PERSISTENCE_RESULT_CRC_MISMATCH:
            return ASSET_TRANSFER_STATUS_CRC_MISMATCH;
        case ASSET_PERSISTENCE_RESULT_VALIDATION_FAILED:
            return ASSET_TRANSFER_STATUS_VALIDATION_FAILED;
        case ASSET_PERSISTENCE_RESULT_STORAGE_ERROR:
            return ASSET_TRANSFER_STATUS_STORAGE_ERROR;
        case ASSET_PERSISTENCE_RESULT_WEAR_BUDGET_EXHAUSTED:
            return ASSET_TRANSFER_STATUS_WEAR_BUDGET_EXHAUSTED;
        case ASSET_PERSISTENCE_RESULT_BAD_STATE:
        default:
            return ASSET_TRANSFER_STATUS_BAD_STATE;
    }
}

static int asset_send_response(
    asset_transfer_send_wire_t send_wire,
    void *send_context,
    uint16_t request_id,
    uint8_t opcode,
    asset_transfer_status_t status,
    uint16_t object_type,
    uint16_t transfer_id,
    uint16_t total_length,
    uint16_t next_offset,
    uint8_t data_length)
{
    const asset_persistence_record_t *record =
        asset_persistence_active_record();
    const uint32_t committed_generation =
        (record != (const asset_persistence_record_t *)0) ?
        record->generation :
        0u;
    const uint16_t payload_length =
        (uint16_t)(18u + data_length);
    const uint32_t wire_length =
        BINARY_FRAME_FIXED_PREFIX_BYTES +
        (uint32_t)payload_length +
        BINARY_FRAME_CRC_BYTES;
    const uint16_t crc_offset =
        (uint16_t)(
            BINARY_FRAME_FIXED_PREFIX_BYTES +
            payload_length);
    uint16_t crc;

    asset_transfer_wire.halfwords[0] =
        (uint16_t)BINARY_FRAME_MAGIC0 |
        (uint16_t)((uint16_t)BINARY_FRAME_MAGIC1 << 8);
    asset_transfer_wire.halfwords[1] =
        (uint16_t)BINARY_FRAME_PROTOCOL_VERSION |
        (uint16_t)(
            (uint16_t)BINARY_FRAME_TYPE_ASSET_TRANSFER_RESPONSE <<
            8);
    asset_transfer_wire.halfwords[2] = 0u;
    asset_transfer_wire.halfwords[3] = request_id;
    asset_transfer_wire.halfwords[4] = payload_length;
    asset_transfer_wire.halfwords[5] =
        (uint16_t)ASSET_TRANSFER_PROTOCOL_VERSION |
        (uint16_t)((uint16_t)opcode << 8);
    asset_transfer_wire.halfwords[6] =
        (uint16_t)status |
        (uint16_t)((uint16_t)asset_session_state() << 8);
    asset_transfer_wire.halfwords[7] = object_type;
    asset_transfer_wire.halfwords[8] = transfer_id;
    asset_transfer_wire.halfwords[9] =
        (uint16_t)(committed_generation & 0xFFFFu);
    asset_transfer_wire.halfwords[10] =
        (uint16_t)(committed_generation >> 16);
    asset_transfer_wire.halfwords[11] = total_length;
    asset_transfer_wire.halfwords[12] = next_offset;
    asset_transfer_wire.halfwords[13] = data_length;

    crc = binary_frame_crc16_ccitt_false(
        &asset_transfer_wire.bytes[2],
        8u + (uint32_t)payload_length);
    asset_transfer_wire.bytes[crc_offset] =
        (uint8_t)(crc & 0xFFu);
    asset_transfer_wire.bytes[crc_offset + 1u] =
        (uint8_t)(crc >> 8);

    return send_wire(
        send_context,
        asset_transfer_wire.bytes,
        wire_length);
}

static int asset_send_simple(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context,
    uint8_t opcode,
    asset_transfer_status_t status,
    uint16_t transfer_id,
    uint16_t total_length,
    uint16_t next_offset)
{
    return asset_send_response(
        send_wire,
        send_context,
        frame->request_id,
        opcode,
        status,
        OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE,
        transfer_id,
        total_length,
        next_offset,
        0u);
}

static asset_transfer_status_t
asset_write_candidate(const uint8_t *data, uint8_t data_length)
{
    return asset_map_persistence_result(
        asset_persistence_write_candidate(data, data_length));
}

static int asset_last_chunk_equal(
    const uint8_t *data,
    uint8_t data_length)
{
    uint8_t index;

    for (index = 0u; index < data_length; ++index)
    {
        if (asset_transfer_state.last_chunk[index] != data[index])
        {
            return 0;
        }
    }

    return 1;
}

static void asset_remember_chunk(
    const uint8_t *data,
    uint8_t data_length)
{
    uint8_t index;

    asset_transfer_state.last_chunk_length = data_length;

    for (index = 0u; index < data_length; ++index)
    {
        asset_transfer_state.last_chunk[index] = data[index];
    }
}

static int asset_last_commit_matches(uint16_t transfer_id)
{
    const asset_persistence_record_t *record =
        asset_persistence_active_record();

    return
        (record != (const asset_persistence_record_t *)0) &&
        (asset_transfer_state.transfer_id == transfer_id) &&
        (asset_transfer_state.transfer_id != 0u) &&
        (record->payload_crc32 == asset_transfer_state.payload_crc32);
}

static int asset_handle_begin(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint16_t transfer_id = 0u;
    uint16_t total_length = 0u;
    uint8_t schema_version = 0u;
    uint32_t payload_crc32 = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;

    {
        object_type = asset_get_u16(&frame->payload[2]);
        transfer_id = asset_get_u16(&frame->payload[4]);
        total_length = asset_get_u16(&frame->payload[6]);
        schema_version = frame->payload[8];
        payload_crc32 = asset_get_u32(&frame->payload[10]);

        if ((frame->payload[9] != 0u) ||
            (frame->payload[14] != 0u) ||
            (frame->payload[15] != 0u))
        {
            status = ASSET_TRANSFER_STATUS_MALFORMED;
        }
        else if (object_type !=
            OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
        {
            status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
        }
        else if (schema_version !=
            OLED_UI_LAYOUT_CONFIG_V1_SCHEMA)
        {
            status = ASSET_TRANSFER_STATUS_BAD_SCHEMA;
        }
        else if (total_length != ASSET_TRANSFER_CURRENT_LENGTH)
        {
            status = ASSET_TRANSFER_STATUS_BAD_LENGTH;
        }
        else if (transfer_id == 0u)
        {
            status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
        }
        else if (asset_transfer_state.active != 0u)
        {
            if ((asset_transfer_state.transfer_id != transfer_id) ||
                (asset_transfer_state.payload_crc32 != payload_crc32))
            {
                status = ASSET_TRANSFER_STATUS_BUSY;
            }
            else
            {
                asset_transfer_state.last_activity_ms =
                    kernel_time_now();
            }
        }
        else
        {
            asset_transfer_state.transfer_id = transfer_id;
            asset_transfer_state.next_offset = 0u;
            asset_transfer_state.payload_crc32 = payload_crc32;
            asset_transfer_state.last_chunk_length = 0u;
            asset_transfer_state.last_activity_ms = kernel_time_now();
            asset_transfer_state.active = 1u;
            status = asset_map_persistence_result(
                asset_persistence_begin_candidate(payload_crc32));

            if (status != ASSET_TRANSFER_STATUS_OK)
            {
                asset_clear_session();
            }
        }
    }

    return asset_send_simple(
        frame,
        send_wire,
        send_context,
        ASSET_TRANSFER_OPCODE_BEGIN,
        status,
        transfer_id,
        (status == ASSET_TRANSFER_STATUS_OK) ?
            ASSET_TRANSFER_CURRENT_LENGTH :
            0u,
        (status == ASSET_TRANSFER_STATUS_OK) ?
            asset_transfer_state.next_offset :
            0u);
}

static int asset_handle_write(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint16_t transfer_id = 0u;
    uint16_t offset = 0u;
    uint8_t data_length = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;

    {
        object_type = asset_get_u16(&frame->payload[2]);
        transfer_id = asset_get_u16(&frame->payload[4]);
        offset = asset_get_u16(&frame->payload[6]);
        data_length = frame->payload[8];

        if ((frame->payload[9] != 0u) ||
            (data_length == 0u) ||
            (data_length > ASSET_TRANSFER_CHUNK_MAX) ||
            (frame->payload_length !=
                (uint16_t)(10u + data_length)))
        {
            status = ASSET_TRANSFER_STATUS_BAD_LENGTH;
        }
        else if (object_type !=
            OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
        {
            status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
        }
        else if ((asset_transfer_state.active == 0u) ||
            (asset_transfer_state.transfer_id != transfer_id))
        {
            status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
        }
        else if (offset < asset_transfer_state.next_offset)
        {
            if (((uint16_t)(offset + data_length) ==
                    asset_transfer_state.next_offset) &&
                (data_length ==
                    asset_transfer_state.last_chunk_length))
            {
                if (asset_last_chunk_equal(
                        &frame->payload[10],
                        data_length) != 0)
                {
                    asset_transfer_state.last_activity_ms =
                        kernel_time_now();
                }
                else
                {
                    status =
                        ASSET_TRANSFER_STATUS_DATA_CONFLICT;
                }
            }
            else
            {
                status = ASSET_TRANSFER_STATUS_BAD_OFFSET;
            }
        }
        else if (offset != asset_transfer_state.next_offset)
        {
            status = ASSET_TRANSFER_STATUS_BAD_OFFSET;
        }
        else if ((uint32_t)offset + data_length >
            ASSET_TRANSFER_CURRENT_LENGTH)
        {
            status = ASSET_TRANSFER_STATUS_BAD_LENGTH;
        }
        else
        {
            status = asset_write_candidate(
                &frame->payload[10],
                data_length);

            if (status == ASSET_TRANSFER_STATUS_OK)
            {
                asset_remember_chunk(
                    &frame->payload[10],
                    data_length);
                asset_transfer_state.next_offset =
                    (uint16_t)(offset + data_length);
                asset_transfer_state.last_activity_ms =
                    kernel_time_now();
            }
            else if ((status ==
                    ASSET_TRANSFER_STATUS_STORAGE_ERROR) ||
                (status ==
                    ASSET_TRANSFER_STATUS_INTERNAL_ERROR))
            {
                asset_clear_session();
            }
        }
    }

    return asset_send_simple(
        frame,
        send_wire,
        send_context,
        ASSET_TRANSFER_OPCODE_WRITE_CHUNK,
        status,
        transfer_id,
        (asset_transfer_state.active != 0u) ?
            ASSET_TRANSFER_CURRENT_LENGTH :
            0u,
        (asset_transfer_state.active != 0u) ?
            asset_transfer_state.next_offset :
            0u);
}

static int asset_handle_commit(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint16_t transfer_id = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;

    object_type = asset_get_u16(&frame->payload[2]);
    transfer_id = asset_get_u16(&frame->payload[4]);

    if (object_type != OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
    {
        status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
    }
    else if (asset_transfer_state.active == 0u)
    {
        if (asset_last_commit_matches(transfer_id) == 0)
        {
            status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
        }
    }
    else if (asset_transfer_state.transfer_id != transfer_id)
    {
        status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
    }
    else if (asset_transfer_state.next_offset !=
        ASSET_TRANSFER_CURRENT_LENGTH)
    {
        status = ASSET_TRANSFER_STATUS_BAD_STATE;
    }
    else
    {
        status = asset_map_persistence_result(
            asset_persistence_commit_candidate());

        if (status == ASSET_TRANSFER_STATUS_OK)
        {
            asset_finish_commit();
        }
        else
        {
            asset_clear_session();
        }
    }

    return asset_send_response(
        send_wire,
        send_context,
        frame->request_id,
        ASSET_TRANSFER_OPCODE_COMMIT,
        status,
        OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE,
        transfer_id,
        (status == ASSET_TRANSFER_STATUS_OK) ?
            ASSET_TRANSFER_CURRENT_LENGTH :
            0u,
        (status == ASSET_TRANSFER_STATUS_OK) ?
            ASSET_TRANSFER_CURRENT_LENGTH :
            0u,
        0u);
}

static int asset_handle_abort(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint16_t transfer_id = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;

    {
        object_type = asset_get_u16(&frame->payload[2]);
        transfer_id = asset_get_u16(&frame->payload[4]);

        if (object_type !=
            OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
        {
            status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
        }
        else if ((asset_transfer_state.active != 0u) &&
            (asset_transfer_state.transfer_id != transfer_id))
        {
            status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
        }
        else if (asset_transfer_state.active != 0u)
        {
            asset_clear_session();
        }
    }

    return asset_send_simple(
        frame,
        send_wire,
        send_context,
        ASSET_TRANSFER_OPCODE_ABORT,
        status,
        transfer_id,
        0u,
        0u);
}

static int asset_handle_status(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint16_t transfer_id = 0u;
    uint16_t total_length = 0u;
    uint16_t next_offset = 0u;
    uint16_t committed_length = 0u;
    uint32_t committed_crc32 = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;
    const asset_persistence_record_t *record =
        asset_persistence_active_record();

    object_type = asset_get_u16(&frame->payload[2]);
    transfer_id = asset_get_u16(&frame->payload[4]);

    if (object_type != OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
    {
        status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
    }
    else if (transfer_id == 0u)
    {
        if (asset_transfer_state.active != 0u)
        {
            total_length = ASSET_TRANSFER_CURRENT_LENGTH;
            next_offset = asset_transfer_state.next_offset;
        }
    }
    else if ((asset_transfer_state.active == 0u) ||
        (asset_transfer_state.transfer_id != transfer_id))
    {
        status = ASSET_TRANSFER_STATUS_BAD_TRANSFER_ID;
    }
    else
    {
        total_length = ASSET_TRANSFER_CURRENT_LENGTH;
        next_offset = asset_transfer_state.next_offset;
        asset_transfer_state.last_activity_ms = kernel_time_now();
    }

    if (record != (const asset_persistence_record_t *)0)
    {
        committed_length = ASSET_TRANSFER_CURRENT_LENGTH;
        committed_crc32 = record->payload_crc32;
    }

    asset_transfer_wire.halfwords[14] = committed_length;
    asset_transfer_wire.halfwords[15] = 0u;
    asset_transfer_wire.halfwords[16] =
        (uint16_t)(committed_crc32 & 0xFFFFu);
    asset_transfer_wire.halfwords[17] =
        (uint16_t)(committed_crc32 >> 16);

    return asset_send_response(
        send_wire,
        send_context,
        frame->request_id,
        ASSET_TRANSFER_OPCODE_STATUS,
        status,
        object_type,
        transfer_id,
        total_length,
        next_offset,
        8u);
}

static int asset_handle_read(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint16_t object_type = 0u;
    uint32_t expected_generation = 0u;
    uint16_t offset = 0u;
    uint8_t requested_length = 0u;
    uint8_t response_length = 0u;
    uint16_t total_length = 0u;
    asset_transfer_status_t status = ASSET_TRANSFER_STATUS_OK;
    const asset_persistence_record_t *record =
        asset_persistence_active_record();

    object_type = asset_get_u16(&frame->payload[2]);
    expected_generation = asset_get_u32(&frame->payload[4]);
    offset = asset_get_u16(&frame->payload[8]);
    requested_length = frame->payload[10];

    if (frame->payload[11] != 0u)
    {
        status = ASSET_TRANSFER_STATUS_MALFORMED;
    }
    else if (object_type != OLED_UI_LAYOUT_CONFIG_V1_OBJECT_TYPE)
    {
        status = ASSET_TRANSFER_STATUS_BAD_OBJECT_TYPE;
    }
    else if (record == (const asset_persistence_record_t *)0)
    {
        status = ASSET_TRANSFER_STATUS_NOT_FOUND;
    }
    else if (record->generation != expected_generation)
    {
        status = ASSET_TRANSFER_STATUS_GENERATION_MISMATCH;
    }
    else if ((requested_length == 0u) ||
        ((uint32_t)offset + requested_length >
            ASSET_TRANSFER_CURRENT_LENGTH))
    {
        status = ASSET_TRANSFER_STATUS_BAD_LENGTH;
    }
    else if (asset_persistence_read_active(
            offset,
            &asset_transfer_wire.bytes[28],
            requested_length) == 0)
    {
        status = ASSET_TRANSFER_STATUS_INTERNAL_ERROR;
    }
    else
    {
        response_length = requested_length;
        total_length = ASSET_TRANSFER_CURRENT_LENGTH;
    }

    return asset_send_response(
        send_wire,
        send_context,
        frame->request_id,
        ASSET_TRANSFER_OPCODE_READ_CHUNK,
        status,
        object_type,
        0u,
        total_length,
        (status == ASSET_TRANSFER_STATUS_OK) ?
            (uint16_t)(offset + response_length) :
            offset,
        response_length);
}

int asset_transfer_handle_frame(
    const binary_frame_view_t *frame,
    asset_transfer_send_wire_t send_wire,
    void *send_context)
{
    uint8_t opcode = 0u;

    if ((frame == (const binary_frame_view_t *)0) ||
        (send_wire == (asset_transfer_send_wire_t)0))
    {
        return 0;
    }

    if ((frame->payload != (const uint8_t *)0) &&
        (frame->payload_length >= 2u))
    {
        opcode = frame->payload[1];
    }

    if ((frame->version != BINARY_FRAME_PROTOCOL_VERSION) ||
        (frame->reserved != 0u) ||
        (frame->request_id == 0u) ||
        (frame->frame_type !=
            BINARY_FRAME_TYPE_ASSET_TRANSFER_REQUEST) ||
        (frame->payload == (const uint8_t *)0) ||
        (frame->payload_length < 2u) ||
        (frame->payload[0] != ASSET_TRANSFER_PROTOCOL_VERSION))
    {
        return asset_send_simple(
            frame,
            send_wire,
            send_context,
            opcode,
            ASSET_TRANSFER_STATUS_MALFORMED,
            0u,
            0u,
            0u);
    }

    if ((opcode >= ASSET_TRANSFER_OPCODE_BEGIN) &&
        (opcode <= ASSET_TRANSFER_OPCODE_READ_CHUNK))
    {
        const uint8_t required_flags =
            (opcode <= ASSET_TRANSFER_OPCODE_COMMIT) ?
            ASSET_TRANSFER_FLAG_ALLOW_DESTRUCTIVE :
            0u;
        uint16_t expected_length = 6u;
        asset_transfer_status_t request_status =
            ASSET_TRANSFER_STATUS_OK;

        if (opcode == ASSET_TRANSFER_OPCODE_BEGIN)
        {
            expected_length = 16u;
        }
        else if (opcode == ASSET_TRANSFER_OPCODE_READ_CHUNK)
        {
            expected_length = 12u;
        }

        if (frame->flags != required_flags)
        {
            request_status = ASSET_TRANSFER_STATUS_BAD_FLAGS;
        }
        else if (opcode == ASSET_TRANSFER_OPCODE_WRITE_CHUNK)
        {
            if (frame->payload_length < 11u)
            {
                request_status = ASSET_TRANSFER_STATUS_MALFORMED;
            }
        }
        else if (frame->payload_length != expected_length)
        {
            request_status = ASSET_TRANSFER_STATUS_MALFORMED;
        }

        if (request_status != ASSET_TRANSFER_STATUS_OK)
        {
            return asset_send_simple(
                frame,
                send_wire,
                send_context,
                opcode,
                request_status,
                0u,
                0u,
                0u);
        }
    }

    switch (opcode)
    {
        case ASSET_TRANSFER_OPCODE_BEGIN:
            return asset_handle_begin(
                frame,
                send_wire,
                send_context);

        case ASSET_TRANSFER_OPCODE_WRITE_CHUNK:
            return asset_handle_write(
                frame,
                send_wire,
                send_context);

        case ASSET_TRANSFER_OPCODE_COMMIT:
            return asset_handle_commit(
                frame,
                send_wire,
                send_context);

        case ASSET_TRANSFER_OPCODE_ABORT:
            return asset_handle_abort(
                frame,
                send_wire,
                send_context);

        case ASSET_TRANSFER_OPCODE_STATUS:
            return asset_handle_status(
                frame,
                send_wire,
                send_context);

        case ASSET_TRANSFER_OPCODE_READ_CHUNK:
            return asset_handle_read(
                frame,
                send_wire,
                send_context);

        default:
                return asset_send_simple(
                frame,
                send_wire,
                send_context,
                opcode,
                ASSET_TRANSFER_STATUS_BAD_OPCODE,
                0u,
                0u,
                0u);
    }
}
