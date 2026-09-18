# STM32 OS — Implementation Plan

<!-- BEGIN STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Current implementation checkpoint — 2026-09-17

### Transport-neutral shell/RPC foundation — PUBLISHED

Published commit/tree:

`0c33304d2db86e54d715905393f49147bb6dd2ea` /
`19ac9b95caca09e991842f6f9963864934b0a334`.

Accepted firmware:

`37196` bytes /
`90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`.

The static allocation-free 32-method command service, explicit response context, UART/CDC text adapters, argument parser, physical reconnect and post-IWDG CDC recovery are accepted and published.

### Published boundary — binary framed transport foundation — GATES 0–7 ACCEPTED

Boundary:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Gate 0 freezes:

- protocol v1 over the existing USB CDC stream, with UART retained as text-only emergency console;
- `A5 5A` binary magic and deterministic text/binary demultiplexing;
- little-endian fixed envelope with request ID, payload length and CRC-16/CCITT-FALSE;
- stable public RPC IDs `0x0001..0x0020`, independent of internal dispatch enum ordinals;
- `HELLO`, `RPC_REQUEST`, chunked `RPC_DATA`, final `RPC_END`, and `PROTOCOL_ERROR`;
- max four arguments / 31 bytes each / 132-byte request payload;
- 48-byte response chunks so maximum `RPC_DATA` frame is exactly one 64-byte USB FS packet;
- explicit `ALLOW_DESTRUCTIVE` flag for destructive methods;
- one narrow nonblocking all-or-none CDC TX-span API for frame atomicity;
- command execution remains through the published `command_service_execute()` path;
- no heap, new task/SVC/IPC/timer subsystem, USB descriptor/PMA redesign, OLED edit, host GUI, firmware update or bootloader.

Canonical protocol:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

Canonical design:
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`

Canonical acceptance:
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`

Accepted Gate 2/3 candidate:

- tested tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- binary safe surface `21/21`, scheduler BUSY `8/8`, binary pressure `128/128` unique IDs;
- retained CDC/UART pressure, physical reconnect, deliberate IWDG reboot and post-reset text+binary recovery: PASS;
- final Flash readback exact;
- Gate 4 `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 5 documentation/evidence finalization: PASS.

Next:

Binary Gate 7 publication is complete at `2fde9025a51021511e73a76b561f7983ca655e2f` / tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`.

### Published boundary — OS application and UI model foundation — GATES 0–7 ACCEPTED

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION` is documentation-only. It freezes the Deus OS product role, static application/lifecycle model, boot-splash/desktop/application-view ownership, real status-bar semantics, firmware-vs-host responsibilities and long-term package/update ordering before new source work.

Canonical design: `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`.
Canonical acceptance: `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`.
Canonical completeness review: `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

The completeness review confirms there is no kernel blocker before boot/desktop/application work. It adds these mandatory later contracts: semantic app events separate from scheduler wake bits; firmware/build/platform/system capability identity for host tooling; bounded recoverable persistence before Flash settings/packages; retained previous-boot crash/reset diagnostics and structured observability as later work; explicit security/trust before network mutation or executable Flash update; controlled update reboot handoff; and portability layering between arch/platform/drivers and kernel/services/apps/UI/protocol. Generic timers/queues/synchronization/runtime statistics remain consumer-driven; heap/filesystem/MPU/RTC/DMA/power frameworks remain deferred until justified.

Gates 0–7 are accepted and ordinary non-force publication is complete at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c` / tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`. The boundary changed documentation only; no firmware build/flash was required.

### Published boundary — Boot / desktop UI foundation — GATES 0–7 ACCEPTED

