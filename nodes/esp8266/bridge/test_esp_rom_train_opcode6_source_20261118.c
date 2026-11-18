/* Source-only actual esp_bridge_core_v0.c mock regression.
 * NEVER opens USB, GPIO, ESP ROM, credentials or physical devices. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/esp_bridge_core_v0.h"
#define CHECK(expr) do {if(!(expr)) {fprintf(stderr, "ROM_TRAIN_FAIL:%d:%s\\n",__LINE__,#expr); return 2;}}while(0)
typedef struct {
    unsigned enters, leaves, tx_calls, rx_calls, total_bytes;
    int tx_ok, rx_ok;
    uint8_t wire[46*5];
} mock_t;
static int can_enter(void *ctx) {(void)ctx;return 1;}
static int enter(void *ctx) {++((mock_t*)ctx)->enters;return 1;}
static void leave(void *ctx) {++((mock_t*)ctx)->leaves;}
static int tx(void *ctx,const uint8_t *buf,uint8_t length) {
    mock_t *m=(mock_t*)ctx; ++m->tx_calls;
    if (!m->tx_ok || m->total_bytes+length>sizeof(m->wire)) return 0;
    memcpy(m->wire+m->total_bytes,buf,length);
    m->total_bytes+=length;return 1;
}
static int rx(void *ctx,uint8_t *b,uint8_t capacity) {
    mock_t *m=(mock_t*)ctx;(void)b;(void)capacity;
    ++m->rx_calls;return m->rx_ok?0:-1;
}
static void setup(esp_bridge_core_v0_t *s,mock_t *m,esp_bridge_io_v0_t *io) {
    memset(m,0,sizeof(*m));m->tx_ok=1;m->rx_ok=1;
    *io=(esp_bridge_io_v0_t){can_enter,enter,leave,tx,rx,m};
    esp_bridge_core_v0_init(s);
}
static uint8_t call(esp_bridge_core_v0_t *s,esp_bridge_io_v0_t *io,
                    uint8_t op,uint16_t seq,const uint8_t *data,uint8_t length) {
    uint8_t in[48]={0},out[49]={0},result;
    if (length>40) return 255;
    in[0]=op;in[1]=0xFA;in[2]=0x10;in[3]=0xB0;in[4]=0x12;
    in[5]=(uint8_t)seq;in[6]=(uint8_t)(seq>>8);in[7]=length;
    if(length!=0)memcpy(in+8,data,length);
    result=esp_bridge_core_v0_handle(s,io,in,(uint8_t)(8+length),out,49);
    return result==9u?out[7]:255;
}
static void make_sync(uint8_t b[46]) {
    unsigned i;memset(b,0,46);b[0]=0xC0;b[2]=8;b[3]=36;
    b[9]=7;b[10]=7;b[11]=0x12;b[12]=0x20;
    for(i=13;i<45;++i) b[i]=0x55;
    b[45]=0xC0;
}
int main(void) {
    esp_bridge_core_v0_t s;esp_bridge_io_v0_t io;mock_t m;
    uint8_t good[46],bad[46];uint16_t seq;unsigned burst;
    make_sync(good);memcpy(bad,good,sizeof(bad));bad[13]^=1;
    /* Old opcode5 is immutable: exact one TX46, reject second. */
    setup(&s,&m,&io);
    CHECK(call(&s,&io,ESP_BRIDGE_OPEN_ROM_SYNC_ONLY,0,0,0)==ESP_BRIDGE_OK);
    CHECK(s.rom_sync_only==1 && s.uart_attempted==0u);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,1,good,40)==ESP_BRIDGE_OK);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,2,good+40,6)==ESP_BRIDGE_OK);
    CHECK(s.uart_attempted==1u && m.tx_calls==1);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,3,good,40)==ESP_BRIDGE_DENIED);
    CHECK(m.tx_calls==1 && m.total_bytes==46);
    CHECK(call(&s,&io,ESP_BRIDGE_CLOSE,3,0,0)==ESP_BRIDGE_OK);
    CHECK(s.rom_sync_only==0 && s.uart_attempted==0);
    puts("OPCODE5_LEGACY_ONE_EXACT_SYNC_ONLY=PASS");
    /* New opcode6: same ROM reset, five strictly bounded 46-byte frames. */
    setup(&s,&m,&io);
    CHECK(call(&s,&io,ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN,0,0,0)==ESP_BRIDGE_OK);
    CHECK(s.rom_sync_only==2 && s.uart_attempted==0u);
    CHECK(call(&s,&io,ESP_BRIDGE_RX,1,0,0)==ESP_BRIDGE_DENIED);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,1,bad,40)==ESP_BRIDGE_DENIED);
    CHECK(s.last_sequence==0 && m.tx_calls==0);
    seq=0;
    for(burst=0;burst<5;burst++) {
        ++seq;
        CHECK(call(&s,&io,ESP_BRIDGE_TX,seq,good,40)==ESP_BRIDGE_OK);
        CHECK(s.rom_sync_tx_offset==40 && m.tx_calls==burst);
        CHECK(call(&s,&io,ESP_BRIDGE_RX,(uint16_t)(seq+1),0,0)==ESP_BRIDGE_DENIED);
        CHECK(s.last_sequence==seq);
        ++seq;
        CHECK(call(&s,&io,ESP_BRIDGE_TX,seq,good+40,6)==ESP_BRIDGE_OK);
        CHECK(s.rom_sync_tx_offset==46 && s.uart_attempted==burst+1);
        CHECK(m.tx_calls==burst+1 && m.total_bytes==46*(burst+1));
        CHECK(memcmp(m.wire+46*burst,good,46)==0);
        ++seq;
        CHECK(call(&s,&io,ESP_BRIDGE_RX,seq,0,0)==ESP_BRIDGE_OK);
        CHECK(m.rx_calls==burst+1);
    }
    CHECK(call(&s,&io,ESP_BRIDGE_TX,(uint16_t)(seq+1),good,40)==ESP_BRIDGE_DENIED);
    CHECK(m.tx_calls==5 && m.total_bytes==230);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,(uint16_t)(seq+1),bad,40)==ESP_BRIDGE_DENIED);
    CHECK(call(&s,&io,ESP_BRIDGE_CLOSE,(uint16_t)(seq+1),0,0)==ESP_BRIDGE_OK);
    CHECK(m.enters==1 && m.leaves==1 && !s.active && !s.rom_sync_only);
    puts("OPCODE6_FIVE_EXACT_ATOMIC_SYNCS_230_BYTES_NO_SIXTH=PASS");
    /* Firmware errors must abort and recover locally without Flash. */
    setup(&s,&m,&io);m.tx_ok=0;
    CHECK(call(&s,&io,ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN,0,0,0)==ESP_BRIDGE_OK);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,1,good,40)==ESP_BRIDGE_OK);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,2,good+40,6)==ESP_BRIDGE_UART_UNCERTAIN);
    esp_bridge_core_v0_abort(&s,&io);
    CHECK(!s.active && !s.faulted && !s.rom_sync_only && m.leaves==1);
    puts("OPCODE6_UART_FAIL_CLOSE_FAILSAFE_NO_FLASH=PASS");
    /* Only ROM op6, never arbitrary generic Flash, can use train mode. */
    setup(&s,&m,&io);
    CHECK(call(&s,&io,ESP_BRIDGE_OPEN_ROM_SYNC_TRAIN,0,0,0)==ESP_BRIDGE_OK);
    CHECK(call(&s,&io,ESP_BRIDGE_TX,1,bad,40)==ESP_BRIDGE_DENIED);
    CHECK(m.total_bytes==0 && m.tx_calls==0);
    esp_bridge_core_v0_abort(&s,&io);
    puts("ROM_TRAIN_ACTUAL_SOURCE_OFFLINE=PASS NO_DEVICE_ACCESS");
    return 0;
}
