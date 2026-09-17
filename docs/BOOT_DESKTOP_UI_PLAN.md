# Deus OS — Boot / Desktop UI Foundation Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT CURRENT**

Boundary ID:

`BOOT_DESKTOP_UI_FOUNDATION`

Published repository prestate:

- `main`, `origin/main` and fresh remote `main` synchronized at `117607f508cd71cf679aa43888b2b05f8143272d`;
- tree `b4e9baf0d8f3a3f6f64f9772b45bad9e130f1ec6`;
- subject `docs: finalize published foundation status`;
- accepted firmware/source baseline remains `2fde9025a51021511e73a76b561f7983ca655e2f` / tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- accepted firmware BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- `OS_APPLICATION_AND_UI_MODEL_FOUNDATION` published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`.

This boundary is the first firmware implementation of the published product/UI architecture. It deliberately remains smaller than the future application runtime.

## 1. Purpose

Replace the current one-shot `DEUS OS / BOOT OK / READY` composition with a bounded runtime-owned local UI lifecycle that truthfully represents boot readiness, USB state, absent networking and monotonic uptime.

The delivered runtime lifecycle is:

```text
RESET
  -> hardware/kernel bootstrap
  -> BOOT_SPLASH
  -> DESKTOP_HOME
```

`APPLICATION_VIEW` is **not** implemented here. It remains owned by `APPLICATION_RUNTIME_FOUNDATION` after the static application registry/lifecycle/event contract exists.

## 2. Existing substrate used unchanged

The published source already provides everything required for this slice:

- 1 ms monotonic `kernel_time_now()` plus wrap-safe elapsed comparison;
- `scheduler_wait_events_timeout()` and `scheduler_sleep_ms()`;
- two production PSP tasks;
- task0 as normal console/runtime owner;
- task1 as heartbeat/liveness owner;
- Thread/PSP-only IWDG reload ownership;
- `usb_cdc_is_configured()`;
- frozen 128x32 framebuffer/layout/console geometry;
- dirty-page presentation;
- current 128x9 status renderer;
- command-driven OLED diagnostics executed from task0 in normal production.

No new task, SVC, queue, mutex, generic timer service, heap or filesystem is required.

## 3. Exact UI states

The first implementation owns exactly two normal states:

```text
BOOT_DESKTOP_UI_SPLASH
BOOT_DESKTOP_UI_HOME
```

Initial bootstrap state is `BOOT_DESKTOP_UI_SPLASH`.

Transition to `BOOT_DESKTOP_UI_HOME` is allowed only when both are true:

1. minimum visible splash dwell has elapsed;
2. production runtime readiness is true.

Minimum splash dwell:

```text
BOOT_DESKTOP_UI_SPLASH_MIN_MS = 1000 ms
```

Runtime readiness means:

- scheduler active;
- production task0 started;
- production task1 heartbeat started;
- IWDG active;
- task0 runtime fault flag clear;
- task1 heartbeat fault flag clear.

USB connection is not a readiness prerequisite. Networking is not a readiness prerequisite.

If runtime readiness never becomes true, the UI remains in splash/not-ready state rather than falsely entering the desktop.

## 4. Splash content

The splash uses the already accepted 128x9 status bar and 21x3 console viewport. No new geometry is introduced.

Console rows:

```text
DEUS OS
STARTING
PLEASE WAIT
```

The splash is presented during bootstrap after SysTick and OLED initialization. This initial pre-scheduler render is allowed because no production task can concurrently own the display yet.

The firmware must not spin or sleep merely to keep the splash visible. Scheduler/IWDG startup proceeds immediately after the first render.

## 5. Desktop/home content

The first home screen is intentionally small and product-neutral:

```text
DEUS OS
DESKTOP
READY
```

This is not a window manager and does not claim an application registry exists. Local input/navigation remains deferred.

The desktop remains useful without a connected PC and is the default runtime view restored by the existing `uiruntime` command once the splash transition has completed.

## 6. Status-bar state model

The accepted status geometry stays exactly 128x9. The three 5x5 indicators become runtime state rather than fixed decoration.

The v1 renderer has two visual indicator states only:

```text
RING    inactive / unavailable / degraded / not-ready
FILLED  active / available / healthy
```

No third visual state is added in this boundary.

Semantic mapping:

```text
indicator 0  SYSTEM
  FILLED = production runtime readiness true
  RING   = bootstrap/not-ready/degraded

indicator 1  USB
  FILLED = usb_cdc_is_configured() != 0
  RING   = CDC not configured

indicator 2  NETWORK
  FILLED = reserved for a future accepted network-online service
  RING   = current required state; networking does not yet exist
```

NETWORK must therefore remain RING throughout this boundary.

`oled_status_bar_t` may be extended with bounded indicator state and an explicit setter. The historical reference self-test must remain able to render the accepted filled/filled/ring `00:00` reference exactly.

## 7. Uptime display

The right field becomes real monotonic uptime `HH:MM`.

Conversion:

```text
total_minutes = kernel_time_now() / 60000
```

For `total_minutes < 6000`:

```text
hours   = total_minutes / 60
minutes = total_minutes % 60
```

At and beyond 100 hours the display saturates at:

```text
99:59
```

This avoids a misleading hour wrap while preserving the fixed two-digit field. Wall-clock/RTC time remains deferred.

## 8. Runtime polling and wake policy

Task0 remains the only normal runtime owner of UI policy.

The console task changes from an indefinite RX-only wait to the already accepted timed wait:

```text
scheduler_wait_events_timeout(
    PRODUCTION_CONSOLE_RX_EVENTS,
    BOOT_DESKTOP_UI_POLL_MS)
