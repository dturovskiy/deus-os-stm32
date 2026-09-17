# Deus OS — Boot / Desktop UI Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED — PUBLISHED `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`**

Boundary ID:

`BOOT_DESKTOP_UI_FOUNDATION`

Canonical design:
`docs/BOOT_DESKTOP_UI_PLAN.md`

## Gate 0 — architecture and source-boundary freeze

Required prestate:

- `main`, `origin/main` and fresh remote `main` synchronized at `117607f508cd71cf679aa43888b2b05f8143272d`;
- tree `b4e9baf0d8f3a3f6f64f9772b45bad9e130f1ec6`;
- subject `docs: finalize published foundation status`;
- clean worktree/index before planning;
- accepted firmware/source baseline remains `2fde9025a51021511e73a76b561f7983ca655e2f`;
- accepted BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- published product/application/UI architecture at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`.

Gate 0 must freeze:

- exact two-state implementation lifecycle `BOOT_SPLASH -> DESKTOP_HOME`;
- explicit deferral of `APPLICATION_VIEW` to `APPLICATION_RUNTIME_FOUNDATION`;
- splash text and desktop text;
- nonblocking 1000 ms minimum splash dwell;
- runtime readiness predicate;
- FILLED/RING semantics for SYSTEM/USB/NETWORK;
- NETWORK forced inactive for this boundary;
- uptime `HH:MM` source and `99:59` saturation behavior;
- task0 250 ms timed-service wake policy;
- update-only-on-visible-change rendering policy;
- bootstrap-MSP initial render plus task0 single normal runtime writer rule;
- no new command/RPC ID requirement;
- exact initial source path authorization.

No source edit/build/flash/commit/push in Gate 0.

Acceptance: **PASS**. Canonical docs are synchronized at exact published prestate `117607f508cd71cf679aa43888b2b05f8143272d`; docs-only review reports nine documentation paths, no non-doc/forbidden paths, no blockers and `git diff --check` PASS. Read-only source inventory confirms the design can be implemented through the published time/scheduler/CDC/OLED interfaces without a new RTOS primitive or expanding the initial three-file source boundary.

## Gate 1 — source implementation and static consistency

Initially authorized source paths:

```text
src/kernel.c
src/kernel/oled_status_bar.c
include/kernel/oled_status_bar.h
```

Required implementation proof:

- status bar can represent FILLED/RING state for all three indicators;
- accepted historical status reference self-test still passes exactly;
- bootstrap renders splash without a dwell spin/sleep;
- task0 owns splash transition and all normal periodic UI service;
- task0 uses `scheduler_wait_events_timeout(PRODUCTION_CONSOLE_RX_EVENTS, 250)` or a semantically equivalent bounded timeout;
- timeout `0` is normal and does not set `production_console_fault`;
- UART and CDC rings remain drained before wait;
- immediate transport events still wake task0;
- `uiruntime` restores current splash/home state without resetting lifecycle;
- no task1/IRQ OLED rendering;
- no new task/SVC/queue/mutex/generic timer/heap/filesystem;
- no USB descriptor/PMA/binary framing/command-registry change;
- no application runtime/API introduced prematurely.

Static source review must additionally prove:

- splash minimum = `1000 ms`;
- task0 UI service poll maximum = `250 ms`;
- desktop transition requires both elapsed dwell and runtime-ready predicate;
- runtime-ready predicate does not depend on USB/network host presence;
- NETWORK indicator cannot become FILLED in this boundary;
- uptime is derived from `kernel_time_now()` and saturates at `99:59` after 100 hours;
- visible OLED presentation occurs only after semantic state changes or explicit restore;
- steady-state semantic refresh does not call `ssd1306_init()` or otherwise issue the SSD1306 `display off` initialization sequence;
- panel initialization is one-time during normal bring-up and may be re-entered only after an actual panel-transfer/recovery failure.

No flash before Gate 2 passes.

Acceptance: **PASS, revision 2**. Implementation remains confined to the exact three authorized source/header paths. The first Gate 2 candidate reached hardware and exposed a real visual defect: every semantic redraw re-entered `ssd1306_init()`, whose initialization sequence starts with SSD1306 `display off`; therefore minute/status refresh could visibly blank the panel. That candidate is superseded. Revision 2 adds explicit panel-initialized ownership: normal semantic redraw never reinitializes or powers off an already initialized panel, while a failed runtime present invalidates the panel state so the next service pass can perform bounded recovery. All previously accepted properties remain: dynamic FILLED/RING indicators with the historical reference preserved, nonblocking splash, visible-dwell timing, task0-only 250 ms service, timeout `0` normal, drain-before-wait ordering, NETWORK inactive, monotonic uptime with `99:59` saturation, snapshot suppression of unchanged redraws, and no scheduler/USB/command-service/linker/startup/application-runtime mutation.

## Gate 2 — fresh GNU build/link/resource validation

Require a fresh build from the exact Gate 1 source candidate using the accepted Arm GNU toolchain and warnings-as-errors discipline.

Required:

- 16-bit/32-bit Cortex-M3/Thumb build succeeds with `-Wall -Wextra -Werror`;
- no undefined symbols;
- no unexpected startup/vector/linker change;
- no new source outside authorized boundary unless documented in Gate 1;
- command registry count remains exactly `32`;
- stable binary RPC IDs remain `0x0001..0x0020`;
- USB descriptors/PMA map unchanged;
- Flash <= `65536` bytes;
- static SRAM <= `20480` bytes;
- task0 and task1 bound stack capacities remain `1024` / `512` bytes;
- `git diff --check` PASS;
- exact BIN/ELF/MAP/source hashes recorded;
- source candidate tree recorded before flash.

Gate 2 evidence must be generated from the exact source candidate that proceeds to hardware.

Acceptance: **PASS**. Revision-2 candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`; BIN `41520` bytes / SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; ELF `70700` bytes / SHA-256 `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`; MAP SHA-256 `D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8`; Flash `41520 / 65536`; SRAM `9792 / 20480`; undefined symbols `0`; exact 32-method RPC/USB/startup/linker invariants retained. Gate 2 evidence SHA-256 `62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729`.

