#include <stdint.h>
#include "drivers/usb_device.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG16(addr) (*(volatile uint16_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

/* RCC */
#define RCC_CFGR        REG32(0x40021004u)
#define RCC_APB1RSTR    REG32(0x40021010u)
#define RCC_APB2ENR     REG32(0x40021018u)
#define RCC_APB1ENR     REG32(0x4002101Cu)

/* GPIOA: PA12 is temporarily owned during boot to force USB disconnect. */
#define GPIOA_CRH       REG32(0x40010804u)
#define GPIOA_BRR       REG32(0x40010814u)

#define RCC_CFGR_SWS_MASK     (0x3u << 2)
#define RCC_CFGR_SWS_PLL      (0x2u << 2)
#define RCC_CFGR_USBPRE       (1u << 22)
#define RCC_APB2_IOPAEN       (1u << 2)
#define RCC_APB1_USBRST       (1u << 23)
#define RCC_APB1_USBEN        (1u << 23)

#define GPIO_PIN_12                   (1u << 12)
#define GPIO_CRH_PA12_SHIFT           16u
#define GPIO_CRH_PA12_MASK            (0xFu << GPIO_CRH_PA12_SHIFT)
#define GPIO_CRH_PA12_OUTPUT_OD_2MHZ  (0x6u << GPIO_CRH_PA12_SHIFT)

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
#define USB_EP_TYPE_BULK      (0x0u << 9)
#define USB_EP_TYPE_CONTROL   (0x1u << 9)
#define USB_EP_TYPE_INTERRUPT (0x3u << 9)
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
#define USB_CDC_NOTIFY_EP       1u
#define USB_CDC_OUT_EP          2u
#define USB_CDC_IN_EP           3u
#define USB_EP0_MAX_PACKET      64u
#define USB_CDC_DATA_MAX_PACKET 64u
#define USB_CDC_NOTIFY_PACKET   16u

/*
 * PMA local byte addresses. CPU-visible PMA addresses use a doubled stride:
 * each 16-bit USB-local word occupies one 32-bit APB address slot.
 */
#define USB_BTABLE_LOCAL        0x000u
#define USB_EP0_TX_LOCAL        0x040u
#define USB_EP0_RX_LOCAL        0x080u
#define USB_EP1_TX_LOCAL        0x0C0u
#define USB_EP2_RX_LOCAL        0x100u
#define USB_EP3_TX_LOCAL        0x140u

#define USB_BTABLE_EP_TX_ADDR(ep) \
    (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 0u)
#define USB_BTABLE_EP_TX_COUNT(ep) \
    (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 2u)
#define USB_BTABLE_EP_RX_ADDR(ep) \
    (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 4u)
#define USB_BTABLE_EP_RX_COUNT(ep) \
    (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 6u)

/* 64-byte receive buffer: BL_SIZE=1 (32-byte blocks), NUM_BLOCK=1. */
#define USB_RX_COUNT_64         0x8400u
#define USB_COUNT_MASK          0x03FFu
#define USB_PMA_LOGICAL_BYTES   512u

#define USB_CDC_RX_RING_CAPACITY 1024u
#define USB_CDC_RX_RING_MASK     (USB_CDC_RX_RING_CAPACITY - 1u)
#define USB_CDC_TX_RING_CAPACITY 2048u
#define USB_CDC_TX_RING_MASK     (USB_CDC_TX_RING_CAPACITY - 1u)

/* USB standard request fields. */
#define USB_REQ_DIR_IN          0x80u
#define USB_REQ_TYPE_MASK       0x60u
#define USB_REQ_TYPE_STANDARD   0x00u
#define USB_REQ_TYPE_CLASS      0x20u
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

#define USB_CDC_REQ_SET_LINE_CODING        0x20u
#define USB_CDC_REQ_GET_LINE_CODING        0x21u
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE 0x22u
#define USB_CDC_NOTIFICATION_SERIAL_STATE   0x20u

#define USB_DESC_DEVICE        1u
#define USB_DESC_CONFIGURATION 2u
#define USB_DESC_STRING        3u

#define USB_CONFIGURATION_NONE 0u
#define USB_CONFIGURATION_ONE  1u
#define USB_CDC_CONTROL_INTERFACE 0u
#define USB_CDC_DATA_INTERFACE    1u

#define USB_EP0_OUT_NONE            0u
#define USB_EP0_OUT_SET_LINE_CODING 1u

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
    USB_EP0_DATA_OUT,
    USB_EP0_STATUS_OUT,
    USB_EP0_STATUS_IN
} usb_ep0_state_t;

