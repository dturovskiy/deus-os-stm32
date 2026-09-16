#include <stdint.h>
#include "drivers/usb_device.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG16(addr) (*(volatile uint16_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

/* RCC */
#define RCC_CFGR        REG32(0x40021004u)
#define RCC_APB1RSTR    REG32(0x40021010u)
#define RCC_APB1ENR     REG32(0x4002101Cu)

#define RCC_CFGR_SWS_MASK     (0x3u << 2)
#define RCC_CFGR_SWS_PLL      (0x2u << 2)
#define RCC_CFGR_USBPRE       (1u << 22)
#define RCC_APB1_USBRST       (1u << 23)
#define RCC_APB1_USBEN        (1u << 23)

/* NVIC: USB_LP_CAN1_RX0 is external IRQ20 on STM32F103 medium density. */
#define NVIC_ISER0            REG32(0xE000E100u)
#define NVIC_ICPR0            REG32(0xE000E280u)
#define NVIC_IPR_USB_LP       REG8(0xE000E414u)
#define NVIC_USB_LP_BIT       (1u << 20)
#define NVIC_USB_LP_PRIORITY  0x80u

/* STM32F103 USB FS device register block / packet memory. */
#define USB_BASE              0x40005C00u
#define USB_PMA_BASE          0x40006000u

/* Dedicated USB FS Device pins: PA11=USB_DM, PA12=USB_DP. */

#define USB_EP_REG(ep)        REG16(USB_BASE + ((uint32_t)(ep) * 4u))
#define USB_CNTR              REG16(USB_BASE + 0x40u)
#define USB_ISTR              REG16(USB_BASE + 0x44u)
#define USB_DADDR             REG16(USB_BASE + 0x4Cu)
#define USB_BTABLE            REG16(USB_BASE + 0x50u)

#define USB_CNTR_FRES         (1u << 0)
#define USB_CNTR_PDWN         (1u << 1)
#define USB_CNTR_RESETM       (1u << 10)
#define USB_CNTR_ERRM         (1u << 13)
#define USB_CNTR_PMAOVRM      (1u << 14)
#define USB_CNTR_CTRM         (1u << 15)

#define USB_ISTR_RESET        (1u << 10)
#define USB_ISTR_ERR          (1u << 13)
#define USB_ISTR_PMAOVR       (1u << 14)
#define USB_ISTR_CTR          (1u << 15)
#define USB_ISTR_EP_ID_MASK   0x000Fu

#define USB_DADDR_EF          (1u << 7)
#define USB_DADDR_ADD_MASK    0x007Fu

/* USB_EPnR fields. Toggle fields are never modified with generic RMW. */
#define USB_EP_CTR_RX         (1u << 15)
#define USB_EP_DTOG_RX        (1u << 14)
#define USB_EP_STAT_RX_MASK   (0x3u << 12)
#define USB_EP_SETUP          (1u << 11)
#define USB_EP_TYPE_MASK      (0x3u << 9)
#define USB_EP_TYPE_CONTROL   (0x1u << 9)
#define USB_EP_KIND           (1u << 8)
#define USB_EP_CTR_TX         (1u << 7)
#define USB_EP_DTOG_TX        (1u << 6)
#define USB_EP_STAT_TX_MASK   (0x3u << 4)
#define USB_EP_EA_MASK        0x000Fu

#define USB_EP_STAT_TX_DISABLED (0x0u << 4)
#define USB_EP_STAT_TX_STALL    (0x1u << 4)
#define USB_EP_STAT_TX_NAK      (0x2u << 4)
#define USB_EP_STAT_TX_VALID    (0x3u << 4)

#define USB_EP_STAT_RX_DISABLED (0x0u << 12)
#define USB_EP_STAT_RX_STALL    (0x1u << 12)
#define USB_EP_STAT_RX_NAK      (0x2u << 12)
#define USB_EP_STAT_RX_VALID    (0x3u << 12)

#define USB_EP0                 0u
#define USB_EP0_MAX_PACKET      64u

/*
 * PMA local byte addresses. CPU-visible PMA addresses use a doubled stride:
 * each 16-bit USB-local word occupies one 32-bit APB address slot.
 */
