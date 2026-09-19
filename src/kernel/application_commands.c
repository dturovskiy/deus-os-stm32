#include <stdint.h>
#include "kernel/application_commands.h"
#include "kernel/application_runtime_bridge.h"

static int application_commands_parse_id(
    const char *text,
    uint16_t *id_out)
{
    uint32_t value = 0u;
    uint32_t base = 10u;
    uint32_t index = 0u;
    uint32_t digits = 0u;

    if ((text == (const char *)0) || (id_out == (uint16_t *)0))
    {
        return 0;
    }

    if ((text[0] == '0') && ((text[1] == 'x') || (text[1] == 'X')))
    {
        base = 16u;
        index = 2u;
    }

    while (text[index] != '\0')
    {
        uint32_t digit;
        char c = text[index];

        if ((c >= '0') && (c <= '9'))
        {
            digit = (uint32_t)(c - '0');
        }
        else if ((base == 16u) && (c >= 'a') && (c <= 'f'))
        {
            digit = 10u + (uint32_t)(c - 'a');
        }
        else if ((base == 16u) && (c >= 'A') && (c <= 'F'))
        {
            digit = 10u + (uint32_t)(c - 'A');
        }
        else
        {
            return 0;
        }

        if (digit >= base)
        {
            return 0;
        }

        value = (value * base) + digit;
        if (value > 0xFFFFu)
        {
            return 0;
        }

        ++digits;
        ++index;
    }

    if ((digits == 0u) || (value == 0u))
    {
        return 0;
    }

    *id_out = (uint16_t)value;
    return 1;
}

static void application_commands_write(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write(context, text);
}

static void application_commands_write_line(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write_line(context, text);
}

static void application_commands_write_hex32(
    command_service_context_t *context,
    uint32_t value)
{
    (void)command_service_write_hex32(context, value);
}

static void application_commands_write_runtime_status(
    command_service_context_t *context,
    const application_runtime_bridge_status_t *status)
{
    application_commands_write(context, " APP_ACTIVE_ID=");
    application_commands_write_hex32(context, status->active_id);
    application_commands_write(context, " APP_FAULT_COUNT=");
    application_commands_write_hex32(context, status->fault_count);
    application_commands_write(context, " APP_EVENT_COUNT=");
    application_commands_write_hex32(context, status->event_count);
    application_commands_write(context, " APP_VIEW_REVISION=");
    application_commands_write_hex32(context, status->view_revision);
    application_commands_write(context, " APP_LAST_EVENT_TYPE=");
    application_commands_write_hex32(context, status->last_event_type);
    application_commands_write(context, " APP_LAST_EVENT_SOURCE=");
    application_commands_write_hex32(context, status->last_event_source);
    application_commands_write(context, "\r\n");
}

static command_service_status_t application_commands_applist(
    command_service_context_t *context)
{
    const application_runtime_bridge_status_t status =
        application_runtime_bridge_status();
    const uint32_t registry_count = application_runtime_registry_count();
    uint32_t index;
    const uint16_t active_id = status.active_id;

    application_commands_write(context, "APP_RUNTIME_ABI=");
    application_commands_write_hex32(context, APPLICATION_RUNTIME_ABI_VERSION);
    application_commands_write(context, " APP_REGISTRY_COUNT=");
    application_commands_write_hex32(
        context,
        registry_count);
    application_commands_write_runtime_status(
        context,
        &status);

    for (index = 0u; index < registry_count; ++index)
    {
        const application_descriptor_t *descriptor =
            application_runtime_registry_at(index);
        application_lifecycle_state_t state;

        if ((descriptor == (const application_descriptor_t *)0) ||
            (application_runtime_bridge_state_at(index, &state) == 0))
        {
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
        }

        application_commands_write(context, "APP_ID=");
        application_commands_write_hex32(context, descriptor->id);
        application_commands_write(context, " NAME=");
        application_commands_write(context, descriptor->name);
        application_commands_write(context, " ABI=");
        application_commands_write_hex32(context, descriptor->abi_version);
        application_commands_write(context, " FLAGS=");
        application_commands_write_hex32(context, descriptor->flags);
        application_commands_write(context, " STATE=");
        application_commands_write(
            context,
            application_runtime_state_name(state));
        application_commands_write(context, " STATE_ID=");
        application_commands_write_hex32(context, (uint32_t)state);
        application_commands_write(context, " ACTIVE=");
        application_commands_write_hex32(
            context,
            (descriptor->id == active_id) ? 1u : 0u);
        application_commands_write(context, "\r\n");
    }

    return COMMAND_SERVICE_STATUS_OK;
}

