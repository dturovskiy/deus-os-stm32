#include <stdint.h>
#include "kernel/scheduler.h"

#define SCHEDULER_SOFTWARE_FRAME_WORDS 8u
#define SCHEDULER_HARDWARE_FRAME_WORDS 8u
#define SCHEDULER_INITIAL_FRAME_WORDS \
    (SCHEDULER_SOFTWARE_FRAME_WORDS + SCHEDULER_HARDWARE_FRAME_WORDS)

#define SCHEDULER_INITIAL_XPSR 0x01000000u
#define SCHEDULER_STACK_FILL   0xA5A5A5A5u

#define SCHEDULER_EXC_RETURN_THREAD_MSP 0xFFFFFFF9u
#define SCHEDULER_EXC_RETURN_THREAD_PSP 0xFFFFFFFDu

#define SCHEDULER_SVC_START 0u
#define SCHEDULER_SVC_YIELD 1u
#define SCHEDULER_SVC_EXIT  2u

#define SCHEDULER_PREEMPT_SPIN_LIMIT 5000000u

#define SCB_ICSR (*(volatile uint32_t *)0xE000ED04u)
#define SCB_SHPR3 (*(volatile uint32_t *)0xE000ED20u)

#define SCB_ICSR_PENDSVCLR       (1u << 27)
#define SCB_ICSR_PENDSVSET       (1u << 28)
#define SCB_SHPR3_PENDSV_MASK    (0xFFu << 16)
#define SCB_SHPR3_PENDSV_LOWEST  (0xFFu << 16)

#define SCHEDULER_NO_TASK SCHEDULER_TASK_COUNT

static scheduler_task_t scheduler_tasks[SCHEDULER_TASK_COUNT];

static uint32_t scheduler_task_stacks
    [SCHEDULER_TASK_COUNT][SCHEDULER_TASK_STACK_WORDS]
    __attribute__((aligned(8)));

uint32_t scheduler_host_r4_r11[8]
    __attribute__((used, aligned(8)));

static volatile uint32_t scheduler_current_index;
static volatile uint32_t scheduler_active;
static volatile uint32_t scheduler_completed_count;
static volatile uint32_t scheduler_target_count;
static volatile uint32_t scheduler_run_result;
static volatile uint32_t scheduler_preempt_enabled;
static volatile uint32_t scheduler_preempt_switch_count;

static volatile uint32_t scheduler_coop_sequence[4];
static volatile uint32_t scheduler_coop_sequence_count;
static uint32_t scheduler_coop_arg0;
static uint32_t scheduler_coop_arg1;
static volatile uint32_t scheduler_coop_error;

static volatile uint32_t scheduler_preempt_sequence[6];
static volatile uint32_t scheduler_preempt_sequence_count;
static volatile uint32_t scheduler_preempt_phase;
static volatile uint32_t scheduler_preempt_error;
static uint32_t scheduler_preempt_arg0;
static uint32_t scheduler_preempt_arg1;

static uint32_t scheduler_exception_pc_address(
    scheduler_task_entry_t entry)
{
    return
        (uint32_t)(
            ((uintptr_t)entry) &
            ~((uintptr_t)1u));
}

static int scheduler_frame_pointer_valid(
    const scheduler_task_t *task,
    const uint32_t *saved_sp)
{
    uintptr_t low;
    uintptr_t high;
    uintptr_t sp;
    uintptr_t frame_bytes =
        (uintptr_t)(
            SCHEDULER_INITIAL_FRAME_WORDS *
            sizeof(uint32_t));

    if (
        (task == (const scheduler_task_t *)0) ||
        (saved_sp == (const uint32_t *)0) ||
        (task->stack_low == (uint32_t *)0) ||
        (task->stack_high == (uint32_t *)0)
    ) {
        return 0;
    }

    low = (uintptr_t)task->stack_low;
    high = (uintptr_t)task->stack_high;
    sp = (uintptr_t)saved_sp;

    return
        ((sp & (uintptr_t)0x7u) == 0u) &&
        (sp >= low) &&
        (sp <= (high - frame_bytes));
}

static uint32_t scheduler_find_next_ready(uint32_t after_index)
{
    uint32_t offset;

    for (offset = 1u;
         offset <= SCHEDULER_TASK_COUNT;
         ++offset)
    {
        uint32_t index =
            (after_index + offset) %
            SCHEDULER_TASK_COUNT;

        if (scheduler_tasks[index].state == SCHEDULER_TASK_READY)
        {
            return index;
        }
    }

    return SCHEDULER_NO_TASK;
}

