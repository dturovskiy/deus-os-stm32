# Deus OS — Target / Update Robustness Closure Acceptance Plan

Status: **FDC-08 / GATE 0 ACCEPTED CONTRACT**

Canonical design: `docs/TARGET_UPDATE_ROBUSTNESS_CLOSURE_PLAN.md`

## Gate 1 — static/source

Require exact authorized path set and prove:

- no unbounded HSE/PLL/SYSCLK readiness loop in runtime or bootloader;
- no unbounded USART1 TXE loop;
- no `FIRMWARE_UPDATE_RESET_FALLBACK_POLLS` policy;
- vector validation enforces `handler < APP_BASE + image_length`;
- INFO/BEGIN/AUTHORIZE/END have no blind retry; DATA retry count remains exactly one;
- bootloader/persistence/metadata ownership and update ABI constants remain frozen.

## Gate 2 — build/resource/host

Fresh application and bootloader build with warnings-as-errors, exact vector/resource/stack ceilings, undefined zero, host Core/Transport tests and Release builds. Bootloader remains <= 8192 Flash and accepted SRAM/stack ceilings. Application remains within 52-KiB executable region and accepted SRAM/task-stack margins.

## Gate 3 — deterministic fault paths

Use static/unit seams where hardware failure is not safely injectable to prove bounded countdown termination, authenticated image-span rejection and elapsed-time deadline wrap safety. Preserve normal-path behavior.

## Gate 4 — real hardware

On the exact candidate prove normal boot/runtime/USB/UART/OLED/scheduler/IWDG; explicit ENTER_BOOTLOADER reset occurs within the frozen elapsed-time bound; recovery USB and runtime restoration remain healthy; at least one non-DATA ambiguous-response/adjudication scenario demonstrates no blind mutation/retry; accepted update authenticity/rollback/persistence ownership remain intact; final Flash is adjudicated exactly.

Clock-source physical failure is not to be fabricated by unsafe wiring. Where oscillator-failure injection is unavailable, linked/static bounded-control-flow proof plus normal hardware clock operation is the acceptance surface.

## Gates 5–7

Canonical docs/security reconciliation, normal local acceptance commit and ordinary non-force publication with fresh-fetch clean `0/0`.

Failure classes: `FDC08_UNBOUNDED_WAIT`, `FDC08_VECTOR_SPAN_FAILURE`, `FDC08_RESET_DEADLINE_FAILURE`, `FDC08_UPDATE_ADJUDICATION_FAILURE`, `FDC08_TRUST_MODEL_DRIFT`, `FDC08_RESOURCE_FAILURE`, `FDC08_HARDWARE_RECOVERY_FAILURE`.
