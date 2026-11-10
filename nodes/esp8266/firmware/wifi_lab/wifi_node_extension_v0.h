/* Deus Node Link Wi-Fi scan extension V0 — SOURCE-ONLY protocol proposal.
 * Does NOT modify/install frozen DNL REV2. Existing v2 ESP worker and
 * installed STM32 normal9 do NOT dispatch types 0x05..0x08 yet.
 * No SDK, I/O, credentials, heap, clocks or persistent storage here.
 */
#ifndef DEUS_WIFI_NODE_EXTENSION_V0_H
#define DEUS_WIFI_NODE_EXTENSION_V0_H
#include <stdint.h>
#include "deus_node_link.h"
#include "wifi_scan_v0.h"

#ifdef __cplusplus
extern "C" {
#endif

/* New optional feature is discovered by a request that old DNL workers
 * reject with DNL_ERROR / DNL_ERR_UNSUPPORTED. No silent capability claim.
 */
#define DEUS_WIFI_EXT_MAJOR 1u
#define DEUS_WIFI_EXT_MINOR 0u
#define DEUS_WIFI_EXT_PAGE_MAX 14u
#define DEUS_WIFI_EXT_MAX_APS DEUS_ESP_WIFI_SCAN_MAX_APS
#define DEUS_WIFI_EXT_MAX_SSID DEUS_ESP_WIFI_SCAN_SSID_MAX
#define DEUS_WIFI_EXT_STATUS_BYTES 11u
#define DEUS_WIFI_EXT_PAGE_HEADER 10u

enum {
    DEUS_WIFI_CMD_CAPS=0x05,
    DEUS_WIFI_CMD_SCAN_START=0x06,
    DEUS_WIFI_CMD_SCAN_STATUS=0x07,
    DEUS_WIFI_CMD_SCAN_AP_PAGE=0x08,
    DEUS_WIFI_REPLY_CAPS=0x85,
    DEUS_WIFI_REPLY_SCAN_START=0x86,
    DEUS_WIFI_REPLY_SCAN_STATUS=0x87,
    DEUS_WIFI_REPLY_SCAN_AP_PAGE=0x88
};
enum {
    DEUS_WIFI_EXT_OK=0,
    DEUS_WIFI_EXT_BAD_ARGUMENT=1,
    DEUS_WIFI_EXT_BAD_MESSAGE=2,
    DEUS_WIFI_EXT_ACTION_REQUIRED=3, /* SCAN_START cannot start SDK here */
    DEUS_WIFI_EXT_REMOTE_ERROR=4,
    DEUS_WIFI_EXT_INCOMPATIBLE=5
};

typedef struct {
    uint8_t type;                /* actual response type */
    uint8_t error;               /* DNL_ERR_* when REMOTE_ERROR */
    uint8_t major, minor, flags, max_aps, max_ssid, max_page;
    uint32_t generation;
    uint8_t state, count, truncated;
    int32_t last_error;
    uint8_t ap_index, ssid_offset, ssid_length, page_length;
    uint8_t channel, authmode;
    int8_t rssi_dbm;
    uint8_t page[DEUS_WIFI_EXT_PAGE_MAX];
} deus_wifi_ext_result_v0_t;

/* Caller owns negotiated session_id, monotonic sequence, and timeout.
 * PAGE uses 32-bit generation, AP index and byte offset; no NUL/UTF8
 * interpretation is done in framing.
 */
int deus_wifi_ext_make_request_v0(uint8_t command, uint16_t sequence,
    uint32_t session_id, uint32_t generation, uint8_t index, uint8_t offset,
    dnl_frame_t *out);

/* Stateless snapshot response factory. SCAN_START returns ACTION_REQUIRED;
 * caller MUST explicitly call the SDK scan service outside this codec, then
 * use make_start_ack on a successful DEUS_WIFI_SCANNING snapshot.
 * Session/order/duplicate handling is the caller's responsibility; the
 * frozen DNL worker has not been extended and still rejects new requests.
 */
int deus_wifi_ext_respond_snapshot_v0(const dnl_frame_t *request,
    const deus_wifi_scan_v0_t *snapshot, dnl_frame_t *out);
int deus_wifi_ext_make_start_ack_v0(const dnl_frame_t *request,
    const deus_wifi_scan_v0_t *started_scan, dnl_frame_t *out);

/* Validates type, sequence, session, length and page correlation; output
 * untouched on any error. For DNL_ERROR returns REMOTE_ERROR and .error.
 * No unbounded text parsing, heap or network access.
 */
int deus_wifi_ext_read_response_v0(const dnl_frame_t *request,
    const dnl_frame_t *reply, deus_wifi_ext_result_v0_t *out);
#ifdef __cplusplus
}
#endif
#endif
