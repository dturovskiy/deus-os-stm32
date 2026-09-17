# STM32 OS — Scheduler Timed Blocking Plan

Status: **C3.7 IMPLEMENTED, HARDWARE ACCEPTED AND PUBLISHED `90df6a690230c9800c0d8497d5597f87a5ae0409`**

Published baseline:

- commit: `33f15f1d23dfa31fabf9f2c83542a5f34046cde8`
- tree: `a5447420a58b0aed7cd541069979fa665df40aa9`
- subject: `feat: migrate normal boot to production scheduler ownership`
- accepted firmware: `21832` bytes
- accepted firmware SHA-256: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`

## 1. Boundary

C3.7 adds **SysTick-backed timed blocking** on top of the already accepted
READY/BLOCKED/event scheduler model.

The slice delivers two production-facing primitives:

```c
int scheduler_sleep_ms(uint32_t duration_ms);

uint32_t scheduler_wait_events_timeout(
    uint32_t events,
    uint32_t timeout_ms);
```

The existing untimed primitive remains valid and keeps its current semantics:

```c
uint32_t scheduler_wait_events(uint32_t events);
```

This slice does **not** add priorities, another production task, a timer task,
an idle task, queues, mutexes, semaphores, dynamic allocation, a new hardware
timer, or any OLED/UI redesign.

## 2. Source boundary

Authorized implementation source boundary:

- `include/kernel/scheduler.h`
- `src/kernel/scheduler.c`
- `src/kernel.c`

Guard-only files:

- `include/kernel/time.h`
- `src/startup.s`
- `linker/stm32f103c8.ld`
- frozen OLED/gfx implementation

The accepted C3.6 source identities before this slice are:

- `src/kernel.c` = `FE046521F7A017B3DE204E984ECC392CA471C82824530B00EFCBFDFA433658F1`
- `src/kernel/scheduler.c` = `FA649EF24569AEE653A1CA022778237F76FF236A3F0B1B38FDDA8FB06F867649`
- `include/kernel/scheduler.h` = `C1163CFDF2B075EB4AAF56AEA690A09D86C5AEE95BB993303799BEAE31AF265A`
- `include/kernel/time.h` = `A0DCB536D50796899D34B47A2DD3630A0BC203281094BA48705F324F5C491795`
- `src/startup.s` = `048C4D3604C3922F1491A6AC475609DE3593C41622648386D391F42688082E17`
- `linker/stm32f103c8.ld` = `43E3269205CA83CFBC98D7664B86DB7293CF634875BF770ED714E7D6AE52BC2A`

No additional source file is required for the first timed-blocking foundation.

## 3. One state model

Timed blocking must **reuse `SCHEDULER_TASK_BLOCKED`**.

Do not introduce a separate `SLEEPING` task state.

Each task gains deadline metadata sufficient to distinguish:

1. untimed event wait;
2. timed event wait;
3. pure sleep.

Canonical TCB semantics:

```text
state == BLOCKED
    wait_events == 0
        deadline_active == 1
            => pure sleep

    wait_events != 0
        deadline_active == 0
            => untimed event wait

    wait_events != 0
        deadline_active == 1
            => timed event wait
```

Required deadline fields:

```text
deadline_ms
deadline_active
```

The existing fields retain their meaning:

```text
wait_events
wake_events
```

A deadline of numeric value zero is valid after wraparound, therefore
`deadline_ms == 0` must never be used as the inactive sentinel.

## 4. Time authority

The authoritative clock remains the existing 1 ms `kernel_ticks` timebase.

Existing wrap-safe kernel time semantics remain the reference:

```c
(int32_t)(now - deadline) >= 0
```

The scheduler must **not independently increment another wall-clock counter**.

The planned scheduler tick interface is:

```c
void scheduler_tick(uint32_t now_ms);
```

`SysTick_Handler()` keeps this order:

```text
++kernel_ticks
scheduler_tick(kernel_ticks)
heartbeat maintenance
```

`scheduler_tick(now_ms)` may retain a scheduler-local **latched snapshot** of
the supplied `now_ms` so SVC blocking code can form an absolute deadline.
That snapshot is not a second clock authority:

- it is assigned only from the `now_ms` argument;
- it is never independently incremented;
- `scheduler_init()` must not reset it.

## 5. Timeout horizon and wraparound

Absolute deadline comparison uses signed modulo subtraction. Therefore the
maximum accepted timeout horizon is:

```text
0x7FFFFFFF ms
```

Any API input above that horizon must be rejected without creating a BLOCKED
state.

Required edge semantics:

- `scheduler_sleep_ms(0)` returns immediately and does not block or yield;
- `scheduler_wait_events_timeout(events, 0)` is a non-blocking event poll:
  consume a matching already-pending event if present, otherwise return `0`;
- event mask `0` is invalid for `scheduler_wait_events_timeout`;
- pure sleep uses the same internal timed-block machinery with `wait_events=0`;
- deadlines must remain correct across the 32-bit millisecond counter wrap.

## 6. SVC model

Keep existing SVC assignments unchanged:

```text
SVC 0 = start
SVC 1 = yield
SVC 2 = exit
SVC 3 = untimed event wait
```

Add exactly one timed-block SVC:

```text
SVC 4 = timed block
```

Canonical SVC 4 inputs:

```text
r0 = requested event mask
     0 => pure sleep
     nonzero => timed event wait

