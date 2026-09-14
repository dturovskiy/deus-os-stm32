# Architecture

Status: **current architecture through published wait/wake baseline `bfed76e0de1c52bf65e60f66c029cc41710fabd0`**

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

## 3. Published baseline vs accepted normal boot

Current published runtime ownership is still:

```text
kernel_main / Thread/MSP
    |
    +-- initialize clocks/GPIO/UART/I2C/OLED/faults/SysTick
    +-- boot banner
    +-- frozen OLED runtime UI
    |
    +-- forever:
          console_poll()
          WFI
```

This is the exact ownership model being replaced by the next slice.

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
