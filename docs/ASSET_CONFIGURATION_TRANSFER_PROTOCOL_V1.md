# Deus OS — Asset / Configuration Binary Transfer Protocol v1

Status: **GATE 0 CONTRACT FROZEN — DOCS ONLY — NO SOURCE IMPLEMENTATION AUTHORIZED**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Protocol family:

`ASSET_CONFIGURATION_TRANSFER_V1`

## 1. Purpose

This contract defines the first-class bounded binary transfer ABI used by the Asset/Configuration foundation.

It is additive to the accepted Binary Framed Transport Protocol v1 and does not reinterpret any accepted frame type, RPC method or command-service behavior.

The protocol is intentionally not a filesystem, generic file transfer service or firmware-update protocol.

Initial accepted consumer:

- object type `0x0001`;
- consumer ID `OLED_UI_LAYOUT_CONFIG_V1`;
- exact consumer payload length `8 bytes`;
- schema version `1`.

The transport has a bounded object ceiling for future promoted non-executable consumers, but no additional object type becomes accepted merely because it fits that ceiling.

## 2. Carrier and scope

The production Asset/Configuration v1 carrier is the accepted USB management interface:

- interface `IF2`;
- bulk OUT endpoint `0x04`;
- bulk IN endpoint `0x84`;
- USB FS max packet `64 bytes`.

Asset/Configuration v1 is **not** accepted over:

- UART;
- the human text shell;
- CDC text commands;
- a network transport.

The accepted binary frame envelope remains transport-neutral, but the initial product feature is exposed only on management IF2.

## 3. Existing frame envelope remains v1

The existing frame envelope is unchanged:

```text
magic              A5 5A
protocol_version   01
payload max        132 bytes
request_id         nonzero for host requests
CRC                CRC-16/CCITT-FALSE
```

Existing accepted frame types remain unchanged.

Two additive frame types are reserved:

```text
0x03  ASSET_TRANSFER_REQUEST
0x85  ASSET_TRANSFER_RESPONSE
```

No existing value is renumbered or reinterpreted.

Unknown transfer opcodes or malformed transfer payloads fail closed.

## 4. Capability negotiation

HELLO protocol capability bit `6` is assigned:

```text
bit 6  ASSET_CONFIGURATION_TRANSFER_V1
```

The accepted HELLO capability flags therefore become `0x0000007F` only after the transfer implementation exists.

This protocol capability is separate from system identity capability bit `5`:

`SYSTEM_IDENTITY_CAP_ASSET_CONFIGURATION_TRANSFER`

System capability bit 5 remains unadvertised until the complete Asset/Configuration boundary passes acceptance.

Normal host product UI/CLI must require the final accepted system capability before exposing persistent mutation as supported functionality. Acceptance harnesses may exercise a pre-publication candidate explicitly.

## 5. Common transfer limits

All multibyte fields are little-endian.

Frozen limits:

```text
transfer protocol version     1
active transfer sessions      1 globally
transfer_id                   uint16, nonzero
object_type                   uint16
object payload max            960 bytes
chunk data max                32 bytes
request frame payload max     42 bytes
response frame payload max    50 bytes
response wire max             62 bytes
session idle timeout          30 seconds
heap allocation               none
```

The 960-byte transport ceiling is chosen so one 1-KiB persistence slot can still reserve exactly 64 bytes for its record envelope. That envelope is a separate Gate 0 contract and must be frozen before Gate 1; this protocol decision does not by itself define persistent layout.

The current object type `0x0001` still requires exact payload length `8`; `960` is not permission to send arbitrary data.

## 6. Full-object integrity

Each BEGIN carries CRC-32/ISO-HDLC for the complete consumer payload.

Parameters:

```text
width       32
poly        0x04C11DB7
init        0xFFFFFFFF
refin       true
refout      true
xorout      0xFFFFFFFF
check       0xCBF43926 for "123456789"
```

The existing per-frame CRC-16 remains mandatory.

Therefore:

- frame CRC-16 protects each wire frame;
- payload CRC-32 protects the reassembled object across the whole transfer;
- consumer validation protects semantic correctness;
- none of these is authentication.

## 7. Request opcodes

`ASSET_TRANSFER_REQUEST` payload byte 0 is transfer protocol version `1`; byte 1 is the opcode.

Frozen opcodes:

```text
0x01  BEGIN
0x02  WRITE_CHUNK
0x03  COMMIT
0x04  ABORT
0x05  STATUS
0x06  READ_CHUNK
```

Frame-level `request_id` correlates one request/response exchange.

