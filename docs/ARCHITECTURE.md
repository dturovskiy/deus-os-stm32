# Architecture

Status: **published through C3.8 `0312bb376c365c0235b9cafe22e927254528f2c4`; C3.9 production heartbeat task hardware accepted, publication pending**

## 1. Boot flow

```text
Reset
  |
  v
Cortex-M3 reads initial MSP from 0x08000000
  |
  v
Cortex-M3 reads Reset_Handler from vector[1]
  |
  v
Reset_Handler
  |
  +-- initialize MSP guard / watermark
  +-- copy .data
  +-- clear .bss
  |
  v
kernel_main() on Thread/MSP
```

The next migration does not change Reset/vector/linker ownership.

## 2. Memory ownership

STM32F103C8 target:

- Flash: 64 KiB
- SRAM: 20 KiB
- initial MSP: `0x20005000`

Upper SRAM contains the accepted dedicated MSP reservation:

```text
0x20005000  _estack
    ^
    | 1984-byte measurable MSP capacity
    |
0x20004840  _emsp_guard
    | 64-byte guard/canary
0x20004800  _smsp_stack
```

Lower SRAM contains `.data`, `.bss`, scheduler/task state, framebuffer, RX ring and task stacks.

Accepted task-stack policy before normal-boot migration:

- internal scheduler pool: `2 x 512 bytes`;
- external console PSP sizing allocation: `1024 bytes`;
- production migration reuses that 1024-byte allocation for the production console task.

## 3. Published normal-boot ownership

Published C3.6 ownership is:

```text
kernel_main / Thread/MSP bootstrap
    |
    +-- clocks/GPIO/UART/I2C/OLED/faults/SysTick
    +-- frozen OLED runtime UI
    +-- scheduler init / bind / prepare
    |
    v
scheduler_start()
    |
    +-- task 0: production console/runtime on Thread/PSP
    +-- task 1: UNUSED
    +-- host Thread/MSP: WFE idle when no task is READY
```

Published commit: `33f15f1d23dfa31fabf9f2c83542a5f34046cde8`.

USART1 IRQ remains the sole DR reader. The RX ring is payload authority and the
scheduler event is notification only.

## 4. Published scheduler architecture

Scheduler facilities already accepted/published:

- two static TCB slots;
- task Thread mode on PSP;
- SVC start/yield/exit;
- PendSV save/restore of `r4-r11`;
- cooperative and command-gated preemptive modes;
- stack fill/canary/high-water instrumentation;
- lifecycle-active guard;
- explicit `UNUSED`, `READY`, `DONE`, `BLOCKED` states;
- SVC event wait;
- ISR-safe event signal;
- pending-event race closure;
- host-MSP WFE idle when incomplete tasks are blocked;
- PSP resume after event wake.

The scheduler host is a Thread/MSP control path.
Tasks execute Thread/PSP.
Exceptions/IRQs execute Handler/MSP.

## 5. UART RX ownership

USART1 RX ownership is intentionally singular:

```text
USART1 hardware
    |
    v
USART1_IRQHandler / Handler/MSP
    |
    +-- sole USART1_DR read
    +-- publish byte into 128-byte SPSC RX ring
    +-- then signal scheduler UART event
```

Thread-mode code never reads `USART1_DR` directly.

The RX ring is authoritative data.
The scheduler event is only a notification to re-check the ring.

This ordering is a permanent invariant for the next migration.

## 6. Accepted wait/wake semantics

Task-side contract:

```text
inspect/drain condition
    |
condition empty
    |
scheduler_wait_events(mask)
    |
BLOCKED
```

Producer contract:

```text
publish authoritative data/state
    |
scheduler_event_signal(mask)
```

Wake contract:

```text
task READY
    |
resume PSP
    |
re-check authoritative data/state
```

Pending-event state closes the race between consumer empty-check and SVC block.

A notification may be consumed even if the task already drained the corresponding byte.
Therefore wake may be observationally spurious and must be tolerated.

## 6.1 Hardware-discovered host-idle race and required invariant

Initial normal-boot hardware acceptance exposed a scheduler host-idle TOCTOU race under dense UART events.

The unsafe host sequence was conceptually:

```text
READY scan -> none
IRQ: BLOCKED task becomes READY
BLOCKED scan -> none
abort run
```

