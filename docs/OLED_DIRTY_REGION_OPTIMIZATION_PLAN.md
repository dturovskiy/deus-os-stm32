# Deus OS — OLED Dirty-Region Optimization Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `39690c9ef103cbcf93272df8bad0359a934b7dc1`**

Boundary ID:

`OLED_DIRTY_REGION_OPTIMIZATION`

Published repository prestate:

- `main == origin/main` at `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`;
- tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`;
- subject `feat: add boot desktop UI foundation`;
- worktree/index clean, ahead/behind `0/0`;
- accepted firmware candidate tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`;
- accepted BIN `41520` bytes / SHA-256 `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`;
- Flash `41520 / 65536`, SRAM `9792 / 20480`;
- physical OLED disposition `PHYSICAL_OLED=PASS`.

## 1. Purpose

Reduce framebuffer CPU work and SSD1306/I2C traffic so normal Deus OS UI updates touch only bytes whose final visible value actually changes. Preserve the accepted 128x32 geometry, status semantics, splash/home lifecycle, command ABI and two-task runtime topology.

This is a measured optimization boundary, not a UI redesign.

## 2. Current cost that justifies the boundary

The current framebuffer tracks only an 8-bit dirty-page mask. `ssd1306_present()` therefore sends all 128 columns of every dirty 8-pixel page. One dirty page costs one 7-byte SSD1306 window payload plus eight 17-byte data payloads, or 143 I2C payload bytes. Four dirty pages cost 572 payload bytes.

The accepted boot/desktop runtime correctly suppresses a redraw when no semantic state changes, but a real minute/indicator update still enters a full framebuffer clear/recomposition path. The panel no longer blanks, but the transfer granularity is still much larger than the actual visible change.

## 3. Memory model

Keep exactly one 512-byte framebuffer. A second/shadow framebuffer is forbidden in this boundary.

Extend `mono_fb_t` with bounded horizontal dirty-span metadata for its existing maximum eight pages:

```text
dirty_pages
page 0: dirty_min_x / dirty_max_x
...
page 7: dirty_min_x / dirty_max_x
```

Use 16-bit X bounds so the gfx module does not acquire an artificial 255-pixel width limit. Eight min/max pairs add exactly 32 bytes to each `mono_fb_t`; production has one static framebuffer object.

Expected persistent optimization metadata, including SSD1306 present telemetry and runtime status state, must add no more than 64 bytes static SRAM over the accepted 9792-byte baseline. No heap allocation is allowed.

## 4. Change-aware framebuffer writes

All framebuffer writes that participate in dirty tracking must compare old and new byte values.

Required rules:

- setting a pixel to its existing value produces no dirty state;
- clearing an already-zero byte produces no dirty state;
- a real byte change marks the containing page and expands only that page's `[min_x,max_x]` span;
- `mono_fb_mark_all_dirty()` marks every valid page with the full `[0,width-1]` span;
- clearing a dirty page also resets its span metadata;
- invalid/clean pages never expose a stale span.

Add one bounded masked-page-byte helper so direct byte writers use the same change detection as `mono_fb_set_pixel()`. The aligned text-renderer fast path must use that helper rather than writing `fb->data[]` and unconditionally dirtying a page.

## 5. SSD1306 partial-window presentation

Replace the page-only window helper used by `ssd1306_present()` with a bounded page+column window:

```text
page p, columns x0..x1
```

For each dirty page:

1. validate its dirty span;
2. program `0x21 x0 x1` and `0x22 p p`;
3. send only bytes `x0..x1`, still using bounded 16-byte data chunks;
4. allow a short final chunk;
5. clear that page/span only after the entire span transfers successfully.

If a window or data write fails, the failed page/span remains retryable. Later dirty pages remain pending. A clean framebuffer remains a successful zero-I2C no-op.

`ssd1306_present_full()` remains available for diagnostics/recovery and keeps its current full-frame semantics.

## 6. Measurement telemetry

Add bounded read-only statistics for the most recent `ssd1306_present()` call. The telemetry must account for at least:

- window payload bytes;
- data payload bytes including each SSD1306 data-control byte;
- I2C write count;
- presented page count.

The telemetry is diagnostic only; it must not alter rendering policy and must remain bounded static state.

Baseline comparison for the current four-page semantic refresh is 572 I2C payload bytes.

Deterministic optimization targets:

- clean/unchanged present: `0` I2C writes / `0` payload bytes;
- `00:00 -> 00:01` status-time update: page 0 only, dirty span contained in the final 3-pixel digit area `x=123..125`, at most 11 I2C payload bytes (`7` window + `1` data-control + `3` framebuffer bytes);
- one USB icon transition: page 0 only, span contained in `x=8..12`, at most 13 I2C payload bytes.

These are acceptance targets, not estimates.

## 7. Status-bar differential rendering

The current status renderer clears and redraws the whole 128x9 reference region. That is incompatible with final-change-only traffic without a shadow framebuffer.

Retain the exact accepted visual result but introduce component dirty state for:

- frame/background;
- SYSTEM indicator;
- USB indicator;
- NETWORK indicator;
- uptime field.

Initial/full restore marks all components dirty. `oled_status_bar_set_time()` dirties only the time component when time changes. Indicator setters dirty only indicators whose semantic value changes. Component rendering must be opaque inside its own fixed region so an old glyph/icon is fully replaced without clearing unrelated status pixels.

The historical exact status reference self-test remains mandatory.

## 8. Runtime UI differential composition

`boot_desktop_ui_render()` must stop using whole-frame clear/recomposition for a normal minute/SYSTEM/USB change.

Required policy:

- first bootstrap render: full composition allowed;
- splash -> home lifecycle transition: full content-region composition allowed;
- explicit `uiruntime` restore: full composition allowed;
- panel-transfer recovery: full composition/reinitialization allowed;
- normal minute change: status time only;
- normal SYSTEM change: SYSTEM icon only;
- normal USB change: USB icon only;
- NETWORK remains inactive in the current product state;
- unchanged 250 ms service wake: zero framebuffer mutation, zero I2C traffic;
- console content is not rerasterized when only status state changed.

Task0 / Thread-PSP remains the only normal runtime writer. No IRQ/task1 rendering is introduced.

## 9. Exact initial source boundary

Gate 1 is initially authorized to modify only:

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

Published SHA-256 guards at Gate 0:

```text
include/gfx/mono_fb.h              3396CC73FDFB37D2201CBFA77DD16CED382A1F9395370A5671BA007C4EC67F57
src/gfx/mono_fb.c                  D0CFBD178AAFDF1911BC9D22E179C69C6586F754BB115C24ED47F5E39A1B555E
src/gfx/text_renderer.c            5E74C2F504B3E506F1CC86C49F45A1AD461DDD880D278F81D5B67E4E49BEE6E9
include/drivers/ssd1306.h           CD12E38522EB3688CC6745750B7F03B52CCC7F8B710AD3CDAD44300C9787695A
src/drivers/ssd1306.c               40489063AAE11D207293B0EFAB8B84E805D34B8856050C36147C4CC41C23C31F
include/kernel/oled_status_bar.h    203289EF2D1FB06885F0EE3AD039EF26D1A5D17A287D9AA99E462855D95BAE6D
src/kernel/oled_status_bar.c        33ABB8A3EE595E59283D77A6E989FE3A48D54730FA023DED0B0A232736D2D23D
src/kernel.c                        0552234204F7A160C336E4FE7606B6E4A363C07D118C4C7010F44A5E0A5C017C
```

No expansion is allowed without an explicit Gate 1 review before mutation.

## 10. Compatibility invariants

The boundary must not change:

- 128x32 physical geometry or frozen status/console coordinates;
- splash/home text or lifecycle timing;
- SYSTEM/USB/NETWORK semantics;
- 32-command registry or public binary RPC IDs `0x0001..0x0020`;
- USB descriptors/PMA/endpoints;
- scheduler task count, priorities, SVC ABI or stack capacities;
- IWDG ownership;
- UART/CDC/binary transport behavior;
- linker/startup/vector table;
- filesystem/heap/application-runtime/network/update state.

Existing `oleddirty`, `oleduiupdate`, `oledstatus`, `uiruntime` methods may emit additional diagnostic proof tokens but must keep their command IDs and existing success/error tokens compatible.

## 11. Acceptance record and gate order

Accepted candidate/evidence:

```text
candidate tree         75f05f689970b760604112b30346b0c328bfaff2
BIN                    44560 bytes
BIN SHA-256             93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677
ELF                    71308 bytes
ELF SHA-256             E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C
MAP SHA-256             00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09
Flash                  44560 / 65536
SRAM                   9848 / 20480
Gate 2 evidence        FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC
Gate 3 log             887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6
Gate 3 evidence        76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214
Gate 4 evidence        1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6
Gate 4                 PHYSICAL_OLED=PASS
```

Measured hardware transfer results are: full semantic baseline `572` payload bytes / `36` writes, clean `0`, one-byte narrow `9`, minute `00:00 -> 00:01` `11` at `x=123..125`, and USB transition `11` at `x=9..11`. Task0/task1 margins remain `328 / 424` bytes after the accepted OLED diagnostics. GCC reports `oled_status_bar_self_test` stack usage `240` bytes against the accepted `256`-byte diagnostic ceiling.

Deferred optimization, robustness, structured-observability, external-storage/memory and diagnostic-build-profile policy is canonical in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`. Repository EOL policy is frozen through `.gitattributes` with LF for source/docs/linker/assembly text.

```text
Gate 0  architecture/source-boundary freeze — PASS
Gate 1  source implementation + deterministic self/static proof — PASS
Gate 2  fresh GNU build/link/resource + transfer-metric validation — PASS
Gate 3  retained hardware/runtime + optimized-transfer proof — PASS
Gate 4  mandatory physical OLED regression — PASS / PHYSICAL_OLED=PASS
Gate 5  docs/evidence finalization — PASS
Gate 6  local acceptance commit — PASS / `39690c9ef103cbcf93272df8bad0359a934b7dc1`
Gate 7  ordinary non-force publication — PASS / PUBLISHED
```

Publication record:

```text
commit                  39690c9ef103cbcf93272df8bad0359a934b7dc1
tree                    f195fac5ce733c36a1d955e0fbe687ee6c83b605
subject                 feat: optimize OLED dirty region updates
Gate 5 evidence         7DAB43E97E238DA35894B767AD1403064905E64D9326F8B4E002AE0816AB6436
Gate 6 evidence         026C6D20AFF0FF0B13ED984217AD14132A25144EA6177CD78D88E88AE506AABB
Gate 7 evidence         FE69CA002582934A19A7D920EE1E9ACB61739E71EDCFC0018C1D1573D4A1F718
final repository        clean / ahead-behind 0/0
```

After Gate 7, exact next boundary:

`APPLICATION_RUNTIME_FOUNDATION`

## 12. Non-goals

This boundary does not add a second framebuffer, DMA, double buffering, animation framework, new font/layout/theme, application runtime, new task/SVC/IPC primitive, USB redesign, host application, networking, persistence, firmware update or bootloader.
