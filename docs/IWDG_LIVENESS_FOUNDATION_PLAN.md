# C4.0 IWDG Production Liveness Foundation Plan

Status: **C4.0 GATES 0–5 ACCEPTED — GATE 6 LOCAL COMMIT NEXT**

Boundary ID:

`IWDG_LIVENESS_FOUNDATION_C4_0`

Published parent:

`39ea5b3d1b72fca8d15e22e7544870ab0704c274` — `feat: add production heartbeat task`

Published tree:

`c4016135374c8c6726a66736943d117f4583066e`

Published firmware:

`24648` bytes /
`4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86`

## 1. Purpose

Add the STM32F103 independent watchdog as a production execution-liveness
safety net without weakening the published scheduler architecture.

The watchdog must detect loss of forward progress. Therefore it must not be
blindly reloaded from SysTick or any interrupt path.

## 2. Hardware choice

Use **IWDG**.

Do not use WWDG in C4.0.

Clock authority:

the independent LSI oscillator.

The kernel millisecond clock is not watchdog clock authority.

Target nominal watchdog period:

approximately `8 s`.

The accepted Gate 1 pair is prescaler `/256` (code `6`) and reload `1249`.
Hardware acceptance uses a broad reset-time window because LSI is not treated
as a precision time source.

## 3. Source layering

Expected Gate 1 paths:

```text
A include/drivers/iwdg.h
A src/drivers/iwdg.c
M src/kernel.c
```

Driver responsibilities:

- bounded LSI-ready/configuration sequencing;
- configure IWDG prescaler/reload;
- start IWDG;
- reload IWDG;
- expose only minimal register-level operations.

Accepted STM32F1 configuration sequence:

`START -> unlock -> PR/RLR -> wait PVU/RVU -> reload`

Waiting on PVU/RVU before IWDG start is forbidden by the accepted hardware repair.

Kernel policy responsibilities:

- boot reset-cause capture;
- watchdog start timing;
- production reload points;
- health telemetry;
- deliberate expiration diagnostic.

The driver must not know scheduler policy.

## 4. Start timing

Boot order becomes:

1. capture RCC reset flags;
2. clear hardware reset flags after capture;
3. existing clocks/GPIO/UART/I2C/OLED/fault/SysTick initialization;
4. existing scheduler init / task bind / priority / prepare;
5. enable debugger IWDG freeze behavior for development halts;
6. start IWDG;
7. enter normal cooperative `scheduler_start()`.

Starting IWDG before the task topology is ready is not permitted.

## 5. Reload policy

Permanent rule:

**No Handler-mode reload.**

Forbidden reload locations:

- `SysTick_Handler()`;
- `USART1_IRQHandler()`;
- SVC/PendSV/HardFault/fault handlers;
- generic IRQ paths;
- fatal fault loop;
- host MSP WFE idle loop.

Allowed production reload points:

### task0

After a confirmed console/runtime progress point such as completing an RX
wake/drain/command-processing cycle.

### task1

After a successful:

```text
scheduler_sleep_ms(500)
heartbeat transition
heartbeat count increment
```

then the task may reload IWDG.

C4.0 therefore implements a system-execution watchdog: continued healthy
production Thread/PSP progress keeps the watchdog alive.

C4.0 does not claim per-task quorum semantics. Existing task-specific telemetry
continues to diagnose task-local failures.

## 6. Fault semantics

`production_fail_closed()` must not reload IWDG.

Fatal exception paths must not reload IWDG.

If IWDG has started, a production fatal/non-progress state is therefore allowed
to terminate in a hardware watchdog reset.

This is intentional.

## 7. Reset-cause capture

At every boot:

1. read RCC reset-cause flags before clearing them;
2. store the snapshot in normal kernel state;
3. record whether the snapshot contains the IWDG reset flag;
4. clear hardware reset flags for the next reset cycle.

Existing `health` is extended rather than adding another safe command.

Required telemetry fields:

```text
WDOG_ACTIVE=
WDOG_RELOAD_COUNT=
RESET_FLAGS=0x........
IWDG_RESET=0x00000000|0x00000001
```

