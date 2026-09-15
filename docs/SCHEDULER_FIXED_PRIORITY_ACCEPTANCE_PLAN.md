# STM32 OS — Scheduler Fixed-Priority Acceptance Plan

Status: **C3.8 GATES 0–5 ACCEPTED — GATE 6 LOCAL COMMIT NEXT**

Published parent:

- commit `90df6a690230c9800c0d8497d5597f87a5ae0409`
- tree `c9e8cfd4bfa1dbbcc6d4d907b9c601c0d70fec59`
- accepted C3.7 firmware `22900` bytes
- SHA-256 `366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A`

Design authority:

- `docs/SCHEDULER_FIXED_PRIORITY_PLAN.md`

## Gate 0 — planning synchronization

Allowed:

- documentation only.

Required:

- local HEAD / origin/main / remote main exact published parent;
- clean worktree/index before planning;
- exact C3.7 source guards;
- add fixed-priority design and acceptance plans;
- synchronize README, MASTER, IMPLEMENTATION_PLAN, PROJECT_HANDOFF,
  ARCHITECTURE, ROADMAP and CHANGELOG;
- temporary Git index contains exactly the planning-doc delta;
- no source/build/flash/commit/push.

## Gate 1 — source implementation

Authorized paths:

```text
include/kernel/scheduler.h
src/kernel/scheduler.c
src/kernel.c
```

Required:

- add static `priority` to TCB;
- constants `0 / 128 / 255`;
- lower numeric value means higher priority;
- `scheduler_init()` defaults every slot to 128;
- add `scheduler_task_priority_set()`;
- reject priority setter while scheduler active;
- stack bind and prepare preserve assigned priority;
- make the single READY selector priority-aware;
- equal priority preserves existing round-robin tie order;
- initial SVC0, cooperative SVC path, host re-entry and PendSV all use it;
- no new SVC;
- production task 0 explicitly DEFAULT priority;
- task 1 remains UNUSED;
- add task0 priority to `schedprod`;
- add safe `schedprio`;
- add `scheduler_priority_self_test()` expected mask `0x0000003F`;
- normal production remains cooperative;
- no event/timed/WFE semantic change.

Before Flash, any Gate 1 failure restores all authorized source paths exactly.

## Gate 2 — fresh GNU build validation

Required:

- isolated fresh build directory;
- warnings as errors;
- exact source/docs identity;
- full temporary-index candidate readiness;
- linked `scheduler_task_priority_set`;
- linked `scheduler_priority_self_test`;
- priority-aware selector proof;
- equal-priority round-robin proof;
- SVC0 / cooperative SVC / PendSV selector integration proof;
- no new SVC number;
- retained timed SVC4 proof;
- retained SysTick->scheduler_tick + SEV wake proof;
- retained PRIMASK/WFE host classification CFG proof;
- production DEFAULT-priority binding proof;
- priority telemetry/diagnostic markers linked;
- measured candidate hash/bytes/text/data/BSS/RAM/stack facts;
- no flash/commit/push.

## Gate 3 — real hardware acceptance

### Baseline

Require:

- exact candidate flash/readback;
- production active/cooperative/Thread-PSP;
- task 0 READY;
- task 1 UNUSED;
- task 0 priority `0x00000080`;
- host-MSP WFE idle increases;
- RX clean;
- no fatal/return marker.

### Priority diagnostic

Host sends:

```text
schedprio\r\n
```

Exact required markers:

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

After `schedprio`:

- production scheduler remains active;
- task 0 priority remains `0x00000080`;
- task 1 remains UNUSED;
- production PSP/canary/margin healthy;
- preempt switch count remains 0 in normal cooperative mode;
- RX drop/error/depth remain zero.

### Retained timed blocking

Run `schedtimed` and retain:

- zero sleep PASS;
- 50 ms sleep not early;
- 50 ms timeout not early;
- deliberate external UART event wake before 500 ms;
- event mask 1;
- exactly one timed result;
- injected `ping` survives in RX ring and yields exact `PONG`.

