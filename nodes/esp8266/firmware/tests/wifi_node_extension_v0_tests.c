/* Actual extension + frozen REV2 codec offline tests: no hardware/SDK.
 * All AP names are synthetic; never print discovered SSIDs to logs.
 */
#include "wifi_node_extension_v0.h"
#include "deus_node_link.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST(cond) do { if (!(cond)) { \
    fprintf(stderr,"WIFI_EXT_FAIL line=%d condition=%s\n",__LINE__,#cond); \
    exit(2); \
} } while (0)
_Static_assert(DNL_MAX_PAYLOAD==24u,"Frozen DNL v0 payload altered");
_Static_assert(DNL_MAX_FRAME==40u,"Frozen DNL v0 frame altered");
_Static_assert(DEUS_WIFI_EXT_PAGE_HEADER+DEUS_WIFI_EXT_PAGE_MAX==
               DNL_MAX_PAYLOAD,"Page must exactly fit DNL payload");

static void expect_wire(const dnl_frame_t *in)
{
    dnl_frame_t received;
    uint8_t raw[DNL_MAX_FRAME];
    size_t length=0u;
    TEST(dnl_encode(in,raw,sizeof(raw),&length)==DNL_OK);
    TEST(length<=DNL_MAX_FRAME);
    TEST(length==(size_t)DNL_FRAME_OVERHEAD+in->payload_length);
    TEST(dnl_decode(raw,length,&received)==DNL_OK);
    TEST(received.type==in->type);
    TEST(received.sequence==in->sequence);
    TEST(received.session_id==in->session_id);
    TEST(received.payload_length==in->payload_length);
    TEST(memcmp(received.payload,in->payload,in->payload_length)==0);
    raw[length-2u]^=0x40u;
    TEST(dnl_decode(raw,length,&received)==DNL_BAD_FRAME);
}
static void query(uint8_t type, uint32_t gen, uint8_t index, uint8_t offset,
                  dnl_frame_t *out)
{
    TEST(deus_wifi_ext_make_request_v0(type,7u,0x77AABBCCu,
                                       gen,index,offset,out)==
         DEUS_WIFI_EXT_OK);
    expect_wire(out);
}
static void respond(const dnl_frame_t *req,
                    const deus_wifi_scan_v0_t *scan,
                    dnl_frame_t *reply,
                    deus_wifi_ext_result_v0_t *got)
{
    TEST(deus_wifi_ext_respond_snapshot_v0(req,scan,reply)==DEUS_WIFI_EXT_OK);
    expect_wire(reply);
    TEST(deus_wifi_ext_read_response_v0(req,reply,got)==DEUS_WIFI_EXT_OK);
    TEST(got->type==(uint8_t)(req->type|0x80u));
}
static void incompatible(const dnl_frame_t *req, const dnl_frame_t *bad)
{
    deus_wifi_ext_result_v0_t got, before;
    memset(&got,0xA5,sizeof(got));
    before=got;
    TEST(deus_wifi_ext_read_response_v0(req,bad,&got)==
         DEUS_WIFI_EXT_INCOMPATIBLE);
    TEST(memcmp(&got,&before,sizeof(got))==0);
}
static void remote(const dnl_frame_t *req,const deus_wifi_scan_v0_t *scan,
                   uint8_t code)
{
    dnl_frame_t reply;
    deus_wifi_ext_result_v0_t got;
    TEST(deus_wifi_ext_respond_snapshot_v0(req,scan,&reply)==DEUS_WIFI_EXT_OK);
    TEST(reply.type==DNL_ERROR);
    TEST(reply.payload_length==2u);
    TEST(reply.payload[0]==code && reply.payload[1]==req->type);
    expect_wire(&reply);
    TEST(deus_wifi_ext_read_response_v0(req,&reply,&got)==
         DEUS_WIFI_EXT_REMOTE_ERROR);
    TEST(got.error==code);
}
static void verify_legacy_worker_unsupported(void)
{
    dnl_worker_t legacy;
    dnl_frame_t hello,req,reply;
    uint8_t packet[DNL_MAX_FRAME],answer[DNL_MAX_FRAME];
    size_t length=0u,answer_length=0u;
    dnl_worker_init(&legacy);
    memset(&hello,0,sizeof(hello));
    hello.type=DNL_HELLO;
    hello.sequence=1u;
    hello.payload_length=4u;
    hello.payload[0]=0xCC;hello.payload[1]=0xBB;
    hello.payload[2]=0xAA;hello.payload[3]=0x77;
    TEST(dnl_encode(&hello,packet,sizeof(packet),&length)==DNL_OK);
    TEST(dnl_worker_process(&legacy,packet,length,answer,sizeof(answer),
                            &answer_length)==DNL_OK);
    TEST(dnl_decode(answer,answer_length,&reply)==DNL_OK);
    TEST(reply.type==DNL_HELLO_ACK);
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_CAPS,2u,0x77AABBCCu,
                                        0,0,0,&req)==DEUS_WIFI_EXT_OK);
    TEST(dnl_encode(&req,packet,sizeof(packet),&length)==DNL_OK);
    TEST(dnl_worker_process(&legacy,packet,length,answer,sizeof(answer),
                            &answer_length)==DNL_OK);
    TEST(dnl_decode(answer,answer_length,&reply)==DNL_OK);
    TEST(reply.type==DNL_ERROR && reply.payload_length==2u);
    TEST(reply.payload[0]==DNL_ERR_UNSUPPORTED);
    TEST(reply.payload[1]==DEUS_WIFI_CMD_CAPS);
    /* A real old worker must not silently advertise new Wi-Fi support. */
}
int main(void)
{
    dnl_frame_t req,reply,bad;
    deus_wifi_scan_v0_t scan;
    deus_wifi_ext_result_v0_t out;
    uint8_t ssid_assembled[DEUS_WIFI_EXT_MAX_SSID];
    uint8_t sentinel[sizeof(dnl_frame_t)];
    uint8_t offset;
    unsigned i;

    verify_legacy_worker_unsupported();
    query(DEUS_WIFI_CMD_CAPS,0u,0u,0u,&req);
    respond(&req,0,&reply,&out);
    TEST(out.major==1u && out.minor==0u && out.flags==1u);
    TEST(out.max_aps==4u && out.max_ssid==32u && out.max_page==14u);
    bad=reply;bad.payload[7]=1u;incompatible(&req,&bad);
    bad=reply;bad.session_id^=1u;incompatible(&req,&bad);
    bad=reply;bad.sequence++;incompatible(&req,&bad);
    bad=reply;bad.payload_length=7u;incompatible(&req,&bad);
    /* Output remains unchanged if build inputs are invalid. */
    memset(&req,0xC4,sizeof(req));
    memcpy(sentinel,&req,sizeof(req));
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_SCAN_AP_PAGE,3u,
         42u,0u,0u,0u,&req)==DEUS_WIFI_EXT_BAD_MESSAGE);
    TEST(memcmp(&req,sentinel,sizeof(req))==0);
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_SCAN_AP_PAGE,3u,
         42u,1u,4u,0u,&req)==DEUS_WIFI_EXT_BAD_MESSAGE);
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_SCAN_AP_PAGE,3u,
         42u,1u,0u,33u,&req)==DEUS_WIFI_EXT_BAD_MESSAGE);
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_CAPS,3u,
         42u,1u,0u,0u,&req)==DEUS_WIFI_EXT_BAD_MESSAGE);
    TEST(deus_wifi_ext_make_request_v0(0x09u,3u,42u,0u,0u,0u,&req)
         ==DEUS_WIFI_EXT_BAD_MESSAGE);
    TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_CAPS,0u,42u,0u,0u,0u,
         &req)==DEUS_WIFI_EXT_BAD_MESSAGE);

    memset(&scan,0,sizeof(scan));
    scan.state=DEUS_WIFI_OFF;
    query(DEUS_WIFI_CMD_SCAN_START,0,0,0,&req);
    memset(&reply,0xA7,sizeof(reply));
    memcpy(sentinel,&reply,sizeof(reply));
    TEST(deus_wifi_ext_respond_snapshot_v0(&req,&scan,&reply)
         ==DEUS_WIFI_EXT_ACTION_REQUIRED);
    TEST(memcmp(sentinel,&reply,sizeof(reply))==0);
    TEST(deus_wifi_ext_make_start_ack_v0(&req,&scan,&reply)
         ==DEUS_WIFI_EXT_BAD_MESSAGE);
    scan.state=DEUS_WIFI_SCANNING;
    scan.generation=0xFABC1234u; /* tests full 32-bit generation */
    TEST(deus_wifi_ext_make_start_ack_v0(&req,&scan,&reply)
         ==DEUS_WIFI_EXT_OK);
    expect_wire(&reply);
    TEST(deus_wifi_ext_read_response_v0(&req,&reply,&out)==
         DEUS_WIFI_EXT_OK);
    TEST(out.type==DEUS_WIFI_REPLY_SCAN_START);
    TEST(out.state==DEUS_WIFI_SCANNING && out.generation==0xFABC1234u);
    bad=reply;bad.payload[4]=DEUS_WIFI_RESULTS;
    incompatible(&req,&bad);

    query(DEUS_WIFI_CMD_SCAN_STATUS,0,0,0,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.generation==0xFABC1234u);
    TEST(out.state==DEUS_WIFI_SCANNING);
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,0,0,&req);
    remote(&req,&scan,DNL_ERR_BUSY);
    scan.state=DEUS_WIFI_READY;
    remote(&req,&scan,DNL_ERR_NOT_READY);

    scan.state=DEUS_WIFI_RESULTS;
    scan.count=4u;
    scan.truncated=1u;
    scan.aps[0].channel=6u;
    scan.aps[0].rssi_dbm=-47;
    scan.aps[0].authmode=3u;
    for (i=0u;i<32u;++i)
        scan.aps[0].ssid[i]=(uint8_t)('a'+(i%26u));
    scan.aps[0].ssid[32]=0u;
    scan.aps[1].ssid[0]=0u; /* hidden/empty AP name */
    scan.aps[2].ssid[0]=0xC1u; /* raw non-UTF8 is not mutated */
    scan.aps[2].ssid[1]=0xFFu;
    scan.aps[2].ssid[2]=0u;

    query(DEUS_WIFI_CMD_SCAN_STATUS,0,0,0,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.count==4u && out.truncated==1u);
    TEST(out.generation==0xFABC1234u);

    offset=0u;
    memset(ssid_assembled,0,sizeof(ssid_assembled));
    while (offset<32u) {
        query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,0,offset,&req);
        respond(&req,&scan,&reply,&out);
        TEST(out.generation==scan.generation && out.ap_index==0u);
        TEST(out.ssid_offset==offset && out.ssid_length==32u);
        TEST(out.page_length>0u && out.page_length<=14u);
        TEST(out.channel==6u && out.rssi_dbm==-47 && out.authmode==3u);
        memcpy(ssid_assembled+offset,out.page,out.page_length);
        offset=(uint8_t)(offset+out.page_length);
    }
    TEST(offset==32u);
    TEST(memcmp(ssid_assembled,scan.aps[0].ssid,32u)==0);
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,0,32u,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.ssid_length==32u && out.page_length==0u);

    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,1,0,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.ssid_length==0u && out.page_length==0u);
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,2,0,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.ssid_length==2u && out.page_length==2u);
    TEST(out.page[0]==0xC1u && out.page[1]==0xFFu);

    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation+1u,0,0,&req);
    remote(&req,&scan,DNL_ERR_BAD_PAYLOAD); /* stale generation */
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,0,0,&req);
    respond(&req,&scan,&reply,&out);
    bad=reply;bad.payload[0]^=0x80u;incompatible(&req,&bad);
    bad=reply;bad.payload[4]=1u;incompatible(&req,&bad);
    bad=reply;bad.payload[5]=14u;incompatible(&req,&bad);
    bad=reply;bad.payload[6]=33u;incompatible(&req,&bad);
    bad=reply;bad.payload_length=10u;incompatible(&req,&bad);
    bad=reply;bad.payload[8]=0x77u;
    /* RSSI is opaque signed byte; this mutation remains protocol-valid. */
    TEST(deus_wifi_ext_read_response_v0(&req,&bad,&out)==
         DEUS_WIFI_EXT_OK);
    {
        dnl_frame_t saved=req;
        TEST(deus_wifi_ext_make_request_v0(DEUS_WIFI_CMD_SCAN_AP_PAGE,7u,
            0x77AABBCCu,scan.generation,0u,33u,&req)==
            DEUS_WIFI_EXT_BAD_MESSAGE);
        TEST(memcmp(&req,&saved,sizeof(req))==0);
    }
    /* Valid 0..3 index can still refer to an absent AP in scan results. */
    scan.count=2u;
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,3,0,&req);
    remote(&req,&scan,DNL_ERR_BAD_PAYLOAD);
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,2,3,&req);
    remote(&req,&scan,DNL_ERR_BAD_PAYLOAD);
    scan.state=DEUS_WIFI_ERROR;
    scan.count=0u;
    scan.truncated=0u;
    scan.last_error=-7;
    query(DEUS_WIFI_CMD_SCAN_STATUS,0,0,0,&req);
    respond(&req,&scan,&reply,&out);
    TEST(out.state==DEUS_WIFI_ERROR && out.last_error==-7);
    query(DEUS_WIFI_CMD_SCAN_AP_PAGE,scan.generation,0,0,&req);
    remote(&req,&scan,DNL_ERR_NOT_READY);
    /* A changed frame header with correct payload is still not correlated. */
    query(DEUS_WIFI_CMD_SCAN_STATUS,0,0,0,&req);
    respond(&req,&scan,&reply,&out);
    bad=reply;
    bad.type=DEUS_WIFI_REPLY_SCAN_AP_PAGE;
    incompatible(&req,&bad);
    puts("ESP_WIFI_NODE_EXTENSION_V0_SOURCE_ONLY=PASS REV2_UNCHANGED_PAGED32_GEN32_ERRORS_NO_SDK_NO_RADIO");
    return 0;
}
