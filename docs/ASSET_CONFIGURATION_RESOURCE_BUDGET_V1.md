# Deus OS — Asset / Configuration Resource Budget v1

Status: **GATE 0 CONTRACT FROZEN — DOCS ONLY — NO SOURCE IMPLEMENTATION AUTHORIZED**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Resource contract:

`ASSET_CONFIGURATION_RESOURCE_BUDGET_V1`

## 1. Accepted baseline

Current accepted firmware resource point:

```text
Flash            50652 bytes
SRAM data+bss    11728 bytes
task0 stack      1024 bytes
task1 stack       512 bytes
task0 margin      424 bytes
task1 margin      424 bytes
MSP reserved     2048 bytes
MSP usable       1984 bytes
physical SRAM   20480 bytes
```

The prior Host/USB ceiling `54780/11824` is historical and is not reused.

## 2. Asset-phase Flash geometry

During Asset/Configuration implementation the standalone application remains at:

`0x08000000`

Its physical linker region is frozen to:

```text
ORIGIN = 0x08000000
LENGTH = 54K
exclusive end = 0x0800D800
```

Pages 54..61 remain unused relocation headroom.

Pages 62..63 remain persistence A/B.

## 3. Gate-2 Flash acceptance ceiling

The application physical region is 54 KiB / 55296 bytes.

Gate-2 accepted application image ceiling is stricter:

`54272 bytes = 53 KiB`

Therefore one complete 1-KiB application page remains unused **inside** the 54-KiB application region after Asset acceptance.

Derivation:

```text
physical application budget     55296
mandatory post-Asset reserve    -1024
Gate-2 Flash ceiling            54272
current accepted firmware       50652
Asset implementation growth      3620 bytes maximum
```

The 1-KiB reserve is intentionally page-granular and retained for later mandatory application-side bootloader relocation/handoff/maintenance work.

Asset Gate 1 may not consume it silently.

If implementation cannot fit under 54272 bytes, Gate 0 must be reopened before widening the ceiling.

## 4. Linker/build invariants

Asset Gate 1 must change the standalone linker Flash LENGTH from 64K to 54K without changing origin.

Required:

```text
FLASH ORIGIN = 0x08000000
FLASH LENGTH = 54K
```

Build must fail on region overflow.

Gate 2 must additionally fail acceptance if final loadable Flash usage exceeds 54272 bytes even though the physical linker region is 55296 bytes.

The repository-owned `scripts/build_firmware.ps1` must own this check after Gate 1 and be re-accepted.

## 5. SRAM acceptance ceiling

Physical SRAM is 20 KiB.

The top 2 KiB remains reserved for MSP:

```text
RAM total                     20480
MSP reservation              -2048
address space below MSP       18432
```

Asset/Configuration Gate-2 static SRAM ceiling:

`12288 bytes = 12 KiB`

This leaves an exact minimum **6144-byte physical address gap** between accepted data+bss usage and the reserved MSP region.

From the current accepted 11728-byte point:

```text
new SRAM ceiling              12288
current accepted SRAM         11728
maximum static growth           560 bytes
```

This deliberately forbids whole-object buffering.

## 6. Static-state policy

New Asset/Configuration target state must remain bounded.

Frozen constraints:

- one active transfer session globally;
- no heap;
- no 960-byte object buffer;
- no 1-KiB page mirror;
- no per-transport duplicate transfer workspace;
- transport chunk bytes are consumed from the existing bounded frame payload;
- one volatile pending payload byte is permitted for STM32F1 halfword alignment;
- one exact last-chunk retry cache of at most 32 bytes is permitted for stop-and-wait idempotency;
- response framing uses one bounded <=64-byte wire workspace;
- CRC-32 implementation must be table-free unless a later measured performance trigger justifies a table;
- consumer runtime state remains one validated layout object.

Named new mutable static state owned specifically by transfer/persistence/consumer plumbing should target **<=256 bytes** total.

The authoritative acceptance limit remains the whole-image 12288-byte SRAM ceiling.

## 7. Task-stack invariants

No new task is authorized.

Stack capacities remain:

```text
task0 production/control  1024 bytes
task1 heartbeat            512 bytes
```

Real-hardware acceptance floors remain:

```text
task0 free margin >= 256 bytes
task1 free margin >= 256 bytes
canaries intact
scheduler faults = 0
```

Transfer parsing, Flash transaction orchestration and consumer activation execute in existing Thread/PSP ownership rather than a new task stack.

Compiler stack-usage files are feasibility evidence; runtime watermark/canary measurements remain authoritative.

## 8. MSP invariant

MSP reservation remains exactly 2048 bytes with 1984-byte measurable capacity.

Asset work must not change that reservation.

Hardware acceptance requires:

- MSP canary intact;
- measured high-water recorded under transfer/Flash/USB pressure;
- free measurable MSP margin >= 1024 bytes.

If existing architecture cannot meet that floor, the boundary must be reviewed rather than enlarging MSP silently.

## 9. Frame/transfer memory invariants

Accepted protocol constants remain:

```text
binary frame payload max       132 bytes
asset chunk max                 32 bytes
asset response wire max         62 bytes
object transport max           960 bytes
active sessions                  1
```

The 960-byte maximum is a storage/transport bound, **not an SRAM allocation size**.

A 960-byte future object must still stream without whole-object target buffering.

## 10. Persistence metadata memory

Persistent envelope size is 64 bytes on Flash but must not imply a persistent 64-byte RAM mirror.

Implementation may decode/validate fields incrementally or through a bounded local structure.

No RAM copy of both A/B slots is authorized.

Boot selection may read/validate one slot at a time.

## 11. Host resource policy

Host Core may buffer one bounded object because host memory is not the constrained target resource.

However:

- target limits remain authoritative;
- host must reject objects beyond negotiated/frozen bounds before transfer;
- host retry logic must not create concurrent target sessions;
- UI layer does not own transfer buffers/state independently of Core.

No new host resource ceiling is required for Gate 0 beyond existing deterministic build/test policy.

## 12. Gate-2 required measurements

Fresh build acceptance must record:

- exact BIN byte count and SHA-256;
- `.text/.data/.bss`;
- total Flash usage;
- total SRAM data+bss usage;
- `_ebss`;
- gap from `_ebss` to `_smsp_stack`;
- MSP reservation symbols;
- task stack capacities;
- stack-usage files for all new/changed functions;
- undefined symbols = 0.

Hard pass criteria:

```text
Flash <= 54272
SRAM  <= 12288
task0 capacity = 1024
task1 capacity = 512
MSP reservation = 2048
```

## 13. Gate-3 hardware resource acceptance

After full transfer/persistence/fault/reconnect stress:

- task0 margin >=256;
- task1 margin >=256;
- MSP margin >=1024;
- all canaries intact;
- scheduler fault count 0;
- management/CDC drops remain within exact acceptance contract;
- no unexpected reset;
- final Flash outside pages 62/63 equals the exact firmware candidate.

## 14. Failure policy

Exceeding any frozen ceiling is a Gate-2/Gate-3 failure, not permission to adjust a number inside the harness.

Possible responses:

- reduce implementation footprint/state;
- reuse existing bounded workspace safely;
- simplify the contract;
- explicitly reopen Gate 0 with measured justification.

Silent ceiling growth is forbidden.

## 15. Future bootloader relationship

The 1-KiB post-Asset application reserve is not bootloader Flash.

Bootloader pages remain the separate 8-KiB lower steady-state region.

The retained 1-KiB application reserve exists so later application relocation/handoff changes do not immediately require consuming the full 54-KiB application region.

Bootloader Gate 0 must remeasure application resource state after relocation.
