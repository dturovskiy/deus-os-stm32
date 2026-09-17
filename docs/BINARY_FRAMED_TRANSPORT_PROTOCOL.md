# STM32 OS — Binary Framed Transport Protocol v1

Status: **PROTOCOL V1 ACCEPTED / PUBLISHED — `2fde9025a51021511e73a76b561f7983ca655e2f`**

Boundary ID:

`BINARY_FRAMED_TRANSPORT_FOUNDATION`

This document is normative for protocol version `1`.

## 1. Transport and coexistence

Protocol v1 runs over the already published USB CDC ACM byte stream. It does not change USB descriptors, endpoint numbers, PMA ownership, Windows `usbser` binding, or the UART emergency console.

UART remains text-only in this boundary.

CDC carries both the existing human text shell and binary frames. Binary traffic is claimed only by the reserved two-byte magic prefix:

```text
A5 5A
```

The existing text shell remains byte-compatible for its accepted ASCII input surface. A lone `A5` that is not followed by `5A` is treated as a binary-sync error; the `A5` is discarded and the following byte is reprocessed from the idle demultiplexer state. Once `A5 5A` is accepted, all bytes through the declared frame body and CRC belong exclusively to the binary parser and must never enter the text parser.

Text parser partial-line state must survive a complete binary frame unchanged.

## 2. Wire byte order

All multi-byte integer fields are little-endian.

## 3. Frame envelope

Every frame is:

```text
offset  size  field
0       1     magic0 = A5
1       1     magic1 = 5A
2       1     protocol_version = 01
3       1     frame_type
4       1     flags
5       1     reserved = 00
6       2     request_id
8       2     payload_length
10      N     payload
10+N    2     CRC-16/CCITT-FALSE
```

CRC parameters:

```text
width       16
polynomial  0x1021
init        0xFFFF
refin       false
refout      false
xorout      0x0000
```

CRC covers bytes starting at `protocol_version` (offset 2) through the final payload byte. The magic bytes and the CRC field itself are not included.

`request_id == 0` is reserved for future unsolicited events. All v1 host requests use IDs `1..65535`; responses echo the request ID exactly.

Unknown nonzero flag bits, nonzero reserved bytes, oversized payloads, malformed payload encoding, and unsupported frame types are protocol errors. A CRC-failed frame is discarded without command execution and without trusting or responding to its request ID.

## 4. Frame types

```text
0x01  HELLO_REQUEST
0x02  RPC_REQUEST
0x81  HELLO_RESPONSE
0x82  RPC_DATA
0x83  RPC_END
0x84  PROTOCOL_ERROR
```

No telemetry/event/file/update frame type is implemented in v1. Their future addition must not reinterpret these frame types.

## 5. Flags

For `RPC_REQUEST`:

```text
bit 0  ALLOW_DESTRUCTIVE
bits 1..7 reserved, must be zero
```

`ALLOW_DESTRUCTIVE` is required when the selected registry descriptor has class `DESTRUCTIVE`. CRC integrity is not treated as destructive authorization by itself.

All flags are zero for v1 `HELLO_REQUEST` and response frames.

## 6. HELLO

`HELLO_REQUEST` has zero payload bytes.

`HELLO_RESPONSE` payload:

```text
offset  size  field
0       1     protocol_version = 1
1       1     command_service_version = 1
2       1     command_max_args = 4
3       1     reserved = 0
4       2     text_line_capacity = 32
6       2     rpc_request_payload_max = 132
8       2     rpc_data_chunk_max = 48
10      2     registry_count = 32
12      4     capability_flags
```

Capability flags:

```text
bit 0  CDC text/binary coexistence
bit 1  CRC-16/CCITT-FALSE integrity
bit 2  stable 16-bit RPC method IDs
bit 3  request-ID correlation
bit 4  explicit destructive-request flag
bit 5  chunked response data
bits 6..31 reserved
```

A host should issue HELLO before relying on protocol-specific capabilities.

## 7. RPC request payload

`RPC_REQUEST` payload:

```text
offset  size  field
0       2     stable RPC method ID
2       1     argc
3       1     reserved = 0
4       ...   argc encoded arguments
```

Each argument is encoded as:

```text
1 byte length
length bytes payload
```

Initial v1 limits:

```text
argc                 <= 4
argument length      <= 31 bytes
request payload      <= 132 bytes
heap allocation      none
```

Argument bytes must not contain NUL. The target copies accepted argument bytes into bounded NUL-terminated scratch storage before invoking the existing command service. Quotes, escaping, globbing and environment expansion do not exist at the binary layer.

## 8. Stable RPC method IDs

These IDs are public protocol ABI. They are not the internal C dispatch enum and must never be derived from enum ordinal position.

