# Normal-Boot Production Task Ownership Migration Plan

Status: **CANONICAL PLANNING BASELINE — IMPLEMENTATION NOT STARTED**

Published baseline:

- commit: `bfed76e0de1c52bf65e60f66c029cc41710fabd0`
- subject: `feat: add scheduler steady-state wait/wake foundation`
- worktree/index at publication: clean
- accepted wait/wake candidate: `build\scheduler_wait_wake_foundation_v2\os.bin`
- candidate: `19932` bytes
- SHA-256: `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`

This document defines the architecture and source boundary for the next scheduler slice:

**NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_MIGRATION**

It must be read together with:

- `docs/MASTER_EXECUTION_CHECKLIST.md`
- `docs/IMPLEMENTATION_PLAN.md`
- `docs/PROJECT_HANDOFF.md`
- `docs/ARCHITECTURE.md`
- `docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`

## 1. Compatibility verdict

The migration described here is consistent with the currently published architecture and plans.

The accepted wait/wake foundation explicitly exists to make this migration possible:

- `SCHEDULER_TASK_BLOCKED` is explicit;
- task wait uses `scheduler_wait_events()` / SVC #3;
- ISR wake uses `scheduler_event_signal()`;
- pending events close the wait-vs-signal race;
- host MSP owns no-runnable idle and parks with `WFE`;
- UART RX publishes into the ring before scheduler notification;
- event consumers re-check the FIFO/condition after wake.

Existing planning already requires the next slice to:

- migrate from the MSP-owned `console_poll(); WFI` normal boot;
- preserve USART1 IRQ/ring ownership;
- preserve frozen OLED behavior;
- preserve lifecycle diagnostic isolation;
- keep `sleep()`/timers and priorities out of this slice.

Therefore the chosen design is a refinement of the current plan, not a competing architecture.

## 2. Chosen production topology

The first production migration uses **one long-lived production console/runtime task**.

```text
Reset_Handler
    |
    v
kernel_main() on Thread/MSP
    |
    +-- clocks / GPIO / UART / I2C
    +-- framebuffer + OLED console init
    +-- faults + SysTick
    +-- UART boot banner
    +-- frozen OLED runtime UI
    +-- scheduler_init()
    +-- bind task 0 to 1024-byte production console stack
    +-- prepare task 0 = production_console_task
    +-- leave task 1 UNUSED
    |
    v
scheduler_start()             cooperative mode
    |
    +---------------- production lifetime ----------------+
    |                                                     |
    | task 0 / Thread/PSP                                 |
    |   console_drain_rx()                                |
    |   scheduler_wait_events(UART_RX_EVENT)              |
    |                                                     |
    | no READY task                                       |
    |   scheduler host / Thread/MSP -> WFE                |
    |                                                     |
    | USART1 IRQ / Handler/MSP                            |
    |   read DR -> publish ring byte -> signal event      |
    |                                                     |
    +-----------------------------------------------------+
```

The first migration deliberately does **not** create:

- a dummy idle task;
- a separate OLED task;
- a UART TX task;
- a heartbeat task;
- a timer service task;
- a priority model.

## 3. Ownership matrix

| Resource / responsibility | Boot phase | Production phase | IRQ/exception phase |
| --- | --- | --- | --- |
| clock/GPIO/UART/I2C init | MSP bootstrap | no ownership change | n/a |
| frozen initial OLED composition | MSP bootstrap | no repeated boot composition | never from IRQ |
| scheduler host / no-ready idle | not active | MSP, `WFE` | n/a |
| exception stack | MSP | MSP | MSP |
| USART1 `DR` read | none in thread | none in thread | **USART1 IRQ only** |
| RX ring producer | none in thread | none in thread | USART1 IRQ |
| RX ring consumer | boot has no console loop | production console PSP task | never |
| UART RX event publication | inactive calls ignored | task can wait | IRQ signals after ring publish |
| console parser / command dispatch | not running yet | **production console PSP task** | never |
| runtime OLED/I2C application calls | initial UI on MSP | console PSP task | never |
| PC13 500 ms heartbeat | SysTick | SysTick | SysTick IRQ |
| invasive scheduler diagnostics | scheduler inactive before production start | blocked with `SCHED_DIAG_BUSY` | n/a |

