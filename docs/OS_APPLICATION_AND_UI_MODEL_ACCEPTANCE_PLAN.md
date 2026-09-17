# Deus OS — Product, Application and UI Model Foundation Acceptance Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT CURRENT**

Boundary ID:

`OS_APPLICATION_AND_UI_MODEL_FOUNDATION`

Canonical design:
`docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`

## Gate 0 — product/application/UI architecture freeze

Required prestate:

- `main`, `origin/main` and remote `main` synchronized at `2fde9025a51021511e73a76b561f7983ca655e2f`;
- published tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`;
- subject `feat: add binary framed transport foundation`;
- clean worktree/index;
- accepted binary framed transport candidate BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`.

Gate 0 is documentation-only. It must define and synchronize:

- Deus OS product role;
- embedded-runtime versus host-management responsibilities;
- application model v1 and lifecycle vocabulary;
- initial two-task runtime ownership constraint;
- system-application concept;
- boot splash / desktop / application-view lifecycle;
- concrete status-bar semantics;
- initial time source policy;
- host plugin versus target app versus target package terminology;
- explicit deferral of arbitrary uploaded native ARM code;
- firmware-update and bootloader ordering;
- application semantic-event contract distinct from scheduler wake bits;
- system identity/capability-discovery requirement for host tooling;
- bounded persistence-safety requirements before Flash-resident settings/packages;
- retained crash/reset and structured-observability backlog;
- security/trust requirement before network mutation or executable firmware update;
- portability layering rule separating arch/platform/drivers from kernel/services/apps/UI/protocol semantics;
- explicit consumer-driven deferral of generic timers, queues, synchronization, runtime statistics, heap/filesystem/MPU/power/RTC/DMA frameworks;
- exact roadmap order after this foundation.

No source edit, build, flash, commit or push in Gate 0.

Acceptance: **PASS**. Product/application/UI architecture and the foundation-gap contracts are synchronized in the canonical plan/review without authorizing firmware mutation.

## Gate 1 — repository/source capability inventory

Read-only proof only. Confirm that the published source actually provides the substrate assumed by the architecture plan:

- two production PSP tasks with task0 console/runtime and task1 heartbeat;
- scheduler timed wait/sleep/event primitives;
- scheduler-internal PRIMASK save/restore critical sections exist, but no public mutex/semaphore/synchronization layer is implied;
- Phase 4 already lists generic timers, message queues, synchronization primitives and runtime statistics as deferred consumer-driven work;
- 128x32 OLED framebuffer and retained 21x3 console;
- accepted 128x9 status bar;
- current boot screen `DEUS OS / BOOT OK / READY`;
- current status time `00:00` hardcoded at runtime render;
- two filled + one ring 5x5 status indicators;
- USB CDC configured-state diagnostics available;
- transport-neutral 32-method command service;
- binary RPC v1 HELLO exposes protocol/service bounds and protocol capability flags but not a complete firmware/build/platform/device identity model;
- current `fault_record` is normal `.bss` runtime state and is cleared on reset;
- reset-cause capture already exists and proves IWDG reset attribution;
- persistent-storage policy is planned but no accepted Flash settings/filesystem contract exists yet;
- no current file transfer, Flash update, bootloader, authentication model or dynamic application loader.

No mutation/build/flash/commit/push.

Acceptance: **PASS**. Read-only source review confirms the two-task production topology, timed/event scheduler substrate, internal PRIMASK critical sections, frozen OLED/runtime state, CDC diagnostics, 32-method command service and binary RPC HELLO capabilities. Production source exposes only Flash latency/prefetch setup and contains no accepted Flash-write/file-transfer/filesystem/heap/dynamic-loader implementation.

## Gate 2 — architecture consistency review

Documentation/static review must prove:

- no conflict with the accepted scheduler/task ownership model;
- no hidden requirement for one task per application;
- no hidden heap/filesystem dependency;
- no claim that binary RPC v1 already supports file transfer or firmware update;
- no claim that a PC is required for normal runtime liveness;
- no status indicator claims connectivity that current hardware/software does not provide;
- future dynamic native-code loading remains explicitly deferred;
- `BOOT_DESKTOP_UI_FOUNDATION` is smaller than and precedes application runtime and Control Panel work;
- `APPLICATION_RUNTIME_FOUNDATION` precedes host application management features and owns the semantic application event/service contract;
- scheduler event bits are not promoted to stable public application event IDs;
- host tooling cannot treat USB VID/PID or binary protocol capability bits as the complete Deus OS system identity model;
- persistent Flash settings/packages cannot be accepted without version/integrity/atomic-commit/recovery/wear policy;
- retained crash/reset diagnostics are documented as later observability work and current BSS fault data is not falsely described as reset-persistent;
- CRC and destructive-intent flags are not described as authentication;
- network-accessible mutation and firmware update require a separate security/trust review;
- future portability preserves arch/platform/driver separation without forcing a speculative universal HAL;
- firmware update/bootloader remains after stable host/runtime APIs and transfer semantics.

No build/flash.

Acceptance: **PASS**. Static documentation/source consistency review found no conflict with current task ownership, no hidden heap/filesystem/one-task-per-app dependency, no false file-transfer/update claim, and no security/identity/persistence/portability ambiguity after the foundation-gap addendum.

## Gate 3 — hardware acceptance

`N/A_DOCS_ONLY` — **PASS / NOT APPLICABLE**

This boundary changes no firmware bytes and therefore requires no target flash or physical hardware acceptance.

## Gate 4 — OLED conditional review

`PHYSICAL_OLED=N/A_DOCS_ONLY_NO_FIRMWARE_CHANGE` — **PASS / NOT APPLICABLE**

The accepted OLED/gfx/status geometry remains untouched.

## Gate 5 — documentation finalization

Require current-state synchronization across at minimum:

- `README.md`;
- `CHANGELOG.md`;
- `docs/ARCHITECTURE.md`;
- `docs/IMPLEMENTATION_PLAN.md`;
- `docs/MASTER_EXECUTION_CHECKLIST.md`;
- `docs/PROJECT_HANDOFF.md`;
- `docs/ROADMAP.md`;
- binary transport plan/protocol/acceptance documents, marking Gate 7 publication complete;
- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`;
- this plan and acceptance plan.

Required roadmap after publication:

```text
OS_APPLICATION_AND_UI_MODEL_FOUNDATION   <- docs architecture freeze
 -> BOOT_DESKTOP_UI_FOUNDATION
 -> APPLICATION_RUNTIME_FOUNDATION
 -> HOST_CONTROL_APPLICATION_FOUNDATION
 -> ASSET_CONFIGURATION_TRANSFER_FOUNDATION
 -> FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION
 -> networking/service extensions
```

No source/build/flash in Gate 5.

Acceptance: **PASS**. Required current-state documents are synchronized, the binary transport publication is recorded, the new foundation-gap review is canonical, `git diff --check` passes, and repository review reports a docs-only change set with no source/header/linker/script/build paths.

## Gate 6 — local acceptance commit

Stage only the accepted documentation path set. Require:

- exact published parent `2fde9025a51021511e73a76b561f7983ca655e2f`;
- no source/header/linker/script/build artifact changes;
- `git diff --cached --check` PASS;
- one local documentation acceptance commit;
- clean worktree/index after commit;
- remote remains at parent;
- ahead/behind `1/0`;
- no push.

## Gate 7 — ordinary non-force publication

Before publication require:

- exact Gate 6 local acceptance commit;
- exact direct parent `2fde9025a51021511e73a76b561f7983ca655e2f`;
- clean repository;
- fresh remote `main` still at the published parent;
- ahead/behind `1/0`;
- fast-forward ancestry.

Publication must be one ordinary non-force:

```text
git push origin main:main
```

After publication require:

- local `HEAD == origin/main == fresh remote main`;
- clean repository;
- ahead/behind `0/0`.

After Gate 7, exact next firmware boundary:

`BOOT_DESKTOP_UI_FOUNDATION`

## Acceptance blockers

This architecture boundary fails if documentation introduces any of the following as an immediate requirement:

- arbitrary uploaded native ARM executables;
- dynamic relocation loader;
- general heap;
- filesystem;
- one scheduler task per app;
- host-PC dependency for watchdog/runtime progress;
- host-only desktop ownership;
- firmware update through the current RPC protocol without a separately accepted update transaction and recovery design;
- simultaneous implementation of desktop, app loader, Control Panel and bootloader in one source boundary.
