# Deus OS — Application Runtime Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `25752fba557b1a1b518265a93bde05d3a6a3f9ad`**

Boundary ID:

`APPLICATION_RUNTIME_FOUNDATION`

Canonical design:
`docs/APPLICATION_RUNTIME_FOUNDATION_PLAN.md`

Published parent:

- commit `39690c9ef103cbcf93272df8bad0359a934b7dc1`;
- tree `f195fac5ce733c36a1d955e0fbe687ee6c83b605`;
- subject `feat: optimize OLED dirty region updates`;
- clean `main`, `HEAD == origin/main`, ahead/behind `0/0`;
- accepted firmware candidate tree `75f05f689970b760604112b30346b0c328bfaff2`;
- BIN `44560` bytes / `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`;
- Flash `44560 / 65536`, SRAM `9848 / 20480`;
- physical OLED `PHYSICAL_OLED=PASS`.

## Gate 0 — application-runtime architecture/source-boundary freeze

Gate 0 is documentation/static review only. No source/build/flash mutation is authorized.

Required architecture freeze:

- v1 applications are statically linked firmware modules;
- exactly two initial system applications: `system.home=0x0001`, `device.info=0x0002`;
- one active foreground application at a time;
- lifecycle values exactly REGISTERED=0, STOPPED=1, STARTING=2, RUNNING=3, BLOCKED=4, STOPPING=5, FAILED=6;
- `BLOCKED` is a reserved valid application ABI state in v1, but initial apps do not enter it and it must not alias scheduler `SCHEDULER_TASK_BLOCKED`;
- callbacks execute only in task0 / Thread-PSP;
- no task-per-app, heap, dynamic loader, queue, mutex, generic timer service or new SVC;
- semantic application event IDs are separate from scheduler event bits;
- event object is fixed-size and pointer-free;
- service state is delivered through a bounded snapshot contract;
- applications render a bounded three-row view model, never SSD1306/framebuffer directly;
- status bar remains system-owned;
- `system.home` keeps the accepted `DEUS OS / DESKTOP / READY` appearance;
- `device.info` renders `DEUS OS / DEVICE INFO / STM32F103`;
- existing RPC IDs `0x0001..0x0020` remain unchanged;
- append only `applist=0x0021`, `appstart=0x0022`, `appstop=0x0023`;
- command-service foundation version advances to `2`; binary frame protocol stays version `1`;
- initial authorized source boundary is exactly the five paths in the canonical design, including two new application-runtime files;
- Flash increment budget <=4096 bytes and static SRAM increment budget <=256 bytes over the published parent;
- task stack capacities stay `1024 / 512` and accepted runtime margins remain >=256 bytes.

Acceptance: **PASS**. The published architecture foundation already requires a static application runtime, semantic event contract distinct from scheduler wake bits and preservation of the two-task topology. Current source has a static command registry and task0 serialized command/UI ownership, so the planned implementation requires no scheduler/USB/OLED-driver rewrite or speculative kernel primitive.

## Gate 1 — source implementation and deterministic/static proof

Authorized initial paths only:

```text
include/kernel/application_runtime.h      NEW
src/kernel/application_runtime.c          NEW
include/kernel/command_service.h
src/kernel/command_service.c
src/kernel.c
```

Existing-file prestate SHA-256 guards:

```text
include/kernel/command_service.h  E11F19270B003A3065A1F7B4B27831E50AC54C7C1B20A540F1C0D51D1B078A3D
src/kernel/command_service.c      F064E3D9A0B3383917E2B30D1BB6227A8BA870A16D49210A735EBF8796B26430
src/kernel.c                      21120F6A6C824B8880C4FE096F2E8B3B9DB9F66879082774A85C13B20A1C8A69
```

Gate 1 must prove:

