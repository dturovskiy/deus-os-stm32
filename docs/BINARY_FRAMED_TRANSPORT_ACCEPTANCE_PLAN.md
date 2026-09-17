# STM32 OS — Binary Framed Transport Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED — `2fde9025a51021511e73a76b561f7983ca655e2f`**

Boundary ID:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Normative wire contract:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

## Gate 0 — planning / protocol synchronization

Required prestate:

- `main`, `origin/main`, remote `main` all at `0c33304d2db86e54d715905393f49147bb6dd2ea`;
- published tree `19ac9b95caca09e991842f6f9963864934b0a334`;
- subject `feat: add transport-neutral shell RPC foundation`;
- clean worktree/index;
- accepted shell/RPC BIN `37196` bytes / `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`.

Gate 0 may modify planning/documentation only.

Gate 0 must freeze:

- boundary ID and gate order;
- CDC-only binary transport for v1; UART stays text-only;
- magic `A5 5A` coexistence rule;
- exact version-1 frame envelope;
- little-endian fields;
- CRC-16/CCITT-FALSE parameters;
- request-ID semantics with zero reserved;
- HELLO negotiation fields/capabilities;
- stable public RPC method IDs `0x0001..0x0020` separate from internal dispatch enum;
- four-argument / 31-byte-per-argument / 132-byte request payload limits;
- 48-byte response data chunk limit and 64-byte maximum `RPC_DATA` wire frame;
- service vs protocol status domains;
- explicit destructive authorization flag;
- need for nonblocking all-or-none CDC frame enqueue;
- no heap/new task/SVC/IPC/USB descriptor change/OLED change.

No source edit, build, flash, commit or push in Gate 0.

## Gate 1 — exact source investigation + implementation

Before mutation, prove from exact published source:

- current command descriptor/lookup/execute API;
- current 32-method registry order and internal dispatch IDs;
- current CDC RX/TX ring sizes and single-byte TX behavior;
- current task0 CDC drain/text parser path;
- current USB descriptors/PMA/endpoint lifecycle and reset behavior;
- current scheduler/IWDG/OLED/startup/linker guards.

Implementation must establish:

- allocation-free binary frame parser/encoder;
- exact v1 envelope/CRC implementation;
- stable public wire IDs with duplicate rejection;
- HELLO request/response;
- bounded RPC request argument decoding into the existing command-service request shape;
- shared command execution through existing `command_service_execute()`;
- binary writer that emits ordered `RPC_DATA` chunks plus `RPC_END`;
- maximum data frame exactly 64 wire bytes;
- newline/full-chunk/final-residual flush semantics;
- destructive confirmation policy;
- bad CRC => no command execution;
- binary/text demultiplexer state on CDC only;
- existing partial text parser state preserved across binary frames;
- one narrow nonblocking all-or-none CDC TX-span enqueue;
- no Handler-mode protocol parsing/execution and no IWDG reload ownership change.

Expected source scope:

```text
include/kernel/binary_frame.h        new
src/kernel/binary_frame.c            new
include/kernel/binary_rpc.h          new
src/kernel/binary_rpc.c              new
include/kernel/command_service.h     adapted
src/kernel/command_service.c         adapted
include/drivers/usb_device.h         adapted
src/drivers/usb_device.c             adapted narrowly for atomic TX span
src/kernel.c                         CDC demux/binding
```

Guard unless explicitly reclassified before mutation:

```text
src/kernel/scheduler.c
include/kernel/scheduler.h
src/startup.s
src/drivers/iwdg.c
include/drivers/iwdg.h
linker/stm32f103c8.ld
OLED/gfx/status-bar files
USB descriptors/PMA constants/endpoint lifecycle
```

## Gate 2 — fresh GNU build/link/static validation

Required:

- fresh isolated Arm GNU build;
- `-Wall -Wextra -Werror`;
- binary frame/RPC TUs compiled and linked;
- no undefined/duplicate symbols;
- `git diff --check` clean;
- exact candidate tree/path set recorded;
- exact 32 stable RPC IDs each present once;
- internal dispatch IDs not used as implicit wire numbering;
- protocol constants exactly match normative doc;
- CRC known-answer tests pass in host/static harness;
- malformed/oversized frame parser tests pass in host/static harness;
- 48-byte RPC data chunk => exactly 64 wire bytes;
- max request payload `132` and max args `4` statically enforced;
- no heap symbols;
- no new task/SVC/vector/IRQ ownership;
- USB descriptor bytes and PMA map exact;
- CDC atomic-span write all-or-none invariant validated from source/static harness;
- scheduler/startup/IWDG/OLED guards exact;
- IWDG reload ownership unchanged;
- Flash <=64 KiB, SRAM <=20 KiB;
- production console/heartbeat/MSP stack floors retained;
- fresh BIN/ELF/map hashes recorded.

