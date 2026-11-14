/* ESP-01 target-side v0 state machine: NO DIRECT HARDWARE / NO ROM FLASH.
 * Source-only integration candidate: NOT installed on the STM32.
 */
#include "kernel/esp_bridge_core_v0.h"

static uint32_t get_u32(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) |
        ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}
static uint16_t get_u16(const uint8_t *b)
{
    return (uint16_t)((uint16_t)b[0] | ((uint16_t)b[1] << 8));
}

void esp_bridge_core_v0_init(esp_bridge_core_v0_t *state)
{
    if (state != (esp_bridge_core_v0_t *)0)
    {
        state->session = 0u;
        state->last_sequence = 0u;
        state->active = 0u;
        state->faulted = 0u;
        state->uart_attempted = 0u;
        state->last_open_reason = ESP_BRIDGE_OPEN_NOT_SEEN;
    }
}

void esp_bridge_core_v0_abort(esp_bridge_core_v0_t *state,
                              const esp_bridge_io_v0_t *io)
{
    uint8_t uncertain;
    if (state == (esp_bridge_core_v0_t *)0)
        return;
    /* No UART request means ESP Flash cannot have started through this
     * bridge. Leave maintenance and permit a fresh session after timeout.
     * Once TX/RX was attempted, preserve fail-closed until MCU recovery. */
    uncertain = (uint8_t)((state->faulted != 0u) ||
                          (state->uart_attempted != 0u));
    if (state->active != 0u && io != (const esp_bridge_io_v0_t *)0 &&
        io->leave != (void (*)(void *))0)
        io->leave(io->context);
    state->session = 0u;
    state->last_sequence = 0u;
    state->active = 0u;
    state->uart_attempted = 0u;
    state->faulted = uncertain;
}

uint8_t esp_bridge_core_v0_handle(esp_bridge_core_v0_t *state,
                                  const esp_bridge_io_v0_t *io,
                                  const uint8_t *in, uint8_t in_len,
                                  uint8_t *out, uint8_t out_cap)
{
    uint8_t op, count, status = ESP_BRIDGE_DENIED, rx_count = 0u;
    uint32_t sid;
    uint16_t seq;
    uint8_t i;

    if (state == (esp_bridge_core_v0_t *)0 ||
        in == (const uint8_t *)0 || out == (uint8_t *)0 ||
        in_len < 8u || out_cap < ESP_BRIDGE_MAX_REPLY_PAYLOAD)
        return 0u;

    op = in[0]; sid = get_u32(in + 1); seq = get_u16(in + 5);
    count = in[7];
    for (i = 0u; i < 7u; ++i) out[i] = in[i];
    out[8] = 0u; /* Data count after status. */

    if (sid == 0u || count > ESP_BRIDGE_MAX_DATA ||
        (uint16_t)(8u + (uint16_t)count) != in_len ||
        (op != ESP_BRIDGE_TX && count != 0u) ||
        (op == ESP_BRIDGE_TX && count == 0u) ||
        op < ESP_BRIDGE_OPEN || op > ESP_BRIDGE_CLOSE)
    {
        status = ESP_BRIDGE_BAD_LENGTH;
    }
    else if (op == ESP_BRIDGE_OPEN)
    {
        /* Preserve original ordered short-circuit admission. Record ONLY
         * which already-existing predicate rejected the last OPEN. */
        if (seq != 0u)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_SEQUENCE;
        else if (state->active != 0u)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_ACTIVE;
        else if (state->faulted != 0u)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_FAULTED;
        else if (io == (const esp_bridge_io_v0_t *)0 ||
                 io->can_enter == (int (*)(void *))0 ||
                 io->enter == (int (*)(void *))0 ||
                 io->leave == (void (*)(void *))0 ||
                 io->tx == (int (*)(void *, const uint8_t *, uint8_t))0 ||
                 io->rx == (int (*)(void *, uint8_t *, uint8_t))0)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_CALLBACKS;
        else if (io->can_enter(io->context) == 0)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_CAN_ENTER;
        else if (io->enter(io->context) == 0)
            state->last_open_reason = ESP_BRIDGE_OPEN_DENY_ENTER;
        else
        {
            state->active = 1u;
            state->session = sid;
            state->last_sequence = 0u;
            state->uart_attempted = 0u;
            state->last_open_reason = ESP_BRIDGE_OPEN_ACCEPTED;
            status = ESP_BRIDGE_OK;
        }
    }
    else if (state->faulted != 0u || state->active == 0u ||
             state->session != sid)
    {
        status = ESP_BRIDGE_BAD_SESSION;
    }
    else if (seq == 0u || seq != (uint16_t)(state->last_sequence + 1u) ||
             state->last_sequence == 0xFFFFu)
    {
        status = ESP_BRIDGE_BAD_SEQUENCE; /* Deny wrap/replay. */
    }
    else if (op == ESP_BRIDGE_TX)
    {
        state->uart_attempted = 1u; /* Record before potentially failing TX. */
        if (io != (const esp_bridge_io_v0_t *)0 &&
            io->tx != (int (*)(void *, const uint8_t *, uint8_t))0 &&
            io->tx(io->context, in + 8, count) != 0)
        {
            status = ESP_BRIDGE_OK;
            state->last_sequence = seq;
        }
        else
        {
            state->faulted = 1u;
            status = ESP_BRIDGE_UART_UNCERTAIN;
        }
    }
    else if (op == ESP_BRIDGE_RX)
    {
        int result = -1;
        state->uart_attempted = 1u; /* Record before potentially failing RX. */
        if (io != (const esp_bridge_io_v0_t *)0 &&
            io->rx != (int (*)(void *, uint8_t *, uint8_t))0)
            result = io->rx(io->context, out + 9, ESP_BRIDGE_MAX_DATA);
        if (result >= 0 && result <= (int)ESP_BRIDGE_MAX_DATA)
        {
            rx_count = (uint8_t)result;
            state->last_sequence = seq;
            status = ESP_BRIDGE_OK;
        }
        else
        {
            state->faulted = 1u;
            status = ESP_BRIDGE_UART_UNCERTAIN;
        }
    }
    else if (op == ESP_BRIDGE_CLOSE)
    {
        if (io != (const esp_bridge_io_v0_t *)0 &&
            io->leave != (void (*)(void *))0)
            io->leave(io->context);
        state->active = 0u;
        state->session = 0u;
        state->last_sequence = 0u;
        state->uart_attempted = 0u;
        status = ESP_BRIDGE_OK;
    }

    out[7] = status;
    out[8] = rx_count;
    return (uint8_t)(9u + rx_count);
}