static uint32_t scheduler_decode_svc_number(
    const uint32_t *saved_sp)
{
    uint32_t stacked_pc;
    const uint16_t *svc_instruction;
    uint16_t instruction;

    if (saved_sp == (const uint32_t *)0)
    {
        return 0xFFFFFFFFu;
    }

    stacked_pc = saved_sp[14];

    if (stacked_pc < 2u)
    {
        return 0xFFFFFFFFu;
    }

    svc_instruction =
        (const uint16_t *)(uintptr_t)(stacked_pc - 2u);

    instruction = *svc_instruction;

    if ((instruction & 0xFF00u) != 0xDF00u)
    {
        return 0xFFFFFFFFu;
    }

    return (uint32_t)(instruction & 0x00FFu);
}

static void scheduler_clear_pending_pendsv(void)
{
    SCB_ICSR = SCB_ICSR_PENDSVCLR;

    __asm volatile (
        "dsb\n"
        "isb\n"
        ::: "memory");
}

static void scheduler_abort_run(void)
{
    scheduler_preempt_enabled = 0u;
    scheduler_clear_pending_pendsv();

    scheduler_active = 0u;
    scheduler_current_index = SCHEDULER_NO_TASK;
    scheduler_run_result = 0u;
}

__attribute__((noreturn, noinline))
static void scheduler_task_return_trap(void *argument)
{
    (void)argument;

    __asm volatile ("svc #2" ::: "memory");

    for (;;)
    {
        __asm volatile ("nop");
    }
}

static void scheduler_self_test_entry(void *argument)
{
    (void)argument;
}

static void scheduler_coop_record(uint32_t value)
{
    if (scheduler_coop_sequence_count < 4u)
    {
        scheduler_coop_sequence[
            scheduler_coop_sequence_count] =
                value;

        ++scheduler_coop_sequence_count;
    }
    else
    {
        scheduler_coop_error = 1u;
    }
}

static void scheduler_coop_task0(void *argument)
{
    if (argument != (void *)&scheduler_coop_arg0)
    {
        scheduler_coop_error = 1u;
        return;
    }

    scheduler_coop_record(0x10u);
    scheduler_yield();
    scheduler_coop_record(0x11u);
}

static void scheduler_coop_task1(void *argument)
{
    if (argument != (void *)&scheduler_coop_arg1)
    {
        scheduler_coop_error = 1u;
        return;
    }

    scheduler_coop_record(0x20u);
    scheduler_yield();
    scheduler_coop_record(0x21u);
}

static void scheduler_configure_pendsv_priority(void)
{
    uint32_t shpr3 = SCB_SHPR3;

    shpr3 &= ~SCB_SHPR3_PENDSV_MASK;
    shpr3 |= SCB_SHPR3_PENDSV_LOWEST;

    SCB_SHPR3 = shpr3;
}

static void scheduler_preempt_record(uint32_t value)
{
    if (scheduler_preempt_sequence_count < 6u)
    {
        scheduler_preempt_sequence[
            scheduler_preempt_sequence_count] =
                value;

        ++scheduler_preempt_sequence_count;
    }
    else
    {
        scheduler_preempt_error = 1u;
    }
}

static int scheduler_preempt_wait_for(
    uint32_t mask)
{
    uint32_t spins = SCHEDULER_PREEMPT_SPIN_LIMIT;

    while ((scheduler_preempt_phase & mask) == 0u)
    {
        if (spins == 0u)
        {
            scheduler_preempt_error = 1u;
            return 0;
        }

        --spins;
    }

    return 1;
}

static void scheduler_preempt_task0(void *argument)
{
    if (argument != (void *)&scheduler_preempt_arg0)
    {
        scheduler_preempt_error = 1u;
        return;
    }

    scheduler_preempt_record(0x30u);
    scheduler_preempt_phase |= 0x01u;

    if (scheduler_preempt_wait_for(0x02u) == 0)
    {
        return;
    }

    scheduler_preempt_record(0x31u);
    scheduler_preempt_phase |= 0x04u;

    if (scheduler_preempt_wait_for(0x08u) == 0)
    {
        return;
    }

    scheduler_preempt_record(0x32u);
}

static void scheduler_preempt_task1(void *argument)
{
    if (argument != (void *)&scheduler_preempt_arg1)
    {
        scheduler_preempt_error = 1u;
        return;
    }

    if ((scheduler_preempt_phase & 0x01u) == 0u)
    {
        scheduler_preempt_error = 1u;
        return;
    }

    scheduler_preempt_record(0x40u);
    scheduler_preempt_phase |= 0x02u;

    if (scheduler_preempt_wait_for(0x04u) == 0)
    {
        return;
    }

    scheduler_preempt_record(0x41u);
    scheduler_preempt_phase |= 0x08u;
    scheduler_preempt_record(0x42u);
}

