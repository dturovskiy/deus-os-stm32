# STM32 OS — Binary Framed Transport Foundation Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED — `2fde9025a51021511e73a76b561f7983ca655e2f`**

Boundary ID:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

Normative wire contract:
`docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`

## 1. Published baseline

This boundary starts from the published transport-neutral shell/RPC foundation:

- commit `0c33304d2db86e54d715905393f49147bb6dd2ea`;
- tree `19ac9b95caca09e991842f6f9963864934b0a334`;
- subject `feat: add transport-neutral shell RPC foundation`;
- accepted firmware candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`;
- accepted BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`;
- Flash `37196` bytes, SRAM `9216` bytes;
- UART and USB CDC text shell, command service, IWDG recovery, physical reconnect and retained regressions accepted.

Roadmap order:

```text
native USB Device core
 -> CDC ACM console
 -> transport-neutral shell/RPC
 -> binary framed transport             <- this boundary
 -> host control application
 -> structured telemetry / transfer features
 -> USB firmware update / recoverable bootloader
```

## 2. Objective

Add one bounded versioned binary request/response transport over the already accepted USB CDC byte stream without duplicating command semantics or changing scheduler ownership.

The binary adapter must invoke the same command-service registry and handlers that the text shell uses.

Target layering:

```text
Windows/Linux host
      |
USB CDC byte stream
      |
text/binary demultiplexer
   |             |
text shell    binary frame parser
   |             |
   +-------> command service <-------+
                 |
              handlers
                 |
       originating response adapter
```

UART remains the human/emergency text console.

## 3. Architectural rules

- no new production task;
- no new SVC;
- no queue/mutex/semaphore/timer subsystem;
- no heap;
- no USB descriptor/PMA/endpoint redesign;
- no binary command semantics separate from `command_service`;
- no Handler-mode parser or command execution;
- no IWDG reload ownership change;
- frozen OLED/gfx/status-bar sources remain unchanged.

## 4. CDC text/binary coexistence

Binary v1 uses reserved magic `A5 5A` on the CDC byte stream.

Outside a binary frame, existing text-shell bytes keep their accepted semantics. Once the magic is accepted, all frame bytes are owned by the binary parser until completion/rejection and are never fed to the text parser.

A complete binary frame must not clear or corrupt an existing partial text line. This permits deterministic sequential coexistence without adding a second USB interface.

UART does not receive the binary demultiplexer in this boundary.

## 5. Wire framing

Protocol version `1` uses:

- fixed magic `A5 5A`;
- explicit version;
- frame type;
- flags;
- 16-bit request ID;
- 16-bit payload length;
- CRC-16/CCITT-FALSE;
- little-endian integer fields.

Exact bytes and error semantics are normative in `BINARY_FRAMED_TRANSPORT_PROTOCOL.md`.

Request ID `0` is reserved for later unsolicited events. Host RPC uses `1..65535`.

## 6. Stable public RPC IDs

This boundary introduces explicit stable 16-bit wire IDs for the 32 already accepted command-service methods.

The public IDs are separate from the internal C dispatch enum. Gate 1 may extend the command descriptor/API with an explicit `rpc_id`/wire-ID field and lookup helper, but it must not expose enum ordinal position as protocol ABI.

Accepted v1 mapping is fixed by the protocol document (`0x0001` through `0x0020`).

## 7. RPC argument adaptation

Binary RPC does not reparse a text command line.

A request carries:

- stable method ID;
- argc;
- up to four bounded arguments;
- per-argument byte length.

The adapter copies accepted argument bytes into bounded NUL-terminated scratch storage and constructs the same `command_service_request_t` consumed by the existing execution path.

Initial limits:

```text
max argc            4
max arg bytes       31 each
max request payload 132 bytes
heap                 none
```

These limits are sufficient for the current service contract and deliberately do not pretend to be the later file/update transport.

## 8. Response strategy

Do not buffer an entire command response.

The command-service writer is adapted to emit bounded binary `RPC_DATA` chunks and a final `RPC_END` frame. Data chunks carry at most `48` command-output bytes so the largest v1 data frame is exactly `64` wire bytes, one USB FS bulk max packet.

Flush a chunk on:

- 48 bytes;
- newline;
- returning-handler completion with residual bytes.

This design preserves bounded RAM usage and allows the existing non-returning `wdogtrip` method to transmit `WDOG_TRIP_ARMED` before reset.

`RPC_END` contains method ID, chunk count, total output bytes, status domain and status code.

## 9. Status ownership

For a structurally valid command request, the binary envelope preserves the existing command-service statuses:

`OK / NOT_FOUND / BAD_ARGS / BUSY / INTERNAL_ERROR`.

Method-specific diagnostic payloads such as `OLED_*_ERR` remain output bytes. This boundary does not invent typed schemas for every old diagnostic.

Binary protocol/parser policy has its own status domain for malformed request shape, bad flags, reserved request ID, destructive authorization and payload limits.

Bad CRC is dropped without command execution and without trusting request metadata.

## 10. Destructive command safety

A binary request for a descriptor classified `DESTRUCTIVE` must set `ALLOW_DESTRUCTIVE`.

Without that bit, the request does not execute and returns a protocol-policy rejection.

This is deliberate transport safety, not a replacement for future authentication/update security.

## 11. TX frame atomicity

Current `usb_cdc_write_byte()` can fail after an arbitrary number of preceding bytes have already entered the TX ring. That behavior is acceptable for human text but not for framed binary data.