No flash in Gate 2.

## Gate 3 — USB CDC binary hardware acceptance

Power policy remains the accepted USB runtime policy: micro-USB VBUS/data powers the target, ST-LINK 3.3 V disconnected, UART adapter VCC disconnected. UART TX/RX, SWDIO/SWCLK and grounds remain connected.

Required proof:

1. flash/verify exact Gate 2 candidate; final readback exact;
2. Windows `usbser` enumerate/open normally after one software reset;
3. existing text `ping` over CDC still returns `PONG`;
4. exact binary HELLO response fields/capabilities;
5. binary ping using wire ID `0x0001`, nonzero request ID, exact request-ID echo and `RPC_END OK`;
6. binary `help ping` and `rpcinfo` through existing command service;
7. every accepted safe command executes over binary RPC with ordered data and final service status;
8. live invasive scheduler diagnostics remain service `BUSY` / payload-compatible;
9. unknown wire ID returns `NOT_FOUND` without invoking another method;
10. bad argc/argument shape returns `BAD_ARGS`;
11. destructive `wdogtrip` without flag does not execute and returns `DESTRUCTIVE_CONFIRM_REQUIRED`;
12. authorized `wdogtrip` emits `WDOG_TRIP_ARMED` data before real IWDG reset; normal `RPC_END` is not required before reset;
13. one bad-CRC frame executes no command; next valid binary request succeeds;
14. malformed/oversized length executes no command and parser resynchronizes;
15. magic/header/payload/CRC split across arbitrary host writes still decodes exactly;
16. `help` output spans multiple `RPC_DATA` frames with sequence numbers `0..N-1`, exact concatenated payload and matching `RPC_END` total byte/chunk counts;
17. request IDs `1`, a midrange value, and `0xFFFF` echo exactly; request ID `0` is rejected;
18. partial CDC text command state survives an intervening complete binary request and later completes correctly;
19. binary request does not inject CR/LF or frame bytes into the text parser;
20. text commands before/after binary traffic remain origin-correct and byte-compatible;
21. binary pressure >=128 ping requests complete with unique IDs and no malformed/CRC response frames;
22. CDC transport reports zero RX/TX drops/errors after binary pressure;
23. retained text CDC pressure `128/128 PONG` passes;
24. retained UART pressure `128/128 PONG` passes;
25. retained CDC class-control, packet-boundary, heartbeat, scheduler, stack/MSP and IWDG regressions pass;
26. physical micro-USB disconnect/reconnect restores text and binary operation without reflash;
27. post-IWDG automatic USB detach/attach and `usbser` reopen restore text and binary operation;
28. final Flash readback exactly matches Gate 2 candidate.

COM number is discovered dynamically.

## Gate 4 — OLED conditional review

If OLED/gfx/status-bar hashes are exact and retained `uiruntime` passes before/after the destructive watchdog proof:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise physical OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record:

- accepted source tree/path set;
- normative wire version and CRC parameters;
- stable method-ID map;
- request/response limits;
- HELLO capabilities;
- parser recovery evidence;
- command-service reuse evidence;
- binary/text coexistence evidence;
- destructive-confirmation proof;
- candidate size/hashes and memory budgets;
- pressure/reconnect/IWDG recovery;
- final exact Flash proof;
- Gate 4 disposition;
- exact next boundary: host control application foundation.

No build/flash/commit/push.

### Gate 2–5 accepted record — 2026-09-16

Tested Gate 2 candidate:

```text
published parent      0c33304d2db86e54d715905393f49147bb6dd2ea
published tree        19ac9b95caca09e991842f6f9963864934b0a334
tested candidate tree c2c3d9743c23ab02329a9652714862fafb5bb17c
BIN bytes             40720
BIN SHA-256           AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022
ELF bytes             65876
ELF SHA-256           4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B
MAP SHA-256           6A71B9E6F8FAB93870C95B967448FEEE6D1656DF1098A63DB90D122125609FD6
Flash used            40720 / 65536
SRAM used             9752 / 20480
```

Exact accepted boundary path set:

```text
CHANGELOG.md
README.md
docs/ARCHITECTURE.md
docs/IMPLEMENTATION_PLAN.md
docs/MASTER_EXECUTION_CHECKLIST.md
docs/NATIVE_USB_DEVICE_CORE_ACCEPTANCE_PLAN.md
docs/PROJECT_HANDOFF.md
docs/ROADMAP.md
docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md
docs/SHELL_RPC_FOUNDATION_PLAN.md
docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md
docs/BINARY_FRAMED_TRANSPORT_PLAN.md
docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md
include/drivers/usb_device.h
include/kernel/command_service.h
include/kernel/binary_frame.h
include/kernel/binary_rpc.h
src/drivers/usb_device.c
src/kernel.c
src/kernel/command_service.c
src/kernel/binary_frame.c
src/kernel/binary_rpc.c
```

Gate 2 static/build acceptance:

- fresh `16 C + 1 ASM` build with `-Wall -Wextra -Werror`: PASS;
- link/objcopy: PASS; undefined symbols: `0`;
- CRC-16/CCITT-FALSE KAT `123456789 -> 0x29B1`: PASS;
- exact stable RPC ID count/range `32`, `0x0001..0x0020`: PASS;
- request payload `132`, max args `4`, max arg bytes `31`, data chunk `48`, max `RPC_DATA` wire frame `64`: PASS;
- vectors, USB descriptors/PMA, scheduler/startup/IWDG/OLED guards and stack floors: PASS.

Gate 3 hardware acceptance:

- exact Gate 2 candidate flash/verify and final exact Flash readback: PASS;
- Windows `usbser`, CDC class controls and dynamic COM reopen: PASS;
- binary HELLO/request-ID echo, `help` chunking/accounting, `help ping`, `rpcinfo`: PASS;
- binary safe core surface `21/21`; invasive scheduler diagnostics `8/8 BUSY`;
- unknown ID, BAD_ARGS, destructive deny/allow, bad CRC, oversize resync and arbitrary split writes: PASS;
- partial CDC text state preserved across binary traffic; no frame-byte injection into text parser: PASS;
- binary pressure `128/128` unique request IDs with zero CDC drops;
- retained text CDC pressure `128/128`; retained UART race `128/128`;
- physical micro-USB disconnect/reconnect restores text + binary without reflash: PASS;
- authorized binary `wdogtrip` emitted `WDOG_TRIP_ARMED`, produced a real IWDG reboot in `7433 ms`, and post-reset UART + CDC text + binary recovered automatically: PASS.

Gate 4 accepted disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Basis: frozen OLED/gfx/status-bar hashes remained exact and `uiruntime` passed both before and after the destructive IWDG proof.

Evidence SHA-256:

- embedded Gate 2 evidence ZIP: `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- Gate 3 hardware log: `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- Gate 3 hardware evidence ZIP: `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`.

Gate 5 documentation/evidence finalization: **PASS**. Gate 5 performs no build, flash, commit, or push.

Gate 6 local acceptance commit: **PASS**. Gate 7 ordinary non-force publication: **PASS** at `2fde9025a51021511e73a76b561f7983ca655e2f`, tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`. The next architecture boundary is `OS_APPLICATION_AND_UI_MODEL_FOUNDATION`.

## Gate 6 — local acceptance commit

Stage only accepted boundary paths. Require exact published parent/tree, exact staged path set, `git diff --cached --check`, one local commit, clean worktree/index, remote still at parent, behind `0` / ahead `1`. No push.

## Gate 7 — ordinary non-force publication

Require exact local acceptance commit, clean repo, remote at published parent, fast-forward ancestry and ahead/behind `1/0` before push.

Executed one ordinary non-force push `origin main:main`. Post-push local `HEAD`, `origin/main` and fresh `FETCH_HEAD` were exact at `2fde9025a51021511e73a76b561f7983ca655e2f`; repository clean, ahead/behind `0/0`. Gate 7 PASS.

## Permanent retained constraints

- bare metal; no HAL/Arduino/FreeRTOS;
- UART remains text emergency diagnostics;
- ST-LINK remains recovery/debug;
- no dual ST-LINK 3.3 V + micro-USB VBUS target powering;
- no UART adapter VCC;
- no USB/UART IRQ command execution or IWDG reload;
- existing two-task production topology retained;
- no heap;
- no generic IPC/timer/runtime-statistics expansion in this boundary;
- OLED/gfx/status-bar frozen;
- encryption/authentication, file transfer, host GUI and firmware update remain later boundaries.
