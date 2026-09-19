#include <stdint.h>
#include "kernel/scheduler_diagnostics.h"
#include "kernel/scheduler.h"
#include "kernel/time.h"

#define SCHED_WORKLOAD_PEER_SPINS 1000000u
#define SCHED_CONSOLE_PROBE_COMMAND_COUNT 17u
#define SCHED_ISOLATION_DIAGNOSTIC_COUNT 7u
#define SCHED_WAIT_WAKE_SENTINEL 0x57u
#define SCHED_TIMED_SLEEP_MS 50u
#define SCHED_TIMED_TIMEOUT_MS 50u
#define SCHED_TIMED_EVENT_TIMEOUT_MS 500u

static volatile uint32_t scheduler_workload_task0_started;
static volatile uint32_t scheduler_workload_task0_done;
static volatile uint32_t scheduler_workload_peer_overlap;
static volatile uint32_t scheduler_workload_peer_done;
static volatile uint32_t scheduler_workload_ui_result;

static volatile uint32_t scheduler_console_probe_task_started;
static volatile uint32_t scheduler_console_probe_task_done;
static volatile uint32_t scheduler_console_probe_peer_overlap;
static volatile uint32_t scheduler_console_probe_peer_done;
static volatile uint32_t scheduler_console_probe_completed_count;
static volatile uint32_t scheduler_console_probe_ui_restore_result;

static volatile uint32_t scheduler_isolation_task_started;
static volatile uint32_t scheduler_isolation_task_done;
static volatile uint32_t scheduler_isolation_peer_overlap;
static volatile uint32_t scheduler_isolation_peer_done;
static volatile uint32_t scheduler_isolation_init_reject;
static volatile uint32_t scheduler_isolation_active_preserved;
static volatile uint32_t scheduler_diagnostic_busy_count;

static volatile uint32_t scheduler_wait_wake_task_started;
static volatile uint32_t scheduler_wait_wake_task_resumed;
static volatile uint32_t scheduler_wait_wake_peer_done;
static volatile uint32_t scheduler_wait_wake_events;
static volatile uint32_t scheduler_wait_wake_byte;
static volatile uint32_t scheduler_wait_wake_byte_ok;
static volatile uint32_t scheduler_wait_wake_framing_bytes;

static const scheduler_diagnostics_bindings_t *scheduler_diagnostics_bindings;

static void console_write(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write(context, text);
}

static void console_write_line(
    command_service_context_t *context,
    const char *text)
{
    (void)command_service_write_line(context, text);
}

static void console_write_hex32(
    command_service_context_t *context,
    uint32_t value)
{
    (void)command_service_write_hex32(context, value);
}

static int scheduler_diagnostics_ui_show(void)
{
    return scheduler_diagnostics_bindings->ui_show();
}

static int scheduler_diagnostics_uart_try_getc(char *byte_out)
{
    return scheduler_diagnostics_bindings->uart_try_getc(byte_out);
}

static command_service_status_t scheduler_diagnostics_execute_bound_request(
    const command_service_request_t *request,
    command_service_context_t *context)
{
    return scheduler_diagnostics_bindings->execute_request(
        request,
        context,
        (void *)0);
}

static void scheduler_workload_oled_task(void *argument)
{
    (void)argument;

    scheduler_workload_task0_started = 1u;
    scheduler_workload_ui_result =
        (scheduler_diagnostics_ui_show() != 0) ? 1u : 0u;
    scheduler_workload_task0_done = 1u;
}

