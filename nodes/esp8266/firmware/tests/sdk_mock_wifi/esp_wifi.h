#ifndef ESP_WIFI_H
#define ESP_WIFI_H
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
typedef struct {int magic;} wifi_init_config_t;
#define WIFI_INIT_CONFIG_DEFAULT() ((wifi_init_config_t){.magic=777})
typedef struct {int dummy;} wifi_scan_config_t;
#define WIFI_STORAGE_RAM 1
#define WIFI_MODE_STA 1
typedef struct {
 uint8_t bssid[6];
 uint8_t ssid[33];
 uint8_t primary;
 int8_t rssi;
 int authmode;
} wifi_ap_record_t;
esp_err_t esp_wifi_init(const wifi_init_config_t *cfg);
esp_err_t esp_wifi_set_storage(int storage);
esp_err_t esp_wifi_set_mode(int mode);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_scan_start(const wifi_scan_config_t *cfg,bool block);
esp_err_t esp_wifi_scan_stop(void);
esp_err_t esp_wifi_scan_get_ap_num(uint16_t *count);
esp_err_t esp_wifi_scan_get_ap_records(uint16_t *count,wifi_ap_record_t *records);
#endif
