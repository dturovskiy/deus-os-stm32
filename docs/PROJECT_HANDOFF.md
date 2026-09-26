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
5. `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` — which host physically owns USB, UART, ST-LINK and the canonical repository;
6. the active boundary’s dedicated `*_PLAN.md` / `*_ACCEPTANCE_PLAN.md`;
7. `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` before building acceptance harnesses.

Historical execution detail lives in `docs/MASTER_EXECUTION_CHECKLIST.md` and `CHANGELOG.md`.

## 2. Repository / host paths

Windows repository:

`D:\Projects\STM32\OS`

Tools:

`D:\Projects\STM32\Tools`

User-run artifact download directory:

`$env:USERPROFILE\Downloads\`

Primary shell:

- PowerShell 7

## 3. Target hardware

MCU:

- STM32F103 medium-density / Cortex-M3 / ARMv7-M
- accepted target discovery Device ID: `0x410`
- 64 KiB Flash
- 20 KiB SRAM

Debug/recovery:

- ST-LINK V2, firmware `V2J48S7`
- SWD 950 kHz is the accepted reliable speed
- read-only fallback ladder when needed: `950, 480, 240, 125 kHz`
- fully close STM32CubeProgrammer GUI before CLI access; the GUI can hold ST-LINK and cause `DEV_CONNECT_ERR`
- ST-LINK remains recovery/debug rather than product management state

UART diagnostics:

- CH340 on Windows `COM3`
- USART1
- 115200 8N1
- UART remains emergency/text diagnostics

USB / display:

- **current bench USB owner is the Ubuntu host on the Mac mini**, reached from Windows through `deus@macmini` / SSH alias `macmini`;
- current target USB management is therefore exercised through Linux libusb on the Mac mini; CDC IF0/1 remains on Linux `cdc_acm`;
- Windows WinUSB remains an accepted product transport when the target is physically attached to Windows, but it is not the current cable owner;
- Windows owns ST-LINK/SWD and CH340/UART in the current bench;
- the target USB cable is also the normal target power source, so the required physical power-cycle/reconnect action occurs on the Mac-mini USB connection;
- accepted OLED is SSD1306-class native `128x32` at I2C address `0x3C`;
- exact operational topology and cross-host harness rules are canonical in `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

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

Linux USB/runtime host on the current Mac-mini bench:

- SSH endpoint `deus@macmini` / alias `macmini`;
- Ubuntu 26.04.1 LTS;
- x86_64;
- .NET SDK 10.0.112 in the accepted environment baseline;
- libusb 1.0.29;
- usbutils 019;
- no persistent canonical repository on the Mac mini is assumed; exact payloads are transferred to bounded remote temporary directories when required.

Exact current accepted product identities and versions are in `docs/CURRENT_STATE.md`.

## 5. Working rules

- Fail closed on unexpected repository/source state.
- Distinguish PRODUCT, HARNESS, ENVIRONMENT and EVIDENCE/STATE failures.
- Inspect the authoritative self-contained evidence ZIP before classifying hardware/runtime acceptance; `run.log` is inside it, and any loose external log is optional compatibility output only.
- Do not treat terminal PASS text alone as acceptance proof.
- Build acceptance harnesses **primitive-first**: independently prove PowerShell process control, SSH/SCP, Git archive portability, .NET/MTP test invocation and any hardware-access primitive on the real execution domain before composing one collector.
- Never collapse the current split-host bench into one generic host: USB/libusb lives on `deus@macmini`; ST-LINK/CubeProgrammer and CH340/UART live on Windows. A Windows-driven reset/Flash step that needs runtime proof must verify USB recovery remotely on the Mac mini.
- Do **not** respond to harness failures with a `v1 -> v2 -> v3 -> ...` patch train. Freeze the composed collector, isolate/prove the failed primitive, then regenerate the collector from proven primitives. See `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` section 5.
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
- one self-contained authoritative evidence ZIP containing chronological `run.log`, structured `outcome.txt`, raw command evidence and hashes; a loose duplicate log is optional only;
- hashes of bound prior evidence where a composite/repaired verifier is used.

Historical evidence hashes are retained in the relevant boundary acceptance plan, execution ledger and changelog.

## 7. Documentation rules

Current-state facts must not be duplicated here.

Use:

- current state: `docs/CURRENT_STATE.md`
- development/acceptance topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- architecture: `docs/ARCHITECTURE.md`
- roadmap: `docs/ROADMAP.md`
- deferred ideas: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- execution history: `docs/MASTER_EXECUTION_CHECKLIST.md`
- historical implementation rationale: `docs/IMPLEMENTATION_PLAN.md`

A completed boundary’s plan/acceptance pair remains the scoped canonical record for that boundary.

## 8. Hardware safety / operator notes

- An original full-Flash backup was captured before replacing the pre-project firmware; the old handoff did not preserve a canonical restore path, so treat this as historical provenance rather than a current recovery artifact.
- Do not power the board simultaneously from conflicting 3.3 V sources.
- Prefer read-only SWD inspection before destructive recovery actions.
- Preserve UART diagnostics while testing USB management.
- Linux management acceptance claims only the vendor management interface; CDC must remain independently usable.
- Treat ST-LINK, UART and USB as different ownership/observability domains; do not fabricate a single generic “connected cable” product state.

## 9. Resume rule

Do **not** continue from old “next boundary” prose in historical documents.

Resume only from `docs/CURRENT_STATE.md`, then open the dedicated plan/acceptance pair for the active boundary.
