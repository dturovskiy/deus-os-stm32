#ifndef KERNEL_USB_MANAGEMENT_H
#define KERNEL_USB_MANAGEMENT_H

#include <stdint.h>
#include "kernel/binary_rpc.h"

int usb_management_runtime_init(binary_rpc_workspace_t *workspace);

uint32_t usb_management_runtime_service(
    const binary_rpc_binding_t *binding);

#endif