r1 = timeout duration in ms
```

No startup/vector change is required because the existing generic SVC handler
already dispatches through `scheduler_svc_dispatch()`.

Untimed SVC 3 and timed SVC 4 must share common internal block/wake logic rather
than becoming two independent scheduler state machines.

## 7. Atomic block and wake rules

All transitions involving event metadata, deadline metadata and task state must
remain serialized under the existing scheduler IRQ critical-section policy.

### Event already pending at timed-wait entry

For `scheduler_wait_events_timeout(events, timeout)`:

1. enter the scheduler critical section;
2. check already-pending events first;
3. if an event matches, consume it and return the event mask immediately;
4. only if there is no match may timeout handling create a timed BLOCKED state.

This preserves the existing pending-event contract.

### Timed block

When blocking:

```text
saved_sp          = current SVC frame
wait_events       = requested events (or 0 for sleep)
wake_events       = 0
deadline_ms       = latched_now + timeout
deadline_active   = 1
state             = BLOCKED
```

### Event wake

`scheduler_event_signal()` retains ring/event semantics and, when waking a
timed event waiter, must atomically:

```text
clear wait_events
clear deadline_active
clear/normalize deadline metadata
store wake_events
store SVC return value = matched event mask
state = READY
```

### Timeout wake

`scheduler_tick(now_ms)` scans the static task table. For every BLOCKED task
with an active reached deadline it must atomically:

```text
clear deadline_active
clear/normalize deadline metadata
clear wait_events
state = READY
```

SVC return value:

```text
timed event wait timeout => 0
pure sleep expiry        => success value for scheduler_sleep_ms()
```

A timeout wake must execute `SEV` after publishing READY state so the existing
host-MSP WFE contract remains explicit and race-safe.

## 8. Event-versus-timeout arbitration

There is no separate arbitration flag.

The first atomic transition that observes the task still `BLOCKED` wins:

```text
event ISR first:
    BLOCKED -> READY by event
    deadline is cancelled
    later SysTick sees no active timed block

SysTick first:
    BLOCKED -> READY by timeout
    event wait is cleared
    later event signal does not wake the task again
