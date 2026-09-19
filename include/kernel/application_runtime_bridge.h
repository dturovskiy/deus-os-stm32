#ifndef KERNEL_APPLICATION_RUNTIME_BRIDGE_H
#define KERNEL_APPLICATION_RUNTIME_BRIDGE_H

#include <stdint.h>
#include "kernel/application_runtime.h"
#include "kernel/oled_console.h"

void application_runtime_bridge_reset(void);
int application_runtime_bridge_initialize(void);

int application_runtime_bridge_service(
    const application_service_snapshot_t *services);

typedef struct
{
    uint32_t fault_count;
    uint32_t event_count;
    uint32_t view_revision;
    uint16_t active_id;
    uint16_t last_event_type;
    uint16_t last_event_source;
} application_runtime_bridge_status_t;

/*
 * Read-side bridge API. Mutable runtime representation remains private to the
 * bridge; callers receive value snapshots or narrow queries only.
 */
application_runtime_bridge_status_t application_runtime_bridge_status(void);
int application_runtime_bridge_is_initialized(void);
int application_runtime_bridge_view_dirty(void);
int application_runtime_bridge_state_at(
    uint32_t index,
    application_lifecycle_state_t *state_out);

int application_runtime_bridge_start(
    uint16_t id,
    const application_service_snapshot_t *services);
int application_runtime_bridge_stop(
    const application_service_snapshot_t *services);

int application_runtime_bridge_apply_view(
    oled_console_t *console,
    uint32_t force_rows);
void application_runtime_bridge_view_consumed(void);

#endif
