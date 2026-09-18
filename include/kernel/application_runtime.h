#ifndef KERNEL_APPLICATION_RUNTIME_H
#define KERNEL_APPLICATION_RUNTIME_H

#include <stdint.h>

#define APPLICATION_RUNTIME_ABI_VERSION       1u
#define APPLICATION_RUNTIME_REGISTRY_COUNT    2u

#define APPLICATION_ID_SYSTEM_HOME            0x0001u
#define APPLICATION_ID_DEVICE_INFO            0x0002u

#define APPLICATION_FLAG_SYSTEM               0x00000001u
#define APPLICATION_EFFECT_VIEW_DIRTY         0x00000001u

#define APPLICATION_VIEW_ROWS                 3u
#define APPLICATION_VIEW_COLUMNS              21u
#define APPLICATION_VIEW_STORAGE_COLUMNS      (APPLICATION_VIEW_COLUMNS + 1u)

typedef enum
{
    APPLICATION_STATE_REGISTERED = 0,
    APPLICATION_STATE_STOPPED = 1,
    APPLICATION_STATE_STARTING = 2,
    APPLICATION_STATE_RUNNING = 3,
    APPLICATION_STATE_BLOCKED = 4,
    APPLICATION_STATE_STOPPING = 5,
    APPLICATION_STATE_FAILED = 6
} application_lifecycle_state_t;

typedef enum
{
    APPLICATION_EVENT_RUNTIME_READY = 0x0001u,
    APPLICATION_EVENT_UPTIME_MINUTE_CHANGED = 0x0002u,
    APPLICATION_EVENT_SYSTEM_HEALTH_CHANGED = 0x0003u,
    APPLICATION_EVENT_USB_STATE_CHANGED = 0x0004u,
    APPLICATION_EVENT_NETWORK_STATE_CHANGED = 0x0005u
} application_event_type_t;

typedef enum
{
    APPLICATION_EVENT_SOURCE_RUNTIME = 0x0001u,
    APPLICATION_EVENT_SOURCE_TIME = 0x0002u,
    APPLICATION_EVENT_SOURCE_SYSTEM = 0x0003u,
    APPLICATION_EVENT_SOURCE_USB = 0x0004u,
    APPLICATION_EVENT_SOURCE_NETWORK = 0x0005u
} application_event_source_t;

typedef struct
{
    uint16_t type;
    uint16_t source;
    uint32_t value0;
    uint32_t value1;
} application_event_t;

typedef struct
{
    uint32_t uptime_ms;
    uint32_t displayed_minute;
    uint8_t system_healthy;
    uint8_t usb_configured;
    uint8_t network_online;
    uint8_t reserved;
} application_service_snapshot_t;

typedef struct
{
    char rows[APPLICATION_VIEW_ROWS][APPLICATION_VIEW_STORAGE_COLUMNS];
} application_view_t;

typedef int (*application_init_callback_t)(void);
typedef int (*application_start_callback_t)(
    const application_service_snapshot_t *services);
typedef uint32_t (*application_event_callback_t)(
    const application_event_t *event,
    const application_service_snapshot_t *services);
typedef int (*application_render_callback_t)(
    application_view_t *view,
    const application_service_snapshot_t *services);
typedef int (*application_stop_callback_t)(
    const application_service_snapshot_t *services);

typedef struct
{
    uint16_t id;
    uint16_t abi_version;
    uint32_t flags;
    const char *name;
    application_init_callback_t init;
    application_start_callback_t start;
    application_event_callback_t event;
    application_render_callback_t render;
    application_stop_callback_t stop;
} application_descriptor_t;

typedef struct
{
    application_view_t view;
    application_view_t staging_view;
    uint32_t fault_count;
    uint32_t event_count;
    uint32_t view_revision;
    uint32_t last_event_value0;
    uint32_t last_event_value1;
    uint16_t active_id;
    uint16_t last_event_type;
    uint16_t last_event_source;
    uint8_t states[APPLICATION_RUNTIME_REGISTRY_COUNT];
    uint8_t initialized;
    uint8_t view_dirty;
    uint8_t render_pending;
    uint8_t reserved;
} application_runtime_t;

void application_view_clear(application_view_t *view);
int application_view_set_row(
    application_view_t *view,
    uint32_t row,
    const char *text);

void application_runtime_reset(application_runtime_t *runtime);
int application_runtime_initialize(application_runtime_t *runtime);
int application_runtime_is_initialized(const application_runtime_t *runtime);

uint32_t application_runtime_registry_count(void);
const application_descriptor_t *application_runtime_registry_at(uint32_t index);
const application_descriptor_t *application_runtime_find(uint16_t id);
const char *application_runtime_state_name(application_lifecycle_state_t state);
int application_runtime_state_get(
    const application_runtime_t *runtime,
    uint16_t id,
    application_lifecycle_state_t *state_out);

uint16_t application_runtime_active_id(const application_runtime_t *runtime);
uint32_t application_runtime_fault_count(const application_runtime_t *runtime);
uint32_t application_runtime_event_count(const application_runtime_t *runtime);
uint32_t application_runtime_view_revision(const application_runtime_t *runtime);
uint16_t application_runtime_last_event_type(
    const application_runtime_t *runtime);
uint16_t application_runtime_last_event_source(
    const application_runtime_t *runtime);

const application_view_t *application_runtime_view_get(
    const application_runtime_t *runtime);
int application_runtime_view_dirty(const application_runtime_t *runtime);
void application_runtime_view_consumed(application_runtime_t *runtime);

int application_runtime_start(
    application_runtime_t *runtime,
    uint16_t id,
    const application_service_snapshot_t *services);
int application_runtime_stop(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services);
int application_runtime_dispatch_event(
    application_runtime_t *runtime,
    const application_event_t *event,
    const application_service_snapshot_t *services);
int application_runtime_service(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services);

#endif
