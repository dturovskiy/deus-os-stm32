#ifndef KERNEL_BINARY_RPC_H
#define KERNEL_BINARY_RPC_H

#include <stdint.h>
#include "kernel/binary_frame.h"
#include "kernel/command_service.h"

#define BINARY_RPC_FLAG_ALLOW_DESTRUCTIVE 0x01u
#define BINARY_RPC_MAX_ARG_BYTES          31u
#define BINARY_RPC_REQUEST_PAYLOAD_MAX   132u
#define BINARY_RPC_CAPABILITY_FLAGS       0x0000003Fu

typedef enum
{
    BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE = 0u,
    BINARY_RPC_STATUS_DOMAIN_PROTOCOL = 1u
} binary_rpc_status_domain_t;

typedef enum
{
    BINARY_RPC_PROTOCOL_MALFORMED_REQUEST = 1u,
    BINARY_RPC_PROTOCOL_BAD_FLAGS = 2u,
    BINARY_RPC_PROTOCOL_DESTRUCTIVE_CONFIRM_REQUIRED = 3u,
    BINARY_RPC_PROTOCOL_REQUEST_ID_RESERVED = 4u,
    BINARY_RPC_PROTOCOL_PAYLOAD_TOO_LARGE = 5u
} binary_rpc_protocol_status_t;

typedef int (*binary_rpc_send_wire_t)(
    void *context,
    const uint8_t *data,
    uint32_t length);

typedef struct
{
    binary_rpc_send_wire_t send_wire;
    void *send_context;
    command_service_handler_t command_handler;
    void *command_handler_context;
    uint32_t source_rx_event_mask;
} binary_rpc_binding_t;

typedef struct
{
    uint32_t hello_requests;
    uint32_t rpc_requests;
    uint32_t protocol_errors;
    uint32_t response_data_frames;
    uint32_t response_end_frames;
    uint32_t tx_failures;
} binary_rpc_diagnostics_t;

typedef struct
{
    char argument_storage[COMMAND_SERVICE_MAX_ARGS][BINARY_RPC_MAX_ARG_BYTES + 1u];
    uint8_t wire[BINARY_RPC_DATA_WIRE_MAX];
} binary_rpc_workspace_t;

typedef struct
{
    binary_rpc_diagnostics_t diagnostics;
    binary_rpc_workspace_t *workspace;
    const binary_rpc_binding_t *active_binding;
    uint32_t total_output_bytes;
    uint16_t active_rpc_id;
    uint16_t active_request_id;
    uint16_t sequence;
    uint8_t data_chunk_length;
    uint8_t tx_failed;
} binary_rpc_state_t;

void binary_rpc_init(
    binary_rpc_state_t *state,
    binary_rpc_workspace_t *workspace);
int binary_rpc_handle_frame(
    binary_rpc_state_t *state,
    const binary_frame_view_t *frame,
    const binary_rpc_binding_t *binding);

#endif
