/* ESP01-008: transport-neutral bounded worker runtime, source-only. */
#ifndef DEUS_ESP01_NODE_RUNTIME_H
#define DEUS_ESP01_NODE_RUNTIME_H
#include "deus_node_link.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
 dnl_worker_t worker;
 dnl_stream_t stream;
 uint32_t frames_received;
 uint32_t frames_replied;
 uint32_t invalid_or_partial_bytes;
 uint32_t tx_failed;
} deus_esp01_node_runtime_t;
void deus_esp01_node_runtime_init(deus_esp01_node_runtime_t *rt);
/* One RX byte at a time. DNL_OK only when one complete response is produced.
 * No asynchronous callback or SDK dependency. Fixed cap 40 bytes.
 * time_ms is monotonic modulo 2^32, per-byte.
 */
int deus_esp01_node_runtime_feed(deus_esp01_node_runtime_t *rt,
  uint8_t byte, uint32_t time_ms, uint8_t *reply, size_t capacity, size_t *reply_len);
#ifdef __cplusplus
}
#endif
#endif
