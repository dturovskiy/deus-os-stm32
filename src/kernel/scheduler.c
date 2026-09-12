#include <stdint.h>
#include "kernel/scheduler.h"

#define SCHEDULER_SOFTWARE_FRAME_WORDS 8u
#define SCHEDULER_HARDWARE_FRAME_WORDS 8u
#define SCHEDULER_INITIAL_FRAME_WORDS \
    (SCHEDULER_SOFTWARE_FRAME_WORDS + SCHEDULER_HARDWARE_FRAME_WORDS)

#define SCHEDULER_INITIAL_XPSR 0x01000000u
#define SCHEDULER_STACK_FILL   0xA5A5A5A5u

static scheduler_task_t scheduler_tasks[SCHEDULER_TASK_COUNT];

static uint32_t scheduler_task_stacks
    [SCHEDULER_TASK_COUNT][SCHEDULER_TASK_STACK_WORDS]
    __attribute__((aligned(8)));

static uint32_t scheduler_code_address(
    scheduler_task_entry_t entry)
{
    return
        (uint32_t)(
            ((uintptr_t)entry) &
            ~((uintptr_t)1u));
}

__attribute__((noreturn))
static void scheduler_task_return_trap(void *argument)
{
    (void)argument;

    for (;;)
    {
        __asm volatile ("nop");
    }
}

static void scheduler_self_test_entry(void *argument)
{
    (void)argument;
}

void scheduler_init(void)
{
    uint32_t task_index;
    uint32_t word_index;

    for (task_index = 0u;
         task_index < SCHEDULER_TASK_COUNT;
         ++task_index)
    {
        scheduler_task_t *task =
            &scheduler_tasks[task_index];

        task->stack_low =
            &scheduler_task_stacks[task_index][0];

        task->stack_high =
            &scheduler_task_stacks
                [task_index][SCHEDULER_TASK_STACK_WORDS];

        task->saved_sp = task->stack_high;
        task->stack_words = SCHEDULER_TASK_STACK_WORDS;
        task->state = SCHEDULER_TASK_UNUSED;

        for (word_index = 0u;
             word_index < SCHEDULER_TASK_STACK_WORDS;
             ++word_index)
        {
            scheduler_task_stacks
                [task_index][word_index] =
                    SCHEDULER_STACK_FILL;
        }
    }
}

int scheduler_task_prepare(
    uint32_t index,
    scheduler_task_entry_t entry,
    void *argument)
{
    scheduler_task_t *task;
    uint32_t *frame;
    uint32_t i;

    if (
        (index >= SCHEDULER_TASK_COUNT) ||
        (entry == (scheduler_task_entry_t)0)
    ) {
        return 0;
    }

    task = &scheduler_tasks[index];

    if (
        (task->stack_low == (uint32_t *)0) ||
        (task->stack_high == (uint32_t *)0) ||
        (task->stack_words < SCHEDULER_INITIAL_FRAME_WORDS) ||
        ((((uintptr_t)task->stack_high) & (uintptr_t)0x7u) != 0u)
    ) {
        return 0;
    }

    frame =
        task->stack_high -
        SCHEDULER_INITIAL_FRAME_WORDS;

    for (i = 0u;
         i < SCHEDULER_SOFTWARE_FRAME_WORDS;
         ++i)
    {
        frame[i] = 0u;
    }

    /*
     * The first eight words model r4-r11 saved by a future PendSV handler.
     * The next eight words are the Cortex-M exception-return hardware frame:
     * r0, r1, r2, r3, r12, lr, pc, xPSR.
     *
     * This slice constructs and validates the frame only. It does not switch
     * CONTROL.SPSEL, activate PSP, install PendSV/SVC handlers, or schedule.
     */
    frame[8] =
        (uint32_t)(uintptr_t)argument;

    frame[9] = 0u;
    frame[10] = 0u;
    frame[11] = 0u;
    frame[12] = 0u;

    frame[13] =
        scheduler_code_address(
            scheduler_task_return_trap);

    frame[14] =
        scheduler_code_address(entry);

    frame[15] = SCHEDULER_INITIAL_XPSR;

    task->saved_sp = frame;
    task->state = SCHEDULER_TASK_READY;

    return 1;
}

