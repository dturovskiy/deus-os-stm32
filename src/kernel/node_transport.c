/* ESP01-010: owner-private OFFLINE candidate. No boot auto-HELLO, no device reset.
 * IRQ38 publishes bytes+arrival times; task0 owns framing/CRC/controller/TX.
 */
#include <stdint.h>
#include <stddef.h>
#include "deus_node_link.h"
#include "drivers/usart2_node.h"
#include "kernel/node_transport.h"
/* Read SysTick AFTER a successful RX dequeue, not once at call entry. */
extern volatile uint32_t kernel_ticks;

#define NODE_SERVICE_MAX_LATENCY_MS 1000u
#define NODE_PING_PERIOD_MS 1000u
#define NODE_DRAIN_BUDGET USART2_NODE_RX_CAPACITY

static dnl_stream_t node_stream;
static dnl_controller_t node_controller;
static deus_node_wifi_client_v0_t node_wifi;
/* Task0-only exclusive USART2 owner; never shared with Node Link. */
static uint8_t node_maintenance_owned;
static uint32_t node_last_service_ms;
static uint32_t node_last_nonce;
static uint32_t node_last_ping_ms;
static uint32_t node_pending_sent_ms;
static uint32_t node_frames;
static uint32_t node_dropped;
static uint32_t node_restarts_required;
static uint32_t node_uart_loss;
/* Task0-only job result diagnostics; no heap and no IRQ-side controller. */
static uint32_t node_job_expected_crc32;
static uint32_t node_jobs_verified;
static uint32_t node_jobs_bad_crc;
static uint32_t node_last_job_crc32;
static uint8_t node_last_job_valid;

static void node_fail_closed(void)
{
    usart2_node_rx_disarm();
    dnl_stream_init(&node_stream);
    dnl_controller_init(&node_controller);
    node_wifi.has_last_reply=0u;
    node_wifi.remote_error=0u;
    node_wifi.pending_request.type=0u;
    node_last_job_valid = 0u;
    ++node_restarts_required;
}

/* No automatic retry with the same challenge after lost RX or session failure. */
void node_transport_init(uint32_t now_ms)
{
    dnl_stream_init(&node_stream);
    dnl_controller_init(&node_controller);
    deus_node_wifi_client_v0_init(&node_wifi);
    node_last_service_ms = now_ms;
    node_last_nonce = 0u;
    node_maintenance_owned = 0u;
    node_last_ping_ms = now_ms;
    node_pending_sent_ms = now_ms;
    node_frames = 0u;
    node_dropped = 0u;
    node_restarts_required = 0u;
    node_uart_loss = 0u;
    node_job_expected_crc32 = 0u;
    node_jobs_verified = 0u;
    node_jobs_bad_crc = 0u;
    node_last_job_crc32 = 0u;
    node_last_job_valid = 0u;
}

int node_transport_start(uint32_t fresh_nonce, uint32_t now_ms)
{
    uint8_t bytes[DNL_MAX_FRAME];
    size_t length = 0u;
    if (node_maintenance_owned != 0u ||
        fresh_nonce == 0u || fresh_nonce == node_last_nonce) return 0;
    node_fail_closed();
    if (dnl_controller_hello(&node_controller, fresh_nonce,
                             bytes, sizeof(bytes), &length) != DNL_OK) {
        node_fail_closed();
        return 0;
    }
    /* Remember the challenge before UART TX: a partial transmission might
     * already have reached the worker even when the bounded write fails. */
    node_last_nonce = fresh_nonce;
    /* RX must be armed BEFORE HELLO TX: an immediate ACK is legitimate. */
    node_last_service_ms = now_ms;
    if (length == 0u || length > DNL_MAX_FRAME) {
        node_fail_closed();
        return 0;
    }
    usart2_node_rx_arm();
    if (usart2_node_write_frame(bytes, (uint32_t)length) == 0) {
        node_fail_closed();
        return 0;
    }
    node_pending_sent_ms = now_ms;
    node_last_ping_ms = now_ms;
    return 1;
}

