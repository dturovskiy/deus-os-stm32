/* Product-source STM32-owned ESP ROM bridge: fake registers only. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/esp_stm32_owner_v1.h"
#include "kernel/esp_boot_control_v0.h"
#include "kernel/node_transport.h"

#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"FAIL line=%d: %s\n", __LINE__, #x); exit(2); } } while(0)
volatile uint32_t esp_gpio_mock_rcc_apb2enr;
volatile uint32_t esp_gpio_mock_gpio_b_crl;
volatile uint32_t esp_gpio_mock_gpio_b_bsrr;
volatile uint32_t esp_gpio_mock_gpio_b_idr;
volatile uint32_t esp_gpio_mock_gpio_b_odr;
static uint8_t pending;
static uint32_t invalidations;
static uint8_t ready;
static uint32_t hello_acks;
static uint32_t rom_boot_bits;
uint32_t usart2_node_rom_boot_status_bits(void) { return rom_boot_bits; }
int node_transport_app_reset_begin(void)
{
    if (pending != 0u) return 0;
    ++invalidations;
    ready = 0u; /* Reset invalidates the accepted Node session. */
    return 1;
}

void node_transport_diagnostics_get(node_transport_diagnostics_t *out)
{
    memset(out, 0, sizeof(*out));
    out->pending = pending;
    out->ready = ready;
    out->hello_acks = hello_acks;
}
int main(void)
{
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_enter(10u) == 0);
    esp_stm32_owner_v1_init(100u);
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    REQUIRE((esp_gpio_mock_rcc_apb2enr & 8u) != 0u);
    REQUIRE((esp_gpio_mock_gpio_b_crl & 0xffu) == 0x66u);
    REQUIRE(esp_stm32_owner_v1_rom_pin_trace() == 0u);
    REQUIRE(esp_stm32_owner_v1_can_enter() != 0);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    /* READY alone or a stale ACK counter alone must not promote boot. */
    ready = 1u;
    esp_stm32_owner_v1_service(101u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    ready = 0u;
    hello_acks = 1u;
    esp_stm32_owner_v1_service(102u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    ready = 1u; /* Validated HELLO_ACK delivered by NodeLink. */
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 2u);
    esp_stm32_owner_v1_service(103u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_NORMAL_LINK_VERIFIED);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 2u); /* No implicit GPIO reset. */
    esp_stm32_owner_v1_service(104u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_NORMAL_LINK_VERIFIED);
    pending = 1u;
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_enter(101u) == 0);
    REQUIRE(esp_stm32_owner_v1_reset_app(101u) == 0);
    REQUIRE(invalidations == 0u);
    pending = 0u;
    /* Successful recovery to ESP ROM does NOT depend on app HELLO. */
    REQUIRE(esp_stm32_owner_v1_enter(110u) != 0);
    REQUIRE(esp_stm32_owner_v1_rom_pin_trace() == 0x00010000u);
    ready = 0u; /* Production maintenance invalidates READY. */
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    REQUIRE(esp_stm32_owner_v1_reset_app(115u) == 0);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == ((uint32_t)1u << 16));
    esp_stm32_owner_v1_service(129u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    /* Simulate released EN HIGH, held-low GPIO0 at MCU pads. */
    esp_gpio_mock_gpio_b_idr = 2u;
    esp_gpio_mock_gpio_b_odr = 2u;
    esp_stm32_owner_v1_service(130u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    esp_stm32_owner_v1_service(230u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() != 0);
    REQUIRE(esp_stm32_owner_v1_rom_pin_trace() == 0x00070AA0u);
    /* Additive compact 74880 boot report does not disturb GPIO trace.
     * 42 bytes, mode3 FLAS_APP, no hardware dependency in test. */
    rom_boot_bits = (42u << 24) | (3u << 20);
    esp_stm32_owner_v1_leave(0u, 240u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() == 0);
    REQUIRE(esp_stm32_owner_v1_rom_pin_trace() == 0x2A370AA0u);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 1u);
    esp_stm32_owner_v1_service(260u);
    esp_stm32_owner_v1_service(360u);
    REQUIRE(esp_stm32_owner_v1_can_enter() != 0);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    /* Old successful HELLO counter cannot resurrect a reset session. */
    esp_stm32_owner_v1_service(361u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    /* New explicit normal-app restart is not a ROM transition. */
    REQUIRE(esp_stm32_owner_v1_reset_app(370u) != 0);
    REQUIRE(invalidations == 1u && esp_stm32_owner_v1_app_reset_count() == 1u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_EN_LOW);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 1u); /* GPIO0 released last */
    REQUIRE(esp_stm32_owner_v1_reset_app(371u) == 0); /* no double pulse */
    REQUIRE(esp_stm32_owner_v1_can_enter() == 0);
    esp_stm32_owner_v1_service(389u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_EN_LOW);
    esp_stm32_owner_v1_service(390u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_SETTLING);
    esp_stm32_owner_v1_service(490u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == ESP_BOOT_APP_AWAIT_HELLO);
    REQUIRE(esp_stm32_owner_v1_can_enter() != 0);
    /* Unknown USB/ROM outcome freezes ESP via EN LOW, no auto resume. */
    REQUIRE(esp_stm32_owner_v1_enter(500u) != 0);
    esp_stm32_owner_v1_service(520u);
    esp_stm32_owner_v1_service(620u);
    REQUIRE(esp_stm32_owner_v1_rom_tx_ready() != 0);
    esp_stm32_owner_v1_leave(1u, 630u);
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
    REQUIRE(esp_stm32_owner_v1_reset_app(300u) == 0);
    REQUIRE(esp_stm32_owner_v1_app_reset_count() == 0u);
    REQUIRE(esp_stm32_owner_v1_boot_state() == 0u);
    REQUIRE(esp_stm32_owner_v1_rom_pin_trace() == 0u);
    REQUIRE(invalidations == 0u);
    esp_stm32_owner_v1_service(350u);
    esp_stm32_owner_v1_leave(1u, 350u);
    REQUIRE(esp_gpio_mock_gpio_b_bsrr == 0u);
    puts("STM32_OWNER_CURRENT_BOARD_DEFAULT_DISABLED=PASS NO_MMIO_NO_ROM=YES");
#endif
    return 0;
}
