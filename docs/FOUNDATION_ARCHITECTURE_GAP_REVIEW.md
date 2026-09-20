# Deus OS — Foundation Architecture Gap Review

Status: **HISTORICAL NORMATIVE GAP REVIEW — APPLICATION EVENT + SYSTEM IDENTITY GAPS RESOLVED; PERSISTENCE PREREQUISITE CARRIED FORWARD TO `docs/CURRENT_STATE.md`**

Published source baseline reviewed:

- commit `2fde9025a51021511e73a76b561f7983ca655e2f`;
- tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- accepted BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`.

This review asks one question: **is the lower kernel/foundation map complete enough that future product direction can change without forcing a rewrite of the accepted substrate?**

Original conclusion: **yes, with a small set of additional contracts that must be explicitly retained in the roadmap.** No missing kernel mechanism blocked the then-current `BOOT_DESKTOP_UI_FOUNDATION` -> `APPLICATION_RUNTIME_FOUNDATION` direction. Since this review, the semantic application-event contract was accepted in `APPLICATION_RUNTIME_FOUNDATION` and the system identity/capability gap was closed by `HOST_CONTROL_APPLICATION_FOUNDATION` through `sysinfo=0x0024`. The active unresolved prerequisite for the next boundary is bounded recoverable persistence before any Flash-resident configuration/assets.

## 1. Verified foundation already present

Published source/docs already provide or have accepted:

- Cortex-M3 startup, vector table and exception ownership;
- HardFault/MemManage/BusFault/UsageFault capture path;
- 72 MHz clock and 1 kHz monotonic kernel time;
- PSP task execution and MSP scheduler-host/fault ownership;
- cooperative production scheduler with accepted priority semantics;
- explicit task states including `BLOCKED`;
- scheduler event wait/wake with pending-event lost-wakeup protection;
- timeout-backed wait and `scheduler_sleep_ms()`;
- WFE idle when no runnable task exists;
- static task stacks with canary/high-water accounting;
- scheduler-internal interrupt critical sections using PRIMASK save/restore;
- production IWDG liveness with Thread/PSP reload ownership;
- UART IRQ/ring/event transport;
- I2C and SSD1306 display stack;
- native USB Device + CDC ACM;
- transport-neutral 32-method command service;
- binary framed RPC v1 with stable RPC IDs, request correlation, CRC, bounded arguments and explicit destructive authorization.

Therefore the next product/application work does not require a scheduler rewrite, a heap, a filesystem, dynamic task creation or a native executable loader.

## 2. Foundation matrix

| Foundation concern | Current state | Normative disposition |
| --- | --- | --- |
| Scheduler, blocking, priorities, task stacks | Implemented and hardware accepted | Keep as current substrate; extend only for measured consumers. |
| Timers | Generic timer callbacks not implemented; timed blocking exists | Correctly deferred. Add a timer service only when a real app/service requires callbacks/deadlines beyond current sleep/wait primitives. |
| Message queues | Not implemented | Correctly deferred until a real producer/consumer payload boundary exists. |
| Synchronization primitives | No public mutex/semaphore layer; scheduler has internal IRQ critical sections | Correctly deferred. Do not expose raw scheduler critical sections as application synchronization. Add bounded primitives only with a concrete shared-ownership consumer. |
| Runtime statistics | Partial counters/high-water diagnostics exist | Keep later observability slice; do not block app/runtime work. |
| System identity / capabilities | Resolved by published Host Control foundation: HELLO remains transport/protocol negotiation and `sysinfo=0x0024` exposes stable OS/platform/architecture/source-tree/service/runtime/capability identity | **Resolved for v1.** Stable physical unit identity/MCU UID remains intentionally absent until a real multi-unit/privacy requirement exists. |
| Application event/service model | Resolved by published `APPLICATION_RUNTIME_FOUNDATION`: bounded pointer-free semantic application events are separate from scheduler wake bits | **Resolved.** Scheduler wake bits remain kernel notification state, not public application ABI. |
| Persistent settings/state | Persistent-storage policy is planned for asset/config transfer | Expand into a versioned bounded persistence contract before first Flash-resident settings/packages: schema/version, integrity, atomic commit, recovery and wear budget. |
| Crash/reset retention | Fault capture exists; current `fault_record` is normal BSS and is cleared on reset | Missing later observability contract. Add retained crash/boot diagnostics so post-reset tooling can recover the previous failure/reset reason. |
| Structured logs/telemetry | Existing command diagnostics and counters are ad hoc but useful | Add a later bounded structured observability model for host TUI/Control Panel; do not replace emergency UART/fault paths. |
| Security/trust | CRC and destructive flag exist | CRC is integrity against corruption, not authentication. Before network remote control or firmware update, define authorization/authenticity/trust policy. |
| Controlled reboot/update handoff | Deliberate IWDG reset proof exists | Normal controlled reboot/update transition belongs to the firmware-update/bootloader boundary. |
| Portability | Current implementation is STM32F103/direct-register specific | Freeze layering rule now: architecture/platform/drivers must remain separable from kernel/services/apps/UI/protocol semantics. Do not introduce a generic HAL merely for hypothetical ports. |
| Power management / RTC / DMA / filesystem / heap / MPU | Not generally implemented | Not missing for current scope. Add only when a real product requirement justifies the complexity and resource cost. |

## 3. Application events must not reuse scheduler event bits as public ABI

The accepted scheduler event mechanism has a narrow kernel meaning:

```text
IRQ / producer
    -> signal event bit
    -> wake blocked scheduler task
    -> task re-checks authoritative FIFO/condition
