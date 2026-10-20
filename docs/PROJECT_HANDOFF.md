# STM32 OS — Project Handoff

> **Public-safe operational view:** Local operator paths, login identities and SSH aliases are placeholders. Substitute values only from separately authorized private operational configuration. This document does not authorize hardware actions; public protocol, ownership and pin-map invariants remain unchanged.

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

`<WINDOWS_OPERATOR_PATH>`

Tools:

`<WINDOWS_OPERATOR_PATH>`

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

- CH340 on Windows `<UART_COM_PORT>`
- USART1
- 115200 8N1
- UART remains emergency/text diagnostics

USB / display:

- **Canonical expected wiring:** native USB/VBUS normally belongs to the Ubuntu Mac mini via libusb, reached from Windows via `<SSH_USER>@<USB_HOST>` / `<USB_HOST_ALIAS>`. CDC IF0/1 normally binds on that Linux USB host;
- **Latest positively observed USB attachment (2026-10-19): Ubuntu Mac mini**, runtime `1209:000C` Management IF2 and CDC IF0/IF1, corroborated by the bounded Management/CDC/UART evidence; the earlier 2026-10-16 Windows WinUSB Desktop Gate 3b connection remains only historical. Reconfirm USB ownership for any future hardware gate;
- verify actual USB device/IF2 on the intended host using bounded read-only discovery before any future hardware gate; no USB rewire, power-cycle or driver change is requested by this documentary status;
- Windows owns ST-LINK/SWD and CH340/UART in the canonical bench; Windows WinUSB remains supported only while the target is actually Windows-attached;
- the native USB cable is the normal target VBUS/power source; any separately approved physical power-cycle/reconnect must be executed on the **verified USB-owning host**, not blindly attributed to Ubuntu;
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

Linux USB/runtime host on the current split-host bench:

- SSH endpoint `<SSH_USER>@<USB_HOST>` / alias `<USB_HOST_ALIAS>`;
- Ubuntu 26.04.1 LTS;
- x86_64;
- .NET SDK 10.0.112 in the accepted environment baseline;
- libusb 1.0.29;
- usbutils 019;
- no persistent canonical repository on the Linux USB host is assumed; exact payloads are transferred to bounded remote temporary directories when required.

Exact current accepted product identities and versions are in `docs/CURRENT_STATE.md`.

## 5. Working rules

- Fail closed on unexpected repository/source state.
- Distinguish PRODUCT, HARNESS, ENVIRONMENT and EVIDENCE/STATE failures.
- Inspect the authoritative self-contained evidence ZIP before classifying hardware/runtime acceptance; `run.log` is inside it. Do not emit a loose duplicate log by default; create one only when the operator explicitly requests compatibility output.
- Do not treat terminal PASS text alone as acceptance proof.
- **Standard launch confirmed on Windows 2026-10-19:** installed **DEUS Runner v1.3.0** is the one default entry point for new `DEUS_OS_OPERATOR_BUNDLE_V2` ZIPs. Launch in PowerShell 7 using **`deus-run <package_id> -ExpectedSha256 <exact_zip_sha256>`**, never a hand-written `Expand-Archive`, direct `py`/`pwsh` or per-bundle bootstrap. Root ZIP files are `apply.ps1`, `manifest.json`, `hashes.sha256`, `PACKAGE_README.txt`; new bundles declare `runner_result_contract=DEUS_OS_RUNNER_RESULT_V1`. `apply.ps1` emits JSON through `DEUS_RUN_RESULT_PATH`, while the runner owns the **last green PASS / red FAIL terminal result**, classification, phase/error and verified evidence SHA. A hardware gate must produce one self-contained `.evidence.zip` in Downloads even on classified failure; the 2026-10-19 **offline runner smoke** was deliberately hardware-free, `EVIDENCE_ZIP=NONE`, and does not validate product Gate 3. When the command is unavailable, diagnose installation/profile with `Get-Command deus-run`, not by changing the gate package. Full contract: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.
- Build acceptance harnesses **primitive-first**: independently prove PowerShell process control, SSH/SCP, Git archive portability, .NET/MTP test invocation and any hardware-access primitive on the real execution domain before composing one collector.
- Never collapse the canonical split-host bench into one generic host: USB/libusb normally belongs to `<SSH_USER>@<USB_HOST>`; ST-LINK/CubeProgrammer and CH340/UART belong to Windows. The 2026-10-16 temporary Windows WinUSB attachment is **historical**; 2026-10-19/20 Linux Management physical evidence verifies that native USB subsequently returned to Ubuntu at those observation times. Reconfirm actual USB attachment for any newly authorized reset/Flash/recovery acceptance; observe USB recovery on the **verified** owning host.
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
- one self-contained authoritative evidence ZIP containing chronological `run.log`, structured `outcome.txt`, raw command evidence and hashes; no loose duplicate log by default;
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
- On the **currently installed composite `1209:000C` runtime**, Management IF2 and USB CDC IF0/1 have separate accepted semantics, so a Management-only observation is not proof of CDC. The owner-approved but unpublished future `1209:000E` Management-only design **intentionally removes USB CDC**; do not impose the old CDC-preservation requirement on its proposed new scope, and never mistake current `000C/IF2` testing for deployed `000E/IF0` acceptance.
- Treat ST-LINK, UART and USB as different ownership/observability domains; do not fabricate a single generic “connected cable” product state.

**2026-10-20 Management-only migration handoff:** two `MANAGEMENT_ONLY_RUNTIME_MIGRATION_*` plans have **owner-approved Gate-0 design but remain unpublished**, with formal `GATE0=NOT_ACCEPTED_UNTIL_DOCS_PUBLISHED`, `ACTIVE_GATE=NONE`; do not execute Gate-4 firmware writes. Current-board `000C/IF2` no-CDC Management/UART bracket and fixed read-only `mspstat/schedprod/fault` PASS are retained in the scoped Acceptance Plan and must not be rerun solely for bookkeeping. Proposed same-tree `000E/IF0` ARM/Host builds PASS offline but have not been flashed. Before any new candidate operation, reconcile exact Gate-1/2 static negatives and Gate-3 rev5 recovery metadata; Gate-2 static budget must not demand uninstalled-`000E` runtime watermarks. Only an explicit later owner GO can permit flash/reset/cable/power changes or signing actions; protected signing/vault/bootloader bytes remain private.

## 9. Resume rule

Do **not** continue from old “next boundary” prose in historical documents.

Resume only from `docs/CURRENT_STATE.md`, then open the dedicated plan/acceptance pair for the active boundary.
