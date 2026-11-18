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
#define USART_SR_TC (1u << 6)
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
/* 36 MHz PCLK1 / 74880 baud rounded to 481. STM32 BRR mantissa/fraction
 * encoding yields 0x01E1; only used for early ESP8266 ROM boot log.
 * Restore normal NodeLink/ROM SYNC to 115200 BEFORE transmitting SYNC. */
#define USART2_BRR_36MHZ_74880 0x01E1u
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
static volatile uint32_t rx_bytes;
static volatile uint32_t tx_bytes;
static volatile uint32_t tx_frames;
/* The normal USART2 IRQ/ring owns early 74880-baud input as well.
 * Only the bounded task0 CLOSE/SYNC boundary parses the boot-mode token. */
static volatile uint32_t rom_boot_received, rom_boot_errors;
static volatile uint8_t rom_boot_mode, rom_boot_active;
static uint32_t rom_boot_rx_base, rom_boot_error_base;
/* Exact ROM-only SYNC diagnostics, latched after CLOSE; no new IRQ path for
 * successful RX, just existing cumulative counters captured at boundaries.
 * Preserve the individual USART error bits on the existing error branch. */
static volatile uint32_t rom_sync_tx_word, rom_sync_rx_word;
static uint32_t rom_sync_rx_base, rom_sync_err_base, rom_sync_drop_base;
static volatile uint8_t rom_sync_active, rom_sync_uart_errors;

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
    rx_bytes = 0u; tx_bytes = 0u; tx_frames = 0u;
    rom_boot_received = 0u; rom_boot_errors = 0u;
    rom_boot_mode = 0u; rom_boot_active = 0u;
    rom_boot_rx_base = 0u; rom_boot_error_base = 0u;
    rom_sync_tx_word = 0u; rom_sync_rx_word = 0u;
    rom_sync_rx_base = 0u; rom_sync_err_base = 0u;
    rom_sync_drop_base = 0u; rom_sync_uart_errors = 0u;
    rom_sync_active = 0u;
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
/* Called exclusively from task0 when the ROM bridge has taken maintenance
 * ownership (NodeLink RX is disarmed). Start BEFORE GPIO0/EN reset pulse.
 * No GPIO writes here; no time delay, busy loop or secret data. */
void usart2_node_rom_boot_capture_begin(void)
{
    uint32_t s = irq_save();
    rom_boot_received = 0u; rom_boot_errors = 0u;
    rom_boot_mode = 0u;
    USART2_BRR = USART2_BRR_36MHZ_74880;
    irq_restore(s);
    /* Existing IRQ38 handler records raw bytes in its 256-slot ring. */
    usart2_node_rx_arm();
    rom_boot_rx_base = rx_bytes;
    rom_boot_error_base = rx_errors;
    rom_boot_active = 1u;
}
/* Only task0, after ROM settle and before the exact SYNC TX.
 * Freeze IRQ38, parse the bounded existing RX ring, return to 115200.
 * Never interpret silence as proof of application or ROM mode. */
void usart2_node_rom_boot_capture_end(void)
{
    usart2_node_sample_t sample;
    uint32_t s, i, tail = 0u;
    int item;
    if (rom_boot_active == 0u) return;
    s = irq_save();
    USART2_CR1 &= ~USART_CR1_RXNEIE;
    NVIC_ICPR1 = NVIC_IRQ38_BIT;
    (void)USART2_SR; (void)USART2_DR;
    USART2_BRR = USART2_BRR_36MHZ_115200;
    rom_boot_active = 0u;
    rom_boot_received = rx_bytes - rom_boot_rx_base;
    rom_boot_errors = rx_errors - rom_boot_error_base;
    irq_restore(s);
    if (rom_boot_errors != 0u) return;
    for (i = 0u; i < RX_CAP; ++i) {
        item = usart2_node_try_read(&sample);
        if (item <= 0) break;
        /* Compact 74880 boot-log signature suffix: "e:(1"/"e:(3".
         * This is a mode hint only, never a verified ROM handshake. */
        tail = (tail << 8) | sample.byte;
        if (tail == 0x653A2831u)
            rom_boot_mode = 1u;
        else if (tail == 0x653A2833u)
            rom_boot_mode = 3u;
    }
}
uint32_t usart2_node_rom_boot_status_bits(void)
{
    const uint32_t count = rom_boot_received > 255u ?
        255u : rom_boot_received;
    return (count << 24) | ((uint32_t)rom_boot_mode << 20) |
        (rom_boot_errors != 0u ? 0x00080000u : 0u);
}
void usart2_node_rx_arm(void)
{
    usart2_node_rx_gate_apply(1u);
}
/* Called only in a ROM-only USB maintenance session, after startup capture
 * has returned UART BRR to 115200 and RXNEIE was armed. No GPIO or TX. */
void usart2_node_rom_sync_trace_begin(void)
{
    uint32_t s = irq_save();
    rom_sync_tx_word = 0u;
    rom_sync_rx_word = (1u << 28);
    rom_sync_rx_base = rx_bytes;
    rom_sync_err_base = rx_errors;
    rom_sync_drop_base = rx_dropped;
    rom_sync_uart_errors = 0u;
    rom_sync_active = 1u;
    irq_restore(s);
}
/* Freeze the exact ROM-session RX delta before normal bridge disarms RX,
 * so subsequent NodeLink HELLO cannot change this forensic snapshot. */
