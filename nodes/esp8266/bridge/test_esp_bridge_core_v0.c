/* Pure-C bridge core simulation: ZERO device access / ZERO ESP reset.
 * Source-only test of STM32-targetable state machine under host GCC.
 */
#include "esp_bridge_core_v0.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    printf("FAIL line=%d expr=%s\n",__LINE__,#x); return 1; } } while (0)
typedef struct {
    int owned, quiescent, approved, enter_ok, fail_tx, fail_rx;
    int enters, leaves, tx_calls, rx_calls;
    uint8_t tx[160], rx[96];
    unsigned txlen, rxlen, rxpos;
} sim_t;

static int permit(void *p) {
    sim_t *s=(sim_t *)p;
    return s->owned && s->quiescent && s->approved;
}
static int enter(void *p) {
    sim_t *s=(sim_t *)p; ++s->enters; return s->enter_ok;
}
static void leave(void *p) { ++((sim_t *)p)->leaves; }
static int tx(void *p,const uint8_t *data,uint8_t length) {
    sim_t *s=(sim_t *)p; ++s->tx_calls;
    if (s->fail_tx || s->txlen+(unsigned)length>sizeof(s->tx)) return 0;
    (void)memcpy(s->tx+s->txlen,data,length);
    s->txlen+=length;
    return 1;
}
static int rx(void *p,uint8_t *out,uint8_t cap) {
    sim_t *s=(sim_t *)p; unsigned count,i;
    ++s->rx_calls;
    if (s->fail_rx) return -1;
    count=s->rxlen-s->rxpos;
    if (count>cap) count=cap;
    for(i=0;i<count;++i) out[i]=s->rx[s->rxpos+i];
    s->rxpos+=count;
    return (int)count;
}
static void init(esp_bridge_core_v0_t *core,sim_t *s,esp_bridge_io_v0_t *io) {
    (void)memset(s,0,sizeof(*s));
    s->owned=1; s->quiescent=1; s->approved=1; s->enter_ok=1;
    io->context=s; io->can_enter=permit; io->enter=enter; io->leave=leave;
    io->tx=tx; io->rx=rx;
    esp_bridge_core_v0_init(core);
}
static uint8_t invoke(esp_bridge_core_v0_t *core,const esp_bridge_io_v0_t *io,
 uint8_t op,uint32_t session,uint16_t seq,const uint8_t *data,uint8_t n,
 uint8_t *out,uint8_t *len)
{
    uint8_t input[64]; unsigned i;
    if(n>sizeof(input)-8u) return 255u;
    input[0]=op;
    for (i=0;i<4;++i) input[i+1]=(uint8_t)(session>>(8u*i));
    input[5]=(uint8_t)seq; input[6]=(uint8_t)(seq>>8u); input[7]=n;
    if(n!=0u && data!=0) (void)memcpy(input+8,data,n);
    *len=esp_bridge_core_v0_handle(core,io,input,(uint8_t)(n+8u),out,49u);
    return *len==0 ? 255u : out[7];
}
int main(void) {
    esp_bridge_core_v0_t core;
    esp_bridge_io_v0_t io;
    sim_t sim;
    uint8_t out[49],len,bytes[41],stop=0;
    const uint32_t sess=0xA1B2C3D4u;
    unsigned i;

    init(&core,&sim,&io);
    CHECK(invoke(&core,0,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(core.active==0 && sim.enters==0);
    sim.owned=0;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(sim.enters==0);
    sim.owned=1; sim.quiescent=0;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    sim.quiescent=1; sim.approved=0;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    sim.approved=1;
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,(const uint8_t *)"x",1,out,&len)==ESP_BRIDGE_BAD_SESSION);
    CHECK(sim.tx_calls==0);
    sim.enter_ok=0;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(!core.active);
    sim.enter_ok=1;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_OK);
    CHECK(core.active==1 && core.session==sess && len==9u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess+1u,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(core.session==sess);
    for(i=0;i<41;++i) bytes[i]=(uint8_t)i;
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,41,out,&len)==ESP_BRIDGE_BAD_LENGTH);
    CHECK(sim.tx_calls==0);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,2,bytes,4,out,&len)==ESP_BRIDGE_BAD_SEQUENCE);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess+1u,1,bytes,4,out,&len)==ESP_BRIDGE_BAD_SESSION);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,40,out,&len)==ESP_BRIDGE_OK);
    CHECK(sim.txlen==40u && sim.tx_calls==1 && len==9u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,40,out,&len)==ESP_BRIDGE_BAD_SEQUENCE);
    CHECK(sim.txlen==40u && sim.tx_calls==1);
    for(i=0;i<80;++i) sim.rx[i]=(uint8_t)(i+1u);
    sim.rxlen=80;
    CHECK(invoke(&core,&io,ESP_BRIDGE_RX,sess,2,0,0,out,&len)==ESP_BRIDGE_OK);
    CHECK(len==49u && out[8]==40u && out[9]==1u && out[48]==40u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_RX,sess,3,0,0,out,&len)==ESP_BRIDGE_OK);
    CHECK(len==49u && out[9]==41u && out[48]==80u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_RX,sess,4,0,0,out,&len)==ESP_BRIDGE_OK);
    CHECK(len==9u && out[8]==0u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_CLOSE,sess,5,0,0,out,&len)==ESP_BRIDGE_OK);
    CHECK(core.active==0u && sim.leaves==1 && sim.rxpos==80u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,6,bytes,1,out,&len)==ESP_BRIDGE_BAD_SESSION);

    init(&core,&sim,&io);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_OK);
    sim.fail_tx=1;
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,1,out,&len)==ESP_BRIDGE_UART_UNCERTAIN);
    CHECK(core.faulted==1u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,1,out,&len)==ESP_BRIDGE_BAD_SESSION);
    CHECK(sim.tx_calls==1);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess+1,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(invoke(&core,&io,ESP_BRIDGE_CLOSE,sess,1,0,0,out,&len)==ESP_BRIDGE_BAD_SESSION);

    init(&core,&sim,&io);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_OK);
    sim.fail_rx=1;
    CHECK(invoke(&core,&io,ESP_BRIDGE_RX,sess,1,0,0,out,&len)==ESP_BRIDGE_UART_UNCERTAIN);
    CHECK(core.faulted==1u);

    init(&core,&sim,&io);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_OK);
    esp_bridge_core_v0_abort(&core,&io);
    CHECK(core.faulted==1u && core.active==0u && sim.leaves==1);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess+1,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,bytes,1,out,&len)==ESP_BRIDGE_BAD_SESSION);

    init(&core,&sim,&io);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,0,0,0,0,out,&len)==ESP_BRIDGE_BAD_LENGTH);
    CHECK(invoke(&core,&io,99,sess,1,0,0,out,&len)==ESP_BRIDGE_BAD_LENGTH);
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,1,0,0,out,&len)==ESP_BRIDGE_BAD_LENGTH);
    CHECK(esp_bridge_core_v0_handle(&core,&io,bytes,7,out,49)==0u);
    CHECK(esp_bridge_core_v0_handle(&core,&io,bytes,8,out,48)==0u);
    CHECK(esp_bridge_core_v0_handle(&core,&io,bytes,8,0,49)==0u);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,&stop,1,out,&len)==ESP_BRIDGE_BAD_LENGTH);

    init(&core,&sim,&io);
    io.tx=0;
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_DENIED);
    CHECK(!core.active);
    init(&core,&sim,&io);
    CHECK(invoke(&core,&io,ESP_BRIDGE_OPEN,sess,0,0,0,out,&len)==ESP_BRIDGE_OK);
    core.last_sequence=0xFFFFu;
    CHECK(invoke(&core,&io,ESP_BRIDGE_TX,sess,0,bytes,1,out,&len)==ESP_BRIDGE_BAD_SEQUENCE);
    CHECK(sim.tx_calls==0);

    printf("ESP_BRIDGE_CORE_V0_OFFLINE=PASS NO_DEVICE_ACCESS=YES\n");
    return 0;
}
