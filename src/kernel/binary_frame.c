#include <stdint.h>
#include "kernel/binary_frame.h"

enum
{
    BINARY_FRAME_PARSER_IDLE = 0,
    BINARY_FRAME_PARSER_MAGIC1,
    BINARY_FRAME_PARSER_HEADER,
    BINARY_FRAME_PARSER_PAYLOAD,
    BINARY_FRAME_PARSER_CRC0,
    BINARY_FRAME_PARSER_CRC1,
    BINARY_FRAME_PARSER_RESYNC,
    BINARY_FRAME_PARSER_RESYNC_MAGIC1
};

typedef char binary_rpc_data_wire_must_be_64_bytes[
    (BINARY_RPC_DATA_WIRE_MAX == 64u) ? 1 : -1];

typedef char binary_frame_request_wire_must_be_144_bytes[
    (BINARY_FRAME_MAX_WIRE_BYTES == 144u) ? 1 : -1];

static uint16_t binary_frame_crc16_update(uint16_t crc, uint8_t byte)
{
    uint32_t bit;

    crc ^= (uint16_t)((uint16_t)byte << 8);

    for (bit = 0u; bit < 8u; ++bit)
    {
        if ((crc & 0x8000u) != 0u)
        {
            crc = (uint16_t)((uint16_t)(crc << 1) ^ 0x1021u);
        }
        else
        {
            crc = (uint16_t)(crc << 1);
        }
    }

    return crc;
}

static void binary_frame_parser_reset(binary_frame_parser_t *parser)
{
    parser->state = BINARY_FRAME_PARSER_IDLE;
    parser->header_index = 0u;
    parser->payload_index = 0u;
    parser->payload_length = 0u;
    parser->crc = 0xFFFFu;
    parser->received_crc_low = 0u;
}

void binary_frame_parser_init(binary_frame_parser_t *parser)
{
    if (parser == (binary_frame_parser_t *)0)
    {
        return;
    }

    parser->frames_received = 0u;
    parser->sync_errors = 0u;
    parser->crc_errors = 0u;
    parser->length_errors = 0u;
    binary_frame_parser_reset(parser);
}

uint16_t binary_frame_crc16_ccitt_false(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0xFFFFu;
    uint32_t index;

    if ((data == (const uint8_t *)0) && (length != 0u))
    {
        return 0u;
    }

    for (index = 0u; index < length; ++index)
    {
        crc = binary_frame_crc16_update(crc, data[index]);
    }

    return crc;
}

