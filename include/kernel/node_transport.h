/* ESP01-010 STM32 task-0 transport candidate; OFFLINE build, no target Flash.
 * Session nonce must be provisioned fresh by a future admitted command path.
 * Never automatically reuse a hardcoded challenge after a reboot.
 */
#ifndef KERNEL_NODE_TRANSPORT_H
#define KERNEL_NODE_TRANSPORT_H
#include <stdint.h>
#include "kernel/node_wifi_client_v0.h"
void node_transport_init(uint32_t now_ms);
void node_transport_service(uint32_t now_ms);
/* Task0-only, explicitly requested ESP app reset: revoke old NodeLink
 * session and disarm RX. Refuses pending work or maintenance ownership. */
int node_transport_app_reset_begin(void);
/* Task context ONLY, session challenge nonzero and different from last local.
 * Cross-reboot uniqueness is the trusted caller's separate obligation.
 * Returns 1 if HELLO was queued/transmitted, otherwise 0.
 */
int node_transport_start(uint32_t fresh_nonce, uint32_t now_ms);
/* Submit 1..16 bytes only once, task0 only; CRC32 is independently verified
 * on receiving a matching DNL_JOB_RESULT. No arbitrary repeat of jobs.
 */
int node_transport_submit_crc32(const uint8_t *data, uint8_t count,
                                uint32_t now_ms);
/* Optional Wi-Fi over shared DNL REV2 controller.
 * Caller is USB/task0 only. Submitting SCAN_START MAY trigger real
 * ESP radio activity when a future WiFi-enabled app is installed;
 * installed ESP v2 responds UNSUPPORTED instead. No auto-HELLO/scan.
 */
int node_transport_wifi_request(uint8_t opcode, uint32_t generation,
                                uint8_t ap_index, uint8_t ssid_offset,
                                uint32_t now_ms);
typedef struct {
    uint8_t ready, pending, valid, remote_error;
    uint32_t successes, remote_errors, timeouts;
    deus_wifi_ext_result_v0_t result;
} node_transport_wifi_snapshot_t;
void node_transport_wifi_snapshot_get(node_transport_wifi_snapshot_t *out);
typedef struct {
    uint32_t frames;
    uint32_t dropped;
    uint32_t restarts_required;
    uint32_t uart_loss;
    uint32_t hello_acks;
    uint32_t hello_timeouts;
    uint32_t last_service_ms;
    uint32_t jobs_verified;
    uint32_t jobs_bad_crc;
    uint32_t last_job_crc32;
    uint8_t last_job_valid;
    uint8_t ready;
    uint8_t pending;
} node_transport_diagnostics_t;
void node_transport_diagnostics_get(node_transport_diagnostics_t *out);
/* Task0-only exclusive UART handover. Pending Node reply prevents entry.
 * Maintenance does not authorize ESP ROM mode, power or Flash.
 * After leaving, a fresh explicit Node HELLO is required. */
int node_transport_maintenance_enter(void);
void node_transport_maintenance_leave(void);
/* Passive task0 snapshot: never acquires/releases UART or services NodeLink. */
uint8_t node_transport_maintenance_is_owned(void);
#endif
