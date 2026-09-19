#include <stdint.h>
#include "kernel/application_runtime_bridge.h"

typedef char application_bridge_view_rows_must_match_oled_console[
    (APPLICATION_VIEW_ROWS == OLED_CONSOLE_ROWS) ? 1 : -1];
typedef char application_bridge_view_columns_must_match_oled_console[
    (APPLICATION_VIEW_COLUMNS == OLED_CONSOLE_COLUMNS) ? 1 : -1];

static application_runtime_t application_runtime_state;
static application_service_snapshot_t application_runtime_last_service_snapshot;
static uint32_t application_runtime_service_snapshot_valid;
static uint32_t application_runtime_ready_event_sent;

static void application_runtime_bridge_snapshot_clear(
    application_service_snapshot_t *snapshot)
{
    if (snapshot == (application_service_snapshot_t *)0)
    {
        return;
    }

    snapshot->uptime_ms = 0u;
    snapshot->displayed_minute = 0u;
    snapshot->system_healthy = 0u;
    snapshot->usb_configured = 0u;
    snapshot->network_online = 0u;
    snapshot->reserved = 0u;
}

static int application_runtime_bridge_dispatch_event(
    uint16_t type,
    uint16_t source,
    uint32_t value0,
    uint32_t value1,
    const application_service_snapshot_t *services)
{
    application_event_t event;

    if (services == (const application_service_snapshot_t *)0)
    {
        return 0;
    }

    event.type = type;
    event.source = source;
    event.value0 = value0;
    event.value1 = value1;

    return application_runtime_dispatch_event(
        &application_runtime_state,
        &event,
        services);
}

void application_runtime_bridge_reset(void)
{
    application_runtime_reset(&application_runtime_state);
    application_runtime_bridge_snapshot_clear(
        &application_runtime_last_service_snapshot);
    application_runtime_service_snapshot_valid = 0u;
    application_runtime_ready_event_sent = 0u;
}

int application_runtime_bridge_initialize(void)
{
    return application_runtime_initialize(&application_runtime_state);
}

int application_runtime_bridge_service(
    const application_service_snapshot_t *services)
{
    if (services == (const application_service_snapshot_t *)0)
    {
        return 0;
    }

    if (application_runtime_state.initialized == 0u)
    {
        return 0;
    }

    if (application_runtime_state.active_id == 0u)
    {
        if (application_runtime_start(
                &application_runtime_state,
                APPLICATION_ID_SYSTEM_HOME,
                services) == 0)
        {
            return 0;
        }
    }

    if (application_runtime_ready_event_sent == 0u)
    {
        if (application_runtime_bridge_dispatch_event(
                APPLICATION_EVENT_RUNTIME_READY,
                APPLICATION_EVENT_SOURCE_RUNTIME,
                application_runtime_state.active_id,
                0u,
                services) == 0)
        {
            return 0;
        }

        application_runtime_ready_event_sent = 1u;
    }

    if (application_runtime_service_snapshot_valid != 0u)
    {
        if (services->displayed_minute !=
            application_runtime_last_service_snapshot.displayed_minute)
        {
            if (application_runtime_bridge_dispatch_event(
                    APPLICATION_EVENT_UPTIME_MINUTE_CHANGED,
                    APPLICATION_EVENT_SOURCE_TIME,
                    services->displayed_minute,
                    application_runtime_last_service_snapshot.displayed_minute,
                    services) == 0)
            {
                return 0;
            }
        }

        if (services->system_healthy !=
            application_runtime_last_service_snapshot.system_healthy)
        {
            if (application_runtime_bridge_dispatch_event(
                    APPLICATION_EVENT_SYSTEM_HEALTH_CHANGED,
                    APPLICATION_EVENT_SOURCE_SYSTEM,
                    services->system_healthy,
                    application_runtime_last_service_snapshot.system_healthy,
                    services) == 0)
            {
                return 0;
            }
        }

        if (services->usb_configured !=
            application_runtime_last_service_snapshot.usb_configured)
        {
            if (application_runtime_bridge_dispatch_event(
                    APPLICATION_EVENT_USB_STATE_CHANGED,
                    APPLICATION_EVENT_SOURCE_USB,
                    services->usb_configured,
                    application_runtime_last_service_snapshot.usb_configured,
                    services) == 0)
            {
                return 0;
            }
        }

        if (services->network_online !=
            application_runtime_last_service_snapshot.network_online)
        {
            if (application_runtime_bridge_dispatch_event(
                    APPLICATION_EVENT_NETWORK_STATE_CHANGED,
                    APPLICATION_EVENT_SOURCE_NETWORK,
                    services->network_online,
                    application_runtime_last_service_snapshot.network_online,
                    services) == 0)
            {
                return 0;
            }
        }
    }

    application_runtime_last_service_snapshot = *services;
    application_runtime_service_snapshot_valid = 1u;

    return application_runtime_service(
        &application_runtime_state,
        services);
}

