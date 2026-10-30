#ifndef DRIVERS_USB_DEVICE_H
#define DRIVERS_USB_DEVICE_H

#include <stdint.h>

/*
 * PRIVATE TEST IDENTITY ONLY. This VID/PID pair is not globally unique for
 * this project and must not be used on redistributed, sold, or manufactured
 * devices. See docs/USB_IDENTITY_POLICY.md before any release.
 */
#define USB_DEVICE_DEVELOPMENT_VID 0x1209u
#define USB_DEVICE_DEVELOPMENT_PID 0x000Eu

#define USB_MANAGEMENT_INTERFACE_NUMBER 0u
#define USB_MANAGEMENT_OUT_ENDPOINT     0x04u
#define USB_MANAGEMENT_IN_ENDPOINT      0x84u
#define USB_MANAGEMENT_MAX_PACKET       64u

typedef void (*usb_management_rx_notify_t)(void);

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

    uint32_t management_configured;
    uint32_t management_rx_packet_count;
    uint32_t management_rx_byte_count;
    uint32_t management_rx_drop_count;
    uint32_t management_rx_high_water;
    uint32_t management_tx_packet_count;
    uint32_t management_tx_byte_count;
    uint32_t management_tx_drop_count;
    uint32_t management_tx_high_water;
    uint32_t management_ms_os_20_request_count;
} usb_device_diagnostics_t;

extern volatile usb_device_diagnostics_t usb_device_diagnostics;

int usb_device_init(uint32_t core_clock_hz);

void usb_management_set_rx_notify(usb_management_rx_notify_t notify);
int usb_management_is_configured(void);
int usb_management_try_getc(uint8_t *byte_out);
int usb_management_write_span_atomic(const uint8_t *data, uint32_t length);

void USB_LP_CAN1_RX0_IRQHandler(void);

#endif
