/* ESP01-010: PA3 weak pull-up MINIMAL SOFTWARE CANDIDATE — NOT INSTALLED.
 * Derived from live STM32 signed-normal-v7 driver, original SHA256
 * ED08B7FFF3E7A91D79589613730C4385D4F8076D13CB587BCB151C5F5E9B6D66.
 * Uses BSRR to select GPIOA PA3 input pull-up before CNF is switched.
 * RXNEIE/IRQ38 remains on at boot: this mitigates floating idle level only,
 * NOT malicious/external noise nor IRQ/WCET upper bound. ARM budget pending.
 */
#include <stdint.h>
#include "drivers/usart2_node.h"

#define REG32(a) (*(volatile uint32_t *)(a))
#define REG8(a)  (*(volatile uint8_t *)(a))
#define RCC_APB1ENR REG32(0x4002101Cu)
#define RCC_APB2ENR REG32(0x40021018u)
#define RCC_USART2EN (1u << 17)
#define RCC_IOPAEN (1u << 2)
#define GPIOA_CRL REG32(0x40010800u)
#define GPIOA_BSRR REG32(0x40010810u)
#define GPIO_PA2_PA3_NIBBLE_MASK (0xFFu << 8)
#define GPIO_PA2_AF_PP_50MHZ (0xBu << 8)
#define GPIO_PA3_PULLUP_IN (0x8u << 12)
#define GPIO_PA3_PULLUP_SELECT (1u << 3)
#define USART2_SR REG32(0x40004400u)
#define USART2_DR REG32(0x40004404u)
#define USART2_BRR REG32(0x40004408u)
#define USART2_CR1 REG32(0x4000440Cu)
#define USART_SR_PE (1u << 0)
#define USART_SR_FE (1u << 1)
#define USART_SR_NE (1u << 2)
#define USART_SR_ORE (1u << 3)
#define USART_SR_RXNE (1u << 5)
#define USART_SR_TXE (1u << 7)
#define USART_SR_ERR (USART_SR_PE|USART_SR_FE|USART_SR_NE|USART_SR_ORE)
#define USART_CR1_RE (1u << 2)
#define USART_CR1_TE (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE (1u << 13)
#define NVIC_ISER1 REG32(0xE000E104u)
#define NVIC_ICPR1 REG32(0xE000E284u)
#define NVIC_IPR_USART2 REG8(0xE000E426u)
#define NVIC_IRQ38_BIT (1u << 6)
#define USART2_BRR_36MHZ_115200 0x0139u
#define TXE_SPIN_LIMIT 30000u
#define RX_CAP USART2_NODE_RX_CAPACITY

/* One ISR writer: head/head_slot. One task writer: tail/tail_slot.
 * No modulo by 2^32 sequencing of a 48-element array: slots are separate.
 * Array writes precede head publication with DMB + compiler memory clobber.
 */
static volatile uint8_t received[RX_CAP];
static volatile uint16_t arrival_low[RX_CAP];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;
static volatile uint16_t rx_head_slot;
static volatile uint16_t rx_tail_slot;
static volatile uint8_t rx_loss;
static volatile uint32_t rx_dropped;
static volatile uint32_t rx_errors;

static void barrier(void)
{
    __asm volatile ("dmb" ::: "memory");
}
static uint32_t irq_save(void)
{
    uint32_t primask;
    __asm volatile ("mrs %0, primask\n"
                    "cpsid i\n" : "=r"(primask) :: "memory");
    return primask;
}
static void irq_restore(uint32_t primask)
{
    __asm volatile ("msr primask, %0\n" :: "r"(primask) : "memory");
}
static uint16_t slot_next(uint16_t slot)
{
    ++slot;
    if (slot == RX_CAP) slot = 0u;
    return slot;
}
void usart2_node_init(void)
{
    uint32_t state = irq_save();
    rx_head = 0u; rx_tail = 0u;
    rx_head_slot = 0u; rx_tail_slot = 0u;
    rx_loss = 0u; rx_dropped = 0u; rx_errors = 0u;
    barrier();

    RCC_APB2ENR |= RCC_IOPAEN;
    RCC_APB1ENR |= RCC_USART2EN;
    /* Select pull-up with PA3 ODR latch before changing PA3 to input PU/PD. */
    GPIOA_BSRR = GPIO_PA3_PULLUP_SELECT;
    GPIOA_CRL = (GPIOA_CRL & ~GPIO_PA2_PA3_NIBBLE_MASK) |
        GPIO_PA2_AF_PP_50MHZ | GPIO_PA3_PULLUP_IN;

    USART2_CR1 = 0u;
    (void)USART2_SR;
    (void)USART2_DR;
    USART2_BRR = USART2_BRR_36MHZ_115200;
    NVIC_ICPR1 = NVIC_IRQ38_BIT;
    NVIC_IPR_USART2 = 0x80u;
    NVIC_ISER1 = NVIC_IRQ38_BIT;
    /* USART2 stays able to TX, but cannot interrupt while Node is absent. */
    USART2_CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
    irq_restore(state);
}
/* Task0-only session transitions, no non-Node peripheral modification.
 * ARM PRIMASK protects CR1, stale status/DR, IRQ pending and ring indexes.
 * NVIC IRQ38 is registered at boot; RXNEIE alone controls dispatch.
 */
