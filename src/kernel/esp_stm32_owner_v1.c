#include "kernel/esp_stm32_owner_v1.h"
#if DEUS_ESP_STM32_BOARD_QUALIFIED
#include "kernel/esp_boot_control_v0.h"
#include "drivers/esp_gpio_board_v0.h"
#include "kernel/node_transport.h"

/* This module owns ONLY qualified STM32-controlled GPIO0/EN.
 * UART bytes remain exclusively in usb_management + USART2. The
 * board-qualification constant must be enabled only in a new signed,
 * exact-hardware deployment after CH340 power/BOOT controls are detached. */
static esp_gpio_board_v0_t board;
static esp_boot_control_v0_t boot;
static uint8_t initialized;
static uint8_t in_bridge;

static int gpio0_set(void *ctx, uint8_t low)
{
    return esp_gpio_board_v0_gpio0_pull_low((esp_gpio_board_v0_t *)ctx, low);
}
static int en_set(void *ctx, uint8_t low)
{
    return esp_gpio_board_v0_en_pull_low((esp_gpio_board_v0_t *)ctx, low);
}
static int qualified_rail(void *ctx)
{
    /* Hardware qualification of a dedicated ESP rail is encoded in the
     * source-locked board variant, not inferred from USB or an ADC read. */
    return ((esp_gpio_board_v0_t *)ctx)->qualified == 1u;
}
static int no_pending_job(void *ctx)
{
    node_transport_diagnostics_t d;
    (void)ctx;
    node_transport_diagnostics_get(&d);
    return d.pending == 0u;
}
static const esp_boot_io_v0_t io = {
    gpio0_set, en_set, qualified_rail, no_pending_job, &board
};
#endif

void esp_stm32_owner_v1_init(uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    esp_gpio_board_v0_reset(&board);
    esp_boot_control_v0_init(&boot);
    in_bridge = 0u;
    initialized = 0u;
    if (esp_gpio_board_v0_init(&board, 1u) != 0 &&
        esp_boot_control_v0_provision(&boot, &io, 1u, now_ms) != 0)
        initialized = 1u;
#else
    (void)now_ms;
#endif
}

int esp_stm32_owner_v1_can_enter(void)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    return initialized == 1u && in_bridge == 0u &&
        boot.provisioned == 1u && no_pending_job(&board) != 0 &&
        (boot.state == ESP_BOOT_APP_AWAIT_HELLO ||
         boot.state == ESP_BOOT_NORMAL_LINK_VERIFIED);
#else
    return 0;
#endif
}

int esp_stm32_owner_v1_enter(uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    if (esp_stm32_owner_v1_can_enter() == 0 ||
        esp_boot_control_v0_recovery_rom_start(
            &boot, &io, 1u, now_ms) == 0)
        return 0;
    in_bridge = 1u;
    return 1;
#else
    (void)now_ms;
    return 0;
#endif
}

int esp_stm32_owner_v1_rom_tx_ready(void)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    return in_bridge == 1u &&
        boot.state == ESP_BOOT_ROM_AWAIT_SYNC;
#else
    return 0;
#endif
}

void esp_stm32_owner_v1_service(uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    if (initialized != 0u)
        esp_boot_control_v0_service(&boot, &io, now_ms);
#else
    (void)now_ms;
#endif
}

void esp_stm32_owner_v1_leave(uint8_t uncertain, uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    if (in_bridge == 0u) return;
    in_bridge = 0u;
    if (uncertain != 0u ||
        esp_boot_control_v0_app_start(&boot, &io, now_ms) == 0)
        esp_boot_control_v0_fault(&boot, &io);
#else
    (void)uncertain;
    (void)now_ms;
#endif
}
