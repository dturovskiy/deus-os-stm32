#ifndef ESP_EVENT_LOOP_H
#define ESP_EVENT_LOOP_H
#include "esp_err.h"
#include <stdint.h>
typedef struct {
    int event_id;
    union {
        struct { uint32_t status; uint8_t number; uint8_t scan_id; } scan_done;
    } event_info;
} system_event_t;
#define SYSTEM_EVENT_SCAN_DONE 21
typedef esp_err_t (*system_event_handler_t)(void *,system_event_t *);
esp_err_t esp_event_loop_init(system_event_handler_t fn,void *ctx);
#endif
