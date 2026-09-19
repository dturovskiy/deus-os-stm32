# Architecture

Status: **`USB_MANAGEMENT_DEVICE_FOUNDATION` published at `1f88083843c6aae9fd228ad2d677f9252b889a11`; `HOST_CONTROL_APPLICATION_FOUNDATION` Gate 0 accepted / Gate 1 source implementation next**

## Current foundation completeness constraints

Canonical review: `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`.

The published kernel/transport substrate is sufficient to proceed to boot/desktop and the static application runtime without speculative kernel expansion. The following rules are now part of the architecture:

- scheduler event bits are internal wake notifications and must not become stable application event IDs;
- `APPLICATION_RUNTIME_FOUNDATION` owns a bounded semantic application event/service contract;
- generic timers, queues, synchronization primitives and runtime statistics remain consumer-driven extensions; scheduler-internal PRIMASK save/restore is not a public mutex/semaphore API;
- host-facing system identity must eventually distinguish firmware/build/platform/service/application capabilities from USB identity and protocol capability flags;
- accepted `KERNEL_COMPOSITION_ROOT_DECOMPOSITION` candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a` uses Flash `48636/65536`, SRAM `10032/20480`, and final task0/task1 margins `384/424`; `src/kernel.c` is reduced from `5100` to `3405` lines with application bridge, application command and scheduler diagnostic ownership extracted without a universal god object or hidden cross-module state; Gate 2 evidence is `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`, Gate 3 evidence is `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`, and Gate 4 is `PHYSICAL_OLED=PASS`; after publication, `USB_MANAGEMENT_DEVICE_FOUNDATION` owns the production Windows USB profile: vendor-specific WinUSB management transport with explicit product identity/device-interface GUID, reusing the accepted binary RPC above transport; CDC/COM must not remain the primary production host API;
- persistent target state requires versioning, integrity, atomic commit/recovery and Flash wear policy before acceptance;
- current `fault_record` is normal `.bss` runtime state and is cleared by reset; bounded previous-boot crash/reset retention is later observability work;
- CRC-16 and explicit destructive intent are not authentication; trust-sensitive network mutation and firmware update require a separate security/authenticity contract;
- controlled reboot/update handoff belongs to the update/bootloader boundary;
- startup/context-switch/register drivers remain arch/platform-specific while kernel/services/apps/UI/protocol semantics should avoid leaking STM32 register details upward;
- heap, filesystem, generic DMA framework, RTC, MPU isolation and broad power-management facilities are not current prerequisites.

## Published boundary — Boot / desktop UI foundation

Canonical design: `docs/BOOT_DESKTOP_UI_PLAN.md`.
Canonical acceptance: `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.

The first UI implementation remains deliberately below the application runtime. It adds only `BOOT_SPLASH -> DESKTOP_HOME`, preserving the two-task topology. Bootstrap may perform the one initial splash render before scheduler start; after that, task0 / Thread-PSP is the single normal OLED writer. Task0 uses the accepted timed wait with a 250 ms timeout to service UI state while retaining immediate UART/CDC event wakes. The minimum splash dwell is 1000 ms but never delays scheduler/IWDG startup.

The status bar keeps the accepted 128x9 geometry. SYSTEM reflects scheduler/task/watchdog readiness, USB reflects actual CDC configured state for this boundary, NETWORK remains inactive until a real network service exists, and the right field is monotonic uptime `HH:MM` saturating at `99:59`. Polling does not imply periodic redraw: the framebuffer/panel is updated only when lifecycle state, an indicator, displayed minute, or explicit `uiruntime` restore changes visible state. Steady-state redraw keeps an initialized OLED powered on; SSD1306 reinitialization/display-off is reserved for initial bring-up or real transfer recovery. This behavior is build-, hardware- and physically accepted on candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`, BIN SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`, with `PHYSICAL_OLED=PASS`.

