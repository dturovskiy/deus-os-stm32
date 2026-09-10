#include <stdint.h>
#include "kernel/time.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* Flash interface */
#define FLASH_ACR       REG32(0x40022000u)
#define FLASH_LATENCY_2 0x2u
#define FLASH_PRFTBE     (1u << 4)

/* Reset and clock control */
#define RCC_CR          REG32(0x40021000u)
#define RCC_CFGR        REG32(0x40021004u)
#define RCC_APB2ENR     REG32(0x40021018u)

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

#define RCC_IOPAEN      (1u << 2)
#define RCC_IOPCEN      (1u << 4)
#define RCC_USART1EN    (1u << 14)

/* GPIOA / GPIOC */
#define GPIOA_CRH       REG32(0x40010804u)

#define GPIOC_CRH       REG32(0x40011004u)
#define GPIOC_BSRR      REG32(0x40011010u)

#define GPIO_PIN_13     (1u << 13)
#define GPIO_RESET_13   (1u << 29)

/* USART1 */
#define USART1_SR       REG32(0x40013800u)
#define USART1_DR       REG32(0x40013804u)
#define USART1_BRR      REG32(0x40013808u)
#define USART1_CR1      REG32(0x4001380Cu)

#define USART_SR_RXNE   (1u << 5)
#define USART_SR_TXE    (1u << 7)
#define USART_CR1_RE    (1u << 2)
#define USART_CR1_TE    (1u << 3)
#define USART_CR1_UE    (1u << 13)

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

    USART1_BRR = USART1_BRR_115200;
    USART1_CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

static void uart_putc(char c)
{
    while ((USART1_SR & USART_SR_TXE) == 0u)
    {
    }

    USART1_DR = (uint32_t)(uint8_t)c;
}

static int uart_try_getc(char *c)
{
    if ((USART1_SR & USART_SR_RXNE) == 0u)
    {
        return 0;
    }

    *c = (char)(uint8_t)USART1_DR;
    return 1;
}

static void uart_write(const char *text)
{
    while (*text != '\0')
    {
        uart_putc(*text);
        ++text;
    }
}

static void uart_write_hex32(uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    uart_write("0x");

    for (uint32_t shift = 28u;; shift -= 4u)
    {
        uart_putc(hex[(value >> shift) & 0xFu]);

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

#define UART_COMMAND_CAPACITY 16u

static char uart_command[UART_COMMAND_CAPACITY];
static uint32_t uart_command_length;

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

static void console_execute(void)
{
    uart_command[uart_command_length] = '\0';

    if (text_equals(uart_command, "ping") != 0)
    {
        uart_write_line("PONG");
    }
    else
    {
        uart_write_line("ERR");
    }

    uart_command_length = 0u;
}

static void console_poll(void)
{
    char c;

    while (uart_try_getc(&c) != 0)
    {
        if ((c == '\r') || (c == '\n'))
        {
            if (uart_command_length != 0u)
            {
                console_execute();
            }

            continue;
        }

        if ((c == '\b') || ((uint8_t)c == 0x7Fu))
        {
            if (uart_command_length != 0u)
            {
                --uart_command_length;
            }

            continue;
        }

        if (uart_command_length < (UART_COMMAND_CAPACITY - 1u))
        {
            uart_command[uart_command_length] = c;
            ++uart_command_length;
        }
        else
        {
            uart_command_length = 0u;
            uart_write_line("ERR");
        }
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
    static uint32_t led_ticks;
    static uint32_t led_on;

    ++kernel_ticks;
    ++led_ticks;

    if (led_ticks >= 500u)
    {
        led_ticks = 0u;
        led_on ^= 1u;

        if (led_on != 0u)
        {
            GPIOC_BSRR = GPIO_RESET_13;
        }
        else
        {
            GPIOC_BSRR = GPIO_PIN_13;
        }
    }
}

void kernel_main(void)
{
    const uint32_t core_clock_hz = clock_init();

    gpio_init();
    uart_init();
    faults_init();
    systick_init(core_clock_hz);

    uart_boot_banner(core_clock_hz);

    /*
     * Poll RX continuously for this first RX milestone.
     * A later USART1 RX interrupt/ring-buffer slice can restore WFI idle
     * without risking UART overrun at 115200 baud.
     */
    for (;;)
    {
        console_poll();
    }
}
