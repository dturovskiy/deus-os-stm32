#include <stdint.h>
#include "drivers/usart1.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

#define RCC_APB2ENR       REG32(0x40021018u)
#define RCC_IOPAEN        (1u << 2)
#define RCC_USART1EN      (1u << 14)
#define GPIOA_CRH         REG32(0x40010804u)
#define USART1_SR         REG32(0x40013800u)
#define USART1_DR         REG32(0x40013804u)
#define USART1_BRR        REG32(0x40013808u)
#define USART1_CR1        REG32(0x4001380Cu)
#define USART_SR_PE       (1u << 0)
#define USART_SR_FE       (1u << 1)
#define USART_SR_NE       (1u << 2)
#define USART_SR_ORE      (1u << 3)
#define USART_SR_RXNE     (1u << 5)
#define USART_SR_TXE      (1u << 7)
#define USART_CR1_RE      (1u << 2)
#define USART_CR1_TE      (1u << 3)
#define USART_CR1_RXNEIE  (1u << 5)
#define USART_CR1_UE      (1u << 13)
#define NVIC_ISER1        REG32(0xE000E104u)
#define NVIC_ICPR1        REG32(0xE000E284u)
#define NVIC_IPR_USART1   REG8(0xE000E425u)
#define NVIC_USART1_BIT   (1u << 5)
#define NVIC_USART1_PRIORITY 0x80u
#define USART1_BRR_115200 0x0271u
#define USART1_RX_RING_MASK (USART1_RX_RING_CAPACITY - 1u)
#define USART1_TX_SPIN_LIMIT 1000000u

static volatile uint8_t rx_ring[USART1_RX_RING_CAPACITY];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;
static volatile uint32_t rx_irq_count;
static volatile uint32_t rx_byte_count;
static volatile uint32_t rx_drop_count;
static volatile uint32_t rx_error_count;
static volatile uint32_t rx_high_water;

void usart1_init(void)
{
    RCC_APB2ENR |= RCC_IOPAEN | RCC_USART1EN;

    GPIOA_CRH &= ~((0xFu << 4) | (0xFu << 8));
    GPIOA_CRH |= ((0xBu << 4) | (0x4u << 8));

    rx_head = 0u;
    rx_tail = 0u;
    rx_irq_count = 0u;
    rx_byte_count = 0u;
    rx_drop_count = 0u;
    rx_error_count = 0u;
    rx_high_water = 0u;

    USART1_BRR = USART1_BRR_115200;
    NVIC_ICPR1 = NVIC_USART1_BIT;
    NVIC_IPR_USART1 = NVIC_USART1_PRIORITY;
    NVIC_ISER1 = NVIC_USART1_BIT;
    USART1_CR1 =
        USART_CR1_RE |
        USART_CR1_TE |
        USART_CR1_RXNEIE |
        USART_CR1_UE;
}

int usart1_write_byte(uint8_t byte)
{
    uint32_t remaining = USART1_TX_SPIN_LIMIT;

    while ((USART1_SR & USART_SR_TXE) == 0u)
    {
        if (remaining == 0u)
        {
            return 0;
        }
        --remaining;
    }

    USART1_DR = (uint32_t)byte;
    return 1;
}

int usart1_try_read(char *value)
{
    const uint32_t tail = rx_tail;

    if ((value == (char *)0) || (tail == rx_head))
    {
        return 0;
    }

    *value = (char)rx_ring[tail & USART1_RX_RING_MASK];
    rx_tail = tail + 1u;
    return 1;
}

int usart1_irq_service(void)
{
    const uint32_t status = USART1_SR;
    int published = 0;

    ++rx_irq_count;

    if ((status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) != 0u)
    {
        ++rx_error_count;
    }

    if ((status & USART_SR_RXNE) != 0u)
    {
        const uint8_t byte = (uint8_t)USART1_DR;
        const uint32_t head = rx_head;
        const uint32_t depth = head - rx_tail;

        ++rx_byte_count;

        if (depth < USART1_RX_RING_CAPACITY)
        {
            const uint32_t next_head = head + 1u;
            const uint32_t next_depth = depth + 1u;

            rx_ring[head & USART1_RX_RING_MASK] = byte;
            rx_head = next_head;
            published = 1;

            if (next_depth > rx_high_water)
            {
                rx_high_water = next_depth;
            }
        }
        else
        {
            ++rx_drop_count;
        }
    }
    else if ((status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) != 0u)
    {
        (void)USART1_DR;
    }

    return published;
}

void usart1_diagnostics_get(usart1_diagnostics_t *diagnostics)
{
    if (diagnostics == (usart1_diagnostics_t *)0)
    {
        return;
    }

    diagnostics->capacity = USART1_RX_RING_CAPACITY;
    diagnostics->depth = rx_head - rx_tail;
    diagnostics->irq_count = rx_irq_count;
    diagnostics->byte_count = rx_byte_count;
    diagnostics->drop_count = rx_drop_count;
    diagnostics->error_count = rx_error_count;
    diagnostics->high_water = rx_high_water;
}
