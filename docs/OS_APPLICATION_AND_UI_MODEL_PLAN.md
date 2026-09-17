# Deus OS — Product, Application and UI Model Foundation Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`**

Boundary ID:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

Published parent baseline:

- commit `2fde9025a51021511e73a76b561f7983ca655e2f`;
- tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- subject `feat: add binary framed transport foundation`;
- clean local/remote `main`, ahead/behind `0/0`;
- accepted firmware BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`.

This is a **documentation-only architecture boundary**. It freezes what Deus OS is intended to be before further firmware or host-application implementation. No source mutation, build, flash, USB protocol change, application loader, bootloader, filesystem or host GUI is authorized by this boundary.

## 1. Product role

Deus OS is a small deterministic embedded operating system for STM32-class controllers. Its primary job is to run device logic locally, expose hardware and system services through stable APIs, provide a minimal local status/UI surface, and remain independently operable when no PC is connected.

The intended product shape is:

```text
physical device / sensors / actuators
               |
         Deus OS firmware
               |
  kernel + drivers + applications
      |                    |
 local OLED UI       USB/UART service API
                           |
                 Deus OS Control Panel
                 Windows / Linux host
```

The STM32 firmware is the device runtime and source of truth for hardware ownership. The future PC application is a management plane, developer/control surface and package/update orchestrator; it is not required for the device to keep running.

## 2. Current accepted capability baseline

The published firmware already provides:

- Cortex-M3 startup/vector/fault ownership;
- 72 MHz system clock and 1 kHz kernel tick;
- production PSP scheduler ownership with blocked/event/timed wake support and fixed priorities;
- production task0 console/runtime and task1 heartbeat;
- Thread/PSP-owned IWDG liveness and deliberate watchdog reset proof;
- UART emergency/text console;
- I2C1 and SSD1306 128x32 display stack;
- retained 21x3 OLED console and frozen 128x9 status-bar geometry;
- native USB Device + CDC ACM;
- transport-neutral command service with 32 registered methods;
- USB CDC text shell;
- binary framed RPC v1 with stable numeric method IDs, request correlation, CRC, bounded arguments and destructive authorization.

The system therefore has a validated kernel/transport substrate, but it does not yet have a formal application runtime, desktop lifecycle, package model or host control application.

## 3. Resource envelope

Current target remains STM32F103 medium-density / Cortex-M3:

```text
CPU          72 MHz
Flash        64 KiB
SRAM         20 KiB
OLED         128x32 monochrome, 512-byte framebuffer
USB          USB FS Device / CDC ACM
UART         USART1 115200 8N1 emergency/text console
```

Current accepted image:

```text
Flash used   40720 / 65536
Flash free   24816 bytes
SRAM used    9752 / 20480
SRAM free    10728 bytes
```

These limits are architectural constraints, not temporary test limits. Designs must prefer static bounded state and avoid introducing a general heap, filesystem or dynamic native-code loader without a concrete measured requirement.

## 4. Application model v1

An application is a registered firmware module with an explicit identity and lifecycle. In v1 applications are **statically linked into the firmware**; they are not arbitrary uploaded ARM binaries.

Target application descriptor concept:

```text
application id
name
version
flags/capabilities
init()
start()
event()
render()
stop()
```

Exact C ABI is deferred to the dedicated `APPLICATION_RUNTIME_FOUNDATION` implementation boundary. The architectural requirements are frozen here:

- public application IDs are explicit and stable, never inferred from table ordinal position;
- application state is explicit and introspectable;
- no application owns hardware registers directly unless the driver contract explicitly grants that ownership;
- applications use kernel/driver/service APIs rather than duplicating hardware policy;
- application failure must not silently transfer watchdog ownership or IRQ ownership;
- application execution is bounded and event driven;
- v1 does not create one scheduler task per application;
- v1 does not require a heap or executable relocation loader.

Initial lifecycle vocabulary:

```text
REGISTERED
STOPPED
STARTING
RUNNING
BLOCKED
STOPPING
FAILED
```

The exact internal representation may be smaller, but host-visible application state must remain deterministic and versioned.

### 4.1 Application event contract

The scheduler event-bit mechanism remains an internal wake/notification primitive and is **not** the public application ABI. `APPLICATION_RUNTIME_FOUNDATION` must define a separate bounded semantic event/service contract with stable event type IDs, explicit source semantics, bounded payload ownership and no pointer-lifetime ambiguity. Initial event concepts may cover timer/input/USB/service/notification/network changes, but exact IDs and payload layout belong to that implementation boundary.

If queued delivery is introduced, backpressure/drop/coalescing semantics must be explicit. A queue is not required merely to define semantic events: initial app dispatch may remain serialized in task0 until a real producer/consumer topology justifies a queue.

Canonical completeness/gap review:
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

## 5. Runtime ownership model

The accepted two-task production topology remains authoritative for the first application-runtime phase:

```text
task0  system console/runtime/application dispatch
       priority 128
       stack 1024 bytes

