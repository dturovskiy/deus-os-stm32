# Deus OS — Deferred optimization, robustness and diagnostics backlog

Status: **CANONICAL DEFERRED POLICY — REVIEW BY MEASUREMENT/TRIGGER, NOT BY SPECULATION**

This document consolidates deferred engineering work identified during `OLED_DIRTY_REGION_OPTIMIZATION`. None of the items below authorizes immediate implementation by itself. Each item requires a real consumer, measurable pressure, or a dedicated independently accepted boundary.

## 1. Optimization triggers

### I2C async / IRQ / DMA

Keep the current synchronous bounded I2C path while dirty-region updates keep transfer cost low. Revisit an IRQ/DMA path only if measured blocking I2C latency or bus occupancy becomes material after additional UI/sensor consumers. Do not introduce a generic DMA framework solely because the peripheral supports DMA.

### Scheduler ready-set scaling

Keep the current bounded O(N) ready scan while production task count remains small; the accepted production topology currently has two tasks. Revisit bitmap/priority-set structures when task count grows materially (roughly 8–16+ is a practical review trigger) and measured scheduler CPU/latency overhead justifies the extra complexity.

### CRC acceleration

Keep bitwise CRC-16/CCITT-FALSE for low-rate control RPC. Consider table-driven or hardware-assisted CRC only if asset/configuration/update streaming benchmarks show CRC CPU cost is material enough to justify extra Flash/data/code.

### WinUSB / streaming copy policy

Future `USB_MANAGEMENT_DEVICE_FOUNDATION` and transfer boundaries should prefer bounded batches, minimal-copy flow and explicit backpressure. Avoid whole-payload duplication in SRAM. The accepted framed RPC semantics remain above transport.

### Asset / configuration / update streaming

Use bounded chunks, explicit backpressure and incremental validation/storage. Do not buffer complete files/images/packages in SRAM. Minimize copies between transport, validation and storage/update consumers.

### Tickless / low-power

Defer tickless idle and broader low-power work until there is a concrete power requirement and application timing constraints are known. Keep the 1 kHz SysTick deterministic baseline meanwhile.

### Continuous resource budgets

Continue measuring Flash, static SRAM and per-task stack high-water at acceptance gates. Prefer removing unnecessary work/data before adding optimization complexity; do not micro-optimize without a measured bottleneck.

## 2. Robustness and latent-bug verification

As boundaries gain long-lived state or transport/storage complexity, add targeted evidence rather than speculative kernel mechanisms:

- soak/stress runs for long-lived runtime paths;
- I2C NACK/stuck-busy and partial-transfer fault injection;
- transport disconnect/reconnect and reset/recovery fault injection;
- wraparound/boundary tests for time, counters, request IDs and ring arithmetic;
- host-side fuzzing for binary-frame lengths, flags, CRC and malformed sequencing;
- power-loss/interrupted-write recovery once persistence/filesystems/update exist;
- race/order tests around IRQ publication and task wake/block/timeout coincidence.

## 3. Structured observability policy

Normal production logging should remain lightweight and bounded. Preferred future design:

- a static binary event-trace ring, no heap;
- compact records: monotonic tick/timestamp + event ID + bounded numeric arguments;
- no `printf` or string formatting in the event-write hot path;
- nonblocking writes with an explicit overflow/drop counter;
- on-demand read/drain through the management protocol;
- only a bounded critical crash/reset snapshot retained across reboot through a `.noinit`/backup-style record;
- no continuous ordinary logging to internal Flash because of latency and wear.

Existing UART/USB command output, emergency UART, fault state, RX drop/error/high-water counters, scheduler stack high-water and watchdog diagnostics remain engineering observability mechanisms.

## 4. Test / diagnostic build-profile policy

Do not assume every acceptance/reference self-test must live permanently in the production image. Classify tests into three groups:

1. host/build-time tests — never linked into target firmware;
2. acceptance diagnostics — target/hardware verification, eligible for exclusion from a later production profile;
3. production health diagnostics — small bounded mechanisms retained in shipping firmware.

Heavy reference/self-tests should migrate toward host/build-time or acceptance-only profiles when a stable production build profile exists. A production binary must receive its own final hardware smoke/acceptance; do not fully test one image and ship different unverified bytes. Continue GCC stack-usage analysis for target-resident diagnostics and enforce explicit stack ceilings.

## 5. External storage and memory expansion policy

Treat nonvolatile storage separately from RAM.

Viable STM32F103-class directions:

- SPI NOR Flash for configuration, assets and update staging;
- microSD over SPI for large removable storage;
- FRAM/EEPROM for small frequently written state where appropriate;
- introduce a bounded block-device abstraction before a filesystem;
- if removable-media files become a real requirement, read-only FAT32 is a reasonable first step before writable semantics and power-loss recovery are accepted.

The current STM32F103 USB path is device-only; consuming commodity USB flash drives requires USB Host capability through an external host controller or a future MCU/platform with USB OTG/Host.

