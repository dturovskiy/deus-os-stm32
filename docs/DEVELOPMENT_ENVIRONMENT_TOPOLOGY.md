# Deus OS — Development / Acceptance Environment Topology

Status: **CANONICAL OPERATIONAL ENVIRONMENT / HARDWARE-OWNERSHIP REFERENCE**

Last audited: 2026-09-25

This document owns the current development, hardware-attachment and acceptance-execution topology. It does **not** define product acceptance state, active gate status or firmware architecture; those remain owned by `docs/CURRENT_STATE.md`, the active boundary plan/acceptance pair and `docs/ARCHITECTURE.md`.

The purpose of this file is to prevent acceptance harnesses from executing a correct operation on the wrong machine or looking for a physical interface on a host that does not own it.

## 1. Canonical physical topology

The current bench is split across two execution hosts:

```text
                         SSH control / artifact transfer
Windows PowerShell 7  ------------------------------------>  Ubuntu on Mac mini
      |                                                        |
      |                                                        |  native USB
      | ST-LINK / SWD                                          |  VBUS + data
      | UART / CH340                                           |
      v                                                        v
                    +---------------------------+
                    |       STM32 target        |
                    |   STM32F103 / Deus OS     |
                    +---------------------------+
```

Exact ownership:

- **STM32 native USB is physically connected to the Ubuntu host on the Mac mini.**
- **STM32 UART/CH340 is physically connected to Windows.**
- **STM32 ST-LINK/SWD is physically connected to Windows.**
- Windows is the primary orchestration/build/debug host.
- Windows reaches the Ubuntu USB host through OpenSSH, using `deus@macmini` / SSH alias `macmini`.

This split-host topology is the default assumption for every new hardware acceptance package until this document is explicitly updated.

## 2. Windows execution domain

Canonical repository:

`D:\Projects\STM32\OS`

Tools root:

`D:\Projects\STM32\Tools`

Operator artifact directory:

`$env:USERPROFILE\Downloads`

Primary shell:

- PowerShell 7.

Windows owns:

- canonical Git working tree and normal Git mutation;
- GNU Arm firmware builds;
- .NET Windows restore/build/test work;
- STM32CubeProgrammer;
- ST-LINK/SWD target access, Flash readback/programming and reset;
- CH340/UART diagnostics;
- top-level acceptance orchestration;
- SSH/SCP control of the remote Ubuntu USB domain.

Current stable hardware/tool facts used by acceptance when relevant:

- ST-LINK V2, firmware `V2J48S7`;
- accepted SWD speed `950 kHz`;
- read-only fallback ladder `950, 480, 240, 125 kHz`;
- CH340 / USART1, historically `COM3`, `115200 8N1`;
- STM32CubeProgrammer `2.23`;
- Arm GNU Toolchain `15.3.Rel1` / GCC `15.3.1`;
- Windows .NET 10 policy is owned by `host/global.json`; the installed SDK selected by that policy is re-proven by each relevant harness.

COM numbers are enumeration facts, not permanent identities. A harness that needs UART must discover/verify the non-target CH340 instead of blindly trusting a historical COM number.

## 3. Ubuntu / Mac mini USB execution domain

SSH endpoint:

- canonical operator form: `deus@macmini`;
- accepted alias form: `macmini`.

The Ubuntu host owns the target's native USB connection. Historical accepted environment evidence records:

- host name `macmini`;
- user `deus`;
- Ubuntu `26.04.1 LTS`, x86_64;
- Linux .NET SDK `10.0.112`;
- libusb `1.0.29`;
- usbutils `019`.

The target composite USB device is `1209:000C` / `Deus OS Device`:

- CDC IF0/1 is normally owned by Linux `cdc_acm`;
- management IF2 is accessed by the Linux libusb adapter;
- the target USB cable also supplies normal target VBUS/power in this bench topology.

A current Windows harness MUST NOT assume that the STM32 USB device, WinUSB interface or CDC COM port is locally enumerable on Windows. Windows WinUSB remains an accepted product transport for a Windows-attached target, but it is **not the physical owner of target USB in the current bench wiring**.

## 4. Repository views are not the Mac mini

