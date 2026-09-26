# Deus OS — Asset / Configuration Persistent A/B Record Contract v1

Status: **FROZEN CONTRACT — CONSUMED BY ACTIVE ASSET WIP CANDIDATE — THIS FILE DOES NOT AUTHORIZE NEW FLASH-IMPLEMENTATION CHANGES**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Persistence contract:

`ASSET_CONFIGURATION_PERSISTENCE_V1`

## 1. Physical ownership

The shared Flash decision owns two independent 1-KiB erase pages:

```text
slot A  page 62  0x0800F800..0x0800FBFF
slot B  page 63  0x0800FC00..0x0800FFFF
```

No other page is part of Asset/Configuration persistence v1.

The current application remains reset owner at `0x08000000` during the Asset phase.

## 2. Slot geometry

Each slot is exactly 1024 bytes:

```text
offset 0x000..0x03F   64-byte record envelope
offset 0x040..0x3FF   payload capacity, maximum 960 bytes
```

The transfer protocol ceiling of 960 bytes is therefore exactly equal to persistent payload capacity.

Initial object type `0x0001` uses only 8 payload bytes.

Unused payload bytes remain erased and are not part of payload integrity.

## 3. Envelope byte layout

All multibyte fields are little-endian.

```text
offset  size  field
0x00    4     magic = bytes 44 45 55 53 ("DEUS")
0x04    1     envelope_version = 1
0x05    1     consumer_schema_version
0x06    2     object_type
0x08    2     header_bytes = 64
0x0A    2     payload_length, 1..960
0x0C    4     generation, nonzero
0x10    4     payload_crc32
0x14    2     flags = 0
0x16    2     reserved0 = 0
0x18    32    reserved, must remain erased 0xFF
0x38    4     header_crc32
0x3C    2     reserved_tail, must remain erased 0xFFFF
0x3E    2     commit_marker
0x40    ...   payload
```

Magic numeric little-endian value is `0x53554544`.

The only valid committed marker is:

`0xA55A`

Erased/uncommitted value is `0xFFFF`.

Any other marker is invalid.

## 4. CRC definitions

Both `payload_crc32` and `header_crc32` use CRC-32/ISO-HDLC frozen by the transfer ABI.

`payload_crc32` covers exactly `payload_length` payload bytes beginning at offset `0x40`.

`header_crc32` covers exactly envelope bytes `0x00..0x17` (24 bytes).

It does not cover:

- reserved erased area;
- the `header_crc32` field itself;
- reserved tail;
- commit marker;
- payload.

Reserved regions are validated separately as exact erased values.

## 5. Record validity

A slot is a valid committed record only if **all** checks pass:

1. commit marker equals `0xA55A`;
2. magic equals `DEUS`;
3. envelope version equals `1`;
4. header size equals `64`;
5. object type is supported;
6. schema version is supported for that object;
7. payload length is valid for that exact consumer and never exceeds `960`;
8. generation is nonzero;
9. flags/reserved0 are zero;
10. reserved bytes `0x18..0x37` remain `0xFF`;
11. reserved tail remains `0xFFFF`;
12. header CRC-32 matches;
13. payload CRC-32 matches;
14. consumer semantic validation passes.

Integrity is checked before activation.

A commit marker by itself is never sufficient.

## 6. Initial consumer constraints

For `OLED_UI_LAYOUT_CONFIG_V1`:

```text
object_type              0x0001
consumer_schema_version  1
payload_length           exactly 8
```

Its semantic validation remains owned by `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`.

## 7. Generation rules

Generation is an unsigned 32-bit monotonically increasing committed-record sequence.

Rules:

- no valid record -> first commit generation `1`;
- next commit -> active generation + 1;
- generation `0` is invalid;
- generation does not wrap;
- if active generation is `0xFFFFFFFF`, further persistent commits are rejected fail-closed.

An interrupted/uncommitted candidate does not become the active generation.

The wear/timing contract may impose a much smaller operational lifetime budget.

## 8. Boot-time selection

Each slot is validated independently.

Selection:

```text
A invalid, B invalid -> compiled default
A valid,   B invalid -> A
A invalid, B valid   -> B
A valid,   B valid   -> higher generation
```

If both are valid with equal generation:

- if the complete meaningful record identity and payload are byte-identical, slot A is selected deterministically and an ambiguity diagnostic is recorded;
- otherwise neither record is allowed to win; firmware uses compiled default and records an ambiguity fault/diagnostic.

Normal product writes must never create equal generations in two slots.

## 9. Inactive-slot choice

For a new changed object:

- active A -> candidate B;
- active B -> candidate A;
- no valid active record -> candidate A.

The active committed slot is never erased to prepare its replacement.

This preserves one previous valid record until the candidate has become independently committed and verified.

## 10. Unchanged-data suppression

Before any erase/program operation, compare the candidate's object type, schema, length and exact consumer payload with the selected committed record.

