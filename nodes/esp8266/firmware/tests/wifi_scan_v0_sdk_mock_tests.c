/* Real WiFi scan_v0.c linked against strictly bounded in-memory
 * ESP8266 SDK v3.4 stand-ins. Never accesses real RF, USB or STM32.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wifi_scan_v0.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#define OK(x) do {if(!(x)){fprintf(stderr,"FAIL line=%d %s\n",__LINE__,#x);exit(2);}}while(0)
static system_event_handler_t ev;
static void *ev_ctx;
static int step_failure; /* 1 event,2 wifi_init,3 storage,4 mode,5 start,6 scan,7 count,8 results */
static uint32_t wifi_calls, scan_calls, scan_stop_calls, get_records_calls;
static uint16_t total_aps;
static wifi_ap_record_t mock_aps[6];
static int mode_seen,storage_seen,nonblock_seen;
void tcpip_adapter_init(void){++wifi_calls;}
esp_err_t esp_event_loop_init(system_event_handler_t fn,void *ctx)
{
    ++wifi_calls;ev=fn;ev_ctx=ctx;return step_failure==1?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_init(const wifi_init_config_t *cfg)
{
    ++wifi_calls;OK(cfg && cfg->magic==777);
    return step_failure==2?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_set_storage(int n)
{
    ++wifi_calls;storage_seen=n;return step_failure==3?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_set_mode(int n)
{
    ++wifi_calls;mode_seen=n;return step_failure==4?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_start(void)
{
    ++wifi_calls;return step_failure==5?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *cfg,bool block)
{
    ++scan_calls;OK(cfg==0 && block==false);
    nonblock_seen=1;
    return step_failure==6?ESP_FAIL:ESP_OK;
}
esp_err_t esp_wifi_scan_stop(void){++scan_stop_calls;return ESP_OK;}
esp_err_t esp_wifi_scan_get_ap_num(uint16_t *number)
{
    OK(number!=0);
    if(step_failure==7)return ESP_FAIL;
    *number=total_aps;return ESP_OK;
}
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *number,wifi_ap_record_t *records)
{
    uint16_t i;
    ++get_records_calls;
    if(step_failure==8)return ESP_FAIL;
    OK(number && records);
    OK(*number<=DEUS_ESP_WIFI_SCAN_MAX_APS);
    for(i=0u;i<*number;++i)records[i]=mock_aps[i];
    return ESP_OK;
}
static void reset_all(void)
{
    memset(mock_aps,0,sizeof(mock_aps));
    wifi_calls=scan_calls=scan_stop_calls=get_records_calls=0;
    total_aps=0;
    step_failure=0;ev=0;ev_ctx=0;mode_seen=storage_seen=nonblock_seen=0;
    deus_wifi_scan_v0_init();
}
static deus_wifi_scan_v0_t snapshot(void)
{
    deus_wifi_scan_v0_t s; memset(&s,0xCD,sizeof(s));
    deus_wifi_scan_v0_snapshot(&s);return s;
}
static void done(void)
{
    system_event_t s={0};
    s.event_id=SYSTEM_EVENT_SCAN_DONE;
    OK(ev!=0);
    OK(ev(ev_ctx,&s)==ESP_OK);
}
int main(void)
{
    deus_wifi_scan_v0_t state;
    uint32_t i;
    reset_all();
    state=snapshot();
    OK(state.state==DEUS_WIFI_OFF && wifi_calls==0 && scan_calls==0);
    OK(!deus_wifi_scan_v0_start(0u)); /* no start until explicit init */
    OK(deus_wifi_scan_v0_radio_start());
    OK(mode_seen==WIFI_MODE_STA && storage_seen==WIFI_STORAGE_RAM && wifi_calls==6);
    OK(!deus_wifi_scan_v0_radio_start()); /* no double init */
    OK(deus_wifi_scan_v0_start(900u) && nonblock_seen);
    state=snapshot();
    OK(state.state==DEUS_WIFI_SCANNING && state.generation==1);
    OK(!deus_wifi_scan_v0_start(901u));
    /* Unrelated SDK event does not complete. */
    {system_event_t other={0};other.event_id=3;OK(ev(ev_ctx,&other)==ESP_OK);}
    deus_wifi_scan_v0_poll(1000u);
    OK(snapshot().state==DEUS_WIFI_SCANNING && get_records_calls==0);
    total_aps=6;
    for(i=0;i<6u;++i){
        memset(mock_aps[i].ssid,(int)('A'+i),32u);
        mock_aps[i].primary=(uint8_t)(i+1u);
        mock_aps[i].rssi=(int8_t)(-33-(int)i);
        mock_aps[i].authmode=(int)(i%3u);
    }
    done();
    deus_wifi_scan_v0_poll(1200u);
    state=snapshot();
    OK(state.state==DEUS_WIFI_RESULTS && state.count==4u && state.truncated==1u);
    OK(state.aps[0].ssid[0]=='A' && state.aps[0].ssid[32]==0u);
    OK(state.aps[3].channel==4u && state.aps[3].rssi_dbm==-36);
    OK(get_records_calls==1u);
    OK(deus_wifi_scan_v0_start(2000u));
    total_aps=0u;done();deus_wifi_scan_v0_poll(2001u);
    state=snapshot();
    OK(state.state==DEUS_WIFI_RESULTS && state.count==0u && state.truncated==0u);
    OK(state.generation==2u);
    OK(deus_wifi_scan_v0_start(0xFFFFFFF0u));
    deus_wifi_scan_v0_poll(0x00000010u);
    OK(snapshot().state==DEUS_WIFI_SCANNING);
    deus_wifi_scan_v0_poll(0x00002000u);
    OK(snapshot().state==DEUS_WIFI_ERROR && scan_stop_calls==1u);
    OK(!deus_wifi_scan_v0_start(0x00003000u));

    for(i=1u;i<=8u;++i) {
        if(i==7u || i==8u){
            reset_all();OK(deus_wifi_scan_v0_radio_start());
            OK(deus_wifi_scan_v0_start(0u));
            total_aps=1u;done();
            step_failure=(int)i;deus_wifi_scan_v0_poll(1u);
        } else if(i==6u) {
            reset_all();OK(deus_wifi_scan_v0_radio_start());
            step_failure=(int)i;
            OK(!deus_wifi_scan_v0_start(0u));
        } else {
            reset_all();step_failure=(int)i;
            OK(!deus_wifi_scan_v0_radio_start());
        }
        OK(snapshot().state==DEUS_WIFI_ERROR);
        OK(!deus_wifi_scan_v0_start(5u));
    }
    /* Failed scan-completion event must not publish fake empty results. */
    reset_all();OK(deus_wifi_scan_v0_radio_start());
    OK(deus_wifi_scan_v0_start(200u));
    {system_event_t bad={0};bad.event_id=SYSTEM_EVENT_SCAN_DONE;
     bad.event_info.scan_done.status=1u;
     OK(ev(ev_ctx,&bad)==ESP_OK);}
    deus_wifi_scan_v0_poll(201u);
    OK(snapshot().state==DEUS_WIFI_ERROR && get_records_calls==0u);
    OK(!deus_wifi_scan_v0_start(202u));
    puts("ESP_WIFI_SCAN_SDK_V34_API_MOCK=PASS START_GUARDED_ASYNC_MAX4_SCAN_TIMEOUT_ERRORS_NO_CREDENTIALS");
    return 0;
}
