/* SOURCE-ONLY optional ESP Wi-Fi dispatcher; not in ESP v2 main/component.mk.
 * Frozen DNL REV2 worker is used unchanged for HELLO/PING/CRC/STATUS.
 * SDK callbacks run only in an application task, never in UART ISR.
 */
#ifndef DEUS_WIFI_NODE_DISPATCH_V0_H
#define DEUS_WIFI_NODE_DISPATCH_V0_H
#include "deus_node_link.h"
#include "wifi_node_extension_v0.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *context;
    /* Return 1 iff new on-demand nonblocking scan genuinely started. */
    int (*start_scan)(void *context, uint32_t now_ms);
    /* Called outside UART receive/IRQ; services SDK completion/timeouts. */
    void (*poll)(void *context, uint32_t now_ms);
    void (*snapshot)(void *context, deus_wifi_scan_v0_t *out);
} deus_wifi_node_ops_v0_t;

typedef struct {
    dnl_worker_t worker;     /* single authoritative session/seq/replay cache */
    dnl_stream_t stream;
    deus_wifi_node_ops_v0_t ops;
    /* Only the scan initiated in the present authenticated HELLO session
     * may provide AP pages. Never leak stale AP data after re-HELLO.
     */
    uint32_t owned_generation;
    uint32_t frames_received;
    uint32_t frames_replied;
    uint32_t invalid_bytes;
} deus_wifi_node_dispatch_v0_t;

/* Init before first UART receive; ops must be complete to advertise CAPS.
 * The caller must separately initialize the SDK scan service. */
void deus_wifi_node_dispatch_v0_init(deus_wifi_node_dispatch_v0_t *runtime,
                                   const deus_wifi_node_ops_v0_t *ops);

/* RAW validated envelope and application response: fixed buffers, no heap.
 * Returns DNL_DROP for invalid/stale/unsolicited, DNL_OK on exact reply.
 * Never calls physical I/O, UART, GPIO, Flash or credentials APIs.
 * CAN cause one explicit scan SDK callback for a new SCAN_START command.
 * Output capacity >=DNL_MAX_FRAME required for Wi-Fi scan admission.
 */
int deus_wifi_node_dispatch_v0_process(deus_wifi_node_dispatch_v0_t *runtime,
    const uint8_t *request, size_t request_length, uint32_t now_ms,
    uint8_t *reply, size_t capacity, size_t *reply_length);

/* Optional UART adapter entry, byte-by-byte REV2 stream reassembly. */
int deus_wifi_node_dispatch_v0_feed(deus_wifi_node_dispatch_v0_t *runtime,
    uint8_t byte, uint32_t now_ms,
    uint8_t *reply, size_t capacity, size_t *reply_length);

/* Must be invoked periodically from the ESP task, independent of UART RX;
 * forwarding callback is no-op if unconfigured. */
void deus_wifi_node_dispatch_v0_poll(deus_wifi_node_dispatch_v0_t *runtime,
                                     uint32_t now_ms);
#ifdef __cplusplus
}
#endif
#endif
