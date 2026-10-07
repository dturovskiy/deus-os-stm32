#include <stdint.h>
#include "kernel/system_service_state.h"

#define SYSTEM_SERVICE_MAX_MINUTES      5999u
#define SYSTEM_SERVICE_SATURATE_MINUTES 6000u

static system_service_state_snapshot_t current_state;
static uint8_t uptime_saturated;

static uint32_t displayed_minute_from_uptime(uint32_t uptime_ms)
{
    const uint32_t total_minutes = uptime_ms / 60000u;

    if ((uptime_saturated != 0u) ||
        (total_minutes >= SYSTEM_SERVICE_SATURATE_MINUTES))
    {
        uptime_saturated = 1u;
        return SYSTEM_SERVICE_MAX_MINUTES;
    }

    return total_minutes;
}

void system_service_state_reset(void)
{
    current_state.uptime_ms = 0u;
    current_state.displayed_minute = 0u;
    current_state.system_healthy = 0u;
    current_state.usb_configured = 0u;
    current_state.network_online = 0u;
    uptime_saturated = 0u;
}

void system_service_state_update(
    uint32_t uptime_ms,
    uint8_t system_healthy,
    uint8_t usb_configured,
    uint8_t network_online)
{
    current_state.uptime_ms = uptime_ms;
    current_state.displayed_minute = displayed_minute_from_uptime(uptime_ms);
    current_state.system_healthy = (system_healthy != 0u) ? 1u : 0u;
    current_state.usb_configured = (usb_configured != 0u) ? 1u : 0u;
    current_state.network_online = (network_online != 0u) ? 1u : 0u;
}

void system_service_state_snapshot_get(
    system_service_state_snapshot_t *snapshot)
{
    if (snapshot != (system_service_state_snapshot_t *)0)
    {
        *snapshot = current_state;
    }
}
