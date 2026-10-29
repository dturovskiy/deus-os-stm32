# STM32 OS — Project Handoff

> **Public-safe operational view:** Local operator paths, login identities and SSH aliases are placeholders. Substitute values only from separately authorized private operational configuration. This document does not authorize hardware actions; public protocol, ownership and pin-map invariants remain unchanged.

Status: **OPERATOR / ENVIRONMENT REFERENCE — CURRENT PROJECT STATE IS OWNED BY `docs/CURRENT_STATE.md`**

This file exists to help a new operator or chat resume work without rediscovering the hardware, tools and working conventions.

**Current handoff 2026-10-29:** The signed-v6 `1209:000E` Management-only runtime is installed and USB info/ping/health has returned the expected responses; OLED looked expected in the owner's qualitative observation. Windows retains ST-LINK/UART ownership; the Ubuntu Mac mini owns native USB in the verified installed-v6 session. Parent workspace `README.md` and `Reports/INDEX.md` now navigate off-Git archives. The 93 used/obsolete ZIPs are in `Bundle/Historical`; their original source paths in dated notes are **historical**, not presently runnable `deus-run` inputs. Do not replay signed-v6 update or obsolete receipt operators. Historical v1.3.1 references below are dated; latest accepted owner use of v1.3.2-source-candidate is recorded in `CURRENT_STATE.md`.


It is **not** a global current-state source and does not own the active boundary.

**Operator recovery from tool-response errors (2026-10-28):** If DEUS MCP accepts a task but a later task-result fetch is unavailable, do **not** declare the build failed or restart it. Read-only check the original `Bundle` artifact size/SHA and canonical validation result, then use verified native Windows `deus-run v1.3.1` only after original ZIP bytes are proven in the Windows `Bundle` root. This is a *control-plane response failure*, not an STM32/USB/Flash observation. Follow the normative `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` continuity section. Do not reuse a denied physical tool action under a different label and never retry an ambiguous reset/Flash.

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

Operator **input ZIP** location (**temporary runner staging, not evidence custody**):

`<OS_WORKSPACE_PARENT>\Bundle` — owner-selected off-Git local staging supported by the separately Windows-native accepted `deus-run v1.3.1` locator upgrade (2026-10-25). The older `$env:USERPROFILE\Downloads\` location is a historical/alternate lookup route, not the current required output or custody directory. A changed input root requires independent Windows runner lookup proof; do not infer it from a Linux/DEUS MCP `Bundle/` view.

Native Windows operator **output evidence** location (owner-created; exact path resolved from canonical OS checkout):

`<OS_WORKSPACE_PARENT>\Evidence` — sibling of `OS`, `Tools`, and `Logs`, outside PUBLIC/PRIVATE Git, original `.evidence.zip` + log preserved on PASS and classified FAIL. This directory is **not** the restricted recovery-image/key store. Windows `deus-run v1.3.1` searches the native-verified owner-selected off-Git `Bundle` input (and retains accepted historical search behavior); the operator child reports the evidence file SHA and absolute location and the runner verifies them. The 2026-10-24 owner reported manual migration of older Downloads evidence; moved originals require independent exact-size/SHA/CRC verification before asserting their new custody.

Artifact home: reviewed/sanitized **runnable bundle ZIPs** may be published to private `dturovskiy/deus-os-stm32-private-docs/operator_artifacts/` only after authorized publication and independent remote-byte readback. Original raw evidence stays owner-local under `Evidence`; original BIN/PKG and recovery images belong in the independently ACL-verified sibling `<OS_WORKSPACE_PARENT>\\ProtectedRecovery` (**created 2026-10-24 01:25 with Windows restricted NTFS ACL smoke; exact historical unsigned rev5 BIN deposited and independently SHA/size re-opened 01:47**; not a signed rescue or independently backed-up vault). Signer keys and DPAPI require their own separately authorized secure custody/restore, **not** relocation into ordinary project directories. Previous empty `%LOCALAPPDATA%\\DeusOS\\ProtectedRecovery` is legacy after the owner changed the storage layout; do not delete it without independently verifying it is empty. Public Git retains only safe metadata. The mandatory process is **ONE operator bundle, one command, adjudicate the original evidence before the next**, per the canonical `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