The existing leading `HEALTH TICK=... PC13=...` format remains intact so
retained parsers continue to work.

Safe command count remains exactly `20`.

## 8. Deliberate expiration proof

Add one destructive command:

`wdogtrip`

It is **not** part of the safe 20-command surface.

It is **not** one of the eight scheduler lifecycle diagnostics.

Behavior:

1. require active production watchdog;
2. print exact marker `WDOG_TRIP_ARMED`;
3. enter a task0 non-progress loop that never reloads, blocks, yields, or
   returns;
4. interrupts may continue, but no Handler path reloads IWDG;
5. because production is cooperative, task1 cannot execute its reload point;
6. IWDG must reset the MCU.

After reboot, `health` must report `IWDG_RESET=0x00000001`.

## 9. Debug behavior

Development/debug halt may freeze IWDG through the STM32 debug-control
facility so a breakpoint does not create a spurious production reset.

Normal runtime must not depend on debugger attachment.

## 10. Scheduler contract

C4.0 does not change:

- scheduler task count;
- task0 priority `128`;
- task1 priority `255`;
- cooperative production;
- SVC `0..4`;
- timed blocking;
- READY selector;
- PSP/MSP ownership;
- event wait/wake semantics.

No scheduler-core source edit is authorized.

## 11. OLED/UI contract

OLED/gfx/status-bar source and geometry remain frozen.

If exact OLED/gfx hashes remain unchanged and `uiruntime` passes hardware
acceptance, physical OLED disposition remains conditional N/A.

## 12. Explicit non-goals

C4.0 does not add:

- WWDG;
- generic timer callbacks;
- message queues;
- semaphores;
- mutexes;
- priority inheritance;
- a watchdog task;
- a third task slot;
- task priority changes;
- dynamic task lifecycle;
- a display task;
- UI changes;
- a new SVC;
- preemptive normal production;
- HAL, Arduino or FreeRTOS.

## 13. Gate sequence

1. Gate 0 — planning/docs synchronization.
2. Gate 1 — IWDG driver + kernel production integration.
3. Gate 2 — fresh GNU build validation.
4. Gate 3 — real hardware acceptance including deliberate watchdog reset.
5. Gate 4 — physical OLED conditional N/A if unchanged-source policy passes.
6. Gate 5 — docs/evidence finalization.
7. Gate 6 — local acceptance commit.
8. Gate 7 — ordinary non-force publication.

## 14. Acceptance record — 2026-09-15

C4.0 acceptance record — Gates 0–5 accepted on 2026-09-15

- boundary: `IWDG_LIVENESS_FOUNDATION_C4_0`;
- published parent remains `39ea5b3d1b72fca8d15e22e7544870ab0704c274` until Gate 6/7;
- accepted source candidate tree: `f8e879f815051d300eec728f2afe03c39222ca47`;
- accepted firmware: `build\iwdg_liveness_foundation_v2\os.bin`,
  `25192` bytes,
  SHA-256 `4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D`;
- STM32F103 IWDG uses LSI, prescaler `/256` (code `6`), reload `1249`,
  nominal approximately `8 s` at 40 kHz;
- repaired hardware sequence is `START -> unlock -> PR/RLR -> wait PVU/RVU -> reload`;
- normal watchdog reload count progressed `39 -> 50`;
- deliberate `wdogtrip` armed exactly and produced a real reboot after `7294 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- post-reset heartbeat delta `3`;
- safe production surface `20/20`;
- scheduler BUSY diagnostics `8/8`;
- retained UART race regression `4 x 32 = 128/128 PONG`;
- fixed-priority self-test `0x0000003F`, task0 `128`, task1 `255`;
- timed blocking remains `4/4`;
- final production stack `604 used / 420 margin`;
- heartbeat stack `80 used / 432 margin`;
- MSP `348 used / 1636 margin`;
- RX drop/error/depth `0/0/0`;
- final Flash readback exactly matches the accepted candidate;
- OLED/gfx/status-bar hashes remain unchanged and automated UI regression passes;
- Gate 4:
  `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 5 documentation/evidence finalization is accepted;
- next gate: **C4.0 Gate 6 — local acceptance commit**.
