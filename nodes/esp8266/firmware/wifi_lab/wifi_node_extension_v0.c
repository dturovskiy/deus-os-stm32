/* Pure no-heap Node Link Wi-Fi extension codec; NOT in installed firmware.
 * Snapshot-only; never initiates scans, traffic, UART, RF or power changes.
 */
#include "wifi_node_extension_v0.h"
#include <string.h>

static void wr32(uint8_t *p, uint32_t v)
{
    p[0]=(uint8_t)v;
    p[1]=(uint8_t)(v>>8);
    p[2]=(uint8_t)(v>>16);
    p[3]=(uint8_t)(v>>24);
}
static uint32_t rd32(const uint8_t *p)
{
    return ((uint32_t)p[0]) | ((uint32_t)p[1]<<8) |
           ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static int command_known(uint8_t command)
{
    return command>=DEUS_WIFI_CMD_CAPS &&
           command<=DEUS_WIFI_CMD_SCAN_AP_PAGE;
}
static int request_valid(const dnl_frame_t *r)
{
    if (r==0 || r->session_id==0u || r->sequence==0u ||
        !command_known(r->type)) return 0;
    if (r->type!=DEUS_WIFI_CMD_SCAN_AP_PAGE)
        return r->payload_length==0u;
    return r->payload_length==6u &&
           rd32(r->payload)!=0u &&
           r->payload[4]<DEUS_WIFI_EXT_MAX_APS &&
           r->payload[5]<=DEUS_WIFI_EXT_MAX_SSID;
}
static int snapshot_valid(const deus_wifi_scan_v0_t *s)
{
    /* DEUS_WIFI_OFF is zero; enum state cannot be negative on the
     * supported target, so a redundant lower bound triggers GCC 15
     * -Wtype-limits under the required -Werror toolchain. Keep the
     * meaningful upper-bound and count/truncation checks unchanged. */
    return s!=0 && s->state<=DEUS_WIFI_ERROR &&
           s->count<=DEUS_WIFI_EXT_MAX_APS && s->truncated<=1u;
}
static void prepare_reply(const dnl_frame_t *request,
                          uint8_t response_type, dnl_frame_t *reply)
{
    memset(reply,0,sizeof(*reply));
    reply->type=response_type;
    reply->sequence=request->sequence;
    reply->session_id=request->session_id;
}
static void write_status(const deus_wifi_scan_v0_t *scan,
                         dnl_frame_t *reply)
{
    reply->payload_length=DEUS_WIFI_EXT_STATUS_BYTES;
    wr32(reply->payload,scan->generation);
    reply->payload[4]=(uint8_t)scan->state;
    reply->payload[5]=scan->count;
    reply->payload[6]=scan->truncated;
    wr32(reply->payload+7,(uint32_t)scan->last_error);
}
static void error_reply(const dnl_frame_t *request, uint8_t code,
                        dnl_frame_t *response)
{
    prepare_reply(request,DNL_ERROR,response);
    response->payload_length=2u;
    response->payload[0]=code;
    response->payload[1]=request->type;
}
int deus_wifi_ext_make_request_v0(uint8_t command,uint16_t sequence,
    uint32_t session_id,uint32_t generation,uint8_t index,uint8_t offset,
    dnl_frame_t *out)
{
    dnl_frame_t next;
    if (out==0) return DEUS_WIFI_EXT_BAD_ARGUMENT;
    if (!command_known(command) || sequence==0u || session_id==0u)
        return DEUS_WIFI_EXT_BAD_MESSAGE;
    if (command==DEUS_WIFI_CMD_SCAN_AP_PAGE) {
        if (generation==0u || index>=DEUS_WIFI_EXT_MAX_APS ||
            offset>DEUS_WIFI_EXT_MAX_SSID)
            return DEUS_WIFI_EXT_BAD_MESSAGE;
    } else if (generation!=0u || index!=0u || offset!=0u) {
        return DEUS_WIFI_EXT_BAD_MESSAGE;
    }
    memset(&next,0,sizeof(next));
    next.type=command;
    next.sequence=sequence;
    next.session_id=session_id;
    if (command==DEUS_WIFI_CMD_SCAN_AP_PAGE) {
        next.payload_length=6u;
        wr32(next.payload,generation);
        next.payload[4]=index;
        next.payload[5]=offset;
    }
    *out=next;
    return DEUS_WIFI_EXT_OK;
}
int deus_wifi_ext_respond_snapshot_v0(const dnl_frame_t *request,
    const deus_wifi_scan_v0_t *snapshot, dnl_frame_t *out)
{
    dnl_frame_t reply;
    uint8_t index,offset,length,page_length;
    if (request==0 || out==0)return DEUS_WIFI_EXT_BAD_ARGUMENT;
    if (!request_valid(request))return DEUS_WIFI_EXT_BAD_MESSAGE;
    if (request->type==DEUS_WIFI_CMD_SCAN_START)
        return DEUS_WIFI_EXT_ACTION_REQUIRED;
    if (request->type!=DEUS_WIFI_CMD_CAPS && !snapshot_valid(snapshot))
        return DEUS_WIFI_EXT_BAD_ARGUMENT;

    prepare_reply(request,(uint8_t)(request->type|0x80u),&reply);
    if (request->type==DEUS_WIFI_CMD_CAPS) {
        reply.payload_length=8u;
        reply.payload[0]=DEUS_WIFI_EXT_MAJOR;
        reply.payload[1]=DEUS_WIFI_EXT_MINOR;
        reply.payload[2]=1u; /* on-demand async scan, no join/credentials */
        reply.payload[3]=DEUS_WIFI_EXT_MAX_APS;
        reply.payload[4]=DEUS_WIFI_EXT_MAX_SSID;
        reply.payload[5]=DEUS_WIFI_EXT_PAGE_MAX;
        reply.payload[6]=DNL_MAX_FRAME;
        reply.payload[7]=0u;
    } else if (request->type==DEUS_WIFI_CMD_SCAN_STATUS) {
        write_status(snapshot,&reply);
    } else {
        if (snapshot->state==DEUS_WIFI_SCANNING) {
            error_reply(request,DNL_ERR_BUSY,&reply);
        } else if (snapshot->state!=DEUS_WIFI_RESULTS) {
            error_reply(request,DNL_ERR_NOT_READY,&reply);
        } else if (rd32(request->payload)!=snapshot->generation ||
                   snapshot->generation==0u ||
                   request->payload[4]>=snapshot->count) {
            error_reply(request,DNL_ERR_BAD_PAYLOAD,&reply);
        } else {
            const deus_wifi_ap_v0_t *ap;
            uint8_t i;
            index=request->payload[4];
            offset=request->payload[5];
            ap=&snapshot->aps[index];
            /* SDK's 33-byte NUL-padded SSID field offers no separate
             * binary length: first NUL terminates, or 32 bytes max.
             * Arbitrary non-UTF8 bytes before NUL pass through as-is.
             */
            length=0u;
            while (length<DEUS_WIFI_EXT_MAX_SSID &&
                   ap->ssid[length]!=0u) ++length;
            if (offset>length) {
                error_reply(request,DNL_ERR_BAD_PAYLOAD,&reply);
            } else {
                page_length=(uint8_t)(length-offset);
                if (page_length>DEUS_WIFI_EXT_PAGE_MAX)
                    page_length=DEUS_WIFI_EXT_PAGE_MAX;
                reply.payload_length=(uint8_t)
                    (DEUS_WIFI_EXT_PAGE_HEADER+page_length);
                wr32(reply.payload,snapshot->generation);
                reply.payload[4]=index;
                reply.payload[5]=offset;
                reply.payload[6]=length;
                reply.payload[7]=ap->channel;
                reply.payload[8]=(uint8_t)ap->rssi_dbm;
                reply.payload[9]=ap->authmode;
                for (i=0u;i<page_length;++i)
                    reply.payload[DEUS_WIFI_EXT_PAGE_HEADER+i]=
                        ap->ssid[offset+i];
            }
        }
    }
    *out=reply;
    return DEUS_WIFI_EXT_OK;
}
int deus_wifi_ext_make_start_ack_v0(const dnl_frame_t *request,
    const deus_wifi_scan_v0_t *started_scan,dnl_frame_t *out)
{
    dnl_frame_t next;
    if (!request || !out || !started_scan)
        return DEUS_WIFI_EXT_BAD_ARGUMENT;
    if (!request_valid(request) ||
        request->type!=DEUS_WIFI_CMD_SCAN_START)
        return DEUS_WIFI_EXT_BAD_MESSAGE;
    /* This is an ACK builder after a separate explicit SDK scan_start
     * succeeded. No scan is started in this pure function. */
    if (!snapshot_valid(started_scan) ||
        started_scan->state!=DEUS_WIFI_SCANNING ||
        started_scan->generation==0u)
        return DEUS_WIFI_EXT_BAD_MESSAGE;
    prepare_reply(request,DEUS_WIFI_REPLY_SCAN_START,&next);
    write_status(started_scan,&next);
    *out=next;
    return DEUS_WIFI_EXT_OK;
}
int deus_wifi_ext_read_response_v0(const dnl_frame_t *request,
    const dnl_frame_t *reply,deus_wifi_ext_result_v0_t *out)
{
    deus_wifi_ext_result_v0_t next;
    uint8_t remaining,expected;
    if (!request || !reply || !out)return DEUS_WIFI_EXT_BAD_ARGUMENT;
    if (!request_valid(request))return DEUS_WIFI_EXT_BAD_MESSAGE;
    if (request->session_id!=reply->session_id ||
        request->sequence!=reply->sequence ||
        reply->payload_length>DNL_MAX_PAYLOAD)
        return DEUS_WIFI_EXT_INCOMPATIBLE;
    memset(&next,0,sizeof(next));
    next.type=reply->type;
    if (reply->type==DNL_ERROR) {
        if (dnl_validate_payload(reply)!=DNL_OK ||
            reply->payload[1]!=request->type)
            return DEUS_WIFI_EXT_INCOMPATIBLE;
        next.error=reply->payload[0];
        *out=next;
        return DEUS_WIFI_EXT_REMOTE_ERROR;
    }
    if (reply->type!=(uint8_t)(request->type|0x80u))
        return DEUS_WIFI_EXT_INCOMPATIBLE;
    if (reply->type==DEUS_WIFI_REPLY_CAPS) {
        if (reply->payload_length!=8u ||
            reply->payload[0]!=DEUS_WIFI_EXT_MAJOR ||
            reply->payload[1]!=DEUS_WIFI_EXT_MINOR ||
            reply->payload[2]!=1u ||
            reply->payload[3]!=DEUS_WIFI_EXT_MAX_APS ||
            reply->payload[4]!=DEUS_WIFI_EXT_MAX_SSID ||
            reply->payload[5]!=DEUS_WIFI_EXT_PAGE_MAX ||
            reply->payload[6]!=DNL_MAX_FRAME ||
            reply->payload[7]!=0u)
            return DEUS_WIFI_EXT_INCOMPATIBLE;
        next.major=reply->payload[0];
        next.minor=reply->payload[1];
        next.flags=reply->payload[2];
        next.max_aps=reply->payload[3];
        next.max_ssid=reply->payload[4];
        next.max_page=reply->payload[5];
    } else if (reply->type==DEUS_WIFI_REPLY_SCAN_START ||
               reply->type==DEUS_WIFI_REPLY_SCAN_STATUS) {
        if (reply->payload_length!=DEUS_WIFI_EXT_STATUS_BYTES ||
            reply->payload[4]>DEUS_WIFI_ERROR ||
            reply->payload[5]>DEUS_WIFI_EXT_MAX_APS ||
            reply->payload[6]>1u ||
            (reply->type==DEUS_WIFI_REPLY_SCAN_START &&
             (reply->payload[4]!=DEUS_WIFI_SCANNING ||
              rd32(reply->payload)==0u)))
            return DEUS_WIFI_EXT_INCOMPATIBLE;
        next.generation=rd32(reply->payload);
        next.state=reply->payload[4];
        next.count=reply->payload[5];
        next.truncated=reply->payload[6];
        next.last_error=(int32_t)rd32(reply->payload+7);
    } else if (reply->type==DEUS_WIFI_REPLY_SCAN_AP_PAGE) {
        if (reply->payload_length<DEUS_WIFI_EXT_PAGE_HEADER ||
            reply->payload_length>DNL_MAX_PAYLOAD ||
            rd32(reply->payload)!=rd32(request->payload) ||
            reply->payload[4]!=request->payload[4] ||
            reply->payload[5]!=request->payload[5] ||
            reply->payload[6]>DEUS_WIFI_EXT_MAX_SSID ||
            reply->payload[5]>reply->payload[6])
            return DEUS_WIFI_EXT_INCOMPATIBLE;
        remaining=(uint8_t)(reply->payload[6]-reply->payload[5]);
        expected=remaining<DEUS_WIFI_EXT_PAGE_MAX ?
                 remaining:DEUS_WIFI_EXT_PAGE_MAX;
        if (reply->payload_length!=
                (uint8_t)(DEUS_WIFI_EXT_PAGE_HEADER+expected))
            return DEUS_WIFI_EXT_INCOMPATIBLE;
        next.generation=rd32(reply->payload);
        next.ap_index=reply->payload[4];
        next.ssid_offset=reply->payload[5];
        next.ssid_length=reply->payload[6];
        next.channel=reply->payload[7];
        next.rssi_dbm=(int8_t)reply->payload[8];
        next.authmode=reply->payload[9];
        next.page_length=expected;
        if (expected!=0u)
            memcpy(next.page,reply->payload+DEUS_WIFI_EXT_PAGE_HEADER,
                   expected);
    } else {
        return DEUS_WIFI_EXT_INCOMPATIBLE;
    }
    *out=next;
    return DEUS_WIFI_EXT_OK;
}
