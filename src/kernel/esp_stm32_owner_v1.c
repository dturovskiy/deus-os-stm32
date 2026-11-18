#include "kernel/esp_stm32_owner_v1.h"
#if DEUS_ESP_STM32_BOARD_QUALIFIED
#include "kernel/esp_boot_control_v0.h"
#include "drivers/esp_gpio_board_v0.h"
#include "kernel/node_transport.h"
#include "drivers/usart2_node.h"

/* Snapshot only the STM32 pad and output-latch bits during a future
 * explicitly authorized ESP ROM transition. Never writes these registers. */
#ifdef DEUS_ESP_GPIO_MMIO_MOCK
extern volatile uint32_t esp_gpio_mock_gpio_b_idr;
extern volatile uint32_t esp_gpio_mock_gpio_b_odr;
#define ESP_GPIOB_IDR esp_gpio_mock_gpio_b_idr
#define ESP_GPIOB_ODR esp_gpio_mock_gpio_b_odr
#else
#define ESP_GPIOB_IDR (*(volatile uint32_t *)0x40010C08u)
#define ESP_GPIOB_ODR (*(volatile uint32_t *)0x40010C0Cu)
#endif

/* This module owns ONLY qualified STM32-controlled GPIO0/EN.
 * UART bytes remain exclusively in usb_management + USART2. The
 * board-qualification constant must be enabled only in a new signed,
 * exact-hardware deployment after CH340 power/BOOT controls are detached. */
static esp_gpio_board_v0_t board;
static esp_boot_control_v0_t boot;
static uint8_t initialized;
static uint8_t in_bridge;
static uint32_t app_reset_count;
/* ROM-only 3-stage readback: low nibble after EN+GPIO0 LOW,
 * next nibble after EN release, next after ROM settle.
 * Each nibble = (IDR[PB1:PB0] << 2) | ODR[PB1:PB0].
 * Bits16..18 indicate stages observed. Persistent after normal CLOSE. */
static uint32_t rom_pin_trace;
static uint32_t rom_pins(void)
{
    return ((ESP_GPIOB_IDR & 3u) << 2) | (ESP_GPIOB_ODR & 3u);
}

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
    app_reset_count = 0u;
    rom_pin_trace = 0u;
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
    rom_pin_trace = 0x00010000u | rom_pins();
    return 1;
#else
    (void)now_ms;
    return 0;
#endif
}

/* No implicit power cycle on USB HELLO/boot; only a dedicated command may
 * initiate this normal-app-only transition on the qualified GPIO board. */
int esp_stm32_owner_v1_reset_app(uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    if (initialized != 1u || in_bridge != 0u ||
        boot.provisioned != 1u || no_pending_job(&board) == 0 ||
        (boot.state != ESP_BOOT_APP_AWAIT_HELLO &&
         boot.state != ESP_BOOT_NORMAL_LINK_VERIFIED) ||
        node_transport_app_reset_begin() == 0)
        return 0;
    if (esp_boot_control_v0_normal_app_reset(&boot, &io, 1u, now_ms) == 0)
        return 0;
    ++app_reset_count;
    return 1;
#else
    (void)now_ms;
    return 0;
#endif
}
uint32_t esp_stm32_owner_v1_rom_pin_trace(void)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    return rom_pin_trace;
#else
    return 0u;
#endif
}
uint32_t esp_stm32_owner_v1_app_reset_count(void)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    return app_reset_count;
#else
    return 0u;
#endif
}
uint32_t esp_stm32_owner_v1_boot_state(void)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    return (uint32_t)boot.state;
#else
    return 0u;
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
    if (initialized != 0u) {
        const esp_boot_state_v0_t before = boot.state;
        esp_boot_control_v0_service(&boot, &io, now_ms);
        if (in_bridge != 0u && before != boot.state) {
            if (boot.state == ESP_BOOT_ROM_SETTLING)
                rom_pin_trace |= 0x00020000u | (rom_pins() << 4);
            else if (boot.state == ESP_BOOT_ROM_AWAIT_SYNC)
                rom_pin_trace |= 0x00040000u | (rom_pins() << 8);
        }
        /* The NodeLink controller has already correlated a fresh HELLO_ACK.
         * Only its accepted READY state may promote the local ESP boot FSM.
         * Maintenance invalidates READY and ROM ownership bars promotion. */
        if (in_bridge == 0u && boot.state == ESP_BOOT_APP_AWAIT_HELLO) {
            node_transport_diagnostics_t d;
            node_transport_diagnostics_get(&d);
            if (d.ready != 0u && d.hello_acks != 0u)
                (void)esp_boot_control_v0_note_fresh_hello(&boot);
        }
    }
#else
    (void)now_ms;
#endif
}

void esp_stm32_owner_v1_leave(uint8_t uncertain, uint32_t now_ms)
{
#if DEUS_ESP_STM32_BOARD_QUALIFIED
    if (in_bridge == 0u) return;
    /* Append early ROM 74880 RX evidence to unused high trace bits.
     * Preserve all three prior GPIOB stage samples in bits18:0.
     * UART RX mode is a hint, never proof of actual ESP pad voltage. */
    rom_pin_trace |= usart2_node_rom_boot_status_bits();
    in_bridge = 0u;
    if (uncertain != 0u ||
        esp_boot_control_v0_app_start(&boot, &io, now_ms) == 0)
        esp_boot_control_v0_fault(&boot, &io);
#else
    (void)uncertain;
    (void)now_ms;
#endif
}
