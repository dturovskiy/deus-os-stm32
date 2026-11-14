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
static uint8_t mock_rx_irq_armed, mock_rx_loss;
static uint32_t wrong_baud_startup_emitted;
static void esp_ch340_reset_boot_chatter_during_open(void)
{
    ++wrong_baud_startup_emitted;
    /* STM32 115200 UART may see ESP boot bytes at different speed and
     * mark FE/NE/ORE as rx_loss IF RXNEIE is armed during external reset. */
    if (mock_rx_irq_armed != 0u) mock_rx_loss = 1u;
}
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
void usart2_node_rx_arm(void)
{
    ++uart_arms;
    /* Real usart2_node_rx_gate_apply flushes error/ring before IRQ enable. */
    mock_rx_loss = 0u;
    mock_rx_irq_armed = 1u;
}
void usart2_node_rx_disarm(void)
{
    mock_rx_irq_armed = 0u;
    mock_rx_loss = 0u;
}
int usart2_node_write_frame(const uint8_t *bytes,uint32_t count) {
    (void)bytes;(void)count;++uart_writes;return 1;
}
int usart2_node_try_read(usart2_node_sample_t *out) {
    (void)out;
    ++uart_reads;
    if (mock_rx_loss != 0u) { mock_rx_loss = 0u; return -1; }
    return 0;
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
    CHECK(node_enters==1u && uart_arms==0u && mock_rx_irq_armed==0u);
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==1u && observation.faulted==0u);
    CHECK(observation.last_open_reason==ESP_BRIDGE_OPEN_ACCEPTED);
    CHECK(observation.node_maintenance_owned==1u);
    CHECK(observation.uart_attempted==0u);
    CHECK(uart_writes==0u);
    /* CH340 toggles external ESP EN/GPIO0 AFTER the STM32 bridge OPEN.
     * ESP boot chatter at an unqualified baud must NOT poison USART RX. */
    esp_ch340_reset_boot_chatter_during_open();
    CHECK(wrong_baud_startup_emitted==1u);
    CHECK(mock_rx_loss==0u && mock_rx_irq_armed==0u);
    /* The first real ROM SLIP fragment is only allowed after OPEN.
       External CH340 boot ownership is a host-side protocol gate. */
    ++req_id;
    setup_input(2u,1u,3u);
    session_payload[8]=0xC0u; session_payload[9]=0x00u;
    session_payload[10]=0xC0u;
    push(0x05u,0u,0u,req_id,session_payload,11u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(uart_writes==1u && uart_arms==1u && mock_rx_irq_armed==1u);
    CHECK(mock_rx_loss==0u);
    /* ESP ROM SYNC spans two USB→USART2 fragments (40+6). The second
     * TX must never re-arm/reset RX and discard early ROM answer bytes. */
    ++req_id;setup_input(2u,2u,6u);
    session_payload[8]=0x55u; session_payload[9]=0x55u;
    session_payload[10]=0x55u; session_payload[11]=0x55u;
    session_payload[12]=0x55u; session_payload[13]=0xC0u;
    push(0x05u,0u,0u,req_id,session_payload,14u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(uart_writes==2u && uart_arms==1u && mock_rx_irq_armed==1u);
    ++req_id;setup_input(3u,3u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(uart_reads>0u);
    ++req_id;setup_input(4u,4u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_leaves==1u);
    /* CLOSED session does not reopen by adding a TX request. */
    ++req_id;setup_input(2u,5u,1u); session_payload[8]=0xc0u;
    push(0x05u,0u,0u,req_id,session_payload,9u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==2u);
    CHECK(uart_writes==2u);
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
    CHECK(node_enters==3u && uart_writes==2u);
    ++req_id;setup_input(4u,1u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_leaves==3u && uart_writes==2u);
    /* Real target idle-lease path (15s) must recover if no UART request
       was ever attempted; both old session and a blind TX are rejected. */
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==0u);
    CHECK(node_enters==4u && node_leaves==3u);
    mocked_time=15001u;
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(node_leaves==4u && uart_writes==2u);
    usb_management_esp_bridge_diagnostics_get(&observation);
    CHECK(observation.active==0u && observation.faulted==0u);
    CHECK(observation.node_maintenance_owned==0u);
    CHECK(observation.uart_attempted==0u);
    ++req_id;setup_input(2u,1u,1u);session_payload[8]=0xC0u;
    push(0x05u,0u,0u,req_id,session_payload,9u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_BAD_SESSION);
    CHECK(uart_writes==2u);
    ++req_id;setup_input(1u,0u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
    CHECK(node_enters==5u);
    ++req_id;setup_input(4u,1u,0u);
    push(0x05u,0u,0u,req_id,session_payload,8u);
    CHECK(usb_management_runtime_service(&binding)==0u);
    CHECK(read_response_status(0x87u,req_id)==ESP_BRIDGE_OK);
    CHECK(node_leaves==5u && uart_writes==2u);
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
    CHECK(uart_writes==2u);
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
    printf("USB_NORMAL14_LATE_RX_GATE_ESP_BOOT_CHATTER_AND_TWO_FRAGMENT_SYNC=PASS NO_HARDWARE=YES\n");
    return 0;
}
