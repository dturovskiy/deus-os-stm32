#ifndef KERNEL_SYSTEM_SERVICE_STATE_H
#define KERNEL_SYSTEM_SERVICE_STATE_H

#include <stdint.h>

typedef struct
{
    uint32_t uptime_ms;
    uint32_t displayed_minute;
    uint8_t system_healthy;
    uint8_t usb_configured;
    uint8_t network_online;
} system_service_state_snapshot_t;

void system_service_state_reset(void);
void system_service_state_update(
    uint32_t uptime_ms,
    uint8_t system_healthy,
    uint8_t usb_configured,
    uint8_t network_online);
void system_service_state_snapshot_get(
    system_service_state_snapshot_t *snapshot);

#endif
