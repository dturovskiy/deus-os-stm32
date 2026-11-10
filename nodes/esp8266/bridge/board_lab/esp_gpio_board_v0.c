/* Isolated STM32F103 GPIOB PB0/PB1 open-drain output adapter.
 * SOURCE ONLY, NOT IN BUILD LIST, NOT INSTALLED ON DEVICE.
 * Requires verified external 3.3V pull-ups and no CH340 control conflicts.
 * HOST MMIO MOCK uses substitute volatile 32-bit register variables.
 */
#include "drivers/esp_gpio_board_v0.h"

#ifdef DEUS_ESP_GPIO_MMIO_MOCK
extern volatile uint32_t esp_gpio_mock_rcc_apb2enr;
extern volatile uint32_t esp_gpio_mock_gpio_b_crl;
extern volatile uint32_t esp_gpio_mock_gpio_b_bsrr;
#define RCC_APB2ENR esp_gpio_mock_rcc_apb2enr
#define GPIOB_CRL esp_gpio_mock_gpio_b_crl
#define GPIOB_BSRR esp_gpio_mock_gpio_b_bsrr
#else
#define REG32(a) (*(volatile uint32_t *)(a))
#define RCC_APB2ENR REG32(0x40021018u)
#define GPIOB_CRL REG32(0x40010C00u)
#define GPIOB_BSRR REG32(0x40010C10u)
#endif

#define RCC_IOPBEN (1u << 3)
#define ESP_PB0_BIT 0u
#define ESP_PB1_BIT 1u
/* CRL: MODE[1:0]=10 (2MHz), CNF[1:0]=01 (open-drain). */
#define GPIO_OD_2MHZ 0x6u

void esp_gpio_board_v0_reset(esp_gpio_board_v0_t *b)
{
    if (b != (esp_gpio_board_v0_t *)0)
        b->qualified=0u;
}
int esp_gpio_board_v0_init(esp_gpio_board_v0_t *b,
                           uint8_t owner_qualified)
{
    uint32_t crl;
    if (b == (esp_gpio_board_v0_t *)0 ||
        owner_qualified != 1u || b->qualified != 0u)
        return 0;
    RCC_APB2ENR |= RCC_IOPBEN;
    /* Set ODR high BEFORE GPIO output configuration; on release
     * external pull-ups hold HIGH, but STM32 will not source ESP VCC. */
    GPIOB_BSRR = (1u << ESP_PB0_BIT) | (1u << ESP_PB1_BIT);
    crl=GPIOB_CRL;
    crl &= ~0xFFu; /* only PB0 and PB1 */
    crl |= GPIO_OD_2MHZ | (GPIO_OD_2MHZ << 4);
    GPIOB_CRL=crl;
    b->qualified=1u;
    return 1;
}
static int set_open_drain(esp_gpio_board_v0_t *b,
                          uint32_t pin, uint8_t pull_low)
{
    if (b == (esp_gpio_board_v0_t *)0 ||
        b->qualified != 1u || pull_low > 1u)
        return 0;
    /* BSRR upper half pulls ODR low, lower half releases it. */
    GPIOB_BSRR = pull_low != 0u ? (1u << (pin + 16u)) : (1u << pin);
    return 1;
}
int esp_gpio_board_v0_gpio0_pull_low(esp_gpio_board_v0_t *b,
                                     uint8_t pull_low)
{
    return set_open_drain(b,ESP_PB0_BIT,pull_low);
}
int esp_gpio_board_v0_en_pull_low(esp_gpio_board_v0_t *b,
                                  uint8_t pull_low)
{
    return set_open_drain(b,ESP_PB1_BIT,pull_low);
}
