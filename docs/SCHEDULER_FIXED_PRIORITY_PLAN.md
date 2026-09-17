# STM32 OS — Scheduler Fixed-Priority Plan

Status: **C3.8 IMPLEMENTED, HARDWARE ACCEPTED AND PUBLISHED `0312bb376c365c0235b9cafe22e927254528f2c4`**

Published baseline:

- commit: `90df6a690230c9800c0d8497d5597f87a5ae0409`
- tree: `c9e8cfd4bfa1dbbcc6d4d907b9c601c0d70fec59`
- subject: `feat: add scheduler timed blocking foundation`
- accepted firmware: `22900` bytes
- accepted firmware SHA-256: `366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A`

## 1. Boundary

C3.8 adds a **static fixed-priority READY-selection policy** to the accepted
scheduler.

It does not create another scheduler state, another blocking mechanism, another
clock, another production task or another exception path.

The existing task lifecycle remains:

```text
UNUSED -> READY -> BLOCKED/READY -> DONE
```

The existing timed/event model remains unchanged.

## 2. Source boundary

Authorized implementation source boundary:

- `include/kernel/scheduler.h`
- `src/kernel/scheduler.c`
- `src/kernel.c`

Guard-only:

- `include/kernel/time.h`
- `src/startup.s`
- `linker/stm32f103c8.ld`
- frozen OLED/gfx implementation

Published source identities before C3.8:

- `include/kernel/scheduler.h` = `F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547`
- `src/kernel/scheduler.c` = `90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618`
- `src/kernel.c` = `44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC`
- `include/kernel/time.h` = `A0DCB536D50796899D34B47A2DD3630A0BC203281094BA48705F324F5C491795`
- `src/startup.s` = `048C4D3604C3922F1491A6AC475609DE3593C41622648386D391F42688082E17`
- `linker/stm32f103c8.ld` = `43E3269205CA83CFBC98D7664B86DB7293CF634875BF770ED714E7D6AE52BC2A`

## 3. Priority namespace

Priority is an unsigned static task attribute.

Canonical constants:

```c
#define SCHEDULER_PRIORITY_HIGHEST 0u
#define SCHEDULER_PRIORITY_DEFAULT 128u
#define SCHEDULER_PRIORITY_LOWEST  255u
```

Policy:

```text
lower numeric value = higher scheduling priority
```

The TCB gains:

```c
uint32_t priority;
```

The field is stored as `uint32_t` deliberately:

- no packed/byte-field ABI complexity;
- natural Cortex-M3 access;
- deterministic TCB layout;
- range is still explicitly limited to `0..255`.

`scheduler_init()` initializes every slot to
`SCHEDULER_PRIORITY_DEFAULT`.

## 4. Static-priority API

Add:

```c
int scheduler_task_priority_set(
    uint32_t index,
    uint32_t priority);
```

Rules:

- invalid task index -> reject;
- priority above `SCHEDULER_PRIORITY_LOWEST` -> reject;
- active scheduler -> reject;
- otherwise assign and return success;
- setting priority is allowed before or after stack bind / task prepare while
  scheduler is inactive;
- `scheduler_task_stack_bind()` and `scheduler_task_prepare()` must preserve an
  already assigned priority.

No runtime/dynamic priority mutation is part of C3.8.

No new SVC number is added.

## 5. One READY selector

The existing round-robin-only `scheduler_find_next_ready()` becomes the single
fixed-priority READY selector.

Selection algorithm:

1. inspect every READY task;
2. select the READY task with the numerically smallest priority;
3. for equal priorities, keep the existing round-robin order beginning after
   `after_index`;
4. if no READY task exists, return `SCHEDULER_NO_TASK`.

The equal-priority rule is mandatory because it preserves the accepted
C3.7 behavior when all tasks retain `SCHEDULER_PRIORITY_DEFAULT`.

No heap, bitmap, queue, tree or sorted list is justified for two static task
slots. C3.8 deliberately keeps an O(task-count) scan.

## 6. Dispatch-path authority

Every path that chooses a READY task must use the same selector:

```text
initial SVC 0 dispatch
cooperative SVC yield/block/exit return path
host-MSP re-entry after WFE
PendSV preemptive dispatch
```

There must not be a second priority policy in PendSV or SysTick.

