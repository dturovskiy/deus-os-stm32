/* Portable STM32 Wi-Fi-over-NodeLink controller, SOURCE ONLY.
 * Frozen DNL controller always owns canonical seq/session, no forked codec.
 * This unit never opens UART/USB, starts scans, resets devices or allocates.
 */
#include "kernel/node_wifi_client_v0.h"
#include <string.h>

static int wifi_opcode(uint8_t type)
{
    return type>=DEUS_WIFI_CMD_CAPS &&
           type<=DEUS_WIFI_CMD_SCAN_AP_PAGE;
}
static void clear_reply(deus_node_wifi_client_v0_t *wifi)
{
    wifi->has_last_reply=0u;
    wifi->remote_error=0u;
    memset(&wifi->last_reply,0,sizeof(wifi->last_reply));
}
static void fail_closed(deus_node_wifi_client_v0_t *wifi,
                        dnl_controller_t *c)
{
    clear_reply(wifi);
    c->pending=0u;
    c->ready=0u;
    c->request_length=0u;
    c->retry_count=0u;
}
void deus_node_wifi_client_v0_init(deus_node_wifi_client_v0_t *wifi)
{
    if (wifi) memset(wifi,0,sizeof(*wifi));
}
uint32_t deus_node_wifi_client_v0_pending_deadline(const dnl_controller_t *c)
{
    if (!c || !c->pending || !wifi_opcode(c->pending_type))
        return 0u;
    return c->pending_type==DEUS_WIFI_CMD_SCAN_START ?
           DEUS_NODE_WIFI_START_DEADLINE_MS :
           DEUS_NODE_WIFI_REQ_DEADLINE_MS;
}
int deus_node_wifi_client_v0_request(
    deus_node_wifi_client_v0_t *wifi, dnl_controller_t *c,
    uint8_t opcode,uint32_t generation,uint8_t ap_index,
    uint8_t ssid_offset,uint32_t now_ms,uint8_t *wire,
    size_t capacity,size_t *wire_length)
{
    dnl_frame_t request;
    size_t count=0u;
    int status;
    if (!wifi || !c || !wire || !wire_length)
        return DNL_BAD_ARGUMENT;
    *wire_length=0u;
    if (!wifi_opcode(opcode))
        return DNL_BAD_MESSAGE;
    if (!c->ready || c->pending || c->session_id==0u)
        return DNL_NOT_READY_OR_BUSY;
    if (c->last_sequence==UINT16_MAX)
        return DNL_REHANDSHAKE_REQUIRED;
    status=deus_wifi_ext_make_request_v0(
        opcode,(uint16_t)(c->last_sequence+1u),c->session_id,
        generation,ap_index,ssid_offset,&request);
    if (status!=DEUS_WIFI_EXT_OK)
        return DNL_BAD_MESSAGE;
    status=dnl_encode(&request,wire,capacity,&count);
    if (status!=DNL_OK)
        return status;
    /* This must be the SAME DNL controller used for CRC/PING, not a
     * separate sequence generator. Commit after encode, prior to UART TX,
     * even when TX later reports failure/uncertainty.
     */
    c->last_sequence=request.sequence;
    c->pending=1u;
    c->pending_type=opcode;
    c->retry_count=0u;
    c->requested_job_length=0u;
    c->request_length=(uint8_t)count;
    memcpy(c->request_bytes,wire,count);
    wifi->pending_request=request;
    wifi->started_ms=now_ms;
    clear_reply(wifi);
    *wire_length=count;
    return DNL_OK;
}
int deus_node_wifi_client_v0_receive(
    deus_node_wifi_client_v0_t *wifi,dnl_controller_t *c,
    const uint8_t *wire,size_t length,uint32_t now_ms)
{
    dnl_frame_t reply;
    deus_wifi_ext_result_v0_t result;
    int status;
    if (!wifi || !c || !wire)return DNL_BAD_ARGUMENT;
    if (!c->pending || !wifi_opcode(c->pending_type))
        return DNL_UNSOLICITED;
    if (dnl_decode(wire,length,&reply)!=DNL_OK)
        return DNL_BAD_FRAME;
    if (reply.session_id!=c->session_id ||
        reply.sequence!=c->last_sequence ||
        wifi->pending_request.session_id!=c->session_id ||
        wifi->pending_request.sequence!=c->last_sequence ||
        wifi->pending_request.type!=c->pending_type)
        return DNL_STALE_SESSION;
    /* An answer after the bounded deadline is never accepted, even
     * when the task-0 parser is delayed by OLED/USB work. The caller
     * must supply the timestamp of the LAST UART byte (not task time).
     */
    if ((uint32_t)(now_ms-wifi->started_ms)>
        deus_node_wifi_client_v0_pending_deadline(c))
        return DNL_DROP;
    status=deus_wifi_ext_read_response_v0(
        &wifi->pending_request,&reply,&result);
    if (status!=DEUS_WIFI_EXT_OK &&
        status!=DEUS_WIFI_EXT_REMOTE_ERROR)
        return DNL_BAD_MESSAGE;
    /* The legacy worker returns UN-SUPPORTED as a valid correlated
     * DNL_ERROR; this does not destroy the whole DNL session.
     */
    c->pending=0u;
    c->retry_count=0u;
    wifi->last_reply=result;
    wifi->received_ms=now_ms;
    wifi->has_last_reply=1u;
    wifi->remote_error=(status==DEUS_WIFI_EXT_REMOTE_ERROR)?
                        result.error:0u;
    if (status==DEUS_WIFI_EXT_REMOTE_ERROR) {
        ++wifi->total_remote_error;
        return DNL_REMOTE_ERROR;
    }
    ++wifi->total_success;
    return DNL_OK;
}
int deus_node_wifi_client_v0_timeout(
    deus_node_wifi_client_v0_t *wifi,dnl_controller_t *c,
    uint32_t now_ms,uint8_t *wire,size_t capacity,size_t *wire_length)
{
    uint32_t deadline;
    if (!wifi || !c || !wire || !wire_length)
        return DNL_BAD_ARGUMENT;
    *wire_length=0u;
    deadline=deus_node_wifi_client_v0_pending_deadline(c);
    if (!deadline) return DNL_UNSOLICITED;
    if ((uint32_t)(now_ms-wifi->started_ms)<=deadline)
        return DNL_DROP;
    ++wifi->total_timeouts;
    /* SCAN_START is an RF-side-effect command. A lost reply is ambiguous;
     * never replay, even byte-identically, across a possible reset.
     */
    if (c->pending_type==DEUS_WIFI_CMD_SCAN_START ||
        c->retry_count!=0u) {
        fail_closed(wifi,c);
        return DNL_REHANDSHAKE_REQUIRED;
    }
    if (capacity<c->request_length)
        return DNL_OUTPUT_TOO_SMALL;
    if (c->request_length==0u ||
        c->request_length>DNL_MAX_FRAME) {
        fail_closed(wifi,c);
        return DNL_REHANDSHAKE_REQUIRED;
    }
    memcpy(wire,c->request_bytes,c->request_length);
    *wire_length=c->request_length;
    c->retry_count=1u;
    wifi->started_ms=now_ms;
    return DNL_OK;
}
