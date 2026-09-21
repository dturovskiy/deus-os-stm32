# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

It intentionally does not duplicate live Git `HEAD`/tree identity. Git itself is authoritative for repository bytes, commit identity, branch state and cleanliness. Documentation-only commits may advance `HEAD` without changing the accepted product boundary.

For architecture, roadmap sequence, boundary contracts, acceptance criteria, evidence policy and historical records, follow the precedence in `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`HOST_CONTROL_APPLICATION_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED**

Boundary acceptance/publication commit:

`e0f49f168542fa1cf49bca451e01b0c077aa8d18`

Boundary commit tree:

`42c77f2cf3d7e9f7f5c1ff24d9d437f61a397be6`

Accepted firmware candidate:

- source candidate tree: `b895955f7738aceb6fca0272d510cc433378c6ab`
- BIN: `50652` bytes
- BIN SHA-256: `FB68993FC998DE77B61FAF9F4949E4E124B95FF867BB456EBA401C9F2709F13F`
- ELF: `483488` bytes
- ELF SHA-256: `978CC710497F79E9A3AFBAFE3E0CD3AE10BA2B6CF0FE6AA7518B7AC5BC5E524C`
- MAP: `207952` bytes
- MAP SHA-256: `F9806E2A71476D6D2DAC2CD481A6C491BACD18CADD713813BB69AF9DA608C032`
- Flash/SRAM: `50652/11728`
- task stacks: `1024/512`
- accepted task0/task1 margins: `424/424`

Accepted host candidate:

- host source tree: `2c5afd9914851300aed15e321cf69c3a2c3daeed`
- target: `net10.0`
- Avalonia: `12.1.2`
- Windows management: WinUSB
- Linux management: libusb-1.0
- Core tests: `21/21`
- Transport tests: `5/5`

Accepted management/runtime contract:

- USB identity: private-development `1209:000C` / `Deus OS Device`
- CDC IF0/1 retained for secondary diagnostics
- management IF2 over EP4 OUT/IN `0x04/0x84`, bulk64
- Windows management GUID: `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`
- binary framing protocol: v1
- command service: v3 / registry 36
- `sysinfo=0x0024`
- system capability mask: `0x0000001F`
- initial applications: `system.home=0x0001`, `device.info=0x0002`

Host foundation acceptance covers Windows CLI/hardware, Windows Desktop, real Linux libusb runtime, application lifecycle, malformed/recovery behavior, 128 unique management pings, fresh reconnect negotiation, IF2-only Linux claiming with CDC retained, zero management/CDC drops and exact final Flash readback.

Canonical design:

`docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`

Canonical acceptance:

`docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`

## 2. Current boundary disposition

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATE 0 REVIEW COMPLETE / DEFERRED_NO_REAL_CONSUMER**

Gate 0 completed as a fail-closed architecture review. It found no current target consumer with a frozen schema that requires persistent asset/configuration storage now.

Therefore:

- Gate 1 implementation is **not authorized**;
- no persistent Flash region is owned by this boundary;
- no Flash erase/program implementation is active;
- no transfer ABI extension is active;
- capability bit 5 remains reserved but unadvertised;
- generic storage/filesystem/package infrastructure remains unauthorized.

Canonical Gate 0 design/disposition:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

Canonical Gate 0 acceptance:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

The deferred OLED configurable-layout and package ideas remain possible future consumers, but they do not become real consumers until a dedicated boundary promotes and freezes one of them.

No product feature implementation boundary is currently active.

Near-term repository readiness work that may proceed without activating speculative product functionality:

1. establish one versioned repository-owned firmware build entrypoint; the exact historical optimization/link/section invocation is not preserved in Git, so it must be recovered from acceptance harness evidence or replaced by an explicitly re-accepted reproducibility baseline rather than guessed;
2. run and review the versioned read-only real-device preflight at `scripts/stm32_readonly_flash_preflight.ps1`; the script exists, but real-board evidence is still pending;
3. resolve host package-lock/SDK restore ambiguity before future host transfer implementation; no `packages.lock.json` files are currently present.

