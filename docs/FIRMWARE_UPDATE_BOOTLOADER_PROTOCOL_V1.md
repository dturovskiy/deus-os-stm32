# Deus OS — Firmware Update / Bootloader Protocol v1

Status: **FROZEN V1 ABI — IMPLEMENTED / GATES 0–7 ACCEPTED / PUBLISHED / PHYSICAL DEPLOYMENT VERIFIED**

Boundary:

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`

Canonical parent design:

`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`

## 1. Scope

This contract freezes the exact v1 firmware-package and wire ABI implemented by the
accepted Bootloader foundation. Gate-5 hardware acceptance exercises this ABI without
changing it, and post-publication deployment evidence SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706` proves the published v2 update path through exact `Ok/Committed` completion and runtime verification. This document remains a protocol authority, not an independent authorization
for source, linker, target Flash, option-byte or RDP mutation.

The bootloader transport is deliberately independent from the runtime RPC registry and
CDC console. It reuses only the accepted Binary Framed Transport v1 envelope.

## 2. Private development USB identity

Runtime management remains:

```text
VID       0x1209
PID       0x000C
Product   Deus OS Device
Topology  composite CDC + management IF2
```

Bootloader v1 uses a distinct private-test topology identity:

```text
VID       0x1209
PID       0x000D
Product   Deus OS Bootloader
Topology  one vendor-specific bulk interface
```

`0x1209:0x000D` is a pid.codes Test PID and is private/in-house testing only. It is not
a product identity and must not be redistributed, sold or manufactured as such.

Bootloader descriptor topology:

```text
interface       0
class           0xFF
subclass        0x00
protocol        0x00
bulk OUT        0x01
bulk IN         0x81
max packet      64
```

Windows binding uses Microsoft OS 2.0 `WINUSB` compatible-ID descriptors. Linux uses
direct libusb. The bootloader has no CDC, IAD, scheduler, OLED, application RPC registry
or composite runtime function.

## 3. Binary envelope

Every update message is exactly one Binary Framed Transport v1 frame and exactly one
64-byte-or-smaller USB bulk packet.

Envelope remains:

```text
offset size field
0      1    magic0 = A5
1      1    magic1 = 5A
2      1    protocol_version = 01
3      1    frame_type
4      1    flags
5      1    reserved = 00
6      2    request_id
8      2    payload_length
10     N    payload
10+N   2    CRC-16/CCITT-FALSE
```

Therefore:

```text
payload_length <= 52
request_id      1..65535
reserved        0
```

CRC parameters remain the accepted v1 CCITT-FALSE parameters and cover offset 2 through
the final payload byte.

Frame types:

```text
0x04 FIRMWARE_UPDATE_REQUEST
0x86 FIRMWARE_UPDATE_RESPONSE
```

Runtime firmware recognizes only `ENTER_BOOTLOADER` from this request family.
Bootloader firmware recognizes `INFO`, `BEGIN`, `AUTHORIZE_HEADER`, `DATA` and
`END`. Other frame types fail closed.

## 4. Update flags

For `FIRMWARE_UPDATE_REQUEST`:

```text
bit 0  ALLOW_DESTRUCTIVE
bits 1..7 reserved = 0
```

`INFO` requires flags `0`.

`ENTER_BOOTLOADER`, `BEGIN`, `AUTHORIZE_HEADER`, `DATA` and `END` require
`ALLOW_DESTRUCTIVE=1`. Unknown flag bits reject the request.

CRC validity is never destructive authorization.

## 5. Request opcodes and exact payloads

Payload byte 0 is always the opcode.

### 5.1 ENTER_BOOTLOADER = 0x01

Runtime management IF2 only.

```text
payload_length = 1
byte 0         = 01
```

On accepted destructive request, runtime writes the frozen BKP_DR1/DR2 one-shot token,
returns an OK/RESETTING response, then performs a bounded delayed `SYSRESETREQ`.
Bootloader consumes and clears the token.

### 5.2 INFO = 0x02

Bootloader only, non-destructive.

```text
payload_length = 1
byte 0         = 02
flags          = 0
```

### 5.3 BEGIN = 0x03

Bootloader only.

```text
payload_length = 49
byte 0         = 03
bytes 1..48    = exact firmware header
```

BEGIN clears only volatile transaction state. It validates structural bounds and stages
the header in SRAM. It performs **no Flash mutation**.

### 5.4 AUTHORIZE_HEADER = 0x04

```text
payload_length = 33
byte 0         = 04
bytes 1..32    = header_auth_tag
```

The bootloader verifies the exact HMAC, target/product/origin binding and version policy.
No Flash byte may be changed before those checks pass.

After successful authorization it selects the metadata page that does not contain the highest authenticated version-floor record, erases only that target page, and preserves the other authenticated metadata page byte-for-byte. No existing non-erased state marker is reprogrammed. Application pages are erased lazily by DATA, one page at first use.

### 5.5 DATA = 0x05

```text
payload_length = 5..51
byte 0         = 05
bytes 1..2     = byte_offset, uint16 little-endian
bytes 3..N     = data, even length 2..48
```

Rules:

- `byte_offset` is even;
- data length is even;
- `byte_offset + data_length <= image_length`;
- one DATA request may not cross a 1-KiB application page boundary;
- first accepted DATA touching a page erases that page before programming;
- programming is aligned 16-bit halfword only;
- programmed bytes are read back before the request is acknowledged.

Normal acceptance requires `byte_offset == expected_offset`.

