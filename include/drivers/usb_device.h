#ifndef DRIVERS_USB_DEVICE_H
#define DRIVERS_USB_DEVICE_H

#include <stdint.h>

/*
 * PRIVATE TEST IDENTITY ONLY. This VID/PID pair is not globally unique for
 * this project and must not be used on redistributed, sold, or manufactured
 * devices. See docs/USB_IDENTITY_POLICY.md before any release.
 */
#define USB_DEVICE_DEVELOPMENT_VID 0x1209u
#define USB_DEVICE_DEVELOPMENT_PID 0x000Au

typedef struct
{
    uint32_t initialized;
    uint32_t bus_reset_count;
    uint32_t setup_count;
    uint32_t transfer_count;
    uint32_t stall_count;
    uint32_t error_count;
    uint32_t pma_overrun_count;
    uint32_t address;
    uint32_t configuration;
    uint32_t last_bm_request_type;
    uint32_t last_b_request;
} usb_device_diagnostics_t;

extern volatile usb_device_diagnostics_t usb_device_diagnostics;

int usb_device_init(uint32_t core_clock_hz);
void USB_LP_CAN1_RX0_IRQHandler(void);

#endif
