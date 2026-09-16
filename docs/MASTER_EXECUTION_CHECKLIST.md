> [!IMPORTANT]

<!-- BEGIN STM32_OS_CURRENT_EXECUTION_STATE_2026_09_14 -->
## Current execution state — 2026-09-16

This section is authoritative.

### Published baseline — USB CDC ACM console foundation

- [x] `main` / `origin/main` / remote `main` = `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`.
- [x] published tree = `bbc6b24e28279075c41410e8abdace89c4b805b7`.
- [x] subject = `feat: add USB CDC ACM console foundation`.
- [x] published firmware candidate = `58548` bytes / `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`.
- [x] Windows `usbser`, physical reconnect, software-reset attach and post-IWDG automatic CDC recovery accepted.
- [x] final published repo state = clean, ahead/behind `0/0`.

### Current boundary — transport-neutral shell/RPC foundation

Boundary ID:

`SHELL_RPC_FOUNDATION`

Gate 0 planning decisions:

- [x] preserve UART and USB CDC as transport adapters, not command-policy owners;
- [x] add one static allocation-free command-service registry;
- [x] execution context owns generic response writer + opaque context + source RX-event mask;
- [x] semantic status taxonomy is `OK / NOT_FOUND / BAD_ARGS / BUSY / INTERNAL_ERROR`;
- [x] retain the existing `32`-byte command line capacity;
- [x] bounded maximum `4` argument tokens with in-place tokenization;
- [x] preserve all existing published command names and accepted response tokens;
- [x] authorize only `help` and `rpcinfo` as new foundation introspection methods;
- [x] retain independent UART/CDC partial-line parser state;
- [x] no binary framing, public numeric RPC IDs, host app, firmware update, bootloader, heap, new task/SVC/IPC, scheduler-core change, USB-driver change or OLED change.

Gate order:

- [x] Gate 0 planning/docs synchronization — **PASS**.
- [x] Gate 1 exact source investigation + command-service implementation — **PASS / SOURCE FINALIZED**.
- [x] Gate 2 fresh GNU build/link/static validation — **PASS**; candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`, BIN `37196` bytes / `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`.
- [x] Gate 3 UART + CDC shell/RPC hardware acceptance and retained regressions — **PASS**.
- [x] Gate 4 OLED conditional review — `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.
- [x] Gate 5 docs/evidence finalization — **PASS**.
- [ ] Gate 6 local acceptance commit — **CURRENT / NEXT**.
- [ ] Gate 7 ordinary non-force publication.

Accepted candidate/evidence:

- [x] candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`;
- [x] BIN `37196` bytes / `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- [x] Flash `37196` bytes / SRAM `9216` bytes;
- [x] Gate 2 evidence `34FB1B6D368A29AF5460174BBDA6AE81E479E5D11FBC0E25FDAC01617105BEA3`;
- [x] Gate 3 evidence `0EE7342129866C86A1AAFBA42E942E2097C78BFDF3CF50D0C93F3A6B71E9A4E9`;
- [x] Gate 3 final Flash readback exact; IWDG reboot/recovery `9042 ms`.

Permanent constraints retained:

- direct-register bare metal; no HAL/Arduino/FreeRTOS;
- UART remains emergency diagnostics;
- ST-LINK remains recovery/debug;
- no dual ST-LINK 3.3 V + micro-USB VBUS powering;
- UART adapter VCC remains disconnected;
- IWDG reload remains Thread/PSP production-progress owned and forbidden from USB/UART IRQ/Handler;
- existing two-task production topology remains authoritative;
- generic timers, IPC/queues/synchronization and runtime statistics remain deferred;
- binary framing remains the next transport boundary after this one;
- OLED/gfx/status-bar remain frozen.

Canonical design:
`docs/SHELL_RPC_FOUNDATION_PLAN.md`

Canonical acceptance plan:
`docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`
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
