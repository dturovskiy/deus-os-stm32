# STM32 OS — Native USB Device Core Foundation Acceptance Plan

Status: **COMPLETED / PUBLISHED — commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`**

Boundary ID:

`NATIVE_USB_DEVICE_CORE_FOUNDATION`

## Gate 0 — planning/docs synchronization

Required:

- published HEAD `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`;
- published tree `e024d425e97f878c170ccfc41f0505dce79277a3`;
- subject `feat: add IWDG liveness foundation`;
- `main`, `origin/main`, live remote `main` all exact;
- worktree clean;
- real Git index clean;
- all published source guards exact;
- install/update only the nine planning documents;
- temporary-index `git diff --cached --check` PASS;
- real index remains unchanged;
- no source edit/build/flash/commit/push.

Next:
`NATIVE_USB_DEVICE_CORE_SOURCE_IMPLEMENTATION`.

## Gate 1 — source investigation + implementation

Before mutation, prove from the exact published source:

- current 72 MHz RCC/PLL configuration;
- correct 48 MHz USB clock derivation;
- exact startup/vector location and USB low-priority IRQ naming;
- exact source path set;
- USB identity policy resolved.

Fail before source mutation if the USB identity policy is unresolved.

Implementation requirements:

- direct-register STM32F103 USB Device core;
- PA11 `USB_DM`, PA12 `USB_DP`;
- explicit PMA/BTABLE ownership;
- endpoint 0 RX/TX buffers;
- USB reset handling;
- SETUP capture;
- bounded EP0 control-transfer state machine;
- standard requests needed for minimal enumeration;
- correct delayed `SET_ADDRESS` semantics;
- no CDC class/data endpoints;
- no new task solely for enumeration;
- no new SVC/IPC/timer/runtime-statistics subsystem;
- no IWDG reload from USB IRQ/Handler;
- no OLED/gfx/status-bar edit.

Failure before flash:
restore exact pre-source bytes.

## Gate 2 — fresh GNU build/link validation

Use fresh isolated build output.

Required:

- `-Wall -Wextra -Werror`;
- exact candidate source tree;
- USB device-core translation unit(s) actually compiled and linked;
- USB IRQ/vector reachability proven;
- no duplicate handler/vector ownership;
- endpoint/PMA buffers fit SRAM;
- Flash/RAM budgets remain valid;
- existing task0/task1/MSP stack gates retained;
- IWDG start/reload ownership retained;
- scheduler/timed/priority/PRIMASK regressions retained;
- safe production command surface unchanged unless explicitly planned;
- no stale binary accepted.

No flash in this gate.

## Gate 3 — real USB hardware acceptance

Power precondition:

**Do not power the board simultaneously from ST-LINK 3.3 V and micro-USB VBUS.**

If USB VBUS powers the target, ST-LINK 3.3 V must be disconnected.

Required hardware proof:

1. flash/verify the exact Gate 2 candidate only;
2. preserve UART as the acceptance control/diagnostic path;
3. prove normal boot, scheduler ownership, IWDG active and heartbeat;
4. attach native micro-USB data link;
5. prove USB bus reset is observed;
6. prove endpoint 0 receives valid SETUP traffic;
7. prove host obtains the expected device descriptor;
8. prove host sets an address and the device enters the addressed state;
9. prove the host reaches the planned minimal enumeration/configuration state;
10. repeat bus reset / re-enumeration enough times to reject one-shot state bugs;
11. disconnect/reconnect USB and recover without reflashing;
12. retain safe production surface regression;
13. retain timed/priority/scheduler BUSY/UART race regression;
14. retain IWDG normal reload and destructive watchdog proof as required by
    risk-based acceptance;
15. require RX drops/errors and stack guards to remain acceptable;
16. final Flash readback exact.

Host evidence must record the enumerated VID/PID, descriptors and device state.
The exact identity must match the Gate 1 documented identity policy.

CDC ACM data transport is not required and must not be claimed by this gate.

## Gate 4 — OLED conditional review

If OLED/gfx/status-bar source hashes are exact and `uiruntime` passes:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Otherwise physical OLED acceptance becomes mandatory.

## Gate 5 — docs/evidence finalization

Record:

- exact source tree;
- candidate size/SHA;
- USB identity policy;
- USB clock proof;
- enumeration evidence;
- reset/re-enumeration evidence;
- retained IWDG/scheduler/UART/stack results;
- OLED Gate 4 disposition;
- next gate.

No build/flash/commit/push.

## Gate 6 — local acceptance commit

Stage only the accepted boundary paths.

Required:

- exact parent;
- exact accepted tree;
- `git diff --cached --check`;
- one local commit;
- exact subject/path set;
- clean worktree/index;
- remote still at parent;
- behind `0`, ahead `1`.

No push.

## Gate 7 — ordinary non-force publication

Pre-push:

- local commit exact;
- worktree/index clean;
- remote at parent;
- fast-forward ancestry;
- behind `0`, ahead `1`.

Execute exactly one ordinary:

`git push origin main:main`

No force.
No force-with-lease.

Post-push:

- local HEAD/tree unchanged;
- `origin/main` and live remote exact published commit;
- behind `0`, ahead `0`;
- worktree/index clean.

## Acceptance record — 2026-09-16

Gates 0–5 are accepted.

Build evidence:

- Gate 2 evidence `5E261F473EA48CAA7ABD2B080E22732D95EEEA24BEA76349A4A10E66088FC81F`;
- source candidate tree `4b798382ef843ef8f488115624d48c0cd1506c75`;
- candidate `43812` bytes /
  `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`;
- accepted 72 MHz SYSCLK retained;
- `USBPRE=0`, therefore USB clock is `48 MHz` from PLL /1.5;
- IRQ20/vector reachability and direct-register USB core linkage passed.

Identity:

- development VID `0x1209`;
- development PID `0x000A`;
- product `Deus OS USB Core`;
- private testing only; not a product/manufacturing identity.

Hardware evidence:

- log `F612D710A6029ADACD0AE51BF5F85B2F5983F22003A6C09787C99045C520ECEC`;
- evidence ZIP `14E7751AFBACEED96DF5816B81969AB33C499FAF182F19BBA89A22A21E2C6B43`;
- initial enumeration: exact device/config descriptors, address `20`, configuration `0`;
- physical disconnect observed and reconnect recovered without reflash at address `21`;
- post-IWDG host control-transfer proof passed;
- total accepted USB enumeration/control-transfer proofs: `3`;
- safe production surface pre-IWDG `20/20` and post-IWDG `20/20`;
- controlled timed-event wake `34 ms` pre-IWDG and `35 ms` post-IWDG;
- scheduler invasive diagnostics `8/8` exact `SCHED_DIAG_BUSY`;
- UART race `128/128 PONG` with zero RX drops/errors;
- deliberate IWDG reboot after `8748 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- final Flash readback exactly matched the accepted candidate while USB VBUS remained the sole target power source.

Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 4 disposition SHA-256:

`FCC6B971D7CCBDDA7866CBD92B56D33955D328EF1D368690B132E06A6801F676`

Gate 5 docs/evidence finalization is accepted. Gate 6 local acceptance commit and Gate 7 ordinary non-force publication are complete.

Published commit/tree:

`3f55f624b72b4c5266ec0e4b0006839c4478bec8` /
`52c2a0efacf9c533d7664316dbfcac344cb2d742`.

Next boundary:

**`USB_CDC_ACM_CONSOLE_FOUNDATION` is published at `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`; `SHELL_RPC_FOUNDATION` is published at `0c33304d2db86e54d715905393f49147bb6dd2ea`; current boundary is `BINARY_FRAMED_TRANSPORT_FOUNDATION`.**

## Permanent retained constraints

- bare metal; no HAL/Arduino/FreeRTOS;
- UART remains emergency console;
- ST-LINK remains recovery/debug;
- no dual ST-LINK-3.3V + micro-USB-VBUS board powering;
- IWDG Handler reload remains forbidden;
- scheduler core semantics remain unchanged unless a later boundary explicitly
  authorizes a scheduler change;
- generic timers deferred;
- queues/synchronization deferred;
- runtime statistics later;
- OLED frozen.
