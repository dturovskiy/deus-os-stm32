/* Actual source-only BOOT/EN sequencing + INA226 telemetry in-memory mocks.
 * The code under test has NO direct MCU register writes / USB/UART access.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/esp_boot_control_v0.h"
#include "kernel/esp_power_monitor_v0.h"

#define REQUIRE(e) do{if(!(e)){fprintf(stderr,"FAIL line=%d %s\n",__LINE__,#e);exit(2);}}while(0)

typedef struct {
    uint8_t power_ok,node_idle,boot_low,en_low;
    uint8_t fail_gpio,fail_en;
    unsigned boot_calls,en_calls;
    char log[64]; unsigned log_used;
    uint8_t bad_i2c,reg_calls;
    uint16_t voltage_raw,shunt_raw;
} fake_t;

static void log_io(fake_t *x,char c)
{
    if(x->log_used<sizeof(x->log)-1u) {
        x->log[x->log_used++]=c;
        x->log[x->log_used]=0;
    }
}
static int gpio(void *ctx,uint8_t low)
{
    fake_t *x=(fake_t *)ctx;
    ++x->boot_calls;log_io(x,low?'B':'b');
    if(x->fail_gpio) return 0;
    x->boot_low=low;return 1;
}
static int en(void *ctx,uint8_t low)
{
    fake_t *x=(fake_t *)ctx;
    ++x->en_calls;log_io(x,low?'E':'e');
    if(x->fail_en) return 0;
    x->en_low=low;return 1;
}
static int rail(void *ctx){return ((fake_t *)ctx)->power_ok!=0u;}
static int node(void *ctx){return ((fake_t *)ctx)->node_idle!=0u;}
static int reg(void *ctx,uint8_t addr,uint8_t register_index,uint16_t *raw)
{
    fake_t *x=(fake_t *)ctx;
    REQUIRE(addr==ESP_POWER_INA226_ADDR);
    ++x->reg_calls;
    if(x->bad_i2c==1u) return -1;
    if(x->bad_i2c==2u) return 0;
    if(register_index==ESP_POWER_BUS_REG) *raw=x->voltage_raw;
    else if(register_index==ESP_POWER_SHUNT_REG) *raw=x->shunt_raw;
    else return 0;
    return 1;
}
static void init(fake_t *f,esp_boot_io_v0_t *i,esp_boot_control_v0_t *b)
{
    memset(f,0,sizeof(*f));f->power_ok=1;f->node_idle=1;
    i->gpio0_pull_low=gpio;i->en_pull_low=en;
    i->rail_verified=rail;i->node_quiescent=node;i->ctx=f;
    esp_boot_control_v0_init(b);
}
static void test_boot_roundtrip(void)
{
    fake_t f; esp_boot_io_v0_t io;esp_boot_control_v0_t b;
    init(&f,&io,&b);
    REQUIRE(b.state==ESP_BOOT_DISABLED && b.provisioned==0);
    REQUIRE(!esp_boot_control_v0_provision(&b,&io,0u,100u));
    REQUIRE(f.boot_calls==0 && f.en_calls==0);
    f.power_ok=0;
    REQUIRE(!esp_boot_control_v0_provision(&b,&io,1u,100u));
    f.power_ok=1;f.node_idle=0;
    REQUIRE(!esp_boot_control_v0_provision(&b,&io,1u,100u));
    f.node_idle=1;
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,100u));
    REQUIRE(strcmp(f.log,"be")==0 && b.state==ESP_BOOT_APP_AWAIT_HELLO);
    REQUIRE(!esp_boot_control_v0_rom_start(&b,&io,110u));
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(b.state==ESP_BOOT_NORMAL_LINK_VERIFIED);
    REQUIRE(!esp_boot_control_v0_note_fresh_hello(&b));
    f.node_idle=0;
    REQUIRE(!esp_boot_control_v0_rom_start(&b,&io,120u));
    f.node_idle=1;
    REQUIRE(esp_boot_control_v0_rom_start(&b,&io,120u));
    REQUIRE(strstr(f.log,"EB")!=NULL && f.en_low==1 && f.boot_low==1);
    esp_boot_control_v0_service(&b,&io,139u);
    REQUIRE(b.state==ESP_BOOT_ROM_EN_LOW && f.en_low==1);
    esp_boot_control_v0_service(&b,&io,140u);
    REQUIRE(b.state==ESP_BOOT_ROM_SETTLING && f.en_low==0 && f.boot_low==1);
    esp_boot_control_v0_service(&b,&io,239u);
    REQUIRE(b.state==ESP_BOOT_ROM_SETTLING);
    esp_boot_control_v0_service(&b,&io,240u);
    REQUIRE(b.state==ESP_BOOT_ROM_AWAIT_SYNC);
    REQUIRE(!esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(esp_boot_control_v0_app_start(&b,&io,250u));
    REQUIRE(b.state==ESP_BOOT_APP_EN_LOW && f.en_low==1 && f.boot_low==0);
    REQUIRE(strstr(f.log,"Eb")!=NULL);
    esp_boot_control_v0_service(&b,&io,270u);
    REQUIRE(b.state==ESP_BOOT_APP_SETTLING && f.en_low==0);
    esp_boot_control_v0_service(&b,&io,370u);
    REQUIRE(b.state==ESP_BOOT_APP_AWAIT_HELLO);
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(b.state==ESP_BOOT_NORMAL_LINK_VERIFIED && f.boot_low==0);
    puts("BOOT_ROUNDTRIP=PASS");
}
static void test_boot_failures(void)
{
    fake_t f;esp_boot_io_v0_t io;esp_boot_control_v0_t b;
    init(&f,&io,&b);f.fail_gpio=1;
    REQUIRE(!esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(b.state==ESP_BOOT_FAULT && b.provisioned==0u);
    REQUIRE(!esp_boot_control_v0_rom_start(&b,&io,1u));
    REQUIRE(!esp_boot_control_v0_provision(&b,&io,1u,2u));
    init(&f,&io,&b);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    f.fail_gpio=1;
    REQUIRE(!esp_boot_control_v0_rom_start(&b,&io,10u));
    REQUIRE(b.state==ESP_BOOT_FAULT && f.en_low==1);
    init(&f,&io,&b);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(esp_boot_control_v0_rom_start(&b,&io,10u));
    f.power_ok=0;
    esp_boot_control_v0_service(&b,&io,15u);
    REQUIRE(b.state==ESP_BOOT_FAULT && f.en_low==1);
    init(&f,&io,&b);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(esp_boot_control_v0_rom_start(&b,&io,10u));
    esp_boot_control_v0_service(&b,&io,1011u);
    REQUIRE(b.state==ESP_BOOT_FAULT && f.en_low==1);
    init(&f,&io,&b);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(esp_boot_control_v0_rom_start(&b,&io,10u));
    f.fail_en=1;
    esp_boot_control_v0_service(&b,&io,35u);
    REQUIRE(b.state==ESP_BOOT_FAULT && b.provisioned==0u);
    puts("BOOT_FAULTS=PASS");
}
static void test_normal_app_reset(void)
{
    fake_t f;esp_boot_io_v0_t io;esp_boot_control_v0_t b;
    init(&f,&io,&b);
    REQUIRE(!esp_boot_control_v0_normal_app_reset(&b,&io,1u,10u));
    REQUIRE(f.en_calls==0u && f.boot_calls==0u);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    REQUIRE(!esp_boot_control_v0_normal_app_reset(&b,&io,0u,10u));
    f.node_idle=0;
    REQUIRE(!esp_boot_control_v0_normal_app_reset(&b,&io,1u,10u));
    f.node_idle=1;
    REQUIRE(esp_boot_control_v0_normal_app_reset(&b,&io,1u,10u));
    REQUIRE(b.state==ESP_BOOT_APP_EN_LOW);
    REQUIRE(f.en_low==1u && f.boot_low==0u);
    REQUIRE(strcmp(f.log,"beEb")==0);
    REQUIRE(!esp_boot_control_v0_normal_app_reset(&b,&io,1u,11u));
    REQUIRE(!esp_boot_control_v0_rom_start(&b,&io,11u));
    esp_boot_control_v0_service(&b,&io,29u);
    REQUIRE(b.state==ESP_BOOT_APP_EN_LOW && f.en_low==1u);
    esp_boot_control_v0_service(&b,&io,30u);
    REQUIRE(b.state==ESP_BOOT_APP_SETTLING && f.en_low==0u);
    esp_boot_control_v0_service(&b,&io,129u);
    REQUIRE(b.state==ESP_BOOT_APP_SETTLING);
    esp_boot_control_v0_service(&b,&io,130u);
    REQUIRE(b.state==ESP_BOOT_APP_AWAIT_HELLO && f.boot_low==0u);
    REQUIRE(esp_boot_control_v0_note_fresh_hello(&b));
    REQUIRE(b.state==ESP_BOOT_NORMAL_LINK_VERIFIED);
    REQUIRE(esp_boot_control_v0_normal_app_reset(&b,&io,1u,200u));
    esp_boot_control_v0_service(&b,&io,220u);
    esp_boot_control_v0_service(&b,&io,320u);
    REQUIRE(b.state==ESP_BOOT_APP_AWAIT_HELLO);
    init(&f,&io,&b);
    REQUIRE(esp_boot_control_v0_provision(&b,&io,1u,0u));
    f.fail_gpio=1;
    REQUIRE(!esp_boot_control_v0_normal_app_reset(&b,&io,1u,1u));
    REQUIRE(b.state==ESP_BOOT_FAULT && f.en_low==1u);
    puts("NORMAL_APP_RESET_GUARDS_EN_PULSE_NO_ROM=PASS");
}
static void test_power_monitor(void)
{
    fake_t f;esp_power_monitor_v0_t m;esp_power_bus_v0_t bus;
    memset(&f,0,sizeof(f));bus.read_reg16=reg;bus.ctx=&f;
    esp_power_monitor_v0_init(&m);
    REQUIRE(esp_power_monitor_v0_status(&m,0u)==ESP_POWER_NOT_AVAILABLE);
    REQUIRE(!esp_power_monitor_v0_configure(&m,0u));
    REQUIRE(!esp_power_monitor_v0_configure(&m,9u));
    REQUIRE(!esp_power_monitor_v0_configure(&m,1001u));
    esp_power_monitor_v0_sample(&m,&bus,5u);
    REQUIRE(m.status==ESP_POWER_NOT_AVAILABLE && f.reg_calls==0u);
    REQUIRE(esp_power_monitor_v0_configure(&m,50u));
    esp_power_monitor_v0_sample(&m,0,10u);
    REQUIRE(m.status==ESP_POWER_NOT_AVAILABLE && m.has_sample==0);
    f.voltage_raw=2640u; /* 3300 mV @1.25 mV/LSB */
    f.shunt_raw=10000u; /* 500 mA @ 50 mOhm */
    esp_power_monitor_v0_sample(&m,&bus,100u);
    REQUIRE(f.reg_calls==2u);
    REQUIRE(m.status==ESP_POWER_VALID && m.bus_mv==3300u);
    REQUIRE(m.current_ma==500 && m.power_mw==1650);
    REQUIRE(esp_power_monitor_v0_status(&m,1100u)==ESP_POWER_VALID);
    REQUIRE(esp_power_monitor_v0_status(&m,1101u)==ESP_POWER_STALE);
    f.shunt_raw=(uint16_t)(int16_t)-20;
    esp_power_monitor_v0_sample(&m,&bus,1200u);
    REQUIRE(m.current_ma==-1 && m.power_mw==-3);
    f.voltage_raw=2240u; /* 2800 mV: not safe to report OK */
    esp_power_monitor_v0_sample(&m,&bus,1400u);
    REQUIRE(esp_power_monitor_v0_status(&m,1400u)==ESP_POWER_OUT_OF_RANGE);
    f.bad_i2c=1;
    esp_power_monitor_v0_sample(&m,&bus,1500u);
    REQUIRE(esp_power_monitor_v0_status(&m,1500u)==ESP_POWER_IO_ERROR);
    REQUIRE(m.has_sample==0u);
    f.bad_i2c=2u;
    esp_power_monitor_v0_sample(&m,&bus,1600u);
    REQUIRE(esp_power_monitor_v0_status(&m,1600u)==ESP_POWER_NOT_AVAILABLE);
    f.bad_i2c=0;f.voltage_raw=2640;f.shunt_raw=0;
    esp_power_monitor_v0_sample(&m,&bus,0xFFFFFFF0u);
    REQUIRE(m.current_ma==0 && m.status==ESP_POWER_VALID);
    REQUIRE(esp_power_monitor_v0_status(&m,0x00000014u)==ESP_POWER_VALID);
    REQUIRE(esp_power_monitor_v0_status(&m,0x00000400u)==ESP_POWER_STALE);
    f.shunt_raw=10000u;
    REQUIRE(esp_power_monitor_v0_configure(&m,100u));
    esp_power_monitor_v0_sample(&m,&bus,10u);
    REQUIRE(m.current_ma==250 && m.power_mw==825);
    puts("INA226_TELEMETRY=PASS");
}
int main(void)
{
    test_boot_roundtrip();
    test_boot_failures();
    test_normal_app_reset();
    test_power_monitor();
    puts("ESP_BOOT_AND_POWER_V0_OFFLINE=PASS NO_GPIO_NO_USB_NO_I2C_NO_FLASH");
    return 0;
}
