#ifndef KERNEL_COMMAND_SERVICE_H
#define KERNEL_COMMAND_SERVICE_H

#include <stdint.h>

#define COMMAND_SERVICE_FOUNDATION_VERSION 1u
#define COMMAND_SERVICE_LINE_CAPACITY       32u
#define COMMAND_SERVICE_MAX_ARGS             4u

typedef enum
{
    COMMAND_SERVICE_STATUS_OK = 0,
    COMMAND_SERVICE_STATUS_NOT_FOUND,
    COMMAND_SERVICE_STATUS_BAD_ARGS,
    COMMAND_SERVICE_STATUS_BUSY,
    COMMAND_SERVICE_STATUS_INTERNAL_ERROR
} command_service_status_t;

typedef enum
{
    COMMAND_SERVICE_CLASS_SAFE = 0,
    COMMAND_SERVICE_CLASS_DIAGNOSTIC,
    COMMAND_SERVICE_CLASS_DESTRUCTIVE
} command_service_class_t;

typedef enum
{
    COMMAND_SERVICE_METHOD_PING = 0,
    COMMAND_SERVICE_METHOD_UPTIME,
    COMMAND_SERVICE_METHOD_HEALTH,
    COMMAND_SERVICE_METHOD_RXSTAT,
    COMMAND_SERVICE_METHOD_CDCSTAT,
    COMMAND_SERVICE_METHOD_MSPSTAT,
    COMMAND_SERVICE_METHOD_SCHEDPROD,
    COMMAND_SERVICE_METHOD_SCHEDTIMED,
    COMMAND_SERVICE_METHOD_SCHEDPRIO,
    COMMAND_SERVICE_METHOD_FAULT,
    COMMAND_SERVICE_METHOD_I2CSCAN,
    COMMAND_SERVICE_METHOD_OLEDPING,
    COMMAND_SERVICE_METHOD_OLEDTEST,
    COMMAND_SERVICE_METHOD_OLEDTEXT,
    COMMAND_SERVICE_METHOD_OLEDRENDER,
    COMMAND_SERVICE_METHOD_OLEDCONSOLE,
    COMMAND_SERVICE_METHOD_OLEDSCROLL,
    COMMAND_SERVICE_METHOD_OLEDDIRTY,
    COMMAND_SERVICE_METHOD_OLEDUIUPDATE,
    COMMAND_SERVICE_METHOD_UIRUNTIME,
    COMMAND_SERVICE_METHOD_OLEDSTATUS,
    COMMAND_SERVICE_METHOD_SCHEDTEST,
    COMMAND_SERVICE_METHOD_SCHEDCOOP,
    COMMAND_SERVICE_METHOD_SCHEDPREEMPT,
    COMMAND_SERVICE_METHOD_SCHEDSTACK,
    COMMAND_SERVICE_METHOD_SCHEDWORKLOAD,
    COMMAND_SERVICE_METHOD_SCHEDCONSOLEPROBE,
    COMMAND_SERVICE_METHOD_SCHEDWAITWAKE,
    COMMAND_SERVICE_METHOD_SCHEDISOLATE,
    COMMAND_SERVICE_METHOD_WDOGTRIP,
    COMMAND_SERVICE_METHOD_HELP,
    COMMAND_SERVICE_METHOD_RPCINFO,
    COMMAND_SERVICE_METHOD_COUNT
} command_service_method_id_t;

typedef int (*command_service_write_byte_t)(void *context, uint8_t byte);

typedef struct
{
    command_service_write_byte_t write_byte;
    void *write_context;
    uint32_t source_rx_event_mask;
    uint32_t write_failed;
} command_service_context_t;

typedef struct
{
    const char *name;
    command_service_method_id_t method_id;
    command_service_class_t command_class;
    uint8_t min_args;
    uint8_t max_args;
} command_service_descriptor_t;

typedef struct
{
    const command_service_descriptor_t *descriptor;
    uint32_t argc;
    const char *argv[COMMAND_SERVICE_MAX_ARGS];
} command_service_request_t;

typedef command_service_status_t (*command_service_handler_t)(
    const command_service_request_t *request,
    command_service_context_t *context,
    void *handler_context);

uint32_t command_service_registry_count(void);
const command_service_descriptor_t *command_service_registry_at(uint32_t index);
const command_service_descriptor_t *command_service_find(const char *name);
const char *command_service_class_name(command_service_class_t command_class);
command_service_status_t command_service_parse_line(
    char *line,
    command_service_request_t *request);
command_service_status_t command_service_write_byte(
    command_service_context_t *context,
    uint8_t byte);
command_service_status_t command_service_write(
    command_service_context_t *context,
    const char *text);
command_service_status_t command_service_write_line(
    command_service_context_t *context,
    const char *text);
command_service_status_t command_service_write_hex32(
    command_service_context_t *context,
    uint32_t value);
command_service_status_t command_service_execute(
    const command_service_request_t *request,
    command_service_context_t *context,
    command_service_handler_t handler,
    void *handler_context);

#endif
