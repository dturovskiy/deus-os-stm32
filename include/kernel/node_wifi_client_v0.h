/* STM32 task0 optional NodeLink Wi-Fi extension V0 — SOURCE ONLY.
 * Frozen dnl_controller_t stays ABI-exact and owns session/seq, PING/CRC.
 * This adapter MUST use the SAME controller as node_transport. No device I/O.
 * Not in installed signed normal9; not yet wired into production build.
 */
#ifndef DEUS_NODE_WIFI_CLIENT_V0_H
#define DEUS_NODE_WIFI_CLIENT_V0_H

#include <stddef.h>
#include <stdint.h>
#include "deus_node_link.h"
#include "wifi_node_extension_v0.h"

#define DEUS_NODE_WIFI_REQ_DEADLINE_MS 650u
/* Explicit SDK radio startup may take substantially longer than DNL PING.
 * For SCAN_START never auto-retry after timeout: execution is ambiguous. */
#define DEUS_NODE_WIFI_START_DEADLINE_MS 3000u

typedef struct {
    dnl_frame_t pending_request;
    deus_wifi_ext_result_v0_t last_reply;
    uint32_t started_ms;
    uint32_t received_ms;
    uint32_t total_success;
    uint32_t total_remote_error;
    uint32_t total_timeouts;
    uint8_t has_last_reply;
    uint8_t remote_error;       /* 0 if successful; DNL_ERR_* otherwise */
} deus_node_wifi_client_v0_t;

void deus_node_wifi_client_v0_init(deus_node_wifi_client_v0_t *wifi);

/* Initiate exactly one request on an already READY shared DNL session.
 * Caller must transmit returned bytes once, using USART2 task0 exclusive
 * owner. On short/failed UART write caller MUST fail-close the session.
 * No Wi-Fi radio calls, no session HELLO generation performed here.
 */
int deus_node_wifi_client_v0_request(
    deus_node_wifi_client_v0_t *wifi, dnl_controller_t *controller,
    uint8_t opcode, uint32_t generation, uint8_t ap_index,
    uint8_t ssid_offset, uint32_t now_ms,
    uint8_t *wire, size_t capacity, size_t *wire_length);

/* Consume ONLY an authenticated/correlated Wi-Fi reply while the shared
 * controller is pending a Wi-Fi type. Returns DNL_UNSOLICITED for legacy
 * replies; DNL_OK / DNL_REMOTE_ERROR on confirmed response.
 * Bad/foreign responses preserve pending; no state is disclosed.
 */
int deus_node_wifi_client_v0_receive(
    deus_node_wifi_client_v0_t *wifi, dnl_controller_t *controller,
    const uint8_t *wire, size_t length, uint32_t now_ms);

/* Return deadline for current opcode (zero if not Wi-Fi pending).
 * If deadline exceeded: SCAN_START forces re-HELLO, other Wi-Fi commands
 * allow exactly one byte-identical retry. Second timeout fail-closes.
 * A timeout never auto-starts a second scan or auto-HELLO.
 */
uint32_t deus_node_wifi_client_v0_pending_deadline(
    const dnl_controller_t *controller);
int deus_node_wifi_client_v0_timeout(
    deus_node_wifi_client_v0_t *wifi, dnl_controller_t *controller,
    uint32_t now_ms, uint8_t *wire, size_t capacity,
    size_t *wire_length);

#endif