static command_service_status_t application_commands_appstart(
    const command_service_request_t *request,
    command_service_context_t *context,
    const application_service_snapshot_t *services)
{
    uint16_t id;

    if ((request->argc != 1u) ||
        (application_commands_parse_id(request->argv[0], &id) == 0) ||
        (application_runtime_find(id) == (const application_descriptor_t *)0))
    {
        return COMMAND_SERVICE_STATUS_BAD_ARGS;
    }

    if (services == (const application_service_snapshot_t *)0)
    {
        application_commands_write_line(context, "APP_RUNTIME_BUSY");
        return COMMAND_SERVICE_STATUS_BUSY;
    }

    if (application_runtime_bridge_start(id, services) == 0)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    application_commands_write(context, "APP_START_OK ID=");
    application_commands_write_hex32(context, id);
    application_commands_write(context, " ACTIVE_ID=");
    application_commands_write_hex32(
        context,
        application_runtime_bridge_status().active_id);
    application_commands_write(context, "\r\n");

    return COMMAND_SERVICE_STATUS_OK;
}

static command_service_status_t application_commands_appstop(
    command_service_context_t *context,
    const application_service_snapshot_t *services)
{
    if (services == (const application_service_snapshot_t *)0)
    {
        application_commands_write_line(context, "APP_RUNTIME_BUSY");
        return COMMAND_SERVICE_STATUS_BUSY;
    }

    if (application_runtime_bridge_stop(services) == 0)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    application_commands_write(context, "APP_STOP_OK ACTIVE_ID=");
    application_commands_write_hex32(
        context,
        application_runtime_bridge_status().active_id);
    application_commands_write(context, "\r\n");

    return COMMAND_SERVICE_STATUS_OK;
}

static command_service_status_t application_commands_rpcinfo(
    command_service_context_t *context)
{
    const application_runtime_bridge_status_t status =
        application_runtime_bridge_status();

    application_commands_write(context, "RPC_FOUNDATION_VERSION=");
    application_commands_write_hex32(
        context,
        COMMAND_SERVICE_FOUNDATION_VERSION);
    application_commands_write(context, " REGISTRY_COUNT=");
    application_commands_write_hex32(
        context,
        command_service_registry_count());
    application_commands_write(context, " LINE_CAPACITY=");
    application_commands_write_hex32(
        context,
        COMMAND_SERVICE_LINE_CAPACITY);
    application_commands_write(context, " MAX_ARGS=");
    application_commands_write_hex32(
        context,
        COMMAND_SERVICE_MAX_ARGS);
    application_commands_write(context, " APP_RUNTIME_ABI=");
    application_commands_write_hex32(
        context,
        APPLICATION_RUNTIME_ABI_VERSION);
    application_commands_write(context, " APP_REGISTRY_COUNT=");
    application_commands_write_hex32(
        context,
        application_runtime_registry_count());
    application_commands_write_runtime_status(
        context,
        &status);

    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t application_commands_execute(
    const command_service_request_t *request,
    command_service_context_t *context,
    const application_service_snapshot_t *services)
{
    switch (request->descriptor->method_id)
    {
        case COMMAND_SERVICE_METHOD_RPCINFO:
            return application_commands_rpcinfo(context);

        case COMMAND_SERVICE_METHOD_APPLIST:
            return application_commands_applist(context);

        case COMMAND_SERVICE_METHOD_APPSTART:
            return application_commands_appstart(
                request,
                context,
                services);

        case COMMAND_SERVICE_METHOD_APPSTOP:
            return application_commands_appstop(
                context,
                services);

        default:
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
}
