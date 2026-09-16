#include <stdint.h>
#include "kernel/time.h"
#include "drivers/ssd1306.h"
#include "drivers/iwdg.h"
#include "drivers/usb_device.h"
#include "gfx/mono_fb.h"
#include "gfx/font5x7.h"
#include "gfx/text_renderer.h"
#include "kernel/oled_console.h"
#include "kernel/oled_status_bar.h"
#include "kernel/oled_ui_layout.h"
#include "kernel/scheduler.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))
#define REG8(addr)  (*(volatile uint8_t *)(addr))

/* Flash interface */
#define FLASH_ACR       REG32(0x40022000u)
#define FLASH_LATENCY_2 0x2u
#define FLASH_PRFTBE     (1u << 4)

/* Reset and clock control */
#define RCC_CR          REG32(0x40021000u)
#define RCC_CFGR        REG32(0x40021004u)
#define RCC_APB2ENR     REG32(0x40021018u)
#define RCC_APB1RSTR     REG32(0x40021010u)
#define RCC_APB1ENR      REG32(0x4002101Cu)
#define RCC_CSR          REG32(0x40021024u)

#define RCC_HSEON       (1u << 16)
#define RCC_HSERDY      (1u << 17)
#define RCC_PLLON       (1u << 24)
#define RCC_PLLRDY      (1u << 25)

#define RCC_SW_MASK     (0x3u << 0)
#define RCC_SW_PLL      (0x2u << 0)
#define RCC_SWS_MASK    (0x3u << 2)
#define RCC_SWS_PLL     (0x2u << 2)
#define RCC_PPRE1_DIV2  (0x4u << 8)
#define RCC_ADCPRE_DIV6 (0x2u << 14)
#define RCC_PLLSRC_HSE  (1u << 16)
#define RCC_PLLMUL_X9   (0x7u << 18)
#define RCC_USBPRE      (1u << 22)

#define RCC_IOPAEN      (1u << 2)
#define RCC_IOPCEN      (1u << 4)
#define RCC_USART1EN    (1u << 14)
#define RCC_CSR_RMVF     (1u << 24)
#define RCC_CSR_IWDGRSTF (1u << 29)

/* GPIOA / GPIOC */
#define GPIOA_CRH       REG32(0x40010804u)
#define GPIOB_CRL       REG32(0x40010C00u)

#define GPIOC_CRH       REG32(0x40011004u)
#define GPIOC_ODR       REG32(0x4001100Cu)
#define GPIOC_BSRR      REG32(0x40011010u)

#define GPIO_PIN_13     (1u << 13)
#define GPIO_RESET_13   (1u << 29)

/* USART1 */
#define USART1_SR       REG32(0x40013800u)
#define USART1_DR       REG32(0x40013804u)
#define USART1_BRR      REG32(0x40013808u)
#define USART1_CR1      REG32(0x4001380Cu)

#define USART_SR_PE     (1u << 0)
#define USART_SR_FE     (1u << 1)
#define USART_SR_NE     (1u << 2)
#define USART_SR_ORE    (1u << 3)
#define USART_SR_RXNE   (1u << 5)
#define USART_SR_TXE    (1u << 7)
#define USART_CR1_RE    (1u << 2)
#define USART_CR1_TE    (1u << 3)
#define USART_CR1_RXNEIE (1u << 5)
#define USART_CR1_UE    (1u << 13)

/* Cortex-M3 NVIC */
#define NVIC_ISER1      REG32(0xE000E104u)
#define NVIC_ICPR1      REG32(0xE000E284u)
#define NVIC_IPR_USART1 REG8(0xE000E425u)
#define NVIC_USART1_BIT (1u << 5)
#define NVIC_USART1_PRIORITY 0x80u
#define I2C1_CR1         REG32(0x40005400u)
#define I2C1_CR2         REG32(0x40005404u)
#define I2C1_DR          REG32(0x40005410u)
#define I2C1_SR1         REG32(0x40005414u)
#define I2C1_SR2         REG32(0x40005418u)
#define I2C1_CCR         REG32(0x4000541Cu)
#define I2C1_TRISE       REG32(0x40005420u)

#define RCC_APB2ENR_IOPBEN   (1u << 3)
#define RCC_APB1ENR_I2C1EN   (1u << 21)
#define RCC_APB1RSTR_I2C1RST (1u << 21)

#define I2C_CR1_PE        (1u << 0)
#define I2C_CR1_START     (1u << 8)
#define I2C_CR1_STOP      (1u << 9)

#define I2C_SR1_SB        (1u << 0)
#define I2C_SR1_ADDR      (1u << 1)
#define I2C_SR1_BTF       (1u << 2)
#define I2C_SR1_TXE       (1u << 7)


#define OLED_GLYPH_ADVANCE         6u
#define I2C_SR1_BERR      (1u << 8)
#define I2C_SR1_ARLO      (1u << 9)
#define I2C_SR1_AF        (1u << 10)
#define I2C_SR2_BUSY      (1u << 1)

#define I2C1_PCLK_MHZ     36u
#define I2C1_CCR_100KHZ   180u
#define I2C1_TRISE_100KHZ 37u
#define I2C_SPIN_LIMIT    100000u
#define SCHED_WORKLOAD_PEER_SPINS 1000000u

#define PRODUCTION_CONSOLE_STACK_WORDS 256u
#define PRODUCTION_CONSOLE_STACK_BYTES \
    (PRODUCTION_CONSOLE_STACK_WORDS * 4u)
#define PRODUCTION_CONSOLE_MIN_MARGIN_BYTES 256u
#define PRODUCTION_HEARTBEAT_STACK_WORDS 128u
#define PRODUCTION_HEARTBEAT_STACK_BYTES \
    (PRODUCTION_HEARTBEAT_STACK_WORDS * 4u)
#define PRODUCTION_HEARTBEAT_MIN_MARGIN_BYTES 256u
#define PRODUCTION_HEARTBEAT_PERIOD_MS 500u
#define PRODUCTION_IWDG_PRESCALER IWDG_PRESCALER_DIV256
#define PRODUCTION_IWDG_RELOAD 1249u
#define PRODUCTION_IWDG_SPIN_LIMIT 1000000u
#define SCHED_CONSOLE_PROBE_COMMAND_COUNT 17u
#define SCHED_ISOLATION_DIAGNOSTIC_COUNT 7u
#define PRODUCTION_UART_RX_EVENT         (1u << 0)
#define PRODUCTION_USB_CDC_RX_EVENT      (1u << 1)
#define PRODUCTION_CONSOLE_RX_EVENTS     \
    (PRODUCTION_UART_RX_EVENT | PRODUCTION_USB_CDC_RX_EVENT)
#define SCHED_WAIT_WAKE_SENTINEL         0x57u
#define SCHED_TIMED_SLEEP_MS             50u
#define SCHED_TIMED_TIMEOUT_MS           50u
#define SCHED_TIMED_EVENT_TIMEOUT_MS     500u

/*
 * PCLK2 = 72 MHz.
 * USARTDIV = 72,000,000 / (16 * 115,200) = 39.0625
 * BRR = mantissa 39, fraction 1 = 0x0271.
 */
#define USART1_BRR_115200 0x0271u

/* Cortex-M3 SysTick */
#define SYST_CSR        REG32(0xE000E010u)
#define SYST_RVR        REG32(0xE000E014u)
#define SYST_CVR        REG32(0xE000E018u)

#define SYST_ENABLE     (1u << 0)
#define SYST_TICKINT    (1u << 1)
#define SYST_CLKSOURCE  (1u << 2)

/* Cortex-M3 System Control Block */
#define SCB_ICSR        REG32(0xE000ED04u)
#define SCB_VTOR        REG32(0xE000ED08u)
#define SCB_SHCSR       REG32(0xE000ED24u)
#define SCB_CFSR        REG32(0xE000ED28u)
#define SCB_HFSR        REG32(0xE000ED2Cu)
#define SCB_DFSR        REG32(0xE000ED30u)
#define SCB_MMFAR       REG32(0xE000ED34u)
#define SCB_BFAR        REG32(0xE000ED38u)
#define SCB_AFSR        REG32(0xE000ED3Cu)

#define SHCSR_MEMFAULTENA (1u << 16)
#define SHCSR_BUSFAULTENA (1u << 17)
#define SHCSR_USGFAULTENA (1u << 18)

#define SRAM_START      0x20000000u
#define SRAM_END        0x20005000u
#define FAULT_MAGIC     0xFA17FA17u

extern uint32_t _smsp_stack;
extern uint32_t _emsp_guard;
extern uint32_t _estack;

#define MSP_STACK_RESERVED_BYTES 2048u
#define MSP_STACK_GUARD_BYTES      64u
#define MSP_STACK_CAPACITY_BYTES (MSP_STACK_RESERVED_BYTES - MSP_STACK_GUARD_BYTES)
#define MSP_STACK_FILL_PATTERN   0xA5A5A5A5u
#define MSP_STACK_GUARD_PATTERN  0xD15EA5E5u

typedef struct
{
    uint32_t magic;
    uint32_t exception_number;
    uint32_t exc_return;
    uint32_t stacked_sp;
    uint32_t stack_valid;

    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;

    uint32_t icsr;
    uint32_t vtor;
    uint32_t shcsr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t afsr;
} fault_record_t;

volatile uint32_t kernel_ticks;
volatile fault_record_t fault_record;

#define UART_RX_RING_CAPACITY 128u
#define UART_RX_RING_MASK     (UART_RX_RING_CAPACITY - 1u)

static volatile uint8_t uart_rx_ring[UART_RX_RING_CAPACITY];
static volatile uint32_t uart_rx_head;
static volatile uint32_t uart_rx_tail;
static volatile uint32_t uart_rx_irq_count;
static volatile uint32_t uart_rx_byte_count;
static volatile uint32_t uart_rx_drop_count;
static volatile uint32_t uart_rx_error_count;
static volatile uint32_t uart_rx_high_water;

typedef enum
{
    CONSOLE_TRANSPORT_UART = 0,
    CONSOLE_TRANSPORT_USB_CDC = 1
} console_transport_t;

static console_transport_t console_output_transport = CONSOLE_TRANSPORT_UART;

static uint8_t oled_framebuffer[SSD1306_FRAMEBUFFER_BYTES];
static mono_fb_t oled_surface;
static oled_console_t oled_console_state;

static volatile uint32_t scheduler_workload_task0_started;
static volatile uint32_t scheduler_workload_task0_done;
static volatile uint32_t scheduler_workload_peer_overlap;
static volatile uint32_t scheduler_workload_peer_done;
static volatile uint32_t scheduler_workload_ui_result;

static uint32_t production_console_stack
    [PRODUCTION_CONSOLE_STACK_WORDS]
    __attribute__((aligned(8)));

static uint32_t production_console_task_cookie;
static volatile uint32_t production_console_task_started;
static volatile uint32_t production_console_wait_count;
static volatile uint32_t production_console_wake_count;
static volatile uint32_t production_console_wake_events;
static volatile uint32_t production_console_command_count;
static volatile uint32_t production_console_fault;

static uint32_t production_heartbeat_stack
    [PRODUCTION_HEARTBEAT_STACK_WORDS]
    __attribute__((aligned(8)));

static uint32_t production_heartbeat_task_cookie;
static volatile uint32_t production_heartbeat_task_started;
static volatile uint32_t production_heartbeat_count;
static volatile uint32_t production_heartbeat_led_on;
static volatile uint32_t production_heartbeat_fault;
static volatile uint32_t production_watchdog_active;
static volatile uint32_t production_watchdog_reload_count;
static uint32_t production_reset_flags;
static uint32_t production_iwdg_reset;

static volatile uint32_t scheduler_console_probe_task_started;
static volatile uint32_t scheduler_console_probe_task_done;
static volatile uint32_t scheduler_console_probe_peer_overlap;
static volatile uint32_t scheduler_console_probe_peer_done;
static volatile uint32_t scheduler_console_probe_completed_count;
static volatile uint32_t scheduler_console_probe_ui_restore_result;

static volatile uint32_t scheduler_isolation_task_started;
static volatile uint32_t scheduler_isolation_task_done;
static volatile uint32_t scheduler_isolation_peer_overlap;
static volatile uint32_t scheduler_isolation_peer_done;
static volatile uint32_t scheduler_isolation_init_reject;
static volatile uint32_t scheduler_isolation_active_preserved;
static volatile uint32_t scheduler_diagnostic_busy_count;

