# STM32 OS — Project Handoff

Status: **OPERATOR / ENVIRONMENT REFERENCE — CURRENT PROJECT STATE IS OWNED BY `docs/CURRENT_STATE.md`**

This file exists to help a new operator or chat resume work without rediscovering the hardware, tools and working conventions.

It is **not** a global current-state source and does not own the active boundary.

## 1. Read order

For a fresh session, read in this order:

1. `docs/CURRENT_STATE.md` — where the project is now;
2. `docs/DOCUMENTATION_MODEL.md` — source-of-truth precedence;
3. `docs/ARCHITECTURE.md` — stable architecture/invariants;
4. `docs/ROADMAP.md` — forward sequence;
5. the active boundary’s dedicated `*_PLAN.md` / `*_ACCEPTANCE_PLAN.md` once they exist;
6. `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` before building acceptance harnesses.

Historical execution detail lives in `docs/MASTER_EXECUTION_CHECKLIST.md` and `CHANGELOG.md`.

## 2. Repository / host paths

Windows repository:

`D:\Projects\STM32\OS`

Tools:

`D:\Projects\STM32\Tools`

User-run artifact download directory:

`C:\Users\DETU\Downloads\`

Primary shell:

- PowerShell 7

## 3. Target hardware

MCU:

- STM32F103 medium-density / Cortex-M3 / ARMv7-M
- 64 KiB Flash
- 20 KiB SRAM

Debug/recovery:

- ST-LINK V2
- SWD 950 kHz is the accepted reliable speed
- read-only fallback ladder when needed: `950, 480, 240, 125 kHz`
- ST-LINK remains recovery/debug rather than product management state

UART diagnostics:

- CH340 on Windows `COM3`
- USART1
- 115200 8N1
- UART remains emergency/text diagnostics

USB:

- target CDC may enumerate as `COM5` on Windows
- product management uses native USB WinUSB management interface
- CDC remains secondary diagnostics
- physical micro-USB unplug/replug is a power-cycle event because it is also the target power source

## 4. Toolchain

Target firmware:

- ARM GNU Toolchain 15.3.rel1
- STM32CubeProgrammer 2.23
- no HAL / Arduino / FreeRTOS

Host:

- .NET 10
- Windows WinUSB
- Linux libusb
- Avalonia desktop

Linux acceptance host used during Host Control acceptance:

- Ubuntu 26.04.1 LTS
- x86_64
- libusb 1.0.29
- usbutils 019

Exact current accepted product identities and versions are in `docs/CURRENT_STATE.md`.

## 5. Working rules

- Fail closed on unexpected repository/source state.
- Distinguish PRODUCT, HARNESS, ENVIRONMENT and EVIDENCE/STATE failures.
- Inspect both external log and full evidence ZIP before classifying hardware/runtime acceptance.
- Do not treat terminal PASS text alone as acceptance proof.
- Keep firmware/host source mutation separate from docs/evidence repair.
- Do not amend accepted commits to repair harness/evidence mistakes.
- Use ordinary non-force publication only.
- Before publication: fresh fetch + direct-parent/ahead-behind proof.
- After publication: fresh fetch + `HEAD == origin/main == FETCH_HEAD` + clean `0/0`.
- Minimize physical USB unplug/replug cycles.
- Do not flash unless the gate explicitly requires it.

Canonical operating rules:

`docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`

## 6. Artifact / evidence rules

Acceptance evidence should preserve:

- exact repo prestate/poststate;
- exact source/candidate identities;
- build/resource results;
- hardware/runtime diagnostics where applicable;
- final classification;
- external log plus full evidence archive;
- hashes of bound prior evidence where a composite/repaired verifier is used.

Historical evidence hashes are retained in the relevant boundary acceptance plan, execution ledger and changelog.

## 7. Documentation rules

Current-state facts must not be duplicated here.

Use:

- current state: `docs/CURRENT_STATE.md`
- architecture: `docs/ARCHITECTURE.md`
- roadmap: `docs/ROADMAP.md`
- deferred ideas: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- historical implementation rationale: `docs/IMPLEMENTATION_PLAN.md`

A completed boundary’s plan/acceptance pair remains the scoped canonical record for that boundary.

## 8. Hardware safety / operator notes

- Do not power the board simultaneously from conflicting 3.3 V sources.
- Prefer read-only SWD inspection before destructive recovery actions.
- Preserve UART diagnostics while testing USB management.
- Linux management acceptance claims only the vendor management interface; CDC must remain independently usable.
- Treat ST-LINK, UART and USB as different ownership/observability domains; do not fabricate a single generic “connected cable” product state.

## 9. Resume rule

Do **not** continue from old “next boundary” prose in historical documents.

Resume only from `docs/CURRENT_STATE.md`, then open the dedicated plan/acceptance pair for the active boundary.
