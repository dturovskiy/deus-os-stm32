/* Source-linked binary RPC framing regression (no USB/device).
 * Exercises actual command_service and binary_frame implementations.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kernel/binary_rpc.h"
#include "kernel/binary_frame.h"
#include "kernel/command_service.h"
#define CHECK(x) do { if (!(x)) {fprintf(stderr,"FAIL %u: %s\n",(unsigned)__LINE__,#x);return 1;} } while(0)
typedef struct {
    uint8_t bytes[16][64];
    uint32_t lens[16];
    uint32_t count;
    uint32_t rejects;
} captured_t;
static int send_frame(void *p,const uint8_t *wire,uint32_t length)
{
    captured_t *c=(captured_t*)p;
    if (c->rejects) { --c->rejects;return 0; }
    if (c->count>=16u||length>64u||length<12u) return 0;
    memcpy(c->bytes[c->count],wire,length);
    c->lens[c->count++]=length;
    return 1;
}
static unsigned handler_called;
static command_service_status_t fake_handler(
    const command_service_request_t *r,command_service_context_t *c,void *p)
{
    uint32_t i;
    (void)p;
    ++handler_called;
    if (r->descriptor->rpc_id==COMMAND_SERVICE_RPC_PING)
        return command_service_write_line(c,"PONG");
    if (r->descriptor->rpc_id==COMMAND_SERVICE_RPC_NODECRC) {
        if (r->argc!=1u || strcmp(r->argv[0],"CRC")!=0)
            return COMMAND_SERVICE_STATUS_BAD_ARGS;
        return command_service_write_line(c,"NODE_CRC_QUEUED");
    }
    if (r->descriptor->rpc_id==COMMAND_SERVICE_RPC_UPTIME) {
        for(i=0u;i<100u;++i)
            if(command_service_write_byte(c,(uint8_t)'A')!=COMMAND_SERVICE_STATUS_OK)
                return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
        return COMMAND_SERVICE_STATUS_OK;
    }
    return command_service_write_line(c,"OK");
}
static binary_frame_view_t view_at(captured_t *c,uint32_t index,
                                   binary_frame_parser_t *parser)
{
    binary_frame_view_t v={0};
    binary_frame_feed_result_t state=BINARY_FRAME_FEED_TEXT;
    uint32_t i;
    binary_frame_parser_init(parser);
    for(i=0;i<c->lens[index];++i)
        state=binary_frame_parser_feed(parser,c->bytes[index][i],&v);
    if (state!=BINARY_FRAME_FEED_FRAME_READY) {
        fprintf(stderr,"BAD DECODE %u state %d\n",index,state);
        v.frame_type=0u;
    }
    return v;
}
static int run_tests(void)
{
    binary_rpc_state_t state;
    binary_rpc_workspace_t work;
    captured_t capture={0};
    binary_rpc_binding_t bind={0};
    binary_frame_view_t f={0};
    binary_frame_parser_t parser;
    binary_frame_view_t rsp;
    uint8_t payload[132]={0};
    uint32_t i;
    CHECK(sizeof(work)==192u);
    CHECK(BINARY_RPC_DATA_WIRE_MAX==64u);
    bind.send_wire=send_frame;
    bind.send_context=&capture;
    bind.command_handler=fake_handler;
    bind.source_rx_event_mask=0x5u;
    bind.capability_flags=0x7Fu;
    binary_rpc_init(&state,&work);
    f.version=BINARY_FRAME_PROTOCOL_VERSION;
    f.frame_type=BINARY_FRAME_TYPE_HELLO_REQUEST;
    f.flags=0u;f.reserved=0u;f.request_id=0x3579u;
    f.payload_length=0u; f.payload=0;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(capture.count==1u);
    CHECK(capture.lens[0]==28u);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_HELLO_RESPONSE);
    CHECK(rsp.request_id==0x3579u&&rsp.payload_length==16u);
    CHECK(rsp.payload[0]==BINARY_FRAME_PROTOCOL_VERSION);
    CHECK(rsp.payload[1]==COMMAND_SERVICE_FOUNDATION_VERSION);
    CHECK(rsp.payload[10]==(uint8_t)command_service_registry_count());
    CHECK(state.diagnostics.hello_requests==1u);
    CHECK(state.active_binding==0);
    capture.count=0u;
    f.frame_type=BINARY_FRAME_TYPE_RPC_REQUEST;
    f.payload=payload; f.payload_length=4u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_PING;payload[1]=0u;
    payload[2]=0u;payload[3]=0u;
    handler_called=0u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(handler_called==1u);
    CHECK(capture.count==2u);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_DATA);
    CHECK(rsp.request_id==f.request_id);
    CHECK(rsp.payload_length==10u);
    CHECK(memcmp(rsp.payload+4,"PONG\r\n",6u)==0);
    rsp=view_at(&capture,1,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload_length==10u);
    CHECK(rsp.payload[8]==BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE);
    CHECK(rsp.payload[9]==COMMAND_SERVICE_STATUS_OK);
    CHECK(state.active_binding==0);
    capture.count=0u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_NODECRC;payload[1]=0;
    payload[2]=1u;payload[3]=0u;
    payload[4]=3u;payload[5]='C';payload[6]='R';payload[7]='C';
    f.payload_length=8u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(handler_called==2u);
    CHECK(capture.count==2u);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_DATA);
    CHECK(memcmp(rsp.payload+4,"NODE_CRC_QUEUED",15u)==0);
    capture.count=0u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_UPTIME;payload[1]=0;
    payload[2]=0;payload[3]=0;f.payload_length=4u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(capture.count==4u); /* 48 + 48 + 4 + END */
    for(i=0;i<3u;++i) {
        rsp=view_at(&capture,i,&parser);
        CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_DATA);
        CHECK(rsp.payload_length== (i==2u?8u:52u));
        CHECK(rsp.payload[2]==i);
    }
    rsp=view_at(&capture,3,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload[2]==3u&&rsp.payload[4]==100u);
    capture.count=0u;
    payload[0]=0xFFu;payload[1]=0x7Fu;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(capture.count==1u);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload[9]==COMMAND_SERVICE_STATUS_NOT_FOUND);
    capture.count=0u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_PING;payload[1]=0;
    f.flags=2u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload[8]==BINARY_RPC_STATUS_DOMAIN_PROTOCOL);
    CHECK(rsp.payload[9]==BINARY_RPC_PROTOCOL_BAD_FLAGS);
    f.flags=0u;capture.count=0u;
    f.payload_length=2u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload[9]==BINARY_RPC_PROTOCOL_MALFORMED_REQUEST);
    f.payload_length=4u; capture.count=0u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_WDOGTRIP;payload[1]=0;
    payload[2]=0;payload[3]=0;
    i=handler_called;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(handler_called==i);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_RPC_END);
    CHECK(rsp.payload[8]==BINARY_RPC_STATUS_DOMAIN_PROTOCOL);
    CHECK(rsp.payload[9]==BINARY_RPC_PROTOCOL_DESTRUCTIVE_CONFIRM_REQUIRED);
    /* New ESP APP RESET is also destructive: no flag => no handler call. */
    capture.count=0u;
    payload[0]=(uint8_t)COMMAND_SERVICE_RPC_ESPAPPRESET;
    i=handler_called;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(handler_called==i && capture.count==1u);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.payload[8]==BINARY_RPC_STATUS_DOMAIN_PROTOCOL);
    CHECK(rsp.payload[9]==BINARY_RPC_PROTOCOL_DESTRUCTIVE_CONFIRM_REQUIRED);
    capture.count=0u;
    f.flags=BINARY_RPC_FLAG_ALLOW_DESTRUCTIVE;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    CHECK(handler_called==i+1u && capture.count==2u);
    rsp=view_at(&capture,1,&parser);
    CHECK(rsp.payload[8]==BINARY_RPC_STATUS_DOMAIN_COMMAND_SERVICE);
    CHECK(rsp.payload[9]==COMMAND_SERVICE_STATUS_OK);
    f.flags=0u;
    capture.count=0u;
    f.request_id=0u;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==1);
    rsp=view_at(&capture,0,&parser);
    CHECK(rsp.frame_type==BINARY_FRAME_TYPE_PROTOCOL_ERROR);
    CHECK(rsp.payload[0]==BINARY_RPC_PROTOCOL_REQUEST_ID_RESERVED);
    f.request_id=0x3579u;
    capture.count=0u;
    capture.rejects=1u;
    f.frame_type=BINARY_FRAME_TYPE_HELLO_REQUEST;
    f.payload_length=0u;f.payload=0;
    CHECK(binary_rpc_handle_frame(&state,&f,&bind)==0);
    CHECK(state.diagnostics.tx_failures>=1u);
    CHECK(state.active_binding==0);
    CHECK(binary_rpc_handle_frame(0,&f,&bind)==0);
    CHECK(binary_rpc_handle_frame(&state,0,&bind)==0);
    puts("BINARY_RPC_TRANSPORT=PASS HELLO_RPC_CRC_CHUNKS_ERRORS_DESTRUCTIVE_DENY_TX_FAILURE");
    return 0;
}
int main(void) {return run_tests();}