```

This is the required **no-double-wake** invariant.

If an event arrives after timeout has won, the existing pending-event policy
applies. The event is not silently converted into the timed-wait result.

Event remains notification, not payload identity. UART RX ring data remains the
authoritative payload.

## 9. Cooperative and preemptive behavior

Cooperative mode:

- timeout may make another task READY;
- it does not preempt the currently running cooperative task;
- when no task is running, host MSP returns from SysTick, observes READY work and
  enters it through the existing host/SVC path.

Preemptive diagnostic mode:

- the existing tick-driven PendSV behavior is preserved;
- a timed wake may participate in the already existing preemptive handoff;
- this slice does not alter scheduling priority policy.

## 10. Host idle

No idle task is added.

When all incomplete tasks are BLOCKED, host Thread/MSP continues to park with
`WFE`.

SysTick remains enabled at 1 kHz and services deadline expiry. The existing
PRIMASK-atomic READY/BLOCKED/terminal classification remains authoritative and
must not be weakened.

## 11. Production integration / safe diagnostic

Normal production console behavior stays event-driven and untimed:

```c
for (;;) {
    console_drain_rx();
    (void)scheduler_wait_events(PRODUCTION_UART_RX_EVENT);
}
```

Do **not** convert the normal console wait into a periodic timeout poll.

Add one production-safe diagnostic command:

```text
schedtimed
```

It exercises timed blocking from the real production PSP task without resetting
or replacing the production scheduler.

Canonical phases:

1. zero-duration sleep no-block check;
2. finite `scheduler_sleep_ms()` check;
3. finite timed UART-event wait that expires with return `0`;
4. emit `SCHED_TIMED_EVENT_ARMED`;
5. finite timed UART-event wait where the host injects `ping\r\n` before the
   deadline;
6. verify event mask wake;
7. return to console drain and process the injected ring payload as normal
   `PONG`.

The command is safe because it only blocks the current production task through
the production scheduler API. It does not call `scheduler_init()`, replace task
slots, start another scheduler run, or enter preemptive diagnostics.

The safe production command surface therefore grows from `18` to `19`.

The historical offline safe-command probe list may remain `17` unless a
separate reason requires changing it.

## 12. Telemetry / validation requirements

The timed diagnostic must emit enough deterministic markers to validate:

```text
SCHED_TIMED_ZERO_OK
SCHED_TIMED_SLEEP_ELAPSED=<hex>
SCHED_TIMED_SLEEP_OK
SCHED_TIMED_TIMEOUT_ELAPSED=<hex>
SCHED_TIMED_TIMEOUT_OK
SCHED_TIMED_EVENT_ARMED
SCHED_TIMED_EVENT_EVENTS=0x00000001
SCHED_TIMED_EVENT_ELAPSED=<hex>
SCHED_TIMED_EVENT_OK
SCHED_TIMED_OK
```

The harness injects `ping\r\n` only after observing
`SCHED_TIMED_EVENT_ARMED`.

Acceptance bounds:

- sleep elapsed in kernel ticks >= requested duration;
- timeout elapsed in kernel ticks >= requested duration;
- externally injected event wake occurs before its configured deadline;
- injected `ping` still produces exact `PONG`;
- no `SCHED_PROD_RETURN`, `SCHED_PROD_UNEXPECTED_RETURN`, or
  `SCHED_PROD_FATAL`.

After the diagnostic:

- production scheduler remains active;
- task 0 remains production PSP owner;
- task 1 remains UNUSED;
- PSP canary remains intact;
- PSP free margin remains >=256 bytes;
- RX drops/errors/final depth remain zero;
- host MSP WFE idle remains active;
- fault telemetry remains zero.

## 13. Build validation

Fresh GNU build is mandatory after source implementation.

Build gate must prove:

- `-Wall -Wextra -Werror`;
- fresh isolated build directory;
- exact source identities;
- new timed APIs linked;
- generic SVC path still linked;
- SVC 4 literal/dispatch present;
- deadline comparison uses wrap-safe signed-delta semantics;
- `scheduler_tick` consumes supplied `now_ms`;
- no independent increment of the scheduler time snapshot;
- SysTick passes current `kernel_ticks`;
- WFE/SEV/PRIMASK machinery remains linked;
- Reset/SVC/PendSV/SysTick/USART1 vectors remain correct;
- BSS/RAM/stack deltas are captured, not assumed;
- candidate fits 64 KiB Flash and 20 KiB SRAM;
- full candidate `git diff --check` and temporary-index readiness PASS.

## 14. Hardware acceptance

Hardware gate must retain all accepted C3.6 regressions:

- exact candidate flash/readback;
- boot-adjacent RX;
- production cooperative PSP ownership;
- host-MSP WFE idle;
- safe surface;
- all eight invasive diagnostics exact `SCHED_DIAG_BUSY`;
- stack/MSP canaries and margins;
- RX zero drop/error/final depth;
- atomic 4 x 32 burst regression;
- OLED runtime restore;
- final exact Flash readback.

Add timed-blocking acceptance:

- `schedtimed` zero-duration phase PASS;
- sleep phase PASS;
- timeout-wins phase PASS;
- event-wins phase PASS with deliberate host injection after ARMED;
- injected `ping` consumed from the RX ring and yields `PONG`;
- scheduler stays active afterward;
- no double wake / duplicate timed result;
- no fatal/return marker.

## 15. Gate order

C3.7 gate order is fixed:

1. **Gate 0** — planning/docs synchronization.
2. **Gate 1** — timed-blocking source implementation.
3. **Gate 2** — fresh GNU build validation.
4. **Gate 3** — real hardware timed-blocking + full scheduler regression.
5. **Gate 4** — manual physical frozen OLED acceptance.
6. **Gate 5** — final documentation/evidence synchronization.
7. **Gate 6** — local acceptance commit.
8. **Gate 7** — non-force fast-forward push.

No gate may combine build, flash, commit and push.

## 16. Non-goals

Explicitly out of scope:

- scheduler priorities;
- more than one production task;
- generic timer callback service;
- timer wheel / heap / sorted timer queue;
- tickless idle;
- dedicated timer task;
- OLED task;
- message queues;
- mutexes/semaphores;
- watchdog policy changes;
- USB/network/ESP work;
- HAL/Arduino/FreeRTOS;
- UI redesign.

With only two static task slots, an O(task-count) deadline scan on each 1 ms tick
is the deliberate first implementation. More complex timer data structures are
not justified in C3.7.

## 17. Acceptance record — 2026-09-15

Implementation matches this design and is hardware accepted.

Candidate:

```text
build\scheduler_timed_blocking_v1\os.bin
22900 bytes
SHA-256 366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A
```

Source identities:

```text
include/kernel/scheduler.h F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547
src/kernel/scheduler.c     90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618
src/kernel.c               44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC
```

Observed timed behavior:

```text
zero-duration sleep             PASS
50 ms sleep                     elapsed 50
50 ms event timeout             elapsed 50, result 0
external UART event wake        elapsed 164, event 0x00000001
timed RX byte/IRQ delta         26/26
host-MSP idle delta             2904
no double wake                  PASS
ring payload -> injected PONG   PASS
```

Retained production/scheduler regression:

```text
safe surface        19/19
diagnostic BUSY     8/8
burst rounds        4/4
burst PONG          128/128
RX drop/error/depth 0/0/0
production stack    580 used / 444 margin
MSP                 340 used / 1644 margin
final Flash         exact candidate
OLED runtime        PASS
physical OLED       PASS
```

Evidence:

- build log `6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84`;
- build evidence `431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199`;
- hardware log `3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769`;
- hardware evidence `ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4`;
- physical visual token `OLED PASS`.

The implementation is accepted but not published until Gate 6 local commit and
Gate 7 non-force push both pass.