The MSP/application boundary is phase-based:

- before scheduler start, MSP is bootstrap owner;
- after scheduler start, MSP is kernel host/idle + exception stack;
- normal application command processing no longer runs on MSP.

## 4. Production console task contract

The task must use the accepted event model, not polling.

Conceptual loop:

```c
for (;;)
{
    console_drain_rx();
    (void)scheduler_wait_events(PRODUCTION_UART_RX_EVENT);
}
```

The order is mandatory: **drain first, wait second**.

Why:

1. `scheduler_event_signal()` intentionally ignores signals while scheduler is inactive.
2. A byte can arrive during boot before the scheduler starts.
3. That byte remains authoritative data in the RX ring.
4. The first production task iteration must therefore drain the ring before waiting.

After scheduler activation, a byte can also race between the empty-ring observation and SVC wait.
The accepted pending-event mechanism closes that race.

A wake may be spurious from the consumer's point of view:

- an event may be latched while the task is already READY/running;
- the task may drain the data before it calls wait;
- the subsequent wait may consume the already-pending event immediately.

Therefore wake means **re-check the ring**, not “one byte is guaranteed to exist”.

## 5. Scheduler mode

The first production migration uses:

**cooperative scheduler mode via `scheduler_start()`**

Rationale:

- there is only one production task in this slice;
- the task voluntarily blocks on RX events;
- enabling periodic preemption adds no scheduling value with one runnable task;
- keeping preemption disabled isolates the ownership migration from priority/timeslice policy;
- SysTick remains active for kernel time and PC13 heartbeat, but `scheduler_tick()` does not request PendSV while production preemption is disabled.

Preemptive scheduler functionality remains accepted and available for diagnostics/future topology work.
It is not removed.

## 6. Task slots and stack ownership

Task slot policy for this slice:

- slot 0: production console/runtime task;
- slot 1: `UNUSED`.

No idle task is created because host MSP already owns the no-ready state.

The existing accepted `1024-byte` console PSP allocation is reused as the production console stack.
The storage should be renamed from probe-specific terminology to production-neutral terminology in `src/kernel.c`.

Expected policy:

```text
production console stack capacity: 1024 bytes
historical accepted high-water:    600 bytes
historical accepted min margin:    424 bytes
migration acceptance floor:        >= 256 bytes free
```

The historical 600-byte measurement is sizing evidence, not a substitute for migration measurement.
The production task must be re-measured under the complete migrated runtime regression.

The legacy internal scheduler pool remains `2 x 512 bytes`.
Slot 1 remains on its default internal stack but `UNUSED`.

## 7. Console surface

The existing 17 non-scheduler safe commands remain production-safe.

The migration may add one new **read-only** command:

`schedprod`

Purpose: expose production scheduler ownership telemetry without reinitializing or perturbing scheduler state.

Planned fields:

```text
SCHED_PROD_ACTIVE=0x00000001
SCHED_PROD_MODE=COOPERATIVE
SCHED_PROD_THREAD_PSP=0x00000001
SCHED_PROD_PSP_IN_RANGE=0x00000001
SCHED_PROD_TASK0_STATE=...
SCHED_PROD_TASK1_STATE=...
SCHED_PROD_CONSOLE_CAPACITY=0x00000400
SCHED_PROD_CONSOLE_USED=...
SCHED_PROD_CONSOLE_MARGIN=...
SCHED_PROD_CONSOLE_CANARY=0x00000001
SCHED_PROD_IDLE_WAITS=...
SCHED_PROD_OK
```

If `schedprod` is added, the production-safe command surface becomes **18 commands**.
The old 17-command probe evidence remains historical sizing evidence; the migration hardware gate must prove the full 18-command production surface.

