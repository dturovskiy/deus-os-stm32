/* Pure bounded wrapper: preserve frozen DNL worker state and wire rules. */
#include "wifi_node_dispatch_v0.h"
#include <string.h>

static int is_wifi(uint8_t type)
{
    return type>=DEUS_WIFI_CMD_CAPS &&
           type<=DEUS_WIFI_CMD_SCAN_AP_PAGE;
}
static uint32_t r32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) |
           ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static int ops_ready(const deus_wifi_node_dispatch_v0_t *rt)
{
    return rt->ops.start_scan && rt->ops.poll && rt->ops.snapshot;
}
static void reply_error(const dnl_frame_t *request, uint8_t err,
                        dnl_frame_t *reply)
{
    memset(reply,0,sizeof(*reply));
    reply->type=DNL_ERROR;
    reply->sequence=request->sequence;
    reply->session_id=request->session_id;
    reply->payload_length=2u;
    reply->payload[0]=err;
    reply->payload[1]=request->type;
}
void deus_wifi_node_dispatch_v0_init(deus_wifi_node_dispatch_v0_t *rt,
                                    const deus_wifi_node_ops_v0_t *ops)
{
    if (!rt) return;
    memset(rt,0,sizeof(*rt));
    dnl_worker_init(&rt->worker);
    dnl_stream_init(&rt->stream);
    if (ops) rt->ops=*ops;
}
int deus_wifi_node_dispatch_v0_process(deus_wifi_node_dispatch_v0_t *rt,
    const uint8_t *raw, size_t length, uint32_t now_ms,
    uint8_t *out, size_t cap, size_t *out_length)
{
    dnl_frame_t request,response;
    deus_wifi_scan_v0_t snapshot;
    dnl_worker_t *worker;
    size_t emitted=0u;
    uint16_t last_sequence;
    int rc;

    if (!rt || !raw || !out || !out_length)
        return DNL_BAD_ARGUMENT;
    *out_length=0u;
    if (dnl_decode(raw,length,&request)!=DNL_OK)
        return DNL_DROP;
    worker=&rt->worker;

    /* DNL_HELLO always belongs to the frozen worker. An identical HELLO
     * replay yields its already-cached ACK; a fresh challenge resets
     * worker sequence and Wi-Fi scan visibility, but NOT the RF device.
     */
    if (request.type==DNL_HELLO) {
        uint32_t previous=worker->session_id;
        rc=dnl_worker_process(worker,raw,length,out,cap,out_length);
        if (rc==DNL_OK && *out_length &&
            worker->session_id!=previous) {
            rt->owned_generation=0u;
        }
        return rc;
    }

    /* Missing ESP SDK callbacks = no advertised Wi-Fi capability.
     * Legacy worker rejects 0x05..0x08 with its UNSUPPORTED error and
     * retains exactly its own duplicate/sequence semantics.
     */
    if (!is_wifi(request.type) || !ops_ready(rt))
        return dnl_worker_process(worker,raw,length,out,cap,out_length);

    if (worker->session_id==0u ||
        request.session_id!=worker->session_id)
        return DNL_DROP;

    /* Same idempotent replay discipline as frozen DNL worker, shared
     * across legacy packets and new Wi-Fi packets. NEVER rerun RF work.
     */
    if (request.sequence==worker->highest_sequence &&
        worker->has_last) {
        if (length==worker->last_request_length &&
            memcmp(raw,worker->last_request,length)==0) {
            if (cap<worker->last_reply_length)
                return DNL_OUTPUT_TOO_SMALL;
            memcpy(out,worker->last_reply,worker->last_reply_length);
            *out_length=worker->last_reply_length;
            return DNL_OK;
        }
        reply_error(&request,DNL_ERR_BAD_SEQUENCE,&response);
        return dnl_encode(&response,out,cap,out_length);
    }
    if (request.sequence<=worker->highest_sequence)
        return DNL_DROP;
    last_sequence=worker->highest_sequence;
    if ((uint32_t)request.sequence!=(uint32_t)last_sequence+1u) {
        reply_error(&request,DNL_ERR_BAD_SEQUENCE,&response);
        return dnl_encode(&response,out,cap,out_length);
    }

    /* A single fixed-size reservation before invoking the SDK prevents
     * output-buffer failure AFTER a scan was started. Guard even short
     * CAPS/STATUS requests to make all Wi-Fi mutations transactional.
     */
    if (cap<DNL_MAX_FRAME)
        return DNL_OUTPUT_TOO_SMALL;

    memset(&snapshot,0,sizeof(snapshot));
    if (request.type==DEUS_WIFI_CMD_SCAN_START) {
        if (request.payload_length!=0u) {
            reply_error(&request,DNL_ERR_BAD_PAYLOAD,&response);
        } else {
            rt->ops.snapshot(rt->ops.context,&snapshot);
            if (snapshot.state==DEUS_WIFI_SCANNING) {
                reply_error(&request,DNL_ERR_BUSY,&response);
            } else if (!rt->ops.start_scan(rt->ops.context,now_ms)) {
                /* No auto-retries: an unsuccessful scan start consumes
                 * this sequence and its same-seq retries return cached
                 * errors rather than issuing a second radio command.
                 */
                reply_error(&request,DNL_ERR_NOT_READY,&response);
            } else {
                rt->ops.snapshot(rt->ops.context,&snapshot);
                if (deus_wifi_ext_make_start_ack_v0(
                        &request,&snapshot,&response)!=DEUS_WIFI_EXT_OK) {
                    reply_error(&request,DNL_ERR_NOT_READY,&response);
                } else {
                    rt->owned_generation=snapshot.generation;
                }
            }
        }
    } else if (request.type==DEUS_WIFI_CMD_SCAN_AP_PAGE &&
               request.payload_length==6u &&
               (rt->owned_generation==0u ||
                r32(request.payload)!=rt->owned_generation)) {
        reply_error(&request,DNL_ERR_NOT_READY,&response);
    } else {
        /* CAPS does not require radio startup; STATUS/PAGE are snapshots.
         * INVALID payloads receive frozen DNL error with same seq/type.
         */
        rt->ops.snapshot(rt->ops.context,&snapshot);
        rc=deus_wifi_ext_respond_snapshot_v0(
            &request,&snapshot,&response);
        if (rc==DEUS_WIFI_EXT_BAD_MESSAGE) {
            reply_error(&request,DNL_ERR_BAD_PAYLOAD,&response);
        } else if (rc!=DEUS_WIFI_EXT_OK) {
            reply_error(&request,DNL_ERR_NOT_READY,&response);
        }
    }

    rc=dnl_encode(&response,out,cap,&emitted);
    if (rc!=DNL_OK)
        return rc;

    /* Commit only after successful encode. Caller owns UART delivery;
     * byte-identical retries of an acknowledged transport failure MUST
     * return cache, not repeat non-idempotent SDK scan calls.
     */
    memcpy(worker->last_request,raw,length);
    worker->last_request_length=(uint8_t)length;
    memcpy(worker->last_reply,out,emitted);
    worker->last_reply_length=(uint8_t)emitted;
    worker->has_last=1u;
    worker->highest_sequence=request.sequence;
    *out_length=emitted;
    return DNL_OK;
}
int deus_wifi_node_dispatch_v0_feed(deus_wifi_node_dispatch_v0_t *rt,
    uint8_t byte,uint32_t now_ms,uint8_t *out,size_t cap,size_t *out_length)
{
    dnl_frame_t frame;
    uint8_t raw[DNL_MAX_FRAME];
    size_t len=0u;
    int rc;
    if (!rt || !out || !out_length)return DNL_BAD_ARGUMENT;
    *out_length=0u;
    rc=dnl_stream_feed(&rt->stream,byte,now_ms,&frame);
    if (rc!=DNL_OK) {
        if (rc==DNL_DROP && rt->invalid_bytes!=UINT32_MAX)
            ++rt->invalid_bytes;
        return rc;
    }
    if (rt->frames_received!=UINT32_MAX) ++rt->frames_received;
    rc=dnl_encode(&frame,raw,sizeof(raw),&len);
    if (rc!=DNL_OK) return rc;
    rc=deus_wifi_node_dispatch_v0_process(
        rt,raw,len,now_ms,out,cap,out_length);
    if (rc==DNL_OK && *out_length &&
        rt->frames_replied!=UINT32_MAX)
        ++rt->frames_replied;
    return rc;
}
void deus_wifi_node_dispatch_v0_poll(deus_wifi_node_dispatch_v0_t *rt,
                                     uint32_t now_ms)
{
    if (rt && rt->ops.poll)
        rt->ops.poll(rt->ops.context,now_ms);
}
