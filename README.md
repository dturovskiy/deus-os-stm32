# Deus OS / STM32 OS

A small bare-metal operating-system project for STM32F103 / ARM Cortex-M3.

The target firmware is written without STM32 HAL, Arduino, FreeRTOS, a heap, or a dynamic process loader. The project uses direct-register drivers, a bounded static scheduler/runtime, native USB, UART diagnostics, an SSD1306 local UI, and a C#/.NET host control application.

## Start here

For the **current project state**, use exactly one source:

- `docs/CURRENT_STATE.md`

For documentation roles and precedence:

- `docs/DOCUMENTATION_MODEL.md`

For stable architecture/invariants:

- `docs/ARCHITECTURE.md`

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
- primary host management: native USB WinUSB
- secondary USB diagnostics: CDC ACM
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

Exact accepted product state, candidate hashes and the active next boundary live in `docs/CURRENT_STATE.md`.

## Project structure

```text
OS/
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
- harness/evidence rules: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`
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
