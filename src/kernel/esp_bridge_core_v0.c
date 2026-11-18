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

/* Exact ESP8266 ROM SYNC, SLIP-encoded, 46 bytes. No Flash, no stub,
 * no baud change, no commands chosen by a host. Prefixes are validated
 * BEFORE UART output so any accepted partial TX is non-destructive. */
#define ESP_ROM_SYNC_WIRE_BYTES 46u
#define ESP_ROM_SYNC_TRAIN_MAX_FRAMES 5u
static uint8_t esp_rom_sync_expected_byte(uint8_t offset)
{
    if (offset == 0u || offset == 45u) return 0xC0u;
    if (offset == 2u) return 0x08u;
    if (offset == 3u) return 0x24u;
    if (offset == 9u || offset == 10u) return 0x07u;
    if (offset == 11u) return 0x12u;
    if (offset == 12u) return 0x20u;
    if (offset >= 13u && offset <= 44u) return 0x55u;
    return 0x00u;
}

static int esp_rom_sync_only_tx_is_exact_prefix(
    uint8_t offset, const uint8_t *data, uint8_t count)
{
    uint8_t i;
    if (offset >= ESP_ROM_SYNC_WIRE_BYTES ||
        count > ESP_ROM_SYNC_WIRE_BYTES - offset)
        return 0;
    for (i = 0u; i < count; ++i)
        if (data[i] != esp_rom_sync_expected_byte(
                (uint8_t)(offset + i)))
            return 0;
    return 1;
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
        state->rom_sync_only = 0u;
        state->rom_sync_tx_offset = 0u;
        state->rom_sync_train = 0u;
        state->rom_sync_frames = 0u;
    }
}

void esp_bridge_core_v0_abort(esp_bridge_core_v0_t *state,
                              const esp_bridge_io_v0_t *io)
{
    uint8_t uncertain;
    if (state == (esp_bridge_core_v0_t *)0)
        return;
    /* Generic UART/Flash may be uncertain and stays latched. In the ROM
     * SYNC-only session EVERY accepted UART TX byte was verified against
     * the immutable, non-destructive ROM command. Recovery of the local
     * bridge is then safe even after a partial TX, lost USB reply or RX
     * framing error. This never clears a preexisting generic-Flash fault:
     * ROM-only OPEN is denied while the fault is already latched. */
    uncertain = (uint8_t)((state->rom_sync_only == 0u) &&
                          ((state->faulted != 0u) ||
                           (state->uart_attempted != 0u)));
    if (state->active != 0u && io != (const esp_bridge_io_v0_t *)0 &&
        io->leave != (void (*)(void *))0)
        io->leave(io->context);
    state->session = 0u;
    state->last_sequence = 0u;
    state->active = 0u;
    state->uart_attempted = 0u;
    state->rom_sync_only = 0u;
    state->rom_sync_tx_offset = 0u;
    state->rom_sync_train = 0u;
    state->rom_sync_frames = 0u;
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
        op < ESP_BRIDGE_OPEN || op > ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN)
    {
        status = ESP_BRIDGE_BAD_LENGTH;
    }
    else if (op == ESP_BRIDGE_OPEN ||
             op == ESP_BRIDGE_OPEN_ROM_SYNC_ONLY ||
             op == ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN)
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
            state->rom_sync_only = (uint8_t)(
                op == ESP_BRIDGE_OPEN_ROM_SYNC_ONLY ||
                op == ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN);
            state->rom_sync_tx_offset = 0u;
            state->rom_sync_train =
                (uint8_t)(op == ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN);
            state->rom_sync_frames = 0u;
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
        uint8_t offset = state->rom_sync_tx_offset;
        /* Op6 alone may stage the NEXT exact SYNC after the previous
         * complete request, at most five in this one hardware session.
         * Op5 is unchanged: any second TX is permanently denied. */
        if (state->rom_sync_only != 0u &&
            state->rom_sync_train != 0u &&
            offset == ESP_ROM_SYNC_WIRE_BYTES &&
            state->rom_sync_frames < ESP_ROM_SYNC_TRAIN_MAX_FRAMES)
            offset = 0u;
        if (state->rom_sync_only != 0u &&
            esp_rom_sync_only_tx_is_exact_prefix(offset, in + 8u, count) == 0)
        {
            /* Reject non-SYNC data BEFORE any UART side effect. Do not
             * advance the sequence: caller must CLOSE/abort, not retry. */
            status = ESP_BRIDGE_DENIED;
            out[7] = status;
            return 9u;
        }
        if (state->rom_sync_only != 0u &&
            (uint8_t)(offset + count) <
                ESP_ROM_SYNC_WIRE_BYTES)
        {
            /* Validate/stage via offset, but emit NO UART prefix.
             * USB ACK gap between 40+6 fragments must not interrupt the
             * ESP8266 ROM auto-baud 0x55 train. */
            state->rom_sync_tx_offset = (uint8_t)(offset + count);
            state->last_sequence = seq;
            status = ESP_BRIDGE_OK;
        }
        else
        {
            uint8_t sync_wire[ESP_ROM_SYNC_WIRE_BYTES];
            const uint8_t *wire = in + 8u;
            uint8_t wire_len = count;
            /* The only ROM output is a freshly reconstructed, already
             * fully prefix-checked exact 46-byte SYNC, in ONE UART call.
             * No host-controlled arbitrary bytes or Flash opcodes. */
            if (state->rom_sync_only != 0u)
            {
                for (i = 0u; i < ESP_ROM_SYNC_WIRE_BYTES; ++i)
                    sync_wire[i] = esp_rom_sync_expected_byte(i);
                wire = sync_wire;
                wire_len = ESP_ROM_SYNC_WIRE_BYTES;
            }
            state->uart_attempted = 1u; /* before potentially partial UART */
            if (io != (const esp_bridge_io_v0_t *)0 &&
                io->tx != (int (*)(void *, const uint8_t *, uint8_t))0 &&
                io->tx(io->context, wire, wire_len) != 0)
            {
                status = ESP_BRIDGE_OK;
                state->last_sequence = seq;
                if (state->rom_sync_only != 0u) {
                    state->rom_sync_tx_offset = ESP_ROM_SYNC_WIRE_BYTES;
                    ++state->rom_sync_frames; /* Exact UART TX completed. */
                }
            }
            else
            {
                state->faulted = 1u;
                status = ESP_BRIDGE_UART_UNCERTAIN;
            }
        }
    }
    else if (op == ESP_BRIDGE_RX)
    {
        int result = -1;
        if (state->rom_sync_only != 0u &&
            state->rom_sync_tx_offset != ESP_ROM_SYNC_WIRE_BYTES)
        {
            out[7] = ESP_BRIDGE_DENIED;
            return 9u; /* RX before all 46 exact TX bytes is forbidden. */
        }
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
        state->rom_sync_only = 0u;
        state->rom_sync_tx_offset = 0u;
        state->rom_sync_train = 0u;
        state->rom_sync_frames = 0u;
        status = ESP_BRIDGE_OK;
    }

    out[7] = status;
    out[8] = rx_count;
    return (uint8_t)(9u + rx_count);
}