`BOOT_DESKTOP_UI_FOUNDATION` is the first firmware consumer of the published UI architecture. The accepted implementation is a two-state `BOOT_SPLASH -> DESKTOP_HOME` runtime lifecycle with nonblocking 1000 ms visible splash dwell, task0-owned 250 ms timed UI service, real SYSTEM/USB/NETWORK indicators, monotonic uptime `HH:MM`, render-on-visible-change behavior, and one-time/recovery-only SSD1306 initialization. It does not introduce `APPLICATION_VIEW`, an application registry/event ABI, local input, host tooling, production WinUSB management USB, networking, persistence, a new RTOS primitive or a third task.

Accepted Gate 2/3 candidate: tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`; BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; ELF `70700` bytes / `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`; MAP `D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8`; Flash `41520 / 65536`; SRAM `9792 / 20480`. Gate 3 retained CDC/UART/binary pressure, USB reconnect, IWDG reboot/recovery and exact final Flash readback all pass. Gate 4 is `PHYSICAL_OLED=PASS` with clean minute/text transitions and no blank pulse/flicker/stale pixels. Gate 5 is docs/evidence-only finalization.

Canonical design: `docs/BOOT_DESKTOP_UI_PLAN.md`.
Canonical acceptance: `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.

Publication is complete at `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8` / tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`; ordinary non-force push is complete and local/remote ahead-behind is `0/0`.

### Published boundary — OLED dirty-region optimization — GATES 0–7 ACCEPTED

`OLED_DIRTY_REGION_OPTIMIZATION` is a measured rendering/transport optimization. It keeps one 512-byte framebuffer, adds 16-bit per-page dirty X spans, makes framebuffer byte writes change-aware, updates the aligned text fast path, sends exact SSD1306 page+column windows, records bounded present-transfer metrics and changes runtime status composition so minute/SYSTEM/USB changes do not clear/recompose the full screen. No shadow framebuffer, heap, DMA or new RTOS primitive was introduced.

Accepted candidate/evidence:

- tree `75f05f689970b760604112b30346b0c328bfaff2`;
- BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- ELF `71308` bytes / `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`;
- MAP `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`, named optimization metadata increment `55` bytes;
- Gate 2 evidence `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`;
- Gate 3 log `887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6`;
- Gate 3 evidence `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`;
- Gate 4 evidence `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`, `PHYSICAL_OLED=PASS`.

Measured hardware transfer contract: prior full semantic refresh `572` payload bytes / `36` writes; clean present `0`; one-byte change `9`; minute update `11` at `x=123..125`; USB indicator update `11` at `x=9..11`. Post-diagnostic task margins are task0 `328` bytes and task1 `424` bytes. Gate 5 records deferred optimization/robustness/observability/storage/test-profile policy in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` and adds repository LF policy through `.gitattributes`.

Canonical design: `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`.
Canonical acceptance: `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`.
Canonical deferred backlog: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.

Gate 6/7 publication is complete at `39690c9ef103cbcf93272df8bad0359a934b7dc1` / tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`, with ordinary non-force push and final ahead/behind `0/0`. Gate 6 evidence is `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence is `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

### Published boundary — Application runtime foundation — GATES 0–7 ACCEPTED / PUBLISHED `25752fba557b1a1b518265a93bde05d3a6a3f9ad`

`APPLICATION_RUNTIME_FOUNDATION` implements the published static application model without scheduler expansion. Gate 0 freezes two built-in system applications (`system.home=0x0001`, `device.info=0x0002`), explicit lifecycle state, one foreground app, task0-only callback/event dispatch, bounded pointer-free semantic events, a bounded service snapshot and three-row application view ownership under the system status bar.

The existing command/RPC surface remains stable at IDs `0x0001..0x0020`; exactly three new methods are appended: `applist=0x0021`, `appstart=0x0022`, `appstop=0x0023`. Command-service foundation version becomes `2`; binary framing/protocol remains v1. No heap, queue, new task/SVC, filesystem, dynamic loader or USB redesign is authorized.

Canonical design: `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`.
Canonical acceptance: `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`.

Accepted source boundary is new `include/kernel/application_runtime.h`, new `src/kernel/application_runtime.c`, plus `include/kernel/command_service.h`, `src/kernel/command_service.c`, and `src/kernel.c`. Gate 2/3 candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c` uses Flash `48604/65536`, SRAM `10032/20480`, and hardware minimum task0/task1 margins `272/424` bytes after full application/binary activity. Evidence SHA-256: Gate 2 `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`; Gate 3 `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; Gate 4 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`; Gate 5 `817D12D74D88F1C0F31C502B0715F038FF356382E56FC0D5421DC237239D78BC`. Gate 6/7 publication is complete at commit `25752fba557b1a1b518265a93bde05d3a6a3f9ad`, tree `24624db70923bdaa77f134cc956423511785c199`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`, Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`. Current boundary is `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` Gate 0 accepted / Gate 1 next; canonical design/acceptance are `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md` and `docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_ACCEPTANCE_PLAN.md`.
<!-- END STM32_OS_IMPLEMENTATION_CHECKPOINT_2026_09_13 -->
## Objective