```

with:

```text
BOOT_DESKTOP_UI_POLL_MS = 250 ms
```

A timeout returning `0` is a normal UI-service wake, not a scheduler fault.

UART/CDC RX events continue to wake task0 immediately. Input rings are drained before each wait exactly as today.

This is not a generic timer subsystem. It is one concrete consumer of the already accepted timed-blocking primitive.

## 9. Render/update policy

Task0 samples the bounded UI inputs:

- current UI state;
- runtime-ready boolean;
- CDC configured boolean;
- NETWORK=false;
- displayed uptime minute.

The OLED is re-composed/presented only when at least one visible semantic value changes:

- splash -> home transition;
- SYSTEM indicator change;
- USB indicator change;
- NETWORK indicator change;
- displayed minute change;
- explicit `uiruntime` restore request.

A 250 ms task0 timeout therefore does **not** imply a 4 Hz I2C redraw.

Controller initialization is not part of a normal semantic refresh. `ssd1306_init()` includes `display off` and is allowed only for the initial panel bring-up or explicit recovery after a failed panel transfer. Minute rollover, USB/status changes and `BOOT_SPLASH -> DESKTOP_HOME` redraws must keep an already initialized panel powered on. If a runtime present fails, the UI invalidates its panel-initialized state so the next service pass may perform a real controller reinitialization and recovery.

This distinction is mandatory: a visible state change may update framebuffer/panel RAM, but it must not blank the panel as a side effect of routine refresh.

## 10. Display ownership

Normal ownership rule:

```text
bootstrap before scheduler: one initial splash render on MSP
scheduler active:          task0 / Thread-PSP is the single normal OLED writer
IRQ/Handler:               never renders OLED
heartbeat task1:           never renders OLED
```

Existing OLED diagnostic commands may temporarily replace the screen, but in normal production they execute synchronously inside task0. `uiruntime` restores the current runtime UI state and does not reset the boot lifecycle.

This boundary does not introduce concurrent framebuffer writers, locks or an OLED mutex.

## 11. Source boundary

Expected authorized source paths for Gate 1:

```text
src/kernel.c
src/kernel/oled_status_bar.c
include/kernel/oled_status_bar.h
```

No source path outside that set is expected unless Gate 1 proves the plan cannot be implemented safely inside the published interfaces. Any expansion must be documented before mutation.

Expected implementation changes:

- add bounded runtime UI state/snapshot state in `kernel.c`;
- add splash/home composition helpers;
- replace bootstrap one-shot runtime screen with splash initialization;
- service UI from production task0 using `scheduler_wait_events_timeout()`;
- preserve UART/CDC drain-before-wait semantics;
- make timeout wake non-faulting;
- extend status-bar state so the three indicators can render FILLED/RING dynamically;
- keep historical status reference self-test exact;
- keep `uiruntime` as a safe compatibility/restore method with unchanged command ID and argument contract.

## 12. Command/protocol compatibility

No command-service registry count or public RPC method ID changes are required.

The existing 32 methods remain stable. In particular:

- `oledstatus` still validates the status/layout path;
- `uiruntime` still returns `OLED_RUNTIME_UI_OK` / `OLED_RUNTIME_UI_ERR` and restores the runtime-owned view;
- binary RPC v1 framing and HELLO contract remain unchanged;
- UART remains text-only emergency diagnostics.

A new public UI RPC command is explicitly out of scope.

## 13. Safety/liveness invariants

The implementation must preserve:

- task0 stack 1024 bytes with accepted minimum 256-byte margin;
- task1 stack 512 bytes with accepted minimum 256-byte margin;
- task1 heartbeat period 500 ms;
- IWDG reload only from concrete Thread/PSP production progress;
- no IWDG reload from SysTick, UART IRQ, USB IRQ or OLED/I2C IRQ path;
- no blocking splash delay before scheduler/IWDG startup;
- no host dependency for SYSTEM readiness;
- USB disconnect/reconnect cannot destabilize scheduler or liveness;
- OLED failure must not become the only diagnostic path; UART emergency diagnostics remain available.

## 14. Explicit non-goals

This boundary does not add:

- application registry/lifecycle callbacks;
- application semantic event ABI;
- local button/encoder/keyboard navigation;
- host Control Panel or host CLI/TUI;
- network implementation;
- RTC/wall-clock time;
- persistent UI settings;
- configurable new geometry/theme system;
- filesystem/heap/native loader;
- generic timers/queues/mutexes;
- third production task;
- USB descriptor/protocol redesign;
- firmware update/bootloader.

## 15. Gate order

```text
Gate 0  architecture/source-boundary freeze
Gate 1  exact source implementation + self/static review
Gate 2  fresh GNU build/link/resource validation
Gate 3  retained UART/USB/scheduler/IWDG hardware acceptance
Gate 4  mandatory physical OLED visual acceptance
Gate 5  docs/evidence finalization
Gate 6  local acceptance commit
Gate 7  ordinary non-force publication
```

Accepted Gate 2/3 candidate/evidence:

```text
candidate tree          41e0c7cd345dd64d3b5336abf2fc46d446f19ecb
BIN                     41520 bytes
BIN SHA-256              A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42
ELF                     70700 bytes
ELF SHA-256              62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02
MAP SHA-256              D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8
Flash                   41520 / 65536
SRAM                    9792 / 20480
Gate 2 evidence          62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729
Gate 3 evidence          2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD
Gate 4                   PHYSICAL_OLED=PASS
```

After Gate 7, exact next boundary:

`OLED_DIRTY_REGION_OPTIMIZATION`

That optimization must retain the single 512-byte framebuffer and use bounded page/column dirty spans; a second framebuffer is not required. After it, proceed to `APPLICATION_RUNTIME_FOUNDATION`.