### Safe surface / isolation

Safe production surface is exactly `20/20`.

All eight invasive diagnostics remain exact `SCHED_DIAG_BUSY`.

### Retained host-idle race regression

Run:

```text
4 rounds x 32 unpaced ping
```

Each round requires:

- `32/32 PONG`;
- exact expected RX byte/IRQ delta;
- no abort/fatal;
- RX drops/errors/final depth zero;
- production scheduler active;
- PSP canary and margin healthy;
- host idle advances.

### Final health

- `schedprod` healthy with priority telemetry;
- MSP telemetry healthy;
- `uiruntime` PASS;
- final `ping` PASS;
- final Flash exact candidate;
- repo/source/docs unchanged by hardware gate.

## Gate 4 — physical OLED policy

Gate 4 is N/A if:

- OLED/gfx hashes are unchanged from C3.7;
- automated runtime UI restore PASS;
- no hardware display anomaly is observed.

Record:

```text
PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS
```

If any prerequisite is false, manual physical acceptance becomes mandatory.

## Gate 5 — documentation finalization

Bind exact Gate 2/3/4 results and update canonical docs with:

- accepted candidate identity;
- fixed-priority semantics;
- selector self-test result;
- active setter rejection;
- safe surface `20/20`;
- retained timed and burst regressions;
- measured stack/RAM facts;
- OLED Gate 4 disposition.

Temporary-index exact commit candidate must PASS.

## Gate 6 — local acceptance commit

- stage exact accepted C3.8 path set only;
- require exact accepted tree;
- create one local acceptance commit;
- verify parent/tree/subject/path set;
- final worktree/index clean;
- remote remains old parent;
- no push.

## Gate 7 — non-force publication

- exact accepted local commit;
- clean worktree/index;
- remote old parent is ancestor;
- expected behind/ahead `0/1`;
- ordinary `git push origin main:main`;
- no force / no force-with-lease;
- final local/remote `0/0`.

## Failure classification

Classify before repair:

```text
HARNESS
SOURCE/BUILD
HARDWARE/PRODUCT
EVIDENCE
```

No history rewrite after a created commit fails post-verification.
No automatic remote rewrite after a successful push with failed post-check.

## Executed acceptance result — 2026-09-15

Completed:

- Gate 0 planning synchronization — PASS.
- Gate 1 source implementation — PASS.
- Gate 2 fresh GNU build — PASS.
- Gate 3 automated real-hardware acceptance — PASS.
- Gate 4 physical OLED — `N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.
- Gate 5 documentation/evidence finalization — this exact final document set.

Accepted candidate:

```text
bytes  23792
SHA256 492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F
```

Priority result:

```text
policy                     LOWER_VALUE_HIGHER
highest/default/lowest     0 / 128 / 255
selector self-test         0x0000003F
task0 before/after         128 / 128
active setter              REJECTED
preempt switches           0
```

Retained acceptance:

```text
safe production surface        20/20
timed blocking                 4/4
sleep/timeout                  50/50 ms
event wake                     171 ms / event 1
invasive scheduler BUSY        8/8
4 x 32 burst                   128/128 PONG
RX drops/errors/final depth    0/0/0
production stack               580 used / 444 margin
MSP                            340 used / 1644 margin
final target readback          exact
OLED runtime restore           PASS
```

Gate 4 disposition:

```text
PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS
```

Evidence:

```text
build log      306AC5D97125F5BEFB4B2DB95E602ED8F6541E32CF364AA61ACD3D93A0E946D0
build evidence 33E699282A456B79A0F15C8B2B6DF756BAD5979867091863599575A851CEA000
hardware log   005D646FD613506896BC1A3961DDA752D9D424AD7F4AD0CFAAEE377C50E05222
hardware ev    41665A892477FB195D00DD722A093F69B09AB2A66669EB872DDB7A3518EACC6C
```

Remaining:

- Gate 6 local acceptance commit;
- Gate 7 ordinary non-force fast-forward publication.