Build a small, understandable, fast bare-metal operating system for STM32F103-class Cortex-M3 hardware while learning ARM/Thumb assembly, exception mechanics, scheduling, memory layout, drivers, and low-level performance.

The project is not trying to reproduce Linux on a microcontroller. It is a compact embedded kernel with explicit control of the hardware.

## Primary target

- MCU family detected: STM32F101/F102/F103 Medium-density
- CPU: ARM Cortex-M3
- ISA: ARMv7-M / Thumb-2
- Flash target: 64 KiB
- SRAM target: 20 KiB
- programmer/debugger: ST-LINK V2
- debug/program transport: SWD
- development host: Windows 10, PowerShell 7
- compiler: Arm GNU Toolchain, `arm-none-eabi`
- programmer: STM32CubeProgrammer CLI

## Architectural principles

1. Bare metal first.
2. No STM32 HAL, Arduino framework, or FreeRTOS in the kernel baseline.
3. Assembly is used where architecture mechanics matter: startup, exception entry/exit, context switching, and measured hot paths.
4. C is used for policy, drivers, data structures, and readable kernel logic unless assembly has a measured benefit.
5. Static allocation is preferred in early kernel phases.
6. Interrupt handlers remain short and deterministic.
7. Hardware drivers do not own scheduler policy.
8. Networking remains outside the core kernel until scheduler, diagnostics, and local drivers are stable.
9. Every optimization requires a baseline measurement.
10. Hardware evidence and source/build evidence are tracked separately.

## Layering target

```text
application / future control services
               |
kernel services / IPC / timers
               |
scheduler / task management
               |
exceptions / SysTick / PendSV
               |
device drivers
               |
register-level STM32 hardware access
               |
startup / linker / vector table
```

## Boot architecture

The CPU begins from Flash at `0x08000000`.

- vector[0] -> initial MSP
- vector[1] -> Reset_Handler with Thumb bit
- Reset_Handler copies `.data`
- Reset_Handler clears `.bss`
- Reset_Handler calls kernel_main
- kernel_main initializes platform facilities
- later, scheduler becomes the steady-state execution owner

## Memory strategy

Current:

- `.isr_vector`, `.text`, `.rodata` in Flash
- `.data`, `.bss` in SRAM
- initial MSP at top of 20 KiB SRAM

Future:

- explicit kernel stack budget
- explicit task stack regions
- static task-control-block pool
- no general-purpose heap until there is a concrete requirement
- stack guards/watermarks before adding many tasks

## Time model

Target initial timebase:

- SYSCLK/HCLK: 72 MHz
- SysTick: 1 kHz
- `kernel_ticks`: 32-bit monotonic millisecond tick

Later:

- wraparound-safe deadline comparisons
- optional higher-resolution cycle timing for benchmarks
- tickless idle only after scheduler correctness is proven

## Exception model

Required dedicated handlers:

- Reset
- NMI
- HardFault
- MemManage
- BusFault
- UsageFault
- SVCall
- PendSV
- SysTick

