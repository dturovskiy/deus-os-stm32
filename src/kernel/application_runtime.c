#include <stdint.h>
#include "kernel/application_runtime.h"

typedef char application_event_size_must_be_12[
    (sizeof(application_event_t) == 12u) ? 1 : -1];
typedef char application_view_rows_must_be_three[
    (APPLICATION_VIEW_ROWS == 3u) ? 1 : -1];
typedef char application_view_columns_must_be_twenty_one[
    (APPLICATION_VIEW_COLUMNS == 21u) ? 1 : -1];
typedef char application_lifecycle_values_must_match[
    ((APPLICATION_STATE_REGISTERED == 0) &&
     (APPLICATION_STATE_STOPPED == 1) &&
     (APPLICATION_STATE_STARTING == 2) &&
     (APPLICATION_STATE_RUNNING == 3) &&
     (APPLICATION_STATE_BLOCKED == 4) &&
     (APPLICATION_STATE_STOPPING == 5) &&
     (APPLICATION_STATE_FAILED == 6)) ? 1 : -1];

static int application_builtin_init(void)
{
    return 1;
}

static int application_builtin_start(
    const application_service_snapshot_t *services)
{
    return services != (const application_service_snapshot_t *)0;
}

static uint32_t application_builtin_event(
    const application_event_t *event,
    const application_service_snapshot_t *services)
{
    if ((event == (const application_event_t *)0) ||
        (services == (const application_service_snapshot_t *)0))
    {
        return 0u;
    }

    return 0u;
}

static int application_system_home_render(
    application_view_t *view,
    const application_service_snapshot_t *services)
{
    if ((view == (application_view_t *)0) ||
        (services == (const application_service_snapshot_t *)0))
    {
        return 0;
    }

    application_view_clear(view);

    return
        (application_view_set_row(view, 0u, "DEUS OS") != 0) &&
        (application_view_set_row(view, 1u, "DESKTOP") != 0) &&
        (application_view_set_row(view, 2u, "READY") != 0);
}

static int application_device_info_render(
    application_view_t *view,
    const application_service_snapshot_t *services)
{
    if ((view == (application_view_t *)0) ||
        (services == (const application_service_snapshot_t *)0))
    {
        return 0;
    }

    application_view_clear(view);

    return
        (application_view_set_row(view, 0u, "DEUS OS") != 0) &&
        (application_view_set_row(view, 1u, "DEVICE INFO") != 0) &&
        (application_view_set_row(view, 2u, "STM32F103") != 0);
}

static int application_builtin_stop(
    const application_service_snapshot_t *services)
{
    return services != (const application_service_snapshot_t *)0;
}

static const application_descriptor_t application_registry[] =
{
    {
        APPLICATION_ID_SYSTEM_HOME,
        APPLICATION_RUNTIME_ABI_VERSION,
        APPLICATION_FLAG_SYSTEM,
        "system.home",
        application_builtin_init,
        application_builtin_start,
        application_builtin_event,
        application_system_home_render,
        application_builtin_stop
    },
    {
        APPLICATION_ID_DEVICE_INFO,
        APPLICATION_RUNTIME_ABI_VERSION,
        APPLICATION_FLAG_SYSTEM,
        "device.info",
        application_builtin_init,
        application_builtin_start,
        application_builtin_event,
        application_device_info_render,
        application_builtin_stop
    }
};

typedef char application_registry_count_must_match[
    ((sizeof(application_registry) / sizeof(application_registry[0])) ==
     APPLICATION_RUNTIME_REGISTRY_COUNT) ? 1 : -1];

static void application_counter_increment(uint32_t *counter)
{
    if ((counter != (uint32_t *)0) && (*counter != 0xFFFFFFFFu))
    {
        ++(*counter);
    }
}

static int application_runtime_index_from_id(
    uint16_t id,
    uint32_t *index_out)
{
    uint32_t index;

    if ((id == 0u) || (index_out == (uint32_t *)0))
    {
        return 0;
    }

    for (index = 0u; index < application_runtime_registry_count(); ++index)
    {
        if (application_registry[index].id == id)
        {
            *index_out = index;
            return 1;
        }
    }

    return 0;
}

static int application_view_equal(
    const application_view_t *left,
    const application_view_t *right)
{
    uint32_t row;
    uint32_t column;

    if ((left == (const application_view_t *)0) ||
        (right == (const application_view_t *)0))
    {
        return 0;
    }

    for (row = 0u; row < APPLICATION_VIEW_ROWS; ++row)
    {
        for (column = 0u; column < APPLICATION_VIEW_STORAGE_COLUMNS; ++column)
        {
            if (left->rows[row][column] != right->rows[row][column])
            {
                return 0;
            }
        }
    }

    return 1;
}