int node_transport_submit_crc32(const uint8_t *data, uint8_t count,
                                uint32_t now_ms)
{
    uint8_t payload[1u + DNL_MAX_JOB_BYTES];
    uint8_t bytes[DNL_MAX_FRAME];
    size_t length = 0u;
    uint32_t i;
    if (node_maintenance_owned != 0u ||
        data == (const uint8_t *)0 || count == 0u ||
        count > DNL_MAX_JOB_BYTES ||
        node_controller.ready == 0u ||
        node_controller.pending != 0u) return 0;
    payload[0] = count;
    for (i = 0u; i < count; ++i) payload[i + 1u] = data[i];
    if (dnl_controller_request(&node_controller, DNL_JOB_CRC32,
                               payload, (uint8_t)(count + 1u),
                               bytes, sizeof(bytes), &length) != DNL_OK ||
        length == 0u || length > DNL_MAX_FRAME) return 0;
    /* Record before TX. A partial write is unsafe to retry as a new job. */
    node_job_expected_crc32 = dnl_crc32_ieee(data, count);
    node_last_job_valid = 0u;
    node_pending_sent_ms = now_ms;
    if (usart2_node_write_frame(bytes, (uint32_t)length) == 0) {
        node_fail_closed();
        return 0;
    }
    return 1;
}

/* Explicit task0 command, never an automatic scan or HELLO.
 * The shared DNL controller's last_sequence/pending is advanced by the
 * optional codec so PING and CRC cannot reuse sequence numbers.
 */
int node_transport_wifi_request(uint8_t opcode,uint32_t generation,
                                uint8_t ap_index,uint8_t ssid_offset,
                                uint32_t now_ms)
{
    uint8_t packet[DNL_MAX_FRAME];
    size_t length=0u;
    if(node_maintenance_owned != 0u) return 0;
    if(deus_node_wifi_client_v0_request(
            &node_wifi,&node_controller,opcode,generation,ap_index,
            ssid_offset,now_ms,packet,sizeof(packet),&length)!=DNL_OK)
        return 0;
    node_pending_sent_ms=now_ms;
    if(length==0u || length>DNL_MAX_FRAME ||
       usart2_node_write_frame(packet,(uint32_t)length)==0) {
        /* The ESP could have received a partial request. Replaying an RF
         * side-effect command as a new sequence would be unsafe.
         */
        node_fail_closed();
        return 0;
    }
    return 1;
}

void node_transport_wifi_snapshot_get(node_transport_wifi_snapshot_t *out)
{
    if(!out)return;
    out->ready=node_controller.ready;
    out->pending=node_controller.pending;
    out->valid=node_wifi.has_last_reply;
    out->remote_error=node_wifi.remote_error;
    out->successes=node_wifi.total_success;
    out->remote_errors=node_wifi.total_remote_error;
    out->timeouts=node_wifi.total_timeouts;
    out->result=node_wifi.last_reply;
}

