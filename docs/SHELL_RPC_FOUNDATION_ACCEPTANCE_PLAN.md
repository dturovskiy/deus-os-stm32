# STM32 OS — Transport-Neutral Shell/RPC Foundation Acceptance Plan

Status: **COMPLETED / PUBLISHED — commit `0c33304d2db86e54d715905393f49147bb6dd2ea`**

Boundary ID:

`SHELL_RPC_FOUNDATION`

## Gate 0 — planning/docs synchronization

Required prestate:

- published HEAD/origin/remote `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`;
- published tree `bbc6b24e28279075c41410e8abdace89c4b805b7`;
- subject `feat: add USB CDC ACM console foundation`;
- clean worktree and real index;
- accepted firmware `58548` bytes / `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`.

Gate 0 may change planning/documentation only.

Required planning decisions:

- boundary ID `SHELL_RPC_FOUNDATION`;
- one transport-neutral command service above UART/CDC;
- static command registry with method name, class, argument bounds and handler binding;
- semantic status taxonomy `OK / NOT_FOUND / BAD_ARGS / BUSY / INTERNAL_ERROR`;
- generic response writer + opaque context;
- source RX-event mask in execution context for same-stream diagnostics;
- existing `32`-byte line capacity retained;
- bounded maximum `4` argument tokens;
- no heap;
- UART and CDC parser state remains independent;
- legacy command names/output tokens remain compatible;
- `help` and `rpcinfo` are the only new foundation commands;
- no binary framing, host app, firmware update, bootloader, scheduler change, IPC, or OLED change.

No source edit, build, flash, commit, or push in Gate 0.

## Gate 1 — exact source investigation + implementation

Before mutation prove from exact published source:

- current command buffer/parser state and line-edit semantics;
- complete existing command set and destructive/diagnostic classification;
- current output routing through `console_output_transport`;
- current `schedtimed` dependence on origin RX event;
- task0-only command execution and UART/CDC ring ownership;
- exact source guard hashes for USB, scheduler, startup, IWDG and OLED files;
- current stack/Flash/SRAM budgets.

Implementation must establish:

- static registry and deterministic lookup;
- generic command execution context with writer callback + opaque context + source event mask;
- bounded in-place tokenization with maximum `4` args;
- semantic service status return independent of physical transport; service `OK` means dispatch/execution completed and does not replace method-specific `*_OK/ERR` payload semantics;
- response-writer failure latched by the execution context and promoted to `INTERNAL_ERROR` on returning handlers;
- UART and CDC adapters invoking the same registry/service;
- independent partial-line parser state;
- exact legacy `ERR` mapping for unknown/bad-argument text commands;
- exact `SCHED_DIAG_BUSY` retained;
- exact `WDOG_TRIP_ARMED` destructive behavior retained;
- `help` zero/one-argument behavior;
- `rpcinfo` deterministic foundation metadata;
- no direct UART/USB selection inside the service API;
- no Handler-mode command dispatch or IWDG reload.

Expected source scope:

```text
include/kernel/command_service.h
src/kernel/command_service.c
src/kernel.c
```

Guard unless explicitly reclassified:

```text
src/drivers/usb_device.c
include/drivers/usb_device.h
src/kernel/scheduler.c
include/kernel/scheduler.h
src/startup.s
src/drivers/iwdg.c
OLED/gfx/status-bar sources
linker/stm32f103c8.ld
```

Failure before flash: restore exact pre-source bytes.

## Gate 2 — fresh GNU build/link/static validation

Required:

- fresh isolated build;
- `-Wall -Wextra -Werror`;
- new command-service translation unit compiled and linked;
- no undefined/duplicate symbols;
- exact candidate tree/path set recorded;
- command registry contains every accepted legacy method plus `help` and `rpcinfo` exactly once;
- duplicate method names rejected by static/harness validation;
- line capacity exactly `32`;
- max args exactly `4`;
- no heap symbols (`malloc/calloc/realloc/free`);
- no new task/SVC/IRQ/vector ownership;
- USB/scheduler/startup/IWDG/OLED guards exact;
- IWDG reload ownership unchanged;
- Flash <= 64 KiB, SRAM <= 20 KiB;
- task0/task1/MSP stack floors retained;
- fresh BIN/ELF/map fingerprints recorded.

No flash in Gate 2.

## Gate 3 — UART + CDC hardware acceptance

Power policy remains the accepted CDC policy: micro-USB VBUS/data is the normal target power source, ST-LINK 3.3 V disconnected, UART adapter VCC disconnected. UART/SWD signal wiring need not be removed.

Required proof:

1. flash/verify exact Gate 2 candidate and final readback exact;
2. UART and Windows `usbser` CDC both enumerate/open normally;
3. `ping` over UART -> exact `PONG` only on UART;
4. `ping` over CDC -> exact `PONG` only on CDC;
5. every previously accepted safe command retains expected output semantics on CDC;
6. retained invasive scheduler diagnostics remain `8/8 SCHED_DIAG_BUSY` in production runtime;
7. `wdogtrip` remains destructive and causes a real IWDG reset;
8. unknown command over UART and CDC -> exact legacy `ERR` on origin only;
9. invalid argument count on a zero-argument method -> exact legacy `ERR`;
10. `help` with no args returns deterministic registry listing on origin only;
11. `help ping` returns deterministic metadata for `ping` and proves one-argument tokenization;
12. `help no_such_method` returns deterministic not-found/legacy error behavior;
13. `rpcinfo` returns foundation version, registry count, line capacity `32`, max args `4`;
14. repeated spaces/tabs tokenize deterministically;
15. backspace/DEL behavior remains correct;
16. line overflow is rejected and parser recovers for the next valid command;
17. UART and CDC partial lines interleave without state corruption or cross-response;
18. `schedtimed` controlled-event proof still wakes from its originating transport;
19. CDC pressure remains at least `128/128 PONG` with zero CDC drops/errors;
20. UART pressure remains `128/128 PONG` with zero UART drops/errors;
21. heartbeat/scheduler/stack/MSP/IWDG regressions remain green;
22. physical micro-USB disconnect/reconnect recovers CDC without reflash;
23. post-IWDG automatic USB detach/attach + usable `usbser` session still passes;
24. final Flash readback exactly matches Gate 2 candidate.

A COM-number change is not a failure; discover CDC dynamically.

## Gate 4 — OLED conditional review

If OLED/gfx/status-bar hashes remain exact and `uiruntime` passes before and after destructive IWDG proof:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise physical OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record:

- accepted source tree/path set;
- command-service API/registry/status contract;
- candidate size/SHA and memory budgets;
- UART/CDC legacy compatibility;
- `help`/`rpcinfo`/argument parser evidence;
- parser/origin isolation;
- pressure and scheduler regressions;
- physical reconnect and post-IWDG USB recovery;
- final exact Flash proof;
- Gate 4 disposition;
- exact next boundary: binary framed transport.

No build/flash/commit/push.

### Accepted Gates 2–5 record — 2026-09-16

- candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`;
- BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- ELF `64816` bytes / SHA-256 `D95F9798612E3F7A03EF27F138DA8DBCA7B956F2B4AAF5D0CEE8179308EEBCBA`;
- Flash `37196` bytes, SRAM `9216` bytes;
- Gate 2 evidence SHA-256 `34FB1B6D368A29AF5460174BBDA6AE81E479E5D11FBC0E25FDAC01617105BEA3`;
- Gate 3 log SHA-256 `0DCECB16F505C60A07AC73790DFAE4D7BFDFAEE56B6154DC503A5AF17B989E70`;
- Gate 3 evidence SHA-256 `0EE7342129866C86A1AAFBA42E942E2097C78BFDF3CF50D0C93F3A6B71E9A4E9`;
- deterministic registry `32/32`; `help`, `help ping`, `rpcinfo` PASS on UART/CDC;
- unknown and invalid-argument requests retain exact legacy `ERR` on the originating transport only;
- repeated separators, backspace, DEL, line-overflow rejection/recovery PASS;
- controlled `schedtimed` origin-event proof PASS on UART and CDC;
- CDC safe surface `20/20` pre-IWDG and `20/20` post-IWDG;
- scheduler diagnostics `8/8 SCHED_DIAG_BUSY`;
- packet-boundary proof `38/38`; CDC pressure `128/128` with zero drops; UART pressure `128/128`;
- physical micro-USB reconnect without reflash PASS;
- IWDG reboot `9042 ms` and automatic post-IWDG `usbser` reopen PASS;
- final Flash readback exact;
- Gate 4 `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- exact next architecture boundary after publication: **binary framed transport**.

## Gate 6 — local acceptance commit

Stage only accepted boundary paths. Require exact published parent/tree, `git diff --cached --check`, exact staged path set, one local commit, clean worktree/index, remote still at parent, behind `0` / ahead `1`. No push.

## Gate 7 — ordinary non-force publication

Pre-push require exact local acceptance commit, clean repo, remote at parent, fast-forward ancestry, behind `0` / ahead `1`.

Execute one ordinary non-force push. Post-push require local HEAD/origin/live remote exact, clean repo, ahead/behind `0/0`.

## Permanent retained constraints

- bare metal; no HAL/Arduino/FreeRTOS;
- UART remains emergency diagnostics;
- ST-LINK remains recovery/debug;
- no dual ST-LINK-3.3V + micro-USB-VBUS target powering;
- no USB IRQ/Handler IWDG reload;
- existing two-task production topology retained;
- no new generic timers or IPC/synchronization;
- no heap;
- OLED/gfx/status-bar frozen;
- binary framing, host application and firmware update remain later boundaries.