#define USB_BTABLE_LOCAL        0x000u
#define USB_EP0_TX_LOCAL        0x040u
#define USB_EP0_RX_LOCAL        0x080u

#define USB_BTABLE_EP0_TX_ADDR  (USB_BTABLE_LOCAL + 0u)
#define USB_BTABLE_EP0_TX_COUNT (USB_BTABLE_LOCAL + 2u)
#define USB_BTABLE_EP0_RX_ADDR  (USB_BTABLE_LOCAL + 4u)
#define USB_BTABLE_EP0_RX_COUNT (USB_BTABLE_LOCAL + 6u)

/* 64-byte receive buffer: BL_SIZE=1 (32-byte blocks), NUM_BLOCK=1. */
#define USB_RX_COUNT_64         0x8400u
#define USB_COUNT_MASK          0x03FFu
#define USB_PMA_LOGICAL_BYTES   512u

/* USB standard request fields. */
#define USB_REQ_DIR_IN          0x80u
#define USB_REQ_TYPE_MASK       0x60u
#define USB_REQ_TYPE_STANDARD   0x00u
#define USB_REQ_RECIP_MASK      0x1Fu
#define USB_REQ_RECIP_DEVICE    0x00u
#define USB_REQ_RECIP_INTERFACE 0x01u
#define USB_REQ_RECIP_ENDPOINT  0x02u

#define USB_REQ_GET_STATUS        0u
#define USB_REQ_CLEAR_FEATURE     1u
#define USB_REQ_SET_FEATURE       3u
#define USB_REQ_SET_ADDRESS       5u
#define USB_REQ_GET_DESCRIPTOR    6u
#define USB_REQ_GET_CONFIGURATION 8u
#define USB_REQ_SET_CONFIGURATION 9u
#define USB_REQ_GET_INTERFACE     10u
#define USB_REQ_SET_INTERFACE     11u

#define USB_DESC_DEVICE        1u
#define USB_DESC_CONFIGURATION 2u
#define USB_DESC_STRING        3u

#define USB_CONFIGURATION_NONE 0u
#define USB_CONFIGURATION_ONE  1u

typedef struct
{
    uint8_t bm_request_type;
    uint8_t b_request;
    uint16_t w_value;
    uint16_t w_index;
    uint16_t w_length;
} usb_setup_packet_t;

typedef enum
{
    USB_EP0_IDLE = 0,
    USB_EP0_DATA_IN,
    USB_EP0_STATUS_OUT,
    USB_EP0_STATUS_IN
} usb_ep0_state_t;

static const uint8_t usb_device_descriptor[] =
{
    18u, USB_DESC_DEVICE,
    0x00u, 0x02u,             /* bcdUSB 2.00 */
    0x00u,                    /* class: per-interface */
    0x00u,
    0x00u,
    USB_EP0_MAX_PACKET,
    (uint8_t)(USB_DEVICE_DEVELOPMENT_VID & 0xFFu),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_VID >> 8),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_PID & 0xFFu),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_PID >> 8),
    0x00u, 0x01u,             /* bcdDevice 1.00 */
    0u,                       /* no manufacturer string */
    1u,                       /* development product string */
    0u,                       /* no serial number */
    1u                        /* one configuration */
};

static const uint8_t usb_configuration_descriptor[] =
{
    /* Configuration descriptor. */
    9u, USB_DESC_CONFIGURATION,
    18u, 0u,                  /* wTotalLength */
    1u,                       /* one interface */
    USB_CONFIGURATION_ONE,
    0u,                       /* no configuration string */
    0x80u,                    /* bus powered */
    50u,                      /* 100 mA */

    /* Minimal vendor-specific interface; no non-control endpoints yet. */
    9u, 4u,                   /* interface descriptor */
    0u,                       /* interface number */
    0u,                       /* alternate setting */
    0u,                       /* no data endpoints */
    0xFFu,                    /* vendor-specific */
    0u,
    0u,
    0u
};

static const uint8_t usb_string_language[] =
{
    4u, USB_DESC_STRING, 0x09u, 0x04u /* en-US */
};