The canonical repository is the Windows tree `D:\Projects\STM32\OS`.

Within the Windows/WSL environment, these two paths have been proven to refer to the same Windows-backed repository object:

- `/mnt/d/Projects/STM32/OS`;
- `/home/deus/projects/deus-os-stm32/OS`.

That WSL bind/view relationship is local to the Windows development environment. It MUST NOT be confused with the remote Ubuntu host reached as `deus@macmini`.

The Mac mini does not need a persistent clone at `/home/deus/projects/deus-os-stm32/OS`. A historical harness that assumed such a remote repository failed correctly. Cross-host acceptance should normally materialize the exact required candidate/payload on Windows, transfer it to a unique remote `/tmp` directory by SCP, verify its hash remotely, execute there, return evidence, and remove the temporary remote directory.

## 5. Power and signal ownership

Normal current bench policy:

- target normal power/VBUS comes from the USB cable attached to the Ubuntu/Mac-mini host;
- ST-LINK provides SWD signals/ground but must not simultaneously provide a conflicting 3.3 V source;
- UART adapter provides TX/RX/ground; adapter VCC remains disconnected when the target is normally USB-powered.

Consequences:

- a **physical target power cycle** means removal/restoration of the target USB power connection on the Mac-mini side;
- disconnecting ST-LINK is not the normal target power-cycle primitive;
- an SWD software reset is initiated from Windows but USB disappearance/re-enumeration must be observed on the Ubuntu/Mac-mini host;
- physical USB reconnect evidence belongs to the USB-owning host even when the top-level package is orchestrated from Windows.

## 6. Acceptance execution-domain matrix

| Operation | Required execution domain |
| --- | --- |
| Git pre/poststate, temporary index, candidate materialization | Windows canonical repository |
| GNU Arm firmware build | Windows |
| Windows .NET restore/build/tests | Windows, with CWD under `host/` |
| STM32CubeProgrammer / ST-LINK read, erase, program, reset | Windows |
| UART / CH340 diagnostics | Windows |
| Target native USB discovery / `lsusb` | Ubuntu on Mac mini |
| Linux libusb management IF2 runtime | Ubuntu on Mac mini |
| Linux CDC presence/retention | Ubuntu on Mac mini |
| Physical USB disconnect/reconnect / USB-powered retention | Mac-mini USB connection, with remote observation |
| Cross-host coordination | Windows PowerShell -> SSH/SCP -> `deus@macmini` |
| Human OLED visual confirmation | physical target/operator |

A composed hardware gate may legitimately span both hosts. It must not collapse these domains into one generic "host".

## 7. Mandatory cross-host harness rules

Any acceptance package that needs both Windows-owned SWD/UART and Mac-mini-owned USB must:

1. identify every step with its execution domain before execution;
2. preflight the Windows resources it will need before destructive target mutation;
3. preflight SSH alias resolution and non-interactive public-key access before destructive target mutation;
4. preflight remote `hostname`, user, OS and target USB presence before destructive target mutation when USB is required;
5. use bounded external-process execution on both hosts;
6. close stdin and capture stdout/stderr independently;
7. use hard timeouts and process-tree cancellation;
8. transfer exact candidate/payload bytes by a bounded SCP primitive and verify SHA-256 after transfer;
9. use a unique remote temporary directory; never depend on a persistent remote repository unless that exact repository is independently proven for the run;
10. return remote raw evidence into the single authoritative Windows-side evidence ZIP;
11. clean remote temporary state after evidence transfer;
12. preserve exact host/domain labels in chronological evidence.

Recommended evidence tokens include:

```text
ORCHESTRATOR_DOMAIN=WINDOWS
CANONICAL_REPOSITORY=D:\Projects\STM32\OS
USB_EXECUTION_DOMAIN=MACMINI_UBUNTU
USB_SSH_TARGET=deus@macmini
UART_EXECUTION_DOMAIN=WINDOWS
STLINK_EXECUTION_DOMAIN=WINDOWS
```

Remote and local command records should also carry an explicit host/domain label so identical commands cannot be mistaken for execution on the other machine.

## 8. Reusable primitive baseline

