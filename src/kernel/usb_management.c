#include <stdint.h>
#include "drivers/usb_device.h"
#include "kernel/binary_frame.h"
#include "kernel/binary_rpc.h"
#include "kernel/usb_management.h"

_Static_assert(BINARY_RPC_DATA_WIRE_MAX <= USB_MANAGEMENT_MAX_PACKET,
    "management RPC response wire frame must fit one EP4 packet");

static binary_frame_parser_t usb_management_parser;
static binary_rpc_state_t usb_management_rpc_state;
static uint32_t usb_management_last_bus_reset_count;

static void usb_management_protocol_reset(void)
{
    binary_rpc_workspace_t *workspace = usb_management_rpc_state.workspace;

    binary_frame_parser_init(&usb_management_parser);

    if (workspace != (binary_rpc_workspace_t *)0)
    {
        binary_rpc_init(&usb_management_rpc_state, workspace);
    }
}

int usb_management_runtime_init(binary_rpc_workspace_t *workspace)
{
    if (workspace == (binary_rpc_workspace_t *)0)
    {
        return 0;
    }

    binary_frame_parser_init(&usb_management_parser);
    binary_rpc_init(&usb_management_rpc_state, workspace);
    usb_management_last_bus_reset_count =
        usb_device_diagnostics.bus_reset_count;
    return 1;
}

uint32_t usb_management_runtime_service(
    const binary_rpc_binding_t *binding)
{
    uint8_t byte;
    uint32_t rpc_requests = 0u;
    const uint32_t bus_reset_count =
        usb_device_diagnostics.bus_reset_count;

    if ((usb_management_rpc_state.workspace ==
            (binary_rpc_workspace_t *)0) ||
        (binding == (const binary_rpc_binding_t *)0))
    {
        return 0u;
    }

    if (bus_reset_count != usb_management_last_bus_reset_count)
    {
        usb_management_protocol_reset();
        usb_management_last_bus_reset_count = bus_reset_count;
    }

    if (usb_management_is_configured() == 0)
    {
        usb_management_protocol_reset();
        return 0u;
    }

    while (usb_management_try_getc(&byte) != 0)
    {
        binary_frame_view_t frame;
        const binary_frame_feed_result_t result =
            binary_frame_parser_feed(
                &usb_management_parser,
                byte,
                &frame);

        if (result == BINARY_FRAME_FEED_FRAME_READY)
        {
            if (frame.frame_type == BINARY_FRAME_TYPE_RPC_REQUEST)
            {
                ++rpc_requests;
            }

            (void)binary_rpc_handle_frame(
                &usb_management_rpc_state,
                &frame,
                binding);
        }
    }

    return rpc_requests;
}