static const uint8_t usb_string_product[] =
{
    34u, USB_DESC_STRING,
    'D', 0u, 'e', 0u, 'u', 0u, 's', 0u,
    ' ', 0u, 'O', 0u, 'S', 0u, ' ', 0u,
    'U', 0u, 'S', 0u, 'B', 0u, ' ', 0u,
    'C', 0u, 'o', 0u, 'r', 0u, 'e', 0u
};

_Static_assert(sizeof(usb_device_descriptor) == 18u,
    "USB device descriptor must be 18 bytes");
_Static_assert(sizeof(usb_configuration_descriptor) == 18u,
    "USB configuration tree must be 18 bytes");
_Static_assert(sizeof(usb_string_language) == 4u,
    "USB language string descriptor must be 4 bytes");
_Static_assert(sizeof(usb_string_product) == 34u,
    "USB product string descriptor must be 34 bytes");
_Static_assert((USB_BTABLE_LOCAL & 0x7u) == 0u,
    "USB BTABLE must be 8-byte aligned");
_Static_assert((USB_EP0_RX_LOCAL + USB_EP0_MAX_PACKET) <= USB_PMA_LOGICAL_BYTES,
    "USB EP0 PMA allocation exceeds STM32F103 packet memory");

volatile usb_device_diagnostics_t usb_device_diagnostics;

static usb_ep0_state_t usb_ep0_state;
static const uint8_t *usb_ep0_in_data;
static uint16_t usb_ep0_in_remaining;
static uint8_t usb_ep0_need_zlp;
static uint8_t usb_pending_address;
static uint8_t usb_pending_address_valid;
static uint8_t usb_pending_configuration;
static uint8_t usb_pending_configuration_valid;

static void usb_pma_write16(uint16_t local_byte_offset, uint16_t value)
{
    REG16(USB_PMA_BASE + ((uint32_t)local_byte_offset * 2u)) = value;
}

static uint16_t usb_pma_read16(uint16_t local_byte_offset)
{
    return REG16(USB_PMA_BASE + ((uint32_t)local_byte_offset * 2u));
}

static void usb_pma_write(
    uint16_t local_byte_offset,
    const uint8_t *source,
    uint16_t length)
{
    uint16_t index;

    for (index = 0u; index < length; index = (uint16_t)(index + 2u))
    {
        uint16_t word = source[index];

        if ((uint16_t)(index + 1u) < length)
        {
            word |= (uint16_t)((uint16_t)source[index + 1u] << 8);
        }

        usb_pma_write16(
            (uint16_t)(local_byte_offset + index),
            word);
    }
}

static void usb_pma_read(
    uint16_t local_byte_offset,
    uint8_t *destination,
    uint16_t length)
{
    uint16_t index;

    for (index = 0u; index < length; index = (uint16_t)(index + 2u))
    {
        const uint16_t word =
            usb_pma_read16(
                (uint16_t)(local_byte_offset + index));

        destination[index] = (uint8_t)(word & 0xFFu);

        if ((uint16_t)(index + 1u) < length)
        {
            destination[index + 1u] =
                (uint8_t)(word >> 8);
        }
    }
}

static uint16_t usb_ep_invariant(uint16_t value)
{
    return (uint16_t)(
        value &
        (USB_EP_CTR_RX |
         USB_EP_TYPE_MASK |
         USB_EP_KIND |
         USB_EP_CTR_TX |
         USB_EP_EA_MASK));
}

static void usb_ep_set_tx_status(
    uint8_t endpoint,
    uint16_t status)
{
    const uint16_t current = USB_EP_REG(endpoint);
    uint16_t write_value = usb_ep_invariant(current);

    write_value |=
        (uint16_t)(
            (current ^ status) &
            USB_EP_STAT_TX_MASK);

    USB_EP_REG(endpoint) = write_value;
}

static void usb_ep_set_rx_status(
    uint8_t endpoint,
    uint16_t status)
{
    const uint16_t current = USB_EP_REG(endpoint);
    uint16_t write_value = usb_ep_invariant(current);

    write_value |=
        (uint16_t)(
            (current ^ status) &
            USB_EP_STAT_RX_MASK);

    USB_EP_REG(endpoint) = write_value;
}

