/* Host-only MMIO register mock for actual, unlinked STM32 PB0/PB1 driver. */
#include <stdint.h>
#include <stdio.h>
#include "drivers/esp_gpio_board_v0.h"
#define CHECK(x) do {if(!(x)){fprintf(stderr,"FAIL:%u:%s\n",(unsigned)__LINE__,#x);return 1;}}while(0)
volatile uint32_t esp_gpio_mock_rcc_apb2enr;
volatile uint32_t esp_gpio_mock_gpio_b_crl;
volatile uint32_t esp_gpio_mock_gpio_b_bsrr;
int main(void)
{
    esp_gpio_board_v0_t b;
    esp_gpio_board_v0_reset(&b);
    esp_gpio_mock_rcc_apb2enr=0x10000000u;
    esp_gpio_mock_gpio_b_crl=0x1234ABCDu;
    esp_gpio_mock_gpio_b_bsrr=0x87650000u;
    CHECK(!esp_gpio_board_v0_init(&b,0u));
    CHECK(!esp_gpio_board_v0_init(&b,2u));
    CHECK(!esp_gpio_board_v0_init(0,1u));
    CHECK(!esp_gpio_board_v0_en_pull_low(&b,1u));
    CHECK(!esp_gpio_board_v0_gpio0_pull_low(&b,1u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==0x87650000u);
    CHECK(esp_gpio_mock_gpio_b_crl==0x1234ABCDu);
    CHECK(esp_gpio_mock_rcc_apb2enr==0x10000000u);
    CHECK(esp_gpio_board_v0_init(&b,1u));
    CHECK(b.qualified==1u);
    CHECK(esp_gpio_mock_rcc_apb2enr==(0x10000000u|8u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==3u);
    CHECK(esp_gpio_mock_gpio_b_crl==0x1234AB66u);
    CHECK(!esp_gpio_board_v0_init(&b,1u)); /* no re-init after active */
    CHECK(esp_gpio_board_v0_gpio0_pull_low(&b,1u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==(1u<<16));
    CHECK(esp_gpio_board_v0_en_pull_low(&b,1u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==(1u<<17));
    CHECK(esp_gpio_board_v0_gpio0_pull_low(&b,0u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==1u);
    CHECK(esp_gpio_board_v0_en_pull_low(&b,0u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==2u);
    CHECK(!esp_gpio_board_v0_en_pull_low(&b,2u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==2u);
    esp_gpio_board_v0_reset(&b);
    CHECK(!esp_gpio_board_v0_en_pull_low(&b,1u));
    CHECK(!esp_gpio_board_v0_gpio0_pull_low(&b,1u));
    CHECK(esp_gpio_mock_gpio_b_bsrr==2u);
    puts("ESP_GPIO_BOARD_V0=PASS PB0_PB1_OD_NO_WRITE_BEFORE_OWNER_QUALIFIED");
    return 0;
}
