> [!IMPORTANT]

<!-- BEGIN STM32_OS_CURRENT_EXECUTION_STATE_2026_09_14 -->
## Current execution state — 2026-09-17

This section is authoritative.

### Published baseline — binary framed transport foundation

- [x] `main` / `origin/main` / remote `main` = `2fde9025a51021511e73a76b561f7983ca655e2f`.
- [x] published tree = `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`.
- [x] subject = `feat: add binary framed transport foundation`.
- [x] accepted firmware candidate = `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`.
- [x] tested source candidate tree = `c2c3d9743c23ab02329a9652714862fafb5bb17c`.
- [x] USB CDC text + binary RPC, malformed-frame recovery, pressure, reconnect and post-IWDG recovery accepted.
- [x] final published repo state = clean, ahead/behind `0/0`.

### Published boundary — binary framed transport foundation

Boundary ID:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Gate 0 planning decisions:

- [x] binary v1 rides over the existing USB CDC byte stream; UART remains text-only emergency diagnostics;
- [x] text/binary CDC demux reserves magic `A5 5A` and preserves partial text-shell state across complete binary frames;
- [x] exact versioned envelope uses request IDs, explicit payload length and little-endian integer fields;
- [x] integrity is CRC-16/CCITT-FALSE (`poly 0x1021`, init `0xFFFF`);
- [x] stable public 16-bit RPC IDs `0x0001..0x0020` are separate from internal dispatch enum ordinals;
- [x] max `4` arguments, max `31` bytes each, max RPC request payload `132` bytes;
- [x] response output is chunked; max data chunk `48` bytes gives an exact `64`-byte maximum RPC_DATA wire frame;
- [x] `HELLO_REQUEST/RESPONSE`, `RPC_REQUEST`, `RPC_DATA`, `RPC_END`, `PROTOCOL_ERROR` are the only v1 frame types;
- [x] destructive RPC requires explicit `ALLOW_DESTRUCTIVE` request flag;
- [x] binary frame TX requires nonblocking all-or-none CDC ring enqueue; no indefinite busy wait;
- [x] command semantics remain owned by the published command service; binary adapter must reuse `command_service_execute()`;
- [x] no heap, new task, SVC, IPC/timer subsystem, USB descriptor/PMA redesign, firmware update, bootloader, host GUI or OLED change.

Gate order:

- [x] Gate 0 planning + normative protocol/docs synchronization — **PASS**.
- [x] Gate 1 exact source investigation + binary framing/RPC implementation — **PASS / SOURCE IMPLEMENTED**.
- [x] Gate 2 fresh GNU build/link/static validation — **PASS**.
- [x] Gate 3 USB CDC binary + retained text/UART hardware acceptance — **PASS**.
- [x] Gate 4 OLED conditional review — **PASS / `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`**.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [x] Gate 6 local acceptance commit — **PASS**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED `2fde9025a51021511e73a76b561f7983ca655e2f`**.

Accepted Gate 2/3 candidate and evidence:

- [x] tested candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- [x] BIN `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- [x] ELF `65876` bytes / `4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B`;
- [x] Flash `40720 / 65536`, SRAM `9752 / 20480`;
- [x] binary safe surface `21/21`, binary scheduler BUSY `8/8`, binary pressure `128/128` unique IDs;
- [x] retained CDC pressure `128/128`, UART race `128/128`, physical reconnect PASS, post-IWDG text+binary recovery PASS;
- [x] final Flash readback exact;
- [x] Gate 2 evidence `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- [x] Gate 3 log `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- [x] Gate 3 evidence `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`.

### Published boundary — OS application and UI model foundation

Boundary ID:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

Gate order:

- [x] Gate 0 product/application/UI architecture freeze — **PASS**.
- [x] Gate 1 published-source capability inventory — **PASS / READ-ONLY**.
- [x] Gate 2 architecture consistency review — **PASS**.
- [x] Gate 3 hardware acceptance — **PASS / `N/A_DOCS_ONLY`**.
- [x] Gate 4 OLED review — **PASS / `PHYSICAL_OLED=N/A_DOCS_ONLY_NO_FIRMWARE_CHANGE`**.
- [x] Gate 5 documentation finalization — **PASS**.
- [x] Gate 6 local docs acceptance commit — **PASS / `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`**.
- [x] Gate 7 ordinary non-force publication — **PASS / PUBLISHED**.

Gate 0 freezes:

- Deus OS as an independently operating deterministic embedded device runtime;
- static firmware-linked applications with explicit IDs/lifecycle for v1;
- no arbitrary uploaded ARM executables or one-task-per-app requirement;
- splash -> desktop/home -> application-view UI lifecycle;
- real SYSTEM / USB / NETWORK status indicators;
- initial status time source = uptime `HH:MM`;
- firmware ownership versus future Control Panel management-plane ownership;
- target apps versus host plugins versus transferable non-executable packages;
- scheduler wake bits remain internal kernel notification state, not the public application-event ABI;
- `APPLICATION_RUNTIME_FOUNDATION` must define bounded semantic app events/services;
- future host tooling requires explicit firmware/build/platform/service/application identity/capability discovery;
- persistent Flash state requires version/integrity/atomic-commit/recovery/wear semantics before acceptance;
- current BSS `fault_record` is not reset-persistent; retained previous-boot crash/reset diagnostics remain later observability work;
- CRC/destructive authorization flags are not authentication; trust-sensitive network/update boundaries require explicit security review;
- portability must keep arch/platform/driver details below kernel/services/apps/UI/protocol semantics without speculative universal HAL work;
- timers/queues/synchronization/runtime statistics and heap/filesystem/RTC/DMA/MPU/power frameworks remain consumer-driven/deferred;
- exact next implementation order beginning with `BOOT_DESKTOP_UI_FOUNDATION`.

Canonical design:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`