static void usb_ep_clear_ctr_rx(uint8_t endpoint)
{
    const uint16_t current = USB_EP_REG(endpoint);
    uint16_t write_value = usb_ep_invariant(current);

    write_value &= (uint16_t)~USB_EP_CTR_RX;
    USB_EP_REG(endpoint) = write_value;
}

static void usb_ep_clear_ctr_tx(uint8_t endpoint)
{
    const uint16_t current = USB_EP_REG(endpoint);
    uint16_t write_value = usb_ep_invariant(current);

    write_value &= (uint16_t)~USB_EP_CTR_TX;
    USB_EP_REG(endpoint) = write_value;
}

static void usb_istr_clear(uint16_t flags)
{
    /*
     * Event bits are rc_w0. Write 0 only to serviced flags and 1 elsewhere;
     * never use a read-modify-write cycle on USB_ISTR.
     */
    USB_ISTR = (uint16_t)~flags;
}

static void usb_ep0_set_tx_count(uint16_t count)
{
    usb_pma_write16(
        USB_BTABLE_EP0_TX_COUNT,
        (uint16_t)(count & USB_COUNT_MASK));
}

static uint16_t usb_ep0_get_rx_count(void)
{
    return (uint16_t)(
        usb_pma_read16(USB_BTABLE_EP0_RX_COUNT) &
        USB_COUNT_MASK);
}

static void usb_ep0_reset_transfer_state(void)
{
    usb_ep0_state = USB_EP0_IDLE;
    usb_ep0_in_data = (const uint8_t *)0;
    usb_ep0_in_remaining = 0u;
    usb_ep0_need_zlp = 0u;
    usb_pending_address = 0u;
    usb_pending_address_valid = 0u;
    usb_pending_configuration = 0u;
    usb_pending_configuration_valid = 0u;
}

static void usb_ep0_stall(void)
{
    usb_ep0_reset_transfer_state();
    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_STALL);
    usb_ep_set_rx_status(
        USB_EP0,
        USB_EP_STAT_RX_STALL);
    ++usb_device_diagnostics.stall_count;
}

static void usb_ep0_arm_rx(void)
{
    usb_ep_set_rx_status(
        USB_EP0,
        USB_EP_STAT_RX_VALID);
}

static void usb_ep0_send_packet(
    const uint8_t *data,
    uint16_t length)
{
    if (length != 0u)
    {
        usb_pma_write(
            USB_EP0_TX_LOCAL,
            data,
            length);
    }

    usb_ep0_set_tx_count(length);
    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_VALID);
}

static void usb_ep0_send_next_in_packet(void)
{
    uint16_t length = usb_ep0_in_remaining;

    if (length > USB_EP0_MAX_PACKET)
    {
        length = USB_EP0_MAX_PACKET;
    }

    usb_ep0_send_packet(
        usb_ep0_in_data,
        length);

    usb_ep0_in_data += length;
    usb_ep0_in_remaining =
        (uint16_t)(usb_ep0_in_remaining - length);
}

static void usb_ep0_start_in(
    const uint8_t *data,
    uint16_t data_length,
    uint16_t requested_length)
{
    uint16_t send_length = data_length;

    if (send_length > requested_length)
    {
        send_length = requested_length;
    }

    usb_ep0_in_data = data;
    usb_ep0_in_remaining = send_length;
    usb_ep0_need_zlp =
        (uint8_t)(
            (send_length != 0u) &&
            (send_length < requested_length) &&
            ((send_length % USB_EP0_MAX_PACKET) == 0u));

    usb_ep0_state = USB_EP0_DATA_IN;

    if (send_length == 0u)
    {
        usb_ep0_send_packet(
            (const uint8_t *)0,
            0u);
    }
    else
    {
        usb_ep0_send_next_in_packet();
    }
}

static void usb_ep0_start_status_in(void)
{
    usb_ep0_state = USB_EP0_STATUS_IN;
    usb_ep0_send_packet(
        (const uint8_t *)0,
        0u);
}