Priority changes only READY selection.

It must not change:

- event bit publication/consumption;
- timed deadline arbitration;
- `BLOCKED` semantics;
- task completion accounting;
- host PRIMASK atomic READY/BLOCKED/terminal classification;
- host MSP WFE ownership.

## 7. Cooperative semantics

Normal production remains cooperative.

In cooperative mode:

- a newly READY higher-priority task does **not** asynchronously interrupt the
  currently running task;
- priority takes effect at the next scheduling point:
  yield, block, exit, or host re-entry;
- if the highest-priority task calls `scheduler_yield()` while it is still
  READY and no equal/higher peer exists, it is selected again;
- lower-priority starvation is therefore possible and intentional.

C3.8 does not add fairness, aging or implicit timeslicing to cooperative mode.

## 8. Preemptive semantics

The existing command-gated preemptive path remains diagnostic/future-facing.

In preemptive mode:

- PendSV uses the same priority-aware selector;
- if a higher-priority task is READY, it wins;
- among equal highest-priority READY tasks, the existing round-robin order
  provides timeslicing;
- a lower-priority task cannot displace a READY higher-priority task;
- existing event/timed wake code may pend PendSV as it already does;
- if the woken task is lower priority, the selector may return the current
  higher-priority task and the switch count must not falsely increment.

C3.8 does not redesign the existing SysTick/PendSV trigger policy.

## 9. Production configuration

Production topology remains unchanged:

```text
slot 0 = production console/runtime
slot 1 = UNUSED
```

No diagnostic helper task is added.

During normal boot, task 0 is explicitly assigned
`SCHEDULER_PRIORITY_DEFAULT` while the scheduler is inactive.

`schedprod` adds read-only telemetry:

```text
SCHED_PROD_TASK0_PRIORITY=0x00000080
```

All existing production ownership, stack, timed blocking and RX contracts
remain unchanged.

## 10. Safe priority diagnostic

Add one production-safe command:

```text
schedprio
```

It must not reset, stop or reinitialize the live production scheduler.

The scheduler exposes a safe selector self-test:

```c
uint32_t scheduler_priority_self_test(void);
```

The self-test uses synthetic local TCB state and the same READY selector used by
real dispatch. It must not mutate the live scheduler task table.

Canonical result mask:

```text
bit 0: high priority wins when stored in slot 1
bit 1: high priority wins when stored in slot 0
bit 2: equal priority round-robin after slot 0 selects slot 1
bit 3: equal priority round-robin after slot 1 selects slot 0
bit 4: higher-priority BLOCKED task is skipped for lower-priority READY task
bit 5: sole READY higher-priority current task remains selected over lower READY
```

Expected mask:

```text
0x0000003F
```

`schedprio` also attempts a live priority mutation while the production
scheduler is active. The setter must reject it and task 0 priority must remain
unchanged.

Required markers:

```text
SCHED_PRIO_POLICY=LOWER_VALUE_HIGHER
SCHED_PRIO_HIGHEST=0x00000000
SCHED_PRIO_DEFAULT=0x00000080
SCHED_PRIO_LOWEST=0x000000FF
SCHED_PRIO_TASK0=0x00000080
SCHED_PRIO_SELFTEST=0x0000003F
SCHED_PRIO_ACTIVE_SET_REJECT_OK
SCHED_PRIO_OK
```

The safe production command surface grows from `19` to `20`.

The historical offline safe-command probe remains `17`.

The eight invasive scheduler diagnostics remain exact `SCHED_DIAG_BUSY` while
production scheduling is active.

## 11. Build proof

Fresh GNU validation must prove:

- exact source/docs identities;
- `-Wall -Wextra -Werror`;
- priority constants and TCB field;
- `scheduler_task_priority_set()` linked;
- `scheduler_priority_self_test()` linked;
- no new SVC assignment;
- the one READY selector contains priority comparison plus equal-priority
  round-robin tie behavior;
- SVC start, SVC task return path and PendSV all use that selector;
- `scheduler_tick()` timed wake semantics remain unchanged;
- PRIMASK/WFE/SEV host-idle race fix remains CFG-proven;
- production task priority is explicitly DEFAULT;
- `schedprod` and `schedprio` priority markers are linked;
- stack/RAM/BSS/Flash facts are measured, not assumed;
- complete candidate passes temporary-index readiness.

