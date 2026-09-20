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
#include "kernel/scheduler_diagnostics.h"
#include "kernel/command_service.h"
#include "kernel/application_runtime.h"
#include "kernel/application_runtime_bridge.h"
#include "kernel/application_commands.h"
#include "kernel/binary_frame.h"
#include "kernel/binary_rpc.h"
#include "kernel/usb_management.h"
#include "kernel/system_identity.h"

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
#define PRODUCTION_UART_RX_EVENT           (1u << 0)
#define PRODUCTION_USB_CDC_RX_EVENT        (1u << 1)
#define PRODUCTION_USB_MANAGEMENT_RX_EVENT (1u << 2)
#define PRODUCTION_CONSOLE_RX_EVENTS       \
    (PRODUCTION_UART_RX_EVENT | \
     PRODUCTION_USB_CDC_RX_EVENT | \
     PRODUCTION_USB_MANAGEMENT_RX_EVENT)
#define BOOT_DESKTOP_UI_SPLASH_MIN_MS    1000u
#define BOOT_DESKTOP_UI_POLL_MS          250u
#define BOOT_DESKTOP_UI_MAX_MINUTES      5999u
#define BOOT_DESKTOP_UI_SATURATE_MINUTES 6000u

typedef enum
{
    BOOT_DESKTOP_UI_SPLASH = 0,
    BOOT_DESKTOP_UI_HOME = 1
} boot_desktop_ui_state_t;

typedef struct
{
    uint8_t state;
    uint8_t system_indicator;
    uint8_t usb_indicator;
    uint8_t network_indicator;
    uint32_t total_minutes;
} boot_desktop_ui_snapshot_t;

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

static uint8_t oled_framebuffer[SSD1306_FRAMEBUFFER_BYTES];
static mono_fb_t oled_surface;
static oled_console_t oled_console_state;
static oled_status_bar_t boot_desktop_ui_status;

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

static boot_desktop_ui_state_t boot_desktop_ui_state;
static kernel_time_ms_t boot_desktop_ui_splash_started_at;
static uint32_t boot_desktop_ui_splash_visible;
static uint32_t boot_desktop_ui_initialized;
static uint32_t boot_desktop_ui_uptime_saturated;
static uint32_t boot_desktop_ui_snapshot_valid;
static uint32_t boot_desktop_ui_panel_initialized;
static boot_desktop_ui_snapshot_t boot_desktop_ui_last_snapshot;

static command_service_status_t console_execute_request(
    const command_service_request_t *request,
    command_service_context_t *context,
    void *handler_context);

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

static int console_uart_write_byte(void *context, uint8_t byte)
{
    (void)context;
    uart_putc((char)byte);
    return 1;
}

static int console_usb_cdc_write_byte(void *context, uint8_t byte)
{
    (void)context;
    return usb_cdc_write_byte(byte);
}

static int binary_rpc_usb_send_wire(
    void *context,
    const uint8_t *data,
    uint32_t length)
{
    (void)context;
    return usb_cdc_write_span_atomic(data, length);
}

static int binary_rpc_management_send_wire(
    void *context,
    const uint8_t *data,
    uint32_t length)
{
    (void)context;
    return usb_management_write_span_atomic(data, length);
}

static binary_frame_parser_t usb_cdc_binary_parser;
static binary_rpc_state_t usb_cdc_binary_rpc_state;
static binary_rpc_workspace_t production_binary_rpc_workspace;

static const binary_rpc_binding_t usb_cdc_binary_rpc_binding =
{
    binary_rpc_usb_send_wire,
    (void *)0,
    console_execute_request,
    (void *)0,
    PRODUCTION_USB_CDC_RX_EVENT
};

static const binary_rpc_binding_t usb_management_binary_rpc_binding =
{
    binary_rpc_management_send_wire,
    (void *)0,
    console_execute_request,
    (void *)0,
    PRODUCTION_USB_MANAGEMENT_RX_EVENT
};

static void console_write(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write(context, text);
}

static void console_write_line(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write_line(context, text);
}