```

This remains an internal scheduling/wake primitive. It must not become the application/service ABI.

This requirement was satisfied by the published `APPLICATION_RUNTIME_FOUNDATION`, which defines a separate bounded semantic event contract rather than exposing scheduler wake bits. The original vocabulary considered concepts such as:

```text
APP_EVENT_TIMER
APP_EVENT_INPUT
APP_EVENT_USB_CHANGED
APP_EVENT_SERVICE_CHANGED
APP_EVENT_NOTIFICATION
APP_EVENT_NETWORK_CHANGED
APP_EVENT_SERVICE_REPLY
```

Exact numeric IDs and payload representation are deferred to that boundary, but the following requirements are already normative:

- explicit stable event type IDs;
- explicit source/owner semantics;
- bounded payload size with no heap requirement;
- no pointer lifetime ambiguity across deferred dispatch;
- no direct exposure of scheduler bit positions as application IDs;
- backpressure/drop/coalescing policy must be explicit if queued delivery is introduced;
- ISR paths may publish/wake bounded state but must not run application policy in Handler mode.

A message queue is therefore not required merely to define application events. Initial dispatch may remain serialized in task0 until a real producer/consumer topology proves a queue is necessary.

## 4. System identity and capability discovery

At the time of this review, binary RPC v1 HELLO exposed only protocol/service bounds and protocol capability flags. That gap is now resolved by the published `HOST_CONTROL_APPLICATION_FOUNDATION`: HELLO remains transport/protocol negotiation, while `sysinfo=0x0024` exposes stable OS/platform/architecture/source-tree/service/runtime/capability identity. Stable physical unit identity and MCU UID remain intentionally outside v1 until a real multi-unit/privacy requirement exists.

The accepted v1 identity contract follows the original stable-meaning rule and includes:

- Deus OS firmware semantic/build version;
- build/source identity suitable for diagnostics;
- platform/architecture identifier, e.g. STM32F103/Cortex-M3 port;
- protocol/service ABI versions;
- available service/application capability bits or discoverable descriptors;
- optional board/device identity when a real use case requires distinguishing multiple units;
- optional MCU unique ID exposure only after privacy/stability semantics are explicitly chosen.

Do not overload USB VID/PID or protocol capability bits as the complete OS identity model.

## 5. Persistence contract

Persistent configuration is not equivalent to a filesystem. The first persistence layer should remain small, bounded and recoverable.

Before `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` writes persistent target state, freeze at minimum:

- record/schema version;
- exact maximum size and Flash ownership region;
- CRC/integrity validation;
- atomic commit strategy so reset/power loss cannot leave ambiguous accepted state;
- previous/default configuration recovery policy;
- erase/program alignment rules;
- Flash erase-cycle/wear budget;
- migration/rejection policy for incompatible versions;
- separation between configuration data and executable firmware/update state.

A general filesystem remains deferred until a concrete storage/product requirement proves it useful.

## 6. Crash, reset and observability contract

Current fault capture is useful during the active fault state, but normal `.bss` initialization clears the current `fault_record` at the next boot. The project also already captures reset flags early enough to identify IWDG reset state.

A later crash/observability boundary should preserve a compact previous-boot record across reset without turning persistent diagnostics into an unbounded log. Candidate retained fields:

- record magic/version/integrity;
- boot counter or boot sequence;
- previous reset-cause flags;
- previous exception/fault class;
- selected stacked PC/LR/xPSR and CFSR/HFSR values when valid;
- watchdog-reset marker;
- clean/controlled reboot marker when that concept exists;
- consumed/cleared semantics so stale crash data is not reported as current.

Storage mechanism (`.noinit`, backup registers/domain, reserved Flash, or another bounded approach) must be chosen in its own implementation boundary after exact reset/power-retention semantics are reviewed.

Structured observability should later unify selected counters/events for host presentation, but emergency UART and deterministic fault behavior remain independent recovery paths.

## 7. Security and trust boundary

The current CRC-16 protects frame integrity against accidental corruption. The `ALLOW_DESTRUCTIVE` flag expresses host intent. Neither provides peer authentication or firmware authenticity.

Before either of the following is accepted:

- network-accessible remote control that can mutate device state;
- firmware update/bootloader that writes executable Flash;

there must be an explicit security/trust design covering, as applicable:

- authorization model for destructive/control operations;
- update image authenticity and integrity;
- anti-confusion/version/target checks;
- recovery behavior after rejected or interrupted update;
- credential/key ownership only if a concrete threat model requires credentials;
- clear distinction between transport CRC, authentication and signed firmware trust.

Do not add cryptography to the current STM32 foundation speculatively. The contract is mandatory before exposing a trust-sensitive boundary, not before boot/desktop/application work.

## 8. Portability boundary

Future ports to a different STM32, Cortex-M class, ARM64 or x86_64 must not require the application/service/UI model to know STM32F103 register addresses.

Current layering policy is therefore:

```text
arch / platform / low-level drivers
              |
          kernel core
              |
      services / applications
              |
         UI / protocol