void usart2_node_rom_sync_trace_end(void)
{
    uint32_t n, drops, errs, s = irq_save();
    if (rom_sync_active != 0u) {
        n = rx_bytes - rom_sync_rx_base;
        drops = rx_dropped - rom_sync_drop_base;
        errs = rx_errors - rom_sync_err_base;
        rom_sync_rx_word = (1u << 29) | (1u << 28) |
            (n > 65535u ? 0x80000000u : 0u) |
            (errs != 0u ? 0x40000000u : 0u) |
            ((drops > 255u ? 255u : drops) << 20) |
            (((uint32_t)rom_sync_uart_errors & 15u) << 16) |
            (n > 65535u ? 65535u : n);
        rom_sync_active = 0u;
    }
    irq_restore(s);
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
    /* During boot capture the existing nonblocking RX ring is reused. */
    if ((sr & USART_SR_ERR) != 0u) {
        if (rom_sync_active != 0u)
            rom_sync_uart_errors |= (uint8_t)(sr & USART_SR_ERR);
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
    ++rx_bytes;
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
static int usart2_node_write_raw(const uint8_t *data,uint32_t length)
{
    uint32_t i;
    for (i = 0u; i < length; ++i) {
        uint32_t remaining = TXE_SPIN_LIMIT;
        while ((USART2_SR & USART_SR_TXE) == 0u) {
            if (remaining == 0u) return 0;
            --remaining;
        }
        USART2_DR = data[i];
        ++tx_bytes;
    }
    ++tx_frames;
    return 1;
}
int usart2_node_write_frame(const uint8_t *data,uint32_t length)
{
    if (data == (const uint8_t *)0 || length == 0u ||
        length > USART2_NODE_MAX_TX_BYTES) return 0;
    return usart2_node_write_raw(data,length);
}
/* Dedicated ROM-only exception to the normal 40B NodeLink limit.
 * Validate the ENTIRE 46B immutable ESP8266 SYNC before the first TX
 * byte; emit one uninterrupted USART2 stream (no USB ACK between 0x55s).
 * This does not authorize generic 46B UART or Flash commands. */
int usart2_node_write_rom_sync46(const uint8_t *data,uint32_t length)
{
    uint32_t i;
    if (data == (const uint8_t *)0 || length != 46u) return 0;
    for (i = 0u; i < 46u; ++i) {
        uint8_t expected = 0u;
        if (i == 0u || i == 45u) expected = 0xC0u;
        else if (i == 2u) expected = 0x08u;
        else if (i == 3u) expected = 0x24u;
        else if (i == 9u || i == 10u) expected = 0x07u;
        else if (i == 11u) expected = 0x12u;
        else if (i == 12u) expected = 0x20u;
        else if (i >= 13u && i <= 44u) expected = 0x55u;
        if (data[i] != expected) return 0;
    }
    /* End-to-end output proof at the STM32 USART register boundary.
     * The existing non-ROM 40-byte NodeLink path remains unchanged.
     * Never infer ESP's electrical RX level or ROM reply from TC alone. */
    if (rom_sync_active != 0u) {
        uint32_t s = irq_save();
        uint32_t cr1 = USART2_CR1;
        uint32_t brr = USART2_BRR;
        rom_sync_tx_word = ((brr & 0xFFFFu) << 16) | (1u << 15) |
            ((cr1 & USART_CR1_RXNEIE) != 0u ? 1u << 14 : 0u) |
            ((cr1 & (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE)) ==
                (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE) ?
                    1u << 13 : 0u);
        irq_restore(s);
        /* Do not transmit a ROM autobaud train at the wrong USART rate,
         * or while RX/USART output is unarmed. Report a register fault. */
        if ((rom_sync_tx_word & ((1u << 14) | (1u << 13))) !=
                ((1u << 14) | (1u << 13)) ||
            brr != USART2_BRR_36MHZ_115200) {
            rom_sync_tx_word |= (1u << 11);
            return 0;
        }
        uint32_t start = tx_bytes, spins = TXE_SPIN_LIMIT;
        int sent = usart2_node_write_raw(data, length);
        uint32_t writes = tx_bytes - start;
        rom_sync_tx_word |= (writes & 63u);
        if (sent == 0) {
            rom_sync_tx_word |= (1u << 11);
            return 0;
        }
        rom_sync_tx_word |= (1u << 10);
        /* The last byte may still be shifting after DR accepts it:
         * bounded TC observation, no interrupt disable, no retransmit. */
        while ((USART2_SR & USART_SR_TC) == 0u && spins != 0u)
            --spins;
        if ((USART2_SR & USART_SR_TC) == 0u) {
            rom_sync_tx_word |= (1u << 11);
            return 0;
        }
        rom_sync_tx_word |= (1u << 12);
        return 1;
    }
    return usart2_node_write_raw(data,length);
}
void usart2_node_diagnostics_get(usart2_node_diagnostics_t *out)
{
    uint32_t primask;
    if (out == (usart2_node_diagnostics_t *)0) return;
    primask = irq_save();
    out->depth = rx_head - rx_tail;
    out->dropped = rx_dropped;
    out->errors = rx_errors;
    out->rx_bytes = rx_bytes;
    out->tx_bytes = tx_bytes;
    out->tx_frames = tx_frames;
    out->rom_sync_tx = rom_sync_tx_word;
    out->rom_sync_rx = rom_sync_rx_word;
    irq_restore(primask);
}
