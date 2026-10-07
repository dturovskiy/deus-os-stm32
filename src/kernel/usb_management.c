#include <stdint.h>
#include "drivers/usb_device.h"
#include "kernel/asset_transfer.h"
#include "kernel/binary_frame.h"
#include "kernel/binary_rpc.h"
#include "kernel/time.h"
#include "kernel/usb_management.h"

#define REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define RCC_APB1ENR REG32(0x4002101Cu)
#define RCC_APB1_BKPEN (1u << 27)
#define RCC_APB1_PWREN (1u << 28)
#define PWR_CR REG32(0x40007000u)
#define PWR_CR_DBP (1u << 8)
#define BKP_DR1 (*(volatile uint16_t *)(uintptr_t)0x40006C04u)
#define BKP_DR2 (*(volatile uint16_t *)(uintptr_t)0x40006C08u)
#define SCB_AIRCR REG32(0xE000ED0Cu)
#define SCB_AIRCR_PRIGROUP (0x7u << 8)
#define SCB_AIRCR_SYSRESETREQ (1u << 2)
#define SCB_AIRCR_VECTKEY (0x5FAu << 16)

#define FIRMWARE_UPDATE_FLAG_ALLOW_DESTRUCTIVE 0x01u
#define FIRMWARE_UPDATE_OPCODE_ENTER_BOOTLOADER 0x01u
#define FIRMWARE_UPDATE_STATUS_OK 0x00u
#define FIRMWARE_UPDATE_STATUS_INVALID_STATE 0x01u
#define FIRMWARE_UPDATE_STATUS_BAD_LENGTH 0x02u
#define FIRMWARE_UPDATE_STATUS_BAD_FLAGS 0x03u
#define FIRMWARE_UPDATE_STATUS_BAD_HEADER 0x04u
#define FIRMWARE_UPDATE_STATE_RESETTING 0x80u
#define FIRMWARE_UPDATE_RESPONSE_PAYLOAD_BYTES 16u
#define FIRMWARE_UPDATE_RESPONSE_WIRE_BYTES 28u
#define FIRMWARE_UPDATE_RESET_FALLBACK_MS 250u
#define FIRMWARE_UPDATE_TOKEN1 0xD35Au
#define FIRMWARE_UPDATE_TOKEN2 0x2CA5u

_Static_assert(BINARY_RPC_DATA_WIRE_MAX <= USB_MANAGEMENT_MAX_PACKET,
    "management RPC response wire frame must fit one EP4 packet");
_Static_assert(ASSET_TRANSFER_RESPONSE_WIRE_MAX <= USB_MANAGEMENT_MAX_PACKET,
    "management Asset response wire frame must fit one EP4 packet");
_Static_assert(FIRMWARE_UPDATE_RESPONSE_WIRE_BYTES <= USB_MANAGEMENT_MAX_PACKET,
    "management firmware update response must fit one EP4 packet");

typedef union
{
    uint8_t bytes[FIRMWARE_UPDATE_RESPONSE_WIRE_BYTES];
    uint16_t halfwords[FIRMWARE_UPDATE_RESPONSE_WIRE_BYTES / 2u];
} firmware_update_response_wire_t;

static binary_frame_parser_t usb_management_parser;
static binary_rpc_state_t usb_management_rpc_state;
static firmware_update_response_wire_t firmware_update_response_wire;
static uint32_t usb_management_last_bus_reset_count;
static uint32_t firmware_update_reset_baseline_packet_count;
static kernel_time_ms_t firmware_update_reset_deadline;
static uint32_t firmware_update_reset_pending;

__attribute__((cold, noreturn)) static void firmware_update_system_reset(void)
{
    __asm volatile("cpsid i" ::: "memory");
    __asm volatile("dsb" ::: "memory");
    SCB_AIRCR =
        (SCB_AIRCR & SCB_AIRCR_PRIGROUP) |
        SCB_AIRCR_VECTKEY |
        SCB_AIRCR_SYSRESETREQ;
    __asm volatile("dsb" ::: "memory");

    for (;;)
    {
    }
}

__attribute__((cold)) static void firmware_update_write_entry_token(void)
{
    RCC_APB1ENR |= RCC_APB1_PWREN | RCC_APB1_BKPEN;
    PWR_CR |= PWR_CR_DBP;
    BKP_DR1 = FIRMWARE_UPDATE_TOKEN1;
    BKP_DR2 = FIRMWARE_UPDATE_TOKEN2;
    __asm volatile("dsb" ::: "memory");
    PWR_CR &= ~PWR_CR_DBP;
}

__attribute__((cold, noinline)) static void firmware_update_reset_service(void)
{
    if (firmware_update_reset_pending == 0u)
    {
        return;
    }

    if (usb_device_diagnostics.management_tx_packet_count !=
        firmware_update_reset_baseline_packet_count)
    {
        firmware_update_system_reset();
    }

    if (kernel_time_reached(
            kernel_time_now(),
            firmware_update_reset_deadline) != 0)
    {
        firmware_update_system_reset();
    }
}

