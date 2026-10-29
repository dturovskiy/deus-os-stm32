# Deus OS / STM32 OS

A small bare-metal operating-system project for STM32F103 / ARM Cortex-M3.

**Language boundary:** Target firmware is C and ARM Assembly (`src/`, `bootloader/`); the Host control application is C# (`host/`). Python is an optional host-side test/CI and one-off operator language, **not part of the firmware, scheduler, bootloader or target image**. One-time local audits and workspace maintenance are retained outside the Git checkout in the parent STM32 workspace. CI-bound regression fixtures remain versioned until safe replacement; see `scripts/README.md`.


The target firmware is written without STM32 HAL, Arduino, FreeRTOS, a heap, or a dynamic process loader. The project uses direct-register drivers, a bounded static scheduler/runtime, native USB, UART diagnostics, an SSD1306 local UI, and a C#/.NET host control application.

## Start here

For the **current project state**, use exactly one source:

- `docs/CURRENT_STATE.md`

For documentation roles and precedence:

- `docs/DOCUMENTATION_MODEL.md`

Public operational documents now use placeholders for workstation paths, SSH credentials and aliases; the accepted docs-only evidence and its limitations are recorded in `docs/PUBLIC_REPOSITORY_SANITIZATION_DOCS_ONLY_ACCEPTANCE.md`. Do not treat those placeholders as physical-hardware commands or assume the entire Git history has been sanitized.

For stable architecture/invariants:

- `docs/ARCHITECTURE.md`

For the current development/acceptance host and cable topology:

- `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`

For forward sequencing:

- `docs/ROADMAP.md`

For historical changes:

- `CHANGELOG.md`

README is an entry point only. It is **not** an authoritative current-state, acceptance, roadmap or evidence document.

## Target

- MCU: STM32F103 medium-density
- CPU: ARM Cortex-M3 / ARMv7-M
- Flash: 64 KiB
- SRAM: 20 KiB
- debug/recovery: SWD / ST-LINK V2
- emergency diagnostics: USART1
- primary product management on the installed signed-v6 runtime: `1209:000E` Management IF0 via libusb on the Ubuntu USB host; the earlier composite `1209:000C` used Management IF2 (Windows WinUSB / Linux libusb). Windows support for the new device is not inferred from Linux acceptance.
- **canonical/default** bench native-USB owner: Ubuntu Mac mini via libusb; Windows owns ST-LINK and UART. The 2026-10-29 owner-native signed-v6 update confirmed the Ubuntu host's real `1209:000E` USB enumeration and Management info/ping/health. Future hardware actions still rediscover the actual device/owner (see `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`).
- historical composite `000C` USB diagnostics: CDC ACM; the installed Management-only `000E` runtime has no CDC interface. USART1/UART remains a separate diagnostics channel.
- local display: SSD1306-class 128x32 OLED

## Implemented foundation areas

- startup/vector table/reset and fault handling
- clock + monotonic kernel time
- static PSP scheduler with blocking/event wake and IWDG liveness
- USART1 IRQ/ring diagnostic console
- native USB Device + CDC ACM
- transport-neutral command service
- binary framed RPC v1
- boot splash / desktop UI
- OLED dirty-region optimization
- static application runtime
- kernel composition-root decomposition
- USB management device
- Windows/Linux host control application
- bounded Asset/Configuration transfer + persistent A/B configuration
- recoverable authenticated firmware update + reset-owning USB bootloader

Exact accepted product state, candidate hashes and the current boundary disposition live in `docs/CURRENT_STATE.md`.

## Project structure

```text
OS/
├── bootloader/
├── docs/
├── host/
├── include/
├── linker/
├── scripts/
└── src/
```

## Documentation classes

- current project state: `docs/CURRENT_STATE.md`
- documentation governance/index: `docs/DOCUMENTATION_MODEL.md`
- architecture: `docs/ARCHITECTURE.md`
- roadmap: `docs/ROADMAP.md`
- per-boundary design: `docs/*_PLAN.md`
- per-boundary acceptance: `docs/*_ACCEPTANCE_PLAN.md`
- deferred/trigger-driven engineering: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator/handoff reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella implementation notes: `docs/IMPLEMENTATION_PLAN.md`
- harness/evidence and universal `deus-run` ZIP launch/result contract: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` (owner-native Windows runner v1.3.2 observed 2026-10-29; historical versions remain dated)
- development/acceptance host + cable topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- chronology: `CHANGELOG.md`

## Design principles

- Bare metal first.
- Keep boot, IRQ, task and ownership paths explicit.
- Prefer bounded static state over speculative dynamic machinery.
- Keep platform/register code below kernel/service/application semantics.
- Measure before optimizing.
- Do not invent connectivity, identity or runtime state that hardware cannot truthfully report.
- Promote deferred ideas only when a real consumer/problem exists and a dedicated boundary freezes scope and acceptance.
- Add filesystem, networking, update/security and other large subsystems only against a concrete product consumer.

## License

MIT
