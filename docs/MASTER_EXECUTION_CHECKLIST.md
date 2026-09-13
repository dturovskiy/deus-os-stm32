> [!IMPORTANT]

<!-- BEGIN STM32_OS_CURRENT_EXECUTION_STATE_2026_09_13 -->
## Current accepted execution state — 2026-09-13

This section is authoritative for the current execution boundary and supersedes stale unchecked rows in older planning sections below.

### Accepted runtime/UI baseline

- [x] Frozen native 128x32 OLED geometry remains unchanged.
- [x] Runtime UI boot lifecycle accepted and published.
- [x] Boot UART contract includes `OLED_RUNTIME_UI_OK`.
- [x] Full UART/OLED regression passed.
- [x] Physical OLED appearance/output accepted.

### Scheduler foundation / cooperative / preemption — ACCEPTED / PUBLISHED

- [x] Slice 9A foundation commit `1a57f79cda42674219e774900ce07a0da8fedaf4`.
- [x] Slice 9B cooperative activation commit `1114621e9a6bc57d5471cf51a922c216b76bebe2`.
- [x] PendSV timer-driven preemption commit `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.
- [x] `schedtest -> SCHED_FOUNDATION_OK`.
- [x] `schedcoop -> SCHED_COOP_OK`.
- [x] `schedpreempt -> SCHED_PREEMPT_OK`.
- [x] CPU-bound no-yield preemption path accepted across `34` complete runs.

### Task stack canary/high-water — ACCEPTED / PUBLISHED

- [x] Acceptance commit `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.
- [x] Existing task stacks remain exactly `2 x 512 bytes`.
- [x] Per-task canary/high-water telemetry is measured from Handler mode/MSP.
- [x] `schedstack -> SCHED_STACK_WATER_OK`.
- [x] Synthetic cooperative/preemptive task paths measured `72 / 512 bytes`.
- [x] Synthetic observed free margin `440 bytes`.
- [x] Canary intact across `34` stack-water commands / `68` scheduler runs.

### Substantive PSP workload — ACCEPTED / PUBLISHED

- [x] `schedworkload -> SCHED_WORKLOAD_OK`.
- [x] Workload task 0 runs frozen OLED runtime full render/present on PSP.
- [x] Workload task 1 is CPU-only with no UART/I2C/OLED access and no voluntary yield.
- [x] Public `scheduler_start_preemptive()` wrapper added.
- [x] Read-only PendSV switch-count telemetry added.
- [x] Current task stacks remain exactly `2 x 512 bytes`.
- [x] Planning feasibility estimate: `212 + 64 = 276 bytes`; margin `236 bytes`.
- [x] Source-build task-0 feasibility estimate: `220 + 64 = 284 bytes`; margin `228 bytes`.
- [x] Source-build task-1 feasibility estimate: `12 + 64 = 76 bytes`; margin `436 bytes`.
- [x] Runtime task-0 high-water: `328 bytes`; measured margin `184 bytes`.
- [x] Runtime task-1 high-water: `80 bytes`; measured margin `432 bytes`.
- [x] PendSV switches observed `124..126`.
- [x] `WORKLOAD_UI_RESULT=1` for every accepted workload run.
- [x] `WORKLOAD_PEER_OVERLAP=1` for every accepted workload run.
- [x] Both task canaries intact for every accepted workload run.
- [x] First workload passed.
- [x] 32/32 workload stress commands passed.
- [x] 4/4 return-to-kernel checkpoint pings passed.
- [x] Final workload passed.
- [x] Total accepted workload commands: `34`.
- [x] Full scheduler/UART/I2C/OLED regression passed.
- [x] Final exact flash identity passed.
- [x] Hardware v1 had exactly two false-negative boot matcher results due to stale `FAULTREC=0x20000270`.
- [x] Recovery v2 proved linked `fault_record=0x20000284`, validated both old boot captures, and passed two fresh corrected boot checks without reflashing.
- [x] Physical frozen OLED output confirmed `DEUS OS / BOOT OK / READY`.
- [x] Candidate binary `14292 bytes`.
- [x] SHA-256 `8C124B0954D65E0F698AD1C62525E72FC4F569D5133EF8F0CE67A8297A80A2CF`.
- [x] `.bss=1952 bytes`; `_ebss=0x200007A0`; SRAM headroom `18528 bytes`.
- [x] Normal boot / console / OLED steady-state scheduler migration remains deferred.
- [x] Substantive PSP workload acceptance commit published as `8f6b922a7d2e55abc3133702e7571057f995da5d`.

