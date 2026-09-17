# Deus OS — OLED Dirty-Region Optimization Acceptance Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT NEXT**

Boundary ID:

`OLED_DIRTY_REGION_OPTIMIZATION`

Canonical design:
`docs/OLED_DIRTY_REGION_OPTIMIZATION_PLAN.md`

## Gate 0 — architecture/source-boundary freeze

Required published prestate:

- `HEAD == origin/main == d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`;
- tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`;
- subject `feat: add boot desktop UI foundation`;
- clean worktree/index, ahead/behind `0/0`;
- accepted firmware candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`;
- BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`;
- Flash `41520 / 65536`, SRAM `9792 / 20480`;
- `PHYSICAL_OLED=PASS` from the preceding boundary.

Gate 0 freezes:

- one 512-byte framebuffer only; no shadow framebuffer;
- 16-bit min/max dirty X span for each of at most eight framebuffer pages;
- change-aware byte writes and no dirty state for no-op writes;
- partial SSD1306 page+column windows with retry-safe dirty clearing;
- bounded last-present transfer telemetry;
- component-dirty status rendering;
- no whole-frame clear/recompose for normal minute/SYSTEM/USB updates;
- exact eight-path initial source boundary from the canonical plan;
- incremental static SRAM budget <= 64 bytes over the accepted 9792-byte baseline;
- command/RPC/USB/scheduler/IWDG/geometry compatibility.

Acceptance: **PASS**. Read-only source review confirms the current page-only dirty mask, 128-column dirty-page transfers, direct aligned-renderer writes, full status-region clear and whole-frame runtime recomposition are the exact mechanisms to optimize. The planned eight-file boundary contains every known write/present/runtime ownership point; no new kernel primitive or second framebuffer is required.

## Gate 1 — implementation and deterministic static/self-test proof

Authorized source paths only:

```text
include/gfx/mono_fb.h
src/gfx/mono_fb.c
src/gfx/text_renderer.c
include/drivers/ssd1306.h
src/drivers/ssd1306.c
include/kernel/oled_status_bar.h
src/kernel/oled_status_bar.c
src/kernel.c
```

Required framebuffer proof:

- unchanged pixel/byte write leaves dirty mask/span unchanged;
- changed pixel expands only its page span;
- multiple writes monotonically expand `[min_x,max_x]` for that page;
- `mono_fb_clear()` dirties only bytes that actually changed to zero;
- `mono_fb_mark_all_dirty()` produces full-width valid spans;
- `mono_fb_clear_dirty()` clears both page bit and span metadata;
- aligned text fast path uses the common change-aware write path;
- generic and aligned renderer framebuffer results remain byte-equivalent.

Required SSD1306 proof:

- dirty page window uses exact dirty `x0..x1`, not `0..127`;
- short final data chunk is bounded correctly;
- clean present issues no window/data write;
- page/span metadata clears only after complete successful transfer;
- failed window/data transfer leaves failed span retryable;
- full-present path remains available and unchanged in semantics;
- present telemetry counts exact payload bytes/writes/pages.

Required UI proof:

- status reference self-test remains exact;
- status time/indicator changes mark only their component;
- normal minute/SYSTEM/USB updates do not call whole-frame `mono_fb_clear()`;
- normal status-only update does not rerender console content;
- full compose remains allowed only for initial render, lifecycle state transition, explicit restore or recovery;
- 250 ms unchanged wake remains zero-render/zero-I2C;
- no new task/IRQ writer/lock.

No flash in Gate 1.

## Gate 2 — fresh build/link/resource and deterministic transfer metrics

Require a fresh exact-source build with the accepted Arm GNU toolchain and `-Wall -Wextra -Werror`.

Required:

- 16 C + 1 ASM topology unless explicitly justified by Gate 1;
- undefined symbols `0`;
- command registry exactly `32` and public RPC IDs `0x0001..0x0020` unchanged;
- USB descriptors/PMA/startup/linker hashes unchanged;
- Flash <= `65536` bytes;
- static SRAM <= `9856` bytes (`9792 + 64` optimization-metadata budget);
- task0/task1 stack capacities remain `1024 / 512` bytes;
- stack static analysis shows no new unbounded local allocation;
- `git diff --check` PASS;
- exact BIN/ELF/MAP/source hashes recorded;
- source candidate tree recorded before flash.

Deterministic transfer-metric proof:

- clean present: `0` writes / `0` payload bytes;
- full-width one-page compatibility case remains bounded and correct;
- `00:00 -> 00:01`: only page 0, span within `x=123..125`, <= `11` I2C payload bytes;
- USB RING/FILLED transition: only page 0, span within `x=8..12`, <= `13` I2C payload bytes;
- baseline comparison records the prior `572`-payload-byte four-page semantic refresh;
- reduction percentages are evidence, not marketing claims.

Gate 2 evidence must bind the exact candidate used by Gate 3.

## Gate 3 — hardware/runtime acceptance

Flash only the exact Gate 2 candidate; read back target Flash first and skip erase/program if already exact.

Retained regressions:

- boot/scheduler/IWDG startup;
- `ping`, `uptime`, `health`, `cdcstat`, `mspstat`, `schedprod`, `schedprio`, `fault`, `i2cscan`, `oledping`, `oledstatus`, `oleddirty`, `oleduiupdate`, `uiruntime`, `help`, `rpcinfo`;
- task0/task1 stack canaries and >=256-byte margins;
- production faults zero;
- CDC pressure `128/128`, zero drops;
- UART pressure `128/128`, zero drops/errors;
- binary RPC pressure/correlation and malformed/CRC/oversize/split recovery;
- physical USB disconnect/reconnect;
- authorized IWDG reboot and automatic UART/CDC/binary recovery;
- final exact Flash readback.

Optimization-specific hardware proof:

- `oleddirty`/equivalent accepted diagnostic proves a narrow dirty span is the only transferred panel region;
- unchanged redraw attempt reports zero present writes;
- deterministic status time case reports <=11 payload bytes;
- deterministic USB indicator case reports <=13 payload bytes;
- diagnostic screen is restored with `uiruntime`;
- no panel reinitialization/display-off occurs on normal optimized updates.

## Gate 4 — mandatory physical OLED regression

Physical review is mandatory because the panel transfer mechanism changes.

Require explicit `PHYSICAL_OLED=PASS` after observing:

- splash -> home remains correct and non-flickering;
- normal minute rollover changes only the expected digits visually, with no blank/off pulse;
- USB indicator transition remains correct, with no surrounding corruption;
- no stale pixels in old glyph/icon locations;
- no page/column clipping artifacts at dirty-span boundaries;
- console text remains unchanged during status-only updates;
- `uiruntime` full restore remains visually correct.

## Gate 5 — docs/evidence finalization

Synchronize at minimum:

- `README.md`;
- `CHANGELOG.md`;
- `docs/ARCHITECTURE.md`;
- `docs/IMPLEMENTATION_PLAN.md`;
- `docs/MASTER_EXECUTION_CHECKLIST.md`;
- `docs/PROJECT_HANDOFF.md`;
- `docs/ROADMAP.md`;
- this plan and its canonical design plan;
- `docs/OLED_UI_ACCEPTED_BASELINE.md` with a post-baseline optimization record.

Record exact candidate/resource hashes, transfer metrics before/after, retained hardware regressions and physical OLED disposition.

Acceptance: **PASS**. Gate 5 additionally adds canonical `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md` and repository `.gitattributes` LF policy. No firmware source/build/flash change is part of Gate 5.

Accepted evidence record:

- candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- ELF `71308` bytes / `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`;
- MAP `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`;
- Gate 2 evidence `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`;
- Gate 3 log `887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6`;
- Gate 3 evidence `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`;
- Gate 4 evidence `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`;
- Gate 4 `PHYSICAL_OLED=PASS`;
- full semantic baseline `572` payload bytes / `36` writes; clean `0`; narrow `9`; minute `11`; USB `11`;
- task0/task1 post-diagnostic margins `328 / 424` bytes;
- final Flash readback exact and all retained CDC/UART/binary/reconnect/IWDG regressions PASS.

## Gate 6 — local acceptance commit

Require exact accepted source/docs path set, complete evidence, `PHYSICAL_OLED=PASS`, no unreviewed paths, `git diff --cached --check` PASS, one local commit, clean repo afterward, remote at direct parent and ahead/behind `1/0` before push.

## Gate 7 — ordinary non-force publication

Fresh-fetch remote state must still equal the Gate 6 parent. Publish with exactly one ordinary non-force `git push origin main:main`. After fresh fetch require `HEAD == origin/main == FETCH_HEAD`, clean repo and ahead/behind `0/0`.

After Gate 7, exact next boundary:

`APPLICATION_RUNTIME_FOUNDATION`

## Acceptance blockers

The boundary fails if it introduces a second framebuffer, heap, DMA solely for OLED optimization, new task/SVC/IPC primitive, layout/geometry change, command/RPC renumbering, USB redesign, unconditional page/full-frame transfer for a narrow steady-state update, stale dirty span after successful present, loss of retry state after failed present, visible clipping/stale pixels, or static SRAM growth above the 64-byte optimization-metadata budget.
