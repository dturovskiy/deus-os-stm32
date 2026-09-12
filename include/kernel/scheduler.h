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
    SCHEDULER_TASK_DONE = 2
} scheduler_task_state_t;

typedef struct
{
    uint32_t * volatile saved_sp;
    uint32_t *stack_low;
    uint32_t *stack_high;
    uint32_t stack_words;
    volatile scheduler_task_state_t state;
} scheduler_task_t;

void scheduler_init(void);

int scheduler_task_prepare(
    uint32_t index,
    scheduler_task_entry_t entry,
    void *argument);

const scheduler_task_t *scheduler_task_get(uint32_t index);

int scheduler_start(void);

void scheduler_yield(void);

int scheduler_self_test(void);

int scheduler_cooperative_self_test(void);

#endif
