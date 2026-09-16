#include <stdint.h>
#include "kernel/command_service.h"

#define COMMAND_ENTRY(name_, rpc_, id_, class_, min_, max_) \
    { (name_), (rpc_), (id_), (class_), (min_), (max_) }

static const command_service_descriptor_t command_registry[] =
{
    COMMAND_ENTRY("ping", COMMAND_SERVICE_RPC_PING, COMMAND_SERVICE_METHOD_PING, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("uptime", COMMAND_SERVICE_RPC_UPTIME, COMMAND_SERVICE_METHOD_UPTIME, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("health", COMMAND_SERVICE_RPC_HEALTH, COMMAND_SERVICE_METHOD_HEALTH, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("rxstat", COMMAND_SERVICE_RPC_RXSTAT, COMMAND_SERVICE_METHOD_RXSTAT, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("cdcstat", COMMAND_SERVICE_RPC_CDCSTAT, COMMAND_SERVICE_METHOD_CDCSTAT, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("mspstat", COMMAND_SERVICE_RPC_MSPSTAT, COMMAND_SERVICE_METHOD_MSPSTAT, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("schedprod", COMMAND_SERVICE_RPC_SCHEDPROD, COMMAND_SERVICE_METHOD_SCHEDPROD, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("schedtimed", COMMAND_SERVICE_RPC_SCHEDTIMED, COMMAND_SERVICE_METHOD_SCHEDTIMED, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("schedprio", COMMAND_SERVICE_RPC_SCHEDPRIO, COMMAND_SERVICE_METHOD_SCHEDPRIO, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("fault", COMMAND_SERVICE_RPC_FAULT, COMMAND_SERVICE_METHOD_FAULT, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("i2cscan", COMMAND_SERVICE_RPC_I2CSCAN, COMMAND_SERVICE_METHOD_I2CSCAN, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledping", COMMAND_SERVICE_RPC_OLEDPING, COMMAND_SERVICE_METHOD_OLEDPING, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledtest", COMMAND_SERVICE_RPC_OLEDTEST, COMMAND_SERVICE_METHOD_OLEDTEST, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledtext", COMMAND_SERVICE_RPC_OLEDTEXT, COMMAND_SERVICE_METHOD_OLEDTEXT, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledrender", COMMAND_SERVICE_RPC_OLEDRENDER, COMMAND_SERVICE_METHOD_OLEDRENDER, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledconsole", COMMAND_SERVICE_RPC_OLEDCONSOLE, COMMAND_SERVICE_METHOD_OLEDCONSOLE, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledscroll", COMMAND_SERVICE_RPC_OLEDSCROLL, COMMAND_SERVICE_METHOD_OLEDSCROLL, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oleddirty", COMMAND_SERVICE_RPC_OLEDDIRTY, COMMAND_SERVICE_METHOD_OLEDDIRTY, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oleduiupdate", COMMAND_SERVICE_RPC_OLEDUIUPDATE, COMMAND_SERVICE_METHOD_OLEDUIUPDATE, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("uiruntime", COMMAND_SERVICE_RPC_UIRUNTIME, COMMAND_SERVICE_METHOD_UIRUNTIME, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("oledstatus", COMMAND_SERVICE_RPC_OLEDSTATUS, COMMAND_SERVICE_METHOD_OLEDSTATUS, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u),
    COMMAND_ENTRY("schedtest", COMMAND_SERVICE_RPC_SCHEDTEST, COMMAND_SERVICE_METHOD_SCHEDTEST, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedcoop", COMMAND_SERVICE_RPC_SCHEDCOOP, COMMAND_SERVICE_METHOD_SCHEDCOOP, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedpreempt", COMMAND_SERVICE_RPC_SCHEDPREEMPT, COMMAND_SERVICE_METHOD_SCHEDPREEMPT, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedstack", COMMAND_SERVICE_RPC_SCHEDSTACK, COMMAND_SERVICE_METHOD_SCHEDSTACK, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedworkload", COMMAND_SERVICE_RPC_SCHEDWORKLOAD, COMMAND_SERVICE_METHOD_SCHEDWORKLOAD, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedconsoleprobe", COMMAND_SERVICE_RPC_SCHEDCONSOLEPROBE, COMMAND_SERVICE_METHOD_SCHEDCONSOLEPROBE, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedwaitwake", COMMAND_SERVICE_RPC_SCHEDWAITWAKE, COMMAND_SERVICE_METHOD_SCHEDWAITWAKE, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("schedisolate", COMMAND_SERVICE_RPC_SCHEDISOLATE, COMMAND_SERVICE_METHOD_SCHEDISOLATE, COMMAND_SERVICE_CLASS_DIAGNOSTIC, 0u, 0u),
    COMMAND_ENTRY("wdogtrip", COMMAND_SERVICE_RPC_WDOGTRIP, COMMAND_SERVICE_METHOD_WDOGTRIP, COMMAND_SERVICE_CLASS_DESTRUCTIVE, 0u, 0u),
    COMMAND_ENTRY("help", COMMAND_SERVICE_RPC_HELP, COMMAND_SERVICE_METHOD_HELP, COMMAND_SERVICE_CLASS_SAFE, 0u, 1u),
    COMMAND_ENTRY("rpcinfo", COMMAND_SERVICE_RPC_RPCINFO, COMMAND_SERVICE_METHOD_RPCINFO, COMMAND_SERVICE_CLASS_SAFE, 0u, 0u)
};

typedef char command_registry_count_must_match[
    ((sizeof(command_registry) / sizeof(command_registry[0])) ==
     (uint32_t)COMMAND_SERVICE_METHOD_COUNT) ? 1 : -1];

static int command_text_equals(const char *a, const char *b)
{
    if ((a == (const char *)0) || (b == (const char *)0))
    {
        return 0;
    }

    while ((*a != '\0') && (*b != '\0'))
    {
        if (*a != *b)
        {
            return 0;
        }

        ++a;
        ++b;
    }

    return (*a == '\0') && (*b == '\0');
}

static int command_is_separator(char c)
{
    return (c == ' ') || (c == '\t');
}

uint32_t command_service_registry_count(void)
{
    return (uint32_t)(sizeof(command_registry) / sizeof(command_registry[0]));
}

const command_service_descriptor_t *command_service_registry_at(uint32_t index)
{
    if (index >= command_service_registry_count())
    {
        return (const command_service_descriptor_t *)0;
    }

    return &command_registry[index];
}

const command_service_descriptor_t *command_service_find(const char *name)
{
    uint32_t index;

    if (name == (const char *)0)
    {
        return (const command_service_descriptor_t *)0;
    }

    for (index = 0u; index < command_service_registry_count(); ++index)
    {
        if (command_text_equals(name, command_registry[index].name) != 0)
        {
            return &command_registry[index];
        }
    }

    return (const command_service_descriptor_t *)0;
}

const command_service_descriptor_t *command_service_find_rpc_id(uint16_t rpc_id)
{
    uint32_t index;

    if (rpc_id == 0u)
    {
        return (const command_service_descriptor_t *)0;
    }

    for (index = 0u; index < command_service_registry_count(); ++index)
    {
        if (command_registry[index].rpc_id == rpc_id)
        {
            return &command_registry[index];
        }
    }

    return (const command_service_descriptor_t *)0;
}

const char *command_service_class_name(command_service_class_t command_class)
{
    switch (command_class)
    {
        case COMMAND_SERVICE_CLASS_SAFE:
            return "SAFE";
        case COMMAND_SERVICE_CLASS_DIAGNOSTIC:
            return "DIAGNOSTIC";
        case COMMAND_SERVICE_CLASS_DESTRUCTIVE:
            return "DESTRUCTIVE";
        default:
            return "UNKNOWN";
    }
}

command_service_status_t command_service_parse_line(
    char *line,
    command_service_request_t *request)
{
    char *cursor;
    char *method;
    uint32_t argc = 0u;
    const command_service_descriptor_t *descriptor;

    if ((line == (char *)0) || (request == (command_service_request_t *)0))
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    request->descriptor = (const command_service_descriptor_t *)0;
    request->argc = 0u;

    for (uint32_t index = 0u; index < COMMAND_SERVICE_MAX_ARGS; ++index)
    {
        request->argv[index] = (const char *)0;
    }

    cursor = line;

    while (command_is_separator(*cursor) != 0)
    {
        ++cursor;
    }

    if (*cursor == '\0')
    {
        return COMMAND_SERVICE_STATUS_BAD_ARGS;
    }

    method = cursor;

    while ((*cursor != '\0') && (command_is_separator(*cursor) == 0))
    {
        ++cursor;
    }

    if (*cursor != '\0')
    {
        *cursor = '\0';
        ++cursor;
    }

    for (;;)
    {
        while (command_is_separator(*cursor) != 0)
        {
            ++cursor;
        }

        if (*cursor == '\0')
        {
            break;
        }

        if (argc >= COMMAND_SERVICE_MAX_ARGS)
        {
            return COMMAND_SERVICE_STATUS_BAD_ARGS;
        }

        request->argv[argc] = cursor;
        ++argc;

        while ((*cursor != '\0') && (command_is_separator(*cursor) == 0))
        {
            ++cursor;
        }

        if (*cursor != '\0')
        {
            *cursor = '\0';
            ++cursor;
        }
    }

    descriptor = command_service_find(method);
    if (descriptor == (const command_service_descriptor_t *)0)
    {
        return COMMAND_SERVICE_STATUS_NOT_FOUND;
    }

    if ((argc < descriptor->min_args) || (argc > descriptor->max_args))
    {
        return COMMAND_SERVICE_STATUS_BAD_ARGS;
    }

    request->descriptor = descriptor;
    request->argc = argc;

    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t command_service_write_byte(
    command_service_context_t *context,
    uint8_t byte)
{
    if (
        (context == (command_service_context_t *)0) ||
        (context->write_byte == (command_service_write_byte_t)0)
    ) {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (context->write_byte(context->write_context, byte) == 0)
    {
        context->write_failed = 1u;
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t command_service_write(
    command_service_context_t *context,
    const char *text)
{
    if (text == (const char *)0)
    {
        if (context != (command_service_context_t *)0)
        {
            context->write_failed = 1u;
        }
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    while (*text != '\0')
    {
        if (command_service_write_byte(context, (uint8_t)*text) != COMMAND_SERVICE_STATUS_OK)
        {
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
        }

        ++text;
    }

    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t command_service_write_line(
    command_service_context_t *context,
    const char *text)
{
    if (command_service_write(context, text) != COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return command_service_write(context, "\r\n");
}

command_service_status_t command_service_write_hex32(
    command_service_context_t *context,
    uint32_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    if (command_service_write(context, "0x") != COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    for (uint32_t shift = 28u;; shift -= 4u)
    {
        if (
            command_service_write_byte(
                context,
                (uint8_t)hex[(value >> shift) & 0xFu]) !=
            COMMAND_SERVICE_STATUS_OK
        ) {
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
        }

        if (shift == 0u)
        {
            break;
        }
    }

    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t command_service_execute(
    const command_service_request_t *request,
    command_service_context_t *context,
    command_service_handler_t handler,
    void *handler_context)
{
    command_service_status_t status;

    if (
        (request == (const command_service_request_t *)0) ||
        (request->descriptor == (const command_service_descriptor_t *)0) ||
        (context == (command_service_context_t *)0) ||
        (context->write_byte == (command_service_write_byte_t)0) ||
        (handler == (command_service_handler_t)0)
    ) {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    context->write_failed = 0u;
    status = handler(request, context, handler_context);

    if (context->write_failed != 0u)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return status;
}