Laptop DDR/DDR2/DDR3 SO-DIMM is not a practical direct expansion for STM32F103: the MCU has no DDR controller and cannot satisfy the required bus width, clocks, signaling, training or refresh. If more RAM becomes a real requirement, evaluate SPI/QSPI PSRAM for modest expansion or move to a future MCU with FMC/external SDRAM support. Hundreds of MB/GB imply a different platform class.

## 6. Repository EOL and operator-log hygiene

Repository line endings are policy, not a developer-machine preference. `.gitattributes` owns LF for source/docs/linker/assembly text so Windows `core.autocrlf` does not flood evidence with conversion warnings.

Known LF->CRLF Git warnings may be suppressed from operator-facing logs only when exact raw stderr is still preserved in evidence. Other stderr must remain visible. Perform normalization deliberately; do not mix unrelated EOL churn into firmware changes.

## 7. Post-decomposition architecture debt

The accepted `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` is a material ownership improvement, not a claim that `src/kernel.c` has reached its final composition-root form. The accepted implementation deliberately stopped before creating speculative abstractions or exposing root-private state merely to reduce line count.

The remaining architecture debt is deferred and trigger-driven:

### Composition-root convergence

`src/kernel.c` still owns more than final composition-root responsibilities. Future extraction candidates remain:

- transport-neutral command execution/domain dispatch that is still rooted in the historical `console_*` implementation;
- production task/liveness glue when its bounded state can move with a coherent owner instead of being exported through hidden `extern` state or a catch-all context;
- low-level RCC/GPIO/UART/I2C/register helpers when a natural platform/driver owner exists.

Do not reopen decomposition merely to reduce line count. Revisit this debt when a new feature would otherwise add another independent reason to change `src/kernel.c`, when host/management work would duplicate domain dispatch, or when low-level platform code blocks clean driver/service layering.

Historical `console_*` naming on transport-neutral dispatch is not itself a functional defect and is not sufficient reason for a rename-only boundary.

### System/service state must remain upstream of presentation

The current boot/desktop integration reconstructs `application_service_snapshot_t` from `boot_desktop_ui_snapshot_t`. This is behaviorally valid for the accepted single OLED presentation path, but it must not become the long-term dependency direction.

Before system state gains multiple presentation/management consumers, the intended direction is:

`system/service state -> bounded semantic service snapshot -> application runtime and presentation consumers`.

UI indicator state must not become the authoritative source of system health, USB state, network state or future host-visible service state. Revisit this before a second presentation target, richer host-control state, networking state, or other non-OLED consumer is introduced.

### Application runtime bridge presentation coupling

`application_runtime_bridge` currently owns both runtime/event integration state and adaptation of the application view into `oled_console_t`. This is acceptable while OLED is the only target presentation consumer and avoids a speculative renderer abstraction.

If a second renderer/presentation consumer appears, split runtime/service ownership from OLED-specific presentation adaptation instead of expanding the bridge into a generic UI god object.

### Scheduler diagnostic binding lifetime

`scheduler_diagnostics_execute_diagnostic()` borrows one binding for the synchronous diagnostic execution, including nested diagnostic scheduler tasks. The module-static binding pointer is therefore valid only under the current serialized/non-reentrant diagnostic contract.

Do not reuse this mechanism as a concurrent diagnostic service, retain the borrowed binding beyond the call, or allow unrelated concurrent diagnostic execution. If diagnostics become concurrent/long-lived, introduce an explicit bounded ownership model in a dedicated reviewed boundary.

### Application stop-failure semantics

The v1 built-in applications do not own independent peripherals/resources, so their stop callbacks are bounded and effectively trivial. Before an application may own a resource whose release can fail, freeze explicit fail-closed lifecycle semantics for a failed current-application `stop()`.

In particular, do not implicitly start a replacement application after an unresolved resource-release failure unless the ownership transition is proven safe. The policy must define resulting lifecycle state, fallback behavior and resource ownership before resource-owning applications are accepted.

### Anti-goals for all follow-up work

None of the debt above authorizes a speculative generic HAL, universal `kernel_context_t`, service locator, heap, dynamic allocation, generic queue/mutex/timer framework, new task, or framework-only refactor. Ownership must move only with a concrete reason-to-change and bounded state.

These items did **not** block the now-published `USB_MANAGEMENT_DEVICE_FOUNDATION`; that boundary reused the accepted command/binary-RPC domain and added only the explicitly frozen USB-management transport/runtime wiring. They likewise do not authorize speculative kernel expansion before the next `HOST_CONTROL_APPLICATION_FOUNDATION` boundary.

## 8. Roadmap placement

These deferred items do not change current roadmap ordering.

Current intended order remains:

1. `APPLICATION_RUNTIME_FOUNDATION`;
2. `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — accepted/published architecture cleanup after measured god-module concentration in `src/kernel.c`;
3. `USB_MANAGEMENT_DEVICE_FOUNDATION`;
4. `HOST_CONTROL_APPLICATION_FOUNDATION`;
5. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` when a real consumer exists;
6. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`;
7. networking/service extensions.

Optimization rule: remove unnecessary work first, measure next, add complexity only against an observed bottleneck.
