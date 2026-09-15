# STM32 OS — Scheduler Timed Blocking Acceptance Plan

Status: **C3.7 GATES 0–5 ACCEPTED — GATE 6 LOCAL COMMIT NEXT**

Published parent:

- commit `33f15f1d23dfa31fabf9f2c83542a5f34046cde8`
- tree `a5447420a58b0aed7cd541069979fa665df40aa9`
- accepted C3.6 firmware `21832` bytes
- firmware SHA-256 `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`

Design authority:

- `docs/SCHEDULER_TIMED_BLOCKING_PLAN.md`

## Gate 0 — planning synchronization

Allowed:

- documentation only.

Required:

- published `main` / `origin/main` / remote `main` all exact parent;
- worktree/index clean before planning;
- exact accepted source guards;
- add the timed-blocking design and acceptance plans;
- synchronize README, MASTER, IMPLEMENTATION_PLAN, PROJECT_HANDOFF,
  ARCHITECTURE, ROADMAP and CHANGELOG;
- temporary Git index contains exactly the planning-doc delta;
- no source/build/flash/commit/push.

## Gate 1 — source implementation

Authorized source paths:

```text
include/kernel/scheduler.h
src/kernel/scheduler.c
src/kernel.c
```

Required implementation contract:

- reuse `SCHEDULER_TASK_BLOCKED`;
- add deadline metadata, not a new task state;
- add `scheduler_sleep_ms()`;
- add `scheduler_wait_events_timeout()`;
- add SVC 4 timed-block path;
- keep existing SVC assignments 0..3 unchanged;
- change tick handoff to `scheduler_tick(uint32_t now_ms)`;
- SysTick calls it with the already incremented `kernel_ticks`;
- scheduler time snapshot is assigned from `now_ms`, never independently
  incremented and never reset by `scheduler_init()`;
- max timeout horizon `0x7FFFFFFF ms`;
- `sleep(0)` immediate/no block/no yield;
- timeout-zero timed event wait acts as pending-event poll;
- event-first and timeout-first each produce exactly one BLOCKED->READY
  transition;
- event wake cancels deadline;
- timeout wake clears event wait;
- timeout wake publishes READY before `SEV`;
- normal production console keeps untimed drain-first event wait;
- add production-safe `schedtimed`;
- no startup/linker/OLED/time-header change.

On source-gate failure before any flash, restore all authorized source paths
byte-for-byte.

## Gate 2 — fresh GNU build validation

Required:

- isolated fresh build directory;
- Arm GNU compile/link with warnings as errors;
- candidate hash/size captured from the fresh build;
- linked symbols for both new APIs;
- SVC 4 dispatch proof;
- wrap-safe deadline compare proof;
- `scheduler_tick(now_ms)` proof;
- no independent scheduler time increment;
- dynamic vector validation;
- stack-usage files interpreted only for standalone linked functions;
- actual BSS/RAM gap measured;
- exact docs/source identity retained;
- temporary-index readiness of the complete C3.7 candidate;
- no flash/commit/push.

## Gate 3 — hardware acceptance

### Baseline regression

Before timed testing:

- exact candidate on target;
- boot markers and immediate `ping -> PONG`;
- production active/cooperative/Thread-PSP;
- task 0 production, task 1 UNUSED;
- host-MSP WFE idle increases;
- safe command surface is `19`;
- all eight invasive scheduler diagnostics return exact `SCHED_DIAG_BUSY`;
- production and MSP canaries intact;
- production margin >=256 bytes;
- RX drops/errors zero.

### Timed command protocol

Host sends:

```text
schedtimed\r\n
```

Target must produce, in order:

```text
SCHED_TIMED_ZERO_OK
SCHED_TIMED_SLEEP_ELAPSED=...
SCHED_TIMED_SLEEP_OK
SCHED_TIMED_TIMEOUT_ELAPSED=...
SCHED_TIMED_TIMEOUT_OK
SCHED_TIMED_EVENT_ARMED
```

After `SCHED_TIMED_EVENT_ARMED`, the host deliberately waits approximately
50 ms, then injects:

```text
ping\r\n
```

Target must then produce:

```text
SCHED_TIMED_EVENT_EVENTS=0x00000001
SCHED_TIMED_EVENT_ELAPSED=...
SCHED_TIMED_EVENT_OK
SCHED_TIMED_OK
PONG
```

Acceptance rules:

- zero phase does not create a blocking interval;
- sleep elapsed >= configured sleep duration;
- timeout phase returns `0` and elapsed >= configured timeout;
- event-wake elapsed is below configured event deadline;
- event mask is exactly UART RX event bit 0;
- exactly one timed result is emitted for each phase;
- injected command payload survives in the ring and yields exact `PONG`;
- no scheduler return/fatal marker.

### Race regression retained

Repeat the accepted C3.6 burst regression:

```text
4 rounds x 32 unpaced ping
```

Each round:

- exactly `32/32 PONG`;
- zero RX drops/errors;
- final RX depth zero;
- scheduler remains active;
- PSP canary intact and margin >=256;
- fault zero;
- no production return/fatal.

### Final health

- `schedprod` healthy;
- MSP telemetry healthy;
- `uiruntime` PASS;
- final `ping` PASS;
- final target Flash readback exact candidate;
- repo/source/docs/index unchanged by hardware gate.

## Gate 4 — physical OLED

Manual confirmation remains separate.

Expected frozen UI:

```text
DEUS OS
BOOT OK
READY
```

Required user acceptance:

```text
OLED PASS
```

## Gate 5 — documentation finalization

Bind exact Gate 2/3/4 evidence and update canonical docs with:

- accepted candidate hash/size;
- deadline/state semantics;
- timed diagnostic outcomes;
- full retained C3.6 regression;
- measured stack/RAM facts;
- hardware + physical OLED result.

Temporary-index commit candidate must pass before commit.

## Gate 6 — local acceptance commit

- stage exact accepted C3.7 path set only;
- require exact temporary-index tree;
- one local acceptance commit;
- worktree/index clean after commit;
- remote remains on old parent;
- no push.

## Gate 7 — non-force publication

- exact accepted local commit;
- exact parent and tree;
- clean worktree/index;
- remote old parent is ancestor;
- expected ahead/behind `1/0`;
- ordinary `git push origin main:main`;
- no force / force-with-lease;
- remote/local converge to `0/0`.

## Failure classification

A failed harness gate is not automatically a product failure.

Always classify first:

```text
HARNESS
SOURCE/BUILD
HARDWARE/PRODUCT
EVIDENCE
```

If Flash has not started, source rollback is allowed and required for failed
source mutation.

After Flash starts, do not roll source back to bytes that no longer correspond
to the target image.

## Executed acceptance result — 2026-09-15

Completed:

- Gate 0 planning synchronization — PASS.
- Gate 1 source implementation — PASS.
- Gate 2 fresh GNU build — PASS.
- Gate 3 automated real-hardware acceptance — PASS.
- Gate 4 physical frozen OLED — `OLED PASS`.
- Gate 5 documentation/evidence finalization — this exact final document set.

Accepted candidate:

```text
bytes  22900
SHA256 366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A
```

Timed hardware result:

```text
SCHED_TIMED_ZERO_OK            PASS
sleep elapsed                  50 ms
timeout elapsed                50 ms
event mask                     0x00000001
event elapsed                  164 ms
timed RX delta                 26
timed host idle delta          2904
no double wake                 PASS
ring payload -> PONG           PASS
```

Retained acceptance:

```text
safe production surface        19/19
invasive scheduler BUSY        8/8
4 x 32 burst                   128/128 PONG
RX drops/errors/final depth    0/0/0
production stack               580 used / 444 margin
MSP                            340 used / 1644 margin
final target readback          exact
OLED runtime restore           PASS
physical frozen OLED           PASS
```

Evidence:

```text
build log      6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84
build evidence 431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199
hardware log   3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769
hardware ev    ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4
```

Remaining gates:

- Gate 6 local acceptance commit;
- Gate 7 plain non-force fast-forward publication.

No priority work begins before Gate 7 PASS.