`transfer_id` identifies the volatile upload session and is independent of `request_id`.

## 8. BEGIN

BEGIN payload is exactly 16 bytes:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode = BEGIN
2       2     object_type
4       2     transfer_id, nonzero
6       2     total_length
8       1     schema_version
9       1     reserved = 0
10      4     payload_crc32
14      2     reserved = 0
```

For `OLED_UI_LAYOUT_CONFIG_V1`:

```text
object_type      = 0x0001
total_length     = 8
schema_version   = 1
```

BEGIN requires frame flag bit 0 `ALLOW_DESTRUCTIVE`.

Reason: BEGIN authorizes entry into a transfer that may cause the inactive persistence page to be prepared/erased by the implementation.

A second different transfer while one is active returns `BUSY`.

An identical duplicate BEGIN with the same `transfer_id`, object metadata and CRC is idempotent and returns current `next_offset`.

A conflicting duplicate BEGIN with the same `transfer_id` is rejected.

## 9. WRITE_CHUNK

WRITE_CHUNK payload:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode = WRITE_CHUNK
2       2     object_type
4       2     transfer_id
6       2     offset
8       1     data_length, 1..32
9       1     reserved = 0
10      N     raw data, N == data_length
```

Maximum payload is `42 bytes`.

WRITE_CHUNK requires `ALLOW_DESTRUCTIVE`.

Normal ordering is strictly sequential:

`offset == next_offset`

Out-of-order data beyond `next_offset` is rejected.

Retry rule:

- a duplicate chunk wholly below `next_offset` is accepted only if its exact bytes match already-written/read-back candidate bytes;
- an exact duplicate returns OK without advancing `next_offset`;
- any conflicting duplicate returns `DATA_CONFLICT`;
- overlapping partial duplicates are rejected rather than merged.

A chunk may not extend past declared `total_length`.

## 10. COMMIT

COMMIT payload is exactly 6 bytes:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode = COMMIT
2       2     object_type
4       2     transfer_id
```

COMMIT requires `ALLOW_DESTRUCTIVE`.

COMMIT is accepted only when:

- an active matching session exists;
- `next_offset == total_length`;
- full-object CRC-32 matches BEGIN;
- the exact consumer schema validates;
- persistence readback/record validation succeeds.

Only the persistence contract may define the final commit-marker write that makes the candidate authoritative.

A successful COMMIT closes the volatile transfer session.

A repeated COMMIT after success is idempotent only when the device can prove the currently committed record matches the same object type, length and CRC; it returns the committed generation without performing another erase/program operation.

## 11. ABORT

ABORT payload is exactly 6 bytes with the same common fields as COMMIT and opcode `ABORT`.

ABORT does not require `ALLOW_DESTRUCTIVE`.

ABORT:

- drops the volatile session;
- never invalidates the previously committed record;
- may leave a partially programmed inactive slot;
- does not make that inactive slot valid;
- does not require immediate erase cleanup.

Repeated ABORT when no matching session exists is idempotent and returns OK / no-session state.

## 12. STATUS

STATUS payload is exactly 6 bytes:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode = STATUS
2       2     object_type
4       2     transfer_id, 0 queries object/global state
```

STATUS is read-only and frame flags must be zero.

It reports:

- current volatile session state if applicable;
- `next_offset`;
- currently committed generation;
- currently committed payload length;
- committed payload CRC-32;
- whether a valid committed object exists.

## 13. READ_CHUNK

READ_CHUNK reads the currently committed object, not an uncommitted upload candidate.

