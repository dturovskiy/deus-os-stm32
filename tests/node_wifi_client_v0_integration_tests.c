/* Cross-architecture SOURCE ONLY integration of exact frozen DNL v2
 * controller, new STM32 optional WiFi client and ESP optional dispatcher.
 * All radios and AP data synthetic. NO STM32/ESP, UART, USB or RF access.
 */
#include "kernel/node_wifi_client_v0.h"
#include "wifi_node_dispatch_v0.h"
#include "deus_node_link.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T(expr) do { if(!(expr)) { fprintf(stderr,"STM32_WIFI_CLIENT_FAIL line=%d %s\n",__LINE__,#expr); exit(3); } } while(0)
#define NONCE_A 0xA55A0921u
#define NONCE_B 0x0921A55Au
static dnl_controller_t controller;
static deus_node_wifi_client_v0_t wifi;
static deus_wifi_node_dispatch_v0_t esp;
static deus_wifi_scan_v0_t scan;
static unsigned scans;
static uint8_t wire[DNL_MAX_FRAME],reply[DNL_MAX_FRAME];
static size_t wire_length,reply_length;

static int sdk_start(void *ctx,uint32_t now_ms)
{
    (void)ctx;(void)now_ms;
    if(scan.state==DEUS_WIFI_SCANNING || scan.state==DEUS_WIFI_ERROR)
        return 0;
    ++scans;
    scan.state=DEUS_WIFI_SCANNING;
    scan.count=0u;
    ++scan.generation;
    return 1;
}
static void sdk_poll(void *ctx,uint32_t now_ms)
{
    (void)ctx;(void)now_ms;
}
static void sdk_snapshot(void *ctx,deus_wifi_scan_v0_t *out)
{
    (void)ctx;*out=scan;
}
static void begin_session(uint32_t nonce)
{
    deus_wifi_node_ops_v0_t ops;
    dnl_controller_init(&controller);
    deus_node_wifi_client_v0_init(&wifi);
    memset(&scan,0,sizeof(scan)); scan.state=DEUS_WIFI_READY;
    scans=0u;
    ops.context=0;ops.start_scan=sdk_start;
    ops.poll=sdk_poll;ops.snapshot=sdk_snapshot;
    deus_wifi_node_dispatch_v0_init(&esp,&ops);
    T(dnl_controller_hello(&controller,nonce,wire,sizeof(wire),
        &wire_length)==DNL_OK);
    T(deus_wifi_node_dispatch_v0_process(&esp,wire,wire_length,
        100u,reply,sizeof(reply),&reply_length)==DNL_OK);
    T(dnl_controller_receive(&controller,reply,reply_length)==DNL_OK);
    T(controller.ready==1u && controller.last_sequence==1u &&
      esp.worker.highest_sequence==1u);
}
static void request(uint8_t cmd,uint32_t gen,uint8_t index,uint8_t offset,
                    uint32_t now)
{
    wire_length=99u;
    T(deus_node_wifi_client_v0_request(&wifi,&controller,cmd,gen,
        index,offset,now,wire,sizeof(wire),&wire_length)==DNL_OK);
    T(wire_length>=DNL_FRAME_OVERHEAD&&wire_length<=DNL_MAX_FRAME);
    T(controller.pending && controller.pending_type==cmd);
    T(controller.last_sequence==esp.worker.highest_sequence+1u);
}
static void deliver(uint32_t now)
{
    T(deus_wifi_node_dispatch_v0_process(&esp,wire,wire_length,now,
        reply,sizeof(reply),&reply_length)==DNL_OK);
}
static void good(uint32_t now)
{
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,reply,
        reply_length,now)==DNL_OK);
    T(controller.pending==0u && controller.ready==1u);
    T(wifi.has_last_reply==1u && wifi.remote_error==0u);
}
static void frozen_ping(void)
{
    size_t len=0u;
    T(dnl_controller_request(&controller,DNL_PING,0,0,
        wire,sizeof(wire),&len)==DNL_OK);
    T(deus_wifi_node_dispatch_v0_process(&esp,wire,len,500u,
        reply,sizeof(reply),&reply_length)==DNL_OK);
    T(dnl_controller_receive(&controller,reply,reply_length)==DNL_OK);
    T(controller.ready && !controller.pending &&
      controller.last_sequence==esp.worker.highest_sequence);
}
static void legacy_unsupported(void)
{
    dnl_worker_t old;
    uint8_t msg[DNL_MAX_FRAME],res[DNL_MAX_FRAME];
    size_t mlen=0u,nlen=0u;
    const uint32_t nonce=NONCE_A;
    dnl_worker_init(&old);
    dnl_controller_init(&controller);
    deus_node_wifi_client_v0_init(&wifi);
    T(dnl_controller_hello(&controller,nonce,msg,sizeof(msg),&mlen)==DNL_OK);
    T(dnl_worker_process(&old,msg,mlen,res,sizeof(res),&nlen)==DNL_OK);
    T(dnl_controller_receive(&controller,res,nlen)==DNL_OK);
    T(deus_node_wifi_client_v0_request(&wifi,&controller,
        DEUS_WIFI_CMD_CAPS,0,0,0,700u,msg,sizeof(msg),&mlen)==DNL_OK);
    T(dnl_worker_process(&old,msg,mlen,res,sizeof(res),&nlen)==DNL_OK);
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,res,nlen,710u)
      ==DNL_REMOTE_ERROR);
    T(wifi.remote_error==DNL_ERR_UNSUPPORTED && wifi.has_last_reply==1u);
    T(controller.ready && !controller.pending);
}
static void wrong_reply_does_not_consume(uint32_t now)
{
    dnl_frame_t bad;
    uint8_t bogus[DNL_MAX_FRAME];
    size_t n=0u;
    T(dnl_decode(reply,reply_length,&bad)==DNL_OK);
    bad.session_id^=0xFFFFFFFFu;
    T(dnl_encode(&bad,bogus,sizeof(bogus),&n)==DNL_OK);
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,bogus,n,now)
      ==DNL_STALE_SESSION);
    T(controller.pending==1u && !wifi.has_last_reply);
    bad.session_id=controller.session_id;
    bad.sequence++;
    T(dnl_encode(&bad,bogus,sizeof(bogus),&n)==DNL_OK);
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,bogus,n,now)
      ==DNL_STALE_SESSION);
    T(controller.pending==1u);
    bogus[n-1u]^=0x55u;
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,bogus,n,now)
      ==DNL_BAD_FRAME);
    T(controller.pending==1u);
}
int main(void)
{
    uint8_t ssid[32], first_request[DNL_MAX_FRAME],first_reply[DNL_MAX_FRAME];
    size_t first_request_size=0u,first_reply_size=0u;
    uint8_t j;
    uint32_t now=110u;

    begin_session(NONCE_A);
    request(DEUS_WIFI_CMD_CAPS,0,0,0,now);
    deliver(now+5u);
    wrong_reply_does_not_consume(now+6u);
    good(now+7u);
    T(wifi.last_reply.major==1u &&
      wifi.last_reply.max_page==14u &&
      wifi.last_reply.max_aps==4u);
    T(controller.last_sequence==2u);
    frozen_ping();
    T(controller.last_sequence==3u);
    /* Asynchronous scan starts once after authenticated request. */
    request(DEUS_WIFI_CMD_SCAN_START,0,0,0,1000u);
    memcpy(first_request,wire,wire_length);
    first_request_size=wire_length;
    deliver(1020u);
    memcpy(first_reply,reply,reply_length);
    first_reply_size=reply_length;
    T(scans==1u && !wifi.has_last_reply);
    T(deus_wifi_node_dispatch_v0_process(&esp,first_request,
      first_request_size,1025u,reply,sizeof(reply),&reply_length)==DNL_OK);
    T(reply_length==first_reply_size);
    T(memcmp(first_reply,reply,reply_length)==0 && scans==1u);
    good(1030u);
    T(wifi.last_reply.state==DEUS_WIFI_SCANNING &&
      wifi.last_reply.generation==1u);

    scan.state=DEUS_WIFI_RESULTS;
    scan.count=1u;
    scan.aps[0].channel=11u;
    scan.aps[0].rssi_dbm=-55;
    scan.aps[0].authmode=3u;
    for(j=0u;j<32u;j++)scan.aps[0].ssid[j]=(uint8_t)('A'+(j%26u));
    scan.aps[0].ssid[32]=0u;
    request(DEUS_WIFI_CMD_SCAN_STATUS,0,0,0,1100u);
    deliver(1105u);good(1110u);
    T(wifi.last_reply.state==DEUS_WIFI_RESULTS);
    T(wifi.last_reply.count==1u && wifi.last_reply.generation==1u);
    T(controller.last_sequence==5u);
    for(j=0u;j<3u;j++) {
        uint8_t offset=(uint8_t)(14u*j);
        request(DEUS_WIFI_CMD_SCAN_AP_PAGE,1u,0u,offset,1200u+j*30u);
        deliver(1202u+j*30u);good(1204u+j*30u);
        T(wifi.last_reply.page_length==(j<2u?14u:4u));
        T(wifi.last_reply.ssid_length==32u);
        T(wifi.last_reply.ssid_offset==offset);
        T(wifi.last_reply.channel==11u && wifi.last_reply.rssi_dbm==-55);
        memcpy(ssid+offset,wifi.last_reply.page,wifi.last_reply.page_length);
    }
    T(memcmp(ssid,scan.aps[0].ssid,32u)==0);
    T(controller.last_sequence==8u &&
      esp.worker.highest_sequence==8u);

    /* Unrecognized generation from ESP must yield correlated remote
     * BAD_PAYLOAD but keep DNL session READY. */
    request(DEUS_WIFI_CMD_SCAN_AP_PAGE,2u,0u,0u,1400u);
    deliver(1405u);
    T(deus_node_wifi_client_v0_receive(&wifi,&controller,reply,
        reply_length,1410u)==DNL_REMOTE_ERROR);
    T(wifi.remote_error==DNL_ERR_NOT_READY && controller.ready);

    /* Retrying read-only caps is byte-identical; no seq drift or scan. */
    request(DEUS_WIFI_CMD_CAPS,0,0,0,2000u);
    memcpy(first_request,wire,wire_length);
    first_request_size=wire_length;
    T(deus_node_wifi_client_v0_timeout(&wifi,&controller,
      2000u+DEUS_NODE_WIFI_REQ_DEADLINE_MS,reply,sizeof(reply),
      &reply_length)==DNL_DROP);
    T(deus_node_wifi_client_v0_timeout(&wifi,&controller,
      2001u+DEUS_NODE_WIFI_REQ_DEADLINE_MS,wire,sizeof(wire),
      &wire_length)==DNL_OK);
    T(first_request_size==wire_length &&
      memcmp(first_request,wire,wire_length)==0);
    T(controller.retry_count==1u && controller.pending==1u);
    deliver(2660u);
    good(2665u);
    T(controller.retry_count==0u && scans==1u);

    /* A second unacknowledged read-only request times out twice and
     * fail-closes the whole link, never preserving a stale READY. */
    request(DEUS_WIFI_CMD_CAPS,0,0,0,3000u);
    T(deus_node_wifi_client_v0_timeout(&wifi,&controller,
       3651u,reply,sizeof(reply),&reply_length)==DNL_OK);
    T(deus_node_wifi_client_v0_timeout(&wifi,&controller,
       4302u,reply,sizeof(reply),&reply_length)==DNL_REHANDSHAKE_REQUIRED);
    T(!controller.ready&&!controller.pending&&!wifi.has_last_reply);
    T(deus_node_wifi_client_v0_request(&wifi,&controller,
       DEUS_WIFI_CMD_CAPS,0,0,0,4400u,wire,sizeof(wire),
       &wire_length)==DNL_NOT_READY_OR_BUSY);

    /* A pending RF start must never automatically retry. */
    begin_session(NONCE_B);
    request(DEUS_WIFI_CMD_SCAN_START,0,0,0,5000u);
    T(deus_node_wifi_client_v0_timeout(&wifi,&controller,
       8001u,reply,sizeof(reply),&reply_length)==DNL_REHANDSHAKE_REQUIRED);
    T(!controller.ready && !controller.pending && scans==0u);

    legacy_unsupported();
    puts("STM32_WIFI_NODE_CLIENT_REV2_ESP_DISPATCH_INTEGRATION=PASS SHA_SESSION_SEQ_PAGED32_REPLAY_TIMEOUT_NO_HARDWARE");
    return 0;
}