static int usb_get_descriptor(
    const usb_setup_packet_t *setup,
    const uint8_t **data,
    uint16_t *length)
{
    const uint8_t descriptor_type =
        (uint8_t)(setup->w_value >> 8);
    const uint8_t descriptor_index =
        (uint8_t)(setup->w_value & 0xFFu);

    if ((setup->bm_request_type != 0x80u) || (data == (const uint8_t **)0) ||
        (length == (uint16_t *)0))
    {
        return 0;
    }

    if ((descriptor_type == USB_DESC_DEVICE) && (descriptor_index == 0u))
    {
        *data = usb_device_descriptor;
        *length = (uint16_t)sizeof(usb_device_descriptor);
        return 1;
    }

    if ((descriptor_type == USB_DESC_CONFIGURATION) &&
        (descriptor_index == 0u))
    {
        *data = usb_configuration_descriptor;
        *length = (uint16_t)sizeof(usb_configuration_descriptor);
        return 1;
    }

    if (descriptor_type == USB_DESC_STRING)
    {
        if (descriptor_index == 0u)
        {
            *data = usb_string_language;
            *length = (uint16_t)sizeof(usb_string_language);
            return 1;
        }

        if (
            (descriptor_index == 1u) &&
            ((setup->w_index == 0x0409u) || (setup->w_index == 0u))
        ) {
            *data = usb_string_product;
            *length = (uint16_t)sizeof(usb_string_product);
            return 1;
        }
    }

    return 0;
}

static int usb_ep0_standard_in(
    const usb_setup_packet_t *setup)
{
    static const uint8_t zero_status[2] = { 0u, 0u };
    static uint8_t one_byte_response;
    const uint8_t *descriptor;
    uint16_t descriptor_length;
    const uint8_t recipient =
        (uint8_t)(setup->bm_request_type & USB_REQ_RECIP_MASK);

    if ((setup->bm_request_type & USB_REQ_TYPE_MASK) != USB_REQ_TYPE_STANDARD)
    {
        return 0;
    }

    if (setup->b_request == USB_REQ_GET_DESCRIPTOR)
    {
        if (
            usb_get_descriptor(
                setup,
                &descriptor,
                &descriptor_length) == 0
        ) {
            return 0;
        }

        usb_ep0_start_in(
            descriptor,
            descriptor_length,
            setup->w_length);
        return 1;
    }

    if (
        (setup->b_request == USB_REQ_GET_STATUS) &&
        (setup->w_value == 0u) &&
        (setup->w_length == 2u)
    ) {
        if (
            (recipient == USB_REQ_RECIP_DEVICE) &&
            (setup->w_index == 0u)
        ) {
            usb_ep0_start_in(
                zero_status,
                2u,
                setup->w_length);
            return 1;
        }

        if (
            (recipient == USB_REQ_RECIP_INTERFACE) &&
            (usb_device_diagnostics.configuration ==
                USB_CONFIGURATION_ONE) &&
            (setup->w_index == 0u)
        ) {
            usb_ep0_start_in(
                zero_status,
                2u,
                setup->w_length);
            return 1;
        }

        if (
            (recipient == USB_REQ_RECIP_ENDPOINT) &&
            ((setup->w_index == 0x0000u) ||
             (setup->w_index == 0x0080u))
        ) {
            usb_ep0_start_in(
                zero_status,
                2u,
                setup->w_length);
            return 1;
        }

        return 0;
    }

    if (
        (setup->b_request == USB_REQ_GET_CONFIGURATION) &&
        (setup->bm_request_type == 0x80u) &&
        (setup->w_value == 0u) &&
        (setup->w_index == 0u) &&
        (setup->w_length == 1u)
    ) {
        one_byte_response =
            (uint8_t)usb_device_diagnostics.configuration;

        usb_ep0_start_in(
            &one_byte_response,
            1u,
            setup->w_length);
        return 1;
    }

    if (
        (setup->b_request == USB_REQ_GET_INTERFACE) &&
        (setup->bm_request_type == 0x81u) &&
        (setup->w_value == 0u) &&
        (setup->w_index == 0u) &&
        (setup->w_length == 1u) &&
        (usb_device_diagnostics.configuration ==
            USB_CONFIGURATION_ONE)
    ) {
        one_byte_response = 0u;

        usb_ep0_start_in(
            &one_byte_response,
            1u,
            setup->w_length);
        return 1;
    }

    return 0;
}

