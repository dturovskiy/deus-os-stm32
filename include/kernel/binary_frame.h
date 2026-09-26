#ifndef KERNEL_BINARY_FRAME_H
#define KERNEL_BINARY_FRAME_H

#include <stdint.h>

#define BINARY_FRAME_MAGIC0                0xA5u
#define BINARY_FRAME_MAGIC1                0x5Au
#define BINARY_FRAME_PROTOCOL_VERSION      1u
#define BINARY_FRAME_FIXED_PREFIX_BYTES   10u
#define BINARY_FRAME_CRC_BYTES             2u
#define BINARY_FRAME_MAX_PAYLOAD          132u
#define BINARY_FRAME_MAX_WIRE_BYTES       (BINARY_FRAME_FIXED_PREFIX_BYTES + BINARY_FRAME_MAX_PAYLOAD + BINARY_FRAME_CRC_BYTES)
#define BINARY_RPC_DATA_CHUNK_MAX          48u
#define BINARY_RPC_DATA_PAYLOAD_MAX        (4u + BINARY_RPC_DATA_CHUNK_MAX)
#define BINARY_RPC_DATA_WIRE_MAX           (BINARY_FRAME_FIXED_PREFIX_BYTES + BINARY_RPC_DATA_PAYLOAD_MAX + BINARY_FRAME_CRC_BYTES)

typedef enum
{
    BINARY_FRAME_TYPE_HELLO_REQUEST = 0x01u,
    BINARY_FRAME_TYPE_RPC_REQUEST = 0x02u,
    BINARY_FRAME_TYPE_ASSET_TRANSFER_REQUEST = 0x03u,
    BINARY_FRAME_TYPE_HELLO_RESPONSE = 0x81u,
    BINARY_FRAME_TYPE_RPC_DATA = 0x82u,
    BINARY_FRAME_TYPE_RPC_END = 0x83u,
    BINARY_FRAME_TYPE_PROTOCOL_ERROR = 0x84u,
    BINARY_FRAME_TYPE_ASSET_TRANSFER_RESPONSE = 0x85u
} binary_frame_type_t;

typedef struct
{
    uint8_t version;
    uint8_t frame_type;
    uint8_t flags;
    uint8_t reserved;
    uint16_t request_id;
    uint16_t payload_length;
    const uint8_t *payload;
} binary_frame_view_t;

typedef enum
{
    BINARY_FRAME_FEED_TEXT = 0,
    BINARY_FRAME_FEED_CONSUMED,
    BINARY_FRAME_FEED_FRAME_READY,
    BINARY_FRAME_FEED_SYNC_ERROR_TEXT,
    BINARY_FRAME_FEED_SYNC_ERROR_CONSUMED,
    BINARY_FRAME_FEED_CRC_ERROR,
    BINARY_FRAME_FEED_LENGTH_ERROR
} binary_frame_feed_result_t;

typedef struct
{
    uint8_t state;
    uint8_t header[8];
    uint8_t payload[BINARY_FRAME_MAX_PAYLOAD];
    uint32_t header_index;
    uint32_t payload_index;
    uint32_t payload_length;
    uint16_t crc;
    uint8_t received_crc_low;
    uint32_t frames_received;
    uint32_t sync_errors;
    uint32_t crc_errors;
    uint32_t length_errors;
} binary_frame_parser_t;

void binary_frame_parser_init(binary_frame_parser_t *parser);
binary_frame_feed_result_t binary_frame_parser_feed(
    binary_frame_parser_t *parser,
    uint8_t byte,
    binary_frame_view_t *frame);
uint16_t binary_frame_crc16_ccitt_false(const uint8_t *data, uint32_t length);
uint32_t binary_frame_encode(
    uint8_t frame_type,
    uint8_t flags,
    uint16_t request_id,
    const uint8_t *payload,
    uint16_t payload_length,
    uint8_t *wire,
    uint32_t wire_capacity);

#endif
