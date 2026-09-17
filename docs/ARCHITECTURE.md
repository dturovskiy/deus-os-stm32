# Architecture

Status: **`BOOT_DESKTOP_UI_FOUNDATION` Gates 0–5 accepted; Gate 6 local acceptance commit current**

## Current foundation completeness constraints

Canonical review: `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

The published kernel/transport substrate is sufficient to proceed to boot/desktop and the static application runtime without speculative kernel expansion. The following rules are now part of the architecture:

- scheduler event bits are internal wake notifications and must not become stable application event IDs;
- `APPLICATION_RUNTIME_FOUNDATION` owns a bounded semantic application event/service contract;
- generic timers, queues, synchronization primitives and runtime statistics remain consumer-driven extensions; scheduler-internal PRIMASK save/restore is not a public mutex/semaphore API;
- host-facing system identity must eventually distinguish firmware/build/platform/service/application capabilities from USB identity and protocol capability flags;
- after `APPLICATION_RUNTIME_FOUNDATION`, `USB_MANAGEMENT_DEVICE_FOUNDATION` owns the production Windows USB profile: vendor-specific WinUSB management transport with explicit product identity/device-interface GUID, reusing the accepted binary RPC above transport; CDC/COM must not remain the primary production host API;
- persistent target state requires versioning, integrity, atomic commit/recovery and Flash wear policy before acceptance;
- current `fault_record` is normal `.bss` runtime state and is cleared by reset; bounded previous-boot crash/reset retention is later observability work;
- CRC-16 and explicit destructive intent are not authentication; trust-sensitive network mutation and firmware update require a separate security/authenticity contract;
- controlled reboot/update handoff belongs to the update/bootloader boundary;
- startup/context-switch/register drivers remain arch/platform-specific while kernel/services/apps/UI/protocol semantics should avoid leaking STM32 register details upward;
- heap, filesystem, generic DMA framework, RTC, MPU isolation and broad power-management facilities are not current prerequisites.

## Current boundary — Boot / desktop UI foundation

Canonical design: `docs/BOOT_DESKTOP_UI_PLAN.md`.
Canonical acceptance: `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.

The first UI implementation remains deliberately below the application runtime. It adds only `BOOT_SPLASH -> DESKTOP_HOME`, preserving the two-task topology. Bootstrap may perform the one initial splash render before scheduler start; after that, task0 / Thread-PSP is the single normal OLED writer. Task0 uses the accepted timed wait with a 250 ms timeout to service UI state while retaining immediate UART/CDC event wakes. The minimum splash dwell is 1000 ms but never delays scheduler/IWDG startup.

The status bar keeps the accepted 128x9 geometry. SYSTEM reflects scheduler/task/watchdog readiness, USB reflects actual CDC configured state for this boundary, NETWORK remains inactive until a real network service exists, and the right field is monotonic uptime `HH:MM` saturating at `99:59`. Polling does not imply periodic redraw: the framebuffer/panel is updated only when lifecycle state, an indicator, displayed minute, or explicit `uiruntime` restore changes visible state. Steady-state redraw keeps an initialized OLED powered on; SSD1306 reinitialization/display-off is reserved for initial bring-up or real transfer recovery. This behavior is build-, hardware- and physically accepted on candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`, with `PHYSICAL_OLED=PASS`.

The next independent UI optimization is `OLED_DIRTY_REGION_OPTIMIZATION`: retain one 512-byte framebuffer, make pixel writes dirty only when byte state actually changes, track bounded horizontal dirty spans per SSD1306 page, and issue page/column windows only for those spans. A second 512-byte shadow framebuffer is not required. `APPLICATION_VIEW` remains deferred to `APPLICATION_RUNTIME_FOUNDATION`; production WinUSB management USB remains a later `USB_MANAGEMENT_DEVICE_FOUNDATION`.

## C4.0 accepted IWDG liveness record

C4.0 acceptance record — Gates 0–7 accepted and published on 2026-09-15

- boundary: `IWDG_LIVENESS_FOUNDATION_C4_0`;
- published commit `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`, direct parent `39ea5b3d1b72fca8d15e22e7544870ab0704c274`;
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
- C4.0 publication is complete; next boundary: **`NATIVE_USB_DEVICE_CORE_FOUNDATION`**.


## Native USB Device core foundation — PUBLISHED 2026-09-16

Boundary:
`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Accepted candidate:

- source tree `4b798382ef843ef8f488115624d48c0cd1506c75`;
- binary `43812` bytes;
- SHA-256 `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`.

