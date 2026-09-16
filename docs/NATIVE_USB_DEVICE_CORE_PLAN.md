# STM32 OS — Native USB Device Core Foundation Plan

Status: **PUBLISHED — commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8` / tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`**

Boundary ID:

`NATIVE_USB_DEVICE_CORE_FOUNDATION`

## 1. Authority and sequencing

The published baseline is C4.0:

- commit `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`;
- tree `e024d425e97f878c170ccfc41f0505dce79277a3`;
- firmware `25192` bytes /
  `4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D`;
- local/remote repository state clean and synchronized.

The roadmap explicitly orders native USB as:

1. minimal STM32F103 USB Device core on PA11/PA12;
2. USB CDC ACM diagnostic/command console;
3. shell/RPC;
4. binary transport;
5. host control/update tooling.

This boundary implements **step 1 only**.

## 2. Why this is the next architecture boundary

The remaining Phase 4 items are intentionally not pulled forward:

- generic timer callbacks remain deferred until a real consumer requires them;
- message queues/synchronization remain deferred until real shared ownership exists;
- runtime statistics remain a later observability slice.

OLED/status-bar work is also deferred.

Therefore the first non-deferred architecture target is the minimal native USB
Device core.

## 3. Hardware target

MCU:

`STM32F103C8T6 / Cortex-M3 / USB full-speed Device peripheral`

Pins:

```text
PA11  USB_DM
PA12  USB_DP
```

This project remains a **USB Device**, not a general USB Host platform.

## 4. Power discipline

Native USB testing introduces a second possible board power source.

Hard rule:

**Do not power the Blue Pill simultaneously from ST-LINK 3.3 V and micro-USB
VBUS.**

For USB hardware acceptance:

- if micro-USB VBUS powers the board, disconnect ST-LINK 3.3 V;
- ST-LINK GND/SWDIO/SWCLK may remain for recovery/debug where electrically safe;
- UART ground/TX/RX may remain as the low-level diagnostic path;
- never instruct a dual-power setup.

## 5. Clock architecture

The accepted system clock remains unchanged.

Target:

```text
SYSCLK = accepted 72 MHz configuration
USBCLK = valid 48 MHz derived from the existing PLL tree
```

Gate 1 must inspect the current RCC configuration and use the STM32F103 USB
prescaler correctly. It must not introduce a parallel clock initialization
path or globally perturb SysTick/UART/I2C timing.

## 6. Driver/core layering

Planned layering:

```text
future CDC ACM / shell / RPC
            |
     USB device core API
            |
 EP0 control-transfer engine
            |
 endpoint + PMA ownership
            |
 STM32F103 USB registers / IRQ
```

The USB register layer does not own scheduler policy.

## 7. USB peripheral foundation

Gate 1 is expected to establish a minimal direct-register USB FS device core:

- peripheral clock/reset control;
- deterministic USB peripheral reset/init;
- USB interrupt enable/acknowledge;
- explicit BTABLE location;
- explicit Packet Memory Area allocation;
- endpoint register helpers that preserve STM32F1 toggle/status semantics;
- endpoint 0 RX/TX buffer ownership;
- USB reset handling;
- SETUP packet capture;
- control-transfer state tracking;
- device address programming at the correct control-transfer phase.

No HAL.

## 8. Endpoint 0 / standard requests

The first core must support only the standard control behavior required for
minimal host enumeration.

Expected request families include:

- `GET_DESCRIPTOR`;
- `SET_ADDRESS`;
- `GET_CONFIGURATION`;
- `SET_CONFIGURATION`;
- any additional mandatory standard request discovered by the exact target-host
  enumeration trace.

Do not add CDC class requests in this boundary.

Unsupported requests fail deterministically with the correct endpoint behavior;
they do not fall through into arbitrary memory or application logic.

## 9. Descriptor identity policy

USB identity is a gate, not an incidental constant.

Rules:

- VID/PID live in one centralized descriptor/policy location;
- do not silently copy ST, Arduino, ST-Link, commercial, or other third-party IDs;
- Gate 1 source mutation must fail closed until an explicit lawful development
  identity policy is documented;
- product-facing identity can change later without changing the device-core
  architecture.

Strings/descriptors must have bounded static storage and validated lengths.

## 10. Interrupt ownership

The USB low-priority device interrupt path may:

- read/ack USB interrupt state;
- move endpoint/PMA data required by the USB protocol engine;
- advance bounded USB core state;
- publish completion/state for later class layers.

It must not:

- reload IWDG merely because an IRQ fired;
- busy-loop waiting for host traffic;
- call OLED rendering;
- become a second shell/console owner;
- add scheduler blocking semantics.

## 11. IWDG interaction

C4.0 remains authoritative.

USB IRQ/handler paths are **not** watchdog liveness proof.

IWDG reload remains tied to accepted Thread/PSP production progress only.
Adding USB must not add handler-mode reload or weaken `wdogtrip`.

## 12. UART / recovery ownership

UART remains:

- emergency diagnostic console;
- hardware acceptance control channel;
- regression transport while USB is immature.

ST-LINK remains:

- flash/recovery;
- SWD debug;
- last-resort recovery if USB firmware is broken.

Native USB does not replace either path in this boundary.

## 13. Scheduler / IPC scope

No new task is justified solely by EP0 enumeration.

No new:

- SVC;
- queue;
- semaphore;
- mutex;
- priority inheritance;
- generic timer callback;
- runtime-statistics subsystem.

If a later CDC transport creates real cross-task ownership, IPC may then be
introduced as its own architecture boundary.

## 14. OLED scope

No OLED/gfx/status-bar source edit is authorized.

If the hashes remain exact and `uiruntime` passes, physical OLED acceptance is:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

## 15. Expected Gate 1 source investigation

Before source mutation, Gate 1 must inspect the exact published code and resolve:

- current RCC clock programming and USB prescaler state;
- startup/vector naming for the STM32F103 USB low-priority IRQ;
- existing register/helper conventions;
- where USB core state should live without moving policy into drivers;
- legal development VID/PID policy;
- exact source path set.

Planning does **not** authorize guessing those details.

## 16. Non-goals

Not in this boundary:

- CDC ACM data endpoints;
- USB shell/RPC;
- binary transport;
- USB firmware update;
- USB bootloader;
- USB Host;
- HID product work;
- networking;
- generic timers;
- IPC/synchronization;
- runtime statistics;
- OLED UI redesign.

## 17. Gate order

```text
Gate 0  planning/docs synchronization
Gate 1  exact source investigation + implementation, with USB identity resolved
Gate 2  fresh GNU build/link/static validation
Gate 3  real hardware USB device/enumeration acceptance
Gate 4  OLED conditional N/A review
Gate 5  docs/evidence finalization
Gate 6  local acceptance commit
Gate 7  ordinary non-force publication
```

Never combine source mutation, build, hardware acceptance, commit and push into
one gate.