## Gate 3 — hardware/runtime acceptance

Flash only the exact Gate 2 candidate.

Mandatory retained functional proof:

- boot UART banner and scheduler/IWDG startup tokens still present;
- `ping`, `uptime`, `health`, `cdcstat`, `mspstat`, `schedprod`, `schedprio`, `fault`, `i2cscan`, `oledping`, `oledstatus`, `uiruntime`, `help`, `rpcinfo` retained;
- all active production scheduler diagnostics remain safely BUSY where required;
- UART pressure/race regression PASS with zero drops/errors;
- CDC text pressure PASS;
- binary RPC pressure/request correlation PASS;
- malformed/CRC binary recovery PASS;
- physical USB disconnect/reconnect recovery PASS;
- deliberate authorized IWDG reset produces real reboot and automatic post-reset UART/CDC recovery;
- final Flash readback matches exact accepted candidate.

Runtime UI behavioral proof:

- splash is shown during boot;
- scheduler and IWDG start while splash is still the visible lifecycle state; no 1 s boot spin delays them;
- desktop transition occurs no earlier than 1000 ms after splash start and without host input;
- SYSTEM indicator is inactive/not-ready before runtime readiness and becomes active once both production tasks/IWDG/scheduler are healthy;
- USB indicator follows actual CDC configured/unconfigured state across host open/configuration and physical disconnect/reconnect;
- NETWORK indicator stays inactive;
- `uiruntime` restores the current runtime-owned splash/home view after an OLED diagnostic temporarily changes the screen;
- task0 stack canary intact and margin >= `256` bytes;
- task1 stack canary intact and margin >= `256` bytes;
- MSP guard/capacity remain healthy;
- production fault flags remain zero.

Uptime status proof:

- status right field is generated from monotonic uptime rather than a hardcoded `00:00` constant;
- hardware observation across a minute boundary must show the displayed minute advance, unless an exact deterministic target-side proof is added and independently accepted before Gate 3.

Acceptance: **PASS**. Exact Gate 2 revision-2 candidate was already resident in Flash, so pre-read matched and no erase/program cycle was performed. Boot/scheduler/IWDG startup PASS; task0/task1 margins `640` / `424` bytes; CDC text pressure `128/128` with zero drops; UART pressure `128/128` with zero drops/errors; binary HELLO/request correlation, safe surface, scheduler BUSY, malformed/CRC/oversize/split-frame recovery and `128/128` unique-ID pressure PASS; physical USB reconnect restored text+binary; authorized IWDG reboot completed with automatic UART/CDC/binary recovery; three USB enumeration proofs PASS; final Flash SHA-256 exactly `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`. Gate 3 log SHA-256 `9481BEFADB5A8F1D17AF6FC0ACDE238FE66B6956940F56DC86A828CBFAA7F900`; Gate 3 evidence SHA-256 `2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD`.

## Gate 4 — mandatory physical OLED acceptance

Because this boundary changes the normal visible runtime UI, physical OLED review is mandatory.

