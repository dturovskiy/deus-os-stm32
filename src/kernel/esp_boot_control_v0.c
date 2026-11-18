/* Hardware-independent ESP GPIO0/EN transition model.
 * Product source candidate. Firmware board ownership is compile-time disabled by default.
 * No actual register access, no GPIO initialization, no power control.
 */
#include "kernel/esp_boot_control_v0.h"

static int io_valid(const esp_boot_io_v0_t *io)
{
    return io != (const esp_boot_io_v0_t *)0 &&
        io->gpio0_pull_low != (int (*)(void *, uint8_t))0 &&
        io->en_pull_low != (int (*)(void *, uint8_t))0 &&
        io->rail_verified != (int (*)(void *))0 &&
        io->node_quiescent != (int (*)(void *))0;
}
void esp_boot_control_v0_init(esp_boot_control_v0_t *s)
{
    if (s == (esp_boot_control_v0_t *)0) return;
    s->state = ESP_BOOT_DISABLED;
    s->started_ms = 0u;
    s->provisioned = 0u;
}

void esp_boot_control_v0_fault(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io)
{
    if (s == (esp_boot_control_v0_t *)0) return;
    s->provisioned = 0u;
    s->state = ESP_BOOT_FAULT;
    /* A best-effort EN-low is safer than releasing an unknown ROM state.
     * If EN cannot be asserted, external physical intervention is required. */
    if (io_valid(io)) {
        (void)io->en_pull_low(io->ctx, 1u);
        (void)io->gpio0_pull_low(io->ctx, 0u);
    }
}
int esp_boot_control_v0_provision(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t owner_qualified, uint32_t now_ms)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        s->state != ESP_BOOT_DISABLED || owner_qualified == 0u ||
        !io_valid(io) || io->rail_verified(io->ctx) == 0 ||
        io->node_quiescent(io->ctx) == 0) return 0;
    /* On boot, both pins high-Z, external pulls maintain normal mode.
     * This doesn't prove ESP normal application HELLO. */
    if (io->gpio0_pull_low(io->ctx, 0u) == 0 ||
        io->en_pull_low(io->ctx, 0u) == 0) {
        esp_boot_control_v0_fault(s, io);
        return 0;
    }
    s->provisioned = 1u;
    s->started_ms = now_ms;
    s->state = ESP_BOOT_APP_AWAIT_HELLO;
    return 1;
}
int esp_boot_control_v0_rom_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        s->provisioned == 0u ||
        s->state != ESP_BOOT_NORMAL_LINK_VERIFIED ||
        !io_valid(io) ||
        io->rail_verified(io->ctx) == 0 ||
        io->node_quiescent(io->ctx) == 0) return 0;

    /* Disable ESP before changing GPIO0 boot strap. */
    if (io->en_pull_low(io->ctx, 1u) == 0) {
        esp_boot_control_v0_fault(s, io); return 0;
    }
    if (io->gpio0_pull_low(io->ctx, 1u) == 0) {
        esp_boot_control_v0_fault(s, io); return 0;
    }
    s->started_ms = now_ms;
    s->state = ESP_BOOT_ROM_EN_LOW;
    return 1;
}
/* Rescue does not depend on a live ESP application HELLO. */
int esp_boot_control_v0_recovery_rom_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t local_board_qualified, uint32_t now_ms)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        local_board_qualified != 1u || s->provisioned != 1u ||
        !io_valid(io) || io->rail_verified(io->ctx) == 0 ||
        io->node_quiescent(io->ctx) == 0)
        return 0;
    if (s->state == ESP_BOOT_NORMAL_LINK_VERIFIED)
        return esp_boot_control_v0_rom_start(s, io, now_ms);
    if (s->state != ESP_BOOT_APP_AWAIT_HELLO)
        return 0;
    if (io->en_pull_low(io->ctx, 1u) == 0 ||
        io->gpio0_pull_low(io->ctx, 1u) == 0) {
        esp_boot_control_v0_fault(s, io);
        return 0;
    }
    s->started_ms = now_ms;
    s->state = ESP_BOOT_ROM_EN_LOW;
    return 1;
}
/* A normal application reset is distinct from ROM recovery and cannot
 * assert GPIO0 low. The scheduler services EN LOW for >=20 ms followed by
 * a >=100 ms settle, never spinning with interrupts or USB disabled. */