static volatile uint32_t scheduler_wait_wake_task_started;
static volatile uint32_t scheduler_wait_wake_task_resumed;
static volatile uint32_t scheduler_wait_wake_peer_done;
static volatile uint32_t scheduler_wait_wake_events;
static volatile uint32_t scheduler_wait_wake_byte;
static volatile uint32_t scheduler_wait_wake_byte_ok;
static volatile uint32_t scheduler_wait_wake_framing_bytes;

static int console_execute_safe_named(const char *command);
static int console_execute_scheduler_diagnostic(const char *command);
static void console_execute_named(const char *command);
static void console_production_scheduler_stats(void);
static void console_scheduler_console_probe_test(void);
static void console_scheduler_isolation_test(void);
static void console_scheduler_wait_wake_test(void);
static void console_scheduler_timed_test(void);
static void console_scheduler_priority_test(void);

kernel_time_ms_t kernel_time_now(void)
{
    return kernel_ticks;
}

int kernel_time_reached(kernel_time_ms_t now, kernel_time_ms_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}

int kernel_time_elapsed(kernel_time_ms_t start, kernel_time_ms_t duration)
{
    return (kernel_time_ms_t)(kernel_time_now() - start) >= duration;
}

static void production_reset_cause_capture(void)
{
    production_reset_flags = RCC_CSR;
    production_iwdg_reset =
        ((production_reset_flags & RCC_CSR_IWDGRSTF) != 0u) ? 1u : 0u;

    RCC_CSR |= RCC_CSR_RMVF;
}

static void production_watchdog_reload(void)
{
    if (production_watchdog_active != 0u)
    {
        iwdg_reload();
        ++production_watchdog_reload_count;
    }
}

static uint32_t clock_init(void)
{
    FLASH_ACR = FLASH_PRFTBE | FLASH_LATENCY_2;

    RCC_CR |= RCC_HSEON;
    while ((RCC_CR & RCC_HSERDY) == 0u)
    {
    }

    RCC_CFGR =
        RCC_PPRE1_DIV2 |
        RCC_ADCPRE_DIV6 |
        RCC_PLLSRC_HSE |
        RCC_PLLMUL_X9;

    RCC_CR |= RCC_PLLON;
    while ((RCC_CR & RCC_PLLRDY) == 0u)
    {
    }

    RCC_CFGR = (RCC_CFGR & ~RCC_SW_MASK) | RCC_SW_PLL;
    while ((RCC_CFGR & RCC_SWS_MASK) != RCC_SWS_PLL)
    {
    }

    /*
     * STM32F103 USB FS requires 48 MHz. With the accepted 72 MHz PLL,
     * USBPRE=0 selects PLLCLK/1.5 = 48 MHz before USBEN is asserted.
     */
    RCC_CFGR &= ~RCC_USBPRE;

    return 72000000u;
}

static void gpio_init(void)
{
    RCC_APB2ENR |= RCC_IOPCEN;

    /* PC13: general-purpose push-pull output, 2 MHz. */
    GPIOC_CRH &= ~(0xFu << 20);
    GPIOC_CRH |=  (0x2u << 20);

    /* Blue Pill LED is active-low. */
    GPIOC_BSRR = GPIO_PIN_13;
}

static void uart_init(void)
{
    /*
     * Enable GPIOA and USART1 on APB2.
     *
     * PA9 / USART1_TX:
     * MODE9 = 11 -> output, max speed 50 MHz
     * CNF9  = 10 -> alternate-function push-pull
     * nibble = 0b1011 = 0xB
     */
    RCC_APB2ENR |= RCC_IOPAEN | RCC_USART1EN;

    /*
     * PA10 / USART1_RX:
     * MODE10 = 00 -> input
     * CNF10  = 01 -> floating input
     * nibble = 0b0100 = 0x4
     *
     * HW-193 TXD is connected to PA10 for the bidirectional console.
     */
    GPIOA_CRH &= ~((0xFu << 4) | (0xFu << 8));
    GPIOA_CRH |=  ((0xBu << 4) | (0x4u << 8));

    uart_rx_head = 0u;
    uart_rx_tail = 0u;
    uart_rx_irq_count = 0u;
    uart_rx_byte_count = 0u;
    uart_rx_drop_count = 0u;
    uart_rx_error_count = 0u;
    uart_rx_high_water = 0u;

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

static void uart_putc(char c)
{
    while ((USART1_SR & USART_SR_TXE) == 0u)
    {
    }

    USART1_DR = (uint32_t)(uint8_t)c;
}

static void console_putc(char c)
{
    if (console_output_transport == CONSOLE_TRANSPORT_USB_CDC)
    {
        (void)usb_cdc_write_byte((uint8_t)c);
        return;
    }

    uart_putc(c);
}

static int uart_try_getc(char *c)
{
    const uint32_t tail = uart_rx_tail;

    if (tail == uart_rx_head)
    {
        return 0;
    }

    *c = (char)uart_rx_ring[tail & UART_RX_RING_MASK];
    uart_rx_tail = tail + 1u;

    return 1;
}

void USART1_IRQHandler(void)
{
    const uint32_t status = USART1_SR;

    ++uart_rx_irq_count;

    if ((status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) != 0u)
    {
        ++uart_rx_error_count;
    }

    if ((status & USART_SR_RXNE) != 0u)
    {
        const uint8_t byte = (uint8_t)USART1_DR;
        const uint32_t head = uart_rx_head;
        const uint32_t depth = head - uart_rx_tail;

        ++uart_rx_byte_count;

        if (depth < UART_RX_RING_CAPACITY)
        {
            const uint32_t next_head = head + 1u;
            const uint32_t next_depth = depth + 1u;

            uart_rx_ring[head & UART_RX_RING_MASK] = byte;
            uart_rx_head = next_head;

            scheduler_event_signal(
                PRODUCTION_UART_RX_EVENT);

            if (next_depth > uart_rx_high_water)
            {
                uart_rx_high_water = next_depth;
            }
        }
        else
        {
            ++uart_rx_drop_count;
        }
    }
    else if ((status & (USART_SR_PE | USART_SR_FE | USART_SR_NE | USART_SR_ORE)) != 0u)
    {
        (void)USART1_DR;
    }
}

static void usb_cdc_rx_event_notify(void)
{
    scheduler_event_signal(PRODUCTION_USB_CDC_RX_EVENT);
}

static void uart_write(const char *text)
{
    while (*text != '\0')
    {
        console_putc(*text);
        ++text;
    }
}

static void uart_write_hex32(uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    uart_write("0x");

    for (uint32_t shift = 28u;; shift -= 4u)
    {
        console_putc(hex[(value >> shift) & 0xFu]);

        if (shift == 0u)
        {
            break;
        }
    }
}

static void uart_write_line(const char *text)
{
    uart_write(text);
    uart_write("\r\n");
}

static void uart_boot_banner(uint32_t core_clock_hz)
{
    uart_write_line("STM32 OS");
    uart_write_line("BOOT OK");

    uart_write("SYSCLK=");
    uart_write_hex32(core_clock_hz);
    uart_write("\r\n");

    uart_write("TICK_HZ=");
    uart_write_hex32(1000u);
    uart_write("\r\n");

    uart_write("FAULTREC=");
    uart_write_hex32((uint32_t)&fault_record);
    uart_write("\r\n");
}

#define CONSOLE_COMMAND_CAPACITY 32u

typedef struct
{
    char data[CONSOLE_COMMAND_CAPACITY];
    uint32_t length;
    console_transport_t transport;
} console_command_state_t;
static console_command_state_t uart_command_state =
{
    { 0 },
    0u,
    CONSOLE_TRANSPORT_UART
};
static console_command_state_t usb_cdc_command_state =
{
    { 0 },
    0u,
    CONSOLE_TRANSPORT_USB_CDC
};

static int text_equals(const char *a, const char *b)
{
    while ((*a != '\0') && (*b != '\0'))
    {
        if (*a != *b)
        {
            return 0;
        }

        ++a;
        ++b;
    }

    return (*a == '\0') && (*b == '\0');
}

static void i2c1_init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB_CRL &= ~((0xFu << 24) | (0xFu << 28));
    GPIOB_CRL |=  ((0xFu << 24) | (0xFu << 28));

    RCC_APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC_APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;

    I2C1_CR1 = 0u;
    I2C1_CR2 = I2C1_PCLK_MHZ;
    I2C1_CCR = I2C1_CCR_100KHZ;
    I2C1_TRISE = I2C1_TRISE_100KHZ;
    I2C1_CR1 = I2C_CR1_PE;
}

static int i2c1_wait_bus_free(void)
{
    uint32_t spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR2 & I2C_SR2_BUSY) != 0u)
    {
        if (spins == 0u)
        {
            return 0;
        }

        --spins;
    }

    return 1;
}

static int i2c1_probe(uint8_t address)
{
    uint32_t spins;
    uint32_t sr1;

    if (i2c1_wait_bus_free() == 0)
    {
        return -1;
    }

    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
    I2C1_CR1 |= I2C_CR1_START;

    spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR1 & I2C_SR1_SB) == 0u)
    {
        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }

        --spins;
    }

    I2C1_DR = ((uint32_t)address << 1);

    spins = I2C_SPIN_LIMIT;

    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_ADDR) != 0u)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            I2C1_CR1 |= I2C_CR1_STOP;
            return 1;
        }

        if ((sr1 & I2C_SR1_AF) != 0u)
        {
            I2C1_SR1 &= ~I2C_SR1_AF;
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if ((sr1 & (I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return -1;
        }

        --spins;
    }
}

int i2c1_write(uint8_t address, const uint8_t *data, uint32_t length)
{
    uint32_t spins;
    uint32_t sr1;
    uint32_t index;

    if ((data == (const uint8_t *)0) || (length == 0u))
    {
        return 0;
    }

    if (i2c1_wait_bus_free() == 0)
    {
        return 0;
    }

    I2C1_SR1 &= ~(I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF);
    I2C1_CR1 |= I2C_CR1_START;

    spins = I2C_SPIN_LIMIT;

    while ((I2C1_SR1 & I2C_SR1_SB) == 0u)
    {
        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        --spins;
    }

    I2C1_DR = ((uint32_t)address << 1);

    spins = I2C_SPIN_LIMIT;

    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_ADDR) != 0u)
        {
            (void)I2C1_SR1;
            (void)I2C1_SR2;
            break;
        }

        if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        --spins;
    }

    for (index = 0u; index < length; ++index)
    {
        spins = I2C_SPIN_LIMIT;

        for (;;)
        {
            sr1 = I2C1_SR1;

            if ((sr1 & I2C_SR1_TXE) != 0u)
            {
                break;
            }

            if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
            {
                I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
                I2C1_CR1 |= I2C_CR1_STOP;
                return 0;
            }

            if (spins == 0u)
            {
                I2C1_CR1 |= I2C_CR1_STOP;
                return 0;
            }

            --spins;
        }

        I2C1_DR = data[index];
    }

    spins = I2C_SPIN_LIMIT;

    for (;;)
    {
        sr1 = I2C1_SR1;

        if ((sr1 & I2C_SR1_BTF) != 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 1;
        }

        if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0u)
        {
            I2C1_SR1 &= ~(I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        if (spins == 0u)
        {
            I2C1_CR1 |= I2C_CR1_STOP;
            return 0;
        }

        --spins;
    }
}







static void oled_fb_draw_char_1x(uint32_t x, uint32_t y, char c)
{
    const mono_font_metrics_t *metrics = font5x7_metrics();
    const uint8_t *glyph = font5x7_glyph(c);
    uint32_t column;
    uint32_t row;

    for (column = 0u; column < metrics->glyph_width; ++column)
    {
        uint8_t bits = glyph[column];

        for (row = 0u; row < metrics->glyph_height; ++row)
        {
            int value =
                (bits & (uint8_t)(1u << row)) != 0u;

            mono_fb_set_pixel(
                &oled_surface,
                (int32_t)(x + column),
                (int32_t)(y + row),
                value);
        }
    }
}