static const uint8_t usb_device_descriptor[] =
{
    18u, USB_DESC_DEVICE,
    0x00u, 0x02u,             /* bcdUSB 2.00 */
    0x02u,                    /* CDC Communications Device Class */
    0x02u,                    /* Abstract Control Model */
    0x00u,
    USB_EP0_MAX_PACKET,
    (uint8_t)(USB_DEVICE_DEVELOPMENT_VID & 0xFFu),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_VID >> 8),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_PID & 0xFFu),
    (uint8_t)(USB_DEVICE_DEVELOPMENT_PID >> 8),
    0x01u, 0x01u,             /* bcdDevice 1.01 */
    0u,                       /* no manufacturer string */
    1u,                       /* development product string */
    0u,                       /* no serial number */
    1u                        /* one configuration */
};

static const uint8_t usb_configuration_descriptor[] =
{
    /* Configuration descriptor. */
    9u, USB_DESC_CONFIGURATION,
    67u, 0u,                  /* wTotalLength */
    2u,                       /* CDC control + CDC data interfaces */
    USB_CONFIGURATION_ONE,
    0u,
    0x80u,                    /* bus powered */
    50u,                      /* 100 mA */

    /* CDC Communication Class Interface. */
    9u, 4u,
    USB_CDC_CONTROL_INTERFACE,
    0u,
    1u,
    0x02u,
    0x02u,
    0x00u,
    0u,

    /* CDC Header Functional Descriptor: CDC 1.10. */
    5u, 0x24u, 0x00u, 0x10u, 0x01u,

    /* CDC Call Management Functional Descriptor. */
    5u, 0x24u, 0x01u, 0x00u, USB_CDC_DATA_INTERFACE,

    /* CDC ACM Functional Descriptor: line coding/control line supported. */
    4u, 0x24u, 0x02u, 0x02u,

    /* CDC Union Functional Descriptor. */
    5u, 0x24u, 0x06u, USB_CDC_CONTROL_INTERFACE, USB_CDC_DATA_INTERFACE,

    /* EP1 IN: CDC notification endpoint. */
    7u, 5u, 0x81u, 0x03u,
    USB_CDC_NOTIFY_PACKET, 0u,
    16u,

    /* CDC Data Class Interface. */
    9u, 4u,
    USB_CDC_DATA_INTERFACE,
    0u,
    2u,
    0x0Au,
    0x00u,
    0x00u,
    0u,

    /* EP2 OUT: host -> device bulk data. */
    7u, 5u, 0x02u, 0x02u,
    USB_CDC_DATA_MAX_PACKET, 0u,
    0u,

    /* EP3 IN: device -> host bulk data. */
    7u, 5u, 0x83u, 0x02u,
    USB_CDC_DATA_MAX_PACKET, 0u,
    0u
};

static const uint8_t usb_string_language[] =
{
    4u, USB_DESC_STRING, 0x09u, 0x04u /* en-US */
};

static const uint8_t usb_string_product[] =
{
    40u, USB_DESC_STRING,
    'D', 0u, 'e', 0u, 'u', 0u, 's', 0u,
    ' ', 0u, 'O', 0u, 'S', 0u, ' ', 0u,
    'C', 0u, 'D', 0u, 'C', 0u, ' ', 0u,
    'C', 0u, 'o', 0u, 'n', 0u, 's', 0u,
    'o', 0u, 'l', 0u, 'e', 0u
};

_Static_assert(sizeof(usb_device_descriptor) == 18u,
    "USB device descriptor must be 18 bytes");
_Static_assert(sizeof(usb_configuration_descriptor) == 67u,
    "USB CDC configuration tree must be 67 bytes");
_Static_assert(sizeof(usb_string_language) == 4u,
    "USB language string descriptor must be 4 bytes");
_Static_assert(sizeof(usb_string_product) == 40u,
    "USB product string descriptor must be 40 bytes");
_Static_assert((USB_BTABLE_LOCAL & 0x7u) == 0u,
    "USB BTABLE must be 8-byte aligned");
_Static_assert((USB_EP0_RX_LOCAL + USB_EP0_MAX_PACKET) <= USB_EP1_TX_LOCAL,
    "USB EP0 RX overlaps EP1 TX PMA");
_Static_assert((USB_EP1_TX_LOCAL + USB_CDC_NOTIFY_PACKET) <= USB_EP2_RX_LOCAL,
    "USB EP1 TX overlaps EP2 RX PMA");
_Static_assert((USB_EP2_RX_LOCAL + USB_CDC_DATA_MAX_PACKET) <= USB_EP3_TX_LOCAL,
    "USB EP2 RX overlaps EP3 TX PMA");
_Static_assert((USB_EP3_TX_LOCAL + USB_CDC_DATA_MAX_PACKET) <= USB_PMA_LOGICAL_BYTES,
    "USB CDC PMA allocation exceeds STM32F103 packet memory");
_Static_assert((USB_CDC_RX_RING_CAPACITY & (USB_CDC_RX_RING_CAPACITY - 1u)) == 0u,
    "USB CDC RX ring capacity must be power-of-two");
_Static_assert((USB_CDC_TX_RING_CAPACITY & (USB_CDC_TX_RING_CAPACITY - 1u)) == 0u,
    "USB CDC TX ring capacity must be power-of-two");