int esp_boot_control_v0_normal_app_reset(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint8_t local_board_qualified, uint32_t now_ms)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        local_board_qualified != 1u || s->provisioned != 1u ||
        (s->state != ESP_BOOT_APP_AWAIT_HELLO &&
         s->state != ESP_BOOT_NORMAL_LINK_VERIFIED) ||
        !io_valid(io) || io->rail_verified(io->ctx) == 0 ||
        io->node_quiescent(io->ctx) == 0)
        return 0;
    /* EN LOW first, then release GPIO0. If either write fails, keep EN LOW. */
    if (io->en_pull_low(io->ctx, 1u) == 0 ||
        io->gpio0_pull_low(io->ctx, 0u) == 0) {
        esp_boot_control_v0_fault(s, io);
        return 0;
    }
    s->started_ms = now_ms;
    s->state = ESP_BOOT_APP_EN_LOW;
    return 1;
}
int esp_boot_control_v0_app_start(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        s->provisioned == 0u ||
        s->state != ESP_BOOT_ROM_AWAIT_SYNC ||
        !io_valid(io) ||
        io->rail_verified(io->ctx) == 0 ||
        io->node_quiescent(io->ctx) == 0) return 0;
    if (io->en_pull_low(io->ctx, 1u) == 0) {
        esp_boot_control_v0_fault(s, io); return 0;
    }
    /* Deassert strap LOW before re-enabling ESP for normal boot. */
    if (io->gpio0_pull_low(io->ctx, 0u) == 0) {
        esp_boot_control_v0_fault(s, io); return 0;
    }
    s->started_ms = now_ms;
    s->state = ESP_BOOT_APP_EN_LOW;
    return 1;
}
void esp_boot_control_v0_service(
    esp_boot_control_v0_t *s, const esp_boot_io_v0_t *io,
    uint32_t now_ms)
{
    uint32_t elapsed;
    if (s == (esp_boot_control_v0_t *)0 || !io_valid(io) ||
        s->provisioned == 0u) return;
    if (io->rail_verified(io->ctx) == 0) {
        esp_boot_control_v0_fault(s, io);return;
    }
    if (s->state != ESP_BOOT_ROM_EN_LOW &&
        s->state != ESP_BOOT_APP_EN_LOW &&
        s->state != ESP_BOOT_ROM_SETTLING &&
        s->state != ESP_BOOT_APP_SETTLING) return;
    elapsed = (uint32_t)(now_ms - s->started_ms);
    if (elapsed > ESP_BOOT_PROGRESS_MAX_MS) {
        esp_boot_control_v0_fault(s, io);return;
    }
    if (s->state == ESP_BOOT_ROM_SETTLING ||
        s->state == ESP_BOOT_APP_SETTLING) {
        if (elapsed >= ESP_BOOT_SETTLE_MS) {
            s->state = s->state == ESP_BOOT_ROM_SETTLING ?
                ESP_BOOT_ROM_AWAIT_SYNC : ESP_BOOT_APP_AWAIT_HELLO;
        }
        return;
    }
    if (elapsed < ESP_BOOT_EN_LOW_MS) return;
    if (io->en_pull_low(io->ctx, 0u) == 0) {
        esp_boot_control_v0_fault(s, io);return;
    }
    s->started_ms = now_ms;
    s->state = s->state == ESP_BOOT_ROM_EN_LOW ?
        ESP_BOOT_ROM_SETTLING : ESP_BOOT_APP_SETTLING;
}
int esp_boot_control_v0_note_fresh_hello(esp_boot_control_v0_t *s)
{
    if (s == (esp_boot_control_v0_t *)0 ||
        s->provisioned == 0u ||
        s->state != ESP_BOOT_APP_AWAIT_HELLO) return 0;
    s->state = ESP_BOOT_NORMAL_LINK_VERIFIED;
    return 1;
}
