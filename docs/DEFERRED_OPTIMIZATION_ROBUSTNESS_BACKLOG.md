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

## 7. Roadmap placement

These deferred items do not insert speculative boundaries before `APPLICATION_RUNTIME_FOUNDATION`.

Current intended order remains:

1. `APPLICATION_RUNTIME_FOUNDATION`;
2. `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` — mandatory architecture cleanup after measured god-module concentration in `src/kernel.c`;
3. `USB_MANAGEMENT_DEVICE_FOUNDATION`;
4. `HOST_CONTROL_APPLICATION_FOUNDATION`;
5. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` when a real consumer exists;
6. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`;
7. networking/service extensions.

Optimization rule: remove unnecessary work first, measure next, add complexity only against an observed bottleneck.
