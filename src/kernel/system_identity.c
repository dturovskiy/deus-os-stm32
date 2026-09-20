#include <stdint.h>
#include "kernel/application_runtime.h"
#include "kernel/binary_frame.h"
#include "kernel/command_service.h"
#include "kernel/system_identity.h"

#ifndef DEUS_FIRMWARE_SOURCE_TREE_HEX
#define DEUS_FIRMWARE_SOURCE_TREE_HEX "UNBOUND"
#endif

static const char system_identity_source_tree[] =
    DEUS_FIRMWARE_SOURCE_TREE_HEX;

static command_service_status_t system_identity_write_text(
    command_service_context_t *context,
    const char *text)
{
    return command_service_write(context, text);
}

static command_service_status_t system_identity_write_hex(
    command_service_context_t *context,
    uint32_t value)
{
    return command_service_write_hex32(context, value);
}

command_service_status_t system_identity_write(
    command_service_context_t *context)
{
    if (context == (command_service_context_t *)0)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, "SYSINFO_ABI=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, SYSTEM_IDENTITY_ABI_VERSION) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_text(
            context,
            " OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n"
        ) != COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, "SOURCE_TREE=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_text(context, system_identity_source_tree) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_text(context, "\r\nPROTOCOL_VERSION=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, BINARY_FRAME_PROTOCOL_VERSION) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, " SERVICE_VERSION=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, COMMAND_SERVICE_FOUNDATION_VERSION) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, " APP_RUNTIME_ABI=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, APPLICATION_RUNTIME_ABI_VERSION) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, "\r\nCAPABILITIES=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, SYSTEM_IDENTITY_CAPABILITIES) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    if (system_identity_write_text(context, " UNIT_ID_KIND=") !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }
    if (system_identity_write_hex(context, SYSTEM_IDENTITY_UNIT_ID_NONE) !=
        COMMAND_SERVICE_STATUS_OK)
    {
        return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return system_identity_write_text(context, "\r\n");
}