Accepted USB architecture and proof:

- direct-register STM32F103 USB FS Device on PA11/PA12;
- system clock remains 72 MHz; `USBPRE=0` derives 48 MHz USB clock;
- IRQ20 `USB_LP_CAN1_RX0` owns USB peripheral servicing;
- BTABLE local `0x000`, EP0 TX `0x040`, EP0 RX `0x080`, MPS `64`;
- exact device/config descriptors are returned through EP0 control transfers;
- delayed address programming occurs after SET_ADDRESS status IN;
- Windows addressed the device (`20`, then `21` after reconnect);
- configuration `0` is accepted for the vendor-specific/no-client-driver core foundation;
- three host enumeration/control-transfer proofs completed, including physical reconnect and post-IWDG recovery;
- development identity `1209:000A` is private-test-only;
- no CDC ACM/data endpoints, new task, SVC, IPC, generic timer subsystem, runtime-statistics subsystem or OLED edit;
- retained scheduler/UART/IWDG regressions passed and final Flash identity is exact;
- OLED Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

The roadmap transport sequence is authoritative:

```text
minimal USB Device core
    -> CDC ACM console
    -> shell/RPC
    -> binary transport
    -> host control/update tooling
```

This boundary is the **core only**.

Layering:

```text
future CDC / transport-neutral services
                |
        USB device core policy
                |
      EP0 control-transfer state
                |
        endpoint / PMA ownership
                |
 STM32F103 USB FS peripheral / IRQ
                |
        PA11 DM / PA12 DP
```

Rules:

- direct registers; no HAL/Arduino/FreeRTOS;
- accepted 72 MHz system clock remains unchanged;
- USB receives a valid 48 MHz clock derived from the accepted clock tree;
- USB device IRQ publishes hardware state into the USB core; it does not own
  unrelated scheduler/application policy;
- PMA/BTABLE and endpoint ownership are explicit and bounded;
- endpoint 0 handles standard control requests required for enumeration;
- USB descriptor identity is centralized;
- do not embed an arbitrary third-party VID/PID;
- CDC ACM was deliberately excluded from this historical core boundary and was later accepted/published as its own boundary;
- UART remains emergency diagnostics;
- ST-LINK remains recovery/debug;
- IWDG reload ownership stays Thread/PSP-only and is not moved into USB IRQ;
- OLED/gfx/status-bar remain unchanged.

Power rule:
when micro-USB VBUS powers the board, ST-LINK 3.3 V supply must be disconnected.

## Accepted architecture — USB CDC ACM console foundation — PUBLISHED `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`

Boundary: `USB_CDC_ACM_CONSOLE_FOUNDATION`.

The published USB core remains the hardware/protocol substrate. CDC adds one class layer and one additional transport into the existing production console task; it does not create a second command language or a third production task.

```text
Windows usbser / CDC COM
          |
     CDC ACM class
          |
  EP2 OUT / EP3 IN
          |
 bounded RX/TX rings
          |
          +------> task0 console/runtime <------ UART RX ring
                         |
                  shared command execution
                         |
             response to originating transport
```

Descriptor topology:

```text
VID:PID 1209:000B (private test only)
device class/subclass 02/02
interface 0 CDC Control + EP1 0x81 interrupt IN
interface 1 CDC Data    + EP2 0x02 bulk OUT + EP3 0x83 bulk IN
```

The accepted CDC implementation adds the bounded EP0 OUT data stage required for `SET_LINE_CODING`, plus `GET_LINE_CODING` and `SET_CONTROL_LINE_STATE`. CDC line coding is virtual USB state and never reconfigures USART1.

Accepted PMA ownership extends the published map to EP1 `0x0C0`, EP2 `0x100`, and EP3 `0x140`, all statically bounded below local `0x200`.

Production ownership remains two cooperative PSP tasks. Task0 owns both the USB RX event and UART RX event. USB IRQ owns bounded endpoint/PMA service and state publication only; Thread/PSP owns parsing, command policy, output policy, and watchdog liveness. UART remains an independent emergency console.

Parser state is per transport so UART and CDC streams may interleave without corrupting a shared partial command. Command semantics remain shared and a response is routed only to its originating transport.

Canonical design: `docs/USB_CDC_ACM_CONSOLE_PLAN.md`.
Canonical acceptance: `docs/USB_CDC_ACM_CONSOLE_ACCEPTANCE_PLAN.md`.

