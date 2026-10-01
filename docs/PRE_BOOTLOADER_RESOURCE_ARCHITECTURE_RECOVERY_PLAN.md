# Deus OS — Pre-Bootloader Resource / Architecture Recovery Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `a8f92f83c2ba8917ad183b1a099c9e21199c9463` — COMPLETE**

Boundary:

`PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY`

Published parent:

- commit `562e786ffa734da055c23144ec4256bc8961bbaf`;
- commit tree `88720a614794d5aef93cf13ac762a4095cb17baf`;
- subject `Accept asset configuration transfer foundation`;
- `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` Gates 0–7 accepted/published;
- accepted activation firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`;
- accepted BIN SHA-256 `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`;
- accepted resource state Flash `54268/54272`, SRAM `11944/12288`;
- accepted system capability mask `0x0000003F`;
- final Asset persistence wear counter `46/64`.

Canonical acceptance:

`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_ACCEPTANCE_PLAN.md`

## 1. Purpose

Recover explicit target-resource margin and reduce measured ownership concentration before
`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` begins.

This is behavior-preserving engineering. It does not add firmware-update semantics,
change the published Asset/Configuration ABI, relocate the application, take reset
ownership away from the application, change USB identity, or introduce new product
features.

The boundary exists because the accepted Asset image has only:

- `4` bytes remaining under the frozen `54272`-byte Flash acceptance ceiling;
- `344` bytes remaining under the frozen `12288`-byte static SRAM ceiling.

Those margins are too small for disciplined continuation even though the current image
still fits the physical 54-KiB application region.

## 2. Audit facts frozen by Gate 0

Read-only repository audit before this plan found:

- `src/kernel.c`: `3459` LOC and approximately `95` functions; it still owns
  multiple independent reasons to change, including low-level RCC/GPIO/UART/I2C glue,
  production task/liveness state, OLED diagnostic paths, console/domain dispatch and
  top-level composition;
- `src/drivers/usb_device.c`: `2365` LOC;
- `src/kernel/scheduler.c`: `1859` LOC;
- `src/kernel/scheduler_diagnostics.c`: `1361` LOC;
- `host/src/DeusOs.Control.Core/DeusDeviceClient.cs`: `1004` LOC;
- target heap API use: none;
- lower-layer dependency audit found no driver/gfx -> kernel dependency violations;
- the Asset boundary itself added only `+11/-3` lines to `src/kernel.c`; new
  persistence/transfer complexity was kept in dedicated modules;
- the build already uses `-ffunction-sections`, `-fdata-sections` and
  `--gc-sections`; therefore linked resident diagnostics are reachable rather than
  ordinary dead-code GC failures;
- the normal build uses `-O2` for most sources and `-Os` only for the four
  Asset-size-sensitive sources;
- production command registry still exposes scheduler/OLED diagnostic commands, so
  substantial target self-test/diagnostic code remains reachable in the production
  image;
- `scheduler_init()` owns default
  `scheduler_task_stacks[2][128]` = `1024` bytes, while normal `kernel_main()`
  rebinds task0/task1 to separate production stacks of `1024` and `512` bytes.
  This makes the default 1-KiB scheduler stack store a concrete production-profile
  SRAM-recovery candidate, subject to build/hardware proof.

These are trigger facts, not authorization to patch source in Gate 0.

## 3. Required order

The sequence is frozen as:

1. post-Asset publication documentation reconciliation;
2. measurement-only current-image resource attribution;
3. freeze exact recovery choices from measurements;
4. behavior-preserving target resource recovery;
5. production-versus-acceptance diagnostic profile only if measurements justify it;
6. second composition-root convergence only where a concrete owner removes a measured
   blocker or a Bootloader-triggered reason to change;
7. fresh build/resource/stack proof;
8. hardware/runtime regression of the exact production bytes;
9. docs/evidence finalization;
10. one normal local acceptance commit;
11. ordinary non-force publication;
12. only then begin `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` Gate 0.

Host `DeusDeviceClient` decomposition is tracked separately. It is not allowed to
inflate this target-resource boundary. Bootloader Gate 0 may promote that host refactor
before adding update-client behavior if the new update protocol would otherwise add
another responsibility to the existing client.

## 4. Gate 1 — measurement-only resource attribution

Gate 1 performs no target I/O and no product-source mutation.

Using the canonical Windows Arm GNU toolchain and exact published source, preserve and
compare:

- exact baseline BIN/ELF/MAP;
- `arm-none-eabi-size` totals and section breakdown;
- `arm-none-eabi-nm -S --size-sort` linked-symbol attribution;
- per-object section sizes;
- all GCC `.su` stack-usage outputs;
- exact source/object compiler flags.

Run controlled build-only variants, one family at a time, with `-O2 -> -Os` for at
least:

- `src/kernel.c`;
- `src/kernel/scheduler.c`;
- `src/kernel/scheduler_diagnostics.c`;
- `src/drivers/usb_device.c`;
- gfx/OLED presentation modules.

Do not combine variants until individual deltas are known.

Gate 1 must also record current USB CDC/management RX/TX high-water and accepted pressure
evidence before any later proposal to shrink ring capacity. Ring sizes must not be
changed from intuition alone.

Gate 1 output must freeze the smallest behavior-preserving source/profile change set.
No source implementation is authorized before that attribution is reviewed.

Gate 1 measurement was accepted from
`stm32_os_preboot_resource_gate1_measurement_v2_20260926_175102.evidence.zip`:

- baseline reproduced exact accepted BIN SHA-256
  `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`,
  Flash `54268`, SRAM `11944`;
- isolated `src/kernel.c -O2 -> -Os`: Flash `52620`, delta `-1648`;
- isolated scheduler core `-O2 -> -Os`: Flash `52908`, delta `-1360`;
- isolated USB device `-O2 -> -Os`: Flash `52668`, delta `-1600`;
- isolated gfx/OLED presentation `-O2 -> -Os`: Flash `52760`, delta `-1508`;
- scheduler diagnostics `-Os` compile succeeded but link was blocked by a
  compiler-generated unresolved `memset` under the current `-nostdlib` runtime
  contract, so that variant is not selected;
- all linked variants left static SRAM at `11944`;
- exact four USB ring high-water values are still unavailable in accepted text
  evidence, therefore `RING_RESIZE_AUTHORIZED=NO`.

The exact Gate-2 recovery set is frozen as:

1. add `src/kernel.c` to the repository-owned `-Os` source profile;
2. remove the scheduler-owned
   `scheduler_task_stacks[2][128]` 1024-byte default BSS store from production
   ownership;
3. make `scheduler_init()` initialize task descriptors without implicit stack
   backing, preserving the existing explicit `scheduler_task_stack_bind()` contract;
4. keep production boot on the already-existing `1024/512` console/heartbeat
   stacks;
5. preserve scheduler diagnostic/self-test behavior by borrowing existing production
   stack storage while the scheduler is inactive: generic two-task tests split the
   1024-byte console stack into two 512-byte diagnostic stacks, while the console
   probe uses the full console stack plus the existing 512-byte heartbeat stack as
   its peer stack; internal self-test call signatures may carry explicit borrowed-stack
   arguments, while externally visible command/RPC/application ABI remains unchanged;
6. do not change USB ring capacities, command/RPC IDs, linker layout, persistence
   ownership, diagnostic availability or Bootloader semantics.

The SRAM effect of removing the duplicate scheduler store is deterministic at the
current baseline: `11944 - 1024 = 10920` bytes before any incidental compiler/linker
layout changes. Combined with the measured `kernel.c -Os` result, this set is the
smallest measured change set expected to satisfy both recovery objectives without a
diagnostic-profile split. Gate 3 must prove the actual linked result; these arithmetic
values are selection evidence, not acceptance of an unbuilt candidate.

Gate 2 accepted the exact six-file implementation from
`stm32_os_preboot_resource_gate2_recovery_recomposed_v1_20260926_210150.evidence.zip`
with evidence SHA-256 `D84AC967C204FC9D09B94EC8AD7B472B173BCDCE4593320F88601B062681E325`
and normalized accepted source-diff SHA-256
`D1EC010F55313DA61DC1366E99E68A619CF4E678EDD2EE443E7D53AB08AD6A06`.
The real index stayed empty and Gate 2 performed no target/Flash mutation.

## 5. Resource recovery targets

Initial minimum recovery objectives are:

- Flash <= `53248` bytes, providing at least 1 KiB headroom under the existing
  53-KiB acceptance ceiling and at least 2 KiB to the physical 54-KiB application end;
- static SRAM <= `11264` bytes, providing at least 1 KiB headroom under the existing
  12-KiB acceptance ceiling;
- task0/task1 hardware stack margins remain >= `256/256`;
- MSP measurable margin remains >= `1024`.

These are recovery objectives, not permission to force risky code changes. If Gate 1
proves they cannot be reached with bounded behavior-preserving changes, the exact
target must be explicitly reopened before mutation rather than met through unreviewed
ABI removal, buffer cuts or opaque tricks.

## 6. Preferred source changes

Priority order after measurement:

1. remove production-only waste with the strongest proof, starting with the duplicate
   default scheduler stack store if the acceptance/self-test profile can own it instead;
2. evaluate measured `-Os` wins per module;
3. classify target tests into host/build-time, acceptance-only and production-health
   diagnostics;
4. move heavy acceptance/reference self-tests out of production only through an
   explicit build-profile contract while preserving the published command/RPC ABI or
   explicitly versioning any deliberate ABI change in a separate boundary;
5. extract coherent root-private ownership from `kernel.c` only when the state and
   reason-to-change move together.

## 7. Forbidden shortcuts

This boundary must not:

- remove commands/RPC IDs merely to make size;
- silently change command-service, binary-frame, Asset or USB ABI;
- shrink USB rings without measured high-water/drop justification;
- create a universal `kernel_context_t`, service locator, generic HAL or catch-all
  dependency object;
- add heap, dynamic allocation, generic queue/mutex/timer/DMA frameworks;
- merge Bootloader/update behavior into the cleanup;
- modify linker origin/VTOR/reset ownership;
- consume persistence pages for scratch/update staging;
- weaken watchdog/fault/recovery behavior;
- test an acceptance image and ship different untested production bytes.

## 8. Production / acceptance profile rule

If a separate acceptance diagnostic profile is introduced:

- profile selection must be explicit and repository-owned;
- public product ABI differences must be explicit rather than accidental;
- heavy acceptance-only target code must not be required for ordinary production
  health;
- the final production image receives its own full resource validation and hardware
  smoke;
- exact production BIN/ELF/MAP identities are the release acceptance identities.

## 9. Kernel composition rule

A second decomposition is justified here only by the now-real trigger:

- current resource pressure;
- future Bootloader/update ownership would otherwise add more reasons to change
  `kernel.c`.

Natural extraction candidates remain low-level RCC/GPIO/UART/I2C ownership and
production task/liveness/watchdog glue where state can move with a coherent owner.

Line-count reduction alone is not acceptance.

Gate 4 accepted the exact Gate-3 production bytes through a bound composite hardware
chain. Final accepted runtime margins are task0/task1 `304/432` and MSP `1592`; primary
management IF2 pressure is `128/128`; scheduler/application/IWDG, canonical CLEAN
persistence, zero USB errors/PMA overruns/drops, physical OLED and physical reconnect
all pass. The final read-only closure evidence
`stm32_os_preboot_resource_gate4_readonly_closure_v1_20260927_190303.evidence.zip`
(SHA-256 `05A8F13552656D3C2AE5B05F8BC0FC927140BF0CBAC964B12721E46BABB2357B`)
proves post-IWDG `IWDG_RESET=1`, Home/health/ping over management IF2/libusb and exact
64-KiB Flash SHA-256 `17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`.

## 10. Exit condition

This boundary is complete only when:

- resource attribution is evidence-backed;
- exact accepted source/profile changes are behavior-preserving;
- Flash/SRAM recovery targets pass or are explicitly reopened with evidence;
- exact production bytes pass retained primary management IF2 / UART / RPC / Asset / UI /
  scheduler / IWDG / fault regressions; unchanged CDC remains secondary diagnostics and
  requires passive configured/drop retention rather than a separate active-tty campaign;
- physical OLED disposition is recorded if presentation bytes change;
- repository and docs are synchronized;
- one local acceptance commit and one ordinary non-force publication complete.

After publication, exact next boundary:

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`

Its Gate 0 must separately freeze 8-KiB bootloader feasibility, application relocation
and vector handoff, update image/authenticity/trust policy, rollback/recovery semantics,
metadata ownership and controlled reboot/update transition.
