#ifndef KERNEL_ESP_BOOT_CONTROL_V0_H
#define KERNEL_ESP_BOOT_CONTROL_V0_H

#include <stdint.h>

/* SOURCE-ONLY, NOT LINKED/INSTALLED. Never call in an IRQ.
 * Future board adapter: proposed B0(PB0)->ESP GPIO0/BOOT,
 * B1(PB1)->ESP EN/CH_PD, both open drain with external 3.3 V pullups.
 * This module emits NO GPIO register writes on its own.
 */
#define ESP_BOOT_EN_LOW_MS 20u
#define ESP_BOOT_SETTLE_MS 100u
#define ESP_BOOT_PROGRESS_MAX_MS 1000u

typedef enum {
    ESP_BOOT_DISABLED = 0,
    ESP_BOOT_NORMAL_LINK_VERIFIED = 1,
    ESP_BOOT_ROM_EN_LOW = 2,
    ESP_BOOT_ROM_AWAIT_SYNC = 3,
    ESP_BOOT_APP_EN_LOW = 4,
    ESP_BOOT_APP_AWAIT_HELLO = 5,
    ESP_BOOT_ROM_SETTLING = 6,
    ESP_BOOT_APP_SETTLING = 7,
    ESP_BOOT_FAULT = 8
} esp_boot_state_v0_t;

typedef struct {
    /* pull_low=1 forces open-drain GPIO low; 0 RELEASES (high impedance).
     * Return 1 only when commanded pin state is known to have been applied. */
    int (*gpio0_pull_low)(void *ctx, uint8_t pull_low);
    int (*en_pull_low)(void *ctx, uint8_t pull_low);
    /* Local, trusted board-owned predicates, NOT host-supplied USB flags. */
    int (*rail_verified)(void *ctx);
    int (*node_quiescent)(void *ctx);
    void *ctx;
} esp_boot_io_v0_t;

typedef struct {
    esp_boot_state_v0_t state;
    uint32_t started_ms;
    uint8_t provisioned; /* nonzero ONLY in separately qualified board adapter */
} esp_boot_control_v0_t;

void esp_boot_control_v0_init(esp_boot_control_v0_t *s);
/* Only firmware-local board adapter may call this after GPIO/rail qualification.
 * A USB session/host request ID must NEVER authorize provisioning. */
int esp_boot_control_v0_provision(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t owner_qualified, uint32_t now_ms);
int esp_boot_control_v0_rom_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms);
/* Privileged, firmware-LOCAL rescue path: allows ROM boot when the ESP app
 * cannot ACK HELLO. owner_qualified MUST originate from a built, electrically
 * reviewed STM32 board configuration, never from USB request contents. */
int esp_boot_control_v0_recovery_rom_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t local_board_qualified, uint32_t now_ms);
int esp_boot_control_v0_app_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms);
/* Explicit normal-application reset. Unlike app_start, this does NOT enter
 * ROM and may be called from APP_AWAIT_HELLO even when the ESP is silent.
 * Local board qualification and an idle Node session are mandatory. */
int esp_boot_control_v0_normal_app_reset(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t local_board_qualified, uint32_t now_ms);
void esp_boot_control_v0_service(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms);
/* External source-specific Node Link HELLO must be authenticated and fresh. */
int esp_boot_control_v0_note_fresh_hello(esp_boot_control_v0_t *s);
/* Never signal normal/ROM READY after a failure; attempt EN low. */
void esp_boot_control_v0_fault(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io);
#endif
