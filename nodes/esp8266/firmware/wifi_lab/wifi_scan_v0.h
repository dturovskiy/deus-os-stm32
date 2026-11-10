#ifndef DEUS_ESP_WIFI_SCAN_V0_H
#define DEUS_ESP_WIFI_SCAN_V0_H
#include <stdint.h>

/* ESP8266 RTOS SDK v3.4 Wi-Fi STA / bounded AP scan service.
 * SOURCE-ONLY candidate; NOT wired to existing Node Link REV2, NOT
 * built/flashed or run on ESP hardware. No SSID/password stored.
 * All functions called in an ESP application TASK, not ISR / callback.
 */
#define DEUS_ESP_WIFI_SCAN_MAX_APS 4u
#define DEUS_ESP_WIFI_SCAN_SSID_MAX 32u
#define DEUS_ESP_WIFI_SCAN_TIMEOUT_MS 8000u
typedef enum {
    DEUS_WIFI_OFF=0,
    DEUS_WIFI_READY=1,
    DEUS_WIFI_SCANNING=2,
    DEUS_WIFI_RESULTS=3,
    DEUS_WIFI_ERROR=4
} deus_wifi_scan_state_v0_t;
typedef struct {
    uint8_t ssid[DEUS_ESP_WIFI_SCAN_SSID_MAX+1u];
    uint8_t channel;
    int8_t rssi_dbm;
    uint8_t authmode;
} deus_wifi_ap_v0_t;
typedef struct {
    deus_wifi_scan_state_v0_t state;
    uint8_t count;
    uint8_t truncated;
    uint8_t reserved;
    int32_t last_error;
    uint32_t generation;
    uint32_t started_ms;
    deus_wifi_ap_v0_t aps[DEUS_ESP_WIFI_SCAN_MAX_APS];
} deus_wifi_scan_v0_t;

void deus_wifi_scan_v0_init(void);
/* Enters STA-only mode, no credentials, no AP join, no TCP, no NodeLink TX. */
int deus_wifi_scan_v0_radio_start(void);
/* Starts one nonblocking scan, never scans automatically or retries errors. */
int deus_wifi_scan_v0_start(uint32_t now_ms);
/* Call periodically from a future dedicated ESP task; no UART starvation. */
void deus_wifi_scan_v0_poll(uint32_t now_ms);
void deus_wifi_scan_v0_snapshot(deus_wifi_scan_v0_t *out);
#endif
