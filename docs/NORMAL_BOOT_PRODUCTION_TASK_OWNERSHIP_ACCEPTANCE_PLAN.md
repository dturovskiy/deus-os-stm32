# Normal-Boot Production Task Ownership Migration — Acceptance Plan

Status: **GATES 0–5 ACCEPTED — COMMIT/PUSH PENDING**

Baseline commit:

`bfed76e0de1c52bf65e60f66c029cc41710fabd0` — `feat: add scheduler steady-state wait/wake foundation`

This plan defines the gate sequence and evidence required to accept:

**NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_MIGRATION**

## Gate 0 — planning/documentation synchronization

Purpose:

- establish one canonical ownership design before source changes;
- remove stale current-state contradictions from authoritative docs;
- bind the migration to the published wait/wake baseline.

Required:

- canonical docs identify `bfed76e0de1c52bf65e60f66c029cc41710fabd0` as current published baseline;
- wait/wake foundation is marked published/closed;
- the next implementation boundary is normal-boot production task ownership migration;
- `sleep()`/timers/priorities remain deferred;
- detailed ownership plan and this acceptance plan are present;
- full documentation commit-candidate check passes through a temporary Git index.

No source edit, build, flash, commit, or push.

## Gate 1 — implementation/source package

Initial source delta:

`src/kernel.c` only.

Hardware-v1 reclassification adds one minimal scheduler-core correction:

`src/kernel/scheduler.c` host-idle atomic READY/BLOCKED terminal reclassification under `PRIMASK`, with abort before interrupt restore.

No scheduler header/API change is planned.

Required source design:

- one long-lived production console task;
- slot 0 = production console;
- slot 1 = UNUSED;
- 1024-byte production console PSP stack;
- `scheduler_start()` cooperative mode;
- drain RX ring before each wait;
- wait on production UART RX event;
- no MSP console fallback after scheduler start;
- initial OLED UI still composed before scheduler start;
- runtime OLED/I2C application calls owned by production console task;
- optional read-only `schedprod` command;
- all invasive scheduler diagnostics remain lifecycle-gated.

Guard files after hardware-v1 reclassification:

- `include/kernel/scheduler.h`
- `src/startup.s`
- `linker/stm32f103c8.ld`
- frozen OLED implementation/baseline files

`src/kernel/scheduler.c` is permitted to change only for the hardware-proven host-idle READY/BLOCKED race closure.

On failure before flash: exact source rollback.

## Gate 2 — source/build validation

Build from the exact source candidate.

Required:

- `-Wall -Wextra -Werror`;
- compile/link/objcopy PASS;
- `git diff --check` PASS;
- exact expected source dirty set;
- guard hashes unchanged;
- candidate `.bin` / `.elf` identity recorded;
- candidate byte count and SHA-256 recorded;
- `.bss`, `_ebss`, SRAM gap below MSP reservation recorded;
- stack-usage output collected for new/changed functions;
- vector table still binds Reset/SVC/PendSV/SysTick/USART1 correctly;
- disassembly proves expected `WFE` scheduler host path remains linked;
- `scheduler_wait_events()` / SVC #3 path remains linked;
- USART1 IRQ still calls event signal only after RX ring publication.

No flash, commit, or push.

## Gate 3 — hardware acceptance

Flash only the exact Gate-2 candidate.

### 3.1 Boot ownership proof

Required boot/runtime markers:

- normal boot reaches the frozen product UI;
- production console task emits an online marker from PSP;
- `schedprod` or equivalent read-only telemetry proves:
  - scheduler active = 1;
  - cooperative production mode;
  - current command execution uses PSP;
  - PSP lies inside the 1024-byte production console stack;
  - slot 0 is the production task;
  - slot 1 remains UNUSED.

### 3.2 Idle / wake proof

After boot and before sending a command:

- allow a deliberate quiet interval;
- production console task must block;
- host scheduler MSP must execute `WFE`;
- idle-wait counter must become nonzero.

Then send UART command traffic and prove:

```text
USART1 IRQ
 -> ring byte publication
 -> scheduler event signal
 -> task BLOCKED -> READY
 -> PSP console resumes
 -> command response
```

At minimum:

- repeated `ping -> PONG`;
- idle count remains positive/increases;
- no lost wake;
- no requirement that one event equals one payload byte.

### 3.3 Boot-race / pending-event proof

Prove the drain-first contract:

- inject command/RX data as early as practical around boot/scheduler activation;
- bytes present in the ring before/around task start must still be processed;
- no dependency on an event being delivered while scheduler was inactive.

### 3.4 Safe production command surface

Run the complete non-invasive production command surface on the real production task.

Historical safe surface: 17 commands.

If `schedprod` is added:
- required production safe surface = 18 commands.

All must complete without:
- task stack canary failure;
- RX drops/errors;
- scheduler lifecycle loss;
- OLED corruption.

### 3.5 Scheduler diagnostic isolation under production ownership

From the active production console, execute all eight invasive scheduler command names:

- `schedtest`
- `schedcoop`
- `schedpreempt`
- `schedstack`
- `schedworkload`
- `schedconsoleprobe`
- `schedwaitwake`
- `schedisolate`

Expected for each:

`SCHED_DIAG_BUSY`

After every rejection:
- scheduler remains active;
- `ping` still returns `PONG`;
- production task remains valid;
- no scheduler reset occurs.

### 3.6 Production PSP stack acceptance

Capacity:

`1024 bytes`

Required:
- canary intact;
- high-water measured after full command/regression surface;
- free margin >= `256 bytes`.

Historical 600-byte high-water is context only.
The migrated runtime measurement is authoritative for the migrated runtime.

### 3.7 MSP acceptance

The accepted MSP reservation remains:

- reserved: 2048 bytes;
- measurable capacity: 1984 bytes.

Required:
- MSP canary intact;
- high-water recorded;
- margin remains comfortably positive;
- scheduler host's long-lived `scheduler_start()` frame does not violate the accepted MSP budget.

### 3.8 UART RX pressure + host-idle race regression

The first hardware run exposed a false scheduler abort under dense UART wake traffic. The corrected candidate must therefore pass a timing-sensitive repeated regression, not only a single burst.

Required:
- at least `4` rounds;
- each round sends `32 x "ping\r\n"` without per-command pacing;
- each round returns exact `32/32` `PONG`;
- no `SCHED_PROD_RETURN`, `SCHED_PROD_UNEXPECTED_RETURN`, `SCHED_PROD_FATAL`, or fault record;
- after each round, `schedprod` reports scheduler still active, task 0 valid, task 1 UNUSED, canary intact, margin >=256, fault=0;
- RX capacity = 128;
- drop count = 0;
- error count = 0;
- depth returns to 0 after every round;
- IRQ/byte deltas exactly match injected burst bytes plus the telemetry command bytes;
- console remains responsive after every round.

USART1 IRQ remains sole `DR` reader.

This gate specifically proves closure of the host sequence:

`READY-none -> IRQ wake to READY -> BLOCKED-none -> false abort`.

### 3.9 OLED/I2C regression

From production PSP:

- I2C scan/transport checks required by current regression;
- OLED command/render/runtime restore path;
- `uiruntime -> OLED_RUNTIME_UI_OK`;
- final framebuffer/UI restored to frozen product presentation.

No IRQ-side OLED/I2C work.

### 3.10 Final target identity

After runtime regression:
- read target Flash;
- verify exact byte identity with the accepted candidate.

## Gate 2B review — first-pass fix NOT eligible for hardware acceptance

Fresh build PASS:
- `21816 bytes`;
- SHA-256 `ED4B1699FBF7274370A43AE34B348C8C3EC179D2137AF53E794E216D64ED4819`;
- two linked `scheduler_find_next_ready` calls in `scheduler_start_mode`;
- BSS/RAM/stack/vector gates PASS.

Static race review:
- the second READY check itself is not atomic with `scheduler_abort_run()`;
- an IRQ can still execute `BLOCKED -> READY` after that check and before abort;
- therefore this candidate must not be flashed for acceptance.