task1  production heartbeat/liveness
       priority 255
       stack 512 bytes

MSP    bootstrap / scheduler host idle / fault ownership
```

Applications do not receive dedicated tasks by default. Initial application callbacks execute from the system/runtime path in task0 / Thread-PSP. Existing timed-event primitives may be used to schedule UI/runtime ticks without creating a generic timer callback subsystem.

A later task-capable application model is allowed only after a real application proves that event-dispatch execution in task0 is insufficient and after stack/SRAM ownership is explicitly budgeted.

## 6. System applications and user applications

The runtime will distinguish system-owned applications from ordinary device applications.

Initial system application candidates:

- `System` / desktop-home status;
- `Console`;
- `Device Info`;
- `I2C Scanner`;
- `Monitor` / health diagnostics.

Later device-specific applications may include sensor monitors, GPIO/relay control, timers, motor/device controllers, network status or other board-specific functions.

System applications may be mandatory and non-removable in early versions. User/device applications remain statically built until package execution semantics are independently designed and accepted.

## 7. Local UI lifecycle

The current firmware directly renders a runtime screen containing the frozen status bar and the three text rows `DEUS OS`, `BOOT OK`, `READY`. There is no distinct splash/desktop state machine yet.

The first firmware boundary after this architecture foundation is:

`BOOT_DESKTOP_UI_FOUNDATION`

Required lifecycle:

```text
RESET
  -> kernel/hardware bootstrap
  -> DEUS OS splash
  -> services/runtime ready
  -> desktop/home screen
  -> application view
```

The splash transition must not create a long busy delay and must not compromise USB enumeration, scheduler startup or IWDG liveness. A minimum visible splash dwell, if used, must be implemented with existing kernel time/event semantics rather than a blocking spin delay.

## 8. Status-bar semantics

The accepted 128x9 geometry remains frozen initially. The currently decorative values become real system state.

Initial semantics:

```text
indicator 0  SYSTEM
             filled = normal production runtime/liveness healthy
             ring/off = degraded/not-ready state

indicator 1  USB
             filled = CDC configured / host link active
             ring/off = no configured CDC host

indicator 2  NETWORK
             filled = network service online
             ring/off = network absent/offline/not yet implemented

right field  TIME
             initial source = uptime HH:MM
             later source may become synchronized wall time or RTC
```

The status bar must not claim network connectivity or wall-clock time that does not exist. Until networking is implemented, the NETWORK indicator explicitly represents unavailable/offline state.

## 9. Desktop / home screen

Because the local display is 128x32 and has no accepted local keyboard/button navigation, the first "desktop" is a compact home/status shell rather than a window manager.

It owns the three text rows below the status bar and may show:

- system/application name;
- current application/state;
- concise host/USB/device status;
- application-specific three-line content when an application owns the view.

The desktop is firmware-owned and remains useful with no host PC attached.

Local navigation input is deferred until a real button/encoder/touch/host-input path is selected. The absence of local navigation must not block application lifecycle or host control.

## 10. Firmware versus host responsibility

Firmware owns:

- startup, scheduler, faults and watchdog;
- hardware drivers and hardware safety;
- application registry/lifecycle;
- local UI/desktop state;
- authoritative device configuration currently resident on the MCU;
- RPC service semantics;
- update acceptance and Flash mutation policy when an update subsystem is later added.

Future Deus OS Control Panel owns:

- device discovery and connection;
- HELLO/protocol negotiation;
- human-friendly command/control UI;
- logs, diagnostics and telemetry presentation;
- application list/start/stop/control UX;
- editing/building configuration and resource packages;
- upload/download orchestration;
- firmware-update orchestration;
- host-side plugin ecosystem.

The host must not be required for normal runtime liveness.

## 11. Plugin and package strategy

Three distinct concepts are used to avoid conflating host plugins with target executable code.

### 11.1 Target applications

Initially statically linked firmware modules. This is the v1 application model.

### 11.2 Host plugins

Future optional plugins for Deus OS Control Panel. They may provide host UI panels, protocol tooling, visualization or device-specific workflows. They execute on Windows/Linux, not on STM32.

### 11.3 Target packages

Later USB-transferable packages may contain non-executable resources such as:

- UI layouts;
- small monochrome icons/bitmaps;
- configuration presets;
- device profiles;
- application data.

They require versioning, CRC/integrity, size limits and persistent-storage policy before implementation.

Arbitrary uploaded native ARM code is explicitly deferred. If dynamic target code becomes a real requirement, bytecode/VM or a constrained native ABI must be evaluated as a separate architecture boundary.

## 12. Firmware update strategy

The accepted binary RPC v1 is a control substrate, not a firmware updater. It currently has no file-transfer, Flash-write, update transaction or bootloader semantics.

Planned evolution:

```text
stable application/runtime APIs
  -> host Control Panel
  -> bounded asset/config transfer
  -> recoverable firmware-update protocol
  -> small bootloader / rollback or recovery path
