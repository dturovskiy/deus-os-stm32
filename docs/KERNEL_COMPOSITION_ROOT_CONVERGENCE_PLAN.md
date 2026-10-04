# Deus OS — Kernel Composition-Root Convergence Plan

Status: **FDC-05 / GATE 0 CONTRACT FROZEN**

Baseline: `21e45b1dd7b1b5b701b087f44d1b29676ce0e6d9`

## Purpose

Close the residual composition-root debt left intentionally after `KERNEL_COMPOSITION_ROOT_DECOMPOSITION`. This is an ownership cleanup, not a line-count exercise and not a framework refactor.

## Confirmed audit finding

`src/kernel.c` still directly owns independent low-level register/state domains for STM32F103 clock/reset control, USART1 hardware plus its RX ring, I2C1 and PC13 status LED. Those domains have natural driver/platform owners and give unrelated hardware concerns independent reasons to change the composition root.

## Frozen source boundary

Authorized new files:

- `include/drivers/stm32f103_clock.h`
- `src/drivers/stm32f103_clock.c`
- `include/drivers/usart1.h`
- `src/drivers/usart1.c`
- `include/drivers/i2c1.h`
- `src/drivers/i2c1.c`
- `include/drivers/status_led.h`
- `src/drivers/status_led.c`

Authorized modified files:

- `src/kernel.c`
- `src/drivers/ssd1306.c` — narrow dependency-seam follow-up: replace the historical manual `i2c1_write()` forward declaration / stale “I2C1 remains in kernel.c” comment with the canonical `drivers/i2c1.h` dependency after I2C1 extraction;
- `scripts/build_firmware.ps1`

FDC-05 may also consume `system_service_state` introduced by FDC-06 but does not own its semantics.

## Ownership contract

- `stm32f103_clock` owns RCC/FLASH clock bring-up and reset-cause capture/clear helpers.
- `usart1` owns USART1/GPIOA/NVIC register programming, bounded TX byte service, the 128-byte RX ring and RX diagnostics.
- `i2c1` owns I2C1/GPIOB/RCC register programming, probe and bounded write behavior.
- `status_led` owns PC13 setup/raw state and active-low writes.
- `src/kernel.c` remains the composition root and owns scheduler policy, task wiring, protocol/service composition, IRQ-to-scheduler event publication and fatal policy.
- Drivers must not call scheduler/application/UI policy.
- `USART1_IRQHandler` may remain root glue but must delegate hardware/ring service to `usart1`; scheduler wake publication remains above the driver.

## Forbidden designs

No universal `kernel_context_t`, service locator, hidden extracted-state `extern`, dependency cycle, heap, HAL, Arduino, FreeRTOS, new task/SVC/queue/mutex/timer framework, or line-count-only split.

## Gates

- Gate 0 — this ownership/source-boundary freeze.
- Gate 1 — exact extraction with static dependency review.
- Gate 2 — fresh firmware build/resource/stack/public-ABI validation.
- Gate 3 — hardware/runtime equivalence including UART/I2C/OLED/USB/scheduler/IWDG and exact Flash candidate.
- Gate 4 — physical OLED disposition (`PASS` when rendering semantics changed; otherwise unchanged-UI automated regression with explicit rationale).
- Gate 5 — documentation and architecture audit reconciliation.
- Gate 6 — normal local acceptance commit.
- Gate 7 — ordinary non-force publication and fresh remote verification.

## Exit criterion

FDC-05 closes only when the natural low-level owners above are no longer root-private implementation domains, no substitute god abstraction or hidden coupling was introduced, build/resource/stack/public ABI remain within accepted ceilings, and hardware equivalence is proven on the exact candidate.