Required before Gate 3:
- atomic READY/BLOCKED classification under PRIMASK;
- terminal abort while PRIMASK remains masked;
- fresh Gate 2C build proving the atomic critical-section instructions and control flow.

## Gate 3A result — hardware v1 FAILED on real scheduler race

Candidate `21804 bytes`, SHA-256 `609BFB2216B178C59E6BC7D0F04F4986571465A24C820E3A34E575DB768858E3`.

Passed:
- production PSP ownership;
- host-MSP WFE idle;
- 18 safe commands;
- 8/8 diagnostic-busy isolation;
- pre-burst production stack margin `444` bytes;
- RX drops/errors `0`.

Failed:
- burst returned 14 `PONG`, then `scheduler_start()` returned `0`;
- fail-closed output: `SCHED_PROD_UNEXPECTED_RETURN`, `SCHED_PROD_FATAL`.

Disposition:
- do not commit/push;
- reclassify source scope;
- apply minimal scheduler host-idle race fix;
- rebuild and repeat Gate 3 with the strengthened burst regression above.

## Gate 4 — manual physical OLED acceptance

Required operator confirmation:

```text
DEUS OS
BOOT OK
READY
```

No geometry/artifact regression.

This is separate from automated I2C/UART acceptance.

## Gate 5 — documentation finalization

Only after hardware + physical PASS:

- record accepted firmware hash/size;
- record production PSP/MSP/RX telemetry;
- record exact production ownership;
- mark migration accepted;
- update next boundary to timer-backed sleep integration;
- keep priorities later;
- run exact hash + structural validation;
- run full temporary-index commit-candidate `git diff --cached --check`.

No commit or push.

## Gate 6 — acceptance commit

Separate package.

Before real staging:
- bind accepted Gate-5 evidence;
- temporary-index rehearsal exact candidate;
- exact file set;
- whitespace check;
- staged blob verification.

Then create one local commit.

No push.

## Gate 7 — non-force push

Separate package.

Require:
- clean worktree/index;
- exact accepted local commit;
- remote still on expected parent;
- fast-forward relation proven.

Allowed publication:

`git push origin HEAD:refs/heads/main`

No force / force-with-lease.

## Milestone exit criteria

The milestone is closed only when:

- normal boot application console ownership is PSP task-owned;
- MSP is bootstrap + scheduler host/idle + exception stack, not application console owner;
- production idle is blocked/event-driven, not polling;
- UART IRQ/ring ownership is unchanged;
- frozen OLED remains accepted;
- production stack/MSP/RX telemetry passes;
- invasive diagnostics cannot reset the live scheduler;
- exact candidate is committed and non-force published.


## Final gate record — 2026-09-14

### Gate 2C — PASS

- fresh GNU build candidate: `21832` bytes;
- SHA-256 `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`;
- BSS `5328`;
- RAM gap below MSP `15152`;
- linked CFG proof of atomic terminal abort + restored blocked WFE path PASS.

### Gate 3C — PASS

- exact candidate flashed and read back;
- boot ownership PASS;
- host-MSP idle PASS;
- safe production surface `18/18`;
- diagnostic isolation `8/8`;
- race regression `4/4 x 32 ping`;
- total `128/128 PONG`;
- exact per-round RX byte/IRQ delta `200`;
- RX drops/errors/depth `0/0/0`;
- final production stack `580 used / 444 margin`, canary intact;
- final MSP `320 used / 1664 margin`, canary intact;
- final scheduler fault `0`;
- final Flash identity exact;
- OLED automated restore PASS.

### Gate 4 — PASS

Manual physical OLED accepted by user:

`OLED PASS`

Frozen UI accepted: `DEUS OS / BOOT OK / READY`.

### Gate 5 — PASS

Canonical documentation finalized to the accepted candidate and full 12-file temporary-index commit-candidate validation required.

Remaining:

- Gate 6 local acceptance commit;
- Gate 7 non-force push.