The second observation (`no BLOCKED`) is not sufficient evidence that the run is terminal, because the task may have become READY between the two scans.

Required invariant for the corrected scheduler:

```text
if first READY scan finds none:
    save PRIMASK and mask interrupts

    re-check READY
    if READY exists:
        restore PRIMASK
        continue

    check BLOCKED
    if BLOCKED exists:
        restore PRIMASK
        WFE / continue

    abort / terminal handling while interrupts are still masked
    restore PRIMASK
```

The entire terminal classification must be one IRQ-atomic observation. An unlocked second READY scan is insufficient because an IRQ may still move `BLOCKED -> READY` between that scan and `scheduler_abort_run()`.

This critical section does not change event semantics. When BLOCKED work exists, PRIMASK is restored before the host park point. If an event occurs between restore and `WFE`, `scheduler_event_signal()` publishes READY and executes `SEV`; the event register therefore prevents a lost wake. The RX ring remains authoritative payload and the event remains notification/wake publication.

This race was observed on candidate `609BFB2216B178C59E6BC7D0F04F4986571465A24C820E3A34E575DB768858E3` after 18 safe commands and eight diagnostic-busy checks had already passed.


## 6.2 Accepted PRIMASK-atomic host classification — 2026-09-14

The host-idle race described above is now closed and hardware accepted.

Final invariant:

```text
save PRIMASK / mask IRQs
classify READY / BLOCKED / terminal atomically

READY:
    restore PRIMASK
    enter task

BLOCKED:
    increment idle count
    restore PRIMASK
    WFE

terminal:
    scheduler_abort_run() while IRQs remain masked
    restore PRIMASK
    exit
```

The blocked path remains lost-wake safe because `scheduler_event_signal()` publishes wake state and executes `SEV`; an event arriving after PRIMASK restore and before `WFE` remains visible in the event register.

Accepted candidate: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`.

Hardware regression: four rounds of 32 unpaced pings, 128/128 responses, zero scheduler abort/fatal, zero RX drops/errors, final depth zero.

## 7. Accepted production ownership architecture

Boundary ID: `NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_MIGRATION`

Canonical design:
`docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md`

After migration:

```text
kernel_main / Thread/MSP
    |
    +-- bootstrap
    +-- initial frozen OLED UI
    +-- scheduler_init
    +-- bind/prepare production console task
    |
    v
scheduler_start() / cooperative / host MSP
    |
    +-- task 0: production console / Thread/PSP / 1024-byte stack
    |
    +-- task 1: UNUSED
    |
    +-- no READY task -> host MSP WFE
```

Production console task:

```text
console_drain_rx()
    |
scheduler_wait_events(UART_RX_EVENT)
    |
wake
    |
console_drain_rx()
```

After scheduler start, MSP no longer owns the application console loop.

## 8. OLED / I2C ownership

Frozen hardware/UI baseline:

- native 128x32 SSD1306-compatible panel;
- I2C address `0x3C`;
- accepted product presentation `DEUS OS / BOOT OK / READY`.

Next migration uses phase ownership:

- MSP bootstrap owns initial UI initialization/composition;
- production console PSP task owns runtime application-level OLED/I2C command calls;
- IRQ handlers do not render or execute OLED/I2C transactions.

A dedicated display task is deferred until there is a real independent UI workload and accepted IPC ownership model.

## 9. SysTick

SysTick remains responsible for:

- 1 kHz kernel tick;
- scheduler tick hook;
- 500 ms PC13 heartbeat.

In the first production migration the scheduler runs cooperatively, so `scheduler_tick()` does not request production PendSV preemption.

The heartbeat stays in SysTick until timer-backed task sleep exists.

## 10. Diagnostic lifecycle isolation

Invasive scheduler diagnostics must not reset a live production scheduler.

While production scheduler is active, all eight console-visible invasive scheduler commands return:

`SCHED_DIAG_BUSY`

This is expected production behavior, not a regression.

Read-only production telemetry may be exposed separately through `schedprod`.

## 11. Failure model

The production console task is persistent.

Unexpected scheduler return must not restore the historical MSP console loop.

Required architecture:

```text
unexpected scheduler return
    -> explicit fatal marker if transport permits
    -> fail-closed parked/panic state