- application descriptor table count exactly `2`;
- stable IDs `0x0001` / `0x0002` are explicit values and not table ordinals;
- registry lookup rejects unknown/zero IDs;
- lifecycle enum/transition helpers preserve the exact seven-state published vocabulary and are bounded and deterministic;
- home is mandatory fallback after target-start failure;
- stopping home is idempotent and cannot leave runtime without a foreground app;
- event type IDs are exactly `0x0001..0x0005` as frozen by design;
- source IDs are exactly `0x0001..0x0005` as frozen by design;
- event object contains no pointer payload and has fixed bounded size;
- no queued delivery state is added;
- callback execution is task0-only integration, never IRQ-side;
- service snapshot contains only monotonic time/system/USB/network state;
- application view has exactly three rows x 21 visible characters;
- application code contains no SSD1306/register/framebuffer ownership;
- app view adaptation uses the existing OLED console/render path;
- existing command IDs `0x0001..0x0020` remain unchanged;
- exactly three appended RPC IDs `0x0021..0x0023`;
- command registry count exactly `35` and `COMMAND_SERVICE_FOUNDATION_VERSION == 2`;
- binary protocol version and capability flags remain semantically unchanged;
- `rpcinfo` changes are additive only;
- no heap, new task, SVC, queue, mutex, filesystem, DMA framework or generic timer subsystem.

No flash in Gate 1.

## Gate 2 — fresh build/link/resource/static validation

Fresh exact-source build with accepted Arm GNU toolchain and:

```text
-Wall -Wextra -Werror -fstack-usage
```

Required:

- expected topology `17 C + 1 ASM` unless Gate 1 explicitly justifies a different exact count;
- all translation units compile warning-free;
- undefined symbols `0`;
- vector/startup/linker/USB descriptor/PMA identity unchanged;
- prior RPC IDs `0x0001..0x0020` unchanged;
- appended IDs exactly `0x0021..0x0023`;
- command registry count `35`;
- command-service version `2`, binary frame protocol version `1`;
- Flash <= `48656 / 65536`;
- static SRAM <= `10104 / 20480`;
- task stack capacities exactly `1024 / 512`;
- GCC stack-usage evidence for new runtime functions and modified task0 call paths;
- no unbounded local allocation;
- exact source hashes, candidate tree, BIN/ELF/MAP sizes/hashes recorded;
- `git diff --check` PASS;
- real index remains untouched by candidate-tree proof.

Gate 2 candidate is immutable for Gate 3 except after classified PRODUCT correction and a fresh Gate 2 rerun.

## Gate 3 — hardware/runtime acceptance

Flash only exact Gate 2 candidate. Read current target Flash first and skip programming if already exact.

Required boot/runtime proof:

- boot/scheduler/IWDG startup retained;
- default application runtime starts `system.home` as active ID `0x0001`;
- production faults remain zero;
- initial task0/task1 margins >=256 bytes;
- `applist` shows exactly two registered apps, home RUNNING and device.info STOPPED;
- text `appstart 0x0002` succeeds and active app becomes `0x0002`;
- repeated `appstart 0x0002` is idempotent;
- text `appstop` returns active app to `0x0001`;
- invalid `appstart` leaves active/lifecycle/view state unchanged;
- binary RPC `applist/appstart/appstop` matches text semantics and stable IDs;
- `rpcinfo` reports command-service version `2`, registry `35` and runtime fields;
- dynamic semantic-event delivery is proven by the real minute transition from task0 with stable application view revision; physical micro-USB disconnect/reconnect is accepted as power-cycle recovery plus USB configured-state/default-home proof because micro-USB is the sole target power source and volatile previous USB state does not survive reset;
- app event counters/state remain bounded and no app rerender occurs for an event whose callback returns no view-dirty effect;
- `uiruntime` restores the currently active app;
- task0/task1 margins remain >=256 after application lifecycle/event/render activity;
- stack canaries intact;
- CDC pressure `128/128`, zero drops;
- UART pressure `128/128`, zero drops/errors;
- binary pressure/correlation and malformed/CRC/oversize/split recovery PASS;
- physical USB disconnect/reconnect PASS;
- authorized IWDG reboot and automatic UART/CDC/binary recovery PASS;
- application runtime returns to deterministic default home after reboot;
- final exact Flash readback PASS;
- repository source candidate remains exact and index unchanged.

