/* Full mock SDK + frozen DNL REV2 + Wi-Fi-aware dispatch integration.
 * All AP records synthetic, no real RF, credentials or hardware access.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "deus_node_link.h"
#include "wifi_node_dispatch_v0.h"
#include "wifi_node_sdk_bridge_v0.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"

#define OK(x) do { if (!(x)) { \
    fprintf(stderr,"WIFI_DISPATCH_FAIL line %d: %s\n",__LINE__,#x); \
    exit(2); } } while (0)

static system_event_handler_t event_handler;
static void *event_ctx;
static wifi_ap_record_t records[6];
static uint16_t total_aps;
static unsigned init_calls, radio_calls, scan_calls, stopped_scans;
static unsigned get_records_calls, fail_at;
static unsigned scan_nonblocking;
static deus_wifi_node_dispatch_v0_t runtime;
static deus_wifi_node_ops_v0_t ops;

/* SDK v3.4 call stubs: bounded no-device stand-ins. */
void tcpip_adapter_init(void) { ++init_calls; }
esp_err_t esp_event_loop_init(system_event_handler_t f,void *ctx)
{
    event_handler=f;event_ctx=ctx;return ESP_OK;
}
esp_err_t esp_wifi_init(const wifi_init_config_t *c)
{
    OK(c && c->magic==777);
    ++init_calls;return fail_at==1?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_set_storage(int n)
{
    OK(n==WIFI_STORAGE_RAM);return ESP_OK;
}
esp_err_t esp_wifi_set_mode(int n)
{
    OK(n==WIFI_MODE_STA);return ESP_OK;
}
esp_err_t esp_wifi_start(void)
{
    ++radio_calls;return fail_at==2?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *c,bool block)
{
    OK(c==0 && block==false);
    ++scan_calls;scan_nonblocking=1u;
    return fail_at==3?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_scan_stop(void)
{
    ++stopped_scans;return ESP_OK;
}
esp_err_t esp_wifi_scan_get_ap_num(uint16_t *out)
{
    OK(out);
    if(fail_at==4)return ESP_FAIL;
    *out=total_aps;return ESP_OK;
}
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *count,
                                      wifi_ap_record_t *out)
{
    uint16_t i;
    ++get_records_calls;
    if(fail_at==5)return ESP_FAIL;
    OK(count&&out && *count<=4u);
    for(i=0u;i<*count;++i)out[i]=records[i];
    return ESP_OK;
}
static void reset_all(void)
{
    memset(records,0,sizeof(records));
    event_handler=0;event_ctx=0;total_aps=0u;
    init_calls=radio_calls=scan_calls=stopped_scans=0u;
    get_records_calls=fail_at=scan_nonblocking=0u;
    deus_wifi_node_sdk_bridge_v0_init();
    ops=deus_wifi_node_sdk_bridge_v0_ops();
    deus_wifi_node_dispatch_v0_init(&runtime,&ops);
}
static void scan_done(void)
{
    system_event_t event={0};
    event.event_id=SYSTEM_EVENT_SCAN_DONE;
    OK(event_handler!=0);
    OK(event_handler(event_ctx,&event)==ESP_OK);
}
static dnl_frame_t frame_from(const uint8_t *bytes,size_t n)
{
    dnl_frame_t f;
    memset(&f,0,sizeof(f));
    OK(dnl_decode(bytes,n,&f)==DNL_OK);
    return f;
}
static dnl_frame_t send_req(const dnl_frame_t *req, uint32_t now_ms)
{
    uint8_t raw[DNL_MAX_FRAME],reply[DNL_MAX_FRAME];
    size_t inlen=0u,outlen=0u;
    OK(dnl_encode(req,raw,sizeof(raw),&inlen)==DNL_OK);
    OK(deus_wifi_node_dispatch_v0_process(&runtime,raw,inlen,now_ms,
         reply,sizeof(reply),&outlen)==DNL_OK);
    OK(outlen>=16u);
    return frame_from(reply,outlen);
}
static dnl_frame_t wifi_request(uint8_t type,uint16_t seq,uint32_t sess,
                                uint32_t gen,uint8_t index,uint8_t offset)
{
    dnl_frame_t f;
    OK(deus_wifi_ext_make_request_v0(type,seq,sess,gen,index,offset,
                                     &f)==DEUS_WIFI_EXT_OK);
    return f;
}
static dnl_frame_t hello(uint32_t challenge)
{
    dnl_frame_t req;
    memset(&req,0,sizeof(req));
    req.type=DNL_HELLO;req.sequence=1u;req.payload_length=4u;
    req.payload[0]=(uint8_t)challenge;
    req.payload[1]=(uint8_t)(challenge>>8);
    req.payload[2]=(uint8_t)(challenge>>16);
    req.payload[3]=(uint8_t)(challenge>>24);
    return req;
}
static void expect_error(const dnl_frame_t *req,uint8_t err,uint32_t now)
{
    dnl_frame_t ans=send_req(req,now);
    deus_wifi_ext_result_v0_t decoded;
    OK(ans.type==DNL_ERROR && ans.payload_length==2u);
    OK(ans.payload[0]==err && ans.payload[1]==req->type);
    OK(deus_wifi_ext_read_response_v0(req,&ans,&decoded)==
       DEUS_WIFI_EXT_REMOTE_ERROR);
    OK(decoded.error==err);
}
static deus_wifi_ext_result_v0_t expect_wifi_reply(
    const dnl_frame_t *req,uint32_t now)
{
    deus_wifi_ext_result_v0_t out;
    dnl_frame_t reply=send_req(req,now);
    OK(deus_wifi_ext_read_response_v0(req,&reply,&out)==
       DEUS_WIFI_EXT_OK);
    return out;
}
int main(void)
{
    const uint32_t SESSION_A=0xABCDEF42u,SESSION_B=0x1234ABCDu;
    dnl_frame_t req,ans,temp;
    deus_wifi_ext_result_v0_t result;
    uint8_t buf[DNL_MAX_FRAME],backup[DNL_MAX_FRAME];
    size_t len=0u,n=0u;
    uint8_t ssid[32];
    uint8_t offsets[3]={0u,14u,28u};
    unsigned j;

    reset_all();
    OK(init_calls==0u && scan_calls==0u && radio_calls==0u);
    req=wifi_request(DEUS_WIFI_CMD_CAPS,1u,SESSION_A,0u,0u,0u);
    OK(dnl_encode(&req,buf,sizeof(buf),&n)==DNL_OK);
    OK(deus_wifi_node_dispatch_v0_process(&runtime,buf,n,1u,
         backup,sizeof(backup),&len)==DNL_DROP);
    OK(scan_calls==0u); /* no session is never admitted */

    req=hello(SESSION_A);ans=send_req(&req,10u);
    OK(ans.type==DNL_HELLO_ACK && ans.session_id==SESSION_A);
    OK(runtime.worker.highest_sequence==1u);
    req=wifi_request(DEUS_WIFI_CMD_CAPS,2u,SESSION_A,0u,0u,0u);
    result=expect_wifi_reply(&req,20u);
    OK(result.max_aps==4u && result.max_ssid==32u);
    OK(result.max_page==14u && result.major==1u && result.flags==1u);
    OK(init_calls==0u && radio_calls==0u && scan_calls==0u);

    /* Frozen DNL PING shares sequence/cache with optional WiFi. */
    memset(&req,0,sizeof(req));
    req.type=DNL_PING;req.sequence=3u;req.session_id=SESSION_A;
    ans=send_req(&req,30u);
    OK(ans.type==DNL_PONG && runtime.worker.highest_sequence==3u);

    req=wifi_request(DEUS_WIFI_CMD_SCAN_START,4u,SESSION_A,0u,0u,0u);
    OK(dnl_encode(&req,buf,sizeof(buf),&n)==DNL_OK);
    OK(deus_wifi_node_dispatch_v0_process(&runtime,buf,n,40u,
         backup,DNL_MAX_FRAME-1u,&len)==DNL_OUTPUT_TOO_SMALL);
    OK(scan_calls==0u && radio_calls==0u);
    result=expect_wifi_reply(&req,41u);
    OK(result.state==DEUS_WIFI_SCANNING && result.generation==1u);
    OK(scan_calls==1u && radio_calls==1u && scan_nonblocking==1u);
    OK(runtime.owned_generation==1u);
    /* Duplicate EXACT request returns cached response and no new radio scan. */
    result=expect_wifi_reply(&req,42u);
    OK(result.generation==1u && scan_calls==1u && radio_calls==1u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,4u,SESSION_A,0u,0u,0u);
    expect_error(&temp,DNL_ERR_BAD_SEQUENCE,43u);
    OK(runtime.worker.highest_sequence==4u);

    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,6u,SESSION_A,0u,0u,0u);
    expect_error(&temp,DNL_ERR_BAD_SEQUENCE,44u);
    OK(runtime.worker.highest_sequence==4u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,5u,SESSION_B,0u,0u,0u);
    OK(dnl_encode(&temp,buf,sizeof(buf),&n)==DNL_OK);
    OK(deus_wifi_node_dispatch_v0_process(&runtime,buf,n,45u,
         backup,sizeof(backup),&len)==DNL_DROP);

    /* BUSY scans don't corrupt sequence or introduce a second call. */
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_START,5u,SESSION_A,0u,0u,0u);
    expect_error(&temp,DNL_ERR_BUSY,46u);
    expect_error(&temp,DNL_ERR_BUSY,47u);
    OK(scan_calls==1u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,6u,SESSION_A,1u,0u,0u);
    expect_error(&temp,DNL_ERR_BUSY,48u);

    /* Real SDK mock event queues result; periodic task polling finalizes. */
    total_aps=6u;
    for(j=0u;j<6u;++j) {
        unsigned k;
        for(k=0u;k<32u;++k)
            records[j].ssid[k]=(uint8_t)('a'+(k%26u));
        records[j].primary=(uint8_t)(j+1u);
        records[j].rssi=(int8_t)(-40-(int)j);
        records[j].authmode=(int)(j%3u);
    }
    scan_done();
    deus_wifi_node_dispatch_v0_poll(&runtime,51u);
    OK(get_records_calls==1u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,7u,SESSION_A,0u,0u,0u);
    result=expect_wifi_reply(&temp,52u);
    OK(result.state==DEUS_WIFI_RESULTS);
    OK(result.generation==1u && result.count==4u && result.truncated==1u);

    for(j=0;j<3u;++j) {
        temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,
                          (uint16_t)(8u+j),SESSION_A,1u,0u,offsets[j]);
        result=expect_wifi_reply(&temp,55u+j);
        OK(result.ssid_offset==offsets[j]);
        OK(result.ssid_length==32u);
        OK(result.page_length==(j==2u?4u:14u));
        memcpy(ssid+offsets[j],result.page,result.page_length);
    }
    OK(memcmp(ssid,records[0].ssid,sizeof(ssid))==0);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,11u,
                      SESSION_A,1u,0u,32u);
    result=expect_wifi_reply(&temp,59u);
    OK(result.page_length==0u && result.ssid_length==32u);

    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,12u,
                      SESSION_A,3u,0u,0u);
    expect_error(&temp,DNL_ERR_NOT_READY,60u); /* stale scan generation */
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_START,13u,SESSION_A,0u,0u,0u);
    result=expect_wifi_reply(&temp,61u);
    OK(result.state==DEUS_WIFI_SCANNING && result.generation==2u);
    OK(scan_calls==2u && radio_calls==1u);
    result=expect_wifi_reply(&temp,62u);
    OK(result.generation==2u && scan_calls==2u);

    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,14u,
                      SESSION_A,1u,0u,0u);
    expect_error(&temp,DNL_ERR_NOT_READY,63u);
    OK(runtime.worker.highest_sequence==14u);

    /* New HELLO resets WiFi AP visibility, not global SDK state.
     * A fresh session cannot page an older scan via guessed generation.
     */
    req=hello(SESSION_B);ans=send_req(&req,65u);
    OK(ans.type==DNL_HELLO_ACK && ans.session_id==SESSION_B);
    OK(runtime.owned_generation==0u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,2u,
                      SESSION_B,2u,0u,0u);
    expect_error(&temp,DNL_ERR_NOT_READY,66u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_START,3u,
                      SESSION_B,0u,0u,0u);
    expect_error(&temp,DNL_ERR_BUSY,67u);
    OK(scan_calls==2u);

    /* Previous SDK async scan completes; new session still cannot page. */
    total_aps=1u;
    scan_done();
    deus_wifi_node_dispatch_v0_poll(&runtime,75u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_AP_PAGE,4u,
                      SESSION_B,2u,0u,0u);
    expect_error(&temp,DNL_ERR_NOT_READY,76u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_START,5u,
                      SESSION_B,0u,0u,0u);
    result=expect_wifi_reply(&temp,77u);
    OK(result.generation==3u && runtime.owned_generation==3u);
    OK(scan_calls==3u);
    /* Exact retry idempotent even with new global SDK state. */
    result=expect_wifi_reply(&temp,78u);
    OK(result.generation==3u && scan_calls==3u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,6u,
                      SESSION_B,0u,0u,0u);
    result=expect_wifi_reply(&temp,79u);
    OK(result.generation==3u && result.state==DEUS_WIFI_SCANNING);

    /* SDK async timeout handled on task poll, no extra start/retry. */
    deus_wifi_node_dispatch_v0_poll(&runtime,9000u);
    OK(stopped_scans==1u);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,7u,
                      SESSION_B,0u,0u,0u);
    result=expect_wifi_reply(&temp,9001u);
    OK(result.state==DEUS_WIFI_ERROR);
    temp=wifi_request(DEUS_WIFI_CMD_SCAN_START,8u,
                      SESSION_B,0u,0u,0u);
    expect_error(&temp,DNL_ERR_NOT_READY,9002u);
    expect_error(&temp,DNL_ERR_NOT_READY,9003u);
    OK(scan_calls==3u);

    /* Missing SDK hooks must not falsely advertise a Wi-Fi capability. */
    deus_wifi_node_dispatch_v0_init(&runtime,0);
    req=hello(SESSION_A);ans=send_req(&req,10000u);
    OK(ans.type==DNL_HELLO_ACK);
    req=wifi_request(DEUS_WIFI_CMD_CAPS,2u,SESSION_A,0u,0u,0u);
    expect_error(&req,DNL_ERR_UNSUPPORTED,10001u);

    /* SDK reports a failed scan-completion event (no fake AP results). */
    reset_all();
    req=hello(SESSION_A);ans=send_req(&req,10500u);
    OK(ans.type==DNL_HELLO_ACK);
    req=wifi_request(DEUS_WIFI_CMD_SCAN_START,2u,SESSION_A,0u,0u,0u);
    result=expect_wifi_reply(&req,10501u);
    OK(result.state==DEUS_WIFI_SCANNING);
    {system_event_t failed_event={0};
     failed_event.event_id=SYSTEM_EVENT_SCAN_DONE;
     failed_event.event_info.scan_done.status=1u;
     OK(event_handler(event_ctx,&failed_event)==ESP_OK);}
    deus_wifi_node_dispatch_v0_poll(&runtime,10502u);
    OK(get_records_calls==0u);
    req=wifi_request(DEUS_WIFI_CMD_SCAN_STATUS,3u,SESSION_A,0u,0u,0u);
    result=expect_wifi_reply(&req,10503u);
    OK(result.state==DEUS_WIFI_ERROR && result.last_error==ESP_FAIL);
    /* SDK init error fail-closed, still no repeated initialization. */
    reset_all();
    fail_at=1u;
    req=hello(SESSION_A);ans=send_req(&req,11000u);
    OK(ans.type==DNL_HELLO_ACK);
    req=wifi_request(DEUS_WIFI_CMD_SCAN_START,2u,SESSION_A,0u,0u,0u);
    expect_error(&req,DNL_ERR_NOT_READY,11001u);
    expect_error(&req,DNL_ERR_NOT_READY,11002u);
    OK(init_calls==2u && radio_calls==0u && scan_calls==0u);

    /* Frozen codec stream feed resynchronizes bytes, not credentials. */
    reset_all();
    req=hello(SESSION_A);
    OK(dnl_encode(&req,buf,sizeof(buf),&n)==DNL_OK);
    len=0u;
    for(j=0u;j<n;++j) {
        int status=deus_wifi_node_dispatch_v0_feed(
            &runtime,buf[j],(uint32_t)(12000u+j),
            backup,sizeof(backup),&len);
        if(j+1u<n) OK(status==DNL_DROP);
        else OK(status==DNL_OK && len!=0u);
    }
    ans=frame_from(backup,len);
    OK(ans.type==DNL_HELLO_ACK && runtime.frames_replied==1u);

    puts("ESP_WIFI_NODE_DISPATCH_SDK_V34_MOCK=PASS SESSION_SEQUENCE_REPLAY_SCAN_PAGES_OLD_SESSION_BLOCKED_NO_RADIO_HARDWARE");
    return 0;
}