static void oled_fb_draw_text_1x(uint32_t x, uint32_t y, const char *text)
{
    while ((*text != '\0') && (x < SSD1306_WIDTH))
    {
        oled_fb_draw_char_1x(x, y, *text);
        x += OLED_GLYPH_ADVANCE;
        ++text;
    }
}

static int ssd1306_show_text_demo(void)
{
    if (ssd1306_init() == 0)
    {
        return 0;
    }

    mono_fb_clear(&oled_surface);
    mono_fb_rect(
        &oled_surface,
        0,
        0,
        (int32_t)SSD1306_WIDTH,
        (int32_t)SSD1306_HEIGHT,
        1);
    oled_fb_draw_text_1x(43u, 12u, "DEUS OS");

    if (ssd1306_present_full(oled_framebuffer) == 0)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static void console_oled_text(void)
{
    if (ssd1306_show_text_demo() != 0)
    {
        uart_write_line("OLED_TEXT_OK");
    }
    else
    {
        uart_write_line("OLED_TEXT_ERR");
    }
}

static void oled_renderer_write(
    mono_rect_t clip,
    int32_t x,
    int32_t y,
    const char *text)
{
    while (*text != '\0')
    {
        text_renderer_draw_cell(
            &oled_surface,
            clip,
            x,
            y,
            *text);

        x += 6;
        ++text;
    }
}

static int ssd1306_show_generic_renderer_test(void)
{
    static const mono_rect_t clip =
    {
        1,
        1,
        126,
        30
    };

    if (ssd1306_init() == 0)
    {
        return 0;
    }

    mono_fb_clear(&oled_surface);
    mono_fb_rect(
        &oled_surface,
        0,
        0,
        (int32_t)SSD1306_WIDTH,
        (int32_t)SSD1306_HEIGHT,
        1);

    /*
     * All rows go through the same generic per-pixel path.
     * There is deliberately no page-aligned renderer optimization.
     */
    oled_renderer_write(clip, 8, 8, "GENERIC");
    oled_renderer_write(clip, 8, 16, "OPAQUE");
    oled_renderer_write(clip, 8, 24, "EDGE");

    /*
     * Opaque overwrite test.
     * The final cell must contain only I. Any remaining M pixels indicate
     * that background/spacer pixels were not deterministically overwritten.
     */
    text_renderer_draw_cell(&oled_surface, clip, 80, 16, 'M');
    text_renderer_draw_cell(&oled_surface, clip, 80, 16, 'I');

    /*
     * Right-edge clip test.
     * R occupies x=122..126. Its spacer column would be x=127, which belongs
     * to the frame and is outside the clip rectangle.
     */
    text_renderer_draw_cell(&oled_surface, clip, 122, 8, 'R');

    /*
     * Bottom-edge clip test.
     * EDGE glyph pixels occupy y=24..30. Cell spacer row y=31 belongs to the
     * frame and is outside the clip rectangle.
     */

    if (ssd1306_present_full(oled_framebuffer) == 0)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static void console_oled_render(void)
{
    if (text_renderer_fast_path_self_test() == 0)
    {
        uart_write_line("OLED_RENDER_EQ_ERR");
        return;
    }

    uart_write_line("OLED_RENDER_EQ_OK");

    if (ssd1306_show_generic_renderer_test() != 0)
    {
        uart_write_line("OLED_RENDER_OK");
    }
    else
    {
        uart_write_line("OLED_RENDER_ERR");
    }
}

static void oled_ui_fill_console_proof(void)
{
    oled_console_clear(&oled_console_state);

    oled_console_write(
        &oled_console_state,
        "ABCDEFGHIJKLMNOPQRSTU");

    oled_console_write_line(
        &oled_console_state,
        "ROW2 OPAQUE");

    oled_console_write(
        &oled_console_state,
        "ROW3 BOTTOM");

    /*
     * Leave the cursor in the pending-next-line state.
     * Do not inject hidden text: Slice 5 makes that state scrollable.
     */
    oled_console_putc(
        &oled_console_state,
        '\n');
}

static int ssd1306_show_ui_layout(
    const oled_ui_layout_t *layout)
{
    oled_status_bar_t status;

    if (
        (layout == (const oled_ui_layout_t *)0) ||
        (oled_ui_layout_validate(layout) == 0)
    ) {
        return 0;
    }

    if (ssd1306_init() == 0)
    {
        return 0;
    }

    mono_fb_clear(&oled_surface);

    oled_status_bar_init(&status);
    oled_status_bar_set_time(
        &status,
        0u,
        0u);

    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    oled_ui_fill_console_proof();

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static int ssd1306_show_console_test(void)
{
    return ssd1306_show_ui_layout(
        oled_ui_layout_default());
}

static void oled_ui_fill_scroll_proof(void)
{
    oled_console_clear(&oled_console_state);

    oled_console_write_line(
        &oled_console_state,
        "SCROLL ONE");

    oled_console_write_line(
        &oled_console_state,
        "SCROLL TWO");

    oled_console_write_line(
        &oled_console_state,
        "SCROLL THREE");

    oled_console_write_line(
        &oled_console_state,
        "SCROLL FOUR");
}

static int ssd1306_show_scroll_test(void)
{
    const oled_ui_layout_t *layout;
    oled_status_bar_t status;

    layout = oled_ui_layout_default();

    if (
        (layout == (const oled_ui_layout_t *)0) ||
        (oled_ui_layout_validate(layout) == 0)
    ) {
        return 0;
    }

    if (ssd1306_init() == 0)
    {
        return 0;
    }

    mono_fb_clear(&oled_surface);

    oled_status_bar_init(&status);
    oled_status_bar_set_time(
        &status,
        0u,
        0u);

    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    oled_ui_fill_scroll_proof();

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static void console_oled_scroll(void)
{
    if (oled_console_scroll_self_test() == 0)
    {
        uart_write_line("OLED_SCROLL_STATE_ERR");
        uart_write_line("OLED_SCROLL_ERR");
        return;
    }

    uart_write_line("OLED_SCROLL_STATE_OK");

    if (ssd1306_show_scroll_test() != 0)
    {
        uart_write_line("OLED_SCROLL_OK");
    }
    else
    {
        uart_write_line("OLED_SCROLL_ERR");
    }
}

static int ssd1306_show_dirty_present_test(void)
{
    uint32_t i;

    if (ssd1306_init() == 0)
    {
        return 0;
    }

    /*
     * Establish a known all-black controller image using the new API.
     * mono_fb_clear() must mark all four pages dirty, and a successful
     * present must clear the mask.
     */
    mono_fb_clear(&oled_surface);

    if (mono_fb_dirty_pages(&oled_surface) != 0x0Fu)
    {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        return 0;
    }

    /*
     * Deliberately make every backing-RAM page white WITHOUT marking any
     * page dirty. Then mark only physical page 2 dirty. A correct dirty
     * presenter sends only page 2: the OLED ends with one white 8-pixel
     * band at y=16..23 while pages 0, 1 and 3 remain black.
     */
    for (i = 0u; i < SSD1306_FRAMEBUFFER_BYTES; ++i)
    {
        oled_framebuffer[i] = 0xFFu;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        return 0;
    }

    mono_fb_set_pixel(
        &oled_surface,
        0,
        16,
        1);

    if (mono_fb_dirty_pages(&oled_surface) != 0x04u)
    {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        return 0;
    }

    /*
     * Zero-dirty present must be a successful no-op.
     */
    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static void console_oled_dirty(void)
{
    if (ssd1306_show_dirty_present_test() != 0)
    {
        uart_write_line("OLED_DIRTY_MASK_OK");
        uart_write_line("OLED_DIRTY_CLEAR_OK");
        uart_write_line("OLED_DIRTY_IDLE_OK");
        uart_write_line("OLED_DIRTY_OK");
    }
    else
    {
        uart_write_line("OLED_DIRTY_ERR");
    }
}

static int oled_runtime_ui_show(void)
{
    const oled_ui_layout_t *layout;
    oled_status_bar_t status;

    layout = oled_ui_layout_default();

    if (
        (layout == (const oled_ui_layout_t *)0) ||
        (oled_ui_layout_validate(layout) == 0)
    ) {
        return 0;
    }

    if (ssd1306_init() == 0)
    {
        return 0;
    }

    mono_fb_clear(&oled_surface);

    oled_status_bar_init(&status);
    oled_status_bar_set_time(
        &status,
        0u,
        0u);

    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    oled_console_clear(&oled_console_state);

    oled_console_write_line(
        &oled_console_state,
        "DEUS OS");

    oled_console_write_line(
        &oled_console_state,
        "BOOT OK");

    oled_console_write(
        &oled_console_state,
        "READY");

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (oled_console_state.dirty_rows != 0u)
    {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        return 0;
    }

    return ssd1306_display_on();
}

static void scheduler_workload_oled_task(void *argument)
{
    (void)argument;

    scheduler_workload_task0_started = 1u;
    scheduler_workload_ui_result =
        (oled_runtime_ui_show() != 0) ? 1u : 0u;
    scheduler_workload_task0_done = 1u;
}

static void scheduler_workload_cpu_peer_task(void *argument)
{
    uint32_t i;

    (void)argument;

    for (i = 0u; i < SCHED_WORKLOAD_PEER_SPINS; ++i)
    {
        if (
            (scheduler_workload_task0_started != 0u) &&
            (scheduler_workload_task0_done == 0u)
        ) {
            scheduler_workload_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_workload_peer_done = 1u;
}


static void console_oled_runtime(void)
{
    if (oled_runtime_ui_show() != 0)
    {
        uart_write_line("OLED_RUNTIME_UI_OK");
    }
    else
    {
        uart_write_line("OLED_RUNTIME_UI_ERR");
    }
}
static void console_oled_ui_update(void)
{
    const oled_ui_layout_t *layout;
    oled_status_bar_t status;
    uint8_t row_mask;
    int proof_ok = 0;

    layout = oled_ui_layout_default();

    if (
        (layout == (const oled_ui_layout_t *)0) ||
        (oled_ui_layout_validate(layout) == 0)
    ) {
        uart_write_line("OLED_UI_LAYOUT_PATH_ERR");
        return;
    }

    if (ssd1306_init() == 0)
    {
        uart_write_line("OLED_UI_INIT_ERR");
        return;
    }

    mono_fb_clear(&oled_surface);

    oled_status_bar_init(&status);
    oled_status_bar_set_time(
        &status,
        0u,
        0u);

    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    oled_console_clear(&oled_console_state);

    oled_console_write_line(
        &oled_console_state,
        "UI PATH ONE");

    oled_console_write_line(
        &oled_console_state,
        "UI PATH TWO");

    oled_console_write(
        &oled_console_state,
        "UI PATH THREE");

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (oled_console_state.dirty_rows != 0u)
    {
        uart_write_line("OLED_UI_BASE_CONSOLE_DIRTY_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0x0Fu)
    {
        uart_write("OLED_UI_BASE_MASK=");
        uart_write_hex32(
            (uint32_t)mono_fb_dirty_pages(&oled_surface));
        uart_write_line("");
        uart_write_line("OLED_UI_BASE_MASK_ERR");
        goto display_on;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        uart_write_line("OLED_UI_BASE_PRESENT_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        uart_write_line("OLED_UI_BASE_CLEAR_ERR");
        goto display_on;
    }

    oled_console_state.cursor_x = 0u;
    oled_console_state.cursor_y = 1u;

    oled_console_write(
        &oled_console_state,
        "DIRTY PAGE2 UPDATE   ");

    if (oled_console_state.dirty_rows != 0x02u)
    {
        uart_write("OLED_UI_CONSOLE_MASK=");
        uart_write_hex32(
            (uint32_t)oled_console_state.dirty_rows);
        uart_write_line("");
        uart_write_line("OLED_UI_CONSOLE_MASK_ERR");
        goto display_on;
    }

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (oled_console_state.dirty_rows != 0u)
    {
        uart_write_line("OLED_UI_CONSOLE_CONSUME_ERR");
        goto display_on;
    }

    uart_write_line("OLED_UI_CONSOLE_DIRTY_OK");

    row_mask = mono_fb_dirty_pages(&oled_surface);

    uart_write("OLED_UI_ROW_MASK=");
    uart_write_hex32((uint32_t)row_mask);
    uart_write_line("");

    /*
     * Present the actual dirty set before validating it. Even if a future
     * regression widens the mask, the panel is still restored and left on.
     */
    if (ssd1306_present(&oled_surface) == 0)
    {
        uart_write_line("OLED_UI_ROW_PRESENT_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        uart_write_line("OLED_UI_ROW_CLEAR_ERR");
        goto display_on;
    }

    uart_write_line("OLED_UI_DIRTY_PRESENT_OK");

    if (row_mask != 0x04u)
    {
        uart_write_line("OLED_UI_DIRTY_RENDER_ERR");
        goto display_on;
    }

    uart_write_line("OLED_UI_DIRTY_RENDER_OK");
    proof_ok = 1;

display_on:
    if (ssd1306_display_on() == 0)
    {
        uart_write_line("OLED_UI_DISPLAY_ON_ERR");
        return;
    }

    uart_write_line("OLED_UI_DISPLAY_ON_OK");

    if (proof_ok != 0)
    {
        uart_write_line("OLED_UI_UPDATE_OK");
    }
    else
    {
        uart_write_line("OLED_UI_UPDATE_ERR");
    }
}

static void console_oled_console(void)
{
    if (ssd1306_show_console_test() != 0)
    {
        uart_write_line("OLED_CONSOLE_OK");
    }
    else
    {
        uart_write_line("OLED_CONSOLE_ERR");
    }
}

static int ssd1306_show_status_test(void)
{
    return ssd1306_show_ui_layout(
        oled_ui_layout_default());
}

static void console_oled_status(void)
{
    if (oled_ui_layout_self_test() == 0)
    {
        uart_write_line("OLED_UI_LAYOUT_ERR");
        uart_write_line("OLED_STATUS_ERR");
        return;
    }

    uart_write_line("OLED_UI_LAYOUT_OK");

    if (oled_status_bar_self_test() == 0)
    {
        uart_write_line("OLED_STATUS_REFERENCE_ERR");
        uart_write_line("OLED_STATUS_ERR");
        return;
    }

    uart_write_line("OLED_STATUS_REFERENCE_OK");

    if (ssd1306_show_status_test() != 0)
    {
        uart_write_line("OLED_STATUS_OK");
    }
    else
    {
        uart_write_line("OLED_STATUS_ERR");
    }
}







static void console_oled_test(void)
{
    if (ssd1306_show_checkerboard() != 0)
    {
        uart_write_line("OLED_TEST_OK");
    }
    else
    {
        uart_write_line("OLED_TEST_ERR");
    }
}

static void console_oled_ping(void)
{
    if (ssd1306_ping() != 0)
    {
        uart_write_line("OLED_CMD_OK");
    }
    else
    {
        uart_write_line("OLED_CMD_ERR");
    }
}

static void console_i2c_scan(void)
{
    uint32_t address;
    uint32_t count = 0u;

    uart_write_line("I2C_SCAN");

    for (address = 0x08u; address <= 0x77u; ++address)
    {
        int result = i2c1_probe((uint8_t)address);

        if (result < 0)
        {
            uart_write_line("I2C_BUS_ERR");
            return;
        }

        if (result != 0)
        {
            uart_write("ADDR=");
            uart_write_hex32(address);
            uart_write("\r\n");
            ++count;
        }
    }

    uart_write("COUNT=");
    uart_write_hex32(count);
    uart_write("\r\n");
}

static void console_write_fault(void)
{
    uart_write("FAULTREC=");
    uart_write_hex32((uint32_t)&fault_record);
    uart_write("\r\n");

    uart_write("MAGIC=");
    uart_write_hex32(fault_record.magic);
    uart_write(" EXC=");
    uart_write_hex32(fault_record.exception_number);
    uart_write("\r\n");

    uart_write("SP=");
    uart_write_hex32(fault_record.stacked_sp);
    uart_write(" VALID=");
    uart_write_hex32(fault_record.stack_valid);
    uart_write("\r\n");

    uart_write("PC=");
    uart_write_hex32(fault_record.pc);
    uart_write(" LR=");
    uart_write_hex32(fault_record.lr);
    uart_write("\r\n");

    uart_write("CFSR=");
    uart_write_hex32(SCB_CFSR);
    uart_write(" HFSR=");
    uart_write_hex32(SCB_HFSR);
    uart_write(" SHCSR=");
    uart_write_hex32(SCB_SHCSR);
    uart_write("\r\n");
}

static void console_scheduler_test(void)
{
    if (scheduler_self_test() != 0)
    {
        uart_write_line("SCHED_FOUNDATION_OK");
    }
    else
    {
        uart_write_line("SCHED_FOUNDATION_ERR");
    }
}

static void console_scheduler_cooperative_test(void)
{
    if (scheduler_cooperative_self_test() != 0)
    {
        uart_write_line("SCHED_COOP_OK");
    }
    else
    {
        uart_write_line("SCHED_COOP_ERR");
    }
}

static void console_scheduler_preemptive_test(void)
{
    if (scheduler_preemptive_self_test() != 0)
    {
        uart_write_line("SCHED_PREEMPT_OK");
    }
    else
    {
        uart_write_line("SCHED_PREEMPT_ERR");
    }
}

static void console_scheduler_stack_water_test(void)
{
    uint32_t coop_used0;
    uint32_t coop_used1;
    uint32_t preempt_used0;
    uint32_t preempt_used1;
    uint32_t capacity0;
    uint32_t capacity1;
    int coop_canary0;
    int coop_canary1;
    int preempt_canary0;
    int preempt_canary1;
    int coop_result;
    int preempt_result;
    int passed;

    coop_result = scheduler_cooperative_self_test();
    coop_used0 = scheduler_stack_high_water_bytes(0u);
    coop_used1 = scheduler_stack_high_water_bytes(1u);
    coop_canary0 = scheduler_stack_canary_intact(0u);
    coop_canary1 = scheduler_stack_canary_intact(1u);

    preempt_result = scheduler_preemptive_self_test();
    preempt_used0 = scheduler_stack_high_water_bytes(0u);
    preempt_used1 = scheduler_stack_high_water_bytes(1u);
    preempt_canary0 = scheduler_stack_canary_intact(0u);
    preempt_canary1 = scheduler_stack_canary_intact(1u);

    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);

    uart_write("STACK_CAPACITY=");
    uart_write_hex32(capacity0);
    uart_write("\r\n");

    uart_write("STACK_COOP_T0_USED=");
    uart_write_hex32(coop_used0);
    uart_write("\r\n");

    uart_write("STACK_COOP_T1_USED=");
    uart_write_hex32(coop_used1);
    uart_write("\r\n");

    uart_write("STACK_PREEMPT_T0_USED=");
    uart_write_hex32(preempt_used0);
    uart_write("\r\n");

    uart_write("STACK_PREEMPT_T1_USED=");
    uart_write_hex32(preempt_used1);
    uart_write("\r\n");

    passed =
        (coop_result != 0) &&
        (preempt_result != 0) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (coop_used0 >= 64u) &&
        (coop_used0 < capacity0) &&
        (coop_used1 >= 64u) &&
        (coop_used1 < capacity1) &&
        (preempt_used0 >= 64u) &&
        (preempt_used0 < capacity0) &&
        (preempt_used1 >= 64u) &&
        (preempt_used1 < capacity1) &&
        (coop_canary0 != 0) &&
        (coop_canary1 != 0) &&
        (preempt_canary0 != 0) &&
        (preempt_canary1 != 0);

    if (passed != 0)
    {
        uart_write_line("SCHED_STACK_WATER_OK");
    }
    else
    {
        uart_write_line("SCHED_STACK_WATER_ERR");
    }
}

static void console_scheduler_workload_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t switches;
    int canary0;
    int canary1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        uart_write_line("SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    scheduler_workload_task0_started = 0u;
    scheduler_workload_task0_done = 0u;
    scheduler_workload_peer_overlap = 0u;
    scheduler_workload_peer_done = 0u;
    scheduler_workload_ui_result = 0u;

    if (
        scheduler_task_prepare(
            0u,
            scheduler_workload_oled_task,
            (void *)0) == 0
    ) {
        uart_write_line("SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    if (
        scheduler_task_prepare(
            1u,
            scheduler_workload_cpu_peer_task,
            (void *)0) == 0
    ) {
        uart_write_line("SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    uart_write("WORKLOAD_CAPACITY=");
    uart_write_hex32(capacity0);
    uart_write("\r\n");

    uart_write("WORKLOAD_T0_USED=");
    uart_write_hex32(used0);
    uart_write("\r\n");

    uart_write("WORKLOAD_T1_USED=");
    uart_write_hex32(used1);
    uart_write("\r\n");

    uart_write("WORKLOAD_SWITCHES=");
    uart_write_hex32(switches);
    uart_write("\r\n");

    uart_write("WORKLOAD_UI_RESULT=");
    uart_write_hex32(scheduler_workload_ui_result);
    uart_write("\r\n");

    uart_write("WORKLOAD_PEER_OVERLAP=");
    uart_write_hex32(scheduler_workload_peer_overlap);
    uart_write("\r\n");

    uart_write("WORKLOAD_CANARY_T0=");
    uart_write_hex32((canary0 != 0) ? 1u : 0u);
    uart_write("\r\n");

    uart_write("WORKLOAD_CANARY_T1=");
    uart_write_hex32((canary1 != 0) ? 1u : 0u);
    uart_write("\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_workload_task0_started != 0u) &&
        (scheduler_workload_task0_done != 0u) &&
        (scheduler_workload_peer_done != 0u) &&
        (scheduler_workload_peer_overlap != 0u) &&
        (scheduler_workload_ui_result != 0u) &&
        (switches >= 2u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    if (passed != 0)
    {
        uart_write_line("SCHED_WORKLOAD_OK");
    }
    else
    {
        uart_write_line("SCHED_WORKLOAD_ERR");
    }
}

static void console_uart_rx_stats(void)
{
    const uint32_t head = uart_rx_head;
    const uint32_t tail = uart_rx_tail;
    const uint32_t depth = head - tail;
    const uint32_t irq_count = uart_rx_irq_count;
    const uint32_t byte_count = uart_rx_byte_count;
    const uint32_t drop_count = uart_rx_drop_count;
    const uint32_t error_count = uart_rx_error_count;
    const uint32_t high_water = uart_rx_high_water;

    uart_write("RX_CAPACITY=");
    uart_write_hex32(UART_RX_RING_CAPACITY);
    uart_write("\r\n");

    uart_write("RX_IRQ_COUNT=");
    uart_write_hex32(irq_count);
    uart_write("\r\n");

    uart_write("RX_BYTE_COUNT=");
    uart_write_hex32(byte_count);
    uart_write("\r\n");

    uart_write("RX_DROP_COUNT=");
    uart_write_hex32(drop_count);
    uart_write("\r\n");

    uart_write("RX_ERROR_COUNT=");
    uart_write_hex32(error_count);
    uart_write("\r\n");

    uart_write("RX_HIGH_WATER=");
    uart_write_hex32(high_water);
    uart_write("\r\n");

    uart_write("RX_DEPTH=");
    uart_write_hex32(depth);
    uart_write("\r\n");

    if (
        (irq_count != 0u) &&
        (byte_count != 0u) &&
        (drop_count == 0u) &&
        (error_count == 0u) &&
        (high_water != 0u) &&
        (high_water <= UART_RX_RING_CAPACITY) &&
        (depth <= UART_RX_RING_CAPACITY))
    {
        uart_write_line("RX_IRQ_RING_OK");
    }
    else
    {
        uart_write_line("RX_IRQ_RING_ERR");
    }
}

static uint32_t msp_stack_reserved_bytes(void)
{
    return (uint32_t)(
        (uintptr_t)&_estack -
        (uintptr_t)&_smsp_stack);
}

static uint32_t msp_stack_capacity_bytes(void)
{
    return (uint32_t)(
        (uintptr_t)&_estack -
        (uintptr_t)&_emsp_guard);
}

static int msp_stack_canary_intact(void)
{
    volatile const uint32_t *word =
        (volatile const uint32_t *)&_smsp_stack;
    volatile const uint32_t *end =
        (volatile const uint32_t *)&_emsp_guard;

    while (word < end)
    {
        if (*word != MSP_STACK_GUARD_PATTERN)
        {
            return 0;
        }

        ++word;
    }

    return 1;
}

static uint32_t msp_stack_high_water_bytes(void)
{
    volatile const uint32_t *word =
        (volatile const uint32_t *)&_emsp_guard;
    volatile const uint32_t *top =
        (volatile const uint32_t *)&_estack;

    while ((word < top) && (*word == MSP_STACK_FILL_PATTERN))
    {
        ++word;
    }

    return (uint32_t)(
        (uintptr_t)&_estack -
        (uintptr_t)word);
}

static uint32_t msp_stack_current_used_bytes(void)
{
    uint32_t current_msp;

    __asm volatile ("mrs %0, msp" : "=r" (current_msp));

    if (current_msp >= (uint32_t)(uintptr_t)&_estack)
    {
        return 0u;
    }

    if (current_msp <= (uint32_t)(uintptr_t)&_smsp_stack)
    {
        return MSP_STACK_RESERVED_BYTES;
    }

    return (uint32_t)(
        (uintptr_t)&_estack -
        (uintptr_t)current_msp);
}

static void console_msp_stack_stats(void)
{
    const uint32_t reserved = msp_stack_reserved_bytes();
    const uint32_t capacity = msp_stack_capacity_bytes();
    const uint32_t used = msp_stack_high_water_bytes();
    const uint32_t current = msp_stack_current_used_bytes();
    const uint32_t margin = (used < capacity) ? (capacity - used) : 0u;
    const uint32_t canary = (msp_stack_canary_intact() != 0) ? 1u : 0u;

    uart_write("MSP_RESERVED=");
    uart_write_hex32(reserved);
    uart_write("\r\n");

    uart_write("MSP_GUARD=");
    uart_write_hex32(MSP_STACK_GUARD_BYTES);
    uart_write("\r\n");

    uart_write("MSP_CAPACITY=");
    uart_write_hex32(capacity);
    uart_write("\r\n");

    uart_write("MSP_USED=");
    uart_write_hex32(used);
    uart_write("\r\n");

    uart_write("MSP_MARGIN=");
    uart_write_hex32(margin);
    uart_write("\r\n");

    uart_write("MSP_CURRENT=");
    uart_write_hex32(current);
    uart_write("\r\n");

    uart_write("MSP_CANARY=");
    uart_write_hex32(canary);
    uart_write("\r\n");

    if (
        (reserved == MSP_STACK_RESERVED_BYTES) &&
        (capacity == MSP_STACK_CAPACITY_BYTES) &&
        (used > 0u) &&
        (used < capacity) &&
        (current > 0u) &&
        (current < capacity) &&
        (canary != 0u))
    {
        uart_write_line("MSP_STACK_OK");
    }
    else
    {
        uart_write_line("MSP_STACK_ERR");
    }
}

static const char * const scheduler_console_probe_commands
    [SCHED_CONSOLE_PROBE_COMMAND_COUNT] =
{
    "ping",
    "uptime",
    "health",
    "rxstat",
    "mspstat",
    "fault",
    "i2cscan",
    "oledping",
    "oledtest",
    "oledtext",
    "oledrender",
    "oledconsole",
    "oledscroll",
    "oleddirty",
    "oleduiupdate",
    "uiruntime",
    "oledstatus"
};

static void scheduler_console_probe_task(void *argument)
{
    uint32_t index;

    if (argument != (void *)scheduler_console_probe_commands)
    {
        scheduler_console_probe_task_done = 1u;
        return;
    }

    scheduler_console_probe_task_started = 1u;

    for (index = 0u;
         index < SCHED_CONSOLE_PROBE_COMMAND_COUNT;
         ++index)
    {
        if (
            console_execute_safe_named(
                scheduler_console_probe_commands[index]) != 0
        ) {
            ++scheduler_console_probe_completed_count;
        }
    }

    scheduler_console_probe_ui_restore_result =
        (oled_runtime_ui_show() != 0) ? 1u : 0u;

    scheduler_console_probe_task_done = 1u;
}

static void scheduler_console_probe_peer_task(void *argument)
{
    volatile uint32_t spin;

    if (argument != (void *)0)
    {
        scheduler_console_probe_peer_done = 1u;
        return;
    }

    for (spin = 0u;
         spin < SCHED_WORKLOAD_PEER_SPINS;
         ++spin)
    {
        if (
            (scheduler_console_probe_task_started != 0u) &&
            (scheduler_console_probe_task_done == 0u)
        ) {
            scheduler_console_probe_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_console_probe_peer_done = 1u;
}

static void console_scheduler_console_probe_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t margin0;
    uint32_t switches;
    int canary0;
    int canary1;
    int bind_result;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        uart_write_line("SCHED_CONSOLE_PROBE_PREPARE_ERR");
        return;
    }

    scheduler_console_probe_task_started = 0u;
    scheduler_console_probe_task_done = 0u;
    scheduler_console_probe_peer_overlap = 0u;
    scheduler_console_probe_peer_done = 0u;
    scheduler_console_probe_completed_count = 0u;
    scheduler_console_probe_ui_restore_result = 0u;

    bind_result =
        scheduler_task_stack_bind(
            0u,
            production_console_stack,
            PRODUCTION_CONSOLE_STACK_WORDS);

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_console_probe_task,
            (void *)scheduler_console_probe_commands);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_console_probe_peer_task,
            (void *)0);

    if (
        (bind_result == 0) ||
        (prepare0 == 0) ||
        (prepare1 == 0)
    ) {
        uart_write_line("SCHED_CONSOLE_PROBE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    margin0 = (used0 < capacity0) ? (capacity0 - used0) : 0u;
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    uart_write("CONSOLE_PROBE_CAPACITY=");
    uart_write_hex32(capacity0);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_USED=");
    uart_write_hex32(used0);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_MARGIN=");
    uart_write_hex32(margin0);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_PEER_CAPACITY=");
    uart_write_hex32(capacity1);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_PEER_USED=");
    uart_write_hex32(used1);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_SWITCHES=");
    uart_write_hex32(switches);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_OVERLAP=");
    uart_write_hex32(scheduler_console_probe_peer_overlap);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_SURFACE_COUNT=");
    uart_write_hex32(SCHED_CONSOLE_PROBE_COMMAND_COUNT);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_COMPLETED=");
    uart_write_hex32(scheduler_console_probe_completed_count);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_UI_RESTORE=");
    uart_write_hex32(scheduler_console_probe_ui_restore_result);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_CANARY=");
    uart_write_hex32((canary0 != 0) ? 1u : 0u);
    uart_write("\r\n");

    uart_write("CONSOLE_PROBE_PEER_CANARY=");
    uart_write_hex32((canary1 != 0) ? 1u : 0u);
    uart_write("\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_console_probe_task_started != 0u) &&
        (scheduler_console_probe_task_done != 0u) &&
        (scheduler_console_probe_peer_done != 0u) &&
        (scheduler_console_probe_peer_overlap != 0u) &&
        (scheduler_console_probe_completed_count ==
            SCHED_CONSOLE_PROBE_COMMAND_COUNT) &&
        (scheduler_console_probe_ui_restore_result != 0u) &&
        (switches >= 2u) &&
        (capacity0 == PRODUCTION_CONSOLE_STACK_BYTES) &&
        (capacity1 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (margin0 >= PRODUCTION_CONSOLE_MIN_MARGIN_BYTES) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    if (passed != 0)
    {
        uart_write_line("SCHED_CONSOLE_PROBE_OK");
    }
    else
    {
        uart_write_line("SCHED_CONSOLE_PROBE_ERR");
    }
}

static void scheduler_wait_wake_task(void *argument)
{
    uint32_t events;
    char byte = 0;

    if (argument != (void *)0)
    {
        scheduler_wait_wake_task_resumed = 1u;
        return;
    }

    scheduler_wait_wake_task_started = 1u;

    /*
     * console_drain_uart_rx() executes a UART command as soon as it consumes
     * CR or LF.
     * With a normal CRLF host line, the second delimiter can still be queued
     * when this scheduler diagnostic starts. Framing bytes are not payload.
     */
    while (uart_try_getc(&byte) != 0)
    {
        if ((byte == '\r') || (byte == '\n'))
        {
            ++scheduler_wait_wake_framing_bytes;
            continue;
        }

        scheduler_wait_wake_byte =
            (uint32_t)(uint8_t)byte;
        scheduler_wait_wake_task_resumed = 1u;
        return;
    }

    /*
     * This token now means the scheduler is active and the wait task itself
     * has reached the wait protocol. A UART event racing with the following
     * SVC is safely latched by scheduler_event_signal().
     */
    uart_write_line("SCHED_WAIT_WAKE_ARMED");

    for (;;)
    {
        events =
            scheduler_wait_events(
                PRODUCTION_UART_RX_EVENT);

        scheduler_wait_wake_events |= events;

        if ((events & PRODUCTION_UART_RX_EVENT) == 0u)
        {
            break;
        }

        while (uart_try_getc(&byte) != 0)
        {
            if ((byte == '\r') || (byte == '\n'))
            {
                ++scheduler_wait_wake_framing_bytes;
                continue;
            }

            scheduler_wait_wake_byte =
                (uint32_t)(uint8_t)byte;

            if (
                (uint32_t)(uint8_t)byte ==
                SCHED_WAIT_WAKE_SENTINEL
            ) {
                scheduler_wait_wake_byte_ok = 1u;
            }

            scheduler_wait_wake_task_resumed = 1u;
            return;
        }
    }

    scheduler_wait_wake_task_resumed = 1u;
}

static void scheduler_wait_wake_peer_task(void *argument)
{
    if (argument == (void *)0)
    {
        scheduler_wait_wake_peer_done = 1u;
    }
}

static void console_scheduler_wait_wake_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t idle_waits;
    int canary0;
    int canary1;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        uart_write_line("SCHED_WAIT_WAKE_PREPARE_ERR");
        return;
    }

    scheduler_wait_wake_task_started = 0u;
    scheduler_wait_wake_task_resumed = 0u;
    scheduler_wait_wake_peer_done = 0u;
    scheduler_wait_wake_events = 0u;
    scheduler_wait_wake_byte = 0u;
    scheduler_wait_wake_byte_ok = 0u;
    scheduler_wait_wake_framing_bytes = 0u;

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_wait_wake_task,
            (void *)0);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_wait_wake_peer_task,
            (void *)0);

    if ((prepare0 == 0) || (prepare1 == 0))
    {
        uart_write_line("SCHED_WAIT_WAKE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    idle_waits = scheduler_idle_wait_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    uart_write("SCHED_WAIT_WAKE_EVENTS=");
    uart_write_hex32(scheduler_wait_wake_events);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_BYTE=");
    uart_write_hex32(scheduler_wait_wake_byte);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_FRAMING_BYTES=");
    uart_write_hex32(scheduler_wait_wake_framing_bytes);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_IDLE_WAITS=");
    uart_write_hex32(idle_waits);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_USED_T0=");
    uart_write_hex32(used0);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_USED_T1=");
    uart_write_hex32(used1);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_CANARY_T0=");
    uart_write_hex32((canary0 != 0) ? 1u : 0u);
    uart_write("\r\n");

    uart_write("SCHED_WAIT_WAKE_CANARY_T1=");
    uart_write_hex32((canary1 != 0) ? 1u : 0u);
    uart_write("\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_wait_wake_task_started != 0u) &&
        (scheduler_wait_wake_task_resumed != 0u) &&
        (scheduler_wait_wake_peer_done != 0u) &&
        (scheduler_wait_wake_events ==
            PRODUCTION_UART_RX_EVENT) &&
        (scheduler_wait_wake_byte_ok != 0u) &&
        (idle_waits != 0u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE) &&
        (task0->wait_events == 0u) &&
        (task1->wait_events == 0u) &&
        (scheduler_is_active() == 0);

    if (passed != 0)
    {
        uart_write_line("SCHED_WAIT_WAKE_OK");
    }
    else
    {
        uart_write_line("SCHED_WAIT_WAKE_ERR");
    }
}

static const char * const scheduler_isolation_diagnostic_commands
    [SCHED_ISOLATION_DIAGNOSTIC_COUNT] =
{
    "schedtest",
    "schedcoop",
    "schedpreempt",
    "schedstack",
    "schedworkload",
    "schedconsoleprobe",
    "schedwaitwake"
};

static void scheduler_isolation_task(void *argument)
{
    uint32_t index;

    if (argument != (void *)scheduler_isolation_diagnostic_commands)
    {
        scheduler_isolation_active_preserved = 0u;
        scheduler_isolation_task_done = 1u;
        return;
    }

    scheduler_isolation_task_started = 1u;

    if (scheduler_init() == 0)
    {
        scheduler_isolation_init_reject = 1u;
    }

    if (scheduler_is_active() == 0)
    {
        scheduler_isolation_active_preserved = 0u;
    }

    for (index = 0u;
         index < SCHED_ISOLATION_DIAGNOSTIC_COUNT;
         ++index)
    {
        if (
            console_execute_scheduler_diagnostic(
                scheduler_isolation_diagnostic_commands[index]) == 0
        ) {
            scheduler_isolation_active_preserved = 0u;
        }

        if (scheduler_is_active() == 0)
        {
            scheduler_isolation_active_preserved = 0u;
        }
    }

    scheduler_isolation_task_done = 1u;
}

static void scheduler_isolation_peer_task(void *argument)
{
    volatile uint32_t spin;

    if (argument != (void *)0)
    {
        scheduler_isolation_peer_done = 1u;
        return;
    }

    for (spin = 0u;
         spin < SCHED_WORKLOAD_PEER_SPINS;
         ++spin)
    {
        if (
            (scheduler_isolation_task_started != 0u) &&
            (scheduler_isolation_task_done == 0u)
        ) {
            scheduler_isolation_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_isolation_peer_done = 1u;
}

static void console_scheduler_isolation_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t switches;
    int canary0;
    int canary1;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        uart_write_line("SCHED_ISOLATE_PREPARE_ERR");
        return;
    }

    scheduler_isolation_task_started = 0u;
    scheduler_isolation_task_done = 0u;
    scheduler_isolation_peer_overlap = 0u;
    scheduler_isolation_peer_done = 0u;
    scheduler_isolation_init_reject = 0u;
    scheduler_isolation_active_preserved = 1u;
    scheduler_diagnostic_busy_count = 0u;

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_isolation_task,
            (void *)scheduler_isolation_diagnostic_commands);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_isolation_peer_task,
            (void *)0);

    if ((prepare0 == 0) || (prepare1 == 0))
    {
        uart_write_line("SCHED_ISOLATE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    uart_write("SCHED_ISOLATE_CAPACITY_T0=");
    uart_write_hex32(capacity0);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_USED_T0=");
    uart_write_hex32(used0);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_CAPACITY_T1=");
    uart_write_hex32(capacity1);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_USED_T1=");
    uart_write_hex32(used1);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_SWITCHES=");
    uart_write_hex32(switches);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_INIT_REJECT=");
    uart_write_hex32(scheduler_isolation_init_reject);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_BLOCKED_DIAGNOSTICS=");
    uart_write_hex32(scheduler_diagnostic_busy_count);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_ACTIVE_PRESERVED=");
    uart_write_hex32(scheduler_isolation_active_preserved);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_OVERLAP=");
    uart_write_hex32(scheduler_isolation_peer_overlap);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_CANARY_T0=");
    uart_write_hex32((canary0 != 0) ? 1u : 0u);
    uart_write("\r\n");

    uart_write("SCHED_ISOLATE_CANARY_T1=");
    uart_write_hex32((canary1 != 0) ? 1u : 0u);
    uart_write("\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_isolation_task_started != 0u) &&
        (scheduler_isolation_task_done != 0u) &&
        (scheduler_isolation_peer_done != 0u) &&
        (scheduler_isolation_init_reject != 0u) &&
        (scheduler_diagnostic_busy_count ==
            SCHED_ISOLATION_DIAGNOSTIC_COUNT) &&
        (scheduler_isolation_active_preserved != 0u) &&
        (scheduler_isolation_peer_overlap != 0u) &&
        (switches >= 2u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE) &&
        (scheduler_is_active() == 0);

    if (passed != 0)
    {
        uart_write_line("SCHED_ISOLATE_OK");
    }
    else
    {
        uart_write_line("SCHED_ISOLATE_ERR");
    }
}



static uint32_t cpu_control_get(void)
{
    uint32_t value;

    __asm volatile (
        "mrs %0, control"
        : "=r" (value));

    return value;
}

static uint32_t cpu_psp_get(void)
{
    uint32_t value;

    __asm volatile (
        "mrs %0, psp"
        : "=r" (value));

    return value;
}

static void console_production_scheduler_stats(void)
{
    const scheduler_task_t *task0 = scheduler_task_get(0u);
    const scheduler_task_t *task1 = scheduler_task_get(1u);
    const uint32_t active =
        (scheduler_is_active() != 0) ? 1u : 0u;
    const uint32_t control = cpu_control_get();
    const uint32_t psp = cpu_psp_get();
    const uint32_t thread_psp =
        ((control & 0x2u) != 0u) ? 1u : 0u;
    const uint32_t console_capacity =
        scheduler_stack_capacity_bytes(0u);
    const uint32_t console_used =
        scheduler_stack_high_water_bytes(0u);
    const uint32_t console_margin =
        (console_used < console_capacity) ?
            (console_capacity - console_used) :
            0u;
    const uint32_t console_canary =
        (scheduler_stack_canary_intact(0u) != 0) ?
            1u :
            0u;
    const uint32_t heartbeat_capacity =
        scheduler_stack_capacity_bytes(1u);
    const uint32_t heartbeat_used =
        scheduler_stack_high_water_bytes(1u);
    const uint32_t heartbeat_margin =
        (heartbeat_used < heartbeat_capacity) ?
            (heartbeat_capacity - heartbeat_used) :
            0u;
    const uint32_t heartbeat_canary =
        (scheduler_stack_canary_intact(1u) != 0) ?
            1u :
            0u;
    const uint32_t idle_waits =
        scheduler_idle_wait_count_get();
    const uint32_t preempt_switches =
        scheduler_preempt_switch_count_get();
    uint32_t psp_in_range = 0u;
    uint32_t task0_state = 0xFFFFFFFFu;
    uint32_t task0_priority = 0xFFFFFFFFu;
    uint32_t task0_wait_events = 0u;
    uint32_t task0_wake_events = 0u;
    uint32_t task1_state = 0xFFFFFFFFu;
    uint32_t task1_priority = 0xFFFFFFFFu;
    int passed;

    if (task0 != (const scheduler_task_t *)0)
    {
        const uint32_t low =
            (uint32_t)(uintptr_t)task0->stack_low;
        const uint32_t high =
            (uint32_t)(uintptr_t)task0->stack_high;

        task0_state = (uint32_t)task0->state;
        task0_priority = task0->priority;
        task0_wait_events = task0->wait_events;
        task0_wake_events = task0->wake_events;

        if ((psp >= low) && (psp <= high))
        {
            psp_in_range = 1u;
        }
    }

    if (task1 != (const scheduler_task_t *)0)
    {
        task1_state = (uint32_t)task1->state;
        task1_priority = task1->priority;
    }

    uart_write("SCHED_PROD_ACTIVE=");
    uart_write_hex32(active);
    uart_write("\r\n");

    uart_write_line("SCHED_PROD_MODE=COOPERATIVE");

    uart_write("SCHED_PROD_THREAD_PSP=");
    uart_write_hex32(thread_psp);
    uart_write("\r\n");

    uart_write("SCHED_PROD_CONTROL=");
    uart_write_hex32(control);
    uart_write("\r\n");

    uart_write("SCHED_PROD_PSP=");
    uart_write_hex32(psp);
    uart_write("\r\n");

    uart_write("SCHED_PROD_PSP_IN_RANGE=");
    uart_write_hex32(psp_in_range);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK0_STATE=");
    uart_write_hex32(task0_state);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK0_PRIORITY=");
    uart_write_hex32(task0_priority);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK0_WAIT_EVENTS=");
    uart_write_hex32(task0_wait_events);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK0_WAKE_EVENTS=");
    uart_write_hex32(task0_wake_events);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK1_STATE=");
    uart_write_hex32(task1_state);
    uart_write("\r\n");

    uart_write("SCHED_PROD_TASK1_PRIORITY=");
    uart_write_hex32(task1_priority);
    uart_write("\r\n");

    uart_write("SCHED_PROD_CONSOLE_CAPACITY=");
    uart_write_hex32(console_capacity);
    uart_write("\r\n");

    uart_write("SCHED_PROD_CONSOLE_USED=");
    uart_write_hex32(console_used);
    uart_write("\r\n");

    uart_write("SCHED_PROD_CONSOLE_MARGIN=");
    uart_write_hex32(console_margin);
    uart_write("\r\n");

    uart_write("SCHED_PROD_CONSOLE_CANARY=");
    uart_write_hex32(console_canary);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_STARTED=");
    uart_write_hex32(production_heartbeat_task_started);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_COUNT=");
    uart_write_hex32(production_heartbeat_count);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_LED_ON=");
    uart_write_hex32(production_heartbeat_led_on);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_FAULT=");
    uart_write_hex32(production_heartbeat_fault);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_CAPACITY=");
    uart_write_hex32(heartbeat_capacity);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_USED=");
    uart_write_hex32(heartbeat_used);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_MARGIN=");
    uart_write_hex32(heartbeat_margin);
    uart_write("\r\n");

    uart_write("SCHED_PROD_HEARTBEAT_CANARY=");
    uart_write_hex32(heartbeat_canary);
    uart_write("\r\n");

    uart_write("SCHED_PROD_IDLE_WAITS=");
    uart_write_hex32(idle_waits);
    uart_write("\r\n");

    uart_write("SCHED_PROD_WAIT_COUNT=");
    uart_write_hex32(production_console_wait_count);
    uart_write("\r\n");

    uart_write("SCHED_PROD_WAKE_COUNT=");
    uart_write_hex32(production_console_wake_count);
    uart_write("\r\n");

    uart_write("SCHED_PROD_WAKE_EVENTS=");
    uart_write_hex32(production_console_wake_events);
    uart_write("\r\n");

    uart_write("SCHED_PROD_COMMAND_COUNT=");
    uart_write_hex32(production_console_command_count);
    uart_write("\r\n");

    uart_write("SCHED_PROD_DIAG_BUSY_COUNT=");
    uart_write_hex32(scheduler_diagnostic_busy_count);
    uart_write("\r\n");

    uart_write("SCHED_PROD_PREEMPT_SWITCHES=");
    uart_write_hex32(preempt_switches);
    uart_write("\r\n");

    uart_write("SCHED_PROD_FAULT=");
    uart_write_hex32(production_console_fault);
    uart_write("\r\n");

    passed =
        (active != 0u) &&
        (production_console_task_started != 0u) &&
        (production_heartbeat_task_started != 0u) &&
        (thread_psp != 0u) &&
        (psp_in_range != 0u) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_READY) &&
        (task0->priority == SCHEDULER_PRIORITY_DEFAULT) &&
        (
            (task1->state == SCHEDULER_TASK_READY) ||
            (task1->state == SCHEDULER_TASK_BLOCKED)
        ) &&
        (task1->priority == SCHEDULER_PRIORITY_LOWEST) &&
        (console_capacity == PRODUCTION_CONSOLE_STACK_BYTES) &&
        (console_margin >= PRODUCTION_CONSOLE_MIN_MARGIN_BYTES) &&
        (console_canary != 0u) &&
        (heartbeat_capacity == PRODUCTION_HEARTBEAT_STACK_BYTES) &&
        (heartbeat_margin >= PRODUCTION_HEARTBEAT_MIN_MARGIN_BYTES) &&
        (heartbeat_canary != 0u) &&
        (preempt_switches == 0u) &&
        (production_console_fault == 0u) &&
        (production_heartbeat_fault == 0u);

    if (passed != 0)
    {
        uart_write_line("SCHED_PROD_OK");
    }
    else
    {
        uart_write_line("SCHED_PROD_ERR");
    }
}

static void console_scheduler_priority_test(void)
{
    const scheduler_task_t *task0 = scheduler_task_get(0u);
    const scheduler_task_t *task1 = scheduler_task_get(1u);
    uint32_t task0_priority_before = 0xFFFFFFFFu;
    uint32_t task0_priority_after = 0xFFFFFFFFu;
    uint32_t task1_priority_before = 0xFFFFFFFFu;
    uint32_t task1_priority_after = 0xFFFFFFFFu;
    uint32_t self_test;
    int task0_active_set_result;
    int task1_active_set_result;
    int passed = 1;

    if (task0 != (const scheduler_task_t *)0)
    {
        task0_priority_before = task0->priority;
    }

    if (task1 != (const scheduler_task_t *)0)
    {
        task1_priority_before = task1->priority;
    }

    self_test = scheduler_priority_self_test();

    task0_active_set_result =
        scheduler_task_priority_set(
            0u,
            SCHEDULER_PRIORITY_HIGHEST);

    task1_active_set_result =
        scheduler_task_priority_set(
            1u,
            SCHEDULER_PRIORITY_DEFAULT);

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    if (task0 != (const scheduler_task_t *)0)
    {
        task0_priority_after = task0->priority;
    }

    if (task1 != (const scheduler_task_t *)0)
    {
        task1_priority_after = task1->priority;
    }

    uart_write_line("SCHED_PRIO_POLICY=LOWER_VALUE_HIGHER");

    uart_write("SCHED_PRIO_HIGHEST=");
    uart_write_hex32(SCHEDULER_PRIORITY_HIGHEST);
    uart_write("\r\n");

    uart_write("SCHED_PRIO_DEFAULT=");
    uart_write_hex32(SCHEDULER_PRIORITY_DEFAULT);
    uart_write("\r\n");

    uart_write("SCHED_PRIO_LOWEST=");
    uart_write_hex32(SCHEDULER_PRIORITY_LOWEST);
    uart_write("\r\n");

    uart_write("SCHED_PRIO_TASK0=");
    uart_write_hex32(task0_priority_after);
    uart_write("\r\n");

    uart_write("SCHED_PRIO_TASK1=");
    uart_write_hex32(task1_priority_after);
    uart_write("\r\n");

    uart_write("SCHED_PRIO_SELFTEST=");
    uart_write_hex32(self_test);
    uart_write("\r\n");

    if (
        (scheduler_is_active() != 0) &&
        (task0_active_set_result == 0) &&
        (task1_active_set_result == 0) &&
        (task0_priority_before == SCHEDULER_PRIORITY_DEFAULT) &&
        (task1_priority_before == SCHEDULER_PRIORITY_LOWEST) &&
        (task0_priority_after == task0_priority_before) &&
        (task1_priority_after == task1_priority_before)
    ) {
        uart_write_line("SCHED_PRIO_ACTIVE_SET_REJECT_OK");
    }
    else
    {
        uart_write_line("SCHED_PRIO_ACTIVE_SET_REJECT_ERR");
        passed = 0;
    }

    if (
        (task0_priority_after != SCHEDULER_PRIORITY_DEFAULT) ||
        (task1_priority_after != SCHEDULER_PRIORITY_LOWEST) ||
        (self_test != 0x0000003Fu)
    ) {
        passed = 0;
    }

    if (passed != 0)
    {
        uart_write_line("SCHED_PRIO_OK");
    }
    else
    {
        uart_write_line("SCHED_PRIO_ERR");
    }
}

static void console_scheduler_timed_test(void)
{
    const uint32_t rx_event =
        (console_output_transport == CONSOLE_TRANSPORT_USB_CDC) ?
            PRODUCTION_USB_CDC_RX_EVENT :
            PRODUCTION_UART_RX_EVENT;
    kernel_time_ms_t start;
    kernel_time_ms_t elapsed;
    uint32_t events;
    int passed = 1;

    if (scheduler_sleep_ms(0u) != 0)
    {
        uart_write_line("SCHED_TIMED_ZERO_OK");
    }
    else
    {
        uart_write_line("SCHED_TIMED_ZERO_ERR");
        passed = 0;
    }

    start = kernel_time_now();

    if (scheduler_sleep_ms(SCHED_TIMED_SLEEP_MS) == 0)
    {
        uart_write_line("SCHED_TIMED_SLEEP_ERR");
        passed = 0;
    }

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    uart_write("SCHED_TIMED_SLEEP_ELAPSED=");
    uart_write_hex32(elapsed);
    uart_write("\r\n");

    if (elapsed >= SCHED_TIMED_SLEEP_MS)
    {
        uart_write_line("SCHED_TIMED_SLEEP_OK");
    }
    else
    {
        uart_write_line("SCHED_TIMED_SLEEP_ERR");
        passed = 0;
    }

    /* Consume a stale notification from the transport that started this test. */
    (void)scheduler_wait_events_timeout(
        rx_event,
        0u);

    start = kernel_time_now();

    events =
        scheduler_wait_events_timeout(
            rx_event,
            SCHED_TIMED_TIMEOUT_MS);

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    uart_write("SCHED_TIMED_TIMEOUT_ELAPSED=");
    uart_write_hex32(elapsed);
    uart_write("\r\n");

    if (
        (events == 0u) &&
        (elapsed >= SCHED_TIMED_TIMEOUT_MS)
    ) {
        uart_write_line("SCHED_TIMED_TIMEOUT_OK");
    }
    else
    {
        uart_write_line("SCHED_TIMED_TIMEOUT_ERR");
        passed = 0;
    }

    (void)scheduler_wait_events_timeout(
        rx_event,
        0u);

    uart_write_line("SCHED_TIMED_EVENT_ARMED");
    start = kernel_time_now();

    events =
        scheduler_wait_events_timeout(
            rx_event,
            SCHED_TIMED_EVENT_TIMEOUT_MS);

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    uart_write("SCHED_TIMED_EVENT_EVENTS=");
    uart_write_hex32(events);
    uart_write("\r\n");

    uart_write("SCHED_TIMED_EVENT_ELAPSED=");
    uart_write_hex32(elapsed);
    uart_write("\r\n");

    if (
        (events == rx_event) &&
        (elapsed < SCHED_TIMED_EVENT_TIMEOUT_MS)
    ) {
        uart_write_line("SCHED_TIMED_EVENT_OK");
    }
    else
    {
        uart_write_line("SCHED_TIMED_EVENT_ERR");
        passed = 0;
    }

    if (passed != 0)
    {
        uart_write_line("SCHED_TIMED_OK");
    }
    else
    {
        uart_write_line("SCHED_TIMED_ERR");
    }
}

static void console_usb_cdc_stats(void)
{
    uart_write("USB_CDC_CONFIGURED=");
    uart_write_hex32(usb_device_diagnostics.cdc_configured);
    uart_write(" CONFIGURATION=");
    uart_write_hex32(usb_device_diagnostics.configuration);
    uart_write(" RX_PACKETS=");
    uart_write_hex32(usb_device_diagnostics.cdc_rx_packet_count);
    uart_write(" RX_BYTES=");
    uart_write_hex32(usb_device_diagnostics.cdc_rx_byte_count);
    uart_write(" RX_DROPS=");
    uart_write_hex32(usb_device_diagnostics.cdc_rx_drop_count);
    uart_write(" RX_HIGH_WATER=");
    uart_write_hex32(usb_device_diagnostics.cdc_rx_high_water);
    uart_write(" TX_PACKETS=");
    uart_write_hex32(usb_device_diagnostics.cdc_tx_packet_count);
    uart_write(" TX_BYTES=");
    uart_write_hex32(usb_device_diagnostics.cdc_tx_byte_count);
    uart_write(" TX_DROPS=");
    uart_write_hex32(usb_device_diagnostics.cdc_tx_drop_count);
    uart_write(" TX_HIGH_WATER=");
    uart_write_hex32(usb_device_diagnostics.cdc_tx_high_water);
    uart_write(" CONTROL=");
    uart_write_hex32(usb_device_diagnostics.cdc_control_line_state);
    uart_write(" BAUD=");
    uart_write_hex32(usb_device_diagnostics.cdc_line_coding_baud);
    uart_write(" STOP=");
    uart_write_hex32(usb_device_diagnostics.cdc_line_coding_stop_bits);
    uart_write(" PARITY=");
    uart_write_hex32(usb_device_diagnostics.cdc_line_coding_parity);
    uart_write(" DATA_BITS=");
    uart_write_hex32(usb_device_diagnostics.cdc_line_coding_data_bits);
    uart_write("\r\n");
}

static int console_execute_safe_named(const char *command)
{
    if (text_equals(command, "ping") != 0)
    {
        uart_write_line("PONG");
    }
    else if (text_equals(command, "uptime") != 0)
    {
        uart_write("UPTIME_MS=");
        uart_write_hex32(kernel_time_now());
        uart_write("\r\n");
    }
    else if (text_equals(command, "health") != 0)
    {
        uart_write("HEALTH TICK=");
        uart_write_hex32(kernel_time_now());
        uart_write(" PC13=");
        uart_write_hex32((GPIOC_ODR & GPIO_PIN_13) != 0u ? 1u : 0u);
        uart_write(" WDOG_ACTIVE=");
        uart_write_hex32(production_watchdog_active);
        uart_write(" WDOG_RELOAD_COUNT=");
        uart_write_hex32(production_watchdog_reload_count);
        uart_write(" RESET_FLAGS=");
        uart_write_hex32(production_reset_flags);
        uart_write(" IWDG_RESET=");
        uart_write_hex32(production_iwdg_reset);
        uart_write("\r\n");
    }
    else if (text_equals(command, "rxstat") != 0)
    {
        console_uart_rx_stats();
    }
    else if (text_equals(command, "cdcstat") != 0)
    {
        console_usb_cdc_stats();
    }
    else if (text_equals(command, "mspstat") != 0)
    {
        console_msp_stack_stats();
    }
    else if (text_equals(command, "schedprod") != 0)
    {
        console_production_scheduler_stats();
    }
    else if (text_equals(command, "schedtimed") != 0)
    {
        console_scheduler_timed_test();
    }
    else if (text_equals(command, "schedprio") != 0)
    {
        console_scheduler_priority_test();
    }
    else if (text_equals(command, "fault") != 0)
    {
        console_write_fault();
    }
    else if (text_equals(command, "i2cscan") != 0)
    {
        console_i2c_scan();
    }
    else if (text_equals(command, "oledping") != 0)
    {
        console_oled_ping();
    }
    else if (text_equals(command, "oledtest") != 0)
    {
        console_oled_test();
    }
    else if (text_equals(command, "oledtext") != 0)
    {
        console_oled_text();
    }
    else if (text_equals(command, "oledrender") != 0)
    {
        console_oled_render();
    }
    else if (text_equals(command, "oledconsole") != 0)
    {
        console_oled_console();
    }
    else if (text_equals(command, "oledscroll") != 0)
    {
        console_oled_scroll();
    }
    else if (text_equals(command, "oleddirty") != 0)
    {
        console_oled_dirty();
    }
    else if (text_equals(command, "oleduiupdate") != 0)
    {
        console_oled_ui_update();
    }
    else if (text_equals(command, "uiruntime") != 0)
    {
        console_oled_runtime();
    }
    else if (text_equals(command, "oledstatus") != 0)
    {
        console_oled_status();
    }
    else
    {
        return 0;
    }

    return 1;
}

static int console_scheduler_diagnostic_block_if_active(void)
{
    if (scheduler_is_active() == 0)
    {
        return 0;
    }

    ++scheduler_diagnostic_busy_count;
    uart_write_line("SCHED_DIAG_BUSY");

    return 1;
}

static int console_execute_scheduler_diagnostic(const char *command)
{
    if (text_equals(command, "schedtest") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_test();
        }
    }
    else if (text_equals(command, "schedcoop") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_cooperative_test();
        }
    }
    else if (text_equals(command, "schedpreempt") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_preemptive_test();
        }
    }
    else if (text_equals(command, "schedstack") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_stack_water_test();
        }
    }
    else if (text_equals(command, "schedworkload") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_workload_test();
        }
    }
    else if (text_equals(command, "schedconsoleprobe") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_console_probe_test();
        }
    }
    else if (text_equals(command, "schedwaitwake") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_wait_wake_test();
        }
    }
    else if (text_equals(command, "schedisolate") != 0)
    {
        if (console_scheduler_diagnostic_block_if_active() == 0)
        {
            console_scheduler_isolation_test();
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

static void console_watchdog_trip(void)
{
    if (production_watchdog_active == 0u)
    {
        uart_write_line("WDOG_TRIP_ERR_INACTIVE");
        return;
    }

    uart_write_line("WDOG_TRIP_ARMED");

    for (;;)
    {
        __asm volatile ("nop");
    }
}

static void console_execute_named(const char *command)
{
    if (text_equals(command, "wdogtrip") != 0)
    {
        console_watchdog_trip();
        return;
    }

    if (console_execute_scheduler_diagnostic(command) != 0)
    {
        return;
    }

    if (console_execute_safe_named(command) == 0)
    {
        uart_write_line("ERR");
    }
}

static void console_execute_state(console_command_state_t *state)
{
    const console_transport_t previous_transport = console_output_transport;

    if ((state == (console_command_state_t *)0) || (state->length == 0u))
    {
        return;
    }

    state->data[state->length] = '\0';
    console_output_transport = state->transport;

    if (production_console_task_started != 0u)
    {
        ++production_console_command_count;
    }

    console_execute_named(state->data);
    state->length = 0u;
    console_output_transport = previous_transport;
}

static void console_feed_char(
    console_command_state_t *state,
    char c)
{
    if (state == (console_command_state_t *)0)
    {
        return;
    }

    if ((c == '\r') || (c == '\n'))
    {
        if (state->length != 0u)
        {
            console_execute_state(state);
        }

        return;
    }

    if ((c == '\b') || ((uint8_t)c == 0x7Fu))
    {
        if (state->length != 0u)
        {
            --state->length;
        }

        return;
    }

    if (state->length < (CONSOLE_COMMAND_CAPACITY - 1u))
    {
        state->data[state->length] = c;
        ++state->length;
    }
    else
    {
        const console_transport_t previous_transport = console_output_transport;

        state->length = 0u;
        console_output_transport = state->transport;
        uart_write_line("ERR");
        console_output_transport = previous_transport;
    }
}

static void console_drain_uart_rx(void)
{
    char c;

    while (uart_try_getc(&c) != 0)
    {
        console_feed_char(&uart_command_state, c);
    }
}

static void console_drain_usb_cdc_rx(void)
{
    char c;

    while (usb_cdc_try_getc(&c) != 0)
    {
        console_feed_char(&usb_cdc_command_state, c);
    }
}


static void production_console_task(void *argument)
{
    uint32_t events;

    if (argument != (void *)&production_console_task_cookie)
    {
        production_console_fault = 1u;
        return;
    }

    production_console_task_started = 1u;
    uart_write_line("SCHED_PROD_CONSOLE_ONLINE");

    for (;;)
    {
        console_drain_uart_rx();
        console_drain_usb_cdc_rx();
        production_watchdog_reload();

        ++production_console_wait_count;

        events =
            scheduler_wait_events(
                PRODUCTION_CONSOLE_RX_EVENTS);

        production_console_wake_events |= events;
        ++production_console_wake_count;

        if ((events & PRODUCTION_CONSOLE_RX_EVENTS) == 0u)
        {
            production_console_fault = 1u;
            return;
        }
    }
}

static void production_heartbeat_task(void *argument)
{
    if (argument != (void *)&production_heartbeat_task_cookie)
    {
        production_heartbeat_fault = 1u;
        return;
    }

    production_heartbeat_task_started = 1u;

    for (;;)
    {
        if (
            scheduler_sleep_ms(
                PRODUCTION_HEARTBEAT_PERIOD_MS) == 0
        ) {
            production_heartbeat_fault = 1u;
            return;
        }

        production_heartbeat_led_on ^= 1u;

        if (production_heartbeat_led_on != 0u)
        {
            GPIOC_BSRR = GPIO_RESET_13;
        }
        else
        {
            GPIOC_BSRR = GPIO_PIN_13;
        }

        ++production_heartbeat_count;
        production_watchdog_reload();
    }
}

__attribute__((noreturn))
static void production_fail_closed(
    const char *reason)
{
    uart_write_line(reason);
    uart_write_line("SCHED_PROD_FATAL");

    for (;;)
    {
        __asm volatile ("wfi");
    }
}

static void faults_init(void)
{
    SCB_SHCSR |=
        SHCSR_MEMFAULTENA |
        SHCSR_BUSFAULTENA |
        SHCSR_USGFAULTENA;
}

static void systick_init(uint32_t core_clock_hz)
{
    SYST_RVR = (core_clock_hz / 1000u) - 1u;
    SYST_CVR = 0u;
    SYST_CSR = SYST_CLKSOURCE | SYST_TICKINT | SYST_ENABLE;
}

static void panic_delay(void)
{
    for (volatile uint32_t i = 0u; i < 900000u; ++i)
    {
        __asm volatile ("nop");
    }
}

__attribute__((noreturn))
void fault_capture(
    uint32_t *stack,
    uint32_t exc_return,
    uint32_t exception_number)
{
    uint32_t stack_address = (uint32_t)stack;
    uint32_t valid =
        ((stack_address & 0x3u) == 0u) &&
        (stack_address >= SRAM_START) &&
        (stack_address <= (SRAM_END - (8u * sizeof(uint32_t))));

    __asm volatile ("cpsid i");

    fault_record.magic = FAULT_MAGIC;
    fault_record.exception_number = exception_number;
    fault_record.exc_return = exc_return;
    fault_record.stacked_sp = stack_address;
    fault_record.stack_valid = valid;

    fault_record.icsr  = SCB_ICSR;
    fault_record.vtor  = SCB_VTOR;
    fault_record.shcsr = SCB_SHCSR;
    fault_record.cfsr  = SCB_CFSR;
    fault_record.hfsr  = SCB_HFSR;
    fault_record.dfsr  = SCB_DFSR;
    fault_record.mmfar = SCB_MMFAR;
    fault_record.bfar  = SCB_BFAR;
    fault_record.afsr  = SCB_AFSR;

    if (valid != 0u)
    {
        fault_record.r0   = stack[0];
        fault_record.r1   = stack[1];
        fault_record.r2   = stack[2];
        fault_record.r3   = stack[3];
        fault_record.r12  = stack[4];
        fault_record.lr   = stack[5];
        fault_record.pc   = stack[6];
        fault_record.xpsr = stack[7];
    }
    else
    {
        fault_record.r0   = 0u;
        fault_record.r1   = 0u;
        fault_record.r2   = 0u;
        fault_record.r3   = 0u;
        fault_record.r12  = 0u;
        fault_record.lr   = 0u;
        fault_record.pc   = 0u;
        fault_record.xpsr = 0u;
    }

    for (;;)
    {
        for (uint32_t i = 0u; i < exception_number; ++i)
        {
            GPIOC_BSRR = GPIO_RESET_13;
            panic_delay();
            GPIOC_BSRR = GPIO_PIN_13;
            panic_delay();
        }

        for (uint32_t i = 0u; i < 4u; ++i)
        {
            panic_delay();
        }
    }
}

void SysTick_Handler(void)
{
    ++kernel_ticks;
    scheduler_tick(kernel_ticks);
}

void kernel_main(void)
{
    uint32_t core_clock_hz;
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    int task0_bind_result;
    int task0_priority_result;
    int task0_prepare_result;
    int task1_bind_result;
    int task1_priority_result;
    int task1_prepare_result;
    int iwdg_start_result;
    int start_result;

    production_reset_cause_capture();
    core_clock_hz = clock_init();

    gpio_init();
    uart_init();
    i2c1_init();
    mono_fb_init(
        &oled_surface,
        oled_framebuffer,
        SSD1306_WIDTH,
        SSD1306_HEIGHT);
    oled_console_init(&oled_console_state);
    faults_init();
    if (usb_device_init(core_clock_hz) == 0)
    {
        production_fail_closed(
            "USB_CORE_INIT_ERR");
    }

    systick_init(core_clock_hz);

    uart_boot_banner(core_clock_hz);

    if (oled_runtime_ui_show() != 0)
    {
        uart_write_line("OLED_RUNTIME_UI_OK");
    }
    else
    {
        uart_write_line("OLED_RUNTIME_UI_ERR");
    }

    /*
     * Normal boot transfers application runtime ownership to two cooperative
     * PSP tasks with disjoint responsibilities.
     *
     * Task 0 owns the production console/runtime path. USART1 IRQ remains the
     * sole USART DR reader, while the USB device IRQ owns CDC endpoint/PMA
     * service. Both IRQ paths publish bytes to independent bounded RX rings
     * before signalling transport-specific scheduler events. Task 0 drains
     * both authoritative rings before each combined event wait.
     *
     * Task 1 owns normal-runtime PC13 heartbeat policy. It sleeps on the
     * scheduler clock and performs one short GPIO update per wake. SysTick
     * owns timekeeping only and never performs normal heartbeat GPIO writes.
     *
     * The fatal fault path intentionally remains an out-of-band PC13 blinker;
     * it is not part of normal production ownership.
     *
     * When both tasks are BLOCKED, the scheduler host owns Thread/MSP and
     * parks with WFE.
     */
    if (scheduler_init() == 0)
    {
        production_fail_closed(
            "SCHED_PROD_INIT_ERR");
    }

    usb_cdc_set_rx_notify(usb_cdc_rx_event_notify);

    uart_command_state.length = 0u;
    usb_cdc_command_state.length = 0u;
    console_output_transport = CONSOLE_TRANSPORT_UART;

    production_console_task_started = 0u;
    production_console_wait_count = 0u;
    production_console_wake_count = 0u;
    production_console_wake_events = 0u;
    production_console_command_count = 0u;
    production_console_fault = 0u;

    production_heartbeat_task_started = 0u;
    production_heartbeat_count = 0u;
    production_heartbeat_led_on = 0u;
    production_heartbeat_fault = 0u;

    production_watchdog_active = 0u;
    production_watchdog_reload_count = 0u;

    task0_bind_result =
        scheduler_task_stack_bind(
            0u,
            production_console_stack,
            PRODUCTION_CONSOLE_STACK_WORDS);

    task0_priority_result =
        scheduler_task_priority_set(
            0u,
            SCHEDULER_PRIORITY_DEFAULT);

    task0_prepare_result =
        scheduler_task_prepare(
            0u,
            production_console_task,
            (void *)&production_console_task_cookie);

    task1_bind_result =
        scheduler_task_stack_bind(
            1u,
            production_heartbeat_stack,
            PRODUCTION_HEARTBEAT_STACK_WORDS);

    task1_priority_result =
        scheduler_task_priority_set(
            1u,
            SCHEDULER_PRIORITY_LOWEST);

    task1_prepare_result =
        scheduler_task_prepare(
            1u,
            production_heartbeat_task,
            (void *)&production_heartbeat_task_cookie);

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    if (
        (task0_bind_result == 0) ||
        (task0_priority_result == 0) ||
        (task0_prepare_result == 0) ||
        (task1_bind_result == 0) ||
        (task1_priority_result == 0) ||
        (task1_prepare_result == 0) ||
        (task0 == (const scheduler_task_t *)0) ||
        (task1 == (const scheduler_task_t *)0) ||
        (task0->state != SCHEDULER_TASK_READY) ||
        (task1->state != SCHEDULER_TASK_READY) ||
        (task0->priority != SCHEDULER_PRIORITY_DEFAULT) ||
        (task1->priority != SCHEDULER_PRIORITY_LOWEST)
    ) {
        production_fail_closed(
            "SCHED_PROD_PREPARE_ERR");
    }

    iwdg_debug_freeze_enable();

    iwdg_start_result =
        iwdg_start(
            PRODUCTION_IWDG_PRESCALER,
            PRODUCTION_IWDG_RELOAD,
            PRODUCTION_IWDG_SPIN_LIMIT);

    if (iwdg_start_result == 0)
    {
        production_fail_closed(
            "IWDG_START_ERR");
    }

    production_watchdog_active = 1u;
    uart_write_line("IWDG_ACTIVE");
    uart_write_line("SCHED_PROD_PREPARE_OK");
    uart_write_line("SCHED_PROD_START");

    start_result = scheduler_start();

    uart_write("SCHED_PROD_RETURN=");
    uart_write_hex32((uint32_t)start_result);
    uart_write("\r\n");

    production_fail_closed(
        "SCHED_PROD_UNEXPECTED_RETURN");
}