## Gate 4 — mandatory physical OLED application-view regression

Require explicit `PHYSICAL_OLED=PASS` after observing:

- splash -> home remains visually identical to the accepted baseline;
- `appstart 0x0002` shows exactly `DEUS OS / DEVICE INFO / STM32F103` in the content area;
- status bar remains geometrically and semantically correct;
- no full-screen blank/off pulse on app switch;
- no stale glyphs or dirty-span clipping;
- minute and USB status updates do not corrupt active application content;
- `appstop` restores exactly `DEUS OS / DESKTOP / READY`;
- `uiruntime` restores whichever app is active.

## Gate 5 — docs/evidence finalization

Synchronize at minimum:

- `README.md`;
- `CHANGELOG.md`;
- `docs/ARCHITECTURE.md`;
- `docs/IMPLEMENTATION_PLAN.md`;
- `docs/MASTER_EXECUTION_CHECKLIST.md`;
- `docs/PROJECT_HANDOFF.md`;
- `docs/ROADMAP.md`;
- canonical application/UI architecture docs where required;
- this plan and acceptance plan.

Record exact app IDs, lifecycle/event ABI, command/RPC additions, resource usage, stack evidence, hardware results and physical OLED disposition. Accepted evidence: Gate 2 `75719A35004401F9A941E187EC93978961252388B03F892DE98CBA40479492A6`; Gate 3 `1158485D1A0C90FA4931589F10298154E6522A568220A48C4A6BE67EFA54FC52`; Gate 3 log `703C41A7493D078E456A5721EAFEFF821A152D217FB3F30AB8C809D7452A6B36`; Gate 4 `ED0F022A60174AEEA487B64216885AFA72D768CA81CF60E14A347DEAC58B2F86`; physical OLED `PASS`.

Do not silently fold USB management-device, host GUI, persistence, update or networking work into Gate 5.

## Gate 6 — local acceptance commit

Acceptance: **PASS**. Exact reviewed source/docs set committed as `25752fba557b1a1b518265a93bde05d3a6a3f9ad`, tree `24624db70923bdaa77f134cc956423511785c199`; `git diff --cached --check` PASS, repo clean after commit, remote still direct parent, ahead/behind `1/0`. Gate 5 evidence `817D12D74D88F1C0F31C502B0715F038FF356382E56FC0D5421DC237239D78BC`; Gate 6 evidence `203F9A99E09F76C7F99B6C06EF073BE4F55949545188F7302E394AB20AEED068`.

## Gate 7 — ordinary non-force publication

Acceptance: **PASS**. One ordinary non-force `git push origin main:main` published `25752fba557b1a1b518265a93bde05d3a6a3f9ad`; fresh fetch verified `HEAD == origin/main == FETCH_HEAD`, clean repo, ahead/behind `0/0`. Gate 7 evidence `855FC891003E1A67EBF6C5FDE559EEF0F1450A83828FCF66EFC6D00DB3C51EBB`.

After Gate 7, exact next boundary:

`KERNEL_COMPOSITION_ROOT_DECOMPOSITION`

## Acceptance blockers

The boundary fails if it introduces or requires any of the following:

- application identity derived from registry ordinal;
- scheduler event-bit values exposed as stable application event IDs;
- callback execution in IRQ/Handler mode;
- one task per app or dynamic task creation;
- heap allocation or arbitrary native code loading;
- queued events without explicit queue/backpressure/drop semantics;
- direct app ownership of STM32 registers, SSD1306 or framebuffer;
- app failure that leaves no mandatory home fallback;
- renumbering any existing RPC ID `0x0001..0x0020`;
- binary protocol version change without an independent protocol reason;
- USB descriptor/PMA redesign;
- new filesystem/persistence/update/network subsystem;
- task stack margin below `256` bytes;
- Flash/SRAM above the frozen Gate 2 ceilings;
- visual corruption or status-bar regression during application switching.