```text
0x0001 ping
0x0002 uptime
0x0003 health
0x0004 rxstat
0x0005 cdcstat
0x0006 mspstat
0x0007 schedprod
0x0008 schedtimed
0x0009 schedprio
0x000A fault
0x000B i2cscan
0x000C oledping
0x000D oledtest
0x000E oledtext
0x000F oledrender
0x0010 oledconsole
0x0011 oledscroll
0x0012 oleddirty
0x0013 oleduiupdate
0x0014 uiruntime
0x0015 oledstatus
0x0016 schedtest
0x0017 schedcoop
0x0018 schedpreempt
0x0019 schedstack
0x001A schedworkload
0x001B schedconsoleprobe
0x001C schedwaitwake
0x001D schedisolate
0x001E wdogtrip
0x001F help
0x0020 rpcinfo
```

Existing names remain the text-shell ABI. These numeric IDs are the binary ABI. Renaming a text method or changing an internal dispatch enum does not silently renumber an accepted RPC ID.

## 9. Chunked response data

Command-service output is streamed as `RPC_DATA` frames rather than buffered as one large response.

`RPC_DATA` payload:

```text
offset  size  field
0       2     RPC method ID
2       2     zero-based sequence number
4       N     raw command output bytes, N <= 48
```

A data chunk is flushed when it reaches 48 bytes, when a newline byte is emitted, or when a returning handler finishes with residual bytes. With the v1 envelope, the largest `RPC_DATA` wire frame is exactly 64 bytes, matching one USB FS bulk max packet.

Raw output bytes preserve current command-service output semantics. The binary envelope supplies method identity, request correlation and structured final status; v1 does not convert every legacy diagnostic payload into a new typed schema.

## 10. RPC completion

A returning RPC request ends with exactly one `RPC_END` frame.

Payload:

```text
offset  size  field
0       2     RPC method ID
2       2     number of RPC_DATA chunks emitted
4       4     total command-output bytes emitted
8       1     status_domain
9       1     status_code
```

Status domains:

```text
0  command service
1  binary protocol policy/parser
```

Command-service status codes preserve the accepted service taxonomy:

```text
0  OK
1  NOT_FOUND
2  BAD_ARGS
3  BUSY
4  INTERNAL_ERROR
```

Protocol status codes:

```text
1  MALFORMED_REQUEST
2  BAD_FLAGS
3  DESTRUCTIVE_CONFIRM_REQUIRED
4  REQUEST_ID_RESERVED
5  PAYLOAD_TOO_LARGE
```

Unknown stable method IDs map to command-service `NOT_FOUND`. Argument-count/argument-shape rejection maps to `BAD_ARGS` where the request is otherwise structurally valid.

`wdogtrip` is intentionally non-returning. When authorized, its `WDOG_TRIP_ARMED\r\n` bytes must be flushed as `RPC_DATA` before watchdog progress is stopped. No `RPC_END` is required before the intentional reset.

## 11. Protocol errors outside RPC completion

`PROTOCOL_ERROR` is used only when a frame header/request ID is trustworthy enough to correlate an error but the frame cannot enter RPC execution. Payload:

```text
offset  size  field
0       1     protocol error code
1       1     observed value / subtype, or 0
```

A bad CRC never produces `PROTOCOL_ERROR`; it is dropped and counted.

An unsupported protocol version may receive a v1 `PROTOCOL_ERROR` with the observed version so a v1-aware host can diagnose negotiation failure.

## 12. TX atomicity and backpressure

A binary wire frame must enter the CDC TX ring atomically: all bytes or none. Gate 1 therefore may add one narrow CDC driver API that attempts to enqueue a bounded byte span atomically without waiting.

The API must not change descriptors, PMA layout, endpoint ownership, IRQ ownership or USB reset behavior. It must not spin waiting for ring space. Failure is explicit and becomes command-service `INTERNAL_ERROR` / binary TX-failure telemetry.

The binary response writer only emits frames of at most 64 bytes in v1.

## 13. Parser recovery

The parser is allocation-free and bounded. It must recover from:

- a lone `A5` not followed by `5A`;
- bad CRC;
- unsupported version;
- unknown frame type;
- length above the accepted inbound maximum;
- malformed RPC payload;
- arbitrary split points across USB OUT packets.

A malformed binary frame must never inject bytes into the text command parser and must never execute a partial command.

## 14. Ownership

USB IRQ remains a bounded byte/ring/event publisher only.

Binary demultiplexing, CRC validation, request decoding, command lookup/execution and binary response framing all execute in production task0 / Thread-PSP.

No binary parser, response writer or USB IRQ path reloads IWDG.

## 15. Non-goals for protocol v1

Not part of this boundary:

- encryption/authentication;
- firmware update or bootloader protocol;
- file transfer;
- framebuffer transfer;
- streaming telemetry/event subscriptions;
- compression;
- USB vendor-specific interface;
- binary transport over UART;
- dynamic allocation;
- a host GUI/control application.

Those features may extend the protocol only after this framing/RPC substrate is independently accepted.
