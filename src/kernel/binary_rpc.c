#include <stdint.h>
#include "kernel/binary_rpc.h"

typedef char binary_rpc_request_payload_bound_must_match[
    (BINARY_RPC_REQUEST_PAYLOAD_MAX == BINARY_FRAME_MAX_PAYLOAD) ? 1 : -1];

typedef char binary_rpc_arg_count_must_be_four[
    (COMMAND_SERVICE_MAX_ARGS == 4u) ? 1 : -1];

typedef char binary_rpc_request_shape_must_fit_132[
    ((4u + (COMMAND_SERVICE_MAX_ARGS * (1u + BINARY_RPC_MAX_ARG_BYTES))) ==
     BINARY_RPC_REQUEST_PAYLOAD_MAX) ? 1 : -1];

typedef enum
{
    BINARY_RPC_DECODE_OK = 0,
    BINARY_RPC_DECODE_MALFORMED,
    BINARY_RPC_DECODE_BAD_ARGS
} binary_rpc_decode_result_t;

static void binary_rpc_put_u16(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)(value & 0xFFu);
    destination[1] = (uint8_t)((value >> 8) & 0xFFu);
}

static void binary_rpc_put_u32(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)(value & 0xFFu);
    destination[1] = (uint8_t)((value >> 8) & 0xFFu);
    destination[2] = (uint8_t)((value >> 16) & 0xFFu);
    destination[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static uint16_t binary_rpc_get_u16(const uint8_t *source)
{
    return (uint16_t)source[0] |
        (uint16_t)((uint16_t)source[1] << 8);
}

static int binary_rpc_send_frame(
    binary_rpc_state_t *state,
    uint8_t frame_type,
    uint16_t request_id,
    const uint8_t *payload,
    uint16_t payload_length)
{
    uint32_t wire_length;

    if ((state == (binary_rpc_state_t *)0) ||
        (state->active_binding == (const binary_rpc_binding_t *)0) ||
        (state->active_binding->send_wire == (binary_rpc_send_wire_t)0))
    {
        return 0;
    }

    wire_length = binary_frame_encode(
        frame_type,
        0u,
        request_id,
        payload,
        payload_length,
        state->wire,
        (uint32_t)sizeof(state->wire));

    if ((wire_length == 0u) ||
        (state->active_binding->send_wire(
            state->active_binding->send_context,
            state->wire,
            wire_length) == 0))
    {
        ++state->diagnostics.tx_failures;
        state->tx_failed = 1u;
        return 0;
    }

    return 1;
}

static int binary_rpc_send_protocol_error(
    binary_rpc_state_t *state,
    uint16_t request_id,
    uint8_t code,
    uint8_t observed)
{
    state->frame_payload[0] = code;
    state->frame_payload[1] = observed;
    ++state->diagnostics.protocol_errors;

    return binary_rpc_send_frame(
        state,
        BINARY_FRAME_TYPE_PROTOCOL_ERROR,
        request_id,
        state->frame_payload,
        2u);
}

static int binary_rpc_flush_data(binary_rpc_state_t *state)
{
    uint32_t index;
    uint16_t payload_length;

    if (state->data_chunk_length == 0u)
    {
        return 1;
    }

    binary_rpc_put_u16(&state->frame_payload[0], state->active_rpc_id);
    binary_rpc_put_u16(&state->frame_payload[2], state->sequence);

    for (index = 0u; index < state->data_chunk_length; ++index)
    {
        state->frame_payload[4u + index] = state->data_chunk[index];
    }

    payload_length = (uint16_t)(4u + state->data_chunk_length);

    if (binary_rpc_send_frame(
            state,
            BINARY_FRAME_TYPE_RPC_DATA,
            state->active_request_id,
            state->frame_payload,
            payload_length) == 0)
    {
        return 0;
    }

    state->total_output_bytes += state->data_chunk_length;
    state->data_chunk_length = 0u;
    ++state->sequence;
    ++state->diagnostics.response_data_frames;
    return 1;
}

static int binary_rpc_output_write_byte(void *context, uint8_t byte)
{
    binary_rpc_state_t *state = (binary_rpc_state_t *)context;

    if ((state == (binary_rpc_state_t *)0) || (state->tx_failed != 0u))
    {
        return 0;
    }

    if (state->data_chunk_length >= BINARY_RPC_DATA_CHUNK_MAX)
    {
        if (binary_rpc_flush_data(state) == 0)
        {
            return 0;
        }
    }

    state->data_chunk[state->data_chunk_length] = byte;
    ++state->data_chunk_length;

    if ((state->data_chunk_length == BINARY_RPC_DATA_CHUNK_MAX) ||
        (byte == (uint8_t)'\n'))
    {
        return binary_rpc_flush_data(state);
    }

    return 1;
}

static int binary_rpc_send_end(
    binary_rpc_state_t *state,
    uint16_t rpc_id,
    uint16_t request_id,
    uint16_t chunk_count,
    uint32_t total_output_bytes,
    uint8_t status_domain,
    uint8_t status_code)
{
    binary_rpc_put_u16(&state->frame_payload[0], rpc_id);
    binary_rpc_put_u16(&state->frame_payload[2], chunk_count);
    binary_rpc_put_u32(&state->frame_payload[4], total_output_bytes);
    state->frame_payload[8] = status_domain;
    state->frame_payload[9] = status_code;

    if (status_domain == BINARY_RPC_STATUS_DOMAIN_PROTOCOL)
    {
        ++state->diagnostics.protocol_errors;
    }

    if (binary_rpc_send_frame(
            state,
            BINARY_FRAME_TYPE_RPC_END,
            request_id,
            state->frame_payload,
            10u) == 0)
    {
        return 0;
    }

    ++state->diagnostics.response_end_frames;
    return 1;
}

static int binary_rpc_send_simple_end(
    binary_rpc_state_t *state,
    uint16_t rpc_id,
    uint16_t request_id,
    uint8_t status_domain,
    uint8_t status_code)
{
    return binary_rpc_send_end(
        state,
        rpc_id,
        request_id,
        0u,
        0u,
        status_domain,
        status_code);
}

static int binary_rpc_send_hello(
    binary_rpc_state_t *state,
    uint16_t request_id)
{
    state->frame_payload[0] = BINARY_FRAME_PROTOCOL_VERSION;
    state->frame_payload[1] = COMMAND_SERVICE_FOUNDATION_VERSION;
    state->frame_payload[2] = COMMAND_SERVICE_MAX_ARGS;
    state->frame_payload[3] = 0u;
    binary_rpc_put_u16(
        &state->frame_payload[4],
        (uint16_t)COMMAND_SERVICE_LINE_CAPACITY);
    binary_rpc_put_u16(
        &state->frame_payload[6],
        (uint16_t)BINARY_RPC_REQUEST_PAYLOAD_MAX);
    binary_rpc_put_u16(
        &state->frame_payload[8],
        (uint16_t)BINARY_RPC_DATA_CHUNK_MAX);
    binary_rpc_put_u16(
        &state->frame_payload[10],
        (uint16_t)command_service_registry_count());
    binary_rpc_put_u32(
        &state->frame_payload[12],
        BINARY_RPC_CAPABILITY_FLAGS);

    return binary_rpc_send_frame(
        state,
        BINARY_FRAME_TYPE_HELLO_RESPONSE,
        request_id,
        state->frame_payload,
        16u);
}

static binary_rpc_decode_result_t binary_rpc_decode_request(
    binary_rpc_state_t *state,
    const binary_frame_view_t *frame,
    uint16_t *rpc_id,
    uint32_t *argc)
{
    uint32_t declared_argc;
    uint32_t offset;
    uint32_t arg_index;
    binary_rpc_decode_result_t result = BINARY_RPC_DECODE_OK;

    if ((frame->payload == (const uint8_t *)0) ||
        (frame->payload_length < 4u))
    {
        return BINARY_RPC_DECODE_MALFORMED;
    }

    *rpc_id = binary_rpc_get_u16(&frame->payload[0]);
    declared_argc = frame->payload[2];
    *argc = declared_argc;

    if (frame->payload[3] != 0u)
    {
        return BINARY_RPC_DECODE_MALFORMED;
    }

    state->request.descriptor = (const command_service_descriptor_t *)0;
    state->request.argc = declared_argc;

    for (arg_index = 0u; arg_index < COMMAND_SERVICE_MAX_ARGS; ++arg_index)
    {
        state->request.argv[arg_index] = (const char *)0;
        state->argument_storage[arg_index][0] = '\0';
    }

    if (declared_argc > COMMAND_SERVICE_MAX_ARGS)
    {
        result = BINARY_RPC_DECODE_BAD_ARGS;
    }

    offset = 4u;

    for (arg_index = 0u; arg_index < declared_argc; ++arg_index)
    {
        uint32_t byte_index;
        uint32_t argument_length;

        if (offset >= frame->payload_length)
        {
            return BINARY_RPC_DECODE_MALFORMED;
        }

        argument_length = frame->payload[offset];
        ++offset;

        if ((offset + argument_length) > frame->payload_length)
        {
            return BINARY_RPC_DECODE_MALFORMED;
        }

        if (argument_length > BINARY_RPC_MAX_ARG_BYTES)
        {
            result = BINARY_RPC_DECODE_BAD_ARGS;
        }

        for (byte_index = 0u; byte_index < argument_length; ++byte_index)
        {
            if (frame->payload[offset + byte_index] == 0u)
            {
                result = BINARY_RPC_DECODE_BAD_ARGS;
            }
        }

        if ((arg_index < COMMAND_SERVICE_MAX_ARGS) &&
            (argument_length <= BINARY_RPC_MAX_ARG_BYTES))
        {
            for (byte_index = 0u; byte_index < argument_length; ++byte_index)
            {
                state->argument_storage[arg_index][byte_index] =
                    (char)frame->payload[offset + byte_index];
            }

            state->argument_storage[arg_index][argument_length] = '\0';
            state->request.argv[arg_index] = state->argument_storage[arg_index];
        }

        offset += argument_length;
    }

    if (offset != frame->payload_length)
    {
        return BINARY_RPC_DECODE_MALFORMED;
    }

    return result;
}

static int binary_rpc_handle_rpc_request(
    binary_rpc_state_t *state,
    const binary_frame_view_t *frame,
    const binary_rpc_binding_t *binding)
{
    command_service_context_t command_context;
    const command_service_descriptor_t *descriptor;
    command_service_status_t status;
    binary_rpc_decode_result_t decode_result;
    uint16_t rpc_id = 0u;
    uint32_t argc = 0u;

    ++state->diagnostics.rpc_requests;

    if ((frame->flags & (uint8_t)~BINARY_RPC_FLAG_ALLOW_DESTRUCTIVE) != 0u)
    {
        if ((frame->payload != (const uint8_t *)0) &&
            (frame->payload_length >= 2u))
        {
            rpc_id = binary_rpc_get_u16(frame->payload);
            return binary_rpc_send_simple_end(
                state,
                rpc_id,
                frame->request_id,
                BINARY_RPC_STATUS_DOMAIN_PROTOCOL,
                BINARY_RPC_PROTOCOL_BAD_FLAGS);
        }

        return binary_rpc_send_protocol_error(
            state,
            frame->request_id,
            BINARY_RPC_PROTOCOL_BAD_FLAGS,
            frame->flags);
    }

    decode_result = binary_rpc_decode_request(state, frame, &rpc_id, &argc);

    if (decode_result == BINARY_RPC_DECODE_MALFORMED)
    {
        if (frame->payload_length >= 2u)
        {
            return binary_rpc_send_simple_end(
                state,
                rpc_id,
                frame->request_id,
                BINARY_RPC_STATUS_DOMAIN_PROTOCOL,
                BINARY_RPC_PROTOCOL_MALFORMED_REQUEST);
        }

        return binary_rpc_send_protocol_error(
            state,
            frame->request_id,
            BINARY_RPC_PROTOCOL_MALFORMED_REQUEST,
            0u);
    }

    descriptor = command_service_find_rpc_id(rpc_id);
    if (descriptor == (const command_service_descriptor_t *)0)
    {
        return binary_rpc_send_simple_end(
            state,
            rpc_id,
            frame->request_id,
            BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE,
            COMMAND_SERVICE_STATUS_NOT_FOUND);
    }

    if ((decode_result == BINARY_RPC_DECODE_BAD_ARGS) ||
        (argc < descriptor->min_args) ||
        (argc > descriptor->max_args))
    {
        return binary_rpc_send_simple_end(
            state,
            rpc_id,
            frame->request_id,
            BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE,
            COMMAND_SERVICE_STATUS_BAD_ARGS);
    }

    if ((descriptor->command_class == COMMAND_SERVICE_CLASS_DESTRUCTIVE) &&
        ((frame->flags & BINARY_RPC_FLAG_ALLOW_DESTRUCTIVE) == 0u))
    {
        return binary_rpc_send_simple_end(
            state,
            rpc_id,
            frame->request_id,
            BINARY_RPC_STATUS_DOMAIN_PROTOCOL,
            BINARY_RPC_PROTOCOL_DESTRUCTIVE_CONFIRM_REQUIRED);
    }

    state->request.descriptor = descriptor;
    state->request.argc = argc;
    state->active_rpc_id = rpc_id;
    state->active_request_id = frame->request_id;
    state->sequence = 0u;
    state->total_output_bytes = 0u;
    state->data_chunk_length = 0u;
    state->tx_failed = 0u;

    command_context.write_byte = binary_rpc_output_write_byte;
    command_context.write_context = state;
    command_context.source_rx_event_mask = binding->source_rx_event_mask;
    command_context.write_failed = 0u;

    status = command_service_execute(
        &state->request,
        &command_context,
        binding->command_handler,
        binding->command_handler_context);

    if ((state->tx_failed == 0u) &&
        (binary_rpc_flush_data(state) == 0))
    {
        status = COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if ((command_context.write_failed != 0u) || (state->tx_failed != 0u))
    {
        status = COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return binary_rpc_send_end(
        state,
        rpc_id,
        frame->request_id,
        state->sequence,
        state->total_output_bytes,
        BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE,
        (uint8_t)status);
}

void binary_rpc_init(binary_rpc_state_t *state)
{
    uint32_t index;

    if (state == (binary_rpc_state_t *)0)
    {
        return;
    }

    state->diagnostics.hello_requests = 0u;
    state->diagnostics.rpc_requests = 0u;
    state->diagnostics.protocol_errors = 0u;
    state->diagnostics.response_data_frames = 0u;
    state->diagnostics.response_end_frames = 0u;
    state->diagnostics.tx_failures = 0u;
    state->request.descriptor = (const command_service_descriptor_t *)0;
    state->request.argc = 0u;

    for (index = 0u; index < COMMAND_SERVICE_MAX_ARGS; ++index)
    {
        state->request.argv[index] = (const char *)0;
        state->argument_storage[index][0] = '\0';
    }

    state->active_binding = (const binary_rpc_binding_t *)0;
    state->active_rpc_id = 0u;
    state->active_request_id = 0u;
    state->sequence = 0u;
    state->total_output_bytes = 0u;
    state->data_chunk_length = 0u;
    state->tx_failed = 0u;
}

int binary_rpc_handle_frame(
    binary_rpc_state_t *state,
    const binary_frame_view_t *frame,
    const binary_rpc_binding_t *binding)
{
    int result;

    if ((state == (binary_rpc_state_t *)0) ||
        (frame == (const binary_frame_view_t *)0) ||
        (binding == (const binary_rpc_binding_t *)0) ||
        (binding->send_wire == (binary_rpc_send_wire_t)0) ||
        (binding->command_handler == (command_service_handler_t)0))
    {
        return 0;
    }

    state->active_binding = binding;
    state->tx_failed = 0u;

    if (frame->version != BINARY_FRAME_PROTOCOL_VERSION)
    {
        result = binary_rpc_send_protocol_error(
            state,
            frame->request_id,
            BINARY_RPC_PROTOCOL_MALFORMED_REQUEST,
            frame->version);
        state->active_binding = (const binary_rpc_binding_t *)0;
        return result;
    }

    if (frame->reserved != 0u)
    {
        result = binary_rpc_send_protocol_error(
            state,
            frame->request_id,
            BINARY_RPC_PROTOCOL_MALFORMED_REQUEST,
            frame->reserved);
        state->active_binding = (const binary_rpc_binding_t *)0;
        return result;
    }

    if (frame->request_id == 0u)
    {
        result = binary_rpc_send_protocol_error(
            state,
            0u,
            BINARY_RPC_PROTOCOL_REQUEST_ID_RESERVED,
            0u);
        state->active_binding = (const binary_rpc_binding_t *)0;
        return result;
    }

    switch (frame->frame_type)
    {
        case BINARY_FRAME_TYPE_HELLO_REQUEST:
            ++state->diagnostics.hello_requests;

            if (frame->flags != 0u)
            {
                result = binary_rpc_send_protocol_error(
                    state,
                    frame->request_id,
                    BINARY_RPC_PROTOCOL_BAD_FLAGS,
                    frame->flags);
            }
            else if (frame->payload_length != 0u)
            {
                result = binary_rpc_send_protocol_error(
                    state,
                    frame->request_id,
                    BINARY_RPC_PROTOCOL_MALFORMED_REQUEST,
                    0u);
            }
            else
            {
                result = binary_rpc_send_hello(state, frame->request_id);
            }
            break;

        case BINARY_FRAME_TYPE_RPC_REQUEST:
            result = binary_rpc_handle_rpc_request(state, frame, binding);
            break;

        default:
            result = binary_rpc_send_protocol_error(
                state,
                frame->request_id,
                BINARY_RPC_PROTOCOL_MALFORMED_REQUEST,
                frame->frame_type);
            break;
    }

    state->active_binding = (const binary_rpc_binding_t *)0;
    return result;
}