`schedprod` must not call `scheduler_init()`, prepare tasks, reset telemetry, or otherwise mutate lifecycle state.

## 8. Scheduler diagnostic behavior after migration

There are currently eight console-visible invasive scheduler commands:

1. `schedtest`
2. `schedcoop`
3. `schedpreempt`
4. `schedstack`
5. `schedworkload`
6. `schedconsoleprobe`
7. `schedwaitwake`
8. `schedisolate`

After normal boot starts the production scheduler, all eight must return exact:

`SCHED_DIAG_BUSY`

and leave the production scheduler/console responsive.

Do not confuse this with the historical internal `schedisolate` count:
`schedisolate` itself starts the active test and checks seven *other* diagnostics.
That historical nested count remains seven.

The normal production console no longer provides an inactive scheduler window.
If invasive offline diagnostics are needed for a future source gate, use a dedicated diagnostic acceptance candidate/workflow rather than resetting the live production scheduler.

## 9. OLED / I2C ownership

Do not create an OLED task in this slice.

Phase ownership is sufficient:

- MSP bootstrap initializes framebuffer/OLED state and shows the frozen initial runtime UI;
- after scheduler start, the production console PSP task is the only thread-mode application caller of OLED/I2C command paths;
- IRQ handlers never render or perform I2C transactions.

Reasons to defer a dedicated OLED task:

- no autonomous periodic UI workload exists yet;
- no message queue/IPC ownership protocol is accepted yet;
- introducing a second display producer would require synchronization or message passing;
- the current 17-command console PSP probe already proved the safe OLED command surface can execute on PSP.

A dedicated display task should be introduced only when an independent UI responsibility actually exists.

## 10. SysTick / heartbeat ownership

Keep the existing 500 ms PC13 heartbeat in `SysTick_Handler()` for this slice.

Moving it to a task would require an accepted timed blocking primitive.
Using polling or busy-wait in a task would violate the purpose of the accepted wait/wake model.

Timer-backed `sleep()` is explicitly the next later scheduler service after production ownership migration.

## 11. UART TX

Keep current synchronous/polling UART TX behavior for this slice.

With one production task:

- synchronous TX does not starve a peer production task because no peer exists;
- interrupts remain enabled;
- RX IRQ/ring ownership remains independent.

A TX ring/IRQ task/service can be considered when multiple production tasks create real contention.

## 12. Expected source boundary

Initial target source delta was:

**`src/kernel.c` only**

Hardware v1 has now reclassified this boundary after exposing a scheduler host-idle race. The corrected C3.6 source delta is:

**`src/kernel.c` + a minimal `src/kernel/scheduler.c` race closure**

Expected `src/kernel.c` work:

- rename/re-purpose the 1024-byte probe stack as production console storage;
- rename the UART event constant to production-neutral terminology;
- add production console task;
- optionally rename `console_poll()` to `console_drain_rx()` to match its actual semantics;
- add `schedprod` read-only telemetry;
- migrate `kernel_main()` from MSP polling loop to scheduler init/bind/prepare/start;
- add fail-closed handling if production scheduler unexpectedly returns.

Scheduler-core reclassification from hardware evidence:

- `src/kernel/scheduler.c` is no longer a byte-identical guard for C3.6;
- the only permitted scheduler-core change is the host-idle READY/BLOCKED classification race closure;
- no scheduler API/header expansion is planned;
- event payload/notification semantics remain unchanged;
- preemptive scheduling policy remains unchanged.

Remaining guards:

- `include/kernel/scheduler.h`
- `src/startup.s`
- `linker/stm32f103c8.ld`
- OLED driver/render/layout modules

The hardware-discovered race is:

```text
host READY scan -> none
UART IRQ wakes BLOCKED task -> READY
host BLOCKED scan -> none
host aborts run incorrectly
```

The first-pass fix added a second READY check before aborting a no-ready/no-blocked host state. Gate 2B proved this call is linked, but static review found one remaining window: an IRQ can wake the task after the second READY scan and before `scheduler_abort_run()`.

