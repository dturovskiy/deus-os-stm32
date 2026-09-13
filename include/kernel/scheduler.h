#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include <stdint.h>

#define SCHEDULER_TASK_COUNT       2u
#define SCHEDULER_TASK_STACK_WORDS 128u

typedef void (*scheduler_task_entry_t)(void *argument);

typedef enum
{
    SCHEDULER_TASK_UNUSED = 0,
    SCHEDULER_TASK_READY = 1,
    SCHEDULER_TASK_DONE = 2,
    SCHEDULER_TASK_BLOCKED = 3
} scheduler_task_state_t;

typedef struct
{
    uint32_t * volatile saved_sp;
    uint32_t *stack_low;
    uint32_t *stack_high;
    uint32_t stack_words;
    volatile scheduler_task_state_t state;
    volatile uint32_t wait_events;
    volatile uint32_t wake_events;
} scheduler_task_t;

int scheduler_init(void);

int scheduler_is_active(void);

int scheduler_task_stack_bind(
    uint32_t index,
    uint32_t *stack_low,
    uint32_t stack_words);

int scheduler_task_prepare(
    uint32_t index,
    scheduler_task_entry_t entry,
    void *argument);

const scheduler_task_t *scheduler_task_get(uint32_t index);

int scheduler_start(void);

int scheduler_start_preemptive(void);

uint32_t scheduler_preempt_switch_count_get(void);

/*
 * Block the current scheduler task until any requested event bit is
 * signaled. A matching event already pending in the active scheduler run
 * is consumed immediately. The returned value is the matched event mask.
 */
uint32_t scheduler_wait_events(uint32_t events);

/*
 * Signal event bits to the active scheduler run. All currently blocked
 * matching tasks are made READY; unmatched bits remain pending until a
 * future waiter consumes them. Calls while the scheduler is inactive are
 * intentionally ignored.
 */
void scheduler_event_signal(uint32_t events);

/* Number of host-MSP WFE park iterations in the current scheduler run. */
uint32_t scheduler_idle_wait_count_get(void);

void scheduler_yield(void);

void scheduler_tick(void);

int scheduler_self_test(void);

int scheduler_cooperative_self_test(void);

int scheduler_preemptive_self_test(void);

uint32_t scheduler_stack_high_water_bytes(uint32_t index);

uint32_t scheduler_stack_capacity_bytes(uint32_t index);

int scheduler_stack_canary_intact(uint32_t index);

#endif
