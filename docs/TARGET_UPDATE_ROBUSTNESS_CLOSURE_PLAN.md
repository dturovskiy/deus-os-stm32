# Deus OS — Target / Update Robustness Closure Plan

Status: **FDC-08 / GATES 0–7 ACCEPTED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`**

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

## Gate 5 accepted state

Exact candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`, application `51972/53248` bytes / SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`, has completed Gates 1–4. Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` proves bounded clock/UART control flow, wrap-safe 250-ms reset-deadline semantics, authenticated image-span vector containment, exact one-retry DATA policy and no-blind-retry non-DATA policy. Final hardware evidence `stm32_os_fdc05_08_consolidated_hardware_gate4_v18_20261007_115017.evidence.zip`, SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`, proves authenticated v3 update, BEGIN reconnect adjudication, DATA exact retry behavior, no END replay, runtime/bootloader recovery, composite reset-deadline acceptance, final candidate runtime recovery, exact signed-image/tail/bootloader/metadata/persistence Flash identity and unchanged trust ownership. Linux USB removal/enumeration timing is retained only as environment observation, not misrepresented as the MCU reset timestamp or a product enumeration bound. Gate 5 docs/security-ownership reconciliation is complete; Gates 6–7 are accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8` by ordinary non-force push with fresh-fetch clean `0/0`.

## Exit criterion

All listed waits and reset-entry contracts are bounded, vector containment is authenticated-span strict, update trust/rollback/persistence ownership is unchanged, and real hardware recovery/failure acceptance passes on the exact candidate.