Canonical acceptance:
`docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`

Canonical foundation gap review:
`docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

Gates 0–7 are accepted. Gate 6 committed exactly the accepted docs-only path set at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`; Gate 7 published it by ordinary non-force fast-forward. The next implementation boundary is `BOOT_DESKTOP_UI_FOUNDATION`.

Permanent constraints retained:

- direct-register bare metal; no HAL/Arduino/FreeRTOS;
- UART remains text emergency diagnostics;
- ST-LINK remains recovery/debug;
- no dual ST-LINK 3.3 V + micro-USB VBUS powering;
- UART adapter VCC remains disconnected;
- IWDG reload remains Thread/PSP production-progress owned and forbidden from USB/UART IRQ/Handler;
- existing two-task production topology remains authoritative;
- USB IRQ remains bounded hardware/ring/event ownership only;
- generic timers, IPC/queues/synchronization and runtime statistics remain deferred;
- OLED/gfx/status-bar remain frozen.

Canonical protocol:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

Canonical design:
`docs/BINARY_FRAMED_TRANSPORT_PLAN.md`

Canonical acceptance:
`docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`
<!-- END STM32_OS_CURRENT_EXECUTION_STATE_2026_09_14 -->

> **OLED UI status: ACCEPTED / FROZEN (2026-09-11).**
> The authoritative hardware-accepted geometry and firmware fingerprint are in
> [`OLED_UI_ACCEPTED_BASELINE.md`](OLED_UI_ACCEPTED_BASELINE.md).
> Any configurable-layout, preset, custom-layout, persistence, or alternate-geometry
> material below is deferred planning and must not override the accepted baseline.
# Master Execution Checklist

This file is the canonical execution gate for the STM32 OS project.

## Current hardware baseline

MCU:

- STM32F103C8T6 / medium-density Cortex-M3
- 64 KiB flash
- 20 KiB SRAM

OLED:

- native `128x32`
- SSD1306-compatible
- I2C address `0x3C`
- framebuffer `512 bytes`
- four pages
- `A8=0x1F`
- `DA=0x02`
- `D3=0`
- start line `0`
- `A1/C8`
- full window columns `0..127`, pages `0..3`

The former 128x64 assumption is obsolete.

## Accepted OLED milestones

- I2C scan / address proof.
- SSD1306 command proof.
- visible output proof.
- SSD1306 driver extraction.
- framebuffer/font extraction.
- raw mapping calibration.
- native 128x32 geometry proof.
- 1x font/native full-frame baseline.
- generic opaque renderer.
- byte-equivalent aligned renderer fast path.
- retained 21x3 console without scrolling.
- accepted status-bar reservation/layout.

Current accepted Slice 4 evidence:

- image size `5680 bytes`
- SHA-256
  `C67AEBA137F645F5BECAEB6410382D44257E1B09EA3BE459BD83AC718F3A09AC`
- framebuffer `512 bytes`
- retained console state `67 bytes`
- physical layout accepted

## OLED console canonical plan

<!-- OLED_CONSOLE_ARCHITECTURE_PLAN_CANONICAL -->

Read together:

- `docs/OLED_SSD1306_HARDWARE_PROFILE.md`
- `docs/OLED_CONSOLE_ARCHITECTURE.md`
- `docs/OLED_CONSOLE_API_CONTRACT.md`
- `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md`
- `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md`
- `docs/OLED_STATUS_BAR_PLAN.md`
- `docs/OLED_UI_LAYOUT_PLAN.md`

The accepted Slice 4 framed geometry is now a compatibility/reference preset,
not a permanently hardcoded production appearance.

Canonical framed UI geometry:

```text
frame:             y=0 and y=31
status content:    y=1..4
status separator:  y=5
status/console gap:y=6
console viewport:  x=1, y=7, width=126, height=23
console rows:      y=7,15,23
bottom gap:        y=30
font:              5x7
cell advance:      6x8
capacity:          21x3
```

No hidden Y remap.
No 2x text workaround.
No 128x64 assumptions.
Console UI is not aligned to SSD1306 pages.

## Current implementation sequence

- [x] Slice 0: restore known-good baseline.
- [x] Slice 1: isolate SSD1306 driver.
- [x] Slice 2: isolate monochrome framebuffer and font.
- [x] Hardware correction: identify and prove native 128x32 panel profile.
- [x] Slice 3A: native 128x32 source + canonical documentation.
- [x] Slice 3B: generic opaque text renderer.
- [x] Slice 3C: framebuffer-equivalent aligned fast path.
- [x] Slice 4: retained 21x3 console without scrolling + accepted reference UI.
- [ ] Slice 4B: configurable UI layout + status bar.
  - [x] 4B.0: initial status architecture/acceptance plan.
  - [x] 4B.0a: static status prototype protocol/isolation proof.
  - [x] 4B.0b: visually reject hardcoded full-frame composition; do not commit it.
  - [x] 4B.0c: define configurable layout/preset/customization plan.
  - [ ] 4B.1: add validated `oled_ui_layout` module.
  - [ ] 4B.1: add `minimal`, `boxed`, `compact` presets.
  - [ ] 4B.1: move borders/separator/regions into layout data.
  - [ ] 4B.1: keep 21x3 console with glyph-based horizontal fit.
  - [ ] 4B.1: retain 3x4 status font + COMM/UART + static `12:34`.
  - [ ] 4B.1: add runtime preset selection.
  - [ ] 4B.1: physically compare at least minimal vs boxed.
  - [ ] 4B.1: accept one or more presets, then commit/push.
  - [ ] 4B.2: add `ui show` and validated RAM-only `ui set`.
  - [ ] 4B.2: prove atomic rejection of invalid custom layouts.
  - [ ] 4B.3: integrate uptime `HH:MM`.
  - [ ] 4B.3: update only on displayed minute/COMM changes.
  - [ ] 4B.4: define PC interchange/converter/configurator protocol.
  - [ ] 4B.4: send layout through UART first.
  - [ ] 4B.4: later reuse the same API through USB CDC.
  - [ ] 4B.5: optional versioned Flash persistence after runtime semantics stabilize.
- [ ] Slice 5: circular 3-row scroll.
- [ ] Slice 6: dirty-page presentation optimization.
- [ ] Slice 7: optional kernel-log integration.

## Development loop

For every hardware-visible change:

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART
 -> physical visual
 -> PASS/FAIL
 -> evidence
```

Do not skip physical acceptance for OLED rendering/layout changes.

## Build discipline

- `-Wall -Wextra -Werror`
- `git diff --check`
- no warning suppression
- no implicit package installation
- no unrelated Git changes
- dynamic ELF symbol resolution
- fail closed on unexpected tree state

## Flash discipline

Close STM32CubeProgrammer GUI before CLI access.

SWD fallback order:

```text
4000 KHz
1000 KHz
400 KHz
```

Once flashing begins, source must remain matched to the flashed image.

## UART regression

Minimum:

```text
BOOT OK
ping -> PONG
```

For OLED work also require:

```text
i2cscan -> 0x3C
oledping -> OLED_CMD_OK
```

Rendering regressions:

```text
oledtext    -> OLED_TEXT_OK
oledrender  -> OLED_RENDER_EQ_OK + OLED_RENDER_OK
oledconsole -> OLED_CONSOLE_OK
```

## Fault diagnostics

`fault_record` resides in BSS and is cleared on reset.

Its address is build-dependent. Tooling must resolve it from the current ELF.

OLED must never be the only fault-reporting sink.

## OLED UI customization workflow

Visual layouts should be designed at exact `128x32` resolution with a
one-pixel grid.

Recommended workflow:

```text
Aseprite / LibreSprite / Piskel / equivalent
 -> 128x32 mockup
 -> select geometry/assets
 -> encode as preset/custom config
 -> target validation
 -> hardware preview
 -> physical accept/reject
