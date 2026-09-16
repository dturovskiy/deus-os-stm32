# STM32 OS — USB CDC ACM Diagnostic/Command Console Acceptance Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT NEXT**

Boundary ID:

`USB_CDC_ACM_CONSOLE_FOUNDATION`

## Gate 0 — planning/docs synchronization

Required prestate:

- published HEAD `3f55f624b72b4c5266ec0e4b0006839c4478bec8`;
- published tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`;
- subject `feat: add native USB device core foundation`;
- `main`, `origin/main`, live remote `main` exact;
- worktree and real index clean before planning mutation.

Gate 0 may change planning/documentation only.

Required planning decisions:

- boundary ID `USB_CDC_ACM_CONSOLE_FOUNDATION`;
- CDC test identity `1209:000B`, private testing only;
- Windows inbox `usbser.sys`, no custom INF;
- device class/subclass `02/02` for automatic Windows binding;
- two-interface CDC ACM topology;
- EP1 notification IN, EP2 bulk OUT, EP3 bulk IN;
- explicit PMA map;
- EP0 bounded OUT-data-stage requirement for `SET_LINE_CODING`;
- no new production task/SVC/IPC;
- shared command semantics with per-transport parser/output ownership;
- UART/ST-LINK recovery retained;
- OLED frozen.

No source edit, build, flash, commit, or push.

Next: `USB_CDC_ACM_CONSOLE_SOURCE_IMPLEMENTATION`.

## Gate 1 — exact source investigation + implementation

Before mutation prove from exact published source:

- accepted USB endpoint/PMA/toggle semantics;
- current EP0 state machine and missing OUT data stage;
- current configuration/reset lifecycle;
- exact task0 wait/wake and UART ring ownership;
- current command parser/output coupling to UART;
- current Flash/SRAM/stack budgets;
- exact source guard hashes;
- exact Windows/CDC descriptor/request requirements used by implementation.

Implementation must establish:

- identity `1209:000B`, product `Deus OS CDC Console`;
- device class/subclass `02/02` for inbox `usbser.sys` binding;
- CDC control interface 0 + CDC data interface 1;
- correct CDC functional descriptors;
- EP1 `0x81` interrupt IN, EP2 `0x02` bulk OUT, EP3 `0x83` bulk IN;
- bounded PMA ownership within local 512 bytes;
- real `SET_CONFIGURATION(1/0)` endpoint lifecycle;
- bounded EP0 OUT data stage;
- `SET_LINE_CODING`, `GET_LINE_CODING`, `SET_CONTROL_LINE_STATE`;
- deterministic stall for unsupported requests;
- bounded CDC RX/TX queues and telemetry;
- no indefinite task spin under USB TX backpressure/disconnect;
- task0 wait/wake integration without a new task;
- per-transport parser state;
- shared command execution with response to originating transport;
- UART independence;
- no USB IRQ IWDG reload.

Expected source scope is limited to the accepted USB core, a minimal CDC class module if layering review justifies it, and task0 console adaptation. Scheduler core/header, startup/vector, linker, and frozen OLED files remain guards unless exact evidence reclassifies them.

Failure before flash: restore exact pre-source bytes.

## Gate 2 — fresh GNU build/link/static validation

Use fresh isolated build output.

Required:

- `-Wall -Wextra -Werror`;
- exact candidate tree/path set;
- CDC translation unit(s) compiled and linked;
- IRQ20 vector still resolves to the USB handler;
- no duplicate endpoint/vector ownership;
- exact device class/subclass/VID/PID;
- exact configuration total length/interface/endpoint counts;
- functional descriptors reference master interface 0 / data interface 1;
- PMA map non-overlapping and below local `0x200`;
- bounded EP0 OUT-data buffer;
- explicit CDC RX/TX ring capacities in SRAM budget;
- Flash <= 64 KiB and SRAM <= 20 KiB;
- accepted MSP/task reservations and stack floors retained;
- scheduler core hashes exact unless scope was formally reclassified;
- OLED/gfx/status-bar hashes exact;
- no Handler-mode IWDG reload;
- no new SVC/IPC/task unless formally authorized;
- fresh BIN/ELF/map fingerprints recorded.

No flash in Gate 2.

## Gate 3 — real CDC ACM hardware acceptance

Power precondition:

**micro-USB VBUS/data is the sole target power during CDC runtime testing; ST-LINK 3.3 V is disconnected. UART adapter VCC remains disconnected.**

ST-LINK GND/SWDIO/SWCLK may remain connected for recovery where electrically safe. Do not require repeated source-power switching merely to run the USB test.

Required hardware proof:

1. flash/verify exact Gate 2 candidate only and final readback exact;
2. UART COM3 remains usable as independent emergency/control channel;
3. Windows enumerates exact `VID_1209&PID_000B` / product `Deus OS CDC Console`;
4. Windows binds Microsoft `usbser.sys` automatically, with no custom INF;
5. device reaches configuration `1` and exposes a dynamic Windows COM port;
6. host opens/closes the CDC COM port repeatedly without firmware reset/fault;
7. class-request proof covers `SET_LINE_CODING`, `GET_LINE_CODING`, and `SET_CONTROL_LINE_STATE`;
8. changing CDC line coding does not alter USART1 115200 emergency-console behavior;
9. CDC `ping` returns exact `PONG` over CDC, proving bulk OUT -> parser -> bulk IN;
10. retained safe command surface passes through CDC with responses on CDC;
11. all eight invasive scheduler commands remain exact `SCHED_DIAG_BUSY` through CDC;
12. parser/response isolation is proven by alternating/interleaving UART and CDC commands without line corruption or cross-transport responses;
13. CDC OUT packet-boundary cases exercise valid command streams crossing exact host writes of 63, 64, and 65 bytes with complete expected responses;
14. CDC pressure proof performs at least `128` `ping` commands with exact response count and zero CDC RX/TX drops/errors;
15. retained UART pressure proof remains `128/128 PONG`, zero UART drops/errors;
16. scheduler/heartbeat/IWDG/stack/MSP regressions remain green;
17. physical micro-USB disconnect/reconnect recovers `usbser` COM transport without reflashing;
18. deliberate `wdogtrip` produces a real IWDG reset, then both UART and CDC recover; the reboot must produce a host-visible USB disconnect/re-attach so Windows does not retain a stale `usbser` session;
19. post-reset `health` reports `IWDG_RESET=1`, CDC re-enumerates into a usable COM session, and CDC command transport works again;
20. final Flash readback exactly matches Gate 2 candidate.

Host evidence must record:

- VID/PID/product;
- device/configuration/interface/endpoint descriptors;
- bound service/driver (`usbser`);
- dynamically assigned COM port;
- configured state;
- line coding/control-line requests;
- CDC RX/TX counters;
- disconnect/reconnect and post-IWDG re-enumeration.

A COM-number change is not a failure. Harnesses must discover the CDC port dynamically from target identity rather than assume a fixed COM number.

## Gate 4 — OLED conditional review

If OLED/gfx/status-bar hashes remain exact and `uiruntime` passes before and after destructive IWDG proof:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise physical OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record exact accepted source tree/path set, candidate size/SHA and memory budgets, CDC identity, descriptor/PMA topology, Windows `usbser`/COM evidence, class-request evidence, command/pressure/transport-isolation evidence, physical reconnect recovery, IWDG reset/recovery, retained UART/scheduler/stack/OLED results, Gate 4 disposition, and exact next boundary.

No build/flash/commit/push.

### Accepted Gates 2–5 record — 2026-09-16

Corrected Gate 2 candidate:

```text
tested source tree  d815a8357b9f77850c08ff071f47be8d2b5d4b53
binary              58548 bytes
SHA-256             D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE
text/data/bss       58500 / 48 / 9152
Flash used           58548 bytes
SRAM used            9200 bytes
```

Gate 3 corrected hardware acceptance:

- exact `1209:000B` device and 67-byte configuration descriptors: PASS;
- Windows `usbser` binding and dynamic COM open: PASS;
- automatic usable CDC attach after MCU software reset: PASS;
- `SET_LINE_CODING 9600 7E2`, `GET_LINE_CODING`, `SET_CONTROL_LINE_STATE`: PASS;
- USART1 remained independently fixed at 115200 8N1: PASS;
- safe command surface pre/post IWDG: `20/20` + `20/20`;
- invasive scheduler diagnostics: `8/8 BUSY`;
- packet-boundary proof: `38/38` expected PONG;
- CDC pressure: `128/128`, zero drops;
- UART pressure: `128/128`;
- dual-transport parser/response isolation: PASS;
- physical micro-USB disconnect/reconnect without reflash: PASS;
- deliberate IWDG reboot: `9028 ms`;
- post-IWDG automatic USB detach/re-attach, `usbser` reopen and CDC `ping`: PASS;
- final Flash readback: exact corrected Gate 2 candidate.

Gate 4 accepted disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Basis: OLED/gfx/status-bar guard hashes remained exact, and `uiruntime` passed both before and after destructive IWDG recovery.

Evidence SHA-256:

- corrected Gate 2 evidence: `49AAFBAFEA0B895DFBEBA7311335069EFD512640A13F200D1F3B9D65D6774869`;
- Gate 3 hardware log: `D009F0D7E12C10C3FBA40037C6EC485E9FA27674901C5C3D6C4AAE72C0D42F35`;
- Gate 3 hardware evidence: `6035C6FD5C0718252B28AAC07CD67C879AA1F14A639DD9A7854D3B74F54D49CB`.

Gate 5 documentation/evidence finalization is accepted. No build, flash, commit, or push is part of Gate 5.

## Gate 6 — local acceptance commit

Stage only accepted boundary paths. Require exact published parent/tree, `git diff --cached --check`, one local commit, exact subject/path set, clean worktree/index, remote still at parent, behind `0` / ahead `1`. No push.

## Gate 7 — ordinary non-force publication

Pre-push require exact local acceptance commit, clean repo, remote at parent, fast-forward ancestry, behind `0` / ahead `1`.

Execute exactly one ordinary:

`git push origin main:main`

No force and no force-with-lease. Post-push require local HEAD/origin/live remote exact, clean repo, ahead/behind `0/0`.

## Permanent retained constraints

- bare metal; no HAL/Arduino/FreeRTOS;
- no custom Windows kernel driver/INF for this boundary;
- UART remains independent emergency console;
- ST-LINK remains recovery/debug;
- no dual ST-LINK-3.3V + micro-USB-VBUS target powering;
- no USB IRQ IWDG reload;
- existing two-task production topology retained;
- scheduler core semantics unchanged unless independently justified;
- generic timers, IPC/queues/synchronization, runtime statistics deferred;
- OLED/gfx/status-bar frozen;
- shell/RPC and binary protocol remain later boundaries.