Gate 1 is therefore authorized to make one narrow USB CDC API extension: a nonblocking all-or-none span enqueue for a prebuilt frame. The binary layer will only submit frames `<=64` bytes.

The extension must:

- publish either the complete frame or no bytes;
- never busy-wait for space;
- preserve existing SPSC TX ring/IRQ ownership;
- preserve descriptors, PMA, endpoint lifecycle and reset behavior;
- leave `usb_cdc_write_byte()` compatibility intact.

## 12. Parser state and diagnostics

The binary parser uses fixed static state only.

Minimum parser telemetry tracked in RAM:

- frames received;
- hello requests;
- RPC requests;
- sync errors;
- CRC errors;
- length errors;
- protocol errors;
- response-data frames;
- response-end frames;
- TX failures.

No new text command is required solely to expose this telemetry in v1. Hardware acceptance may inspect protocol behavior directly.

## 13. HELLO / capability discovery

A zero-payload `HELLO_REQUEST` returns protocol/service versions, text line capacity, binary request limit, chunk size, registry count and capability flags.

This is the negotiation point future host tooling uses before issuing RPC.

## 14. Expected Gate 1 source scope

Expected new modules:

```text
include/kernel/binary_frame.h
src/kernel/binary_frame.c
include/kernel/binary_rpc.h
src/kernel/binary_rpc.c
```

Expected adapted files:

```text
include/kernel/command_service.h
src/kernel/command_service.c
include/drivers/usb_device.h
src/drivers/usb_device.c
src/kernel.c
```

Reason for each adapted area:

- command service: stable explicit wire-ID metadata/lookup only;
- USB driver: atomic nonblocking frame-span enqueue only;
- kernel: CDC text/binary demultiplexing and binding to the existing command handler.

Any scheduler/startup/linker/IWDG/OLED scope expansion is forbidden unless a concrete implementation blocker is proven before mutation.

## 15. Build/static acceptance — ACCEPTED 2026-09-16

Gate 2 proved:

- fresh GNU build with `-Wall -Wextra -Werror`;
- all new translation units compiled and linked;
- no heap symbols;
- no duplicate wire IDs;
- exact 32-ID mapping;
- binary data frame maximum exactly 64 wire bytes;
- inbound request/storage bounds compile-time checked;
- no new task/SVC/vector ownership;
- retained USB descriptor/PMA identity exact;
- retained scheduler/startup/IWDG/OLED guards exact except the explicitly authorized narrow USB TX API implementation change;
- Flash <=64 KiB, SRAM <=20 KiB;
- production stack floors retained.

No flash in Gate 2.

## 16. Hardware acceptance — ACCEPTED 2026-09-16

Gate 3 proved:

- exact Gate 2 image flash/readback;
- normal Windows `usbser` attach/open;
- exact HELLO response;
- binary `ping`, `help ping`, `rpcinfo` with request-ID echo;
- all accepted safe commands over binary RPC;
- diagnostic methods return service `BUSY` in live production runtime;
- unknown wire ID -> `NOT_FOUND` without text-shell contamination;
- bad argc -> `BAD_ARGS`;
- destructive request without flag rejected;
- authorized `wdogtrip` emits data marker then causes real IWDG reset;
- bad CRC executes nothing and parser recovers;
- bad/oversized length recovers;
- magic/header/payload split at arbitrary CDC packet/write boundaries;
- long output (`help`) produces ordered chunk sequence + exact final byte count;
- partial text line survives an intervening binary RPC frame;
- text shell remains fully usable before/after binary traffic;
- >=128 binary ping requests complete with unique request IDs and no CDC drops/errors;
- retained text CDC/UART pressure, scheduler, stack/MSP, reconnect and post-IWDG recovery remain green;
- final Flash readback exact.

Accepted candidate/evidence:

- tested candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`;
- binary safe surface `21/21`, scheduler BUSY `8/8`, binary pressure `128/128` unique IDs;
- retained CDC/UART pressure, physical reconnect, destructive IWDG and automatic post-reset text+binary recovery: PASS;
- final Flash readback exact;
- Gate 4 `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- Gate 2 evidence `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- Gate 3 log `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- Gate 3 evidence `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`;
- Gate 5 documentation/evidence finalization: PASS;
- Gate 6 local acceptance commit: PASS;
- Gate 7 ordinary non-force publication: PASS at `2fde9025a51021511e73a76b561f7983ca655e2f`, tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`; next architecture boundary is `OS_APPLICATION_AND_UI_MODEL_FOUNDATION`.

## 17. Non-goals

Not in this boundary:

- encryption/authentication;
- file transfer;
- firmware update;
- bootloader;
- framebuffer/bitmap transfer;
- telemetry subscriptions;
- host GUI;
- USB vendor-specific bulk interface;
- binary protocol over UART;
- dynamic memory;
- generic IPC/timers.

## 18. Gate order

```text
Gate 0  planning + normative protocol/docs synchronization
Gate 1  exact source investigation + binary framing/RPC implementation
Gate 2  fresh GNU build/link/static validation
Gate 3  USB CDC binary + retained text/UART hardware acceptance
Gate 4  OLED conditional N/A review
Gate 5  docs/evidence finalization
Gate 6  local acceptance commit
Gate 7  ordinary non-force publication
```

Do not combine source mutation, build, hardware acceptance, commit and push into one gate.