volatile usb_device_diagnostics_t usb_device_diagnostics;

static usb_ep0_state_t usb_ep0_state;
static const uint8_t *usb_ep0_in_data;
static uint16_t usb_ep0_in_remaining;
static uint8_t usb_ep0_need_zlp;
static uint8_t usb_ep0_out_request;
static uint16_t usb_ep0_out_expected;
static uint8_t usb_pending_address;
static uint8_t usb_pending_address_valid;
static uint8_t usb_pending_configuration;
static uint8_t usb_pending_configuration_valid;
static uint16_t usb_pending_control_line_state;
static uint8_t usb_pending_control_line_state_valid;
static uint8_t usb_pending_line_coding[7];
static uint8_t usb_pending_line_coding_valid;
static uint8_t usb_line_coding[7] =
{
    0x00u, 0xC2u, 0x01u, 0x00u, /* 115200 */
    0u,                         /* 1 stop bit */
    0u,                         /* no parity */
    8u                          /* 8 data bits */
};

static volatile uint8_t usb_cdc_rx_ring[USB_CDC_RX_RING_CAPACITY];
static volatile uint32_t usb_cdc_rx_head;
static volatile uint32_t usb_cdc_rx_tail;
static volatile uint8_t usb_cdc_tx_ring[USB_CDC_TX_RING_CAPACITY];
static volatile uint32_t usb_cdc_tx_head;
static volatile uint32_t usb_cdc_tx_tail;
static volatile uint16_t usb_cdc_tx_inflight;
static volatile uint8_t usb_cdc_tx_active;
static volatile uint8_t usb_cdc_notify_active;
static volatile uint8_t usb_cdc_notify_pending;
static usb_cdc_rx_notify_t usb_cdc_rx_notify;

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

static void usb_ep_clear_dtog_rx(uint8_t endpoint)
{
    const uint16_t current = USB_EP_REG(endpoint);

    if ((current & USB_EP_DTOG_RX) != 0u)
    {
        USB_EP_REG(endpoint) =
            (uint16_t)(usb_ep_invariant(current) | USB_EP_DTOG_RX);
    }
}

static void usb_ep_clear_dtog_tx(uint8_t endpoint)
{
    const uint16_t current = USB_EP_REG(endpoint);

    if ((current & USB_EP_DTOG_TX) != 0u)
    {
        USB_EP_REG(endpoint) =
            (uint16_t)(usb_ep_invariant(current) | USB_EP_DTOG_TX);
    }
}

static void usb_ep_runtime_reset(uint8_t endpoint, uint16_t type)
{
    usb_ep_set_tx_status(endpoint, USB_EP_STAT_TX_DISABLED);
    usb_ep_set_rx_status(endpoint, USB_EP_STAT_RX_DISABLED);
    usb_ep_clear_ctr_rx(endpoint);
    usb_ep_clear_ctr_tx(endpoint);
    usb_ep_clear_dtog_rx(endpoint);
    usb_ep_clear_dtog_tx(endpoint);
    USB_EP_REG(endpoint) =
        (uint16_t)(type | (uint16_t)endpoint);
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
        USB_BTABLE_EP_TX_COUNT(USB_EP0),
        (uint16_t)(count & USB_COUNT_MASK));
}

static uint16_t usb_ep0_get_rx_count(void)
{
    return (uint16_t)(
        usb_pma_read16(USB_BTABLE_EP_RX_COUNT(USB_EP0)) &
        USB_COUNT_MASK);
}