static void node_accept_frame(const dnl_frame_t *frame,
                              uint32_t last_byte_arrival_ms,
                              uint32_t now_ms)
{
    uint8_t raw[DNL_MAX_FRAME];
    size_t n = 0u;
    int status;
    uint8_t was_pending_wifi=(uint8_t)(
        deus_node_wifi_client_v0_pending_deadline(&node_controller)!=0u);

    /* Complete a reply by its *last byte arrival time*, not delayed task0
     * service time. Without this guard, a correctly CRC'd late HELLO_ACK
     * can incorrectly make the controller ready before retry timeout runs.
     * Reject stale/early frames with uint32 wrap-safe unsigned arithmetic;
     * the ingress timestamp has already passed the 1000 ms age bound.
     * This check is independent of the frozen DNL codec and controller.
     */
    if (node_controller.pending != 0u) {
        uint32_t deadline_ms = was_pending_wifi ?
            deus_node_wifi_client_v0_pending_deadline(&node_controller) :
            dnl_response_timeout_ms(node_controller.pending_type);
        if (deadline_ms != 0u &&
            (uint32_t)(last_byte_arrival_ms - node_pending_sent_ms) >
            deadline_ms) {
            ++node_dropped;
            return;
        }
    }

    /* The shared stream parser already checked CRC. Re-encode as exact
     * canonical wire bytes for the frozen controller's correlation API. */
    if (dnl_encode(frame, raw, sizeof(raw), &n) != DNL_OK) {
        ++node_dropped;
        return;
    }
    {
        uint8_t was_pending_job = (uint8_t)(
            node_controller.pending != 0u &&
            node_controller.pending_type == DNL_JOB_CRC32);
        if(was_pending_wifi)
            status=deus_node_wifi_client_v0_receive(
                &node_wifi,&node_controller,raw,n,last_byte_arrival_ms);
        else
            status=dnl_controller_receive(&node_controller,raw,n);
        if (status == DNL_OK && was_pending_job != 0u) {
            uint32_t actual;
            /* The frozen controller validates seq/session/length, not the
             * computed CRC32 value. Complete the end-to-end job check here. */
            if (frame->type != DNL_JOB_RESULT ||
                frame->payload_length != 5u) {
                ++node_jobs_bad_crc;
                node_fail_closed();
                return;
            }
            actual = (uint32_t)frame->payload[1] |
                ((uint32_t)frame->payload[2] << 8) |
                ((uint32_t)frame->payload[3] << 16) |
                ((uint32_t)frame->payload[4] << 24);
            if (actual != node_job_expected_crc32) {
                ++node_jobs_bad_crc;
                node_fail_closed();
                return;
            }
            ++node_jobs_verified;
            node_last_job_crc32 = actual;
            node_last_job_valid = 1u;
        }
    }
    if (status == DNL_OK) {
        ++node_frames;
        if (node_controller.pending == 0u &&
            node_controller.ready != 0u) node_last_ping_ms = now_ms;
    } else if (status == DNL_REMOTE_ERROR) {
        ++node_dropped;
        if(!was_pending_wifi)node_fail_closed();
    } else {
        ++node_dropped;
    }
}