Proposed owner-portable historical operator input archive: `<OS_WORKSPACE_PARENT>\\OperatorArchive` as an additional sibling of `OS`, `Tools`, `Logs`, `Evidence`, and `ProtectedRecovery`, outside Git. **Not yet created, copied, hash-attested or accepted.** The 2026-10-24 02:14 bounded Windows Downloads metadata inventory found 75 named operator ZIPs and assorted old runner scripts; treat these as untrusted archival candidates until independently validating each ZIP's source bytes/CRC/manifest, scope, secret screening and exact destination rehash, separately verifying enough disk space, and explicitly authorizing retention/cleanup. Do not place DPAPI/escrow/private signing material, raw firmware/recovery BIN, or original execution evidence in OperatorArchive, or treat arbitrary old ZIPs as executable commands on a new host. On new PC, install an accepted runner separately and validate shell profile; don't copy an old profile/installer blindly. Exact audit evidence SHA `BEA298BC6FFA059AF0FF09C96C1B4DAB4D2912D44537F1FD7164CCBF9A19CB35`, indexed ZIP verified. `GATE3=NOT_ACCEPTED`, `GATE4=NOT_AUTHORIZED`.

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
- **Latest owner-reported terminal-only USB observation (2026-10-28): Ubuntu Mac mini**, runtime `1209:000C` on physical `usb1/1-8`, bus `001`, ephemeral device `090`, CDC IF0/IF1 + vendor IF2; Linux node `root:plugdev 0660`, user `deus` in `plugdev` with shell `test -r/-w` both YES. This is NOT original indexed Evidence, no native `libusb_open()` or bootloader `000D` proof and no Gate-3 acceptance promotion. Earlier scoped native USB evidence (2026-10-19/27) remains authoritative for its own exact checks; the temporary 2026-10-16 Windows WinUSB Desktop Gate 3b connection is historical. Reconfirm USB ownership for any future hardware gate;
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
- **Standard launch confirmed on Windows 2026-10-19 (v1.3.0), with off-Git `Bundle` locator upgrade natively accepted 2026-10-25 (installed v1.3.1):** the installed **DEUS Runner** is the one default entry point for new `DEUS_OS_OPERATOR_BUNDLE_V2` ZIPs. Launch in PowerShell 7 using **`deus-run <package_id> -ExpectedSha256 <exact_zip_sha256>`**, never a hand-written `Expand-Archive`, direct `py`/`pwsh` or per-bundle bootstrap. Root ZIP files are `apply.ps1`, `manifest.json`, `hashes.sha256`, `PACKAGE_README.txt`; new bundles declare `runner_result_contract=DEUS_OS_RUNNER_RESULT_V1`. `apply.ps1` emits JSON through `DEUS_RUN_RESULT_PATH`, while the runner owns the **last green PASS / red FAIL terminal result**, classification, phase/error and verified evidence SHA. A new Windows Gate-3 hardware or qualification run must produce one self-contained `.evidence.zip` under the validated OS-workspace-sibling `Evidence` directory, including on classified failure; the 2026-10-19 **offline runner smoke** was deliberately hardware-free, `EVIDENCE_ZIP=NONE`, and does not validate product Gate 3. When the command is unavailable, diagnose installation/profile with `Get-Command deus-run`, not by changing the gate package. Full contract: `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.
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
- On the **currently installed composite `1209:000C` runtime**, Management IF2 and USB CDC IF0/1 have separate accepted semantics, so a Management-only observation is not proof of CDC. The Gate-0-published but unflashed future `1209:000E` Management-only design **intentionally removes USB CDC**; do not impose the old CDC-preservation requirement on its proposed new scope, and never mistake current `000C/IF2` testing for deployed `000E/IF0` acceptance.
- Treat ST-LINK, UART and USB as different ownership/observability domains; do not fabricate a single generic “connected cable” product state.

**2026-10-20 Management-only migration handoff:** two `MANAGEMENT_ONLY_RUNTIME_MIGRATION_*` plans are **Gate-0 ACCEPTED/PUBLISHED** at `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09` (exact-SHA [CI 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) **SUCCESS**), with `ACTIVE_GATE=GATE1_OFFLINE`; do not execute Gate-4 firmware writes. Current-board `000C/IF2` no-CDC Management/UART bracket and fixed read-only `mspstat/schedprod/fault` PASS are retained in the scoped Acceptance Plan and must not be rerun solely for bookkeeping. Proposed same-tree `000E/IF0` ARM/Host builds PASS offline but have not been flashed. Before any new candidate operation, reconcile exact Gate-1/2 static negatives and Gate-3 rev5 recovery metadata; Gate-2 static budget must not demand uninstalled-`000E` runtime watermarks. Only an explicit later owner GO can permit flash/reset/cable/power changes or signing actions; protected signing/vault/bootloader bytes remain private.

## 9. Resume rule

Do **not** continue from old “next boundary” prose in historical documents.

Resume only from `docs/CURRENT_STATE.md`, then open the dedicated plan/acceptance pair for the active boundary.