binary_frame_feed_result_t binary_frame_parser_feed(
    binary_frame_parser_t *parser,
    uint8_t byte,
    binary_frame_view_t *frame)
{
    if ((parser == (binary_frame_parser_t *)0) ||
        (frame == (binary_frame_view_t *)0))
    {
        return BINARY_FRAME_FEED_TEXT;
    }

    switch (parser->state)
    {
        case BINARY_FRAME_PARSER_IDLE:
            if (byte == BINARY_FRAME_MAGIC0)
            {
                parser->state = BINARY_FRAME_PARSER_MAGIC1;
                return BINARY_FRAME_FEED_CONSUMED;
            }
            return BINARY_FRAME_FEED_TEXT;

        case BINARY_FRAME_PARSER_MAGIC1:
            if (byte == BINARY_FRAME_MAGIC1)
            {
                parser->state = BINARY_FRAME_PARSER_HEADER;
                parser->header_index = 0u;
                parser->payload_index = 0u;
                parser->payload_length = 0u;
                parser->crc = 0xFFFFu;
                return BINARY_FRAME_FEED_CONSUMED;
            }

            ++parser->sync_errors;

            if (byte == BINARY_FRAME_MAGIC0)
            {
                parser->state = BINARY_FRAME_PARSER_MAGIC1;
                return BINARY_FRAME_FEED_SYNC_ERROR_CONSUMED;
            }

            binary_frame_parser_reset(parser);
            return BINARY_FRAME_FEED_SYNC_ERROR_TEXT;

        case BINARY_FRAME_PARSER_HEADER:
            parser->header[parser->header_index] = byte;
            parser->crc = binary_frame_crc16_update(parser->crc, byte);
            ++parser->header_index;

            if (parser->header_index == (uint32_t)sizeof(parser->header))
            {
                parser->payload_length =
                    (uint32_t)parser->header[6] |
                    ((uint32_t)parser->header[7] << 8);

                if (parser->payload_length > BINARY_FRAME_MAX_PAYLOAD)
                {
                    ++parser->length_errors;
                    parser->state = BINARY_FRAME_PARSER_RESYNC;
                    return BINARY_FRAME_FEED_LENGTH_ERROR;
                }
                else if (parser->payload_length == 0u)
                {
                    parser->state = BINARY_FRAME_PARSER_CRC0;
                }
                else
                {
                    parser->payload_index = 0u;
                    parser->state = BINARY_FRAME_PARSER_PAYLOAD;
                }
            }

            return BINARY_FRAME_FEED_CONSUMED;

        case BINARY_FRAME_PARSER_PAYLOAD:
            parser->payload[parser->payload_index] = byte;
            parser->crc = binary_frame_crc16_update(parser->crc, byte);
            ++parser->payload_index;

            if (parser->payload_index == parser->payload_length)
            {
                parser->state = BINARY_FRAME_PARSER_CRC0;
            }

            return BINARY_FRAME_FEED_CONSUMED;

        case BINARY_FRAME_PARSER_CRC0:
            parser->received_crc_low = byte;
            parser->state = BINARY_FRAME_PARSER_CRC1;
            return BINARY_FRAME_FEED_CONSUMED;

        case BINARY_FRAME_PARSER_CRC1:
        {
            const uint16_t received_crc =
                (uint16_t)parser->received_crc_low |
                (uint16_t)((uint16_t)byte << 8);

            if (received_crc != parser->crc)
            {
                ++parser->crc_errors;
                binary_frame_parser_reset(parser);
                return BINARY_FRAME_FEED_CRC_ERROR;
            }

            frame->version = parser->header[0];
            frame->frame_type = parser->header[1];
            frame->flags = parser->header[2];
            frame->reserved = parser->header[3];
            frame->request_id =
                (uint16_t)parser->header[4] |
                (uint16_t)((uint16_t)parser->header[5] << 8);
            frame->payload_length = (uint16_t)parser->payload_length;
            frame->payload = parser->payload;

            ++parser->frames_received;
            binary_frame_parser_reset(parser);
            return BINARY_FRAME_FEED_FRAME_READY;
        }

        case BINARY_FRAME_PARSER_RESYNC:
            if (byte == BINARY_FRAME_MAGIC0)
            {
                parser->state = BINARY_FRAME_PARSER_RESYNC_MAGIC1;
            }
            return BINARY_FRAME_FEED_CONSUMED;

        case BINARY_FRAME_PARSER_RESYNC_MAGIC1:
            if (byte == BINARY_FRAME_MAGIC1)
            {
                parser->state = BINARY_FRAME_PARSER_HEADER;
                parser->header_index = 0u;
                parser->payload_index = 0u;
                parser->payload_length = 0u;
                parser->crc = 0xFFFFu;
            }
            else if (byte != BINARY_FRAME_MAGIC0)
            {
                parser->state = BINARY_FRAME_PARSER_RESYNC;
            }
            return BINARY_FRAME_FEED_CONSUMED;

        default:
            binary_frame_parser_reset(parser);
            return BINARY_FRAME_FEED_CONSUMED;
    }
}

uint32_t binary_frame_encode(
    uint8_t frame_type,
    uint8_t flags,
    uint16_t request_id,
    const uint8_t *payload,
    uint16_t payload_length,
    uint8_t *wire,
    uint32_t wire_capacity)
{
    uint32_t index;
    uint32_t wire_length;
    uint16_t crc;

    if ((wire == (uint8_t *)0) ||
        ((payload == (const uint8_t *)0) && (payload_length != 0u)) ||
        (payload_length > BINARY_FRAME_MAX_PAYLOAD))
    {
        return 0u;
    }

    wire_length = BINARY_FRAME_FIXED_PREFIX_BYTES +
        (uint32_t)payload_length + BINARY_FRAME_CRC_BYTES;

    if (wire_capacity < wire_length)
    {
        return 0u;
    }

    wire[0] = BINARY_FRAME_MAGIC0;
    wire[1] = BINARY_FRAME_MAGIC1;
    wire[2] = BINARY_FRAME_PROTOCOL_VERSION;
    wire[3] = frame_type;
    wire[4] = flags;
    wire[5] = 0u;
    wire[6] = (uint8_t)(request_id & 0xFFu);
    wire[7] = (uint8_t)((request_id >> 8) & 0xFFu);
    wire[8] = (uint8_t)(payload_length & 0xFFu);
    wire[9] = (uint8_t)((payload_length >> 8) & 0xFFu);

    for (index = 0u; index < (uint32_t)payload_length; ++index)
    {
        wire[BINARY_FRAME_FIXED_PREFIX_BYTES + index] = payload[index];
    }

    crc = binary_frame_crc16_ccitt_false(
        &wire[2],
        8u + (uint32_t)payload_length);

    wire[BINARY_FRAME_FIXED_PREFIX_BYTES + payload_length] =
        (uint8_t)(crc & 0xFFu);
    wire[BINARY_FRAME_FIXED_PREFIX_BYTES + payload_length + 1u] =
        (uint8_t)((crc >> 8) & 0xFFu);

    return wire_length;
}
