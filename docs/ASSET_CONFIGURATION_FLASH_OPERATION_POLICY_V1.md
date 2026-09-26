# Deus OS — Asset / Configuration Flash Wear, Timing and Runtime Continuity Contract v1

Status: **FROZEN CONTRACT — CONSUMED BY ACTIVE ASSET WIP CANDIDATE — THIS FILE DOES NOT AUTHORIZE NEW FLASH-IMPLEMENTATION CHANGES**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Contract:

`ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1`

## 1. Authoritative silicon basis

For STM32F103x8/xB medium-density devices, ST DS5319 freezes the relevant worst-case Flash characteristics used by this contract:

```text
16-bit programming time    max 70 us
1-KiB page erase time      max 40 ms
Flash endurance            min 10 kcycles/page
programming voltage        2.0..3.6 V
```

ST PM0075 additionally defines:

- main Flash programming in 16-bit halfwords;
- page erase as the normal bounded erase primitive;
- HSI must be ON for program/erase;
- Flash reads/code/data fetches stall while a Flash write/erase operation is in progress;
- program and erase operations must be verified after completion.

These are hardware facts, not measured project estimates.

## 2. Allowed Flash operations

Asset/Configuration v1 may use only:

- unlock/lock FPEC as required;
- erase exactly one inactive persistence page at a time;
- aligned 16-bit halfword programming inside that page;
- readback/verification;
- status/error inspection.

Forbidden:

- mass erase;
- option-byte mutation;
- RDP/WRP mutation;
- application-page erase/program;
- bootloader-region erase/program;
- programming both persistence pages as one transaction.

## 3. HSI and clock rule

Before starting a Flash erase/program primitive, firmware must prove HSI is ON.

Asset persistence must not switch SYSCLK merely to perform Flash programming.

The accepted 72-MHz runtime clock topology remains owned by the existing platform initialization.

If HSI cannot be confirmed available, mutation fails closed before page erase.

## 4. Raw silicon-time budget

A maximum-size 960-byte persistent payload programs at most:

```text
metadata 0x00..0x17       12 halfwords
header CRC-32              2 halfwords
payload 960 bytes        480 halfwords
final commit marker        1 halfword
--------------------------------------
maximum programmed        495 halfwords
```

DS5319 worst-case programming time:

`495 * 70 us = 34.65 ms`

One page erase worst case:

`40 ms`

Maximum silicon BUSY-time arithmetic for one full-size changed commit:

`74.65 ms`

For current `OLED_UI_LAYOUT_CONFIG_V1` with an 8-byte payload:

```text
12 + 2 + 4 + 1 = 19 halfwords
19 * 70 us = 1.33 ms
+ 40 ms page erase
= 41.33 ms worst-case silicon BUSY arithmetic
```

Readback, validation, scheduler work and USB response time are separate from those datasheet BUSY maxima.

## 5. Product timing ceilings

Frozen acceptance ceilings:

```text
single page erase BSY                    <= 40 ms datasheet max
single halfword program BSY              <= 70 us datasheet max
sum of Flash BSY time / changed commit   <= 80 ms
whole mutation phase                     <= 150 ms
COMMIT request -> response               <= 500 ms
host COMMIT timeout                      >= 2000 ms
```

The 80-ms BUSY ceiling is the rounded acceptance bound above the 74.65-ms maximum-size datasheet arithmetic.

The 150-ms mutation ceiling permits bounded software/readback overhead without hiding an unexpectedly slow controller.

The 500-ms protocol ceiling includes normal scheduling and response transmission.

Exceeding a product ceiling is an acceptance failure even if the host has not timed out.

## 6. Same-bank execution stall is expected

The application executes from the same main Flash array that contains persistence.

Therefore v1 deliberately accepts bounded CPU/interrupt service stalls during each Flash program/erase BUSY interval.

v1 does not require:

- relocating the persistence driver to SRAM;
- relocating the vector table to SRAM;
- RAM-resident USB IRQ handling.

The design instead relies on:

- the datasheet-bounded Flash operation times;
- existing USB buffering;
- the long watchdog window;
- deterministic post-operation continuity checks.

If hardware acceptance shows USB/PMA/RX loss cannot remain within the frozen continuity contract, Gate 0 must be reopened; the harness must not silently introduce a RAM execution subsystem.

## 7. Watchdog ownership

The Flash driver and persistence transaction code must **never reload IWDG**.

Existing production liveness ownership remains unchanged:

- task0/task1 progress owns legitimate reloads;
- IRQ/Handler/Flash wait loops do not.

The accepted real IWDG expiration path was approximately 7.3 seconds, while the maximum allowed Asset mutation phase is 150 ms.

Therefore a bounded normal Flash transaction does not need special watchdog servicing.

If Flash logic hangs, IWDG must remain capable of resetting the device.

## 8. Interrupt/critical-section rule

Implementation may use short critical sections for register/state publication where required.

It must not intentionally hold PRIMASK across the entire page erase or across the entire full-record programming transaction.

The Flash array itself can stall code fetch while BSY is active; adding a software-wide interrupt mask around the whole operation provides no accepted benefit and increases latency risk.

After each completed Flash primitive, normal interrupt/scheduler servicing must be allowed before the next independent primitive unless the FPEC programming sequence itself requires an immediately adjacent register operation.

## 9. USB management continuity

A persistent COMMIT is an exclusive management mutation.

While a COMMIT transaction is active:

- the host sends no second management request on IF2;
- target need not process another management frame concurrently;
- USB configuration and descriptors must not change;
- firmware must not deliberately disconnect/re-enumerate USB;
- bus-reset count must not increase due to normal persistence;
- management endpoint/ring ownership remains unchanged.