Unused handlers may initially terminate in a deterministic default panic loop.

## Scheduler plan

Stage A:
- cooperative tasks
- static TCBs
- PSP-based task stacks

Stage B:
- PendSV context switch
- assembly save/restore of callee-saved registers

Stage C:
- SysTick preemption
- fixed-priority scheduling
- idle task

Stage D:
- sleep queues
- timers
- IPC
- synchronization

## Diagnostics plan

Order:

1. LED/status pin
2. HardFault capture
3. UART text logging
4. SSD1306 local kernel console
5. structured runtime counters

OLED is a status console, not the primary high-volume log transport.

## Networking plan

Future ESP-01/ESP8266 role:

- connected over UART
- ESP side owns Wi-Fi association and TCP/IP-facing complexity
- STM32 owns kernel and device logic
- communication uses a framed, versioned protocol
- web control is implemented above that protocol
- network failures must not destabilize scheduler/interrupt core

## Performance goals

Track at minimum:

- Flash footprint
- static SRAM footprint
- per-task stack/TCB cost
- interrupt latency
- context-switch cycles
- scheduler overhead
- idle behavior
- driver throughput where relevant

"Fast" means measured latency/throughput with deterministic behavior, not simply writing more assembly.

## Development workflow

For each slice:

1. read MASTER_EXECUTION_CHECKLIST.md
2. read PROJECT_HANDOFF.md
3. define exact source files to change
4. build only
5. inspect symbols/vector/disassembly as relevant
6. produce a log
7. only then flash
8. verify programmer success
9. verify physical behavior
10. update PROJECT_HANDOFF.md after acceptance

## Deferred topics

Do not implement yet:

- dynamic heap allocator
- filesystem
- USB stack implementation (architecture planned; implementation deferred until the current scheduler lifecycle boundary is closed)
- TCP/IP directly on STM32
- user/kernel privilege separation
- MPU isolation
- firmware update protocol
- multicore support
- graphical game engine

These can be revisited only after the scheduler and diagnostics baseline is stable.

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_IMPLEMENTATION_ACCEPTED_20260913 -->
## Accepted scheduler steady-state wait/wake foundation — 2026-09-13

The scheduler now has the minimum accepted production blocking model required before normal-boot ownership migration:

- `UNUSED`, `READY`, `DONE`, and explicit `BLOCKED` task states;
- per-task wait/wake event state;
- SVC-based task wait transition;
- ISR-safe event signalling;
- pending-event protection against lost wakeups;
- stable host-MSP `WFE` idle while all incomplete tasks are blocked;
- PSP task resume after IRQ wake;
- UART event integration after RX ring publication;
- framing-aware FIFO re-check semantics for event-driven consumers.

Hardware proof covers repeated block -> idle -> UART IRQ -> wake -> resume cycles, canaries, stack margins, lifecycle isolation, legacy scheduler regressions, RX pressure, MSP telemetry, OLED runtime restoration, target Flash identity, and manual physical OLED acceptance.

### Next implementation slice

**Normal-boot production task ownership / migration**

The detailed design is now canonical in
`docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_PLAN.md`, with gate criteria in
`docs/NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_ACCEPTANCE_PLAN.md`.

The first migration uses one cooperative production console task on the accepted
1024-byte PSP stack, leaves slot 1 UNUSED, preserves IRQ/ring ownership, and
keeps host MSP as scheduler idle owner using WFE. The task drains the RX ring
before waiting on the UART event. Initial OLED composition remains bootstrap
work; runtime OLED/I2C application calls move with console ownership to PSP.

Still deferred to later independent gates:

- `sleep()` / timer-backed blocking on top of the accepted event model;
- scheduler priorities;
- broader application/task topology changes.
<!-- END STM32_OS_SCHED_WAIT_WAKE_IMPLEMENTATION_ACCEPTED_20260913 -->
