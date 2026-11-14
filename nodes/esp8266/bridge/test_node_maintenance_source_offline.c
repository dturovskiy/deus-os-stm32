/* Offline Node Link versus ESP maintenance handover: no hardware.
 * Compile the ACTUAL src/kernel/node_transport.c + frozen DNL codec.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kernel/node_transport.h"
#include "drivers/usart2_node.h"
volatile uint32_t kernel_ticks;
static unsigned uart_arm, uart_disarm, uart_write;
static uint8_t uart_was_armed;
#define CHECK(x) do {if(!(x)){printf("FAIL line=%d %s\n",__LINE__,#x);return 2;}}while(0)
void usart2_node_init(void) {}
void usart2_node_rx_arm(void) {++uart_arm;uart_was_armed=1;}
void usart2_node_rx_disarm(void) {++uart_disarm;uart_was_armed=0;}
int usart2_node_try_read(usart2_node_sample_t *p) {(void)p;return 0;}
int usart2_node_write_frame(const uint8_t *bytes,uint32_t n) {
    CHECK(bytes!=0 && n>0u && n<=USART2_NODE_MAX_TX_BYTES);
    ++uart_write;return 1;
}
void usart2_node_diagnostics_get(usart2_node_diagnostics_t *p) {(void)p;}
int main(void) {
    node_transport_diagnostics_t diag;
    const uint8_t job[2]={1,2};
    node_transport_init(0u);
    node_transport_diagnostics_get(&diag);
    CHECK(diag.ready==0u && diag.pending==0u);
    CHECK(node_transport_maintenance_is_owned()==0u);
    CHECK(node_transport_maintenance_enter()==1);
    CHECK(uart_disarm==1 && uart_was_armed==0);
    CHECK(node_transport_maintenance_is_owned()==1u);
    CHECK(node_transport_maintenance_enter()==0);
    CHECK(node_transport_start(0xC0FFE001u,1u)==0);
    CHECK(node_transport_submit_crc32(job,2,1u)==0);
    kernel_ticks=3000;
    node_transport_service(3000u);
    CHECK(uart_write==0 && uart_arm==0); /* no synthetic HELLO, PING, retry */
    node_transport_maintenance_leave();
    CHECK(uart_disarm==2 && uart_was_armed==0);
    CHECK(node_transport_maintenance_is_owned()==0u);
    node_transport_maintenance_leave();
    CHECK(uart_disarm==2);
    node_transport_diagnostics_get(&diag);
    CHECK(diag.ready==0 && diag.pending==0);
    CHECK(node_transport_start(0xC0FFE001u,3001u)==1);
    CHECK(uart_write==1 && uart_arm==1 && uart_was_armed==1);
    CHECK(node_transport_maintenance_enter()==0); /* HELLO pending */
    CHECK(node_transport_maintenance_is_owned()==0u);
    CHECK(uart_disarm==3); /* failed enter does not clear active UART */
    printf("NODE_MAINTENANCE_SOURCE_EXCLUSIVE_OFFLINE=PASS NO_HARDWARE=YES\n");
    return 0;
}