These are infrastructure prerequisites, not permission to implement persistent Flash mutation.

## 3. Planned product sequencing

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` may be reopened only when a concrete consumer exists and Gate 0 can freeze its exact schema, Flash partition, transfer ABI, transaction/recovery model and resource budget.

Forward product order remains:

1. accepted/reactivated Asset/Configuration work when a real consumer exists;
2. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`;
3. networking/service extensions.

Deferring Asset/Configuration at Gate 0 does not automatically promote firmware update ahead of its own prerequisites or security/recovery contract.

Firmware update requires separate authenticity/security, controlled reboot/update handoff and recoverable Flash transaction design. Networking remains a later service boundary and does not authorize speculative TCP/IP expansion on the STM32F103.

Canonical sequence:

`docs/ROADMAP.md`

## 4. Completed/published foundation sequence

The following major foundations are complete/published and are **not active work**:

- scheduler timed blocking;
- scheduler fixed priority;
- production heartbeat task;
- IWDG production liveness;
- normal-boot production task ownership;
- native USB device core;
- USB CDC ACM diagnostics/console;
- transport-neutral shell/RPC;
- binary framed transport v1;
- OS application/UI model foundation;
- boot/desktop UI foundation;
- OLED dirty-region optimization;
- application runtime foundation;
- kernel composition-root decomposition;
- USB management device foundation;
- Host control application foundation.

Their `*_PLAN.md` and `*_ACCEPTANCE_PLAN.md` files are retained as immutable scoped design/proof records, not as competing global current-state documents.

## 5. Deferred / trigger-driven work

The following items exist as possible future work but are **not active roadmap boundaries unless explicitly promoted**:

### Performance and kernel/runtime

- I2C async/IRQ/DMA after measured bus pressure;
- scheduler ready-set acceleration after measured task-count/CPU pressure;
- CRC acceleration after measured streaming cost;
- bounded minimal-copy/backpressure refinements for transfer/update streaming;
- tickless/low-power after a real power requirement;
- generic timers/queues/synchronization/runtime statistics only for real consumers;
- further composition-root convergence when ownership pressure warrants it.

### Robustness and observability

- previous-boot crash/reset retention;
- bounded structured binary event tracing;
- soak/fault-injection campaigns;
- diagnostic/test build profiles;
- explicit application stop-failure semantics before resource-owning apps require them.

### UI/configuration ideas

- alternate OLED layout presets;
- runtime custom layout editing;
- PC layout configurator/import;
- persisted UI layout only through an accepted persistence contract;
- RTC wall-clock;
- notification rendering/queue semantics;
- optional kernel-log presentation.

`docs/OLED_UI_LAYOUT_PLAN.md` and `docs/OLED_STATUS_BAR_PLAN.md` are deferred future-consumer designs, not active implementation plans.

### Storage/platform expansion

- general filesystem;
- external storage/block-device layers without a concrete consumer;
- MPU/user-kernel isolation;
- broad power-management framework;
- stable physical unit identity/MCU UID until a real multi-unit/privacy requirement exists.

Canonical deferred policy:

`docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

## 6. Known architecture state

The initial Asset/Configuration Gate 0 review completed without finding a substrate blocker; it was deferred solely because no current consumer with a frozen persistence schema exists.

Resolved gaps:

- semantic application events are separate from scheduler wake bits;
- system identity/capability discovery is provided through `sysinfo`;
- USB management and host transport are separated from CDC diagnostics;
- Windows/Linux host control share one protocol/Core model;
- kernel composition-root risk was materially reduced without introducing a god context/service locator.

Still intentionally unresolved until a consumer requires them:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- firmware-update authenticity;
- network mutation security;
- richer observability/runtime-statistics framework.

These remain consumer- or boundary-specific future concerns. None changes the current `DEFERRED_NO_REAL_CONSUMER` disposition.

## 7. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- forward ordering: `docs/ROADMAP.md`
- active/completed boundary design contract: matching `*_PLAN.md`
- boundary proof/acceptance contract: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- historical change chronology: `CHANGELOG.md`
- execution history/checklists: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator/handoff reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella implementation notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