static void console_write_hex32(
    command_service_context_t *context,
    uint32_t value)
{
    (void)command_service_write_hex32(context, value);
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

static void usb_management_rx_event_notify(void)
{
    scheduler_event_signal(PRODUCTION_USB_MANAGEMENT_RX_EVENT);
}

static void uart_emergency_write(const char *text)
{
    while (*text != '\0')
    {
        uart_putc(*text);
        ++text;
    }
}

static void uart_emergency_write_hex32(uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    uart_emergency_write("0x");

    for (uint32_t shift = 28u;; shift -= 4u)
    {
        uart_putc(hex[(value >> shift) & 0xFu]);

        if (shift == 0u)
        {
            break;
        }
    }
}

static void uart_emergency_write_line(const char *text)
{
    uart_emergency_write(text);
    uart_emergency_write("\r\n");
}

static void uart_boot_banner(uint32_t core_clock_hz)
{
    uart_emergency_write_line("STM32 OS");
    uart_emergency_write_line("BOOT OK");

    uart_emergency_write("SYSCLK=");
    uart_emergency_write_hex32(core_clock_hz);
    uart_emergency_write("\r\n");

    uart_emergency_write("TICK_HZ=");
    uart_emergency_write_hex32(1000u);
    uart_emergency_write("\r\n");

    uart_emergency_write("FAULTREC=");
    uart_emergency_write_hex32((uint32_t)&fault_record);
    uart_emergency_write("\r\n");
}

typedef struct
{
    char data[COMMAND_SERVICE_LINE_CAPACITY];
    uint32_t length;
    command_service_context_t context;
} console_command_state_t;

static console_command_state_t uart_command_state =
{
    { 0 },
    0u,
    {
        console_uart_write_byte,
        (void *)0,
        PRODUCTION_UART_RX_EVENT,
        0u
    }
};

static console_command_state_t usb_cdc_command_state =
{
    { 0 },
    0u,
    {
        console_usb_cdc_write_byte,
        (void *)0,
        PRODUCTION_USB_CDC_RX_EVENT,
        0u
    }
};

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

static void console_oled_text(command_service_context_t *context)
{
    if (ssd1306_show_text_demo() != 0)
    {
        console_write_line(context, "OLED_TEXT_OK");
    }
    else
    {
        console_write_line(context, "OLED_TEXT_ERR");
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

static void console_oled_render(command_service_context_t *context)
{
    if (text_renderer_fast_path_self_test() == 0)
    {
        console_write_line(context, "OLED_RENDER_EQ_ERR");
        return;
    }

    console_write_line(context, "OLED_RENDER_EQ_OK");

    if (ssd1306_show_generic_renderer_test() != 0)
    {
        console_write_line(context, "OLED_RENDER_OK");
    }
    else
    {
        console_write_line(context, "OLED_RENDER_ERR");
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
    mono_fb_mark_all_dirty(&oled_surface);

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
    mono_fb_mark_all_dirty(&oled_surface);

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

static void console_oled_scroll(command_service_context_t *context)
{
    if (oled_console_scroll_self_test() == 0)
    {
        console_write_line(context, "OLED_SCROLL_STATE_ERR");
        console_write_line(context, "OLED_SCROLL_ERR");
        return;
    }

    console_write_line(context, "OLED_SCROLL_STATE_OK");

    if (ssd1306_show_scroll_test() != 0)
    {
        console_write_line(context, "OLED_SCROLL_OK");
    }
    else
    {
        console_write_line(context, "OLED_SCROLL_ERR");
    }
}

static int ssd1306_show_dirty_present_test(
    command_service_context_t *context)
{
    const oled_ui_layout_t *layout;
    oled_status_bar_t status;
    ssd1306_present_stats_t stats;
    uint32_t min_x;
    uint32_t max_x;
    uint32_t payload_bytes;

    if (context == (command_service_context_t *)0)
    {
        return 0;
    }

    if (mono_fb_dirty_region_self_test() == 0)
    {
        return 0;
    }

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

    /*
     * Full-width compatibility baseline: four pages, each one exact
     * 0..127 span. This is the historical 572-payload-byte refresh cost.
     */
    mono_fb_clear(&oled_surface);
    mono_fb_mark_all_dirty(&oled_surface);

    if (mono_fb_dirty_pages(&oled_surface) != 0x0Fu)
    {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    ssd1306_present_stats_get(&stats);
    payload_bytes =
        stats.window_payload_bytes +
        stats.data_payload_bytes;

    if (
        (payload_bytes != 572u) ||
        (stats.window_payload_bytes != 28u) ||
        (stats.data_payload_bytes != 544u) ||
        (stats.i2c_write_count != 36u) ||
        (stats.presented_pages != 4u) ||
        (mono_fb_dirty_pages(&oled_surface) != 0u)
    ) {
        return 0;
    }

    console_write(context, "OLED_DIRTY_BASELINE_PAYLOAD=");
    console_write_hex32(context, payload_bytes);
    console_write(context, "\r\n");

    console_write(context, "OLED_DIRTY_BASELINE_WRITES=");
    console_write_hex32(context, stats.i2c_write_count);
    console_write(context, "\r\n");

    /* Clean present must perform no I2C transaction at all. */
    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    ssd1306_present_stats_get(&stats);

    if (
        (stats.window_payload_bytes != 0u) ||
        (stats.data_payload_bytes != 0u) ||
        (stats.i2c_write_count != 0u) ||
        (stats.presented_pages != 0u)
    ) {
        return 0;
    }

    console_write_line(context, "OLED_DIRTY_IDLE_ZERO_IO_OK");

    /* One changed framebuffer byte must transfer one exact column only. */
    mono_fb_set_pixel(
        &oled_surface,
        40,
        16,
        1);

    if (
        (mono_fb_dirty_pages(&oled_surface) != 0x04u) ||
        (mono_fb_dirty_span(
            &oled_surface,
            2u,
            &min_x,
            &max_x) == 0) ||
        (min_x != 40u) ||
        (max_x != 40u)
    ) {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    ssd1306_present_stats_get(&stats);
    payload_bytes =
        stats.window_payload_bytes +
        stats.data_payload_bytes;

    if (
        (payload_bytes != 9u) ||
        (stats.i2c_write_count != 2u) ||
        (stats.presented_pages != 1u) ||
        (mono_fb_dirty_pages(&oled_surface) != 0u)
    ) {
        return 0;
    }

    console_write(context, "OLED_DIRTY_NARROW_PAYLOAD=");
    console_write_hex32(context, payload_bytes);
    console_write(context, "\r\n");

    /*
     * Establish exact status reference 00:00, then prove 00:00 -> 00:01
     * touches only the final 3-pixel digit region.
     */
    mono_fb_clear(&oled_surface);
    mono_fb_mark_all_dirty(&oled_surface);
    oled_status_bar_init(&status);
    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    oled_status_bar_set_time(&status, 0u, 1u);
    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    if (
        (mono_fb_dirty_pages(&oled_surface) != 0x01u) ||
        (mono_fb_dirty_span(
            &oled_surface,
            0u,
            &min_x,
            &max_x) == 0) ||
        (min_x < 123u) ||
        (max_x > 125u)
    ) {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    ssd1306_present_stats_get(&stats);
    payload_bytes =
        stats.window_payload_bytes +
        stats.data_payload_bytes;

    if (
        (payload_bytes > 11u) ||
        (stats.i2c_write_count != 2u) ||
        (stats.presented_pages != 1u)
    ) {
        return 0;
    }

    console_write(context, "OLED_DIRTY_TIME_MIN_X=");
    console_write_hex32(context, min_x);
    console_write(context, "\r\n");
    console_write(context, "OLED_DIRTY_TIME_MAX_X=");
    console_write_hex32(context, max_x);
    console_write(context, "\r\n");
    console_write(context, "OLED_DIRTY_TIME_PAYLOAD=");
    console_write_hex32(context, payload_bytes);
    console_write(context, "\r\n");

    /* Default reference USB is FILLED; change only USB to RING. */
    oled_status_bar_set_indicators(
        &status,
        OLED_STATUS_INDICATOR_FILLED,
        OLED_STATUS_INDICATOR_RING,
        OLED_STATUS_INDICATOR_RING);
    oled_status_bar_render(
        &status,
        &oled_surface,
        layout->status_rect);

    if (
        (mono_fb_dirty_pages(&oled_surface) != 0x01u) ||
        (mono_fb_dirty_span(
            &oled_surface,
            0u,
            &min_x,
            &max_x) == 0) ||
        (min_x < 8u) ||
        (max_x > 12u)
    ) {
        return 0;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        return 0;
    }

    ssd1306_present_stats_get(&stats);
    payload_bytes =
        stats.window_payload_bytes +
        stats.data_payload_bytes;

    if (
        (payload_bytes > 13u) ||
        (stats.i2c_write_count != 2u) ||
        (stats.presented_pages != 1u)
    ) {
        return 0;
    }

    console_write(context, "OLED_DIRTY_USB_MIN_X=");
    console_write_hex32(context, min_x);
    console_write(context, "\r\n");
    console_write(context, "OLED_DIRTY_USB_MAX_X=");
    console_write_hex32(context, max_x);
    console_write(context, "\r\n");
    console_write(context, "OLED_DIRTY_USB_PAYLOAD=");
    console_write_hex32(context, payload_bytes);
    console_write(context, "\r\n");

    return ssd1306_display_on();
}

static void console_oled_dirty(command_service_context_t *context)
{
    if (ssd1306_show_dirty_present_test(context) != 0)
    {
        console_write_line(context, "OLED_DIRTY_MASK_OK");
        console_write_line(context, "OLED_DIRTY_CLEAR_OK");
        console_write_line(context, "OLED_DIRTY_IDLE_OK");
        console_write_line(context, "OLED_DIRTY_REGION_OK");
        console_write_line(context, "OLED_DIRTY_TIME_OK");
        console_write_line(context, "OLED_DIRTY_USB_OK");
        console_write_line(context, "OLED_DIRTY_OK");
    }
    else
    {
        console_write_line(context, "OLED_DIRTY_ERR");
    }
}

static int boot_desktop_ui_runtime_ready(void)
{
    return
        (scheduler_is_active() != 0) &&
        (production_console_task_started != 0u) &&
        (production_heartbeat_task_started != 0u) &&
        (production_watchdog_active != 0u) &&
        (production_console_fault == 0u) &&
        (production_heartbeat_fault == 0u);
}

static void boot_desktop_ui_initialize(void)
{
    boot_desktop_ui_state = BOOT_DESKTOP_UI_SPLASH;
    boot_desktop_ui_splash_started_at = 0u;
    boot_desktop_ui_splash_visible = 0u;
    boot_desktop_ui_uptime_saturated = 0u;
    boot_desktop_ui_snapshot_valid = 0u;
    boot_desktop_ui_panel_initialized = 0u;
    oled_status_bar_init(&boot_desktop_ui_status);
    boot_desktop_ui_initialized = 1u;
}

static void boot_desktop_ui_update_state(void)
{
    if (
        (boot_desktop_ui_initialized != 0u) &&
        (boot_desktop_ui_state == BOOT_DESKTOP_UI_SPLASH) &&
        (boot_desktop_ui_splash_visible != 0u) &&
        (boot_desktop_ui_runtime_ready() != 0) &&
        (kernel_time_elapsed(
            boot_desktop_ui_splash_started_at,
            BOOT_DESKTOP_UI_SPLASH_MIN_MS) != 0)
    ) {
        boot_desktop_ui_state = BOOT_DESKTOP_UI_HOME;
    }
}

static uint32_t boot_desktop_ui_display_minutes(void)
{
    uint32_t total_minutes =
        kernel_time_now() / 60000u;

    if (
        (boot_desktop_ui_uptime_saturated != 0u) ||
        (total_minutes >= BOOT_DESKTOP_UI_SATURATE_MINUTES)
    ) {
        boot_desktop_ui_uptime_saturated = 1u;
        return BOOT_DESKTOP_UI_MAX_MINUTES;
    }

    return total_minutes;
}

static void boot_desktop_ui_snapshot_build(
    boot_desktop_ui_snapshot_t *snapshot)
{
    if (snapshot == (boot_desktop_ui_snapshot_t *)0)
    {
        return;
    }

    snapshot->state = boot_desktop_ui_state;
    snapshot->system_indicator =
        (boot_desktop_ui_runtime_ready() != 0) ?
            OLED_STATUS_INDICATOR_FILLED :
            OLED_STATUS_INDICATOR_RING;
    snapshot->usb_indicator =
        (usb_cdc_is_configured() != 0) ?
            OLED_STATUS_INDICATOR_FILLED :
            OLED_STATUS_INDICATOR_RING;
    snapshot->network_indicator = OLED_STATUS_INDICATOR_RING;
    snapshot->total_minutes =
        boot_desktop_ui_display_minutes();
}

static int boot_desktop_ui_snapshot_equal(
    const boot_desktop_ui_snapshot_t *left,
    const boot_desktop_ui_snapshot_t *right)
{
    if (
        (left == (const boot_desktop_ui_snapshot_t *)0) ||
        (right == (const boot_desktop_ui_snapshot_t *)0)
    ) {
        return 0;
    }

    return
        (left->state == right->state) &&
        (left->system_indicator == right->system_indicator) &&
        (left->usb_indicator == right->usb_indicator) &&
        (left->network_indicator == right->network_indicator) &&
        (left->total_minutes == right->total_minutes);
}

static void boot_desktop_ui_application_snapshot_build(
    const boot_desktop_ui_snapshot_t *ui_snapshot,
    application_service_snapshot_t *application_snapshot)
{
    if (
        (ui_snapshot == (const boot_desktop_ui_snapshot_t *)0) ||
        (application_snapshot == (application_service_snapshot_t *)0)
    ) {
        return;
    }

    application_snapshot->uptime_ms = kernel_time_now();
    application_snapshot->displayed_minute = ui_snapshot->total_minutes;
    application_snapshot->system_healthy =
        (ui_snapshot->system_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            1u : 0u;
    application_snapshot->usb_configured =
        (ui_snapshot->usb_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            1u : 0u;
    application_snapshot->network_online =
        (ui_snapshot->network_indicator == OLED_STATUS_INDICATOR_FILLED) ?
            1u : 0u;
    application_snapshot->reserved = 0u;
}

static int boot_desktop_ui_application_service(
    const boot_desktop_ui_snapshot_t *ui_snapshot)
{
    application_service_snapshot_t current;

    if (ui_snapshot == (const boot_desktop_ui_snapshot_t *)0)
    {
        return 0;
    }

    if (ui_snapshot->state != BOOT_DESKTOP_UI_HOME)
    {
        return 1;
    }

    boot_desktop_ui_application_snapshot_build(ui_snapshot, &current);

    return application_runtime_bridge_service(&current);
}

static int boot_desktop_ui_application_snapshot_current(
    application_service_snapshot_t *snapshot)
{
    boot_desktop_ui_snapshot_t ui_snapshot;

    if (
        (snapshot == (application_service_snapshot_t *)0) ||
        (boot_desktop_ui_state != BOOT_DESKTOP_UI_HOME) ||
        (application_runtime_bridge_is_initialized() == 0)
    ) {
        return 0;
    }

    boot_desktop_ui_snapshot_build(&ui_snapshot);
    boot_desktop_ui_application_snapshot_build(&ui_snapshot, snapshot);
    return 1;
}

static int boot_desktop_ui_render(int force)
{
    const oled_ui_layout_t *layout;
    boot_desktop_ui_snapshot_t snapshot;
    uint32_t hours;
    uint32_t minutes;
    uint32_t display_enable_required = 0u;
    uint32_t full_compose = 0u;

    if (boot_desktop_ui_initialized == 0u)
    {
        boot_desktop_ui_initialize();
    }

    boot_desktop_ui_update_state();
    boot_desktop_ui_snapshot_build(&snapshot);

    if (boot_desktop_ui_application_service(&snapshot) == 0)
    {
        return 0;
    }

    if (
        (force == 0) &&
        (boot_desktop_ui_snapshot_valid != 0u) &&
        (application_runtime_bridge_view_dirty() == 0) &&
        (boot_desktop_ui_snapshot_equal(
            &snapshot,
            &boot_desktop_ui_last_snapshot) != 0)
    ) {
        return 1;
    }

    layout = oled_ui_layout_default();

    if (
        (layout == (const oled_ui_layout_t *)0) ||
        (oled_ui_layout_validate(layout) == 0)
    ) {
        return 0;
    }

    if (boot_desktop_ui_panel_initialized == 0u)
    {
        if (ssd1306_init() == 0)
        {
            return 0;
        }

        boot_desktop_ui_panel_initialized = 1u;
        display_enable_required = 1u;
        full_compose = 1u;
    }

    if (
        (force != 0) ||
        (boot_desktop_ui_snapshot_valid == 0u) ||
        (
            (boot_desktop_ui_snapshot_valid != 0u) &&
            (snapshot.state != boot_desktop_ui_last_snapshot.state)
        )
    ) {
        full_compose = 1u;
    }

    hours = snapshot.total_minutes / 60u;
    minutes = snapshot.total_minutes % 60u;

    oled_status_bar_set_indicators(
        &boot_desktop_ui_status,
        snapshot.system_indicator,
        snapshot.usb_indicator,
        snapshot.network_indicator);
    oled_status_bar_set_time(
        &boot_desktop_ui_status,
        hours,
        minutes);

    if (full_compose != 0u)
    {
        mono_fb_clear(&oled_surface);
        mono_fb_mark_all_dirty(&oled_surface);
        oled_status_bar_mark_all_dirty(&boot_desktop_ui_status);
    }

    oled_status_bar_render(
        &boot_desktop_ui_status,
        &oled_surface,
        layout->status_rect);

    if (
        (snapshot.state == BOOT_DESKTOP_UI_SPLASH) &&
        (full_compose != 0u)
    ) {
        oled_console_clear(&oled_console_state);
        oled_console_write_line(
            &oled_console_state,
            "DEUS OS");
        oled_console_write_line(
            &oled_console_state,
            "STARTING");
        oled_console_write(
            &oled_console_state,
            "PLEASE WAIT");

        oled_console_render(
            &oled_console_state,
            &oled_surface,
            layout->console_rect);

        if (oled_console_state.dirty_rows != 0u)
        {
            boot_desktop_ui_snapshot_valid = 0u;
            return 0;
        }
    }
    else if (
        (snapshot.state == BOOT_DESKTOP_UI_HOME) &&
        (
            (full_compose != 0u) ||
            (application_runtime_bridge_view_dirty() != 0)
        )
    ) {
        if (application_runtime_bridge_apply_view(
                &oled_console_state,
                full_compose) == 0)
        {
            boot_desktop_ui_snapshot_valid = 0u;
            return 0;
        }

        oled_console_render(
            &oled_console_state,
            &oled_surface,
            layout->console_rect);

        if (oled_console_state.dirty_rows != 0u)
        {
            boot_desktop_ui_snapshot_valid = 0u;
            return 0;
        }
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        boot_desktop_ui_panel_initialized = 0u;
        boot_desktop_ui_snapshot_valid = 0u;
        return 0;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        boot_desktop_ui_snapshot_valid = 0u;
        return 0;
    }

    if (
        (display_enable_required != 0u) &&
        (ssd1306_display_on() == 0)
    ) {
        boot_desktop_ui_panel_initialized = 0u;
        boot_desktop_ui_snapshot_valid = 0u;
        return 0;
    }

    if (snapshot.state == BOOT_DESKTOP_UI_HOME)
    {
        application_runtime_bridge_view_consumed();
    }

    if (
        (snapshot.state == BOOT_DESKTOP_UI_SPLASH) &&
        (boot_desktop_ui_splash_visible == 0u)
    ) {
        boot_desktop_ui_splash_started_at = kernel_time_now();
        boot_desktop_ui_splash_visible = 1u;
    }

    boot_desktop_ui_last_snapshot = snapshot;
    boot_desktop_ui_snapshot_valid = 1u;

    return 1;
}

static int oled_runtime_ui_show(void)
{
    return boot_desktop_ui_render(1);
}

static void boot_desktop_ui_service(void)
{
    (void)boot_desktop_ui_render(0);
}


static void console_oled_runtime(command_service_context_t *context)
{
    if (oled_runtime_ui_show() != 0)
    {
        console_write_line(context, "OLED_RUNTIME_UI_OK");
    }
    else
    {
        console_write_line(context, "OLED_RUNTIME_UI_ERR");
    }
}
static void console_oled_ui_update(command_service_context_t *context)
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
        console_write_line(context, "OLED_UI_LAYOUT_PATH_ERR");
        return;
    }

    if (ssd1306_init() == 0)
    {
        console_write_line(context, "OLED_UI_INIT_ERR");
        return;
    }

    mono_fb_clear(&oled_surface);
    mono_fb_mark_all_dirty(&oled_surface);

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
        console_write_line(context, "OLED_UI_BASE_CONSOLE_DIRTY_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0x0Fu)
    {
        console_write(context, "OLED_UI_BASE_MASK=");
        console_write_hex32(context,
            (uint32_t)mono_fb_dirty_pages(&oled_surface));
        console_write_line(context, "");
        console_write_line(context, "OLED_UI_BASE_MASK_ERR");
        goto display_on;
    }

    if (ssd1306_present(&oled_surface) == 0)
    {
        console_write_line(context, "OLED_UI_BASE_PRESENT_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        console_write_line(context, "OLED_UI_BASE_CLEAR_ERR");
        goto display_on;
    }

    oled_console_state.cursor_x = 0u;
    oled_console_state.cursor_y = 1u;

    oled_console_write(
        &oled_console_state,
        "DIRTY PAGE2 UPDATE   ");

    if (oled_console_state.dirty_rows != 0x02u)
    {
        console_write(context, "OLED_UI_CONSOLE_MASK=");
        console_write_hex32(context,
            (uint32_t)oled_console_state.dirty_rows);
        console_write_line(context, "");
        console_write_line(context, "OLED_UI_CONSOLE_MASK_ERR");
        goto display_on;
    }

    oled_console_render(
        &oled_console_state,
        &oled_surface,
        layout->console_rect);

    if (oled_console_state.dirty_rows != 0u)
    {
        console_write_line(context, "OLED_UI_CONSOLE_CONSUME_ERR");
        goto display_on;
    }

    console_write_line(context, "OLED_UI_CONSOLE_DIRTY_OK");

    row_mask = mono_fb_dirty_pages(&oled_surface);

    console_write(context, "OLED_UI_ROW_MASK=");
    console_write_hex32(context, (uint32_t)row_mask);
    console_write_line(context, "");

    /*
     * Present the actual dirty set before validating it. Even if a future
     * regression widens the mask, the panel is still restored and left on.
     */
    if (ssd1306_present(&oled_surface) == 0)
    {
        console_write_line(context, "OLED_UI_ROW_PRESENT_ERR");
        goto display_on;
    }

    if (mono_fb_dirty_pages(&oled_surface) != 0u)
    {
        console_write_line(context, "OLED_UI_ROW_CLEAR_ERR");
        goto display_on;
    }

    console_write_line(context, "OLED_UI_DIRTY_PRESENT_OK");

    if (row_mask != 0x04u)
    {
        console_write_line(context, "OLED_UI_DIRTY_RENDER_ERR");
        goto display_on;
    }

    console_write_line(context, "OLED_UI_DIRTY_RENDER_OK");
    proof_ok = 1;

display_on:
    if (ssd1306_display_on() == 0)
    {
        console_write_line(context, "OLED_UI_DISPLAY_ON_ERR");
        return;
    }

    console_write_line(context, "OLED_UI_DISPLAY_ON_OK");

    if (proof_ok != 0)
    {
        console_write_line(context, "OLED_UI_UPDATE_OK");
    }
    else
    {
        console_write_line(context, "OLED_UI_UPDATE_ERR");
    }
}

static void console_oled_console(command_service_context_t *context)
{
    if (ssd1306_show_console_test() != 0)
    {
        console_write_line(context, "OLED_CONSOLE_OK");
    }
    else
    {
        console_write_line(context, "OLED_CONSOLE_ERR");
    }
}

static int ssd1306_show_status_test(void)
{
    return ssd1306_show_ui_layout(
        oled_ui_layout_default());
}

static void console_oled_status(command_service_context_t *context)
{
    if (oled_ui_layout_self_test() == 0)
    {
        console_write_line(context, "OLED_UI_LAYOUT_ERR");
        console_write_line(context, "OLED_STATUS_ERR");
        return;
    }

    console_write_line(context, "OLED_UI_LAYOUT_OK");

    if (oled_status_bar_self_test() == 0)
    {
        console_write_line(context, "OLED_STATUS_REFERENCE_ERR");
        console_write_line(context, "OLED_STATUS_ERR");
        return;
    }

    console_write_line(context, "OLED_STATUS_REFERENCE_OK");

    if (ssd1306_show_status_test() != 0)
    {
        console_write_line(context, "OLED_STATUS_OK");
    }
    else
    {
        console_write_line(context, "OLED_STATUS_ERR");
    }
}







static void console_oled_test(command_service_context_t *context)
{
    if (ssd1306_show_checkerboard() != 0)
    {
        console_write_line(context, "OLED_TEST_OK");
    }
    else
    {
        console_write_line(context, "OLED_TEST_ERR");
    }
}

static void console_oled_ping(command_service_context_t *context)
{
    if (ssd1306_ping() != 0)
    {
        console_write_line(context, "OLED_CMD_OK");
    }
    else
    {
        console_write_line(context, "OLED_CMD_ERR");
    }
}

static void console_i2c_scan(command_service_context_t *context)
{
    uint32_t address;
    uint32_t count = 0u;

    console_write_line(context, "I2C_SCAN");

    for (address = 0x08u; address <= 0x77u; ++address)
    {
        int result = i2c1_probe((uint8_t)address);

        if (result < 0)
        {
            console_write_line(context, "I2C_BUS_ERR");
            return;
        }

        if (result != 0)
        {
            console_write(context, "ADDR=");
            console_write_hex32(context, address);
            console_write(context, "\r\n");
            ++count;
        }
    }

    console_write(context, "COUNT=");
    console_write_hex32(context, count);
    console_write(context, "\r\n");
}

static void console_write_fault(command_service_context_t *context)
{
    console_write(context, "FAULTREC=");
    console_write_hex32(context, (uint32_t)&fault_record);
    console_write(context, "\r\n");

    console_write(context, "MAGIC=");
    console_write_hex32(context, fault_record.magic);
    console_write(context, " EXC=");
    console_write_hex32(context, fault_record.exception_number);
    console_write(context, "\r\n");

    console_write(context, "SP=");
    console_write_hex32(context, fault_record.stacked_sp);
    console_write(context, " VALID=");
    console_write_hex32(context, fault_record.stack_valid);
    console_write(context, "\r\n");

    console_write(context, "PC=");
    console_write_hex32(context, fault_record.pc);
    console_write(context, " LR=");
    console_write_hex32(context, fault_record.lr);
    console_write(context, "\r\n");

    console_write(context, "CFSR=");
    console_write_hex32(context, SCB_CFSR);
    console_write(context, " HFSR=");
    console_write_hex32(context, SCB_HFSR);
    console_write(context, " SHCSR=");
    console_write_hex32(context, SCB_SHCSR);
    console_write(context, "\r\n");
}

static void console_uart_rx_stats(command_service_context_t *context)
{
    const uint32_t head = uart_rx_head;
    const uint32_t tail = uart_rx_tail;
    const uint32_t depth = head - tail;
    const uint32_t irq_count = uart_rx_irq_count;
    const uint32_t byte_count = uart_rx_byte_count;
    const uint32_t drop_count = uart_rx_drop_count;
    const uint32_t error_count = uart_rx_error_count;
    const uint32_t high_water = uart_rx_high_water;

    console_write(context, "RX_CAPACITY=");
    console_write_hex32(context, UART_RX_RING_CAPACITY);
    console_write(context, "\r\n");

    console_write(context, "RX_IRQ_COUNT=");
    console_write_hex32(context, irq_count);
    console_write(context, "\r\n");

    console_write(context, "RX_BYTE_COUNT=");
    console_write_hex32(context, byte_count);
    console_write(context, "\r\n");

    console_write(context, "RX_DROP_COUNT=");
    console_write_hex32(context, drop_count);
    console_write(context, "\r\n");

    console_write(context, "RX_ERROR_COUNT=");
    console_write_hex32(context, error_count);
    console_write(context, "\r\n");

    console_write(context, "RX_HIGH_WATER=");
    console_write_hex32(context, high_water);
    console_write(context, "\r\n");

    console_write(context, "RX_DEPTH=");
    console_write_hex32(context, depth);
    console_write(context, "\r\n");

    if (
        (irq_count != 0u) &&
        (byte_count != 0u) &&
        (drop_count == 0u) &&
        (error_count == 0u) &&
        (high_water != 0u) &&
        (high_water <= UART_RX_RING_CAPACITY) &&
        (depth <= UART_RX_RING_CAPACITY))
    {
        console_write_line(context, "RX_IRQ_RING_OK");
    }
    else
    {
        console_write_line(context, "RX_IRQ_RING_ERR");
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

static void console_msp_stack_stats(command_service_context_t *context)
{
    const uint32_t reserved = msp_stack_reserved_bytes();
    const uint32_t capacity = msp_stack_capacity_bytes();
    const uint32_t used = msp_stack_high_water_bytes();
    const uint32_t current = msp_stack_current_used_bytes();
    const uint32_t margin = (used < capacity) ? (capacity - used) : 0u;
    const uint32_t canary = (msp_stack_canary_intact() != 0) ? 1u : 0u;

    console_write(context, "MSP_RESERVED=");
    console_write_hex32(context, reserved);
    console_write(context, "\r\n");

    console_write(context, "MSP_GUARD=");
    console_write_hex32(context, MSP_STACK_GUARD_BYTES);
    console_write(context, "\r\n");

    console_write(context, "MSP_CAPACITY=");
    console_write_hex32(context, capacity);
    console_write(context, "\r\n");

    console_write(context, "MSP_USED=");
    console_write_hex32(context, used);
    console_write(context, "\r\n");

    console_write(context, "MSP_MARGIN=");
    console_write_hex32(context, margin);
    console_write(context, "\r\n");

    console_write(context, "MSP_CURRENT=");
    console_write_hex32(context, current);
    console_write(context, "\r\n");

    console_write(context, "MSP_CANARY=");
    console_write_hex32(context, canary);
    console_write(context, "\r\n");

    if (
        (reserved == MSP_STACK_RESERVED_BYTES) &&
        (capacity == MSP_STACK_CAPACITY_BYTES) &&
        (used > 0u) &&
        (used < capacity) &&
        (current > 0u) &&
        (current < capacity) &&
        (canary != 0u))
    {
        console_write_line(context, "MSP_STACK_OK");
    }
    else
    {
        console_write_line(context, "MSP_STACK_ERR");
    }
}

static void console_usb_cdc_stats(command_service_context_t *context)
{
    console_write(context, "USB_CDC_CONFIGURED=");
    console_write_hex32(context, usb_device_diagnostics.cdc_configured);
    console_write(context, " CONFIGURATION=");
    console_write_hex32(context, usb_device_diagnostics.configuration);
    console_write(context, " RX_PACKETS=");
    console_write_hex32(context, usb_device_diagnostics.cdc_rx_packet_count);
    console_write(context, " RX_BYTES=");
    console_write_hex32(context, usb_device_diagnostics.cdc_rx_byte_count);
    console_write(context, " RX_DROPS=");
    console_write_hex32(context, usb_device_diagnostics.cdc_rx_drop_count);
    console_write(context, " RX_HIGH_WATER=");
    console_write_hex32(context, usb_device_diagnostics.cdc_rx_high_water);
    console_write(context, " TX_PACKETS=");
    console_write_hex32(context, usb_device_diagnostics.cdc_tx_packet_count);
    console_write(context, " TX_BYTES=");
    console_write_hex32(context, usb_device_diagnostics.cdc_tx_byte_count);
    console_write(context, " TX_DROPS=");
    console_write_hex32(context, usb_device_diagnostics.cdc_tx_drop_count);
    console_write(context, " TX_HIGH_WATER=");
    console_write_hex32(context, usb_device_diagnostics.cdc_tx_high_water);
    console_write(context, " CONTROL=");
    console_write_hex32(context, usb_device_diagnostics.cdc_control_line_state);
    console_write(context, " BAUD=");
    console_write_hex32(context, usb_device_diagnostics.cdc_line_coding_baud);
    console_write(context, " STOP=");
    console_write_hex32(context, usb_device_diagnostics.cdc_line_coding_stop_bits);
    console_write(context, " PARITY=");
    console_write_hex32(context, usb_device_diagnostics.cdc_line_coding_parity);
    console_write(context, " DATA_BITS=");
    console_write_hex32(context, usb_device_diagnostics.cdc_line_coding_data_bits);
    console_write(context, "\r\n");
}

static void console_write_command_descriptor(
    command_service_context_t *context,
    const command_service_descriptor_t *descriptor)
{
    if (descriptor == (const command_service_descriptor_t *)0)
    {
        return;
    }

    console_write(context, "HELP_METHOD=");
    console_write(context, descriptor->name);
    console_write(context, " CLASS=");
    console_write(context, command_service_class_name(descriptor->command_class));
    console_write(context, " MIN_ARGS=");
    console_write_hex32(context, descriptor->min_args);
    console_write(context, " MAX_ARGS=");
    console_write_hex32(context, descriptor->max_args);
    console_write(context, "\r\n");
}

static command_service_status_t console_command_help(
    const command_service_request_t *request,
    command_service_context_t *context)
{
    if (request->argc == 0u)
    {
        const uint32_t count = command_service_registry_count();

        console_write(context, "HELP_COUNT=");
        console_write_hex32(context, count);
        console_write(context, "\r\nHELP_METHODS=");

        for (uint32_t index = 0u; index < count; ++index)
        {
            const command_service_descriptor_t *descriptor =
                command_service_registry_at(index);

            if (descriptor == (const command_service_descriptor_t *)0)
            {
                return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
            }

            if (index != 0u)
            {
                console_write(context, " ");
            }

            console_write(context, descriptor->name);
        }

        console_write(context, "\r\n");
        return COMMAND_SERVICE_STATUS_OK;
    }

    if (request->argc == 1u)
    {
        const command_service_descriptor_t *descriptor =
            command_service_find(request->argv[0]);

        if (descriptor == (const command_service_descriptor_t *)0)
        {
            return COMMAND_SERVICE_STATUS_NOT_FOUND;
        }

        console_write_command_descriptor(context, descriptor);
        return COMMAND_SERVICE_STATUS_OK;
    }

    return COMMAND_SERVICE_STATUS_BAD_ARGS;
}

static command_service_status_t console_execute_application_method(
    const command_service_request_t *request,
    command_service_context_t *context)
{
    application_service_snapshot_t services;
    const application_service_snapshot_t *services_ptr =
        (const application_service_snapshot_t *)0;

    if (
        (request->descriptor->method_id == COMMAND_SERVICE_METHOD_APPSTART) ||
        (request->descriptor->method_id == COMMAND_SERVICE_METHOD_APPSTOP)
    ) {
        if (boot_desktop_ui_application_snapshot_current(&services) != 0)
        {
            services_ptr = &services;
        }
    }

    return application_commands_execute(
        request,
        context,
        services_ptr);
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

static void console_production_scheduler_stats(command_service_context_t *context)
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

    console_write(context, "SCHED_PROD_ACTIVE=");
    console_write_hex32(context, active);
    console_write(context, "\r\n");

    console_write_line(context, "SCHED_PROD_MODE=COOPERATIVE");

    console_write(context, "SCHED_PROD_THREAD_PSP=");
    console_write_hex32(context, thread_psp);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_CONTROL=");
    console_write_hex32(context, control);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_PSP=");
    console_write_hex32(context, psp);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_PSP_IN_RANGE=");
    console_write_hex32(context, psp_in_range);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK0_STATE=");
    console_write_hex32(context, task0_state);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK0_PRIORITY=");
    console_write_hex32(context, task0_priority);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK0_WAIT_EVENTS=");
    console_write_hex32(context, task0_wait_events);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK0_WAKE_EVENTS=");
    console_write_hex32(context, task0_wake_events);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK1_STATE=");
    console_write_hex32(context, task1_state);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_TASK1_PRIORITY=");
    console_write_hex32(context, task1_priority);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_CONSOLE_CAPACITY=");
    console_write_hex32(context, console_capacity);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_CONSOLE_USED=");
    console_write_hex32(context, console_used);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_CONSOLE_MARGIN=");
    console_write_hex32(context, console_margin);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_CONSOLE_CANARY=");
    console_write_hex32(context, console_canary);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_STARTED=");
    console_write_hex32(context, production_heartbeat_task_started);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_COUNT=");
    console_write_hex32(context, production_heartbeat_count);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_LED_ON=");
    console_write_hex32(context, production_heartbeat_led_on);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_FAULT=");
    console_write_hex32(context, production_heartbeat_fault);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_CAPACITY=");
    console_write_hex32(context, heartbeat_capacity);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_USED=");
    console_write_hex32(context, heartbeat_used);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_MARGIN=");
    console_write_hex32(context, heartbeat_margin);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_HEARTBEAT_CANARY=");
    console_write_hex32(context, heartbeat_canary);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_IDLE_WAITS=");
    console_write_hex32(context, idle_waits);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_WAIT_COUNT=");
    console_write_hex32(context, production_console_wait_count);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_WAKE_COUNT=");
    console_write_hex32(context, production_console_wake_count);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_WAKE_EVENTS=");
    console_write_hex32(context, production_console_wake_events);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_COMMAND_COUNT=");
    console_write_hex32(context, production_console_command_count);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_DIAG_BUSY_COUNT=");
    console_write_hex32(context, scheduler_diagnostics_busy_count());
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_PREEMPT_SWITCHES=");
    console_write_hex32(context, preempt_switches);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PROD_FAULT=");
    console_write_hex32(context, production_console_fault);
    console_write(context, "\r\n");

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
        console_write_line(context, "SCHED_PROD_OK");
    }
    else
    {
        console_write_line(context, "SCHED_PROD_ERR");
    }
}

static command_service_status_t console_execute_safe_method(
    const command_service_request_t *request,
    command_service_context_t *context)
{
    switch (request->descriptor->method_id)
    {
        case COMMAND_SERVICE_METHOD_RPCINFO:
        case COMMAND_SERVICE_METHOD_APPLIST:
        case COMMAND_SERVICE_METHOD_APPSTART:
        case COMMAND_SERVICE_METHOD_APPSTOP:
            return console_execute_application_method(request, context);

        case COMMAND_SERVICE_METHOD_SYSINFO:
            return system_identity_write(context);

        case COMMAND_SERVICE_METHOD_SCHEDPROD:
            console_production_scheduler_stats(context);
            return COMMAND_SERVICE_STATUS_OK;

        case COMMAND_SERVICE_METHOD_SCHEDTIMED:
            return scheduler_diagnostics_execute_timed(context);

        case COMMAND_SERVICE_METHOD_SCHEDPRIO:
            return scheduler_diagnostics_execute_priority(context);

        case COMMAND_SERVICE_METHOD_PING:
            console_write_line(context, "PONG");
            break;

        case COMMAND_SERVICE_METHOD_UPTIME:
            console_write(context, "UPTIME_MS=");
            console_write_hex32(context, kernel_time_now());
            console_write(context, "\r\n");
            break;

        case COMMAND_SERVICE_METHOD_HEALTH:
            console_write(context, "HEALTH TICK=");
            console_write_hex32(context, kernel_time_now());
            console_write(context, " PC13=");
            console_write_hex32(context, (GPIOC_ODR & GPIO_PIN_13) != 0u ? 1u : 0u);
            console_write(context, " WDOG_ACTIVE=");
            console_write_hex32(context, production_watchdog_active);
            console_write(context, " WDOG_RELOAD_COUNT=");
            console_write_hex32(context, production_watchdog_reload_count);
            console_write(context, " RESET_FLAGS=");
            console_write_hex32(context, production_reset_flags);
            console_write(context, " IWDG_RESET=");
            console_write_hex32(context, production_iwdg_reset);
            console_write(context, "\r\n");
            break;

        case COMMAND_SERVICE_METHOD_RXSTAT:
            console_uart_rx_stats(context);
            break;

        case COMMAND_SERVICE_METHOD_CDCSTAT:
            console_usb_cdc_stats(context);
            break;

        case COMMAND_SERVICE_METHOD_MSPSTAT:
            console_msp_stack_stats(context);
            break;

        case COMMAND_SERVICE_METHOD_FAULT:
            console_write_fault(context);
            break;

        case COMMAND_SERVICE_METHOD_I2CSCAN:
            console_i2c_scan(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDPING:
            console_oled_ping(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDTEST:
            console_oled_test(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDTEXT:
            console_oled_text(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDRENDER:
            console_oled_render(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDCONSOLE:
            console_oled_console(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDSCROLL:
            console_oled_scroll(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDDIRTY:
            console_oled_dirty(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDUIUPDATE:
            console_oled_ui_update(context);
            break;

        case COMMAND_SERVICE_METHOD_UIRUNTIME:
            console_oled_runtime(context);
            break;

        case COMMAND_SERVICE_METHOD_OLEDSTATUS:
            console_oled_status(context);
            break;

        case COMMAND_SERVICE_METHOD_HELP:
            return console_command_help(request, context);

        default:
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return COMMAND_SERVICE_STATUS_OK;
}

static command_service_status_t console_execute_scheduler_diagnostic(
    const command_service_request_t *request,
    command_service_context_t *context)
{
    static const scheduler_diagnostics_bindings_t bindings =
    {
        console_execute_request,
        oled_runtime_ui_show,
        uart_try_getc,
        production_console_stack,
        PRODUCTION_CONSOLE_STACK_WORDS,
        PRODUCTION_CONSOLE_MIN_MARGIN_BYTES,
        PRODUCTION_UART_RX_EVENT
    };

    return scheduler_diagnostics_execute_diagnostic(
        request,
        context,
        &bindings);
}

static void console_watchdog_trip(command_service_context_t *context)
{
    if (production_watchdog_active == 0u)
    {
        console_write_line(context, "WDOG_TRIP_ERR_INACTIVE");
        return;
    }

    if (command_service_write_line(context, "WDOG_TRIP_ARMED") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return;
    }

    for (;;)
    {
        __asm volatile ("nop");
    }
}

static command_service_status_t console_execute_request(
    const command_service_request_t *request,
    command_service_context_t *context,
    void *handler_context)
{
    (void)handler_context;
    if (
        (request == (const command_service_request_t *)0) ||
        (request->descriptor == (const command_service_descriptor_t *)0)
    ) {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    switch (request->descriptor->command_class)
    {
        case COMMAND_SERVICE_CLASS_SAFE:
            return console_execute_safe_method(request, context);

        case COMMAND_SERVICE_CLASS_DIAGNOSTIC:
            return console_execute_scheduler_diagnostic(request, context);

        case COMMAND_SERVICE_CLASS_DESTRUCTIVE:
            if (request->descriptor->method_id != COMMAND_SERVICE_METHOD_WDOGTRIP)
            {
                return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
            }

            console_watchdog_trip(context);
            return COMMAND_SERVICE_STATUS_OK;

        default:
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
}

static void console_execute_state(console_command_state_t *state)
{
    command_service_request_t request;
    command_service_status_t status;

    if ((state == (console_command_state_t *)0) || (state->length == 0u))
    {
        return;
    }

    state->data[state->length] = '\0';
    state->context.write_failed = 0u;

    if (production_console_task_started != 0u)
    {
        ++production_console_command_count;
    }

    status = command_service_parse_line(state->data, &request);
    if (status == COMMAND_SERVICE_STATUS_OK)
    {
        status =
            command_service_execute(
                &request,
                &state->context,
                console_execute_request,
                (void *)0);
    }

    if (
        (status == COMMAND_SERVICE_STATUS_NOT_FOUND) ||
        (status == COMMAND_SERVICE_STATUS_BAD_ARGS) ||
        ((status == COMMAND_SERVICE_STATUS_INTERNAL_ERROR) &&
         (state->context.write_failed == 0u))
    ) {
        (void)command_service_write_line(&state->context, "ERR");
    }

    state->length = 0u;
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

    if (state->length < (COMMAND_SERVICE_LINE_CAPACITY - 1u))
    {
        state->data[state->length] = c;
        ++state->length;
    }
    else
    {
        state->length = 0u;
        state->context.write_failed = 0u;
        (void)command_service_write_line(&state->context, "ERR");
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
        binary_frame_view_t frame;
        const binary_frame_feed_result_t feed_result =
            binary_frame_parser_feed(
                &usb_cdc_binary_parser,
                (uint8_t)c,
                &frame);

        if ((feed_result == BINARY_FRAME_FEED_TEXT) ||
            (feed_result == BINARY_FRAME_FEED_SYNC_ERROR_TEXT))
        {
            console_feed_char(&usb_cdc_command_state, c);
        }
        else if (feed_result == BINARY_FRAME_FEED_FRAME_READY)
        {
            if ((frame.frame_type == BINARY_FRAME_TYPE_RPC_REQUEST) &&
                (production_console_task_started != 0u))
            {
                ++production_console_command_count;
            }

            (void)binary_rpc_handle_frame(
                &usb_cdc_binary_rpc_state,
                &frame,
                &usb_cdc_binary_rpc_binding);
        }
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

    if (application_runtime_bridge_initialize() == 0)
    {
        production_console_fault = 1u;
        return;
    }

    uart_emergency_write_line("SCHED_PROD_CONSOLE_ONLINE");

    for (;;)
    {
        console_drain_uart_rx();
        console_drain_usb_cdc_rx();
        production_console_command_count +=
            usb_management_runtime_service(
                &usb_management_binary_rpc_binding);
        production_watchdog_reload();
        boot_desktop_ui_service();

        ++production_console_wait_count;

        events =
            scheduler_wait_events_timeout(
                PRODUCTION_CONSOLE_RX_EVENTS,
                BOOT_DESKTOP_UI_POLL_MS);

        production_console_wake_events |= events;
        ++production_console_wake_count;

        if ((events & ~PRODUCTION_CONSOLE_RX_EVENTS) != 0u)
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
    uart_emergency_write_line(reason);
    uart_emergency_write_line("SCHED_PROD_FATAL");

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
    application_runtime_bridge_reset();
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
        uart_emergency_write_line("OLED_RUNTIME_UI_OK");
    }
    else
    {
        uart_emergency_write_line("OLED_RUNTIME_UI_ERR");
    }

    /*
     * Normal boot transfers application runtime ownership to two cooperative
     * PSP tasks with disjoint responsibilities.
     *
     * Task 0 owns the production console/runtime path. USART1 IRQ remains the
     * sole USART DR reader, while the USB device IRQ owns CDC and
     * management endpoint/PMA service. IRQ paths publish bytes to independent
     * bounded RX rings before signalling transport-specific scheduler events.
     * Task 0 drains all authoritative transport rings before each combined
     * event wait; management parsing/RPC execution therefore remains Thread/PSP.
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
    usb_management_set_rx_notify(usb_management_rx_event_notify);

    uart_command_state.length = 0u;
    uart_command_state.context.write_failed = 0u;
    usb_cdc_command_state.length = 0u;
    usb_cdc_command_state.context.write_failed = 0u;
    binary_frame_parser_init(&usb_cdc_binary_parser);
    binary_rpc_init(
        &usb_cdc_binary_rpc_state,
        &production_binary_rpc_workspace);

    if (usb_management_runtime_init(
            &production_binary_rpc_workspace) == 0)
    {
        production_fail_closed(
            "USB_MANAGEMENT_RUNTIME_INIT_ERR");
    }

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
    uart_emergency_write_line("IWDG_ACTIVE");
    uart_emergency_write_line("SCHED_PROD_PREPARE_OK");
    uart_emergency_write_line("SCHED_PROD_START");

    start_result = scheduler_start();

    uart_emergency_write("SCHED_PROD_RETURN=");
    uart_emergency_write_hex32((uint32_t)start_result);
    uart_emergency_write("\r\n");

    production_fail_closed(
        "SCHED_PROD_UNEXPECTED_RETURN");
}
