# Deus OS — Target / Update Robustness Closure Plan

Status: **FDC-08 / GATE 0 CONTRACT FROZEN**

Baseline: `21e45b1dd7b1b5b701b087f44d1b29676ce0e6d9`

## Purpose

Remove the remaining unbounded target waits and tighten the authenticated update/handoff boundary without weakening the published update trust model.

## Confirmed audit findings

The current source still contains:

- unbounded runtime HSE-ready, PLL-ready and SYSCLK-switch waits in `src/kernel.c`;
- unbounded bootloader HSE-ready, PLL-ready and SYSCLK-switch waits;
- unbounded runtime USART1 TXE wait;
- runtime `ENTER_BOOTLOADER` fallback based on `4096` task/service polls instead of elapsed time;
- bootloader reset-handler validation against the entire application region rather than the authenticated `[APP_BASE, APP_BASE + image_length)` span.

Host non-DATA update requests already retain the accepted FDC-01 rule: no blind retry; timed-out IDs are abandoned centrally. DATA alone has one exact timeout retry. FDC-08 must preserve this policy and prove the end-to-end recovery behavior rather than add another retry mechanism.

## Frozen source boundary

Authorized:

- `src/drivers/stm32f103_clock.c` / `include/drivers/stm32f103_clock.h` from FDC-05;
- `src/drivers/usart1.c` / `include/drivers/usart1.h` from FDC-05;
- `src/kernel.c` for fail-closed integration;
- `src/kernel/usb_management.c`;
- `bootloader/bootloader.c`.

No change to firmware package layout, HMAC/digest algorithms, target/product binding, rollback floor, metadata commit ordering, persistence ownership, endpoint topology or public update opcodes is authorized.

## Frozen behavior

- Runtime and bootloader clock readiness/switch waits are finite and return explicit failure to their owner.
- Runtime clock failure enters deterministic fail-closed behavior; it must not spin forever waiting on oscillator state.
- Bootloader clock failure enters deterministic bounded failure/park behavior; no unauthenticated application handoff is created.
- USART1 byte TX is finite. Normal command writers must surface writer failure; emergency/fatal output may be best-effort but can never block forever.
- Runtime update-entry fallback uses wrap-safe kernel elapsed time/deadline, while successful management TX completion may still trigger earlier reset.
- Reset handler Thumb address must be inside the authenticated image span, not stale executable bytes beyond signed `image_length`.
- INFO/BEGIN/AUTHORIZE/END retain no blind retry. Ambiguous response loss is handled by reconnect + INFO/adjudication + safe restart semantics; DATA retains exactly one immediate exact retry.

## Gates

0 contract freeze; 1 exact implementation/static proof; 2 fresh application+bootloader build/resource/stack/host tests; 3 deterministic negative/static fault-path tests; 4 real hardware clock/update-entry/update ambiguity/recovery acceptance; 5 docs/security-ownership reconciliation; 6 local commit; 7 ordinary publication.

## Exit criterion

All listed waits and reset-entry contracts are bounded, vector containment is authenticated-span strict, update trust/rollback/persistence ownership is unchanged, and real hardware recovery/failure acceptance passes on the exact candidate.