application_runtime_bridge_status_t application_runtime_bridge_status(void)
{
    application_runtime_bridge_status_t status;

    status.fault_count = application_runtime_state.fault_count;
    status.event_count = application_runtime_state.event_count;
    status.view_revision = application_runtime_state.view_revision;
    status.active_id = application_runtime_state.active_id;
    status.last_event_type = application_runtime_state.last_event_type;
    status.last_event_source = application_runtime_state.last_event_source;

    return status;
}

int application_runtime_bridge_is_initialized(void)
{
    return application_runtime_state.initialized != 0u;
}

int application_runtime_bridge_view_dirty(void)
{
    return application_runtime_state.view_dirty != 0u;
}

int application_runtime_bridge_state_at(
    uint32_t index,
    application_lifecycle_state_t *state_out)
{
    if ((state_out == (application_lifecycle_state_t *)0) ||
        (index >= APPLICATION_RUNTIME_REGISTRY_COUNT))
    {
        return 0;
    }

    *state_out =
        (application_lifecycle_state_t)application_runtime_state.states[index];

    return 1;
}

int application_runtime_bridge_start(
    uint16_t id,
    const application_service_snapshot_t *services)
{
    return application_runtime_start(
        &application_runtime_state,
        id,
        services);
}

int application_runtime_bridge_stop(
    const application_service_snapshot_t *services)
{
    return application_runtime_stop(
        &application_runtime_state,
        services);
}

int application_runtime_bridge_apply_view(
    oled_console_t *console,
    uint32_t force_rows)
{
    const application_view_t *view;
    uint32_t row;
    uint32_t column;
    uint8_t dirty_rows = 0u;

    if (console == (oled_console_t *)0)
    {
        return 0;
    }

    view = application_runtime_view_get(&application_runtime_state);
    if (view == (const application_view_t *)0)
    {
        return 0;
    }

    if ((force_rows != 0u) || (console->first_row != 0u))
    {
        dirty_rows =
            (uint8_t)((1u << APPLICATION_VIEW_ROWS) - 1u);
    }

    console->first_row = 0u;

    for (row = 0u; row < APPLICATION_VIEW_ROWS; ++row)
    {
        uint32_t text_ended = 0u;

        for (column = 0u; column < APPLICATION_VIEW_COLUMNS; ++column)
        {
            char desired = ' ';

            if (text_ended == 0u)
            {
                char source = view->rows[row][column];

                if (source == '\0')
                {
                    text_ended = 1u;
                }
                else
                {
                    desired = source;
                }
            }

            if (console->cells[row][column] != desired)
            {
                console->cells[row][column] = desired;
                dirty_rows |= (uint8_t)(1u << row);
            }
        }
    }

    console->cursor_x = 0u;
    console->cursor_y = (uint8_t)OLED_CONSOLE_ROWS;
    console->dirty_rows |= dirty_rows;

    return 1;
}

void application_runtime_bridge_view_consumed(void)
{
    application_runtime_view_consumed(&application_runtime_state);
}
