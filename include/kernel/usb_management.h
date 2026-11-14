#ifndef KERNEL_USB_MANAGEMENT_H
#define KERNEL_USB_MANAGEMENT_H

#include <stdint.h>
#include "kernel/binary_rpc.h"

/* Passive diagnostics for a FUTURE firmware, appended to the existing
 * read-only "node status" output. Never opens a bridge or touches USB/GPIO. */
typedef struct
{
    uint8_t active;
    uint8_t faulted;
    uint8_t uart_attempted;
    uint8_t last_open_reason;
    uint8_t node_maintenance_owned;
    uint8_t firmware_update_reset_pending;
    uint8_t split_adapter_profile;
} usb_management_esp_bridge_diagnostics_t;

void usb_management_esp_bridge_diagnostics_get(
    usb_management_esp_bridge_diagnostics_t *out);

int usb_management_runtime_init(binary_rpc_workspace_t *workspace);

uint32_t usb_management_runtime_service(
    const binary_rpc_binding_t *binding);

#endif