static void usb_ep0_reset_transfer_state(void)
{
    usb_ep0_state = USB_EP0_IDLE;
    usb_ep0_in_data = (const uint8_t *)0;
    usb_ep0_in_remaining = 0u;
    usb_ep0_need_zlp = 0u;
    usb_ep0_out_request = USB_EP0_OUT_NONE;
    usb_ep0_out_expected = 0u;
    usb_pending_address = 0u;
    usb_pending_address_valid = 0u;
    usb_pending_configuration = 0u;
    usb_pending_configuration_valid = 0u;
    usb_pending_control_line_state = 0u;
    usb_pending_control_line_state_valid = 0u;
    usb_pending_line_coding_valid = 0u;
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

static void usb_ep0_start_out(uint8_t request, uint16_t expected_length)
{
    usb_ep0_out_request = request;
    usb_ep0_out_expected = expected_length;
    usb_ep0_state = USB_EP0_DATA_OUT;
    usb_ep_set_tx_status(USB_EP0, USB_EP_STAT_TX_NAK);
    usb_ep0_arm_rx();
}

static void usb_cdc_reset_rings(void)
{
    usb_cdc_rx_head = 0u;
    usb_cdc_rx_tail = 0u;
    usb_cdc_tx_head = 0u;
    usb_cdc_tx_tail = 0u;
    usb_cdc_tx_inflight = 0u;
    usb_cdc_tx_active = 0u;
    usb_cdc_notify_active = 0u;
    usb_cdc_notify_pending = 0u;
}

static int usb_cdc_line_coding_valid(const uint8_t *coding)
{
    uint32_t baud;

    if (coding == (const uint8_t *)0)
    {
        return 0;
    }

    baud =
        (uint32_t)coding[0] |
        ((uint32_t)coding[1] << 8) |
        ((uint32_t)coding[2] << 16) |
        ((uint32_t)coding[3] << 24);

    if ((baud == 0u) || (coding[4] > 2u) || (coding[5] > 4u))
    {
        return 0;
    }

    if (
        (coding[6] != 5u) &&
        (coding[6] != 6u) &&
        (coding[6] != 7u) &&
        (coding[6] != 8u) &&
        (coding[6] != 16u)
    ) {
        return 0;
    }

    return 1;
}

static void usb_cdc_notification_kick(void)
{
    static const uint8_t serial_state_zero[10] =
    {
        0xA1u, USB_CDC_NOTIFICATION_SERIAL_STATE,
        0u, 0u,
        USB_CDC_CONTROL_INTERFACE, 0u,
        2u, 0u,
        0u, 0u
    };

    if (
        (usb_device_diagnostics.cdc_configured == 0u) ||
        (usb_cdc_notify_active != 0u) ||
        (usb_cdc_notify_pending == 0u)
    ) {
        return;
    }

    usb_pma_write(
        USB_EP1_TX_LOCAL,
        serial_state_zero,
        (uint16_t)sizeof(serial_state_zero));
    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_CDC_NOTIFY_EP),
        (uint16_t)sizeof(serial_state_zero));

    usb_cdc_notify_pending = 0u;
    usb_cdc_notify_active = 1u;
    usb_ep_set_tx_status(USB_CDC_NOTIFY_EP, USB_EP_STAT_TX_VALID);
}

static void usb_cdc_endpoints_disable(void)
{
    usb_ep_runtime_reset(USB_CDC_NOTIFY_EP, USB_EP_TYPE_INTERRUPT);
    usb_ep_runtime_reset(USB_CDC_OUT_EP, USB_EP_TYPE_BULK);
    usb_ep_runtime_reset(USB_CDC_IN_EP, USB_EP_TYPE_BULK);

    USB_EP_REG(USB_CDC_NOTIFY_EP) = 0u;
    USB_EP_REG(USB_CDC_OUT_EP) = 0u;
    USB_EP_REG(USB_CDC_IN_EP) = 0u;

    usb_cdc_reset_rings();
    usb_device_diagnostics.cdc_configured = 0u;
}

static void usb_cdc_endpoints_enable(void)
{
    usb_pma_write16(
        USB_BTABLE_EP_TX_ADDR(USB_CDC_NOTIFY_EP),
        USB_EP1_TX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_CDC_NOTIFY_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_ADDR(USB_CDC_NOTIFY_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_COUNT(USB_CDC_NOTIFY_EP),
        0u);

    usb_pma_write16(
        USB_BTABLE_EP_TX_ADDR(USB_CDC_OUT_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_CDC_OUT_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_ADDR(USB_CDC_OUT_EP),
        USB_EP2_RX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP_RX_COUNT(USB_CDC_OUT_EP),
        USB_RX_COUNT_64);

    usb_pma_write16(
        USB_BTABLE_EP_TX_ADDR(USB_CDC_IN_EP),
        USB_EP3_TX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_CDC_IN_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_ADDR(USB_CDC_IN_EP),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_COUNT(USB_CDC_IN_EP),
        0u);

    usb_ep_runtime_reset(
        USB_CDC_NOTIFY_EP,
        USB_EP_TYPE_INTERRUPT);
    usb_ep_runtime_reset(
        USB_CDC_OUT_EP,
        USB_EP_TYPE_BULK);
    usb_ep_runtime_reset(
        USB_CDC_IN_EP,
        USB_EP_TYPE_BULK);

    usb_ep_set_rx_status(USB_CDC_NOTIFY_EP, USB_EP_STAT_RX_DISABLED);
    usb_ep_set_tx_status(USB_CDC_NOTIFY_EP, USB_EP_STAT_TX_NAK);

    usb_ep_set_tx_status(USB_CDC_OUT_EP, USB_EP_STAT_TX_DISABLED);
    usb_ep_set_rx_status(USB_CDC_OUT_EP, USB_EP_STAT_RX_VALID);

    usb_ep_set_rx_status(USB_CDC_IN_EP, USB_EP_STAT_RX_DISABLED);
    usb_ep_set_tx_status(USB_CDC_IN_EP, USB_EP_STAT_TX_NAK);

    usb_cdc_reset_rings();
    usb_device_diagnostics.cdc_configured = 1u;
}

static void usb_cdc_tx_kick(void)
{
    uint32_t available;
    uint16_t length;
    uint16_t index;

    if (
        (usb_device_diagnostics.configuration != USB_CONFIGURATION_ONE) ||
        (usb_device_diagnostics.cdc_configured == 0u) ||
        (usb_cdc_tx_active != 0u)
    ) {
        return;
    }

    available = usb_cdc_tx_head - usb_cdc_tx_tail;
    if (available == 0u)
    {
        return;
    }

    length = (available > USB_CDC_DATA_MAX_PACKET) ?
        USB_CDC_DATA_MAX_PACKET :
        (uint16_t)available;

    for (index = 0u; index < length; index = (uint16_t)(index + 2u))
    {
        uint16_t word =
            usb_cdc_tx_ring[(usb_cdc_tx_tail + index) & USB_CDC_TX_RING_MASK];

        if ((uint16_t)(index + 1u) < length)
        {
            word |=
                (uint16_t)(
                    (uint16_t)usb_cdc_tx_ring[
                        (usb_cdc_tx_tail + index + 1u) & USB_CDC_TX_RING_MASK]
                    << 8);
        }

        usb_pma_write16(
            (uint16_t)(USB_EP3_TX_LOCAL + index),
            word);
    }

    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_CDC_IN_EP),
        length);
    usb_cdc_tx_inflight = length;
    usb_cdc_tx_active = 1u;
    usb_ep_set_tx_status(USB_CDC_IN_EP, USB_EP_STAT_TX_VALID);
}

