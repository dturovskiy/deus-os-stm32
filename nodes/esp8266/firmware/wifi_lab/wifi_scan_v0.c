/* SDK v3.4 Wi-Fi scanning: SOURCE-ONLY, not in existing ESP v2 app.
 * NO known network credentials, no NVS persistence, no transport protocol.
 */
#include <stdint.h>
#include <string.h>
#include "wifi_scan_v0.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "esp_err.h"

static deus_wifi_scan_v0_t g_scan;
static volatile uint8_t g_scan_done;
static volatile uint8_t g_scan_event_failed;

static esp_err_t deus_wifi_event(void *ctx, system_event_t *event)
{
    (void)ctx;
    if (event != (system_event_t *)0 &&
        event->event_id == SYSTEM_EVENT_SCAN_DONE &&
        g_scan.state == DEUS_WIFI_SCANNING) {
        /* Real SDK3.4 event_info.scan_done.status: 0 success, 1 failure.
         * Treat a failed RF scan as ERROR, never as an empty AP list.
         */
        g_scan_event_failed=(uint8_t)(event->event_info.scan_done.status!=0u);
        g_scan_done=1u;
    }
    return ESP_OK;
}
void deus_wifi_scan_v0_init(void)
{
    memset(&g_scan,0,sizeof(g_scan));
    g_scan.state=DEUS_WIFI_OFF;
    g_scan_done=0u;
    g_scan_event_failed=0u;
}
static int failed(esp_err_t result)
{
    g_scan.last_error=(int32_t)result;
    g_scan.state=DEUS_WIFI_ERROR;
    g_scan_done=0u;
    g_scan_event_failed=0u;
    return 0;
}
int deus_wifi_scan_v0_radio_start(void)
{
    esp_err_t rc;
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();
    if (g_scan.state != DEUS_WIFI_OFF) return 0;
    tcpip_adapter_init();
    rc=esp_event_loop_init(deus_wifi_event,(void *)0);
    if(rc!=ESP_OK)return failed(rc);
    rc=esp_wifi_init(&cfg);
    if(rc!=ESP_OK)return failed(rc);
    rc=esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if(rc!=ESP_OK)return failed(rc);
    rc=esp_wifi_set_mode(WIFI_MODE_STA);
    if(rc!=ESP_OK)return failed(rc);
    rc=esp_wifi_start();
    if(rc!=ESP_OK)return failed(rc);
    g_scan.state=DEUS_WIFI_READY;
    return 1;
}
int deus_wifi_scan_v0_start(uint32_t now_ms)
{
    esp_err_t rc;
    if(g_scan.state!=DEUS_WIFI_READY &&
       g_scan.state!=DEUS_WIFI_RESULTS) return 0;
    /* Generation 0 is forbidden on the wire. Do not silently wrap an
     * already valid UINT32_MAX scan ID and allow stale AP pages.
     */
    if(g_scan.generation==UINT32_MAX)return failed(ESP_FAIL);
    g_scan.count=0u;
    g_scan.truncated=0u;
    g_scan_done=0u;
    g_scan_event_failed=0u;
    g_scan.started_ms=now_ms;
    g_scan.state=DEUS_WIFI_SCANNING;
    rc=esp_wifi_scan_start((const wifi_scan_config_t *)0,0);
    if(rc!=ESP_OK)return failed(rc);
    ++g_scan.generation;
    return 1;
}
void deus_wifi_scan_v0_poll(uint32_t now_ms)
{
    wifi_ap_record_t found[DEUS_ESP_WIFI_SCAN_MAX_APS];
    uint16_t total=0u,request=DEUS_ESP_WIFI_SCAN_MAX_APS;
    esp_err_t rc;
    uint32_t i;
    if(g_scan.state!=DEUS_WIFI_SCANNING) return;
    if(g_scan_done==0u) {
        if((uint32_t)(now_ms-g_scan.started_ms)>DEUS_ESP_WIFI_SCAN_TIMEOUT_MS) {
            (void)esp_wifi_scan_stop();
            (void)failed(ESP_FAIL);
        }
        return;
    }
    g_scan_done=0u;
    if(g_scan_event_failed!=0u){(void)failed(ESP_FAIL);return;}
    rc=esp_wifi_scan_get_ap_num(&total);
    if(rc!=ESP_OK){(void)failed(rc);return;}
    if(total<request)request=total;
    if(request!=0u){
        memset(found,0,sizeof(found));
        rc=esp_wifi_scan_get_ap_records(&request,found);
        if(rc!=ESP_OK){(void)failed(rc);return;}
        for(i=0u;i<(uint32_t)request;++i){
            /* Preserve SSID bytes; never assume it is UTF-8. */
            memcpy(g_scan.aps[i].ssid,found[i].ssid,
                   DEUS_ESP_WIFI_SCAN_SSID_MAX);
            g_scan.aps[i].ssid[DEUS_ESP_WIFI_SCAN_SSID_MAX]=0u;
            g_scan.aps[i].channel=found[i].primary;
            g_scan.aps[i].rssi_dbm=found[i].rssi;
            g_scan.aps[i].authmode=(uint8_t)found[i].authmode;
        }
    } else {
        /* Some SDK builds require read-all to release scan dynamic storage;
         * defer this branch to SDK native validation if empty-list cleanup differs. */
    }
    g_scan.count=(uint8_t)request;
    g_scan.truncated=(uint8_t)(total>request ? 1u:0u);
    g_scan.state=DEUS_WIFI_RESULTS;
}
void deus_wifi_scan_v0_snapshot(deus_wifi_scan_v0_t *out)
{
    if(out!=(deus_wifi_scan_v0_t *)0)
        *out=g_scan;
}