```

This preserves a single owner for normal application execution.

## 12. Layering

```text
application/control services
           |
production tasks / future IPC
           |
scheduler / blocking / task management
           |
SVC / PendSV / SysTick / exceptions
           |
device drivers
   |       |       |
 UART     I2C     GPIO
   |
 RX IRQ/ring
           |
register-level STM32 hardware
           |
startup / linker / vector table
```

Drivers do not own scheduler policy.
IRQ code publishes hardware state/events; tasks own policy/work.

## 13. Deferred architecture

After production ownership migration:

1. timer-backed `sleep()` / timed waits using the accepted event/blocking model;
2. scheduler priorities;
3. additional production tasks only when an independent responsibility justifies them;
4. message queues/synchronization when shared ownership actually exists;
5. native USB CDC and later transport-neutral shell/RPC;
6. networking later.

Do not create parallel blocking, idle, or ownership mechanisms.


## C3.7 accepted timed-blocking architecture

C3.7 extends the existing BLOCKED state; it does not add a sleeping state.

```text
BLOCKED
 |
 +-- wait_events != 0, deadline inactive
 |      untimed event wait
 |
 +-- wait_events != 0, deadline active
 |      timed event wait
 |
 +-- wait_events == 0, deadline active
        sleep
```

Deadline metadata:

```text
deadline_ms
deadline_active
```

Time authority remains the existing 1 ms `kernel_ticks`. `SysTick_Handler()`
increments `kernel_ticks` and passes the resulting time to
`scheduler_tick(now_ms)`. Scheduler-local `scheduler_now_ms` is only a latch of
that supplied value; it is neither independently incremented nor reset by
`scheduler_init()`.

Wrap-safe expiry:

```c
(int32_t)(now - deadline) >= 0
```

Maximum accepted timeout horizon is `0x7FFFFFFF ms`.

SVC ownership:

```text
0 start
1 yield
2 exit
3 untimed event wait
4 timed block
```

Wake arbitration is state-based and atomic:

```text
event wins:
    clear deadline
    BLOCKED -> READY
    return event mask

timeout wins:
    clear event wait
    BLOCKED -> READY
    return timeout result
```

The second contender observes a non-BLOCKED task and cannot wake it again.
Hardware `schedtimed` acceptance proved no duplicate timed result.

Host idle remains MSP `WFE`. Timeout wake publishes READY before `SEV`. The
PRIMASK-atomic READY/BLOCKED/terminal host classification accepted in C3.6 is
retained.

Normal production console waiting remains untimed and drain-first. UART ring
data remains payload authority; scheduler event remains notification only.

Accepted hardware candidate:

```text
22900 bytes
366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A
```

Acceptance:

- timed phases `4/4`;
- 50 ms sleep;
- 50 ms timeout;
- external UART event at 164 ms with mask 1;
- injected `ping` survives in the ring and yields exact `PONG`;
- `19/19` safe surface;
- `8/8` invasive diagnostics BUSY;
- retained `4 x 32`, `128/128 PONG`;
- RX `0/0/0`;
- production `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact;
- automated + physical OLED PASS.

Priorities, generic timer callbacks, tickless idle, timer task and additional
production tasks remain outside C3.7.

## C3.8 accepted fixed-priority architecture

C3.8 changes READY selection only.

```text
READY set
   |
   +-- choose numerically smallest priority
   |
   +-- equal priority: round-robin order after previous task
```

Priority namespace:

```text
0   highest
128 default
255 lowest
```

Priority is a static TCB attribute. `scheduler_task_priority_set()` rejects
mutation while the scheduler is active.

The same selector remains authoritative for:

- initial SVC0 task entry;
- cooperative SVC scheduling;
- host-MSP re-entry;
- PendSV dispatch.

Cooperative mode remains cooperative. Priority affects the next scheduling
point; it does not create an asynchronous preemption point.

Equal priorities preserve round-robin tie behavior. The existing diagnostic
preemptive path also uses the same selector.

Priority does not alter event delivery, deadline expiry, BLOCKED semantics,
WFE idle, PRIMASK classification or task completion.

Production topology remains slot0 console/runtime at priority 128 and slot1
UNUSED.

Accepted hardware candidate:

```text
23792 bytes
492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F
```

Acceptance:

