# C3.9 Production Heartbeat Task Acceptance Plan

Status: **C3.9 GATES 0–5 ACCEPTED — GATE 6 LOCAL COMMIT NEXT**

Boundary:

`PRODUCTION_HEARTBEAT_TASK_OWNERSHIP_C3_9`

Published parent:

`0312bb376c365c0235b9cafe22e927254528f2c4`

## Gate 0 — planning/docs

Required:

- exact clean published C3.8 prestate;
- exact published source guards;
- seven authority docs updated;
- this design and acceptance plan added;
- no source edit;
- no build;
- no flash;
- no commit;
- no push;
- real Git index remains clean;
- temporary-index candidate readiness PASS.

## Gate 1 — source implementation

Authorized source path:

`src/kernel.c`

Required implementation:

- task1 heartbeat stack = `128` words / `512` bytes;
- heartbeat minimum runtime margin = `256` bytes;
- heartbeat period = `500 ms`;
- task0 priority = `128`;
- task1 priority = `255`;
- bind/prepare both production tasks;
- task1 uses `scheduler_sleep_ms()`;
- task1 owns post-start PC13 writes;
- SysTick no longer owns PC13 heartbeat;
- no busy delay;
- no new command;
- safe command surface remains `20`;
- `schedprod` gains task1/heartbeat telemetry;
- `schedprio` reports task1 priority and retains self-test `0x0000003F`;
- `health` remains PC13 ODR observer.

Required guards:

- `src/kernel/scheduler.c` unchanged;
- `include/kernel/scheduler.h` unchanged;
- `include/kernel/time.h` unchanged;
- `src/startup.s` unchanged;
- linker unchanged;
- OLED/gfx unchanged.

No build, flash, commit or push.

## Gate 2 — fresh GNU build validation

Build with Arm GNU `-Wall -Wextra -Werror`.

Required proof:

- exact source/docs identities;
- only `src/kernel.c` changed from C3.8 source set;
- task1 stack symbol exactly `512` bytes;
- task0 stack remains `1024` bytes;
- linked `production_heartbeat_task`;
- linked heartbeat task calls `scheduler_sleep_ms`;
- linked `SysTick_Handler` still calls `scheduler_tick`;
- source/linked proof that SysTick no longer implements PC13 heartbeat policy;
- task1 priority binding to `SCHEDULER_PRIORITY_LOWEST`;
- task0 remains `SCHEDULER_PRIORITY_DEFAULT`;
- no new SVC;
- retained SVC4 timed APIs;
- retained host PRIMASK/WFE race closure;
- RAM/Flash fit;
- stack-usage gates;
- exact temporary-index commit candidate.

No flash.

## Gate 3 — real hardware acceptance

Flash only the exact Gate 2 candidate.

### Boot ownership

Required:

- normal boot reaches task0 PSP console;
- immediate `ping` -> one `PONG`;
- scheduler remains cooperative;
- host MSP WFE remains idle path.

### Two-task production telemetry

`schedprod` must prove:

```text
SCHED_PROD_ACTIVE=0x00000001
SCHED_PROD_THREAD_PSP=0x00000001
SCHED_PROD_TASK0_STATE=0x00000001
SCHED_PROD_TASK0_PRIORITY=0x00000080
SCHED_PROD_TASK1_PRIORITY=0x000000FF
SCHED_PROD_HEARTBEAT_STARTED=0x00000001
SCHED_PROD_HEARTBEAT_FAULT=0x00000000
SCHED_PROD_HEARTBEAT_CAPACITY=0x00000200
SCHED_PROD_HEARTBEAT_CANARY=0x00000001
SCHED_PROD_PREEMPT_SWITCHES=0x00000000
SCHED_PROD_FAULT=0x00000000
SCHED_PROD_OK
```

Task1 state may be `READY` or `BLOCKED`, but must never be `UNUSED` or `DONE`.

Heartbeat stack margin must be at least `256` bytes.

Console stack margin remains at least `256` bytes.

### Heartbeat progression / GPIO ownership

Use `schedprod` heartbeat count plus existing `health` PC13 ODR telemetry.

Required:

- heartbeat count progresses during quiet production time;
- at least two heartbeat periods are observed;
- PC13 ODR transitions are consistent with heartbeat-count parity;
- host idle-wait count progresses while both tasks sleep/block;
- no heartbeat fault.

This is automated; no manual LED observation is required.

### Priority regression

`schedprio` must retain:

```text
SCHED_PRIO_POLICY=LOWER_VALUE_HIGHER
SCHED_PRIO_HIGHEST=0x00000000
SCHED_PRIO_DEFAULT=0x00000080
SCHED_PRIO_LOWEST=0x000000FF
SCHED_PRIO_TASK0=0x00000080
SCHED_PRIO_TASK1=0x000000FF
SCHED_PRIO_SELFTEST=0x0000003F
SCHED_PRIO_ACTIVE_SET_REJECT_OK
SCHED_PRIO_OK
```

Both task priorities remain unchanged after rejected active mutation.

### Retained C3.8/C3.7/C3.6 regression

Required:

- safe production surface exactly `20/20`;
- `schedtimed` `4/4`;
- eight invasive scheduler commands exact `SCHED_DIAG_BUSY`;
- ping healthy after each BUSY result;
- `4 rounds x 32 unpaced ping`;
- total `128/128 PONG`;
- each round exact RX byte/IRQ delta `200`;
- RX drops/errors/final depth `0/0/0`;
- production and heartbeat stack canaries healthy;
- MSP canary/margin healthy;
- final `uiruntime` PASS;
- final `ping` PASS;
- final Flash readback exact candidate.

## Gate 4 — OLED physical disposition

If all OLED/gfx source hashes remain exact and `uiruntime` passes:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise manual OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record exact candidate hashes, two-task telemetry, heartbeat progress,
priority regression, retained regressions and Gate 4 disposition.

Temporary-index exact candidate readiness required.

No commit.

## Gate 6 — local acceptance commit

Create one local acceptance commit only after Gates 0–5 pass.

No push.

## Gate 7 — non-force publication

Require exact accepted local commit, clean repository, remote parent identity,
fast-forward ancestry and ahead/behind `0/1`.

Perform one ordinary:

`git push origin main:main`

No force and no force-with-lease.

After push require local/remote `0/0`.

## Acceptance invariant

C3.9 is accepted only if task1 is a genuine persistent production heartbeat
owner, SysTick no longer contains heartbeat policy, and all prior production
scheduler/UART/timed/OLED regressions remain green.


## Acceptance result — 2026-09-15

Gate 0 planning/docs: **PASS**

Gate 1 source implementation: **PASS**

Gate 2 fresh GNU build: **PASS**

Gate 3 hardware acceptance: **PASS**

- candidate `24648` bytes / `4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86`;
- heartbeat phase `1/1`;
- heartbeat count `3 -> 10`, delta `7`;
- PC13 transitions `5`, both ODR states;
- task1 stack `80 used / 432 margin`;
- priority phase `1/1`, self-test `0x0000003F`;
- task0 `128 -> 128`;
- task1 `255 -> 255`;
- safe surface `20/20`;
- timed blocking `4/4`;
- diagnostics `8/8 BUSY`;
- `4 x 32 = 128/128 PONG`;
- RX drop/error/depth `0/0/0`;
- MSP `340 used / 1644 margin`;
- `OLED_RUNTIME_UI_OK`;
- final Flash exact.

Gate 4 physical OLED:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 5 documentation/evidence finalization: **PASS when this final document set
is installed and its temporary-index readiness check succeeds.**

Next:

**Gate 6 — local acceptance commit.**
