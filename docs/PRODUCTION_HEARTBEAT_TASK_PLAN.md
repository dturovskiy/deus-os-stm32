# C3.9 Production Heartbeat Task Ownership Plan

Status: **C3.9 HARDWARE ACCEPTED — GATE 6 LOCAL COMMIT NEXT**

Boundary ID:

`PRODUCTION_HEARTBEAT_TASK_OWNERSHIP_C3_9`

Published parent:

`0312bb376c365c0235b9cafe22e927254528f2c4` — `feat: add fixed-priority scheduler policy`

Published tree:

`4789d9f4e2712fb3c0d7c9f926cd8b853c818e32`

Published C3.8 firmware:

`23792` bytes /
`492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F`

## 1. Purpose

Activate the existing scheduler slot 1 for a real second production
responsibility without inventing shared ownership or IPC.

The responsibility is the 500 ms PC13 heartbeat currently executed by
`SysTick_Handler()`.

C3.7 already published timer-backed task sleep. Therefore the heartbeat no
longer belongs in SysTick.

## 2. Production topology

```text
slot 0
  production console/runtime
  priority 128
  1024-byte stack

slot 1
  production heartbeat
  priority 255
  512-byte stack

host
  Thread/MSP
  WFE when both production tasks are BLOCKED
```

No third task and no task-count change.

## 3. Static constants

Canonical source constants:

```c
#define PRODUCTION_HEARTBEAT_STACK_WORDS 128u
#define PRODUCTION_HEARTBEAT_STACK_BYTES \
    (PRODUCTION_HEARTBEAT_STACK_WORDS * 4u)
#define PRODUCTION_HEARTBEAT_MIN_MARGIN_BYTES 256u
#define PRODUCTION_HEARTBEAT_PERIOD_MS 500u
```

Task priorities:

```text
task0 = SCHEDULER_PRIORITY_DEFAULT = 128
task1 = SCHEDULER_PRIORITY_LOWEST  = 255
```

Lower numeric value remains higher priority.

## 4. Task1 ownership

After `scheduler_start()`, task1 is the sole writer of PC13.

Bootstrap exception:

`gpio_init()` may establish the initial active-low LED-off state before the
scheduler starts.

Task1 must not:

- write UART;
- consume the UART RX ring;
- access OLED/I2C;
- execute console commands;
- own scheduler policy.

## 5. Task1 loop

Canonical behavior:

```c
for (;;) {
    if (scheduler_sleep_ms(PRODUCTION_HEARTBEAT_PERIOD_MS) == 0) {
        production_heartbeat_fault = 1u;
        return;
    }

    production_heartbeat_led_on ^= 1u;

    if (production_heartbeat_led_on != 0u) {
        GPIOC_BSRR = GPIO_RESET_13;
    } else {
        GPIOC_BSRR = GPIO_PIN_13;
    }

    ++production_heartbeat_count;
}
```

The task sleeps before the first transition.

No busy delay is allowed.

## 6. SysTick after migration

`SysTick_Handler()` keeps only time/scheduler ownership:

```c
++kernel_ticks;
scheduler_tick(kernel_ticks);
```

SysTick stops writing PC13 entirely after scheduler start.

It must not contain:

- `led_ticks`;
- `led_on`;
- `GPIOC_BSRR` writes;
- any 500 ms heartbeat policy.

PC13 heartbeat policy becomes Thread/PSP work.

## 7. Kernel-main startup order

Normal boot remains:

1. clocks/GPIO/UART/I2C/OLED/fault setup;
2. SysTick setup;
3. boot banner and frozen OLED runtime UI;
4. `scheduler_init()`;
5. reset production telemetry;
6. bind task0 1024-byte console stack;
7. set task0 priority 128;
8. prepare task0 console;
9. bind task1 512-byte heartbeat stack;
10. set task1 priority 255;
11. prepare task1 heartbeat;
12. verify both TCBs;
13. `scheduler_start()` cooperative.

The shared priority selector naturally starts task0 first because 128 < 255.
Task1 first runs after task0 blocks.

## 8. Telemetry

No new console command is added.

`schedprod` is extended with:

```text
SCHED_PROD_TASK1_STATE=
SCHED_PROD_TASK1_PRIORITY=0x000000FF
SCHED_PROD_HEARTBEAT_STARTED=0x00000001
SCHED_PROD_HEARTBEAT_COUNT=
SCHED_PROD_HEARTBEAT_LED_ON=
SCHED_PROD_HEARTBEAT_FAULT=0x00000000
SCHED_PROD_HEARTBEAT_CAPACITY=0x00000200
SCHED_PROD_HEARTBEAT_USED=
SCHED_PROD_HEARTBEAT_MARGIN=
SCHED_PROD_HEARTBEAT_CANARY=0x00000001
```

