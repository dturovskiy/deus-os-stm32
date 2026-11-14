/* Product-source STM32-owned ESP ROM bridge: fake registers only. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/esp_stm32_owner_v1.h"
#include "kernel/node_transport.h"

#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"FAIL line=%d: %s\n", __LINE__, #x); exit(2); } } while(0)
volatile uint32_t esp_gpio_mock_rcc_apb2enr;
volatile uint32_t esp_gpio_mock_gpio_b_crl;
volatile uint32_t esp_gpio_mock_gpio_b_bsrr;
static uint8_t pending;

void node_transport_diagnostics_get(node_transport_diagnostics_t *out)
{
    memset(out, 0, sizeof(*out));
    out->pending = pending;
}
int main(void)
{
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_enter(10u) == 0);
    esp_stm32_owner_v1_init(100u);
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    REQUIRE((esp_gpio_mock_rcc_apb2enr & 8u) != 0u);
    REQUIRE((esp_gpio_mock_gpio_b_crl & 0xffu) == 0x66u);
    REQUIRE(esp_stm32_owner_v1_can_enter() != 0);
    pending = 1u;
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_enter(101u) == 0);
    pending = 0u;
    /* Successful recovery to ESP ROM does NOT depend on app HELLO. */
    REQUIRE(esp_stm32_owner_v1_enter(110u) != 0);
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == ((uint32_t)1u << 16));
    esp_stm32_owner_v1_service(129u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    esp_stm32_owner_v1_service(130u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    esp_stm32_owner_v1_service(230u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() != 0);
    /* Clean explicit CLOSE releases GPIO0 before EN normal reboot. */
    esp_stm32_owner_v1_leave(0u, 240u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 1u);
    esp_stm32_owner_v1_service(260u);
    esp_stm32_owner_v1_service(360u);
    REQUIRE(esp_stm32_owner_v1_can_enter() != 0);
    /* Unknown USB/ROM outcome freezes ESP via EN LOW, no auto resume. */
    REQUIRE(esp_stm32_owner_v1_enter(400u) != 0);
    esp_stm32_owner_v1_service(420u);
    esp_stm32_owner_v1_service(520u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() != 0);
    esp_stm32_owner_v1_leave(1u, 530u);
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    puts("STM32_OWNER_VERIFIED_BOARD_MOCK=PASS ROM_RECOVERY_NO_APP_HELLO=PASS ABORT_EN_LOW=PASS");
#else
    REQUIRE(esp_gpio_mock_rcc_apb2enr == 0u);
    REQUIRE(esp_gpio_mock_gpio_b_crl == 0u);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 0u);
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_enter(200u) == 0);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    esp_stm32_owner_v1_service(350u);
    esp_stm32_owner_v1_leave(1u, 350u);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 0u);
    puts("STM32_OWNER_CURRENT_BOARD_DEFAULT_DISABLED=PASS NO_MMIO_NO_ROM=YES");
#endif
    return 0;
}