The final required fix is therefore an IRQ-atomic terminal classification:
- save PRIMASK / mask interrupts;
- re-check READY;
- if no READY, check BLOCKED;
- restore PRIMASK before `WFE` when BLOCKED work exists;
- when neither READY nor BLOCKED exists, perform `scheduler_abort_run()` before restoring PRIMASK.

This remains a minimal `src/kernel/scheduler.c` correction and does not expand the scheduler API.

## 12.1 Hardware-v1 reclassification evidence

Candidate:

- `21804 bytes`
- SHA-256 `609BFB2216B178C59E6BC7D0F04F4986571465A24C820E3A34E575DB768858E3`

Passed before failure:

- automatic production scheduler start;
- `SCHED_PROD_CONSOLE_ONLINE` from Thread/PSP;
- host-MSP WFE idle progression;
- full `18/18` safe command surface;
- all `8/8` invasive diagnostics rejected with `SCHED_DIAG_BUSY`;
- production stack `580 / 1024`, margin `444`, canary intact;
- RX drops/errors `0` before burst.

Failure:

- burst produced 14 `PONG`;
- scheduler returned `0`;
- fail-closed emitted `SCHED_PROD_UNEXPECTED_RETURN` and `SCHED_PROD_FATAL`.

This evidence makes the scheduler-core race closure mandatory before C3.6 can proceed.

## 13. Fail-closed return semantics

The production console task is intended to be persistent.

Therefore a normal return from `scheduler_start()` after migration is not a normal application path.

Forbidden fallback:

```text
scheduler returns
    -> resume old MSP console_poll/WFI loop
```

That would create dual ownership and hide a scheduler failure.

Required policy:

```text
scheduler returns unexpectedly
    -> emit explicit fatal/diagnostic marker from MSP if possible
    -> enter fail-closed parked/panic state
    -> do not resume normal MSP console ownership
```

## 14. Deliberate non-goals

Not part of this slice:

- timer-backed `sleep()`;
- timed event waits;
- scheduler priorities;
- a second production application task;
- message queues or mutexes;
- dedicated OLED task;
- UART TX service task;
- moving heartbeat out of SysTick;
- USB/ESP networking work;
- UI geometry redesign;
- HAL/Arduino/FreeRTOS.

## 15. Architectural success condition

The slice is successful only when normal runtime ownership has changed from:

```text
MSP application loop -> console_poll() -> WFI
```

to:

```text
MSP bootstrap
    -> cooperative scheduler host
    -> PSP production console task
    -> BLOCKED on UART event when idle
    -> host MSP WFE
```

without introducing a second blocking model, dual console owners, or new shared-device concurrency.


## 16. Final accepted result — 2026-09-14

Accepted C3.6 candidate:

- path: `build\normal_boot_production_ownership_atomic_racefix_v1\os.bin`
- bytes: `21832`
- SHA-256: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`
- scheduler core: `src/kernel/scheduler.c` SHA-256 `FA649EF24569AEE653A1CA022778237F76FF236A3F0B1B38FDDA8FB06F867649`
- production ownership source: `src/kernel.c` SHA-256 `FE046521F7A017B3DE204E984ECC392CA471C82824530B00EFCBFDFA433658F1`

The migration architecture in this document is accepted.

Final hardware facts:

- production scheduler automatic and cooperative;
- console execution on PSP in task 0;
- task 1 UNUSED;
- host MSP WFE idle;
- safe surface 18/18;
- diagnostics 8/8 BUSY;
- atomic burst regression 4 x 32 PASS;
- 128/128 PONG;
- zero RX drops/errors and final depth zero;
- production stack 580 used / 444 margin;
- MSP 320 used / 1664 margin;
- final target identity exact;
- OLED runtime restore PASS;
- physical `OLED PASS`.

The scheduler-core exception to the original source boundary is now closed and accepted: only the PRIMASK-atomic host classification change in `src/kernel/scheduler.c` was required. No scheduler API/header expansion was introduced.

Publication is the only remaining milestone work.