static void usb_cdc_handle_out(void)
{
    const uint16_t count =
        (uint16_t)(
            usb_pma_read16(USB_BTABLE_EP_RX_COUNT(USB_CDC_OUT_EP)) &
            USB_COUNT_MASK);
    uint16_t index;
    uint16_t word = 0u;
    uint32_t accepted = 0u;

    usb_ep_clear_ctr_rx(USB_CDC_OUT_EP);
    ++usb_device_diagnostics.cdc_rx_packet_count;
    usb_device_diagnostics.cdc_rx_byte_count += count;

    for (index = 0u; index < count; ++index)
    {
        const uint32_t depth = usb_cdc_rx_head - usb_cdc_rx_tail;
        uint8_t byte;

        if ((index & 1u) == 0u)
        {
            word = usb_pma_read16((uint16_t)(USB_EP2_RX_LOCAL + index));
            byte = (uint8_t)(word & 0xFFu);
        }
        else
        {
            byte = (uint8_t)(word >> 8);
        }

        if (depth < USB_CDC_RX_RING_CAPACITY)
        {
            const uint32_t next_depth = depth + 1u;

            usb_cdc_rx_ring[usb_cdc_rx_head & USB_CDC_RX_RING_MASK] = byte;
            ++usb_cdc_rx_head;
            ++accepted;

            if (next_depth > usb_device_diagnostics.cdc_rx_high_water)
            {
                usb_device_diagnostics.cdc_rx_high_water = next_depth;
            }
        }
        else
        {
            ++usb_device_diagnostics.cdc_rx_drop_count;
        }
    }

    usb_pma_write16(
        USB_BTABLE_EP_RX_COUNT(USB_CDC_OUT_EP),
        USB_RX_COUNT_64);
    usb_ep_set_rx_status(USB_CDC_OUT_EP, USB_EP_STAT_RX_VALID);

    if ((accepted != 0u) && (usb_cdc_rx_notify != (usb_cdc_rx_notify_t)0))
    {
        usb_cdc_rx_notify();
    }
}

static void usb_cdc_handle_in(void)
{
    usb_ep_clear_ctr_tx(USB_CDC_IN_EP);

    if (usb_cdc_tx_active != 0u)
    {
        usb_cdc_tx_tail += usb_cdc_tx_inflight;
        ++usb_device_diagnostics.cdc_tx_packet_count;
        usb_device_diagnostics.cdc_tx_byte_count += usb_cdc_tx_inflight;
        usb_cdc_tx_inflight = 0u;
        usb_cdc_tx_active = 0u;
    }

    usb_ep_set_tx_status(USB_CDC_IN_EP, USB_EP_STAT_TX_NAK);
    usb_cdc_tx_kick();
}