## Accepted architecture — transport-neutral shell/RPC foundation — PUBLISHED `0c33304d2db86e54d715905393f49147bb6dd2ea`

Boundary: `SHELL_RPC_FOUNDATION`.

The accepted UART and USB CDC byte transports remain unchanged. This boundary separates command semantics from text framing and physical transport identity so future binary RPC can reuse exactly the same command service.

```text
UART RX ring -> text shell parser ---+
                                     |
USB CDC RX -> text shell parser -----+--> static command registry/service
                                     |          |
future binary RPC adapter -----------+          +--> handler
                                                +--> generic response writer
```

The Gate 1 candidate now replaces the old string-chain/global transport coupling with `command_service_parse_line()` plus `command_service_execute()`. The execution context is explicit at every normal command-dispatch boundary and contains a generic response writer, opaque writer context, source RX-event mask, and latched writer-failure state. There is no global active UART/CDC selector in the normal command path.

Implemented service invariants:

- no heap; static registry and fixed bounds only;
- text line capacity remains `32` bytes;
- maximum `4` argument tokens;
- method lookup and argument validation are transport-neutral;
- semantic service statuses are `OK`, `NOT_FOUND`, `BAD_ARGS`, `BUSY`, `INTERNAL_ERROR`;
- `OK` denotes completed service dispatch, while method-specific operational success/failure remains encoded by established payload tokens such as `OLED_*_OK/ERR` and `SCHED_*_OK/ERR`;
- response-writer failure is latched in the execution context and promoted to `INTERNAL_ERROR`;
- legacy text responses remain compatible (`ERR`, `SCHED_DIAG_BUSY`, existing success/error tokens);
- `help` and `rpcinfo` are the only new foundation introspection methods;
- `schedtimed` consumes the source RX-event mask from the explicit originating context;
- UART-specific boot/fatal/recovery output remains intentionally outside the command service as an emergency path;
- task0 remains the only normal command executor in Thread/PSP;
- UART/USB IRQs remain bounded data/event publishers only;
- no new task, SVC, scheduler state, IPC, timer subsystem, heap, USB class change, or OLED change;
- binary wire framing and public numeric RPC method IDs remain the next independent transport boundary.

Canonical design: `docs/SHELL_RPC_FOUNDATION_PLAN.md`.
Canonical acceptance: `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`.

## Accepted architecture — binary framed transport foundation — PUBLISHED `2fde9025a51021511e73a76b561f7983ca655e2f`

Boundary: `BINARY_FRAMED_TRANSPORT_FOUNDATION`.

The binary transport is an adapter above the published USB CDC byte stream and below the same command service already used by the text shell. It does not create a second command language.

```text
Windows/Linux host
        |
     USB CDC
        |
text/binary demultiplexer
   |              |
text parser    binary frame parser
   |              |
   +-------> command service <-------+
                  |
               handlers
                  |
         originating response adapter
```

Protocol v1 architecture:

- binary sync magic is `A5 5A`; a complete binary frame is isolated from the text parser;
- existing partial CDC text-line state survives an intervening binary frame;
- all multi-byte wire fields are little-endian;
- CRC-16/CCITT-FALSE protects version/type/flags/reserved/request-ID/length/payload;
- stable public 16-bit method IDs are explicit protocol ABI and are never inferred from the internal C dispatch enum;
- host request ID `0` is reserved; requests `1..65535` are echoed by responses;
- request argument adaptation is bounded to four arguments of at most 31 bytes each;
- command output is streamed as bounded `RPC_DATA` frames rather than buffered wholesale;
- 48 command-output bytes per data chunk produce an exact 64-byte maximum `RPC_DATA` wire frame;
- returning commands finish with structured `RPC_END` status while preserving existing method-specific diagnostic payload bytes;
- destructive methods require an explicit request flag; CRC integrity alone is not destructive authorization;
- binary TX requires a nonblocking all-or-none bounded CDC span enqueue so a ring-full condition cannot publish a partial frame;
- USB IRQ remains endpoint/PMA/ring/event ownership only; parsing, CRC, RPC dispatch and response framing remain task0 / Thread-PSP work;
- UART remains the independent text emergency console;
- no heap, new task, SVC, IPC/timer subsystem, USB descriptor/PMA redesign or OLED change is introduced by the foundation.

Canonical wire contract: `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`.
Canonical design: `docs/BINARY_FRAMED_TRANSPORT_PLAN.md`.
Canonical acceptance: `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`.

Accepted binary transport proof — 2026-09-16:

- tested source candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- fresh `16 C + 1 ASM` `-Wall -Wextra -Werror` build, exact `32` public RPC IDs and CRC KAT accepted;
- hardware: binary safe surface `21/21`, scheduler BUSY `8/8`, binary pressure `128/128` unique IDs, retained CDC/UART pressure, physical reconnect, destructive binary IWDG proof, automatic post-reset text+binary recovery and exact final Flash readback all PASS;
- frozen OLED/gfx/status-bar remained exact; Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 5 documentation/evidence finalization is accepted; no source/build/flash/commit/push occurred in Gate 5.

Gate 6 local acceptance commit and Gate 7 ordinary non-force publication are accepted. Published commit/tree: `2fde9025a51021511e73a76b561f7983ca655e2f` / `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`.

## Current architecture — Deus OS product/application/UI model foundation

Boundary: `OS_APPLICATION_AND_UI_MODEL_FOUNDATION`.

This is a documentation-only architecture freeze above the accepted kernel/driver/command/RPC substrate. Deus OS is defined as an independently operating deterministic embedded device runtime; a future PC Control Panel is a management plane rather than a runtime dependency.

The initial application model is static and event-driven: applications are firmware-linked modules with explicit stable IDs and lifecycle state, not arbitrary uploaded ARM executables. The accepted two-task production topology remains authoritative; application dispatch initially belongs to task0 / Thread-PSP and does not imply one scheduler task per application.

The local UI lifecycle is explicitly separated into bootstrap splash, desktop/home and application view. The current decorative status bar gains real v1 semantics: SYSTEM, USB and NETWORK indicators plus uptime `HH:MM`. NETWORK must remain offline/unavailable until a network service is actually present; the UI must not fabricate connectivity or wall-clock time.

Firmware owns hardware, safety, application lifecycle and the local UI. The future Deus OS Control Panel owns discovery, protocol negotiation, human-facing management UI, diagnostics presentation, host plugins and later transfer/update orchestration.

Canonical plan: `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`.
Canonical acceptance: `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`.

Exact implementation order after this docs-only boundary:

```text
BOOT_DESKTOP_UI_FOUNDATION
 -> APPLICATION_RUNTIME_FOUNDATION
 -> HOST_CONTROL_APPLICATION_FOUNDATION
 -> ASSET_CONFIGURATION_TRANSFER_FOUNDATION
 -> FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION
 -> networking/service extensions
```

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


## C4.0 planned IWDG production liveness architecture

C4.0 adds a hardware-independent-clock watchdog without changing scheduler
policy or task topology.

```text
                      +---------------------+
task0 progress ------>|                     |
                      | liveness reload     |----> IWDG reload key
task1 heartbeat ----->| policy (Thread/PSP) |
                      +---------------------+
                                |
                                X  never from Handler mode

SysTick -------------------------------> kernel time / scheduler tick only
USART1 IRQ ----------------------------> RX ring / event only
fault path ----------------------------> no watchdog reload
wdogtrip task0 loop -------------------> no watchdog reload -> IWDG reset
```

The watchdog uses STM32F103 IWDG and its independent LSI clock domain. It does
not use `kernel_ticks` as watchdog clock authority and it does not create a
kernel timer callback.

Normal reload policy is intentionally simple for the first watchdog slice:

- task0 may reload after a confirmed production console/runtime progress point;
- task1 may reload after a successful 500 ms sleep and heartbeat transition;
- no interrupt, exception, SysTick, fault path, or idle loop reloads IWDG.

This creates a system-execution watchdog. If production Thread/PSP execution
stops making progress, IWDG is allowed to expire.

A failure of one task while another continues is still diagnosed by existing
task-specific production telemetry; C4.0 does not claim per-task quorum
watchdog semantics.

Boot/reset ownership:

1. capture RCC reset flags before clearing them;
2. retain the captured flags in normal initialized kernel state;
3. clear hardware reset flags for the next reset cycle;
4. expose reset evidence through existing `health`;
5. start IWDG only after production tasks are configured and immediately before
   steady-state scheduler start.

A destructive `wdogtrip` diagnostic intentionally parks task0 without further
reload. In cooperative production this prevents task1 from running, so the
hardware watchdog must reset the MCU. After reboot, `health` must report that
the captured reset cause contains the IWDG reset flag.

The existing 20-command safe surface is unchanged. `wdogtrip` is destructive
and outside that safe set.

OLED/status-bar ownership and geometry remain frozen.