static void scheduler_workload_cpu_peer_task(void *argument)
{
    uint32_t i;

    (void)argument;

    for (i = 0u; i < SCHED_WORKLOAD_PEER_SPINS; ++i)
    {
        if (
            (scheduler_workload_task0_started != 0u) &&
            (scheduler_workload_task0_done == 0u)
        ) {
            scheduler_workload_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_workload_peer_done = 1u;
}



static void console_scheduler_test(command_service_context_t *context)
{
    if (scheduler_self_test() != 0)
    {
        console_write_line(context, "SCHED_FOUNDATION_OK");
    }
    else
    {
        console_write_line(context, "SCHED_FOUNDATION_ERR");
    }
}

static void console_scheduler_cooperative_test(command_service_context_t *context)
{
    if (scheduler_cooperative_self_test() != 0)
    {
        console_write_line(context, "SCHED_COOP_OK");
    }
    else
    {
        console_write_line(context, "SCHED_COOP_ERR");
    }
}

static void console_scheduler_preemptive_test(command_service_context_t *context)
{
    if (scheduler_preemptive_self_test() != 0)
    {
        console_write_line(context, "SCHED_PREEMPT_OK");
    }
    else
    {
        console_write_line(context, "SCHED_PREEMPT_ERR");
    }
}

static void console_scheduler_stack_water_test(command_service_context_t *context)
{
    uint32_t coop_used0;
    uint32_t coop_used1;
    uint32_t preempt_used0;
    uint32_t preempt_used1;
    uint32_t capacity0;
    uint32_t capacity1;
    int coop_canary0;
    int coop_canary1;
    int preempt_canary0;
    int preempt_canary1;
    int coop_result;
    int preempt_result;
    int passed;

    coop_result = scheduler_cooperative_self_test();
    coop_used0 = scheduler_stack_high_water_bytes(0u);
    coop_used1 = scheduler_stack_high_water_bytes(1u);
    coop_canary0 = scheduler_stack_canary_intact(0u);
    coop_canary1 = scheduler_stack_canary_intact(1u);

    preempt_result = scheduler_preemptive_self_test();
    preempt_used0 = scheduler_stack_high_water_bytes(0u);
    preempt_used1 = scheduler_stack_high_water_bytes(1u);
    preempt_canary0 = scheduler_stack_canary_intact(0u);
    preempt_canary1 = scheduler_stack_canary_intact(1u);

    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);

    console_write(context, "STACK_CAPACITY=");
    console_write_hex32(context, capacity0);
    console_write(context, "\r\n");

    console_write(context, "STACK_COOP_T0_USED=");
    console_write_hex32(context, coop_used0);
    console_write(context, "\r\n");

    console_write(context, "STACK_COOP_T1_USED=");
    console_write_hex32(context, coop_used1);
    console_write(context, "\r\n");

    console_write(context, "STACK_PREEMPT_T0_USED=");
    console_write_hex32(context, preempt_used0);
    console_write(context, "\r\n");

    console_write(context, "STACK_PREEMPT_T1_USED=");
    console_write_hex32(context, preempt_used1);
    console_write(context, "\r\n");

    passed =
        (coop_result != 0) &&
        (preempt_result != 0) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (coop_used0 >= 64u) &&
        (coop_used0 < capacity0) &&
        (coop_used1 >= 64u) &&
        (coop_used1 < capacity1) &&
        (preempt_used0 >= 64u) &&
        (preempt_used0 < capacity0) &&
        (preempt_used1 >= 64u) &&
        (preempt_used1 < capacity1) &&
        (coop_canary0 != 0) &&
        (coop_canary1 != 0) &&
        (preempt_canary0 != 0) &&
        (preempt_canary1 != 0);

    if (passed != 0)
    {
        console_write_line(context, "SCHED_STACK_WATER_OK");
    }
    else
    {
        console_write_line(context, "SCHED_STACK_WATER_ERR");
    }
}

static void console_scheduler_workload_test(command_service_context_t *context)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t switches;
    int canary0;
    int canary1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        console_write_line(context, "SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    scheduler_workload_task0_started = 0u;
    scheduler_workload_task0_done = 0u;
    scheduler_workload_peer_overlap = 0u;
    scheduler_workload_peer_done = 0u;
    scheduler_workload_ui_result = 0u;

    if (
        scheduler_task_prepare(
            0u,
            scheduler_workload_oled_task,
            (void *)0) == 0
    ) {
        console_write_line(context, "SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    if (
        scheduler_task_prepare(
            1u,
            scheduler_workload_cpu_peer_task,
            (void *)0) == 0
    ) {
        console_write_line(context, "SCHED_WORKLOAD_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    console_write(context, "WORKLOAD_CAPACITY=");
    console_write_hex32(context, capacity0);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_T0_USED=");
    console_write_hex32(context, used0);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_T1_USED=");
    console_write_hex32(context, used1);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_SWITCHES=");
    console_write_hex32(context, switches);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_UI_RESULT=");
    console_write_hex32(context, scheduler_workload_ui_result);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_PEER_OVERLAP=");
    console_write_hex32(context, scheduler_workload_peer_overlap);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_CANARY_T0=");
    console_write_hex32(context, (canary0 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    console_write(context, "WORKLOAD_CANARY_T1=");
    console_write_hex32(context, (canary1 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_workload_task0_started != 0u) &&
        (scheduler_workload_task0_done != 0u) &&
        (scheduler_workload_peer_done != 0u) &&
        (scheduler_workload_peer_overlap != 0u) &&
        (scheduler_workload_ui_result != 0u) &&
        (switches >= 2u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    if (passed != 0)
    {
        console_write_line(context, "SCHED_WORKLOAD_OK");
    }
    else
    {
        console_write_line(context, "SCHED_WORKLOAD_ERR");
    }
}


static const char * const scheduler_console_probe_commands
    [SCHED_CONSOLE_PROBE_COMMAND_COUNT] =
{
    "ping",
    "uptime",
    "health",
    "rxstat",
    "mspstat",
    "fault",
    "i2cscan",
    "oledping",
    "oledtest",
    "oledtext",
    "oledrender",
    "oledconsole",
    "oledscroll",
    "oleddirty",
    "oleduiupdate",
    "uiruntime",
    "oledstatus"
};

static void scheduler_console_probe_task(void *argument)
{
    command_service_context_t *context =
        (command_service_context_t *)argument;
    uint32_t index;

    if (context == (command_service_context_t *)0)
    {
        scheduler_console_probe_task_done = 1u;
        return;
    }

    scheduler_console_probe_task_started = 1u;

    for (index = 0u;
         index < SCHED_CONSOLE_PROBE_COMMAND_COUNT;
         ++index)
    {
        command_service_request_t request =
        {
            command_service_find(scheduler_console_probe_commands[index]),
            0u,
            { (const char *)0 }
        };

        if (
            (request.descriptor != (const command_service_descriptor_t *)0) &&
            (scheduler_diagnostics_execute_bound_request(&request, context) ==
                COMMAND_SERVICE_STATUS_OK)
        ) {
            ++scheduler_console_probe_completed_count;
        }
    }

    scheduler_console_probe_ui_restore_result =
        (scheduler_diagnostics_ui_show() != 0) ? 1u : 0u;

    scheduler_console_probe_task_done = 1u;
}

static void scheduler_console_probe_peer_task(void *argument)
{
    volatile uint32_t spin;

    if (argument != (void *)0)
    {
        scheduler_console_probe_peer_done = 1u;
        return;
    }

    for (spin = 0u;
         spin < SCHED_WORKLOAD_PEER_SPINS;
         ++spin)
    {
        if (
            (scheduler_console_probe_task_started != 0u) &&
            (scheduler_console_probe_task_done == 0u)
        ) {
            scheduler_console_probe_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_console_probe_peer_done = 1u;
}

static void console_scheduler_console_probe_test(command_service_context_t *context)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t margin0;
    uint32_t switches;
    int canary0;
    int canary1;
    int bind_result;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        console_write_line(context, "SCHED_CONSOLE_PROBE_PREPARE_ERR");
        return;
    }

    scheduler_console_probe_task_started = 0u;
    scheduler_console_probe_task_done = 0u;
    scheduler_console_probe_peer_overlap = 0u;
    scheduler_console_probe_peer_done = 0u;
    scheduler_console_probe_completed_count = 0u;
    scheduler_console_probe_ui_restore_result = 0u;

    bind_result =
        scheduler_task_stack_bind(
            0u,
            scheduler_diagnostics_bindings->console_stack,
            scheduler_diagnostics_bindings->console_stack_words);

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_console_probe_task,
            (void *)context);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_console_probe_peer_task,
            (void *)0);

    if (
        (bind_result == 0) ||
        (prepare0 == 0) ||
        (prepare1 == 0)
    ) {
        console_write_line(context, "SCHED_CONSOLE_PROBE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    margin0 = (used0 < capacity0) ? (capacity0 - used0) : 0u;
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    console_write(context, "CONSOLE_PROBE_CAPACITY=");
    console_write_hex32(context, capacity0);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_USED=");
    console_write_hex32(context, used0);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_MARGIN=");
    console_write_hex32(context, margin0);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_PEER_CAPACITY=");
    console_write_hex32(context, capacity1);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_PEER_USED=");
    console_write_hex32(context, used1);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_SWITCHES=");
    console_write_hex32(context, switches);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_OVERLAP=");
    console_write_hex32(context, scheduler_console_probe_peer_overlap);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_SURFACE_COUNT=");
    console_write_hex32(context, SCHED_CONSOLE_PROBE_COMMAND_COUNT);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_COMPLETED=");
    console_write_hex32(context, scheduler_console_probe_completed_count);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_UI_RESTORE=");
    console_write_hex32(context, scheduler_console_probe_ui_restore_result);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_CANARY=");
    console_write_hex32(context, (canary0 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    console_write(context, "CONSOLE_PROBE_PEER_CANARY=");
    console_write_hex32(context, (canary1 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_console_probe_task_started != 0u) &&
        (scheduler_console_probe_task_done != 0u) &&
        (scheduler_console_probe_peer_done != 0u) &&
        (scheduler_console_probe_peer_overlap != 0u) &&
        (scheduler_console_probe_completed_count ==
            SCHED_CONSOLE_PROBE_COMMAND_COUNT) &&
        (scheduler_console_probe_ui_restore_result != 0u) &&
        (switches >= 2u) &&
        (capacity0 == (scheduler_diagnostics_bindings->console_stack_words * 4u)) &&
        (capacity1 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (margin0 >= scheduler_diagnostics_bindings->console_min_margin_bytes) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    if (passed != 0)
    {
        console_write_line(context, "SCHED_CONSOLE_PROBE_OK");
    }
    else
    {
        console_write_line(context, "SCHED_CONSOLE_PROBE_ERR");
    }
}

static void scheduler_wait_wake_task(void *argument)
{
    command_service_context_t *context =
        (command_service_context_t *)argument;
    uint32_t events;
    char byte = 0;

    if (context == (command_service_context_t *)0)
    {
        scheduler_wait_wake_task_resumed = 1u;
        return;
    }

    scheduler_wait_wake_task_started = 1u;

    /*
     * console_drain_uart_rx() executes a UART command as soon as it consumes
     * CR or LF.
     * With a normal CRLF host line, the second delimiter can still be queued
     * when this scheduler diagnostic starts. Framing bytes are not payload.
     */
    while (scheduler_diagnostics_uart_try_getc(&byte) != 0)
    {
        if ((byte == '\r') || (byte == '\n'))
        {
            ++scheduler_wait_wake_framing_bytes;
            continue;
        }

        scheduler_wait_wake_byte =
            (uint32_t)(uint8_t)byte;
        scheduler_wait_wake_task_resumed = 1u;
        return;
    }

    /*
     * This token now means the scheduler is active and the wait task itself
     * has reached the wait protocol. A UART event racing with the following
     * SVC is safely latched by scheduler_event_signal().
     */
    console_write_line(context, "SCHED_WAIT_WAKE_ARMED");

    for (;;)
    {
        events =
            scheduler_wait_events(
                scheduler_diagnostics_bindings->uart_rx_event_mask);

        scheduler_wait_wake_events |= events;

        if ((events & scheduler_diagnostics_bindings->uart_rx_event_mask) == 0u)
        {
            break;
        }

        while (scheduler_diagnostics_uart_try_getc(&byte) != 0)
        {
            if ((byte == '\r') || (byte == '\n'))
            {
                ++scheduler_wait_wake_framing_bytes;
                continue;
            }

            scheduler_wait_wake_byte =
                (uint32_t)(uint8_t)byte;

            if (
                (uint32_t)(uint8_t)byte ==
                SCHED_WAIT_WAKE_SENTINEL
            ) {
                scheduler_wait_wake_byte_ok = 1u;
            }

            scheduler_wait_wake_task_resumed = 1u;
            return;
        }
    }

    scheduler_wait_wake_task_resumed = 1u;
}

static void scheduler_wait_wake_peer_task(void *argument)
{
    if (argument == (void *)0)
    {
        scheduler_wait_wake_peer_done = 1u;
    }
}

static void console_scheduler_wait_wake_test(command_service_context_t *context)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t idle_waits;
    int canary0;
    int canary1;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        console_write_line(context, "SCHED_WAIT_WAKE_PREPARE_ERR");
        return;
    }

    scheduler_wait_wake_task_started = 0u;
    scheduler_wait_wake_task_resumed = 0u;
    scheduler_wait_wake_peer_done = 0u;
    scheduler_wait_wake_events = 0u;
    scheduler_wait_wake_byte = 0u;
    scheduler_wait_wake_byte_ok = 0u;
    scheduler_wait_wake_framing_bytes = 0u;

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_wait_wake_task,
            (void *)context);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_wait_wake_peer_task,
            (void *)0);

    if ((prepare0 == 0) || (prepare1 == 0))
    {
        console_write_line(context, "SCHED_WAIT_WAKE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    idle_waits = scheduler_idle_wait_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    console_write(context, "SCHED_WAIT_WAKE_EVENTS=");
    console_write_hex32(context, scheduler_wait_wake_events);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_BYTE=");
    console_write_hex32(context, scheduler_wait_wake_byte);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_FRAMING_BYTES=");
    console_write_hex32(context, scheduler_wait_wake_framing_bytes);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_IDLE_WAITS=");
    console_write_hex32(context, idle_waits);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_USED_T0=");
    console_write_hex32(context, used0);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_USED_T1=");
    console_write_hex32(context, used1);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_CANARY_T0=");
    console_write_hex32(context, (canary0 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    console_write(context, "SCHED_WAIT_WAKE_CANARY_T1=");
    console_write_hex32(context, (canary1 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_wait_wake_task_started != 0u) &&
        (scheduler_wait_wake_task_resumed != 0u) &&
        (scheduler_wait_wake_peer_done != 0u) &&
        (scheduler_wait_wake_events ==
            scheduler_diagnostics_bindings->uart_rx_event_mask) &&
        (scheduler_wait_wake_byte_ok != 0u) &&
        (idle_waits != 0u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE) &&
        (task0->wait_events == 0u) &&
        (task1->wait_events == 0u) &&
        (scheduler_is_active() == 0);

    if (passed != 0)
    {
        console_write_line(context, "SCHED_WAIT_WAKE_OK");
    }
    else
    {
        console_write_line(context, "SCHED_WAIT_WAKE_ERR");
    }
}

static const char * const scheduler_isolation_diagnostic_commands
    [SCHED_ISOLATION_DIAGNOSTIC_COUNT] =
{
    "schedtest",
    "schedcoop",
    "schedpreempt",
    "schedstack",
    "schedworkload",
    "schedconsoleprobe",
    "schedwaitwake"
};

static void scheduler_isolation_task(void *argument)
{
    command_service_context_t *context =
        (command_service_context_t *)argument;
    uint32_t index;

    if (context == (command_service_context_t *)0)
    {
        scheduler_isolation_active_preserved = 0u;
        scheduler_isolation_task_done = 1u;
        return;
    }

    scheduler_isolation_task_started = 1u;

    if (scheduler_init() == 0)
    {
        scheduler_isolation_init_reject = 1u;
    }

    if (scheduler_is_active() == 0)
    {
        scheduler_isolation_active_preserved = 0u;
    }

    for (index = 0u;
         index < SCHED_ISOLATION_DIAGNOSTIC_COUNT;
         ++index)
    {
        command_service_request_t request =
        {
            command_service_find(scheduler_isolation_diagnostic_commands[index]),
            0u,
            { (const char *)0 }
        };

        if (
            (request.descriptor == (const command_service_descriptor_t *)0) ||
            (scheduler_diagnostics_execute_diagnostic(&request, context, scheduler_diagnostics_bindings) !=
                COMMAND_SERVICE_STATUS_BUSY)
        ) {
            scheduler_isolation_active_preserved = 0u;
        }

        if (scheduler_is_active() == 0)
        {
            scheduler_isolation_active_preserved = 0u;
        }
    }

    scheduler_isolation_task_done = 1u;
}

static void scheduler_isolation_peer_task(void *argument)
{
    volatile uint32_t spin;

    if (argument != (void *)0)
    {
        scheduler_isolation_peer_done = 1u;
        return;
    }

    for (spin = 0u;
         spin < SCHED_WORKLOAD_PEER_SPINS;
         ++spin)
    {
        if (
            (scheduler_isolation_task_started != 0u) &&
            (scheduler_isolation_task_done == 0u)
        ) {
            scheduler_isolation_peer_overlap = 1u;
        }

        __asm volatile ("nop");
    }

    scheduler_isolation_peer_done = 1u;
}

static void console_scheduler_isolation_test(command_service_context_t *context)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    uint32_t used0;
    uint32_t used1;
    uint32_t capacity0;
    uint32_t capacity1;
    uint32_t switches;
    int canary0;
    int canary1;
    int prepare0;
    int prepare1;
    int start_result;
    int passed;

    if (scheduler_init() == 0)
    {
        console_write_line(context, "SCHED_ISOLATE_PREPARE_ERR");
        return;
    }

    scheduler_isolation_task_started = 0u;
    scheduler_isolation_task_done = 0u;
    scheduler_isolation_peer_overlap = 0u;
    scheduler_isolation_peer_done = 0u;
    scheduler_isolation_init_reject = 0u;
    scheduler_isolation_active_preserved = 1u;
    scheduler_diagnostic_busy_count = 0u;

    prepare0 =
        scheduler_task_prepare(
            0u,
            scheduler_isolation_task,
            (void *)context);

    prepare1 =
        scheduler_task_prepare(
            1u,
            scheduler_isolation_peer_task,
            (void *)0);

    if ((prepare0 == 0) || (prepare1 == 0))
    {
        console_write_line(context, "SCHED_ISOLATE_PREPARE_ERR");
        return;
    }

    start_result = scheduler_start_preemptive();

    used0 = scheduler_stack_high_water_bytes(0u);
    used1 = scheduler_stack_high_water_bytes(1u);
    capacity0 = scheduler_stack_capacity_bytes(0u);
    capacity1 = scheduler_stack_capacity_bytes(1u);
    canary0 = scheduler_stack_canary_intact(0u);
    canary1 = scheduler_stack_canary_intact(1u);
    switches = scheduler_preempt_switch_count_get();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    console_write(context, "SCHED_ISOLATE_CAPACITY_T0=");
    console_write_hex32(context, capacity0);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_USED_T0=");
    console_write_hex32(context, used0);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_CAPACITY_T1=");
    console_write_hex32(context, capacity1);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_USED_T1=");
    console_write_hex32(context, used1);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_SWITCHES=");
    console_write_hex32(context, switches);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_INIT_REJECT=");
    console_write_hex32(context, scheduler_isolation_init_reject);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_BLOCKED_DIAGNOSTICS=");
    console_write_hex32(context, scheduler_diagnostic_busy_count);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_ACTIVE_PRESERVED=");
    console_write_hex32(context, scheduler_isolation_active_preserved);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_OVERLAP=");
    console_write_hex32(context, scheduler_isolation_peer_overlap);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_CANARY_T0=");
    console_write_hex32(context, (canary0 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    console_write(context, "SCHED_ISOLATE_CANARY_T1=");
    console_write_hex32(context, (canary1 != 0) ? 1u : 0u);
    console_write(context, "\r\n");

    passed =
        (start_result != 0) &&
        (scheduler_isolation_task_started != 0u) &&
        (scheduler_isolation_task_done != 0u) &&
        (scheduler_isolation_peer_done != 0u) &&
        (scheduler_isolation_init_reject != 0u) &&
        (scheduler_diagnostic_busy_count ==
            SCHED_ISOLATION_DIAGNOSTIC_COUNT) &&
        (scheduler_isolation_active_preserved != 0u) &&
        (scheduler_isolation_peer_overlap != 0u) &&
        (switches >= 2u) &&
        (capacity0 == (SCHEDULER_TASK_STACK_WORDS * 4u)) &&
        (capacity1 == capacity0) &&
        (used0 >= 64u) &&
        (used0 < capacity0) &&
        (used1 >= 64u) &&
        (used1 < capacity1) &&
        (canary0 != 0) &&
        (canary1 != 0) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE) &&
        (scheduler_is_active() == 0);

    if (passed != 0)
    {
        console_write_line(context, "SCHED_ISOLATE_OK");
    }
    else
    {
        console_write_line(context, "SCHED_ISOLATE_ERR");
    }
}

static void console_scheduler_priority_test(command_service_context_t *context)
{
    const scheduler_task_t *task0 = scheduler_task_get(0u);
    const scheduler_task_t *task1 = scheduler_task_get(1u);
    uint32_t task0_priority_before = 0xFFFFFFFFu;
    uint32_t task0_priority_after = 0xFFFFFFFFu;
    uint32_t task1_priority_before = 0xFFFFFFFFu;
    uint32_t task1_priority_after = 0xFFFFFFFFu;
    uint32_t self_test;
    int task0_active_set_result;
    int task1_active_set_result;
    int passed = 1;

    if (task0 != (const scheduler_task_t *)0)
    {
        task0_priority_before = task0->priority;
    }

    if (task1 != (const scheduler_task_t *)0)
    {
        task1_priority_before = task1->priority;
    }

    self_test = scheduler_priority_self_test();

    task0_active_set_result =
        scheduler_task_priority_set(
            0u,
            SCHEDULER_PRIORITY_HIGHEST);

    task1_active_set_result =
        scheduler_task_priority_set(
            1u,
            SCHEDULER_PRIORITY_DEFAULT);

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    if (task0 != (const scheduler_task_t *)0)
    {
        task0_priority_after = task0->priority;
    }

    if (task1 != (const scheduler_task_t *)0)
    {
        task1_priority_after = task1->priority;
    }

    console_write_line(context, "SCHED_PRIO_POLICY=LOWER_VALUE_HIGHER");

    console_write(context, "SCHED_PRIO_HIGHEST=");
    console_write_hex32(context, SCHEDULER_PRIORITY_HIGHEST);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PRIO_DEFAULT=");
    console_write_hex32(context, SCHEDULER_PRIORITY_DEFAULT);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PRIO_LOWEST=");
    console_write_hex32(context, SCHEDULER_PRIORITY_LOWEST);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PRIO_TASK0=");
    console_write_hex32(context, task0_priority_after);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PRIO_TASK1=");
    console_write_hex32(context, task1_priority_after);
    console_write(context, "\r\n");

    console_write(context, "SCHED_PRIO_SELFTEST=");
    console_write_hex32(context, self_test);
    console_write(context, "\r\n");

    if (
        (scheduler_is_active() != 0) &&
        (task0_active_set_result == 0) &&
        (task1_active_set_result == 0) &&
        (task0_priority_before == SCHEDULER_PRIORITY_DEFAULT) &&
        (task1_priority_before == SCHEDULER_PRIORITY_LOWEST) &&
        (task0_priority_after == task0_priority_before) &&
        (task1_priority_after == task1_priority_before)
    ) {
        console_write_line(context, "SCHED_PRIO_ACTIVE_SET_REJECT_OK");
    }
    else
    {
        console_write_line(context, "SCHED_PRIO_ACTIVE_SET_REJECT_ERR");
        passed = 0;
    }

    if (
        (task0_priority_after != SCHEDULER_PRIORITY_DEFAULT) ||
        (task1_priority_after != SCHEDULER_PRIORITY_LOWEST) ||
        (self_test != 0x0000003Fu)
    ) {
        passed = 0;
    }

    if (passed != 0)
    {
        console_write_line(context, "SCHED_PRIO_OK");
    }
    else
    {
        console_write_line(context, "SCHED_PRIO_ERR");
    }
}

static void console_scheduler_timed_test(command_service_context_t *context)
{
    const uint32_t rx_event =
        context->source_rx_event_mask;
    kernel_time_ms_t start;
    kernel_time_ms_t elapsed;
    uint32_t events;
    int passed = 1;

    if (scheduler_sleep_ms(0u) != 0)
    {
        console_write_line(context, "SCHED_TIMED_ZERO_OK");
    }
    else
    {
        console_write_line(context, "SCHED_TIMED_ZERO_ERR");
        passed = 0;
    }

    start = kernel_time_now();

    if (scheduler_sleep_ms(SCHED_TIMED_SLEEP_MS) == 0)
    {
        console_write_line(context, "SCHED_TIMED_SLEEP_ERR");
        passed = 0;
    }

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    console_write(context, "SCHED_TIMED_SLEEP_ELAPSED=");
    console_write_hex32(context, elapsed);
    console_write(context, "\r\n");

    if (elapsed >= SCHED_TIMED_SLEEP_MS)
    {
        console_write_line(context, "SCHED_TIMED_SLEEP_OK");
    }
    else
    {
        console_write_line(context, "SCHED_TIMED_SLEEP_ERR");
        passed = 0;
    }

    /* Consume a stale notification from the transport that started this test. */
    (void)scheduler_wait_events_timeout(
        rx_event,
        0u);

    start = kernel_time_now();

    events =
        scheduler_wait_events_timeout(
            rx_event,
            SCHED_TIMED_TIMEOUT_MS);

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    console_write(context, "SCHED_TIMED_TIMEOUT_ELAPSED=");
    console_write_hex32(context, elapsed);
    console_write(context, "\r\n");

    if (
        (events == 0u) &&
        (elapsed >= SCHED_TIMED_TIMEOUT_MS)
    ) {
        console_write_line(context, "SCHED_TIMED_TIMEOUT_OK");
    }
    else
    {
        console_write_line(context, "SCHED_TIMED_TIMEOUT_ERR");
        passed = 0;
    }

    (void)scheduler_wait_events_timeout(
        rx_event,
        0u);

    console_write_line(context, "SCHED_TIMED_EVENT_ARMED");
    start = kernel_time_now();

    events =
        scheduler_wait_events_timeout(
            rx_event,
            SCHED_TIMED_EVENT_TIMEOUT_MS);

    elapsed =
        (kernel_time_ms_t)(kernel_time_now() - start);

    console_write(context, "SCHED_TIMED_EVENT_EVENTS=");
    console_write_hex32(context, events);
    console_write(context, "\r\n");

    console_write(context, "SCHED_TIMED_EVENT_ELAPSED=");
    console_write_hex32(context, elapsed);
    console_write(context, "\r\n");

    if (
        (events == rx_event) &&
        (elapsed < SCHED_TIMED_EVENT_TIMEOUT_MS)
    ) {
        console_write_line(context, "SCHED_TIMED_EVENT_OK");
    }
    else
    {
        console_write_line(context, "SCHED_TIMED_EVENT_ERR");
        passed = 0;
    }

    if (passed != 0)
    {
        console_write_line(context, "SCHED_TIMED_OK");
    }
    else
    {
        console_write_line(context, "SCHED_TIMED_ERR");
    }
}

static int console_scheduler_diagnostic_block_if_active(command_service_context_t *context)
{
    if (scheduler_is_active() == 0)
    {
        return 0;
    }

    ++scheduler_diagnostic_busy_count;
    console_write_line(context, "SCHED_DIAG_BUSY");

    return 1;
}

command_service_status_t scheduler_diagnostics_execute_diagnostic(
    const command_service_request_t *request,
    command_service_context_t *context,
    const scheduler_diagnostics_bindings_t *bindings)
{
    if (console_scheduler_diagnostic_block_if_active(context) != 0)
    {
        return COMMAND_SERVICE_STATUS_BUSY;
    }

    scheduler_diagnostics_bindings = bindings;

    switch (request->descriptor->method_id)
    {
        case COMMAND_SERVICE_METHOD_SCHEDTEST:
            console_scheduler_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDCOOP:
            console_scheduler_cooperative_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDPREEMPT:
            console_scheduler_preemptive_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDSTACK:
            console_scheduler_stack_water_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDWORKLOAD:
            console_scheduler_workload_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDCONSOLEPROBE:
            console_scheduler_console_probe_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDWAITWAKE:
            console_scheduler_wait_wake_test(context);
            break;

        case COMMAND_SERVICE_METHOD_SCHEDISOLATE:
            console_scheduler_isolation_test(context);
            break;

        default:
            return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    }

    return COMMAND_SERVICE_STATUS_OK;
}

uint32_t scheduler_diagnostics_busy_count(void)
{
    return scheduler_diagnostic_busy_count;
}

command_service_status_t scheduler_diagnostics_execute_timed(
    command_service_context_t *context)
{
    console_scheduler_timed_test(context);
    return COMMAND_SERVICE_STATUS_OK;
}

command_service_status_t scheduler_diagnostics_execute_priority(
    command_service_context_t *context)
{
    console_scheduler_priority_test(context);
    return COMMAND_SERVICE_STATUS_OK;
}