static int usb_ep0_standard_out(
    const usb_setup_packet_t *setup)
{
    if ((setup->bm_request_type & USB_REQ_TYPE_MASK) != USB_REQ_TYPE_STANDARD)
    {
        return 0;
    }

    if (
        (setup->b_request == USB_REQ_SET_ADDRESS) &&
        (setup->bm_request_type == 0x00u) &&
        ((setup->w_value & 0xFF80u) == 0u) &&
        (setup->w_index == 0u) &&
        (setup->w_length == 0u)
    ) {
        usb_pending_address =
            (uint8_t)(setup->w_value & USB_DADDR_ADD_MASK);
        usb_pending_address_valid = 1u;
        usb_ep0_start_status_in();
        return 1;
    }

    if (
        (setup->b_request == USB_REQ_SET_CONFIGURATION) &&
        (setup->bm_request_type == 0x00u) &&
        ((setup->w_value == USB_CONFIGURATION_NONE) ||
         (setup->w_value == USB_CONFIGURATION_ONE)) &&
        (setup->w_index == 0u) &&
        (setup->w_length == 0u)
    ) {
        usb_pending_configuration =
            (uint8_t)setup->w_value;
        usb_pending_configuration_valid = 1u;
        usb_ep0_start_status_in();
        return 1;
    }

    if (
        (setup->b_request == USB_REQ_SET_INTERFACE) &&
        (setup->bm_request_type == 0x01u) &&
        (setup->w_value == 0u) &&
        (setup->w_index == 0u) &&
        (setup->w_length == 0u) &&
        (usb_device_diagnostics.configuration ==
            USB_CONFIGURATION_ONE)
    ) {
        usb_ep0_start_status_in();
        return 1;
    }

    /*
     * No feature selector is implemented in this foundation. Reject
     * CLEAR_FEATURE / SET_FEATURE deterministically instead of silently
     * accepting semantics the core does not own.
     */
    if (
        (setup->b_request == USB_REQ_CLEAR_FEATURE) ||
        (setup->b_request == USB_REQ_SET_FEATURE)
    ) {
        return 0;
    }

    return 0;
}

static void usb_ep0_handle_setup(void)
{
    usb_setup_packet_t setup;
    uint8_t raw[8];

    usb_pma_read(
        USB_EP0_RX_LOCAL,
        raw,
        (uint16_t)sizeof(raw));

    setup.bm_request_type = raw[0];
    setup.b_request = raw[1];
    setup.w_value =
        (uint16_t)((uint16_t)raw[2] | ((uint16_t)raw[3] << 8));
    setup.w_index =
        (uint16_t)((uint16_t)raw[4] | ((uint16_t)raw[5] << 8));
    setup.w_length =
        (uint16_t)((uint16_t)raw[6] | ((uint16_t)raw[7] << 8));

    ++usb_device_diagnostics.setup_count;
    usb_device_diagnostics.last_bm_request_type =
        setup.bm_request_type;
    usb_device_diagnostics.last_b_request =
        setup.b_request;

    usb_ep0_reset_transfer_state();

    /* A new SETUP token recovers EP0 from a previous protocol stall. */
    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_NAK);
    usb_ep0_arm_rx();

    if ((setup.bm_request_type & USB_REQ_DIR_IN) != 0u)
    {
        if (usb_ep0_standard_in(&setup) == 0)
        {
            usb_ep0_stall();
        }
    }
    else
    {
        if (usb_ep0_standard_out(&setup) == 0)
        {
            usb_ep0_stall();
        }
    }
}