The current environment has already produced accepted executed proofs for several building blocks used by later composed harnesses:

- bounded Windows child-process execution with redirected stdin/stdout/stderr, hard timeout and process-tree cancellation via `Kill(true)`;
- Windows OpenSSH alias/public-key execution and SCP transfer to the Mac-mini Ubuntu domain, including bounded remote temporary-directory workflows;
- Windows .NET locked restore followed by Release build with `host/global.json` in scope; the dedicated repair primitive passed after the corruption-matrix v3 harness exposed a stale restore-state assumption;
- Linux libusb access to the target management interface on the Mac-mini USB domain in prior Host Control acceptance.

These proofs authorize reuse of the **primitive semantics**, not blind reuse of stale paths/candidate hashes. Every composed harness still performs lightweight preflight of the actual endpoint/tool/interface it is about to use, and any topology/toolchain change that affects a primitive requires re-proof before destructive composition.

## 9. USB-management rule for the current bench

For current Asset/Configuration hardware acceptance:

- management IF2 traffic must be exercised through Linux/libusb on the Mac-mini Ubuntu host unless the operator explicitly rewires target USB to Windows and that topology change is re-proven;
- the Linux adapter is a first-class implementation, not a test shim: it discovers `1209:000C`, validates vendor IF2 `FF/00/00`, claims only interface 2 and uses EP4 OUT/IN `0x04/0x84`; it must not detach a kernel driver from IF2;
- production `deus-cp config ...` uses `AssetAccessPolicy.PublishedOnly`; while system capability bit 5 remains OFF during pre-publication Gates 1–5, ordinary CLI Asset commands are expected to reject the feature;
- Gate-5 acceptance that intentionally exercises the pre-publication Asset carrier must use the transport-neutral Core with `AssetAccessPolicy.PrepublicationAcceptance` through an acceptance-only helper/runner, not weaken the production CLI policy;
- Windows may still drive ST-LINK fault selectors, Flash corruption/readback and reset;
- Windows may still observe UART diagnostics;
- after a Windows-driven reset or Flash operation, runtime USB recovery/HELLO/RPC proof must be performed on the Mac-mini USB host;
- a package that looks only for local Windows WinUSB and declares the target absent is a harness-topology defect, not product evidence.

Failure class: `HARNESS-EXECUTION-DOMAIN-OWNERSHIP-01`.

## 10. Bundle / archive construction for this ecosystem

The operator-facing workflow remains:

`download ZIP -> run one PowerShell command on Windows -> return one self-contained evidence ZIP`.

A delivery ZIP that spans both hosts should contain, as applicable:

- `apply.ps1` — Windows orchestrator;
- `manifest.json`;
- `PACKAGE_README.txt`;
- `hashes.sha256`;
- Windows-local payloads;
- a bounded Linux payload script/archive to SCP to the Mac mini;
- exact candidate artifacts required by the gate.

The orchestrator owns the final evidence package. Remote Linux evidence is copied back and embedded in it; the operator must not manually merge Windows and Linux evidence.

The package must follow `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` for timestamps, section banners, categories, process control, evidence hashing, final red/green RESULT presentation and failure classification.

## 11. Topology-sensitive anti-patterns

Do not:

- search Windows for the current target USB device and treat absence as product failure;
- use local Windows WinUSB when the target cable is physically attached to the Mac mini;
- assume the Mac mini contains the canonical repository;
- confuse WSL `/home/deus/projects/...` with the remote Mac-mini filesystem;
- run CubeProgrammer on the Mac mini in the current topology;
- run CH340/UART acceptance on the Mac mini in the current topology;
- use ST-LINK 3.3 V or UART-adapter VCC as an accidental second target supply;
- call a Windows SWD reset a complete runtime proof without remote USB recovery verification;
- require the operator to manually combine two evidence archives.

## 12. Change control

If any cable ownership or execution host changes, update this document before composing the next hardware acceptance harness.

A topology change invalidates only assumptions that depend on that topology; it does not by itself invalidate accepted protocol/product contracts. The affected hardware-access and SSH/SCP primitives must be re-proven on the new execution domain before a composed gate is run.
