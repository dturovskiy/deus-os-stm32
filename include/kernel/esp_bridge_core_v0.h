/* ESP Flash Bridge v0 CORE ONLY; OFFLINE host-model candidate.
 * Source-only firmware integration candidate, NOT installed STM32 v8. Never in IRQ.
 * No GPIO/ROM/reset/USB/UART operations exist in this module.
 */
#ifndef ESP_BRIDGE_CORE_V0_H
#define ESP_BRIDGE_CORE_V0_H

#include <stdint.h>
#define ESP_BRIDGE_MAX_DATA 40u
#define ESP_BRIDGE_MAX_REPLY_PAYLOAD 49u
enum { ESP_BRIDGE_OPEN = 1u, ESP_BRIDGE_TX = 2u,
       ESP_BRIDGE_RX = 3u, ESP_BRIDGE_CLOSE = 4u,
       /* Separate non-destructive ROM SYNC-only session. The legacy OPEN
        * remains generic/Flash-uncertain and is always fail-closed. */
       ESP_BRIDGE_OPEN_ROM_SYNC_ONLY = 5u,
       /* New distinct ROM-only admission: at most five identical SYNC46
        * frames after ONE STM32-owned boot transition; no Flash commands. */
       ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN = 6u };
enum { ESP_BRIDGE_OK = 0u, ESP_BRIDGE_DENIED = 1u,
       ESP_BRIDGE_BAD_SESSION = 2u, ESP_BRIDGE_BAD_SEQUENCE = 3u,
       ESP_BRIDGE_BAD_LENGTH = 4u, ESP_BRIDGE_UART_UNCERTAIN = 5u };
/* Read-only diagnostics of LAST syntactically valid OPEN, not a new
 * admission or reset API. Never call firmware callbacks from the getter.
 * A value remains latched until a subsequent OPEN or MCU initialization. */
enum {
    ESP_BRIDGE_OPEN_NOT_SEEN = 0u,
    ESP_BRIDGE_OPEN_ACCEPTED = 1u,
    ESP_BRIDGE_OPEN_DENY_SEQUENCE = 2u,
    ESP_BRIDGE_OPEN_DENY_ACTIVE = 3u,
    ESP_BRIDGE_OPEN_DENY_FAULTED = 4u,
    ESP_BRIDGE_OPEN_DENY_CALLBACKS = 5u,
    ESP_BRIDGE_OPEN_DENY_CAN_ENTER = 6u,
    ESP_BRIDGE_OPEN_DENY_ENTER = 7u
};

typedef struct {
    uint32_t session;
    uint16_t last_sequence;
    uint8_t active;
    uint8_t faulted;
    /* 1 after any accepted UART TX/RX request, including a failed call.
     * A pre-UART abort is recoverable; an uncertain UART abort is not. */
    uint8_t uart_attempted;
    uint8_t last_open_reason;
    /* True ONLY after OPEN_ROM_SYNC_ONLY; no arbitrary UART TX is possible.
     * The kernel verifies exact 46-byte ESP8266 ROM SYNC stream before TX.
     * Safe mode permits local bridge recovery, never ESP Flash recovery. */
    uint8_t rom_sync_only;
    uint8_t rom_sync_tx_offset;
    uint8_t rom_sync_train; /* op6 only; legacy op5 remains single-SYNC */
    uint8_t rom_sync_frames; /* completed exact SYNC46 bursts, max five */
} esp_bridge_core_v0_t;

typedef struct {
    /* Only privileged STM32 firmware may supply these callbacks.
     * can_enter MUST prove sole physical ESP control + quiescent Node Link.
     * Null callbacks imply DENIED, not permissive defaults. */
    int (*can_enter)(void *context);
    int (*enter)(void *context);           /* park Node Link atomically */
    void (*leave)(void *context);          /* no ESP reset/normal-boot claim */
    int (*tx)(void *context, const uint8_t *data, uint8_t length);
    int (*rx)(void *context, uint8_t *data, uint8_t capacity);
    void *context;
} esp_bridge_io_v0_t;

void esp_bridge_core_v0_init(esp_bridge_core_v0_t *state);
void esp_bridge_core_v0_abort(esp_bridge_core_v0_t *state,
                              const esp_bridge_io_v0_t *io);
/* Outer Management frame has already been CRC/header/request-id validated.
 * Input is exact decapsulated op/session/seq/count/data bytes.
 * Reply is exact response payload: op/session/seq/status/count/data.
 * Returns 0 for structurally unparseable inputs or invalid output buffer;
 * otherwise 9..49; caller adds existing USB Management frame/CRC.
 * no delays, no alloc, no direct hardware.
 */
uint8_t esp_bridge_core_v0_handle(esp_bridge_core_v0_t *state,
                                  const esp_bridge_io_v0_t *io,
                                  const uint8_t *in, uint8_t in_len,
                                  uint8_t *out, uint8_t out_cap);
#endif