Require visual confirmation of:

- correct 128x32 panel mapping;
- no geometry regression in accepted 128x9 status bar and three-row console viewport;
- splash text legible and correctly placed;
- automatic desktop transition visible;
- desktop text legible and correctly placed;
- SYSTEM/USB/NETWORK icon semantics match actual runtime state;
- uptime field does not overlap frame/right inset;
- no stale pixels or dirty-page artifacts across splash -> home, USB state change, minute update, diagnostic -> `uiruntime` restore;
- no flicker caused by 250 ms service polling when no visible state changes;
- no panel blank/off pulse at a normal minute rollover, USB-indicator change, SYSTEM-indicator change or splash -> home transition.

Gate 4 result must be explicit `PHYSICAL_OLED=PASS` before acceptance.

Acceptance: **PASS / `PHYSICAL_OLED=PASS`**. Operator confirmed the runtime appearance is correct: digits and text transition cleanly, no unintended content is rendered, and no visible flicker, panel blank/off pulse or stale-pixel artifact remains.

## Gate 5 — documentation/evidence finalization

Synchronize at minimum:

- `README.md`;
- `CHANGELOG.md`;
- `docs/ARCHITECTURE.md`;
- `docs/IMPLEMENTATION_PLAN.md`;
- `docs/MASTER_EXECUTION_CHECKLIST.md`;
- `docs/PROJECT_HANDOFF.md`;
- `docs/ROADMAP.md`;
- `docs/BOOT_DESKTOP_UI_PLAN.md`;
- `docs/BOOT_DESKTOP_UI_ACCEPTANCE_PLAN.md`.

Record exact accepted candidate hashes, resource usage, hardware evidence, OLED disposition and retained regression results.

Acceptance: **PASS**. Canonical docs record the exact Gate 2/3 candidate, build/resource identity, retained hardware regressions, Gate 3 evidence hashes and Gate 4 `PHYSICAL_OLED=PASS`. Gate 5 changes documentation only; the hardware-accepted firmware source bytes remain unchanged.

After Gate 5, exact next gate is Gate 6 local acceptance commit. No push in Gate 5.

## Gate 6 — local acceptance commit

Require:

- exact Gate 5 accepted source/docs candidate;
- all hardware-visible evidence complete;
- Gate 4 `PHYSICAL_OLED=PASS`;
- no unreviewed paths;
- `git diff --cached --check` PASS;
- one local acceptance commit;
- clean worktree/index after commit;
- remote remains at direct parent;
- ahead/behind `1/0`;
- no push yet.

## Gate 7 — ordinary non-force publication

Before publication:

- fresh fetch proves remote `main` still equals the Gate 6 direct parent;
- local repo clean;
- ahead/behind `1/0`;
- exact Gate 6 commit is a direct fast-forward descendant.

Publish with exactly one ordinary non-force:

```text
git push origin main:main
```

After publication require:

- fresh fetch;
- `HEAD == origin/main == FETCH_HEAD`;
- clean worktree/index;
- ahead/behind `0/0`.

Acceptance: **PASS / PUBLISHED**. Gate 6 commit `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`, tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`, direct parent `117607f508cd71cf679aa43888b2b05f8143272d`, subject `feat: add boot desktop UI foundation`. Gate 7 used one ordinary non-force `git push origin main:main`; fresh fetch proved `HEAD == origin/main == FETCH_HEAD == d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`, clean repository and ahead/behind `0/0`.

After Gate 7, exact next architecture/firmware boundary:

`OLED_DIRTY_REGION_OPTIMIZATION`

Acceptance intent for that next boundary is measurement-driven: preserve the single 512-byte framebuffer, dirty only bytes whose value actually changes, track bounded min/max changed columns per SSD1306 page, present only affected page/column spans, and record I2C traffic reduction. `APPLICATION_RUNTIME_FOUNDATION` follows after that optimization.

## Acceptance blockers

This boundary fails if it introduces any of the following:

- blocking 1 s boot delay before scheduler/IWDG startup;
- OLED render from SysTick/USB/UART IRQ or task1;
- third production task solely for UI;
- one-task-per-view/application design;
- generic timer/queue/mutex subsystem without a real additional consumer;
- application registry/lifecycle/event ABI implementation in this slice;
- fake network-online indication;
- wall-clock claim without RTC/synchronization source;
- command/RPC renumbering;
- USB descriptor/PMA redesign;
- steady-state runtime redraw that re-enters SSD1306 initialization or intentionally issues `display off`;
- host connection required for desktop transition;
- heap/filesystem/native executable loading;
- firmware update/bootloader work.