After the COMMIT response the same open device/transport instance must remain usable.

Required immediate post-COMMIT proof:

1. STATUS or READ_CHUNK for the committed generation;
2. management `ping`/HELLO-equivalent retained protocol check;
3. CDC/UART diagnostics remain available according to their accepted profiles.

No reconnect is part of the normal success path.

## 10. CDC/UART and scheduler continuity

Flash BUSY may delay service temporarily but must not corrupt transport/runtime state.

Hardware acceptance must prove across repeated changed commits:

- management RX/TX drop counts remain zero;
- CDC RX/TX drop counts remain zero during the defined continuity test;
- UART RX drop/error counts remain zero during the defined continuity test;
- scheduler remains active;
- scheduler fault count remains zero;
- task canaries remain intact;
- heartbeat resumes/progresses after each commit;
- no unexpected IWDG reset.

A temporary latency spike during the <=150-ms mutation window is allowed.

Data loss, parser desynchronization, bus reset, scheduler failure or unexpected watchdog reset is not.

## 11. Wear model

DS5319 guarantees a minimum 10,000 erase/program cycles per Flash page under its specified conditions.

Successful changed commits alternate pages A/B.

Asset v1 freezes the successful committed-generation lifetime ceiling:

`10000 changed committed records`

With strict A/B rotation, 10,000 successful changed commits produce at most:

`5000 erase cycles per persistence page`

This consumes at most half of the datasheet minimum endurance and leaves at least 5000 page cycles of datasheet-rated margin per page for interrupted attempts, development/acceptance use and service margin.

Generation above 10,000 is not accepted by the product policy even though the 32-bit record field can represent it.

At generation 10,000:

- unchanged data remains readable and accepted with zero writes;
- STATUS/READ remain available;
- a changed COMMIT returns `WEAR_BUDGET_EXHAUSTED`;
- firmware does not erase either slot.

## 12. Expected product update frequency

`OLED_UI_LAYOUT_CONFIG_V1` is human/operator configuration.

Normal product design assumption:

`long-term average <= 1 changed persistent commit per day`

At that rate 10,000 changed commits exceed 27 years.

Short setup/configuration bursts are allowed but count toward the same lifetime generation budget.

Forbidden automatic wear sources:

- periodic persistence;
- boot-time rewrite;
- display-refresh persistence;
- heartbeat persistence;
- unchanged-value slot rotation;
- automatic repair writes.

## 13. Failed/interrupted attempt policy

An interrupted attempt may consume an inactive-page erase without advancing committed generation.

Therefore the successful-generation counter is not a complete physical erase-cycle meter under repeated hostile power interruption.

v1 controls this risk by policy:

- automatic host retry loops after `STORAGE_ERROR` are forbidden;
- after disconnect/reset during mutation, host re-queries state before a new explicit BEGIN;
- one operator action may not spin indefinitely creating fresh destructive sessions;
- acceptance fault injection records and bounds the exact number of deliberate erase attempts;
- adversarial repeated power-cut wear is not treated as authenticated/authorized-safe behavior in this local unauthenticated boundary.

A future threat model requiring protection against malicious local wear exhaustion belongs to a later authorization/security boundary.

## 14. Unchanged-data suppression

The persistence contract's unchanged-data comparison happens **before erase**.

An unchanged candidate must produce:

```text
erase count delta     0
program count delta   0
generation delta      0
```

This is both a correctness and wear requirement.

## 15. Voltage/preflight rule

The silicon programming range is 2.0..3.6 V.

The accepted current-board preflight observed approximately 3.15..3.16 V.

Before destructive hardware acceptance:

- current-board voltage/protection identity must be revalidated;
- programming is not intentionally exercised outside the documented voltage range;
- a changed physical board/MCU requires renewed preflight.

v1 does not add a speculative PVD/voltage-monitoring subsystem solely for persistence.

Power-loss safety is provided by the A/B commit contract, not by pretending firmware can predict power removal.

## 16. Required instrumentation

Gate 1 may add bounded read-only diagnostics sufficient to accept the contract:

- page erase attempt count since boot;
- halfword program attempt count since boot;
- successful commit count since boot;
- last/max mutation elapsed time;
- last/max erase BSY time;
- last/max program BSY time;
- storage error count;
- wear-policy rejection count.

These counters are volatile diagnostics; they are not themselves persisted.

They must not become a second settings database.

## 17. Gate-3 timing/wear acceptance

Real-hardware acceptance must exercise:

1. unchanged candidate -> zero mutation counters;
2. changed 8-byte consumer commit;
3. changed commit rotating to the other slot;
4. repeated bounded changed commits sufficient to exercise A/B rotation;
5. post-commit STATUS/readback;
6. management/CDC/UART/runtime regression.

Required measurements:

- every page erase <=40 ms observed or datasheet-bound measurement method explicitly classified if host timing cannot resolve BSY exactly;
- every halfword program <=70 us by on-target timer/BSY instrumentation where measurable;
- current-consumer whole mutation <=150 ms;
- COMMIT request-to-response <=500 ms;
- zero unexpected USB bus resets;
- zero management/CDC/UART drops/errors in the continuity profile;
- no IWDG reset;
- scheduler/task/MSP resource invariants retained.

Observed timing materially above the frozen ceilings fails the gate.

## 18. Non-goals

This contract does not:

- make Flash wear cryptographically protected;
- support high-rate logging;
- provide EEPROM emulation;
- create a wear-leveling filesystem;
- move code to SRAM;
- alter IWDG configuration;
- change USB descriptors or packet sizes;
- authorize firmware update.