static void application_view_copy(
    application_view_t *destination,
    const application_view_t *source)
{
    uint32_t row;
    uint32_t column;

    if ((destination == (application_view_t *)0) ||
        (source == (const application_view_t *)0))
    {
        return;
    }

    for (row = 0u; row < APPLICATION_VIEW_ROWS; ++row)
    {
        for (column = 0u; column < APPLICATION_VIEW_STORAGE_COLUMNS; ++column)
        {
            destination->rows[row][column] = source->rows[row][column];
        }
    }
}

static int application_runtime_render_active(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services)
{
    application_view_t *candidate;
    const application_descriptor_t *descriptor;
    uint32_t index;

    if ((runtime == (application_runtime_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (runtime->active_id == 0u) ||
        (application_runtime_index_from_id(runtime->active_id, &index) == 0))
    {
        return 0;
    }

    descriptor = &application_registry[index];

    if ((runtime->states[index] != (uint8_t)APPLICATION_STATE_RUNNING) ||
        (descriptor->render == (application_render_callback_t)0))
    {
        return 0;
    }

    candidate = &runtime->staging_view;
    application_view_clear(candidate);

    if (descriptor->render(candidate, services) == 0)
    {
        if (runtime->render_pending == 0u)
        {
            application_counter_increment(&runtime->fault_count);
        }

        runtime->render_pending = 1u;
        return 0;
    }

    runtime->render_pending = 0u;

    if (application_view_equal(&runtime->view, candidate) == 0)
    {
        application_view_copy(&runtime->view, candidate);
        runtime->view_dirty = 1u;
        application_counter_increment(&runtime->view_revision);
    }

    return 1;
}

static int application_runtime_start_home_fallback(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services)
{
    uint32_t home_index;
    const application_descriptor_t *home;

    if ((runtime == (application_runtime_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (application_runtime_index_from_id(
            APPLICATION_ID_SYSTEM_HOME,
            &home_index) == 0))
    {
        return 0;
    }

    home = &application_registry[home_index];

    if (runtime->states[home_index] == (uint8_t)APPLICATION_STATE_FAILED)
    {
        runtime->active_id = 0u;
        return 0;
    }

    runtime->states[home_index] = (uint8_t)APPLICATION_STATE_STARTING;

    if ((home->start == (application_start_callback_t)0) ||
        (home->start(services) == 0))
    {
        runtime->states[home_index] = (uint8_t)APPLICATION_STATE_FAILED;
        runtime->active_id = 0u;
        application_counter_increment(&runtime->fault_count);
        return 0;
    }

    runtime->states[home_index] = (uint8_t)APPLICATION_STATE_RUNNING;
    runtime->active_id = APPLICATION_ID_SYSTEM_HOME;
    (void)application_runtime_render_active(runtime, services);

    return 1;
}

void application_view_clear(application_view_t *view)
{
    uint32_t row;
    uint32_t column;

    if (view == (application_view_t *)0)
    {
        return;
    }

    for (row = 0u; row < APPLICATION_VIEW_ROWS; ++row)
    {
        for (column = 0u; column < APPLICATION_VIEW_STORAGE_COLUMNS; ++column)
        {
            view->rows[row][column] = '\0';
        }
    }
}

int application_view_set_row(
    application_view_t *view,
    uint32_t row,
    const char *text)
{
    uint32_t column = 0u;

    if ((view == (application_view_t *)0) ||
        (text == (const char *)0) ||
        (row >= APPLICATION_VIEW_ROWS))
    {
        return 0;
    }

    while ((text[column] != '\0') && (column < APPLICATION_VIEW_COLUMNS))
    {
        view->rows[row][column] = text[column];
        ++column;
    }

    if (text[column] != '\0')
    {
        return 0;
    }

    while (column < APPLICATION_VIEW_STORAGE_COLUMNS)
    {
        view->rows[row][column] = '\0';
        ++column;
    }

    return 1;
}

void application_runtime_reset(application_runtime_t *runtime)
{
    uint32_t index;

    if (runtime == (application_runtime_t *)0)
    {
        return;
    }

    application_view_clear(&runtime->view);
    application_view_clear(&runtime->staging_view);
    runtime->fault_count = 0u;
    runtime->event_count = 0u;
    runtime->view_revision = 0u;
    runtime->last_event_value0 = 0u;
    runtime->last_event_value1 = 0u;
    runtime->active_id = 0u;
    runtime->last_event_type = 0u;
    runtime->last_event_source = 0u;

    for (index = 0u; index < APPLICATION_RUNTIME_REGISTRY_COUNT; ++index)
    {
        runtime->states[index] = (uint8_t)APPLICATION_STATE_REGISTERED;
    }

    runtime->initialized = 0u;
    runtime->view_dirty = 0u;
    runtime->render_pending = 0u;
    runtime->reserved = 0u;
}

int application_runtime_initialize(application_runtime_t *runtime)
{
    uint32_t index;
    uint32_t home_index;

    if (runtime == (application_runtime_t *)0)
    {
        return 0;
    }

    if (runtime->initialized != 0u)
    {
        return 1;
    }

    for (index = 0u; index < APPLICATION_RUNTIME_REGISTRY_COUNT; ++index)
    {
        runtime->states[index] = (uint8_t)APPLICATION_STATE_REGISTERED;

        if ((application_registry[index].init == (application_init_callback_t)0) ||
            (application_registry[index].init() == 0))
        {
            runtime->states[index] = (uint8_t)APPLICATION_STATE_FAILED;
            application_counter_increment(&runtime->fault_count);
        }
        else
        {
            runtime->states[index] = (uint8_t)APPLICATION_STATE_STOPPED;
        }
    }

    runtime->initialized = 1u;

    if (application_runtime_index_from_id(
            APPLICATION_ID_SYSTEM_HOME,
            &home_index) == 0)
    {
        return 0;
    }

    return runtime->states[home_index] ==
        (uint8_t)APPLICATION_STATE_STOPPED;
}

int application_runtime_is_initialized(const application_runtime_t *runtime)
{
    return
        (runtime != (const application_runtime_t *)0) &&
        (runtime->initialized != 0u);
}

uint32_t application_runtime_registry_count(void)
{
    return (uint32_t)(
        sizeof(application_registry) /
        sizeof(application_registry[0]));
}

const application_descriptor_t *application_runtime_registry_at(uint32_t index)
{
    if (index >= application_runtime_registry_count())
    {
        return (const application_descriptor_t *)0;
    }

    return &application_registry[index];
}

const application_descriptor_t *application_runtime_find(uint16_t id)
{
    uint32_t index;

    if (application_runtime_index_from_id(id, &index) == 0)
    {
        return (const application_descriptor_t *)0;
    }

    return &application_registry[index];
}

const char *application_runtime_state_name(application_lifecycle_state_t state)
{
    switch (state)
    {
        case APPLICATION_STATE_REGISTERED:
            return "REGISTERED";
        case APPLICATION_STATE_STOPPED:
            return "STOPPED";
        case APPLICATION_STATE_STARTING:
            return "STARTING";
        case APPLICATION_STATE_RUNNING:
            return "RUNNING";
        case APPLICATION_STATE_BLOCKED:
            return "BLOCKED";
        case APPLICATION_STATE_STOPPING:
            return "STOPPING";
        case APPLICATION_STATE_FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

int application_runtime_state_get(
    const application_runtime_t *runtime,
    uint16_t id,
    application_lifecycle_state_t *state_out)
{
    uint32_t index;

    if ((runtime == (const application_runtime_t *)0) ||
        (state_out == (application_lifecycle_state_t *)0) ||
        (application_runtime_index_from_id(id, &index) == 0))
    {
        return 0;
    }

    *state_out = (application_lifecycle_state_t)runtime->states[index];
    return 1;
}

uint16_t application_runtime_active_id(const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->active_id;
}

uint32_t application_runtime_fault_count(const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->fault_count;
}

uint32_t application_runtime_event_count(const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->event_count;
}

uint32_t application_runtime_view_revision(const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->view_revision;
}

uint16_t application_runtime_last_event_type(
    const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->last_event_type;
}

uint16_t application_runtime_last_event_source(
    const application_runtime_t *runtime)
{
    if (runtime == (const application_runtime_t *)0)
    {
        return 0u;
    }

    return runtime->last_event_source;
}

const application_view_t *application_runtime_view_get(
    const application_runtime_t *runtime)
{
    if ((runtime == (const application_runtime_t *)0) ||
        (runtime->active_id == 0u))
    {
        return (const application_view_t *)0;
    }

    return &runtime->view;
}

int application_runtime_view_dirty(const application_runtime_t *runtime)
{
    return
        (runtime != (const application_runtime_t *)0) &&
        (runtime->view_dirty != 0u);
}

void application_runtime_view_consumed(application_runtime_t *runtime)
{
    if (runtime != (application_runtime_t *)0)
    {
        runtime->view_dirty = 0u;
    }
}

int application_runtime_start(
    application_runtime_t *runtime,
    uint16_t id,
    const application_service_snapshot_t *services)
{
    uint32_t target_index;
    uint32_t current_index;
    const application_descriptor_t *target;

    if ((runtime == (application_runtime_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (runtime->initialized == 0u) ||
        (application_runtime_index_from_id(id, &target_index) == 0))
    {
        return 0;
    }

    if ((runtime->active_id == id) &&
        (runtime->states[target_index] == (uint8_t)APPLICATION_STATE_RUNNING))
    {
        return 1;
    }

    if (runtime->states[target_index] == (uint8_t)APPLICATION_STATE_FAILED)
    {
        return 0;
    }

    if ((runtime->active_id != 0u) &&
        (application_runtime_index_from_id(
            runtime->active_id,
            &current_index) != 0))
    {
        const application_descriptor_t *current =
            &application_registry[current_index];

        runtime->states[current_index] =
            (uint8_t)APPLICATION_STATE_STOPPING;

        if ((current->stop == (application_stop_callback_t)0) ||
            (current->stop(services) == 0))
        {
            runtime->states[current_index] =
                (uint8_t)APPLICATION_STATE_FAILED;
            application_counter_increment(&runtime->fault_count);
        }
        else
        {
            runtime->states[current_index] =
                (uint8_t)APPLICATION_STATE_STOPPED;
        }

        runtime->active_id = 0u;
    }

    target = &application_registry[target_index];
    runtime->states[target_index] = (uint8_t)APPLICATION_STATE_STARTING;

    if ((target->start == (application_start_callback_t)0) ||
        (target->start(services) == 0))
    {
        runtime->states[target_index] = (uint8_t)APPLICATION_STATE_FAILED;
        application_counter_increment(&runtime->fault_count);

        if (id != APPLICATION_ID_SYSTEM_HOME)
        {
            (void)application_runtime_start_home_fallback(runtime, services);
        }

        return 0;
    }

    runtime->states[target_index] = (uint8_t)APPLICATION_STATE_RUNNING;
    runtime->active_id = id;
    (void)application_runtime_render_active(runtime, services);

    return 1;
}

int application_runtime_stop(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services)
{
    if ((runtime == (application_runtime_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (runtime->initialized == 0u))
    {
        return 0;
    }

    if (runtime->active_id == APPLICATION_ID_SYSTEM_HOME)
    {
        return 1;
    }

    return application_runtime_start(
        runtime,
        APPLICATION_ID_SYSTEM_HOME,
        services);
}

int application_runtime_dispatch_event(
    application_runtime_t *runtime,
    const application_event_t *event,
    const application_service_snapshot_t *services)
{
    uint32_t index;
    uint32_t effects;
    const application_descriptor_t *descriptor;

    if ((runtime == (application_runtime_t *)0) ||
        (event == (const application_event_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (runtime->initialized == 0u) ||
        (runtime->active_id == 0u) ||
        (application_runtime_index_from_id(runtime->active_id, &index) == 0))
    {
        return 0;
    }

    descriptor = &application_registry[index];

    if ((runtime->states[index] != (uint8_t)APPLICATION_STATE_RUNNING) ||
        (descriptor->event == (application_event_callback_t)0))
    {
        return 0;
    }

    runtime->last_event_type = event->type;
    runtime->last_event_source = event->source;
    runtime->last_event_value0 = event->value0;
    runtime->last_event_value1 = event->value1;
    application_counter_increment(&runtime->event_count);

    effects = descriptor->event(event, services);

    if ((effects & ~APPLICATION_EFFECT_VIEW_DIRTY) != 0u)
    {
        application_counter_increment(&runtime->fault_count);
        effects &= APPLICATION_EFFECT_VIEW_DIRTY;
    }

    if ((effects & APPLICATION_EFFECT_VIEW_DIRTY) != 0u)
    {
        return application_runtime_render_active(runtime, services);
    }

    return 1;
}

int application_runtime_service(
    application_runtime_t *runtime,
    const application_service_snapshot_t *services)
{
    if ((runtime == (application_runtime_t *)0) ||
        (services == (const application_service_snapshot_t *)0) ||
        (runtime->initialized == 0u))
    {
        return 0;
    }

    if (runtime->render_pending != 0u)
    {
        return application_runtime_render_active(runtime, services);
    }

    return 1;
}