- `schedprio` runtime phase `1/1`;
- selector self-test `0x0000003F`;
- active priority mutation rejected;
- task0 priority unchanged `128 -> 128`;
- cooperative preempt switches `0`;
- safe surface `20/20`;
- timed blocking `4/4`;
- `8/8` invasive diagnostics BUSY;
- retained `4 x 32`, `128/128 PONG`;
- RX `0/0/0`;
- production `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact.

OLED/gfx implementation is unchanged from C3.7 and automated runtime restore
passed, therefore Gate 4 is recorded as:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Dynamic priority mutation, inheritance, aging, deadline scheduling and
additional production tasks remain outside C3.8.


## C3.9 planned production heartbeat task ownership

C3.9 activates the existing second scheduler slot for a concrete independent
responsibility: the Blue Pill PC13 heartbeat.

```text
                    +-------------------------------+
USART1 IRQ/ring --->| task0 console/runtime         |
                    | priority 128 / PSP / 1024 B   |
                    +-------------------------------+
                                   |
                                   | blocks on UART event
                                   v

SysTick --------> scheduler tick / deadlines
                   |
                   +------ wakes task1 every 500 ms
                                   |
                                   v
                    +-------------------------------+
                    | task1 PC13 heartbeat          |
                    | priority 255 / PSP / 512 B    |
                    +-------------------------------+
                                   |
                                   +--> GPIOC_BSRR

both BLOCKED -> host Thread/MSP WFE
```

Ownership rules:

- task0 remains sole runtime owner of UART TX, console policy, OLED and I2C;
- USART1 IRQ remains sole `USART1_DR` reader and RX-ring producer;
- task1 is sole post-scheduler writer of PC13;
- SysTick stops writing PC13 entirely;
- bootstrap `gpio_init()` may set the initial LED-off state before scheduler
  start.

Task1 timing:

```text
scheduler_sleep_ms(500)
toggle PC13
increment heartbeat count
repeat
```

Priority:

- task0 = `SCHEDULER_PRIORITY_DEFAULT` = `128`;
- task1 = `SCHEDULER_PRIORITY_LOWEST` = `255`.

This preserves console precedence if both tasks become READY at the same
scheduling point, while normal production remains cooperative.

C3.9 does not need IPC. The two production tasks have disjoint write ownership
and share only scheduler/time infrastructure plus read-only telemetry.

The safe command surface remains unchanged. Existing `schedprod` is extended
with task1/heartbeat stack and lifecycle telemetry. Existing `schedprio`
retains its selector self-test and verifies both production priorities without
changing them while active. Existing `health` reads PC13 ODR and is used by the
hardware harness to correlate GPIO state with heartbeat progress.

No new scheduler state, SVC, queue, semaphore, mutex, timer callback, OLED
worker task, preemptive production mode, HAL, Arduino or FreeRTOS is introduced.


## C3.9 acceptance record — 2026-09-15

Accepted firmware:

`24648` bytes /
`4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86`.

The planned two-task ownership model is validated on real hardware:

- task0 remains the cooperative console/runtime task at priority `128`;
- task1 is the persistent PC13 heartbeat task at priority `255`;
- task1 uses `scheduler_sleep_ms(500)` and remains READY/BLOCKED only;
- SysTick contains time/scheduler work, not normal heartbeat GPIO policy;
- heartbeat task stack high-water is `80 B`, margin `432 B`;
- console task high-water is `616 B`, margin `408 B`;
- host Thread/MSP WFE idle remains healthy;
- PC13 hardware sampling observed `5` transitions and both ODR states;
- heartbeat telemetry progressed `3 -> 10` in the dedicated phase;
- priority selector self-test is `0x0000003F`;
- active priority mutation remains rejected for both tasks;
- cooperative preempt-switch count remains `0`;
- safe surface `20/20`, timed blocking `4/4`, invasive BUSY `8/8`;
- retained UART race regression `128/128 PONG`, RX drop/error/depth `0/0/0`;
- MSP margin `1644 B`;
- final Flash readback matches the exact accepted candidate.

No IPC, new SVC, scheduler state, alternate clock, OLED task, HAL, Arduino or
FreeRTOS was introduced.

OLED Gate 4 is:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

The next repository mutation after Gate 5 is one local C3.9 acceptance commit.