### Stack-analysis rule learned from this gate

- [x] Runtime task-0 high-water `328 bytes` exceeded the `.su`-based `284-byte` estimate.
- [x] Treat the current `.su` direct-call-chain calculation as feasibility/sanity evidence, not a conservative upper bound.
- [x] Runtime watermark/canary evidence is authoritative for stack sizing until the static method is strengthened.
- [x] A 512-byte PSP stack is accepted for the exact tested frozen OLED render/present workload with `184 bytes` measured margin.
- [x] Console-task PSP stack budget accepted at `1024 bytes` for the exact tested 17-command safe surface; high-water `600 bytes`, margin `424 bytes`.
- [x] Kernel/MSP stack budget measured separately and accepted with runtime watermark/canary evidence.

### Current acceptance source change set

Immediately before the console PSP stack-budget acceptance commit, the exact source delta is:

```text
 M include/kernel/scheduler.h
 M src/kernel.c
 M src/kernel/scheduler.c
```

### Production ownership decision — ACCEPTED

- [x] Current scheduler lifecycle identified as a global run-to-completion host launcher.
- [x] Scheduler wait/block/sleep states confirmed absent.
- [x] Scheduler self-tests confirmed to reinitialize global scheduler state.
- [x] Active-production nested scheduler diagnostics classified unsafe until separated.
- [x] Full current console linked feasibility estimate: `524 bytes`.
- [x] Full current console + 64-byte context reserve: `588 bytes`.
- [x] 512-byte PSP stack rejected for the full current console surface.
- [x] Static method remains feasibility-only after prior observed `44-byte` underprediction.
- [x] Normal-boot migration remained blocked.
- [x] First prerequisite selected: USART1 RX IRQ/ring-buffer foundation.

### USART1 RX IRQ / ring-buffer foundation — ACCEPTED / PUBLISHED

- [x] `USART1_IRQHandler` is the sole `USART1_DR` reader.
- [x] Vector table extended through external IRQ37 and linked USART1 handler verified.
- [x] RXNE interrupt enabled; USART1 NVIC priority `0x80`.
- [x] 128-byte SPSC RX ring added.
- [x] Existing `uart_try_getc()` retained as ring consumer API.
- [x] MSP-owned console retained; production scheduler not started.
- [x] `WFI` idle restored.
- [x] `rxstat -> RX_IRQ_RING_OK` added.
- [x] Candidate `15084 bytes`, SHA-256 `E25DC54C149EB9DA7B26F5378868DAA790847A1C4C7B4CFB970F294CFB738EFB`.
- [x] USART1 IRQ static frame `12 bytes`; SRAM headroom `18368 bytes`.
- [x] Four `32 x ping` burst rounds returned exact `32/32` PONG each.
- [x] Every round produced exact `+167` IRQ and `+167` byte deltas including the telemetry command.
- [x] Observed ring high-water `29 / 128 bytes`.
- [x] Zero RX drops and zero RX errors.
- [x] Ring depth returned to zero after every accepted burst.
- [x] Fresh-reset burst repeated exact `167` IRQ / `167` byte counters and high-water `29`.
- [x] Scheduler/workload/health/I2C/OLED regressions passed.
- [x] Final exact flash identity passed.
- [x] Physical OLED remained `DEUS OS / BOOT OK / READY`.
- [x] Normal-boot task migration remains deferred.
- [x] Full console 512-byte PSP stack remains not accepted.
- [x] Acceptance commit published as `53054e16c5b4dbb54626b0f080c9492629ccb285`.

### MSP runtime high-water / guard — ACCEPTED / PUBLISHED