void node_transport_service(uint32_t now_ms)
{
    usart2_node_sample_t sample;
    dnl_frame_t frame;
    uint32_t i;
    int rc;

    /* During maintenance USART2 RX belongs exclusively to the bridge.
     * No Node RX draining, PING generation, retry or controller mutation. */
    if (node_maintenance_owned != 0u) {
        node_last_service_ms = now_ms;
        return;
    }

    /* 16-bit ISR timestamps can only be unwrapped while task service
     * latency is bounded. On starvation, discard bytes without parsing. */
    if ((uint32_t)(now_ms - node_last_service_ms) >
        NODE_SERVICE_MAX_LATENCY_MS) {
        node_fail_closed();
        ++node_uart_loss;
        for (i = 0u; i <= NODE_DRAIN_BUDGET; ++i) {
            rc = usart2_node_try_read(&sample);
            if (rc <= 0) break;
        }
        node_last_service_ms = now_ms;
        return;
    }

    /* Drain RX and process valid replies before checking response deadlines:
     * a reply that arrived in time must not be timed out by task0/OLED delay. */
    for (i = 0u; i < NODE_DRAIN_BUDGET; ++i) {
        uint16_t age;
        uint32_t captured_ms;
        rc = usart2_node_try_read(&sample);
        if (rc == 0) break;
        if (rc < 0) {
            ++node_uart_loss;
            node_fail_closed();
            break;
        }
        /* The ISR stamped the consumed byte before this clock read.
         * Do not use the call-entry now_ms: SysTick/IRQ38 may have run. */
        now_ms = kernel_ticks;
        age = (uint16_t)((uint16_t)now_ms - sample.arrival_ms_low);
        if (age > NODE_SERVICE_MAX_LATENCY_MS) {
            ++node_uart_loss;
            node_fail_closed();
            break;
        }
        captured_ms = now_ms - (uint32_t)age;
        if (dnl_stream_feed(&node_stream, sample.byte,
                            captured_ms, &frame) == DNL_OK) {
            node_accept_frame(&frame, captured_ms, now_ms);
        }
    }
    node_last_service_ms = now_ms;

    if (node_stream.has_started != 0u &&
        (uint32_t)(now_ms - node_stream.started_at_ms) >
        DNL_PARTIAL_TIMEOUT_MS) {
        dnl_stream_init(&node_stream);
        ++node_dropped;
    }

    if (node_controller.pending != 0u) {
        uint32_t wifi_deadline =
            deus_node_wifi_client_v0_pending_deadline(&node_controller);
        uint32_t deadline = wifi_deadline ? wifi_deadline :
            dnl_response_timeout_ms(node_controller.pending_type);
        if (deadline != 0u &&
            (uint32_t)(now_ms - node_pending_sent_ms) > deadline) {
            uint8_t bytes[DNL_MAX_FRAME];
            size_t length = 0u;
            int retry=wifi_deadline ?
                deus_node_wifi_client_v0_timeout(
                    &node_wifi,&node_controller,now_ms,
                    bytes,sizeof(bytes),&length) :
                dnl_controller_timeout(&node_controller,bytes,
                                       sizeof(bytes),&length);
            if (retry==DNL_OK && length != 0u &&
                usart2_node_write_frame(bytes, (uint32_t)length) != 0) {
                node_pending_sent_ms=now_ms;
            } else {
                /* Lost SCAN_START acknowledgement is ambiguous; do NOT
                 * automatically replay or negotiate another HELLO.
                 */
                node_fail_closed();
            }
        }
    }

    if (node_controller.ready != 0u &&
        node_controller.pending == 0u &&
        (uint32_t)(now_ms - node_last_ping_ms) >= NODE_PING_PERIOD_MS) {
        uint8_t bytes[DNL_MAX_FRAME];
        size_t length = 0u;
        if (dnl_controller_request(&node_controller, DNL_PING,
                                   (const uint8_t *)0, 0u,
                                   bytes, sizeof(bytes), &length) == DNL_OK &&
            length != 0u &&
            usart2_node_write_frame(bytes, (uint32_t)length) != 0) {
            node_pending_sent_ms = now_ms;
            node_last_ping_ms = now_ms;
        } else {
            node_fail_closed();
        }
    }
}

/* Task0-only. Pending Node traffic may not be discarded during handover.
 * RX disarm flushes IRQ38 and the ring before the bridge can own bytes. */
int node_transport_maintenance_enter(void)
{
    if (node_maintenance_owned != 0u || node_controller.pending != 0u)
        return 0;
    usart2_node_rx_disarm();
    dnl_stream_init(&node_stream);
    dnl_controller_init(&node_controller);
    node_wifi.has_last_reply=0u;
    node_wifi.pending_request.type=0u;
    node_wifi.remote_error=0u;
    node_last_job_valid = 0u;
    node_maintenance_owned = 1u;
    return 1;
}

/* Never assert that ESP has returned to normal boot or that Node is READY. */
void node_transport_maintenance_leave(void)
{
    if (node_maintenance_owned == 0u) return;
    usart2_node_rx_disarm();
    dnl_stream_init(&node_stream);
    dnl_controller_init(&node_controller);
    node_wifi.has_last_reply=0u;
    node_wifi.pending_request.type=0u;
    node_wifi.remote_error=0u;
    node_last_job_valid = 0u;
    node_maintenance_owned = 0u;
}

void node_transport_diagnostics_get(node_transport_diagnostics_t *out)
{
    if (out == (node_transport_diagnostics_t *)0) return;
    out->frames = node_frames;
    out->dropped = node_dropped;
    out->restarts_required = node_restarts_required;
    out->uart_loss = node_uart_loss;
    out->last_service_ms = node_last_service_ms;
    out->jobs_verified = node_jobs_verified;
    out->jobs_bad_crc = node_jobs_bad_crc;
    out->last_job_crc32 = node_last_job_crc32;
    out->last_job_valid = node_last_job_valid;
    out->ready = node_controller.ready;
    out->pending = node_controller.pending;
}