```

The STM32 does not decode PNG/SVG/Figma files directly.

PC tooling converts layouts/assets into target configuration or packed 1-bit
bitmaps.

Runtime configuration is transport-independent:

```text
UART now
USB CDC later
```

No keyboard or mouse needs to be physically connected to the STM32 for normal
configuration.

## OLED UI freeze checkpoint — ACCEPTED 2026-09-11

This checkpoint supersedes earlier open OLED UI-layout planning entries.

- [x] Native 128x32 panel profile accepted.
- [x] Retained semantic console accepted.
- [x] Status bar exact reference accepted.
- [x] One-pixel clock right inset accepted.
- [x] One-pixel blank row below status bar accepted.
- [x] Three console rows accepted with one-pixel inter-row gaps.
- [x] Console remains 21x3 using compact `5x6` glyphs / `6x7` cells.
- [x] No side/bottom frame below the status bar.
- [x] Exact accepted binary reproduced before commit:
  `7396 bytes`,
  `4DDA68DA96215F6FC2960007B37AB5F8808BA9284EC8A726AEFFFC4B97FCF9C4`.
- [x] Full UART/OLED regression passed.
- [x] Physical OLED appearance accepted.
- [x] UI styling frozen.

Deferred, not blocking the current roadmap:

- [ ] Runtime/custom layout editing — deferred.
- [ ] PC configurator/import — deferred.
- [ ] UI persistence — deferred.
- [ ] Uptime/RTC clock behavior — deferred.

- [x] Center status-bar field reserved for notifications:
  `x=20..107, y=2..6`, with guard columns `x=19` and `x=108`.
- [ ] Notification rendering/queue semantics — deferred to dedicated subsystem.

### Slice 5 — circular retained console scroll

- [x] Rotate retained rows through `first_row`.
- [x] Reuse/clear the old physical top row as the new bottom row.
- [x] Preserve pending-wrap / pending-next-line semantics.
- [x] No framebuffer `memmove` as the scrolling mechanism.
- [x] No SSD1306 hardware scroll.
- [x] `OLED_SCROLL_STATE_OK`.
- [x] `OLED_SCROLL_OK`.
- [x] Physical proof accepted: `SCROLL TWO / SCROLL THREE / SCROLL FOUR`.
- [x] Frozen UI implementation remains unchanged.

### Slice 6 — dirty-page SSD1306 present

- [x] Add `ssd1306_present(mono_fb_t *fb)`.
- [x] Send only pages selected by `mono_fb_dirty_pages()`.
- [x] Treat zero dirty pages as a successful no-op.
- [x] Clear each dirty bit only after that page transfers successfully.
- [x] Preserve failed/later pages for retry.
- [x] Keep `ssd1306_present_full()` for full-refresh/diagnostic use.
- [x] Physical proof accepted: only `y=16..23` became white while clean RAM pages also contained `0xFF`.
- [x] `OLED_DIRTY_MASK_OK`.
- [x] `OLED_DIRTY_CLEAR_OK`.
- [x] `OLED_DIRTY_IDLE_OK`.
- [x] `OLED_DIRTY_OK`.
- [x] Frozen UI implementation remains unchanged.

**Historical note:** the dirty-page runtime-integration boundary was completed before Slice 8; see the current accepted execution state above.
## Slice 7 — dirty-page UI integration — ACCEPTED

Status: **accepted**

Acceptance criteria completed:

- [x] Shared status/console UI path uses `ssd1306_present()`.
- [x] Scroll UI path uses `ssd1306_present()`.
- [x] Retained console dirty rows are consumed after rasterization.
- [x] Clean logical console rows are skipped during incremental render.
- [x] Row-1-only update produces framebuffer dirty mask `0x04`.
- [x] Dirty-page present clears the page mask after successful transfer.
- [x] Frozen status-bar/layout implementation remains unchanged.
- [x] Full UART regression suite passes.
- [x] Flash/verify/reset passes at SWD 4000 KHz.
- [x] Physical OLED result accepted.

Accepted firmware fingerprint:

- binary size: 9828 bytes
- SHA-256: `4015795457F6844EFA768F97E7C59C8170015F147199874B61B524A1289AA5E8`

Next boundary: move dirty-page presentation from acceptance/demo commands into the normal runtime UI lifecycle without reopening frozen geometry.

## Slice 8 — normal runtime OLED boot UI — ACCEPTED 2026-09-12

Status: **accepted**

Acceptance criteria completed:

- [x] Normal reset initializes the frozen product UI automatically after the UART boot banner.
- [x] Runtime composition reuses the accepted `oled_ui_layout_default()`, status renderer, retained console, and `ssd1306_present()` path.
- [x] Frozen status-bar/layout/font implementation files remain unchanged.
- [x] Runtime UI shows the frozen status bar plus `DEUS OS`, `BOOT OK`, and `READY`.
- [x] The proof clock remains `00:00`; uptime/RTC behavior stays deferred.
- [x] `uiruntime` restores the same product UI after explicit diagnostic screens.
- [x] Boot emits `OLED_RUNTIME_UI_OK` after successful OLED initialization/presentation.
- [x] Full UART/OLED regression suite passes.
- [x] Flash/verify/reset passed at SWD 4000 KHz in the acceptance run.
- [x] Final physical OLED appearance accepted.

Accepted firmware fingerprint:

- binary size: 10148 bytes
- SHA-256: `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`
- `.text`: 10148 bytes
- `.data`: 0 bytes
- `.bss`: 716 bytes
- framebuffer: 512 bytes
- retained console state: 67 bytes

Slice 8 closes the transition from acceptance/demo UI commands to the normal
runtime boot lifecycle. Frozen UI geometry and styling remain closed; select the
next non-UI/system slice separately.

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_MASTER_ACCEPTED_20260913 -->
## Production scheduler steady-state wait/wake foundation — ACCEPTED 2026-09-13

Status: **hardware and physical acceptance complete**

Acceptance criteria completed:

- [x] Explicit runnable vs blocked state exists through `SCHEDULER_TASK_BLOCKED`.
- [x] Task wait path uses `scheduler_wait_events()` through SVC #3.
- [x] ISR-side event signalling can wake blocked tasks safely.
- [x] Pending-event semantics close the check-vs-block lost-wakeup race.
- [x] When no task is runnable but blocked work remains, scheduler ownership parks on preserved host MSP with `WFE`; it does not busy-spin and does not falsely finish the run.
- [x] UART RX event publication occurs after ring-buffer byte publication.
- [x] Event consumer re-checks RX FIFO after wake and separates CR/LF protocol framing from payload.
- [x] Four UART IRQ wait/wake hardware rounds pass with raw sentinel `0x57`.
- [x] Positive idle-WFE count is proven in every round.
- [x] Task canaries remain intact; observed diagnostic task high-water stays within 512-byte task stacks.
- [x] Lifecycle isolation blocks all seven invasive scheduler diagnostics while active.
- [x] Existing foundation/cooperative/preemptive/stack/workload/console-probe diagnostics pass before and after wait/wake proof.
- [x] USART1 RX ring reports zero drops and zero errors after the complete regression.
- [x] MSP guard/canary and margin remain healthy.
- [x] Frozen OLED runtime regression passes.
- [x] Final physical OLED appearance is operator-confirmed PASS.
- [x] Final target Flash readback exactly matches the accepted candidate.

Accepted firmware fingerprint:

- path: `build\scheduler_wait_wake_foundation_v2\os.bin`
- bytes: `19932`
- SHA-256: `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`

Accepted evidence:

- source/build log SHA-256: `220316AF92C3328F9B0D4F850B6EBF9F09A345CD673AAD81A301ED0E9ADD0219`
- source/build ZIP SHA-256: `A6A449669CFDD401E569578C40D91169C8FC8E7A5CF557600E945FE92AAF2D3B`
- hardware log SHA-256: `970656981374C7252A96748B5EF16A17600ABAFDFE73624870DE4C73F33C484A`
- hardware ZIP SHA-256: `C4FD9B7774934102A95F4A66C5BF7AC586D0A85C360BA9DF34C7C1315A1B9590`
- physical OLED: `PASS_OPERATOR_CONFIRMED_2026-09-13`

Procedural rulebook for host scripts, evidence and recovery:
`docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`

Publication is handled by separate commit and non-force push gates.

**Next implementation boundary after publication:** normal-boot production task ownership / migration. Do not fold `sleep()` / timer integration or priorities into that migration gate.
<!-- END STM32_OS_SCHED_WAIT_WAKE_MASTER_ACCEPTED_20260913 -->