- [x] Dedicated upper-SRAM MSP reservation: `2048 bytes` at `0x20004800..0x20005000`.
- [x] Bottom guard/canary: `64 bytes`; measurable capacity: `1984 bytes`.
- [x] Reset handler initializes guard + watermark before its first `BL kernel_main`.
- [x] `mspstat -> MSP_STACK_OK` added as read-only telemetry.
- [x] Candidate `15620 bytes`, SHA-256 `C15634184CAB7BA3C5CE503773EB7BA4BB45DDB4B32A3D399F6BE587C518D907`.
- [x] Initial high-water `400 bytes`; initial margin `1584 bytes`.
- [x] Accepted maximum MSP high-water `596 bytes`.
- [x] Minimum observed MSP margin `1388 bytes`, above the `512-byte` acceptance floor.
- [x] MSP guard canary remained intact.
- [x] `12/12` composite nested-pressure rounds passed across `oledstatus`, `schedpreempt`, and `schedworkload`.
- [x] Each composite included `16 x ping`; RX ring high-water reached `80 / 128 bytes` with zero drops/errors and depth zero after snapshots.
- [x] Fresh-reset MSP proof: `572 bytes` used / `1412 bytes` margin.
- [x] Final scheduler workload, health, I2C/OLED regressions and exact flash identity passed.
- [x] Physical OLED remained `DEUS OS / BOOT OK / READY`.
- [x] Console remains MSP-owned; production scheduler still not started during normal boot.
- [x] Normal-boot task migration remains deferred.
- [x] Acceptance commit published as `d3efae463cd65e087f0c1a3556de640105ed1b44`.

### Console PSP stack-budget foundation — HARDWARE + PHYSICAL ACCEPTED / COMMIT PENDING

- [x] Inactive-only external task-stack binding added.
- [x] Legacy internal scheduler stacks remain `2 x 512 bytes`.
- [x] Dedicated aligned external console probe stack is exactly `1024 bytes` at `0x20000048`.
- [x] `UART_COMMAND_CAPACITY=32`; longest command `schedconsoleprobe` is 17 characters.
- [x] Safe PSP surface is exact `17` non-scheduler commands; scheduler diagnostics are excluded.
- [x] Candidate `17028 bytes`, SHA-256 `13539E4F0C422FF0E3C373EF4167F3238FFA6D9A1B09F309C1A07787F2596C7F`.
- [x] `.bss=5224 bytes`; `_ebss=0x20000C68`; RAM gap below MSP `15256 bytes`; linked `fault_record=0x2000074C`.
- [x] `4/4` standalone probes passed.
- [x] `8/8` composite `schedconsoleprobe + 16 x ping` probes passed.
- [x] PSP high-water `600 / 1024 bytes`; minimum margin `424 bytes` >= `256-byte` floor.
- [x] Peer high-water `88 / 512 bytes`; maximum switches `693`; overlap `1`; both canaries intact.
- [x] Exact safe surface `17/17` completed.
- [x] RX high-water `80 / 128`; zero drops/errors; depth zero after accepted stress.
- [x] Exact `+105` IRQ / `+105` byte deltas in all `8/8` composites.
- [x] MSP high-water `360 / 1984`; minimum margin `1624`; canary intact.
- [x] Fresh-reset probe and exact fresh `+105` delta passed.
- [x] Legacy scheduler/health/I2C/OLED regressions and exact final flash identity passed.
- [x] Physical OLED remained `DEUS OS / BOOT OK / READY`.
- [x] `1024 bytes` accepted for the exact tested 17-command safe console workload.
- [x] Normal boot remains MSP-owned; production scheduler still not started.

### Planned native USB / host control direction

- [ ] STM32F103 USB Device core on PA11/PA12 without HAL.
- [ ] USB CDC ACM command/diagnostic console.
- [ ] Transport-neutral shell/RPC shared with UART.
- [ ] Binary transport after CDC semantics stabilize.
- [ ] Cross-platform Windows/Linux host application, provisional name **Deus OS CP** (`Deus OS Control Panel`).
- [ ] Product name may change later; branding must not define protocol architecture.
- [ ] UART remains emergency console; ST-LINK remains recovery/debug.

### Next active scheduler boundary

Implement and prove **production scheduler lifecycle / scheduler-diagnostic isolation** without changing normal-boot ownership. Scheduler diagnostics that reinitialize global scheduler state must not run inside an active production scheduler context. Keep the accepted `1024-byte` safe-console PSP workload budget as evidence; wait/block/wake semantics and normal-boot migration remain later gates.
<!-- END STM32_OS_CURRENT_EXECUTION_STATE_2026_09_13 -->

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