static void usb_ep0_handle_rx(uint16_t endpoint_value)
{
    const uint16_t count = usb_ep0_get_rx_count();

    usb_ep_clear_ctr_rx(USB_EP0);
    ++usb_device_diagnostics.transfer_count;

    if ((endpoint_value & USB_EP_SETUP) != 0u)
    {
        if (count == 8u)
        {
            usb_ep0_handle_setup();
        }
        else
        {
            usb_ep0_stall();
        }

        return;
    }

    if ((usb_ep0_state == USB_EP0_STATUS_OUT) && (count == 0u))
    {
        usb_ep0_state = USB_EP0_IDLE;
        usb_ep_set_tx_status(
            USB_EP0,
            USB_EP_STAT_TX_NAK);
        usb_ep0_arm_rx();
        return;
    }

    usb_ep0_stall();
}

static void usb_ep0_apply_pending_status_action(void)
{
    if (usb_pending_address_valid != 0u)
    {
        USB_DADDR =
            (uint16_t)(
                USB_DADDR_EF |
                usb_pending_address);

        usb_device_diagnostics.address =
            usb_pending_address;
        usb_pending_address_valid = 0u;

        if (usb_pending_address == 0u)
        {
            usb_device_diagnostics.configuration =
                USB_CONFIGURATION_NONE;
        }
    }

    if (usb_pending_configuration_valid != 0u)
    {
        usb_device_diagnostics.configuration =
            usb_pending_configuration;
        usb_pending_configuration_valid = 0u;
    }
}

static void usb_ep0_handle_tx(void)
{
    usb_ep_clear_ctr_tx(USB_EP0);
    ++usb_device_diagnostics.transfer_count;

    if (usb_ep0_state == USB_EP0_DATA_IN)
    {
        if (usb_ep0_in_remaining != 0u)
        {
            usb_ep0_send_next_in_packet();
            return;
        }

        if (usb_ep0_need_zlp != 0u)
        {
            usb_ep0_need_zlp = 0u;
            usb_ep0_send_packet(
                (const uint8_t *)0,
                0u);
            return;
        }

        usb_ep0_state = USB_EP0_STATUS_OUT;
        usb_ep_set_tx_status(
            USB_EP0,
            USB_EP_STAT_TX_NAK);
        usb_ep0_arm_rx();
        return;
    }

    if (usb_ep0_state == USB_EP0_STATUS_IN)
    {
        usb_ep0_apply_pending_status_action();
        usb_ep0_state = USB_EP0_IDLE;
        usb_ep_set_tx_status(
            USB_EP0,
            USB_EP_STAT_TX_NAK);
        usb_ep0_arm_rx();
        return;
    }

    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_NAK);
}

static void usb_ep0_handle_ctr(void)
{
    uint16_t endpoint_value = USB_EP_REG(USB_EP0);

    /*
     * RX first: SETUP has transaction-reset semantics and takes precedence
     * over a stale TX completion from the previous control transfer.
     */
    if ((endpoint_value & USB_EP_CTR_RX) != 0u)
    {
        usb_ep0_handle_rx(endpoint_value);
    }

    endpoint_value = USB_EP_REG(USB_EP0);

    if ((endpoint_value & USB_EP_CTR_TX) != 0u)
    {
        usb_ep0_handle_tx();
    }
}

static void usb_bus_reset(void)
{
    /*
     * USB bus reset/FRES resets endpoint configuration/status. CTR flags are
     * explicitly cleared here because RM0008 documents them as preserved.
     */
    USB_EP_REG(USB_EP0) = 0u;

    USB_BTABLE = USB_BTABLE_LOCAL;

    usb_pma_write16(
        USB_BTABLE_EP0_TX_ADDR,
        USB_EP0_TX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP0_TX_COUNT,
        0u);
    usb_pma_write16(
        USB_BTABLE_EP0_RX_ADDR,
        USB_EP0_RX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP0_RX_COUNT,
        USB_RX_COUNT_64);

    USB_EP_REG(USB_EP0) =
        (uint16_t)(USB_EP_TYPE_CONTROL | USB_EP0);

    usb_ep0_reset_transfer_state();

    usb_device_diagnostics.address = 0u;
    usb_device_diagnostics.configuration =
        USB_CONFIGURATION_NONE;

    USB_DADDR = USB_DADDR_EF;

    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_NAK);
    usb_ep0_arm_rx();

    ++usb_device_diagnostics.bus_reset_count;
}