```

Portability rules:

- startup, exception mechanics, context switching and direct peripheral registers are architecture/platform-specific;
- kernel contracts should avoid leaking STM32 register details upward;
- application/service IDs, lifecycle, semantic events and host-visible protocol semantics should be platform-neutral where practical;
- driver ownership APIs may remain intentionally small instead of introducing a heavyweight universal HAL;
- portability refactoring is justified when a second real target exists, not before.

## 9. What is explicitly not missing now

The following are not prerequisites for the current foundation and must not be implemented only to make the OS look more complete:

- general-purpose heap;
- filesystem;
- ELF/native application loader;
- dynamic task creation;
- POSIX process model;
- userspace/kernelspace split;
- virtual memory;
- MPU isolation;
- direct TCP/IP stack on STM32;
- generic DMA framework;
- RTC/wall clock service;
- deep/tickless power-management framework;
- multicore support.

Any of these may become legitimate later when a real target/application supplies the requirement.

## 10. Ordering constraints added by this review

The existing roadmap remains valid. This review adds requirements inside later boundaries rather than inserting speculative kernel implementation before the desktop.

```text
OS_APPLICATION_AND_UI_MODEL_FOUNDATION
 -> BOOT_DESKTOP_UI_FOUNDATION
 -> APPLICATION_RUNTIME_FOUNDATION
      includes semantic application event/service contract
 -> KERNEL_COMPOSITION_ROOT_DECOMPOSITION
      behavior-preserving cleanup required after measured src/kernel.c integration concentration
 -> USB_MANAGEMENT_DEVICE_FOUNDATION
      production WinUSB management transport reusing accepted binary RPC semantics
 -> HOST_CONTROL_APPLICATION_FOUNDATION
      system identity/capability discovery resolved by published `sysinfo=0x0024`
 -> ASSET_CONFIGURATION_TRANSFER_FOUNDATION
      requires bounded persistence contract before Flash-resident settings/packages
 -> FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION
      requires controlled reboot/update handoff + security/trust contract
 -> networking/service extensions
      require authorization/trust review before remote mutation
```

`CRASH_OBSERVABILITY` / structured telemetry remains a later independently accepted service boundary and may be scheduled when host tooling or a real product first benefits from it. It does not block boot/desktop or the initial static application runtime.

Generic timers, queues, synchronization and runtime statistics remain **consumer-driven Phase 4 work**. They are not to be implemented merely because they are traditional RTOS features.

## 11. Foundation completeness conclusion

For the present STM32F103 stage, the lower architecture is complete enough to proceed without choosing the final product use case.

The governing rule is:

```text
real application/service need
        -> explicit service contract
        -> smallest required kernel primitive
```

not:

```text
traditional RTOS feature list
        -> speculative primitive
        -> search for a future consumer
```

This keeps the STM32F103 implementation small while preserving a path toward richer product modules and later ports to more capable hardware.