Task1 state is valid when `READY` or `BLOCKED`. `UNUSED` and `DONE` are
production failures.

`schedprod` still requires:

- active scheduler;
- task0 READY / priority 128;
- console stack healthy;
- cooperative preempt-switch count 0;
- no production fault.

`schedprio` retains:

```text
SCHED_PRIO_SELFTEST=0x0000003F
SCHED_PRIO_ACTIVE_SET_REJECT_OK
```

and additionally reports:

```text
SCHED_PRIO_TASK1=0x000000FF
```

Both production priorities must remain unchanged after rejected active-set
attempts.

`health` remains the read-only PC13 ODR observer.

The safe production command surface remains exactly `20/20`.

## 9. Priority semantics

C3.9 does not change fixed-priority policy.

Task0 is higher priority than task1.

Because normal production is cooperative:

- task1 does not asynchronously preempt task0;
- task1 normally runs when task0 blocks;
- if both are READY at a scheduling point, task0 wins;
- task1 work is deliberately short and immediately blocks again.

No preemptive production mode is enabled.

## 10. IPC decision

C3.9 adds **no IPC**.

Reason:

```text
task0 writes UART/OLED/I2C
task1 writes PC13
```

There is no shared mutable payload requiring a queue, mutex, semaphore or
message channel.

Telemetry reads do not transfer ownership.

## 11. Scheduler-core scope

No scheduler-core source change is authorized.

Gate 1 source edit is expected to be exactly:

`src/kernel.c`

These remain byte-identical guards:

- `src/kernel/scheduler.c`;
- `include/kernel/scheduler.h`;
- `include/kernel/time.h`;
- `src/startup.s`;
- `linker/stm32f103c8.ld`.

No new SVC number is added.

## 12. Failure behavior

Task1 maintains:

```text
production_heartbeat_started
production_heartbeat_count
production_heartbeat_led_on
production_heartbeat_fault
```

A failed sleep or invalid entry cookie sets heartbeat fault and returns.

`schedprod` treats heartbeat fault, task1 `UNUSED`, task1 `DONE`, bad priority,
bad canary, or insufficient stack margin as failure.

Unexpected return of the overall scheduler remains fail-closed exactly as in
C3.8.

## 13. OLED policy

C3.9 does not change OLED/gfx source or geometry.

If OLED/gfx hashes remain exact and `uiruntime` passes in hardware acceptance,
Gate 4 is:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

## 14. Explicit non-goals

C3.9 does not add:

- IPC/message queues;
- mutexes/semaphores;
- priority inheritance;
- dynamic task priority;
- dynamic task create/destroy;
- a display task;
- shared OLED/I2C ownership;
- generic timer callbacks;
- another kernel clock;
- another scheduler state;
- a new SVC;
- preemptive normal production;
- HAL, Arduino or FreeRTOS.

## 15. Gate sequence

1. Gate 0 — canonical planning/docs synchronization.
2. Gate 1 — `src/kernel.c` implementation only.
3. Gate 2 — fresh GNU build validation.
4. Gate 3 — real hardware two-production-task acceptance.
5. Gate 4 — physical OLED conditional N/A if unchanged-source policy passes.
6. Gate 5 — docs/evidence finalization.
7. Gate 6 — local acceptance commit.
8. Gate 7 — ordinary non-force publication.


## 16. Acceptance record — 2026-09-15

Accepted candidate:

`build\production_heartbeat_task_v1\os.bin`

`24648` bytes /
`4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86`.

Hardware acceptance:

```text
task0 priority       128 -> 128
task1 priority       255 -> 255
priority self-test   0x0000003F
heartbeat count      3 -> 10
heartbeat delta      7
PC13 transitions     5
heartbeat stack      80 used / 432 margin
console stack        616 used / 408 margin
MSP                  340 used / 1644 margin
safe surface         20/20
timed blocking       4/4
diagnostic BUSY      8/8
burst regression     128/128 PONG
RX drop/error/depth  0/0/0
preempt switches     0
```

The normal-runtime PC13 ownership model is accepted. The fatal-fault PC13
blinker remains an explicit out-of-band diagnostic exception.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

No IPC, new SVC, scheduler-core modification, new scheduler state or UI source
change was required.

Gates 0–5 are accepted. Gate 6 is the local acceptance commit.
