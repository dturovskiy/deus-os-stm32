# C4.0 IWDG Production Liveness Foundation Acceptance Plan

Status: **C4.0 GATES 0–7 ACCEPTED / PUBLISHED `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`**

Boundary:

`IWDG_LIVENESS_FOUNDATION_C4_0`

Published parent:

`39ea5b3d1b72fca8d15e22e7544870ab0704c274`

Published tree:

`c4016135374c8c6726a66736943d117f4583066e`

## Gate 0 — planning/docs

Required:

- exact clean published C3.9 prestate;
- HEAD/origin/main/live remote all at the published parent;
- seven authority docs updated;
- this design and acceptance plan added;
- no source edit;
- no build;
- no flash;
- no commit;
- no push;
- real Git index remains clean;
- exact temporary-index commit-candidate readiness.

## Gate 1 — source implementation

Authorized source paths:

```text
src/kernel.c
src/drivers/iwdg.c
include/drivers/iwdg.h
```

Required:

- minimal register-level IWDG driver;
- bounded LSI-ready/configuration behavior;
- static nominal approximately 8 s IWDG configuration;
- reset-cause capture before reset-flag clear;
- debugger IWDG freeze configuration;
- watchdog start immediately before normal scheduler ownership;
- task0 Thread/PSP progress reload point;
- task1 heartbeat Thread/PSP reload point;
- no reload in SysTick/IRQ/Handler/fault/host-idle paths;
- existing `health` extended with watchdog/reset telemetry;
- destructive `wdogtrip` with exact `WDOG_TRIP_ARMED`;
- safe surface remains `20`;
- existing eight scheduler diagnostics remain `8`;
- no scheduler core/header edit;
- no new SVC;
- no IPC;
- no OLED/gfx/status-bar edit.

No build, flash, commit or push.

## Gate 2 — fresh GNU build validation

Use Arm GNU `-Wall -Wextra -Werror -fstack-usage`.

Required proof:

- exact C3.9 parent/source/doc identities;
- only the authorized three source paths differ;
- new driver compiles and links;
- linked kernel start path reaches IWDG start before `scheduler_start`;
- linked task0 and task1 paths both reach the reload function;
- linked SysTick and USART1 IRQ do not reach the reload function;
- fault path and `production_fail_closed` do not reach reload;
- `wdogtrip` links to a non-returning no-reload path;
- no scheduler-core source change;
- no SVC5;
- task0 stack remains `1024 B`;
- task1 stack remains `512 B`;
- direct stack-usage gates remain healthy;
- Flash/RAM/MSP fit remains healthy;
- exact temporary-index candidate readiness.

No flash.

## Gate 3 — real hardware acceptance

Flash only the exact Gate 2 candidate.

### Boot / active watchdog

Required after normal boot:

```text
ping -> PONG
health -> existing HEALTH prefix plus watchdog fields
WDOG_ACTIVE=0x00000001
WDOG_RELOAD_COUNT > 0 after production progress
```

Initial `IWDG_RESET` is recorded but not required to be zero.

### Normal-runtime watchdog retention

While watchdog is active, retain:

- two-task heartbeat progression and PC13 transitions;
- task1 priority `255`, stack margin at least `256 B`, fault `0`;
- task0 priority `128`, stack margin at least `256 B`;
- safe surface exactly `20/20`;
- priority self-test `0x0000003F`;
- timed blocking `4/4`;
- eight scheduler diagnostics exact `SCHED_DIAG_BUSY`;
- `4 rounds x 32` unpaced ping = `128/128 PONG`;
- RX drops/errors/final depth `0/0/0`;
- MSP canary/margin healthy;
- watchdog reload count progresses without any unexpected reboot.

### Deliberate expiration

After all non-destructive regressions pass:

1. issue `wdogtrip`;
2. require exact UART marker `WDOG_TRIP_ARMED`;
3. do not send any further command to keep the system alive;
4. observe target reboot within a broad hardware acceptance window;
5. reconnect UART after boot;
6. require `ping -> PONG`;
7. require `health` reports:
   `WDOG_ACTIVE=0x00000001` and `IWDG_RESET=0x00000001`.

The harness must tolerate COM3 closing/reopening around the hardware reset.

### Post-reset retained proof

After deliberate IWDG reset:

- heartbeat progresses again;
- safe production surface remains `20/20`;
- `schedprio` still passes;
- `schedtimed` still passes;
- `uiruntime` prints `OLED_RUNTIME_UI_OK`;
- final `ping` passes;
- final Flash readback exactly matches the Gate 2 candidate.

## Gate 4 — OLED disposition

If OLED/gfx/status-bar source hashes are exact and `uiruntime` passes:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise manual physical OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record:

- exact candidate hash/size;
- IWDG configuration;
- normal watchdog reload progression;
- deliberate expiration time;
- captured IWDG reset cause;
- post-reset scheduler/UART/timed/OLED regression;
- exact final Flash identity;
- Gate 4 disposition.

No commit.

## Gate 6 — local acceptance commit

Create one local acceptance commit only after Gates 0–5 pass.

No push.

## Gate 7 — non-force publication

Require exact accepted commit, clean repo, remote parent identity,
fast-forward ancestry and pre-push `behind=0 / ahead=1`.

Perform only:

`git push origin main:main`

No force and no force-with-lease.

After publication require `behind=0 / ahead=0`.

## Acceptance invariant

C4.0 is accepted only if ordinary production keeps IWDG alive through real
Thread/PSP progress, Handler-mode code cannot mask a production lockup by
reloading the watchdog, a deliberate no-progress state produces a real IWDG
reset, reset cause is captured after reboot, and all published C3.9 scheduler,
UART, timed-blocking and frozen-OLED regressions remain green.

## Acceptance record — 2026-09-15

Gates 0–5 are accepted.

Build evidence:

- log `D7034BB4597EEF3C5CF1CA6025BAA148BCAD1F21D9534009EF242EAD334A0421`;
- evidence `96773CADE1440BA844FE1455E049109C4099318F57A8F906D7F28148471E5709`;
- source candidate tree `f8e879f815051d300eec728f2afe03c39222ca47`;
- candidate `25192` bytes /
  `4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D`.

Hardware evidence:

- log `6C613D31AEF61968021288F41EED3D28EE8B14DBB8C7A129B21636858676B2D9`;
- evidence `08410EF7BC5F330CF2D18BD7CEDF5E85D83FCD4A825C3D5BD0D0C1E4F7D24F66`;
- normal reload `39 -> 50`;
- exact `WDOG_TRIP_ARMED`;
- real IWDG reboot after `7294 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- post-reset heartbeat delta `3`;
- post-reset safe surface `20/20`;
- final Flash exact.

Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 4 disposition SHA-256:

`3B69B950D80AB8402D873E02567BEF4976E846CC585EE680CCD852B4E96778AE`

The stale trailing `MANUAL_PHYSICAL_OLED_ACCEPTANCE` text emitted by the
hardware runner is non-authoritative legacy text; the hardware evidence outcome
and manifest record `MANUAL_PHYSICAL_OLED_REQUIRED=False`.

Next:

**Gate 6 — local acceptance commit.**