The boot/desktop boundary is published at commit `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`, tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`; final repository state after publication was clean at ahead/behind `0/0`.

## Published boundary — OLED dirty-region optimization

Canonical design: `docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`.
Canonical acceptance: `docs/OLED_DIRTY_REGION_OPTIMIZATION_ACCEPTANCE_PLAN.md`.
Canonical deferred engineering policy: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`.

`OLED_DIRTY_REGION_OPTIMIZATION` is accepted through Gate 7 and published at commit `39690c9ef103cbcf93272df8bad0359a934b7dc1`, tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`. It retains one 512-byte framebuffer and adds bounded 16-bit horizontal dirty spans for the existing eight-page maximum. No-op writes remain clean; the aligned renderer uses the same change-aware byte path; `ssd1306_present()` programs exact page+column windows and transfers only the dirty span. The production `mono_fb_t` growth is 32 bytes; total named persistent optimization metadata is 55 bytes and accepted static SRAM is `9848 / 20480`, below the frozen `9856` ceiling. Normal minute/SYSTEM/USB updates are component-local and do not clear/recompose the whole framebuffer or rerasterize console content; initial render, splash->home, explicit restore and recovery may still perform full composition. A second framebuffer remains forbidden.

Accepted candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`, BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`. Hardware measured the historical four-page semantic refresh at `572` payload bytes / `36` writes, clean present at `0`, one changed byte at `9`, minute `00:00 -> 00:01` at `11`, and USB indicator transition at `11`. Task0 retained `328` bytes margin after both `oledstatus` and `oleddirty`; physical OLED review is `PHYSICAL_OLED=PASS` with no blank pulse, stale pixels, clipping or console corruption.

Deferred performance work is measurement-triggered, not speculative: I2C IRQ/DMA only after observed bus/latency pressure; scheduler ready-set acceleration only after materially larger task counts and measured overhead; CRC acceleration only when streaming cost justifies it; WinUSB/asset/update paths must use bounded batching/minimal-copy/backpressure; tickless/low-power remains deferred until a real power requirement exists. Structured observability should use a bounded binary event ring rather than continuous printf/Flash logging. Heavy acceptance/reference self-tests are candidates for host/build-time or acceptance-only profiles, while the final production binary must receive its own hardware smoke acceptance. External storage policy favors SPI NOR, microSD/FRAM and bounded block-device/filesystem layers; laptop DDR SO-DIMM is not a practical F103 expansion.

Gate 6/7 publication is complete by ordinary non-force push; final repository state is clean at ahead/behind `0/0`. Gate 6 evidence is `026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB`; Gate 7 evidence is `FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718`.

## Published boundary — Application runtime foundation

Canonical design: `docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`.
Canonical acceptance: `docs/APPLICATION_RUNTIME_FOUNDATION_ACCEPTANCE_PLAN.md`.

`APPLICATION_RUNTIME_FOUNDATION` implements the static firmware-linked application model frozen by the published architecture foundation. Gate 0 fixes two initial system applications (`system.home=0x0001`, `device.info=0x0002`), one foreground app, explicit lifecycle state, task0-only serialized callbacks, a pointer-free fixed semantic event object, a bounded system-state service snapshot and a three-row application view model. Applications own content rows only; the status bar, drivers, scheduler, IWDG and IRQ policy remain system-owned.

The command/RPC surface is extended additively, not renumbered: existing IDs `0x0001..0x0020` remain unchanged and `applist/appstart/appstop` use `0x0021..0x0023`. The command-service foundation version advances to `2` while binary frame protocol v1 remains unchanged. No heap, event queue, mutex, new task/SVC, dynamic loader, filesystem or USB redesign is part of the boundary.

Initial Gate 1 source boundary is limited to new `include/kernel/application_runtime.h` / `src/kernel/application_runtime.c` plus `include/kernel/command_service.h`, `src/kernel/command_service.c` and `src/kernel.c`. Production WinUSB management USB remains the later `USB_MANAGEMENT_DEVICE_FOUNDATION`.

## Published boundary — Kernel composition-root decomposition

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION` is accepted and published as `fa75307fb392718a1d10d52770a6a111c97208e7`. The published implementation boundary is `USB_MANAGEMENT_DEVICE_FOUNDATION`; Gates 0–7 are accepted on repaired descriptor candidate tree `46841b52d351277deb134a6f4709619087b477af`, BIN `50172` / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`, at Flash/SRAM `50172/11728` within frozen ceilings `54780/11824`. Gate 3 composite hardware acceptance (`v11+v14`) proves `REV_0102`, automatic inbox `winusb.inf` binding on interface 2, exact HELLO/RPC/application lifecycle and malformed-frame recovery, physical USB reconnect, authorized IWDG recovery, 128/128 unique WinUSB pings with management RX/TX drops `0/0`, CDC isolation/binary continuity, UART `32/32`, task0/task1 margins `448/424`, zero scheduler faults/canary failures, and final Flash equal to the accepted BIN. Gate 3 evidence SHA-256 is `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6`; log SHA-256 is `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`. OLED/UI/application rendering remained unchanged, so Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`. Gate 5 documentation finalization, Gate 6 local acceptance commit and Gate 7 ordinary non-force publication are complete. Published USB-management commit/tree are `1f88083843c6aae9fd228ad2d677f9252b889a11` / `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`; post-push verification proved `HEAD == origin/main == FETCH_HEAD` and a clean repository. `HOST_CONTROL_APPLICATION_FOUNDATION` Gate 0 is accepted. The host foundation freezes C#/.NET 10, a transport-neutral Core, direct Windows WinUSB discovery, Linux libusb management-interface access, CLI-first acceptance, Avalonia desktop presentation, one in-flight RPC per device, reconnect as a fresh HELLO + identity negotiation, and no stable physical unit identity in v1. Firmware Gate 1 adds only safe `sysinfo=0x0024`, advances command-service v2 -> v3 / registry 35 -> 36, retains binary protocol v1, and exposes stable OS/platform/architecture/source-tree/capability identity without changing USB descriptors, endpoint topology, tasks or local UI. Canonical design/acceptance: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md` and `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`. Gate 1 source implementation is next.