const scheduler_task_t *scheduler_task_get(uint32_t index)
{
    if (index >= SCHEDULER_TASK_COUNT)
    {
        return (const scheduler_task_t *)0;
    }

    return &scheduler_tasks[index];
}

static int scheduler_validate_prepared_task(
    const scheduler_task_t *task,
    scheduler_task_entry_t entry,
    void *argument)
{
    const uint32_t *frame;
    uint32_t i;

    if (
        (task == (const scheduler_task_t *)0) ||
        (task->state != SCHEDULER_TASK_READY) ||
        (task->stack_low == (uint32_t *)0) ||
        (task->stack_high == (uint32_t *)0) ||
        (task->stack_words != SCHEDULER_TASK_STACK_WORDS) ||
        (task->saved_sp !=
            (task->stack_high -
             SCHEDULER_INITIAL_FRAME_WORDS)) ||
        ((((uintptr_t)task->saved_sp) & (uintptr_t)0x7u) != 0u) ||
        (task->stack_low[0] != SCHEDULER_STACK_FILL)
    ) {
        return 0;
    }

    frame = task->saved_sp;

    for (i = 0u;
         i < SCHEDULER_SOFTWARE_FRAME_WORDS;
         ++i)
    {
        if (frame[i] != 0u)
        {
            return 0;
        }
    }

    return
        (frame[8] ==
            (uint32_t)(uintptr_t)argument) &&
        (frame[9] == 0u) &&
        (frame[10] == 0u) &&
        (frame[11] == 0u) &&
        (frame[12] == 0u) &&
        (frame[13] ==
            scheduler_code_address(
                scheduler_task_return_trap)) &&
        (frame[14] ==
            scheduler_code_address(entry)) &&
        (frame[15] == SCHEDULER_INITIAL_XPSR);
}

int scheduler_self_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;

    scheduler_init();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    if (
        (task0 == (const scheduler_task_t *)0) ||
        (task1 == (const scheduler_task_t *)0) ||
        (scheduler_task_get(SCHEDULER_TASK_COUNT) !=
            (const scheduler_task_t *)0)
    ) {
        return 0;
    }

    if (
        (task0->state != SCHEDULER_TASK_UNUSED) ||
        (task1->state != SCHEDULER_TASK_UNUSED) ||
        (task0->stack_words != SCHEDULER_TASK_STACK_WORDS) ||
        (task1->stack_words != SCHEDULER_TASK_STACK_WORDS) ||
        (task0->saved_sp != task0->stack_high) ||
        (task1->saved_sp != task1->stack_high) ||
        ((((uintptr_t)task0->stack_low) & (uintptr_t)0x7u) != 0u) ||
        ((((uintptr_t)task1->stack_low) & (uintptr_t)0x7u) != 0u) ||
        (task0->stack_high != task1->stack_low)
    ) {
        return 0;
    }

    if (
        scheduler_task_prepare(
            0u,
            scheduler_self_test_entry,
            (void *)task0) == 0
    ) {
        return 0;
    }

    if (
        scheduler_task_prepare(
            1u,
            scheduler_self_test_entry,
            (void *)task1) == 0
    ) {
        return 0;
    }

    if (
        scheduler_validate_prepared_task(
            task0,
            scheduler_self_test_entry,
            (void *)task0) == 0
    ) {
        return 0;
    }

    if (
        scheduler_validate_prepared_task(
            task1,
            scheduler_self_test_entry,
            (void *)task1) == 0
    ) {
        return 0;
    }

    /*
     * Restore the foundation to its pre-start state. The self-test must not
     * leave a task logically READY because this slice never schedules it.
     */
    scheduler_init();

    return
        (task0->state == SCHEDULER_TASK_UNUSED) &&
        (task1->state == SCHEDULER_TASK_UNUSED) &&
        (task0->saved_sp == task0->stack_high) &&
        (task1->saved_sp == task1->stack_high);
}
