/* OFFLINE actual USB Management ESP-bridge frame dispatch test.
 * Compiled against byte-identical product usb_management.c with ONLY
 * ARM-only reset inline instructions neutralized in temporary test copy.
 * Never access memory mapped peripherals. USB OUT/IN are in-memory mocks.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drivers/usb_device.h"
#include "drivers/usart2_node.h"
#include "kernel/binary_frame.h"
#include "kernel/binary_rpc.h"
#include "kernel/esp_bridge_core_v0.h"
#include "kernel/usb_management.h"
#include "kernel/asset_transfer.h"
#include "kernel/node_transport.h"
#include "kernel/time.h"

#define CHECK(c) do { if (!(c)) { \
    printf("USB_BRIDGE_MOCK_FAIL line=%d expr=%s\n",__LINE__,#c);exit(2); } } while (0)

volatile usb_device_diagnostics_t usb_device_diagnostics;
static uint8_t inbound[8192], response[64];
static uint32_t inbound_len, inbound_pos, response_len, responses;
static uint32_t uart_reads, uart_writes, uart_arms, node_enters, node_leaves;
static uint32_t asset_requests, rpc_requests_seen, reset_sessions, asset_polls;
static uint32_t mocked_configured=1;
static uint8_t mocked_node_maintenance_owned;
static uint8_t mocked_node_enter_allowed=1u;
static uint32_t send_fail;
static kernel_time_ms_t mocked_time;
static binary_rpc_workspace_t workspace;
static binary_rpc_binding_t binding;

int usb_management_is_configured(void) {return (int)mocked_configured;}
int usb_management_try_getc(uint8_t *out) {
    if(inbound_pos>=inbound_len) {inbound_pos=inbound_len=0;return 0;}
    *out=inbound[inbound_pos++];return 1;
}
int send_wire(void *ctx,const uint8_t *bytes,uint32_t length) {
    (void)ctx;
    if(length>sizeof(response) || length<12u) return 0;
    ++responses;
    memcpy(response,bytes,length);
    response_len=length;
    return !send_fail;
}
void asset_transfer_init(void) {}
void asset_transfer_reset_session(void) {++reset_sessions;}
void asset_transfer_poll(void) {++asset_polls;}
int asset_transfer_handle_frame(const binary_frame_view_t *frame,
   asset_transfer_send_wire_t send,void *ctx) {
    (void)frame;(void)send;(void)ctx;
    ++asset_requests;return 1;
}
void binary_rpc_init(binary_rpc_state_t *s,binary_rpc_workspace_t *w) {
    memset(s,0,sizeof(*s));s->workspace=w;
}
int binary_rpc_handle_frame(binary_rpc_state_t *s,
 const binary_frame_view_t *frame,const binary_rpc_binding_t *b) {
    (void)s;(void)frame;(void)b;
    ++rpc_requests_seen;return 1;
}
kernel_time_ms_t kernel_time_now(void){return mocked_time;}
int kernel_time_reached(kernel_time_ms_t a,kernel_time_ms_t b){return a>=b;}
int kernel_time_elapsed(kernel_time_ms_t a,kernel_time_ms_t b){(void)a;(void)b;return 0;}
int node_transport_maintenance_enter(void){
    ++node_enters;
    if (mocked_node_enter_allowed==0u) return 0;
    mocked_node_maintenance_owned=1u;
    return 1;
}
void node_transport_maintenance_leave(void){
    ++node_leaves;
    mocked_node_maintenance_owned=0u;
}
uint8_t node_transport_maintenance_is_owned(void){
    return mocked_node_maintenance_owned;
}
void usart2_node_rx_arm(void){++uart_arms;}
void usart2_node_rx_disarm(void){}
int usart2_node_write_frame(const uint8_t *bytes,uint32_t count) {
    (void)bytes;(void)count;++uart_writes;return 1;
}
int usart2_node_write_rom_sync46(const uint8_t *data,uint32_t length) {
    if (data == (const uint8_t *)0 || length != 46u) return 0;
    ++uart_writes;return 1;
}
int usart2_node_try_read(usart2_node_sample_t *out) {
    (void)out;++uart_reads;return 0;
}
static uint16_t req_id=10u;
static uint8_t session_payload[48];
static void setup_input(uint8_t op,uint16_t seq,uint8_t count) {
    memset(session_payload,0,sizeof(session_payload));
    session_payload[0]=op;
    session_payload[1]=0x78;session_payload[2]=0x56;session_payload[3]=0x34;session_payload[4]=0x12;
    session_payload[5]=(uint8_t)seq;session_payload[6]=(uint8_t)(seq>>8);
    session_payload[7]=count;
}
static void push(uint8_t type,uint8_t flags,uint8_t reserved,uint16_t request,
    const uint8_t *payload,uint16_t size) {
    uint8_t tmp[144];uint32_t len,i;
    len=binary_frame_encode(type,flags,request,payload,size,tmp,sizeof(tmp));
    CHECK(len>0u);
    /* Deliberately modify reserved byte only after CRC recomputation. */
    if(reserved!=0u) {
        tmp[5]=reserved;
        {
            uint16_t c=binary_frame_crc16_ccitt_false(tmp+2,len-4u);
            tmp[len-2]=(uint8_t)c;tmp[len-1]=(uint8_t)(c>>8);
        }
    }
    CHECK(inbound_len+len<sizeof(inbound));
    for(i=0;i<len;++i)inbound[inbound_len+i]=tmp[i];
    inbound_len += len;
}
static uint8_t read_response_status(uint8_t expected_type,uint16_t request) {
    binary_frame_parser_t parser;binary_frame_view_t frame;
    uint32_t i;int got=0;uint8_t status=255u;
    binary_frame_parser_init(&parser);
    for(i=0;i<response_len;++i){
        if(binary_frame_parser_feed(&parser,response[i],&frame)==BINARY_FRAME_FEED_FRAME_READY) {
            CHECK(frame.frame_type==expected_type);
            CHECK(frame.request_id==request);
            CHECK(frame.version==1u);
            CHECK(frame.payload_length==9u);
            CHECK(frame.payload[8]==0u);
            status=frame.payload[7];
            ++got;
        }
    }
    CHECK(got==1);
    return status;
}
static void expect_no_uart(void){
    CHECK(uart_reads==0u && uart_writes==0u && uart_arms==0u &&
          node_enters==0u && node_leaves==0u);
}
int main(void) {
    uint32_t before;
    usb_management_esp_bridge_diagnostics_t observation;
    binding.send_wire=send_wire;
    binding.send_context=0;
    binding.command_handler=0;
    binding.command_handler_context=0;
    binding.source_rx_event_mask=0;
    binding.capability_flags=0;
    CHECK(usb_management_runtime_init(&workspace)==1);
    expect_no_uart();
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==0u && observation.faulted==0u);
    CHECK(observation.last_open_reason==ESP_BRIDGE_OPEN_NOT_SEEN);
    CHECK(observation.node_maintenance_owned==0u);
    CHECK(observation.firmware_update_reset_pending==0u);
    CHECK(observation.split_adapter_profile==1u);
    expect_no_uart(); /* read-only getter invokes zero admission callbacks */
    /* Only a SOURCE-SELECTED CH340 BOOT + STM32 UART split profile
       may park NodeLink and admit bridge OPEN. */
    setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(responses==1u && response_len<=64u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_enters==1u && uart_arms==0u); /* RX gated until first bridge TX */
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==1u && observation.faulted==0u);
    CHECK(observation.last_open_reason==ESP_BRIDGE_OPEN_ACCEPTED);
    CHECK(observation.node_maintenance_owned==1u);
    CHECK(observation.uart_attempted==0u);
    CHECK(uart_writes==0u);
    /* The first real ROM SLIP fragment is only allowed after OPEN.
       External CH340 boot ownership is a host-side protocol gate. */
    ++req_id;
    setup_input(2u,1u,3u);
    session_payload[8]=0xC0u; session_payload[9]=0x00u;
    session_payload[10]=0xC0u;
    push(0x05u,0u,0u,req_id,session_payload,11u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(uart_writes==1u && uart_arms==1u); /* RX enabled with first TX */
    ++req_id;setup_input(3u,2u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(uart_reads>0u);
    ++req_id;setup_input(4u,3u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_leaves==1u);
    /* CLOSED session does not reopen by adding a TX request. */
    ++req_id;setup_input(2u,4u,1u); session_payload[8]=0xc0u;
    push(0x05u,0u,0u,req_id,session_payload,9u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==2u);
    CHECK(uart_writes==1u);
    /* Lost USB-IN reply after OPEN, BEFORE any UART operation, is
       recoverable. It releases NodeLink once and a fresh session may OPEN. */
    send_fail=1u;
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    send_fail=0u;
    CHECK(node_enters==2u && node_leaves==2u);
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_enters==3u && uart_writes==1u);
    ++req_id;setup_input(4u,1u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_leaves==3u && uart_writes==1u);
    /* Real target idle-lease path (15s) must recover if no UART request
       was ever attempted; both old session and a blind TX are rejected. */
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_enters==4u && node_leaves==3u);
    mocked_time=15001u;
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(node_leaves==4u && uart_writes==1u);
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==0u && observation.faulted==0u);
    CHECK(observation.node_maintenance_owned==0u);
    CHECK(observation.uart_attempted==0u);
    ++req_id;setup_input(2u,1u,1u);session_payload[8]=0xC0u;
    push(0x05u,0u,0u,req_id,session_payload,9u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_BAD_SESSION);
    CHECK(uart_writes==1u);
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
    CHECK(node_enters==5u);
    ++req_id;setup_input(4u,1u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
    CHECK(node_leaves==5u && uart_writes==1u);
    /* Force an actual target OPEN admission rejection, no UART or CH340. */
    mocked_node_enter_allowed=0u;
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_DENIED);
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==0u && observation.faulted==0u);
    CHECK(observation.node_maintenance_owned==0u);
    CHECK(observation.last_open_reason==ESP_BRIDGE_OPEN_DENY_ENTER);
    CHECK(node_enters==6u && node_leaves==5u);
    CHECK(uart_writes==1u);
    mocked_node_enter_allowed=1u;
    /* The USB bridge shares the old dispatcher and never changes
       a Flash update response or carries a CH340 UART byte. */
    ++req_id;push(0x03u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(asset_requests==1u);
    ++req_id;push(0x02u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==1u);
    CHECK(rpc_requests_seen==1u);
    before=responses;
    ++req_id;{
        uint8_t invalid[1]={0xFFu};
        push(0x04u,0u,0u,req_id,invalid,1u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(responses==before+1);
    }
    /* TRUE firmware dispatcher coverage of new v15 ROM-only USB opcode5:
     * exact 46-byte SYNC split over TX40+TX6; timeout auto releases only
     * the STM32 bridge, not MCU/ESP physical reset or Flash. */
    {
        uint8_t sync[46]={0};
        uint32_t initial_writes=uart_writes;
        uint32_t initial_arms=uart_arms;
        uint32_t initial_leaves=node_leaves;
        unsigned index;
        sync[0]=0xC0u;sync[2]=8u;sync[3]=36u;
        sync[9]=7u;sync[10]=7u;sync[11]=0x12u;sync[12]=0x20u;
        for(index=13u;index<45u;++index)sync[index]=0x55u;
        sync[45]=0xC0u;
        ++req_id;setup_input(ESP_BRIDGE_OPEN_ROM_SYNC_ONLY,0u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        CHECK(uart_arms==initial_arms);
        ++req_id;setup_input(ESP_BRIDGE_TX,1u,40u);
        memcpy(session_payload+8u,sync,40u);
        push(0x05u,0u,0u,req_id,session_payload,48u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        CHECK(uart_arms==initial_arms && uart_writes==initial_writes);
        ++req_id;setup_input(ESP_BRIDGE_TX,2u,6u);
        memcpy(session_payload+8u,sync+40u,6u);
        push(0x05u,0u,0u,req_id,session_payload,14u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        CHECK(uart_writes==initial_writes+1u && uart_arms==initial_arms+1u);
        ++req_id;setup_input(ESP_BRIDGE_RX,3u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        mocked_time+=15001u;
        CHECK(usb_management_runtime_service(&binding)==0u);
        usb_management_esp_bridge_diagnostics_get(&observation);
        CHECK(observation.faulted==0u && observation.active==0u &&
              observation.node_maintenance_owned==0u &&
              node_leaves==initial_leaves+1u);
        ++req_id;setup_input(ESP_BRIDGE_OPEN_ROM_SYNC_ONLY,0u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        ++req_id;setup_input(ESP_BRIDGE_CLOSE,1u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        printf("USB_REAL_DISPATCH_ROM_ONLY_STAGED40_6_ATOMIC46_IDLE_ABORT_RECOVERY=PASS\n");
    }
    /* The legacy arbitrary UART path still never recovers on timeout:
     * a future ESP Flash operation must remain fail-closed. */
    {
        uint32_t initial_writes=uart_writes;
        ++req_id;setup_input(ESP_BRIDGE_OPEN,0u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        ++req_id;setup_input(ESP_BRIDGE_TX,1u,1u);
        session_payload[8]=0xD9u; /* deliberately not an ESP ROM SYNC byte */
        push(0x05u,0u,0u,req_id,session_payload,9u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
        CHECK(uart_writes==initial_writes+1u);
        mocked_time+=15001u;
        CHECK(usb_management_runtime_service(&binding)==0u);
        usb_management_esp_bridge_diagnostics_get(&observation);
        CHECK(observation.faulted==1u && observation.active==0u &&
              observation.node_maintenance_owned==0u);
        ++req_id;setup_input(ESP_BRIDGE_OPEN_ROM_SYNC_ONLY,0u,0u);
        push(0x05u,0u,0u,req_id,session_payload,8u);
        CHECK(usb_management_runtime_service(&binding)==0u);
        CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_DENIED);
        CHECK(uart_writes==initial_writes+1u);
        printf("USB_REAL_DISPATCH_GENERIC_FLASH_LATCH_UNBYPASSABLE=PASS\n");
    }
    printf("USB_SPLIT_CURRENT_WIRING_REAL_DISPATCH=PASS ONE_OPEN_TX_RX_CLOSE_UNWIND_NO_RETRY=YES\n");
    return 0;
}