The accepted decomposition reduces `src/kernel.c` from `5100` to `3405` lines (-33.235%). `application_runtime_bridge` owns mutable application-runtime integration state, semantic-event snapshotting and application-view adaptation; `application_commands` owns `rpcinfo/applist/appstart/appstop`; `scheduler_diagnostics` owns diagnostic orchestration. The composition root retains top-level initialization/wiring, production task binding/start, top-level IRQ/exception glue and production scheduler observability that depends on root-private production state. No universal `kernel_context_t`, service locator, hidden extracted-module extern state, heap, new task/SVC/queue/mutex/generic timer/DMA/persistence machinery or dependency cycle was introduced.

Accepted candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` bytes / SHA-256 `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF SHA-256 `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP SHA-256 `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536`, SRAM `10032/20480`, final task0/task1 margins `384/424`.

Linked ownership movement is measurable rather than cosmetic: `console_execute_request 8644 -> 6780`, `boot_desktop_ui_render 1420 -> 804`, `kernel_main 1116 -> 1112`; scheduler diagnostic orchestration moved from former monolithic `console_execute_scheduler_diagnostic=3284` into `scheduler_diagnostics_execute_diagnostic=3312`. Explicit owner entry points include `application_commands_execute=776`, `application_runtime_bridge_service=316`, and `application_runtime_bridge_apply_view=216` bytes.

Gate 2 evidence SHA-256 `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; Gate 3 evidence SHA-256 `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; Gate 4 `PHYSICAL_OLED=PASS`.

The accepted decomposition is substantial but intentionally not treated as the terminal shape of `src/kernel.c`. Remaining trigger-driven architecture debt is canonicalized in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`: further composition-root convergence, service-state direction independent of UI presentation, conditional separation of OLED adaptation from the application runtime bridge, the synchronous/non-reentrant scheduler-diagnostic binding contract, and explicit stop-failure semantics before resource-owning applications. These are deferred follow-ups, not blockers for the current USB-management boundary, and they do not authorize speculative framework/HAL/god-context work.

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