Request payload is exactly 12 bytes:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode = READ_CHUNK
2       2     object_type
4       4     expected_generation
8       2     offset
10      1     requested_length, 1..32
11      1     reserved = 0
```

Flags must be zero.

If `expected_generation` does not equal the currently committed generation, the request returns `GENERATION_MISMATCH` and no data.

This prevents a multi-request readback from silently mixing generations.

## 14. Response payload

Every accepted/malformed transfer request with a trustworthy frame header receives exactly one `ASSET_TRANSFER_RESPONSE`.

Common response prefix is 18 bytes:

```text
offset  size  field
0       1     transfer_version = 1
1       1     opcode echo
2       1     status
3       1     session_state
4       2     object_type
6       2     transfer_id
8       4     committed_generation, 0 if none
12      2     total_length
14      2     next_offset / read offset after returned data
16      1     data_length, 0..32
17      1     reserved = 0
18      N     data, READ_CHUNK only
```

Maximum response payload is `50 bytes`.

Maximum response wire size is:

`10 + 50 + 2 = 62 bytes`

so every transfer response fits one current 64-byte USB management IN packet.

For non-read responses `data_length == 0`.

## 15. Session states

```text
0  NONE
1  RECEIVING
2  COMPLETE_UNCOMMITTED
```

Committed-object presence is reported independently through `committed_generation != 0`.

A successful COMMIT returns session state `NONE` and the new committed generation.

## 16. Status codes

```text
0   OK
1   MALFORMED
2   BAD_FLAGS
3   BAD_OPCODE
4   BAD_OBJECT_TYPE
5   BAD_SCHEMA
6   BAD_LENGTH
7   BAD_TRANSFER_ID
8   BUSY
9   BAD_STATE
10  BAD_OFFSET
11  DATA_CONFLICT
12  CRC_MISMATCH
13  VALIDATION_FAILED
14  NOT_FOUND
15  GENERATION_MISMATCH
16  STORAGE_ERROR
17  WEAR_BUDGET_EXHAUSTED
18  INTERNAL_ERROR
```

Unsupported object types fail as `BAD_OBJECT_TYPE`; they are not treated as opaque files.

## 17. Disconnect, USB reset and timeout

The transfer session is volatile.

On any of:

- management interface deconfiguration;
- USB bus reset;
- physical disconnect/reconnect;
- 30 seconds without a valid matching transfer request;

the target abandons the volatile session.

After abandonment:

- no partial candidate becomes authoritative;
- previous committed object remains valid;
- an incomplete inactive record remains invalid;
- host must start a fresh BEGIN with a new transfer ID;
- v1 does **not** resume a partial upload after reconnect.

This is an intentional simplicity/recovery property.

## 18. Request retry and idempotency

The protocol does not rely on replaying the same frame `request_id`.

A host retry may use a new request ID.

Idempotency is defined by transfer/object state:

- identical BEGIN: safe;
- exact duplicate already-accepted WRITE_CHUNK: safe;
- repeated STATUS: safe;
- repeated READ_CHUNK against same generation: safe;
- repeated ABORT: safe;
- repeated successful COMMIT: no second Flash mutation when committed identity matches.

Conflicting duplicates fail closed.

## 19. Mutation authorization

Frame flag bit 0 retains the accepted meaning `ALLOW_DESTRUCTIVE` for transfer mutation.

Required:

- BEGIN — yes;
- WRITE_CHUNK — yes;
- COMMIT — yes;
- ABORT — no;
- STATUS — no;
- READ_CHUNK — no.

No additional flag bits are accepted in v1.

This is explicit operator intent, not authentication.

## 20. Parser/memory ownership

Implementation must remain:

- allocation-free;
- bounded by the existing 132-byte frame parser payload buffer;
- one active transfer session;
- small fixed transfer state only;
- no whole-object SRAM buffer assumed.

Payload bytes may be streamed into the inactive persistence candidate under the separately frozen persistence transaction contract.

USB IRQ ownership remains transport/ring publication only. Transfer parsing, semantic validation and persistence control remain Thread/PSP service work.

## 21. Backward compatibility

The implementation must retain:

- protocol version `1`;
- all accepted frame types and meanings;
- RPC service version `3` unless independently advanced for another accepted reason;
- registry `36` unless independently advanced by an additive RPC boundary;
- existing RPC max payload `132`;
- existing RPC data chunk max `48`.

Asset transfer is not tunneled through RPC strings and does not consume new RPC method IDs merely to carry binary chunks.

## 22. Acceptance requirements

Gate acceptance must prove at minimum:

- existing HELLO/RPC tests remain byte-compatible except the additive protocol capability bit;
- frame types `0x03/0x85` reject malformed lengths/flags/opcodes;
- exact max 32-byte chunks;
- exact consumer length/schema rejection;
- sequential write enforcement;
- exact duplicate retry acceptance;
- conflicting duplicate rejection;
- nonzero transfer ID enforcement;
- one-session BUSY behavior;
- whole-object CRC mismatch rejection;
- consumer validation failure rejection;
- COMMIT idempotency without duplicate Flash write;
- STATUS/readback generation consistency;
- disconnect/reset/timeout abandonment;
- no partial activation;
- final management/CDC drop/error regression remains zero;
- capability bit 5 remains unavailable until the full boundary is accepted.

## 23. Non-goals

v1 does not define:

- firmware update;
- executable image upload;
- authentication/encryption;
- arbitrary filenames/paths;
- directory listing;
- append semantics;
- multiple concurrent transfers;
- resume-after-reconnect;
- network transfer;
- generic host file synchronization.
