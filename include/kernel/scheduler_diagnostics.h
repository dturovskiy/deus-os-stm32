#ifndef KERNEL_SCHEDULER_DIAGNOSTICS_H
#define KERNEL_SCHEDULER_DIAGNOSTICS_H

#include <stdint.h>
#include "kernel/command_service.h"

typedef command_service_status_t (*scheduler_diagnostics_execute_request_t)(
    const command_service_request_t *request,
    command_service_context_t *context,
    void *handler_context);

typedef int (*scheduler_diagnostics_ui_show_t)(void);
typedef int (*scheduler_diagnostics_uart_try_getc_t)(char *byte_out);

typedef struct
{
    scheduler_diagnostics_execute_request_t execute_request;
    scheduler_diagnostics_ui_show_t ui_show;
    scheduler_diagnostics_uart_try_getc_t uart_try_getc;
    uint32_t *console_stack;
    uint32_t console_stack_words;
    uint32_t console_min_margin_bytes;
    uint32_t uart_rx_event_mask;
} scheduler_diagnostics_bindings_t;

uint32_t scheduler_diagnostics_busy_count(void);

command_service_status_t scheduler_diagnostics_execute_timed(
    command_service_context_t *context);

command_service_status_t scheduler_diagnostics_execute_priority(
    command_service_context_t *context);

/*
 * The composition root supplies a fully initialized non-null binding and a
 * validated request/descriptor. bindings is borrowed for the duration of this
 * diagnostic execution, including nested scheduler tasks, and is never mutated.
 */
command_service_status_t scheduler_diagnostics_execute_diagnostic(
    const command_service_request_t *request,
    command_service_context_t *context,
    const scheduler_diagnostics_bindings_t *bindings);

#endif