/* Single non-inlined transition: both paths must clear stale RX/IRQ state.
 * Clearing both ring indexes is safe: transitions are task0-only and
 * protected by PRIMASK, while IRQ38 is the sole ring producer.
 * Keep out-of-line so ARM -Os doesn't duplicate expensive register reset.
 */
static __attribute__((noinline)) void usart2_node_rx_gate_apply(uint32_t enable)
{
    uint32_t state = irq_save();
    USART2_CR1 &= ~USART_CR1_RXNEIE;
    NVIC_ICPR1 = NVIC_IRQ38_BIT;
    (void)USART2_SR;
    (void)USART2_DR;
    rx_head = 0u; rx_tail = 0u;
    rx_head_slot = 0u; rx_tail_slot = 0u;
    rx_loss = 0u;
    barrier();
    if (enable != 0u) {
        USART2_CR1 |= USART_CR1_RXNEIE;
    }
    irq_restore(state);
}
void usart2_node_rx_disarm(void)
{
    usart2_node_rx_gate_apply(0u);
}
void usart2_node_rx_arm(void)
{
    usart2_node_rx_gate_apply(1u);
}
int usart2_node_irq_service(uint32_t arrival_ms)
{
    /* Hardware RXNEIE is the sole IRQ38 enable; no shadow flag to race. */
    if ((USART2_CR1 & USART_CR1_RXNEIE) == 0u) return 0;
    uint32_t sr = USART2_SR;
    uint32_t value;
    uint32_t depth;

    if ((sr & (USART_SR_ERR | USART_SR_RXNE)) == 0u) return 0;
    value = USART2_DR; /* F1 flag clear sequence: status then data. */
    if ((sr & USART_SR_ERR) != 0u) {
        ++rx_errors;
        rx_loss = 1u;
        return 1; /* Signal task0 to discard partial frame. */
    }
    if ((sr & USART_SR_RXNE) == 0u) return 0;
    depth = rx_head - rx_tail;
    if (rx_loss != 0u || depth >= RX_CAP) {
        ++rx_dropped;
        rx_loss = 1u;
        return 1;
    }
    received[rx_head_slot] = (uint8_t)value;
    arrival_low[rx_head_slot] = (uint16_t)arrival_ms;
    barrier();
    rx_head_slot = slot_next(rx_head_slot);
    ++rx_head;
    return 1;
}
int usart2_node_try_read(usart2_node_sample_t *sample)
{
    uint32_t primask;
    if (sample == (usart2_node_sample_t *)0) return -2;
    primask = irq_save();
    if (rx_loss != 0u) {
        /* One bounded fail-closed reset, never performed in the IRQ. */
        rx_tail = rx_head;
        rx_tail_slot = rx_head_slot;
        rx_loss = 0u;
        barrier();
        irq_restore(primask);
        return -1;
    }
    if (rx_tail == rx_head) {
        irq_restore(primask);
        return 0;
    }
    barrier();
    sample->byte = received[rx_tail_slot];
    sample->arrival_ms_low = arrival_low[rx_tail_slot];
    rx_tail_slot = slot_next(rx_tail_slot);
    ++rx_tail;
    irq_restore(primask);
    return 1;
}
int usart2_node_write_frame(const uint8_t *data,uint32_t length)
{
    uint32_t i;
    if (data == (const uint8_t *)0 || length == 0u ||
        length > USART2_NODE_MAX_TX_BYTES) return 0;
    for (i = 0u; i < length; ++i) {
        uint32_t remaining = TXE_SPIN_LIMIT;
        while ((USART2_SR & USART_SR_TXE) == 0u) {
            if (remaining == 0u) return 0;
            --remaining;
        }
        USART2_DR = data[i];
    }
    return 1;
}
void usart2_node_diagnostics_get(usart2_node_diagnostics_t *out)
{
    uint32_t primask;
    if (out == (usart2_node_diagnostics_t *)0) return;
    primask = irq_save();
    out->depth = rx_head - rx_tail;
    out->dropped = rx_dropped;
    out->errors = rx_errors;
    irq_restore(primask);
}