static void usb_cdc_handle_ctr(uint8_t endpoint)
{
    uint16_t endpoint_value = USB_EP_REG(endpoint);

    if (endpoint == USB_CDC_NOTIFY_EP)
    {
        if ((endpoint_value & USB_EP_CTR_TX) != 0u)
        {
            usb_ep_clear_ctr_tx(endpoint);
            usb_cdc_notify_active = 0u;
            usb_ep_set_tx_status(endpoint, USB_EP_STAT_TX_NAK);
            usb_cdc_notification_kick();
        }

        endpoint_value = USB_EP_REG(endpoint);
        if ((endpoint_value & USB_EP_CTR_RX) != 0u)
        {
            usb_ep_clear_ctr_rx(endpoint);
            ++usb_device_diagnostics.error_count;
        }
        return;
    }

    if (endpoint == USB_CDC_OUT_EP)
    {
        if ((endpoint_value & USB_EP_CTR_RX) != 0u)
        {
            usb_cdc_handle_out();
        }

        endpoint_value = USB_EP_REG(endpoint);
        if ((endpoint_value & USB_EP_CTR_TX) != 0u)
        {
            usb_ep_clear_ctr_tx(endpoint);
            ++usb_device_diagnostics.error_count;
        }
        return;
    }

    if (endpoint == USB_CDC_IN_EP)
    {
        if ((endpoint_value & USB_EP_CTR_TX) != 0u)
        {
            usb_cdc_handle_in();
        }

        endpoint_value = USB_EP_REG(endpoint);
        if ((endpoint_value & USB_EP_CTR_RX) != 0u)
        {
            usb_ep_clear_ctr_rx(endpoint);
            ++usb_device_diagnostics.error_count;
        }
    }
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
            ((setup->w_index == USB_CDC_CONTROL_INTERFACE) ||
             (setup->w_index == USB_CDC_DATA_INTERFACE))
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
             (setup->w_index == 0x0080u) ||
             ((usb_device_diagnostics.configuration == USB_CONFIGURATION_ONE) &&
              ((setup->w_index == 0x0081u) ||
               (setup->w_index == 0x0002u) ||
               (setup->w_index == 0x0083u))))
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
        ((setup->w_index == USB_CDC_CONTROL_INTERFACE) ||
         (setup->w_index == USB_CDC_DATA_INTERFACE)) &&
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
        ((setup->w_index == USB_CDC_CONTROL_INTERFACE) ||
         (setup->w_index == USB_CDC_DATA_INTERFACE)) &&
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

static int usb_ep0_class_in(const usb_setup_packet_t *setup)
{
    if (
        (usb_device_diagnostics.configuration != USB_CONFIGURATION_ONE) ||
        (setup->bm_request_type != 0xA1u) ||
        (setup->w_index != USB_CDC_CONTROL_INTERFACE)
    ) {
        return 0;
    }

    if (
        (setup->b_request == USB_CDC_REQ_GET_LINE_CODING) &&
        (setup->w_value == 0u) &&
        (setup->w_length == 7u)
    ) {
        usb_ep0_start_in(
            usb_line_coding,
            (uint16_t)sizeof(usb_line_coding),
            setup->w_length);
        return 1;
    }

    return 0;
}