void scheduler_init(void)
{
    uint32_t task_index;
    uint32_t word_index;

    scheduler_current_index = SCHEDULER_NO_TASK;
    scheduler_active = 0u;
    scheduler_completed_count = 0u;
    scheduler_target_count = 0u;
    scheduler_run_result = 0u;
    scheduler_preempt_enabled = 0u;
    scheduler_preempt_switch_count = 0u;

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
        (entry == (scheduler_task_entry_t)0) ||
        (scheduler_active != 0u)
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
     * The first eight words model r4-r11 saved by the cooperative SVC
     * handler. The next eight words are the Cortex-M hardware frame:
     * r0, r1, r2, r3, r12, lr, pc, xPSR.
     */
    frame[8] =
        (uint32_t)(uintptr_t)argument;

    frame[9] = 0u;
    frame[10] = 0u;
    frame[11] = 0u;
    frame[12] = 0u;

    /*
     * Exception return restores Thumb state from xPSR.T, so the initial PC
     * is stored halfword-aligned. If the task entry later returns normally,
     * LR is consumed by BX LR and therefore must retain the Thumb-state bit.
     */
    frame[13] =
        (uint32_t)(uintptr_t)
            scheduler_task_return_trap;

    frame[14] =
        scheduler_exception_pc_address(entry);

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
        (scheduler_frame_pointer_valid(
            task,
            task->saved_sp) == 0) ||
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
            (uint32_t)(uintptr_t)
                scheduler_task_return_trap) &&
        ((frame[13] & 1u) != 0u) &&
        (frame[14] ==
            scheduler_exception_pc_address(entry)) &&
        ((frame[14] & 1u) == 0u) &&
        (frame[15] == SCHEDULER_INITIAL_XPSR);
}

