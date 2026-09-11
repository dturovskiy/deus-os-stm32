#include <stdint.h>
#include "kernel/time.h"
#include "drivers/ssd1306.h"
#include "gfx/mono_fb.h"
#include "gfx/font5x7.h"
#include "gfx/text_renderer.h"

#define REG32(addr) (*(volatile uint32_t *)(addr))

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

#define USART_SR_RXNE   (1u << 5)
#define USART_SR_TXE    (1u << 7)
#define USART_CR1_RE    (1u << 2)
#define USART_CR1_TE    (1u << 3)
#define USART_CR1_UE    (1u << 13)
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

static uint8_t oled_framebuffer[SSD1306_FRAMEBUFFER_BYTES];
static mono_fb_t oled_surface;

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
    if (ssd1306_show_generic_renderer_test() != 0)
    {
        uart_write_line("OLED_RENDER_OK");
    }
    else
    {
        uart_write_line("OLED_RENDER_ERR");
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

static void console_execute(void)
{
    uart_command[uart_command_length] = '\0';

    if (text_equals(uart_command, "ping") != 0)
    {
        uart_write_line("PONG");
    }
    else if (text_equals(uart_command, "uptime") != 0)
    {
        uart_write("UPTIME_MS=");
        uart_write_hex32(kernel_time_now());
        uart_write("\r\n");
    }
    else if (text_equals(uart_command, "health") != 0)
    {
        uart_write("HEALTH TICK=");
        uart_write_hex32(kernel_time_now());
        uart_write(" PC13=");
        uart_write_hex32((GPIOC_ODR & GPIO_PIN_13) != 0u ? 1u : 0u);
        uart_write("\r\n");
    }
    else if (text_equals(uart_command, "fault") != 0)
    {
        console_write_fault();
    }
    else if (text_equals(uart_command, "i2cscan") != 0)
    {
        console_i2c_scan();
    }
    else if (text_equals(uart_command, "oledping") != 0)
    {
        console_oled_ping();
    }
    else if (text_equals(uart_command, "oledtest") != 0)
    {
        console_oled_test();
    }
    else if (text_equals(uart_command, "oledtext") != 0)
    {
        console_oled_text();
    }
    else if (text_equals(uart_command, "oledrender") != 0)
    {
        console_oled_render();
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
    i2c1_init();
    mono_fb_init(
        &oled_surface,
        oled_framebuffer,
        SSD1306_WIDTH,
        SSD1306_HEIGHT);
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
