# Deus OS — Kernel Composition-Root Decomposition Acceptance Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT NEXT**

Boundary ID:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

Canonical design:

`docs/KERNEL_COMPOSITION_ROOT_DECOMPOSITION_PLAN.md`

Published parent:

- commit `25752fba557b1a1b518265a93bde05d3a6a3f9ad`;
- tree `24624db70923bdaa77f134cc956423511785c199`;
- accepted firmware candidate tree `ba8b7066c8c435b7bca4fdef3932f27c5055761c`;
- BIN `48604` bytes / `2D6994532ABB82B7CA478E416ABC984F7E0DAFF98A07414FF974A3885DCC95D2`;
- Flash `48604 / 65536`, SRAM `10032 / 20480`;
- task0/task1 minimum accepted margins `272 / 424`;
- Application Runtime `PHYSICAL_OLED=PASS`.

## Gate 0 — architecture/source-boundary freeze

Gate 0 is docs/static review only.

Required freeze:

- behavior-preserving decomposition only;
- no product feature work;
- no public ABI/protocol/USB/scheduler/OLED change;
- no universal `kernel_context_t`/service locator/god object;
- no hidden extern access to private `kernel.c` state;
- no dependency cycles;
- explicit domain ownership and one-way dependency direction;
- initial authorized source set exactly as frozen by the design plan;
- Gate 2 resource ceilings stay Flash <= `48656`, SRAM <= `10104`;
- task stacks stay `1024 / 512`, runtime margins >= `256`.

Gate 0 acceptance records the measured parent:

- `src/kernel.c = 5100` lines;
- `console_execute_request = 8644` linked bytes;
- `console_execute_scheduler_diagnostic = 3284` linked bytes;
- `boot_desktop_ui_render = 1420` linked bytes;
- `kernel_main = 1116` linked bytes.

## Gate 1 — source decomposition/static proof

Required:

- changed paths remain inside frozen source boundary or are explicitly expanded before mutation;
- extracted modules have coherent ownership;
- `src/kernel.c` materially decreases;
- no public command/RPC renumbering;
- command registry remains `35`;
- application runtime ABI/lifecycle/event constants unchanged;
- no transport/USB descriptor change;
- no task/scheduler/IWDG policy change;
- no heap, new task, SVC, queue, mutex, generic timer, DMA or persistence;
- `git diff --check` PASS;
- no commit/flash.

Acceptance: **PASS**. Exact changed source set is seven paths; `src/kernel.c` is `3405` lines versus parent `5100` (-33.235%). Extracted ownership is application runtime bridge, application commands and scheduler diagnostics; root-private production scheduler observability remains in the composition root because moving it would require hidden state exposure or an oversized snapshot. No god object/service locator, hidden extracted-state extern, heap or new runtime mechanism was introduced.

## Gate 2 — fresh build/link/resource/structure validation

Fresh Arm GNU build with `-Wall -Wextra -Werror -fstack-usage`.

Require:

- warning-free compile/link;
- undefined symbols `0`;
- vectors/startup/linker/USB identity unchanged;
- Flash <= `48656`;
- SRAM <= `10104`;
- task stacks exactly `1024 / 512`;
- exact candidate tree/BIN/ELF/MAP identities;
- exact old/new `kernel.c` line count;
- linked size comparison for former monolithic command/diagnostic/render/main sections;
- dependency/source-boundary proof;
- real Git index untouched.

Acceptance: **PASS**. Candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`; BIN `48636` bytes / SHA-256 `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`; ELF SHA-256 `83FD4C9B9589A7E1B619A3B0C82BF2AB9050D5B4572DAD30BA1414CA8354CF8B`; MAP SHA-256 `0BB014FAA418374AFDC77EE9389B3D7BCE631FB0D33EA451952142F78DCC2AD8`; Flash `48636/65536` <= `48656`; SRAM `10032/20480` <= `10104`.

Linked sizes after decomposition: `console_execute_request=6780`, `boot_desktop_ui_render=804`, `kernel_main=1112`, `scheduler_diagnostics_execute_diagnostic=3312`, `application_commands_execute=776`, `application_runtime_bridge_service=316`, `application_runtime_bridge_apply_view=216`. Gate 2 evidence SHA-256 `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; authoritative Gate 2 build log SHA-256 `88A0AEA8569D404FC8F498124F042F2142EA43440CF9D9CF4096A3C3D12585E0`; evidence-repair log SHA-256 `904C5C476D3A4ED29CAB80532B30FE5B1DD757DF770CC8B1ACEA6AEC1C18A25F`.