Exactly one immediate retry is idempotent: if the request exactly matches the immediately
preceding accepted DATA offset, length and bytes, it returns the previous success without
erasing, programming or updating the payload SHA-256 a second time.

Any other stale, skipped, overlapping or future offset returns `OUT_OF_SEQUENCE`.

The bootloader therefore keeps at most one 48-byte previous-chunk cache; no whole-image
buffer or persistent partial-transfer journal exists.

### 5.6 END = 0x06

```text
payload_length = 1
byte 0         = 06
```

END requires `expected_offset == image_length`. It verifies:

1. streamed payload SHA-256 equals the digest already authenticated by `header_auth_tag`;
2. readback of the complete programmed payload as required by the acceptance build;
3. initial MSP structure/range/alignment;
4. reset-handler Thumb bit and address range.

Only after all checks pass may the inactive metadata slot be written and its
`0xA55A` commit marker programmed last.

## 6. Fixed firmware header

The header is exactly **48 bytes**:

```text
offset size field
0      4    product_id = 0x534F4544
4      1    image_format_version = 1
5      1    reserved = 0
6      2    target_device_id = 0x0410
8      4    image_length
12     4    firmware_version
16     32   payload_sha256
```

All integers are little-endian.

Application origin is deliberately not repeated as a header field. v1 fixes it to
`0x08002000` and includes that fixed 32-bit little-endian value in the header HMAC input.
This preserves explicit origin authentication while keeping BEGIN inside one 64-byte USB
packet.

Bounds:

```text
8 <= image_length <= 53248
image_length % 4 == 0
firmware_version != 0
reserved == 0
```

The application image must be linked for `0x08002000`.

## 7. Firmware package

Exact package bytes:

```text
header[48]
header_auth_tag[32]
application_bytes[image_length]
```

The host updater consumes a pre-authenticated package. The normal update client does
**not** need or receive the secret HMAC key.

## 8. Authentication

v1 uses one 256-bit HMAC-SHA-256 update key injected from an external secret source.
The key is not committed to Git and is not emitted to logs/evidence.

Fixed origin bytes:

```text
application_origin_le32 = 00 20 00 08
```

Domain bytes:

```text
DEUSHDR1 = 44 45 55 53 48 44 52 31
```

Exact tag:

```text
header_auth_tag =
    HMAC-SHA-256(
        key,
        DEUSHDR1 || application_origin_le32 || header[48])
```

The header HMAC authenticates the payload SHA-256 field. The programmed application is independently hashed and must equal that authenticated digest. A second whole-image HMAC is intentionally absent because it would duplicate the same authenticity property while adding streaming crypto state. HMAC comparisons are constant-time.

## 9. Target/version policy

Exact v1 target tuple:

```text
product_id            0x534F4544
target_device_id      0x0410
application_origin    0x08002000 (implicit + authenticated)
image_format_version  1
```

Version floor policy:

- normal update with valid committed app: candidate `> floor`;
- recovery with no valid committed app: candidate `>= floor`;
- candidate `< floor`: reject;
- no authenticated metadata: floor `0`;
- version `0xFFFFFFFF`: v1 counter exhausted, no wrap.

## 10. Response payload

Every `FIRMWARE_UPDATE_RESPONSE` uses exactly 16 payload bytes:

```text
offset size field
0      1    opcode_echo
1      1    status
2      1    state
3      1    reserved = 0
4      2    expected_offset
6      2    reserved = 0
8      4    version_floor
12     4    committed_version
```

All integers are little-endian. `committed_version=0` means no bootable committed app.
`expected_offset` is zero when no authorized receive transaction is active.

Runtime ENTER_BOOTLOADER response uses state `0x80 RESETTING`, offsets/versions zero.

Bootloader states:

```text
0x00 RECOVERY_IDLE
0x01 HEADER_STAGED
0x02 AUTHORIZED
0x03 RECEIVING
0x04 VERIFYING
0x05 COMMITTED
0x80 RESETTING
```

Status values:

```text
0x00 OK
0x01 INVALID_STATE
0x02 BAD_LENGTH
0x03 BAD_FLAGS
0x04 BAD_HEADER
0x05 AUTH_FAILED
0x06 TARGET_MISMATCH
0x07 VERSION_REJECTED
0x08 OUT_OF_SEQUENCE
0x09 FLASH_FAILED
0x0A DIGEST_FAILED
0x0B VECTOR_INVALID
0x0C INTERNAL_ERROR
```

Envelope CRC failure is handled by the Binary Framed parser and does not execute an
update opcode.

## 11. Recovery/idempotency

Transaction state and the previous DATA cache are volatile.

A target reset during an update discards transaction state. Host recovery always begins
again with INFO -> BEGIN -> AUTHORIZE_HEADER and re-sends the complete image.

Persistent partial-resume semantics are explicitly absent in v1.

Boot validity remains owned exclusively by authenticated firmware metadata and the final `0xA55A` commit marker defined in the parent plan. `0xA55A` is written only once from erased `0xFFFF`; there is no in-place retired-marker transition. Older authenticated metadata may remain marked `0xA55A` solely as rollback-floor evidence and is non-bootable when its payload digest no longer matches the current application. Pages 62/63 are never firmware transfer state.

## 12. Security boundary

The HMAC contract protects the supported update transport from unauthenticated firmware
authorization. Because RDP remains disabled and the symmetric key resides in bootloader
Flash, v1 does not claim resistance to an attacker with physical SWD read/write access.

A future production-security boundary may replace HMAC with asymmetric release
signatures. That change must be explicit; it is not implied by this v1 contract.