## 12. Hardware acceptance

Hardware acceptance must retain every accepted C3.7 production regression:

- exact candidate Flash/readback;
- immediate production PSP ownership;
- host MSP WFE idle;
- timed `schedtimed` 4/4 behavior;
- RX zero drop/error/final depth;
- all eight invasive diagnostics BUSY;
- retained `4 x 32` burst regression;
- production and MSP canaries/margins;
- OLED runtime restore;
- final ping;
- final exact Flash identity.

Priority-specific acceptance:

- `schedprod` reports task 0 priority `0x00000080`;
- `schedprio` policy constants exact;
- selector self-test exact `0x0000003F`;
- active priority-set rejection PASS;
- priority remains unchanged after rejection;
- scheduler remains active/cooperative on PSP afterward;
- no fatal/return marker.

Safe production surface becomes `20/20`.

## 13. Physical OLED policy

C3.8 does not authorize any OLED/gfx source change or UI geometry change.

Gate 4 physical OLED is therefore **conditionally N/A** when all of the
following hold:

- all frozen OLED/gfx source hashes remain exact C3.7 values;
- automated `uiruntime` restore PASS;
- hardware acceptance reports no display anomaly.

If any condition fails, Gate 4 becomes a required manual physical OLED check
before documentation finalization.

## 14. Gate order

C3.8 gate order:

1. **Gate 0** — planning/docs synchronization.
2. **Gate 1** — fixed-priority source implementation.
3. **Gate 2** — fresh GNU build validation.
4. **Gate 3** — hardware priority + full retained C3.7 regression.
5. **Gate 4** — physical OLED conditional N/A, otherwise manual acceptance.
6. **Gate 5** — documentation/evidence finalization.
7. **Gate 6** — local acceptance commit.
8. **Gate 7** — ordinary non-force fast-forward push.

No gate combines source edit, build, flash, commit or push.

## 15. Non-goals

Explicitly out of scope:

- dynamic priority changes while scheduler active;
- priority inheritance, donation or ceiling protocols;
- aging/fairness guarantees;
- deadline scheduling;
- EDF;
- additional production tasks;
- task creation/destruction while active;
- message queues, mutexes, semaphores;
- generic timer callbacks;
- tickless idle;
- changing event/timed wait semantics;
- HAL/Arduino/FreeRTOS;
- OLED/UI redesign.

## 16. Acceptance record — 2026-09-15

Implementation matches this design and is hardware accepted.

Candidate:

```text
build\scheduler_fixed_priority_v1\os.bin
23792 bytes
SHA-256 492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F
```

Source identities:

```text
include/kernel/scheduler.h A48DCBB90D08FAD03F2D426AA2A129A8D8858204380D4A518D7094A318F5D841
src/kernel/scheduler.c     B59B08373662B841C2CC077C92DE18D7FA21DA6DCE4E1DEE435F86AB58566C88
src/kernel.c               235813864C83E7E813A31288DBE45635AF9948213CC3352A039FC4AC31D82A8D
```

Priority runtime:

```text
policy                     LOWER_VALUE_HIGHER
range                      0..255
default                    128
selector self-test         0x0000003F
task0 priority before      128
task0 priority after       128
active mutation            rejected
cooperative preempt count  0
```

Retained regression:

```text
safe surface        20/20
timed phases        4/4
diagnostic BUSY     8/8
burst rounds        4/4
burst PONG          128/128
RX drop/error/depth 0/0/0
production stack    580 used / 444 margin
MSP                 340 used / 1644 margin
final Flash         exact candidate
OLED runtime        PASS
```

Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Evidence:

- build log `306AC5D97125F5BEFB4B2DB95E602ED8F6541E32CF364AA61ACD3D93A0E946D0`;
- build evidence `33E699282A456B79A0F15C8B2B6DF756BAD5979867091863599575A851CEA000`;
- hardware log `005D646FD613506896BC1A3961DDA752D9D424AD7F4AD0CFAAEE377C50E05222`;
- hardware evidence `41665A892477FB195D00DD722A093F69B09AB2A66669EB872DDB7A3518EACC6C`.

The implementation is accepted but is not published until Gate 6 local commit
and Gate 7 non-force push both pass.