## Gate 3 — hardware/runtime equivalence

Flash only exact Gate 2 candidate.

Require retained Application Runtime acceptance:

- boot/scheduler/IWDG startup;
- default home and two-app registry;
- text/binary app lifecycle and idempotence;
- invalid-start no mutation;
- minute semantic event without view rerender;
- active-app `uiruntime` restore;
- CDC/UART/binary pressure `128/128`;
- malformed/CRC/oversize/split recovery;
- physical micro-USB power-cycle recovery;
- IWDG recovery;
- production/app fault counters zero;
- stack canaries intact;
- task0/task1 margins >= `256`;
- final exact Flash readback;
- exact repository source state.

Acceptance: **PASS**. Boot/scheduler/IWDG/USB startup passed; text and binary application lifecycle/idempotence/invalid-start no-mutation passed; minute event advanced without application rerender; text/binary scheduler diagnostic BUSY was `8/8`; CDC/UART/binary pressure was `128/128`; malformed/CRC/oversize/split recovery, physical reconnect and authorized IWDG recovery passed; final production/application faults were zero; final task0/task1 margins were `384/424`; final Flash readback and repository state matched the exact Gate 2 candidate. Gate 3 evidence SHA-256 `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; log SHA-256 `4F3714F2D8772F032D3D15CFA5C3A61E69B53D69C60E8D0E77101297153D5856`.

## Gate 4 — physical OLED equivalence

If UI/application integration code moved, require explicit `PHYSICAL_OLED=PASS` confirming:

- splash/home unchanged;
- device.info unchanged;
- status bar unchanged;
- app switch no blank/off pulse;
- no stale glyphs/clipping;
- minute/status updates do not corrupt content;
- `uiruntime` restores active app;
- appstop restores home.

Acceptance: **PASS**. Operator reported `PHYSICAL_OLED=PASS` after observing unchanged splash/home, correct `device.info`, correct status bar, clean app switching without blank/off pulse, no stale glyphs/clipping, clean minute/status updates, `uiruntime` active-app restore and `appstop` home restore.

## Gate 5 — docs/evidence finalization

Synchronize canonical docs and record:

- exact module ownership after decomposition;
- before/after `kernel.c` line count;
- before/after linked function/object sizes;
- exact resource changes;
- hardware and physical equivalence;
- any justified source-boundary changes.

Acceptance: **PASS**. Canonical/current-state docs are synchronized to the exact accepted candidate, module ownership, before/after source/linked measurements, resources, Gate 2/3 evidence identities and Gate 4 physical result. No source-boundary expansion beyond the seven accepted paths was required. Gate 6 local acceptance commit is next.

Accepted evidence: Gate 2 `E46AD8B481D612D29F9E514106D11A9FAF861FA0CAF22AF6764D2711B6485549`; authoritative Gate 2 build log `88A0AEA8569D404FC8F498124F042F2142EA43440CF9D9CF4096A3C3D12585E0`; Gate 2 evidence-repair log `904C5C476D3A4ED29CAB80532B30FE5B1DD757DF770CC8B1ACEA6AEC1C18A25F`; Gate 3 `2FE593A80FB42BF3808AFAE397A3205FD64824873FF867ED3277D91010AFAC42`; Gate 3 log `4F3714F2D8772F032D3D15CFA5C3A61E69B53D69C60E8D0E77101297153D5856`; Gate 4 `PHYSICAL_OLED=PASS`.

## Gate 6 — local acceptance commit

Require exact reviewed source/docs set, complete evidence, `git diff --cached --check` PASS, clean repo after commit and direct remote parent.

## Gate 7 — ordinary non-force publication

One ordinary `git push origin main:main`, then fresh fetch requiring:

- `HEAD == origin/main == FETCH_HEAD`;
- clean repo;
- ahead/behind `0/0`.

After Gate 7, exact next boundary:

`USB_MANAGEMENT_DEVICE_FOUNDATION`

## Acceptance blockers

Fail the boundary for:

- semantic/visible feature change hidden as refactor;
- public RPC/protocol/USB/scheduler behavior drift;
- catch-all god object replacing the god-module;
- hidden cross-module extern state;
- circular dependencies;
- new heap/task/SVC/queue/mutex/timer/DMA/persistence machinery;
- task margin < `256`;
- Flash/SRAM above frozen ceilings without explicit pre-approved architectural justification;
- loss of retained hardware/physical behavior.