If identical:

- return the existing committed generation;
- perform zero page erase operations;
- perform zero Flash program operations;
- do not rotate slots.

Compiled default with no committed record is **not** implicitly written during boot.

## 11. Candidate programming sequence

For changed data, implementation order is frozen:

1. determine authoritative current record/default;
2. choose inactive slot;
3. erase **only** the inactive 1-KiB page;
4. confirm the page is erased;
5. program envelope metadata with commit marker left erased;
6. program payload sequentially;
7. program `header_crc32`;
8. read back envelope/payload;
9. validate all metadata except commit marker;
10. verify payload CRC-32 and consumer semantics from readback bytes;
11. program the 16-bit commit marker `0xA55A` as the **final authority-changing Flash program operation**;
12. read back the commit marker;
13. validate the complete slot again using the normal committed-record validator;
14. only then select/activate the new consumer object in runtime state.

The previous slot remains untouched through step 14.

## 12. Halfword programming rule

The persistence implementation must program Flash in the target's supported aligned programming unit.

Envelope fields and the final commit marker are halfword-aligned.

For an odd payload length, the final programmed halfword uses:

- low byte = final payload byte;
- high byte = erased value `0xFF`.

Payload CRC covers only declared payload bytes, never padding.

No code may intentionally program the commit-marker halfword before all other candidate verification is complete.

## 13. Reset/power-loss outcomes

For reset/power loss at any point before final commit marker programming:

- old committed slot remains authoritative if one exists;
- candidate slot is invalid because commit marker is not exact;
- if no old valid slot exists, compiled default is used.

For reset/power loss after the commit marker is programmed:

- new slot is authoritative only if it passes the complete normal validator;
- otherwise old valid slot wins;
- if neither validates, compiled default wins.

No interrupted state may require guessing which payload is active.

## 14. Partial candidate handling

A partial/incompatible candidate may remain physically programmed in the inactive page.

It is not authoritative.

Cleanup is lazy:

- next valid BEGIN targeting that inactive slot erases it before reuse;
- boot does not erase invalid slots merely because they are invalid;
- ABORT does not require an erase.

This avoids unnecessary wear and boot-time Flash mutation.

## 15. Activation ordering

Persistent authority and runtime activation are separate steps.

Correct order:

```text
candidate receive
 -> transport integrity
 -> complete persistent candidate
 -> readback verification
 -> final commit marker
 -> committed-record revalidation
 -> authoritative selection
 -> runtime consumer activation
```

Runtime activation before persistence commit is forbidden for a persistent COMMIT request.

A volatile preview feature, if ever added, would require a separate future contract.

## 16. Readback

STATUS/READ_CHUNK exposes only the currently selected committed object.

It must not expose an incomplete candidate as though committed.

Readback reports:

- selected generation;
- exact payload length;
- payload CRC-32;
- payload bytes.

If no valid committed record exists, persistent readback returns NOT_FOUND; the compiled default remains runtime fallback but is not falsely represented as a committed Flash record.

## 17. Error handling

Flash controller/program/verify failure:

- never programs the final commit marker after a failed prerequisite;
- leaves old committed record unchanged;
- reports STORAGE_ERROR;
- abandons the volatile transfer session;
- requires a fresh BEGIN for retry.

A commit-marker write/readback failure is treated the same way.

## 18. Object-type isolation

v1 accepts only explicitly promoted object types.

Initial set:

```text
0x0001 OLED_UI_LAYOUT_CONFIG_V1
```

An unknown object type is invalid even if its envelope, CRC and size are otherwise well formed.

Adding an object type requires its own consumer contract and acceptance.

## 19. Boot behavior

Boot-time persistence processing is read-only.

Boot must not:

- erase an invalid slot;
- repair a slot automatically;
- rewrite the selected record;
- persist the compiled default;
- advance generation.

Boot only validates, selects and activates.

## 20. Ownership and non-goals

Asset/Configuration owns erase/program operations only inside pages 62 and 63.

It does not own:

- bootloader pages;
- application code pages;
- option bytes;
- firmware-update metadata;
- generic key/value storage;
- filesystem allocation.

The bootloader may not repurpose pages 62/63 without reopening the shared Flash decision.

## 21. Acceptance requirements

Gate acceptance must prove:

- exact 64-byte envelope layout;
- exact A/B addresses;
- exact final commit-marker ordering;
- unchanged-data zero-write suppression;
- first commit to A;
- next changed commit to B;
- old slot retained until new slot complete;
- valid/invalid selection matrix;
- generation ordering;
- equal-generation ambiguity behavior;
- corrupt header CRC fallback;
- corrupt payload CRC fallback;
- unsupported schema/type fallback;
- incomplete commit-marker fallback;
- odd-length padding rule with CRC over exact length;
- boot performs no Flash mutation;
- final runtime object equals selected persisted payload or compiled default.