```

Normal development continues to use the current accepted flash/recovery workflow until the update subsystem is independently accepted.

## 13. Networking strategy

Networking remains a later service layer, likely through ESP8266/ESP-class hardware or another explicit network coprocessor path. The STM32 remains the authoritative device runtime.

Network APIs should expose the same application/service semantics rather than creating a second command universe. Network loss must not destabilize scheduler, watchdog or local UI ownership.

### 13.1 Foundation contracts retained for later boundaries

The lower architecture review found no blocker that requires speculative kernel work before boot/desktop/application runtime. It does, however, freeze these later contracts:

- **system identity/capabilities:** protocol HELLO currently describes transport/protocol capabilities, not the complete OS/device identity. Before mature host tooling depends on identity, define firmware/build/platform/service/application discovery semantics;
- **persistent state:** before first Flash-resident settings/packages, define version/schema, size bounds, integrity, atomic commit, reset/power-loss recovery, wear budget and incompatible-version behavior; a filesystem is not implied;
- **crash/reset observability:** current `fault_record` is runtime `.bss` state and is cleared on reset. A later boundary should preserve a bounded previous-boot crash/reset record and boot/reset history semantics;
- **structured observability:** selected counters/events should later become a bounded host-readable telemetry/log model without replacing UART emergency diagnostics or deterministic fault handling;
- **security/trust:** CRC-16 and `ALLOW_DESTRUCTIVE` are not authentication. Network mutation and executable Flash update require an explicit authorization/authenticity/trust contract before acceptance;
- **controlled reboot/update handoff:** belongs to `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`, separate from the deliberate watchdog reset diagnostic;
- **portability:** architecture/platform/low-level drivers must remain separable from kernel/services/apps/UI/protocol semantics. Do not add a heavyweight generic HAL solely for hypothetical ports; refactor further when a second real target exists.

Generic timers, message queues, synchronization and runtime statistics remain consumer-driven Phase 4 work. Power management, RTC, DMA framework, filesystem, heap, MPU isolation and similar facilities are not current prerequisites and remain use-case driven.

## 14. Development and support policy

Each major layer remains independently versioned and gate-accepted:

```text
kernel / scheduler / drivers
        -> command service
        -> binary protocol
        -> product/application model
        -> boot/desktop UI
        -> application runtime
        -> host Control Panel
        -> asset/config transfer
        -> firmware update/bootloader
        -> networking extensions
```

Rules:

- preserve direct-register bare-metal kernel policy;
- no HAL/Arduino/FreeRTOS substitution;
- no hidden dynamic allocation;
- no source mutation in a docs-only boundary;
- every hardware-visible UI change receives hardware acceptance;
- every transport/update change receives malformed-input and recovery testing;
- do not overload one boundary with UI, loader, filesystem, host GUI and bootloader work.

## 15. Roadmap after this docs-only foundation

Exact order:

1. `BOOT_DESKTOP_UI_FOUNDATION`
   - nonblocking boot splash;
   - desktop/home transition;
   - real SYSTEM/USB/NETWORK indicator semantics;
   - uptime `HH:MM` status time;
   - preserve frozen geometry and current two-task topology.

2. `APPLICATION_RUNTIME_FOUNDATION`
   - static application descriptors/IDs;
   - lifecycle/state registry;
   - system applications;
   - bounded semantic application event/service contract distinct from scheduler wake bits;
   - list/status/start/stop/control service API;
   - no dynamic native loader.

3. `HOST_CONTROL_APPLICATION_FOUNDATION`
   - Windows/Linux host application;
   - device discovery and HELLO;
   - usable system identity/build/platform/capability discovery before host UX depends on it;
   - health/console/application control;
   - binary RPC client foundation.

4. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION`
   - bounded versioned upload/download for non-executable resources/configuration;
   - persistent storage contract before Flash-resident target state: versioning, integrity, atomic commit/recovery and wear policy.

5. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`
   - recoverable native-USB update path;
   - controlled reboot/update handoff;
   - explicit Flash/update transaction safety;
   - security/trust contract covering update authenticity and authorization before executable Flash mutation.

6. networking/service extensions.
   - reuse the same application/service model;
   - require authorization/trust review before network-accessible state mutation.

Later, independently scheduled when a real consumer benefits: bounded crash/reset retention and structured observability/telemetry.

## 16. Non-goals of this boundary

This documentation boundary does not implement:

- splash or desktop firmware code;
- application registry code;
- new scheduler tasks;
- dynamic executable loading;
- filesystem;
- heap;
- asset transfer;
- Flash persistence;
- firmware update;
- bootloader;
- Control Panel executable;
- networking;
- OLED geometry redesign.

Those remain separate independently accepted boundaries.