__attribute__((cold, noinline)) static int firmware_update_send_response(
    const binary_frame_view_t *frame,
    const binary_rpc_binding_t *binding,
    uint8_t status,
    uint8_t state)
{
    if (binding->send_wire == (binary_rpc_send_wire_t)0)
    {
        return 0;
    }

    firmware_update_response_wire.halfwords[5] =
        (uint16_t)FIRMWARE_UPDATE_OPCODE_ENTER_BOOTLOADER |
        (uint16_t)((uint16_t)status << 8);
    firmware_update_response_wire.halfwords[6] = state;

    (void)binary_frame_finalize_in_place(
        BINARY_FRAME_TYPE_FIRMWARE_UPDATE_RESPONSE,
        0u,
        frame->request_id,
        FIRMWARE_UPDATE_RESPONSE_PAYLOAD_BYTES,
        firmware_update_response_wire.bytes);

    return binding->send_wire(
        binding->send_context,
        firmware_update_response_wire.bytes,
        FIRMWARE_UPDATE_RESPONSE_WIRE_BYTES);
}

__attribute__((cold, noinline)) static void firmware_update_handle_runtime_frame(
    const binary_frame_view_t *frame,
    const binary_rpc_binding_t *binding)
{
    uint8_t status = FIRMWARE_UPDATE_STATUS_OK;

    if ((frame->version != BINARY_FRAME_PROTOCOL_VERSION) ||
        (frame->request_id == 0u))
    {
        status = FIRMWARE_UPDATE_STATUS_BAD_HEADER;
    }
    else if ((frame->flags != FIRMWARE_UPDATE_FLAG_ALLOW_DESTRUCTIVE) ||
        (frame->reserved != 0u))
    {
        status = FIRMWARE_UPDATE_STATUS_BAD_FLAGS;
    }
    else if ((frame->payload_length != 1u) ||
        (frame->payload == (const uint8_t *)0))
    {
        status = FIRMWARE_UPDATE_STATUS_BAD_LENGTH;
    }
    else if (frame->payload[0] != FIRMWARE_UPDATE_OPCODE_ENTER_BOOTLOADER)
    {
        status = FIRMWARE_UPDATE_STATUS_INVALID_STATE;
    }

    if (status != FIRMWARE_UPDATE_STATUS_OK)
    {
        (void)firmware_update_send_response(frame, binding, status, 0u);
        return;
    }

    if (firmware_update_reset_pending != 0u)
    {
        (void)firmware_update_send_response(
            frame,
            binding,
            FIRMWARE_UPDATE_STATUS_INVALID_STATE,
            FIRMWARE_UPDATE_STATE_RESETTING);
        return;
    }

    firmware_update_reset_baseline_packet_count =
        usb_device_diagnostics.management_tx_packet_count;

    if (firmware_update_send_response(
            frame,
            binding,
            FIRMWARE_UPDATE_STATUS_OK,
            FIRMWARE_UPDATE_STATE_RESETTING) == 0)
    {
        return;
    }

    firmware_update_write_entry_token();
    firmware_update_reset_deadline =
        kernel_time_now() + FIRMWARE_UPDATE_RESET_FALLBACK_MS;
    firmware_update_reset_pending = 1u;
}

static void usb_management_protocol_reset(void)
{
    binary_rpc_workspace_t *workspace =
        usb_management_rpc_state.workspace;

    binary_frame_parser_init(&usb_management_parser);
    asset_transfer_reset_session();

    if (workspace != (binary_rpc_workspace_t *)0)
    {
        binary_rpc_init(
            &usb_management_rpc_state,
            workspace);
    }
}

int usb_management_runtime_init(binary_rpc_workspace_t *workspace)
{
    if (workspace == (binary_rpc_workspace_t *)0)
    {
        return 0;
    }

    binary_frame_parser_init(&usb_management_parser);
    binary_rpc_init(
        &usb_management_rpc_state,
        workspace);
    asset_transfer_init();
    firmware_update_reset_baseline_packet_count = 0u;
    firmware_update_reset_deadline = 0u;
    firmware_update_reset_pending = 0u;

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

    if (firmware_update_reset_pending != 0u)
    {
        firmware_update_reset_service();
    }
    asset_transfer_poll();

    if (bus_reset_count !=
        usb_management_last_bus_reset_count)
    {
        usb_management_protocol_reset();
        usb_management_last_bus_reset_count =
            bus_reset_count;
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
            if (frame.frame_type ==
                BINARY_FRAME_TYPE_FIRMWARE_UPDATE_REQUEST)
            {
                firmware_update_handle_runtime_frame(&frame, binding);
                if (firmware_update_reset_pending != 0u)
                {
                    firmware_update_reset_service();
                }
                continue;
            }

            if (frame.frame_type ==
                BINARY_FRAME_TYPE_ASSET_TRANSFER_REQUEST)
            {
                (void)asset_transfer_handle_frame(
                    &frame,
                    binding->send_wire,
                    binding->send_context);
                continue;
            }

            if (frame.frame_type ==
                BINARY_FRAME_TYPE_RPC_REQUEST)
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
