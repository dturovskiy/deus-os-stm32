#ifndef KERNEL_APPLICATION_COMMANDS_H
#define KERNEL_APPLICATION_COMMANDS_H

#include "kernel/application_runtime.h"
#include "kernel/command_service.h"

/*
 * request and request->descriptor are validated once by the composition-root
 * command dispatcher before this domain handler is entered.
 */
command_service_status_t application_commands_execute(
    const command_service_request_t *request,
    command_service_context_t *context,
    const application_service_snapshot_t *services);

#endif