static void usb_transceiver_startup_delay(uint32_t core_clock_hz)
{
    /*
     * RM0008 requires waiting tSTARTUP while FRES remains asserted after the
     * transceiver leaves PDWN. Use a deliberately conservative bounded spin:
     * at 72 MHz the loop budget is far above one microsecond.
     */
    volatile uint32_t spin;
    const uint32_t iterations =
        (core_clock_hz / 1000000u) * 4u;

    for (spin = 0u; spin < iterations; ++spin)
    {
        __asm volatile ("nop");
    }
}

int usb_device_init(uint32_t core_clock_hz)
{
    if (
        (core_clock_hz != 72000000u) ||
        ((RCC_CFGR & RCC_CFGR_SWS_MASK) != RCC_CFGR_SWS_PLL) ||
        ((RCC_CFGR & RCC_CFGR_USBPRE) != 0u)
    ) {
        return 0;
    }

    usb_device_diagnostics.initialized = 0u;
    usb_device_diagnostics.bus_reset_count = 0u;
    usb_device_diagnostics.setup_count = 0u;
    usb_device_diagnostics.transfer_count = 0u;
    usb_device_diagnostics.stall_count = 0u;
    usb_device_diagnostics.error_count = 0u;
    usb_device_diagnostics.pma_overrun_count = 0u;
    usb_device_diagnostics.address = 0u;
    usb_device_diagnostics.configuration = USB_CONFIGURATION_NONE;
    usb_device_diagnostics.last_bm_request_type = 0u;
    usb_device_diagnostics.last_b_request = 0u;

    usb_ep0_reset_transfer_state();

    RCC_APB1ENR |= RCC_APB1_USBEN;
    RCC_APB1RSTR |= RCC_APB1_USBRST;
    RCC_APB1RSTR &= ~RCC_APB1_USBRST;

    /*
     * Keep the digital USB core forced in reset while bringing the analog
     * transceiver out of power-down, then wait tSTARTUP before releasing FRES.
     */
    USB_CNTR = (uint16_t)(USB_CNTR_FRES | USB_CNTR_PDWN);
    USB_CNTR = USB_CNTR_FRES;
    usb_transceiver_startup_delay(core_clock_hz);
    USB_CNTR = 0u;

    /* Direct write clears all pending rc_w0 USB interrupt event bits. */
    USB_ISTR = 0u;
    USB_BTABLE = USB_BTABLE_LOCAL;
    USB_DADDR = 0u;

    NVIC_ICPR0 = NVIC_USB_LP_BIT;
    NVIC_IPR_USB_LP = NVIC_USB_LP_PRIORITY;

    USB_CNTR =
        (uint16_t)(
            USB_CNTR_CTRM |
            USB_CNTR_RESETM |
            USB_CNTR_ERRM |
            USB_CNTR_PMAOVRM);

    NVIC_ISER0 = NVIC_USB_LP_BIT;

    usb_device_diagnostics.initialized = 1u;
    return 1;
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    uint16_t status = USB_ISTR;

    if ((status & USB_ISTR_RESET) != 0u)
    {
        usb_istr_clear(USB_ISTR_RESET);
        usb_bus_reset();
    }

    while ((USB_ISTR & USB_ISTR_CTR) != 0u)
    {
        const uint16_t current = USB_ISTR;
        const uint8_t endpoint =
            (uint8_t)(current & USB_ISTR_EP_ID_MASK);

        if (endpoint == USB_EP0)
        {
            usb_ep0_handle_ctr();
        }
        else
        {
            /*
             * This foundation owns only EP0. No other endpoint is enabled;
             * fail closed if unexpected hardware state reports one.
             */
            USB_EP_REG(endpoint) = 0u;
            ++usb_device_diagnostics.error_count;
        }
    }

    status = USB_ISTR;

    if ((status & USB_ISTR_PMAOVR) != 0u)
    {
        usb_istr_clear(USB_ISTR_PMAOVR);
        ++usb_device_diagnostics.pma_overrun_count;
    }

    if ((status & USB_ISTR_ERR) != 0u)
    {
        usb_istr_clear(USB_ISTR_ERR);
        ++usb_device_diagnostics.error_count;
    }
}