static int usb_ep0_class_out(const usb_setup_packet_t *setup)
{
    if (
        (usb_device_diagnostics.configuration != USB_CONFIGURATION_ONE) ||
        (setup->bm_request_type != 0x21u) ||
        (setup->w_index != USB_CDC_CONTROL_INTERFACE)
    ) {
        return 0;
    }

    if (
        (setup->b_request == USB_CDC_REQ_SET_LINE_CODING) &&
        (setup->w_value == 0u) &&
        (setup->w_length == 7u)
    ) {
        usb_ep0_start_out(
            USB_EP0_OUT_SET_LINE_CODING,
            7u);
        return 1;
    }

    if (
        (setup->b_request == USB_CDC_REQ_SET_CONTROL_LINE_STATE) &&
        ((setup->w_value & 0xFFFCu) == 0u) &&
        (setup->w_length == 0u)
    ) {
        usb_pending_control_line_state = setup->w_value;
        usb_pending_control_line_state_valid = 1u;
        usb_ep0_start_status_in();
        return 1;
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
        int handled = 0;

        if ((setup.bm_request_type & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_STANDARD)
        {
            handled = usb_ep0_standard_in(&setup);
        }
        else if ((setup.bm_request_type & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS)
        {
            handled = usb_ep0_class_in(&setup);
        }

        if (handled == 0)
        {
            usb_ep0_stall();
        }
    }
    else
    {
        int handled = 0;

        if ((setup.bm_request_type & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_STANDARD)
        {
            handled = usb_ep0_standard_out(&setup);
        }
        else if ((setup.bm_request_type & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS)
        {
            handled = usb_ep0_class_out(&setup);
        }

        if (handled == 0)
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

    if (usb_ep0_state == USB_EP0_DATA_OUT)
    {
        if (
            (usb_ep0_out_request == USB_EP0_OUT_SET_LINE_CODING) &&
            (usb_ep0_out_expected == 7u) &&
            (count == 7u)
        ) {
            usb_pma_read(
                USB_EP0_RX_LOCAL,
                usb_pending_line_coding,
                7u);

            if (usb_cdc_line_coding_valid(usb_pending_line_coding) == 0)
            {
                usb_ep0_stall();
                return;
            }

            usb_pending_line_coding_valid = 1u;
            usb_ep0_out_request = USB_EP0_OUT_NONE;
            usb_ep0_out_expected = 0u;
            usb_ep0_start_status_in();
            return;
        }

        usb_ep0_stall();
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
            usb_cdc_endpoints_disable();
            usb_device_diagnostics.configuration =
                USB_CONFIGURATION_NONE;
        }
    }

    if (usb_pending_configuration_valid != 0u)
    {
        if (usb_pending_configuration == USB_CONFIGURATION_ONE)
        {
            usb_device_diagnostics.configuration = USB_CONFIGURATION_ONE;
            usb_cdc_endpoints_enable();
        }
        else
        {
            usb_cdc_endpoints_disable();
            usb_device_diagnostics.configuration = USB_CONFIGURATION_NONE;
        }

        usb_pending_configuration_valid = 0u;
    }

    if (usb_pending_control_line_state_valid != 0u)
    {
        usb_device_diagnostics.cdc_control_line_state =
            usb_pending_control_line_state;
        usb_pending_control_line_state_valid = 0u;
        usb_cdc_notify_pending = 1u;
        usb_cdc_notification_kick();
    }

    if (usb_pending_line_coding_valid != 0u)
    {
        uint32_t baud;
        uint32_t index;

        for (index = 0u; index < 7u; ++index)
        {
            usb_line_coding[index] = usb_pending_line_coding[index];
        }

        baud =
            (uint32_t)usb_line_coding[0] |
            ((uint32_t)usb_line_coding[1] << 8) |
            ((uint32_t)usb_line_coding[2] << 16) |
            ((uint32_t)usb_line_coding[3] << 24);

        usb_device_diagnostics.cdc_line_coding_baud = baud;
        usb_device_diagnostics.cdc_line_coding_stop_bits = usb_line_coding[4];
        usb_device_diagnostics.cdc_line_coding_parity = usb_line_coding[5];
        usb_device_diagnostics.cdc_line_coding_data_bits = usb_line_coding[6];
        usb_pending_line_coding_valid = 0u;
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
        USB_BTABLE_EP_TX_ADDR(USB_EP0),
        USB_EP0_TX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP_TX_COUNT(USB_EP0),
        0u);
    usb_pma_write16(
        USB_BTABLE_EP_RX_ADDR(USB_EP0),
        USB_EP0_RX_LOCAL);
    usb_pma_write16(
        USB_BTABLE_EP_RX_COUNT(USB_EP0),
        USB_RX_COUNT_64);

    USB_EP_REG(USB_EP0) =
        (uint16_t)(USB_EP_TYPE_CONTROL | USB_EP0);

    usb_ep0_reset_transfer_state();
    usb_cdc_endpoints_disable();

    usb_line_coding[0] = 0x00u;
    usb_line_coding[1] = 0xC2u;
    usb_line_coding[2] = 0x01u;
    usb_line_coding[3] = 0x00u;
    usb_line_coding[4] = 0u;
    usb_line_coding[5] = 0u;
    usb_line_coding[6] = 8u;

    usb_device_diagnostics.address = 0u;
    usb_device_diagnostics.configuration = USB_CONFIGURATION_NONE;
    usb_device_diagnostics.cdc_control_line_state = 0u;
    usb_device_diagnostics.cdc_line_coding_baud = 115200u;
    usb_device_diagnostics.cdc_line_coding_stop_bits = 0u;
    usb_device_diagnostics.cdc_line_coding_parity = 0u;
    usb_device_diagnostics.cdc_line_coding_data_bits = 8u;

    USB_DADDR = USB_DADDR_EF;

    usb_ep_set_tx_status(
        USB_EP0,
        USB_EP_STAT_TX_NAK);
    usb_ep0_arm_rx();

    ++usb_device_diagnostics.bus_reset_count;
}

static void usb_force_host_disconnect(uint32_t core_clock_hz)
{
    volatile uint32_t spin;
    uint32_t saved_pa12_config;
    const uint32_t iterations = core_clock_hz / 50u;

    /*
     * Blue Pill-class STM32F103 boards use a fixed external D+ pull-up. A
     * system/IWDG reset therefore resets the USB macrocell without necessarily
     * creating a host-visible detach. usbser can keep the old pipe/session and
     * refuse to reopen it after the MCU has restarted.
     *
     * Before enabling the USB macrocell, temporarily own PA12 as a 2 MHz
     * open-drain GPIO and hold D+ low. The loop budget is >=20 ms even at the
     * theoretical one-cycle minimum per iteration, comfortably beyond USB
     * disconnect debounce. Restoring the prior PA12 GPIO configuration releases
     * D+; the normal USB initialization below then starts a fresh host session.
     */
    RCC_APB1ENR &= ~RCC_APB1_USBEN;
    RCC_APB2ENR |= RCC_APB2_IOPAEN;
    saved_pa12_config = GPIOA_CRH & GPIO_CRH_PA12_MASK;

    GPIOA_BRR = GPIO_PIN_12;
    GPIOA_CRH =
        (GPIOA_CRH & ~GPIO_CRH_PA12_MASK) |
        GPIO_CRH_PA12_OUTPUT_OD_2MHZ;

    for (spin = 0u; spin < iterations; ++spin)
    {
        __asm volatile ("nop");
    }

    GPIOA_CRH =
        (GPIOA_CRH & ~GPIO_CRH_PA12_MASK) |
        saved_pa12_config;
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
    usb_device_diagnostics.cdc_configured = 0u;
    usb_device_diagnostics.cdc_rx_packet_count = 0u;
    usb_device_diagnostics.cdc_rx_byte_count = 0u;
    usb_device_diagnostics.cdc_rx_drop_count = 0u;
    usb_device_diagnostics.cdc_rx_high_water = 0u;
    usb_device_diagnostics.cdc_tx_packet_count = 0u;
    usb_device_diagnostics.cdc_tx_byte_count = 0u;
    usb_device_diagnostics.cdc_tx_drop_count = 0u;
    usb_device_diagnostics.cdc_tx_high_water = 0u;
    usb_device_diagnostics.cdc_control_line_state = 0u;
    usb_device_diagnostics.cdc_line_coding_baud = 115200u;
    usb_device_diagnostics.cdc_line_coding_stop_bits = 0u;
    usb_device_diagnostics.cdc_line_coding_parity = 0u;
    usb_device_diagnostics.cdc_line_coding_data_bits = 8u;

    usb_cdc_rx_notify = (usb_cdc_rx_notify_t)0;
    usb_cdc_reset_rings();
    usb_ep0_reset_transfer_state();

    usb_force_host_disconnect(core_clock_hz);

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

void usb_cdc_set_rx_notify(usb_cdc_rx_notify_t notify)
{
    usb_cdc_rx_notify = notify;
}

int usb_cdc_is_configured(void)
{
    return
        (usb_device_diagnostics.configuration == USB_CONFIGURATION_ONE) &&
        (usb_device_diagnostics.cdc_configured != 0u);
}

int usb_cdc_try_getc(char *c)
{
    const uint32_t tail = usb_cdc_rx_tail;

    if ((c == (char *)0) || (tail == usb_cdc_rx_head))
    {
        return 0;
    }

    *c = (char)usb_cdc_rx_ring[tail & USB_CDC_RX_RING_MASK];
    usb_cdc_rx_tail = tail + 1u;
    return 1;
}

int usb_cdc_write_byte(uint8_t byte)
{
    const uint32_t head = usb_cdc_tx_head;
    const uint32_t depth = head - usb_cdc_tx_tail;

    if (usb_cdc_is_configured() == 0)
    {
        ++usb_device_diagnostics.cdc_tx_drop_count;
        return 0;
    }

    if (depth >= USB_CDC_TX_RING_CAPACITY)
    {
        ++usb_device_diagnostics.cdc_tx_drop_count;
        return 0;
    }

    usb_cdc_tx_ring[head & USB_CDC_TX_RING_MASK] = byte;
    usb_cdc_tx_head = head + 1u;

    if ((depth + 1u) > usb_device_diagnostics.cdc_tx_high_water)
    {
        usb_device_diagnostics.cdc_tx_high_water = depth + 1u;
    }

    usb_cdc_tx_kick();
    return 1;
}

int usb_cdc_write_span_atomic(const uint8_t *data, uint32_t length)
{
    const uint32_t head = usb_cdc_tx_head;
    const uint32_t depth = head - usb_cdc_tx_tail;
    uint32_t index;
    uint32_t next_depth;

    if (length == 0u)
    {
        return 1;
    }

    if ((data == (const uint8_t *)0) ||
        (usb_cdc_is_configured() == 0) ||
        (length > USB_CDC_TX_RING_CAPACITY) ||
        (depth > USB_CDC_TX_RING_CAPACITY) ||
        (length > (USB_CDC_TX_RING_CAPACITY - depth)))
    {
        usb_device_diagnostics.cdc_tx_drop_count += length;
        return 0;
    }

    /*
     * The ring has one Thread-mode producer. Copy the complete span while the
     * published head remains unchanged; USB IRQ may drain older bytes but
     * cannot observe any byte from this span until the single head update.
     */
    for (index = 0u; index < length; ++index)
    {
        usb_cdc_tx_ring[(head + index) & USB_CDC_TX_RING_MASK] = data[index];
    }

    usb_cdc_tx_head = head + length;
    next_depth = depth + length;

    if (next_depth > usb_device_diagnostics.cdc_tx_high_water)
    {
        usb_device_diagnostics.cdc_tx_high_water = next_depth;
    }

    usb_cdc_tx_kick();
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
        else if (
            (usb_device_diagnostics.cdc_configured != 0u) &&
            ((endpoint == USB_CDC_NOTIFY_EP) ||
             (endpoint == USB_CDC_OUT_EP) ||
             (endpoint == USB_CDC_IN_EP))
        ) {
            usb_cdc_handle_ctr(endpoint);
        }
        else
        {
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