uint32_t *scheduler_svc_dispatch(
    uint32_t *saved_sp,
    uint32_t exc_return)
{
    uint32_t next_index;
    uint32_t svc_number;
    scheduler_task_t *current_task;
    scheduler_task_t *next_task;

    if ((exc_return & 4u) == 0u)
    {
        if (
            (scheduler_active == 0u) ||
            (scheduler_current_index != SCHEDULER_NO_TASK) ||
            (saved_sp != (uint32_t *)0)
        ) {
            scheduler_abort_run();
            return (uint32_t *)0;
        }

        next_index =
            scheduler_find_next_ready(
                SCHEDULER_TASK_COUNT - 1u);

        if (next_index >= SCHEDULER_TASK_COUNT)
        {
            scheduler_abort_run();
            return (uint32_t *)0;
        }

        next_task = &scheduler_tasks[next_index];

        if (
            scheduler_frame_pointer_valid(
                next_task,
                next_task->saved_sp) == 0
        ) {
            scheduler_abort_run();
            return (uint32_t *)0;
        }

        scheduler_current_index = next_index;

        return next_task->saved_sp;
    }

    if (
        (scheduler_active == 0u) ||
        (scheduler_current_index >= SCHEDULER_TASK_COUNT) ||
        (saved_sp == (uint32_t *)0)
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    current_task =
        &scheduler_tasks[scheduler_current_index];

    if (
        scheduler_frame_pointer_valid(
            current_task,
            saved_sp) == 0
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    svc_number = scheduler_decode_svc_number(saved_sp);

    if (svc_number == SCHEDULER_SVC_YIELD)
    {
        current_task->saved_sp = saved_sp;
    }
    else if (svc_number == SCHEDULER_SVC_EXIT)
    {
        current_task->saved_sp = saved_sp;
        current_task->state = SCHEDULER_TASK_DONE;
        ++scheduler_completed_count;
    }
    else
    {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    next_index =
        scheduler_find_next_ready(
            scheduler_current_index);

    if (next_index >= SCHEDULER_TASK_COUNT)
    {
        scheduler_preempt_enabled = 0u;
        scheduler_clear_pending_pendsv();

        scheduler_current_index = SCHEDULER_NO_TASK;
        scheduler_active = 0u;

        scheduler_run_result =
            (scheduler_completed_count ==
             scheduler_target_count) ?
                1u :
                0u;

        return (uint32_t *)0;
    }

    next_task = &scheduler_tasks[next_index];

    if (
        scheduler_frame_pointer_valid(
            next_task,
            next_task->saved_sp) == 0
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    scheduler_current_index = next_index;

    return next_task->saved_sp;
}

uint32_t *scheduler_pendsv_dispatch(
    uint32_t *saved_sp)
{
    uint32_t next_index;
    scheduler_task_t *current_task;
    scheduler_task_t *next_task;

    if (
        (scheduler_active == 0u) ||
        (scheduler_preempt_enabled == 0u) ||
        (scheduler_current_index >= SCHEDULER_TASK_COUNT) ||
        (saved_sp == (uint32_t *)0)
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    current_task =
        &scheduler_tasks[scheduler_current_index];

    if (
        scheduler_frame_pointer_valid(
            current_task,
            saved_sp) == 0
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    current_task->saved_sp = saved_sp;

    next_index =
        scheduler_find_next_ready(
            scheduler_current_index);

    if (next_index >= SCHEDULER_TASK_COUNT)
    {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    next_task = &scheduler_tasks[next_index];

    if (
        scheduler_frame_pointer_valid(
            next_task,
            next_task->saved_sp) == 0
    ) {
        scheduler_abort_run();
        return (uint32_t *)0;
    }

    if (next_index != scheduler_current_index)
    {
        ++scheduler_preempt_switch_count;
    }

    scheduler_current_index = next_index;

    return next_task->saved_sp;
}

__attribute__((naked))
void PendSV_Handler(void)
{
    __asm volatile (
        "tst lr, #4\n"
        "beq 3f\n"

        "mrs r0, psp\n"
        "sub r0, r0, #32\n"
        "mov r2, r0\n"
        "stmia r2!, {r4-r11}\n"
        "msr psp, r0\n"
        "bl scheduler_pendsv_dispatch\n"
        "cmp r0, #0\n"
        "beq 1f\n"
        "ldmia r0!, {r4-r11}\n"
        "msr psp, r0\n"
        "ldr lr, =0xFFFFFFFD\n"
        "bx lr\n"

        "1:\n"
        "ldr r2, =scheduler_host_r4_r11\n"
        "ldmia r2!, {r4-r11}\n"
        "ldr lr, =0xFFFFFFF9\n"
        "bx lr\n"

        "3:\n"
        "bx lr\n"
    );
}

__attribute__((naked))
void SVC_Handler(void)
{
    __asm volatile (
        "tst lr, #4\n"
        "beq 1f\n"

        "mrs r0, psp\n"
        "sub r0, r0, #32\n"
        "mov r2, r0\n"
        "stmia r2!, {r4-r11}\n"
        "msr psp, r0\n"
        "mov r1, lr\n"
        "bl scheduler_svc_dispatch\n"
        "cmp r0, #0\n"
        "beq 2f\n"
        "ldmia r0!, {r4-r11}\n"
        "msr psp, r0\n"
        "ldr lr, =0xFFFFFFFD\n"
        "bx lr\n"

        "1:\n"
        "ldr r2, =scheduler_host_r4_r11\n"
        "stmia r2!, {r4-r11}\n"
        "movs r0, #0\n"
        "mov r1, lr\n"
        "bl scheduler_svc_dispatch\n"
        "cmp r0, #0\n"
        "beq 2f\n"
        "ldmia r0!, {r4-r11}\n"
        "msr psp, r0\n"
        "ldr lr, =0xFFFFFFFD\n"
        "bx lr\n"

        "2:\n"
        "ldr r2, =scheduler_host_r4_r11\n"
        "ldmia r2!, {r4-r11}\n"
        "ldr lr, =0xFFFFFFF9\n"
        "bx lr\n"
    );
}

static int scheduler_start_mode(
    uint32_t preemptive)
{
    uint32_t index;
    uint32_t ready_count = 0u;
    int result;

    if (scheduler_active != 0u)
    {
        return 0;
    }

    for (index = 0u;
         index < SCHEDULER_TASK_COUNT;
         ++index)
    {
        if (scheduler_tasks[index].state == SCHEDULER_TASK_READY)
        {
            if (
                scheduler_frame_pointer_valid(
                    &scheduler_tasks[index],
                    scheduler_tasks[index].saved_sp) == 0
            ) {
                return 0;
            }

            ++ready_count;
        }
    }

    if (ready_count == 0u)
    {
        return 0;
    }

    if (preemptive != 0u)
    {
        scheduler_configure_pendsv_priority();
    }

    scheduler_current_index = SCHEDULER_NO_TASK;
    scheduler_completed_count = 0u;
    scheduler_target_count = ready_count;
    scheduler_run_result = 0u;
    scheduler_preempt_enabled =
        (preemptive != 0u) ? 1u : 0u;
    scheduler_preempt_switch_count = 0u;
    scheduler_active = 1u;

    __asm volatile ("svc #0" ::: "memory");

    result =
        (scheduler_active == 0u) &&
        (scheduler_current_index == SCHEDULER_NO_TASK) &&
        (scheduler_run_result != 0u);

    scheduler_preempt_enabled = 0u;

    return result;
}

int scheduler_start(void)
{
    return scheduler_start_mode(0u);
}

void scheduler_yield(void)
{
    __asm volatile ("svc #1" ::: "memory");
}

void scheduler_tick(void)
{
    if (
        (scheduler_active != 0u) &&
        (scheduler_preempt_enabled != 0u) &&
        (scheduler_current_index < SCHEDULER_TASK_COUNT)
    ) {
        SCB_ICSR = SCB_ICSR_PENDSVSET;

        __asm volatile (
            "dsb\n"
            "isb\n"
            ::: "memory");
    }
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

    scheduler_init();

    return
        (task0->state == SCHEDULER_TASK_UNUSED) &&
        (task1->state == SCHEDULER_TASK_UNUSED) &&
        (task0->saved_sp == task0->stack_high) &&
        (task1->saved_sp == task1->stack_high);
}

int scheduler_cooperative_self_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    int start_result;
    int passed;

    scheduler_init();

    scheduler_coop_sequence[0] = 0u;
    scheduler_coop_sequence[1] = 0u;
    scheduler_coop_sequence[2] = 0u;
    scheduler_coop_sequence[3] = 0u;
    scheduler_coop_sequence_count = 0u;
    scheduler_coop_error = 0u;

    if (
        scheduler_task_prepare(
            0u,
            scheduler_coop_task0,
            (void *)&scheduler_coop_arg0) == 0
    ) {
        scheduler_init();
        return 0;
    }

    if (
        scheduler_task_prepare(
            1u,
            scheduler_coop_task1,
            (void *)&scheduler_coop_arg1) == 0
    ) {
        scheduler_init();
        return 0;
    }

    start_result = scheduler_start();

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    passed =
        (start_result != 0) &&
        (scheduler_coop_error == 0u) &&
        (scheduler_coop_sequence_count == 4u) &&
        (scheduler_coop_sequence[0] == 0x10u) &&
        (scheduler_coop_sequence[1] == 0x20u) &&
        (scheduler_coop_sequence[2] == 0x11u) &&
        (scheduler_coop_sequence[3] == 0x21u) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    scheduler_init();

    return passed;
}

int scheduler_preemptive_self_test(void)
{
    const scheduler_task_t *task0;
    const scheduler_task_t *task1;
    int start_result;
    int passed;

    scheduler_init();

    scheduler_preempt_sequence[0] = 0u;
    scheduler_preempt_sequence[1] = 0u;
    scheduler_preempt_sequence[2] = 0u;
    scheduler_preempt_sequence[3] = 0u;
    scheduler_preempt_sequence[4] = 0u;
    scheduler_preempt_sequence[5] = 0u;
    scheduler_preempt_sequence_count = 0u;
    scheduler_preempt_phase = 0u;
    scheduler_preempt_error = 0u;

    if (
        scheduler_task_prepare(
            0u,
            scheduler_preempt_task0,
            (void *)&scheduler_preempt_arg0) == 0
    ) {
        scheduler_init();
        return 0;
    }

    if (
        scheduler_task_prepare(
            1u,
            scheduler_preempt_task1,
            (void *)&scheduler_preempt_arg1) == 0
    ) {
        scheduler_init();
        return 0;
    }

    start_result = scheduler_start_mode(1u);

    task0 = scheduler_task_get(0u);
    task1 = scheduler_task_get(1u);

    passed =
        (start_result != 0) &&
        (scheduler_preempt_error == 0u) &&
        (scheduler_preempt_switch_count >= 3u) &&
        (scheduler_preempt_sequence_count == 6u) &&
        (scheduler_preempt_sequence[0] == 0x30u) &&
        (scheduler_preempt_sequence[1] == 0x40u) &&
        (scheduler_preempt_sequence[2] == 0x31u) &&
        (scheduler_preempt_sequence[3] == 0x41u) &&
        (scheduler_preempt_sequence[4] == 0x42u) &&
        (scheduler_preempt_sequence[5] == 0x32u) &&
        (task0 != (const scheduler_task_t *)0) &&
        (task1 != (const scheduler_task_t *)0) &&
        (task0->state == SCHEDULER_TASK_DONE) &&
        (task1->state == SCHEDULER_TASK_DONE);

    scheduler_init();

    return passed;
}
