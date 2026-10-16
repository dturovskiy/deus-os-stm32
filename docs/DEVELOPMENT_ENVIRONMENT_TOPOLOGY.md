# Deus OS — Development / Acceptance Environment Topology

> **Public-safe operational view:** Local operator paths, login identities and SSH aliases are placeholders. Substitute values only from separately authorized private operational configuration. This document does not authorize hardware actions; public protocol, ownership and pin-map invariants remain unchanged.

Status: **CANONICAL OPERATIONAL ENVIRONMENT / HARDWARE-OWNERSHIP REFERENCE**

Last canonical-layout audit: 2026-10-07. Last separately accepted temporary USB attachment: Windows on 2026-10-16 (Desktop Inspector read-only Gate 3b).

This document owns the current development, hardware-attachment and acceptance-execution topology. It does **not** define product acceptance state, active gate status or firmware architecture; those remain owned by `docs/CURRENT_STATE.md`, the active boundary plan/acceptance pair and `docs/ARCHITECTURE.md`.

The purpose of this file is to prevent acceptance harnesses from executing a correct operation on the wrong machine or looking for a physical interface on a host that does not own it.

## 1. Canonical physical topology

**Important distinction — designed/default bench versus physically observed connection:** The **canonical expected/default wiring** remains native STM32 USB/VBUS on Ubuntu Mac mini, with UART/CH340 and ST-LINK/SWD on Windows. However, the **last positively observed native-USB attachment** was temporary **Windows WinUSB** during the operator-attested Desktop Gate 3b run on 2026-10-16. **Physical return of the USB cable to Ubuntu is NOT_VERIFIED**. Thus the canonical diagram below is not evidence that Ubuntu owns the cable *right now*. Before any new hardware acceptance, verify current USB ownership on the intended host using bounded read-only discovery; do not assume the cable moved back, invoke the other host's USB API blindly, change drivers or power-cycle/reconnect merely for this documentary clarification. The temporary approved Windows test did not change this canonical bench design or authorize further hardware activity.

The canonical/default bench design is split across two execution hosts:

```text
                         SSH control / artifact transfer
Windows PowerShell 7  ------------------------------------>  Linux USB host (Ubuntu)
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

Canonical/default intended ownership (reverify physical attachment before execution):

- **STM32 native USB is normally intended to be connected to the Linux USB host (Ubuntu).** Last verified connection was instead Windows on 2026-10-16; return is unconfirmed.
- **STM32 UART/CH340 is physically connected to Windows.**
- **STM32 ST-LINK/SWD is physically connected to Windows.**
- Windows is the primary orchestration/build/debug host.
- Windows reaches the Ubuntu USB host through OpenSSH, using `<SSH_USER>@<USB_HOST>` / SSH alias `<USB_HOST_ALIAS>`.

This split-host topology is the **expected/default operating model**, not proof of current cable attachment. Every new hardware acceptance package must independently discover the physical USB-owning host first; persistent topology changes require an explicitly approved document update.

## 2. Windows execution domain

Canonical repository:

`<WINDOWS_OPERATOR_PATH>`

Tools root:

`<WINDOWS_OPERATOR_PATH>`

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
- CH340 / USART1, historically `<UART_COM_PORT>`, `115200 8N1`;
- STM32CubeProgrammer `2.23`;
- Arm GNU Toolchain `15.3.Rel1` / GCC `15.3.1`;
- Windows .NET 10 policy is owned by `host/global.json`; the installed SDK selected by that policy is re-proven by each relevant harness.

COM numbers are enumeration facts, not permanent identities. A harness that needs UART must discover/verify the non-target CH340 instead of blindly trusting a historical COM number.

## 3. Linux USB execution domain (Ubuntu)

SSH endpoint:

- canonical operator form: `<SSH_USER>@<USB_HOST>`;
- accepted alias form: `<USB_HOST_ALIAS>`.

The Ubuntu host owns the target's native USB connection. Historical accepted environment evidence records:

- host name `<USB_HOST_ALIAS>`;
- user `<SSH_USER>`;
- Ubuntu `26.04.1 LTS`, x86_64;
- Linux .NET SDK `10.0.112`;
- libusb `1.0.29`;
- usbutils `019`.

The target composite USB device is `1209:000C` / `Deus OS Device`:

- CDC IF0/1 is normally owned by Linux `cdc_acm`;
- management IF2 is accessed by the Linux libusb adapter;
- the target USB cable also supplies normal target VBUS/power in this bench topology.

The firmware-update bootloader enumerates separately as private-test `1209:000D` / `Deus OS Bootloader`. Enumeration alone does not guarantee unprivileged libusb open authority: accepted hardware evidence has observed the current `/dev/bus/usb/BBB/DDD` node as `root:root` mode `664`, causing `LIBUSB_ERROR_ACCESS (-3)` for user `<SSH_USER>`. When a bootloader transaction requires permission repair, the accepted bench fixture is bounded to the exact currently enumerated ephemeral node: one operator-visible interactive privilege step may change that node to owner `deus`, mode `600`; no password is captured, no persistent udev rule is created, and the actual bootloader protocol must then execute unprivileged as `deus`. Bus/device tokens from `lsusb` are already zero-padded decimal strings and must be used as strings when constructing `/dev/bus/usb/$bus/$dev` (for example device `099`), not numerically reinterpreted.

A Windows harness MUST NOT assume that STM32 USB/WinUSB/CDC is locally enumerable merely from the canonical Ubuntu-owned design. Windows WinUSB is an accepted transport **when physical discovery confirms a Windows-attached target** (last observed at the temporary 2026-10-16 UI test). Conversely Ubuntu libusb must not be assumed reachable until its USB preflight passes; the later return of the cable is unverified.

## 4. Repository views are not the Linux USB host

The canonical repository is the Windows tree `<WINDOWS_OPERATOR_PATH>`.

**DEUS MCP runs in WSL on the Windows development machine, not on the Linux USB host.** Its project root `<WSL_OPERATOR_PATH>` and repository path `<WSL_OPERATOR_PATH>` are local WSL views of the Windows-backed working tree. A DEUS MCP file/Git operation is therefore **local WSL repository access**, not remote Linux USB execution, and it does not by itself prove Windows PowerShell/ARM toolchain availability. Windows remains the accepted GNU Arm build/SWD/UART execution domain; native USB remains on Linux USB host (Ubuntu).

Within the Windows/WSL environment, the Windows checkout and local WSL bind/view refer to the same Windows-backed repository object. Operator-specific absolute paths are intentionally omitted from this public view:

- Windows checkout: `<WINDOWS_OPERATOR_PATH>`;
- local WSL view: `<WSL_OPERATOR_PATH>`.

That WSL bind/view relationship is local to the Windows development environment. It MUST NOT be confused with the remote Ubuntu host reached as `<SSH_USER>@<USB_HOST>`.

The Linux USB host does not need a persistent clone at `<WSL_OPERATOR_PATH>`. A historical harness that assumed such a remote repository failed correctly. Cross-host acceptance should normally materialize the exact required candidate/payload on Windows, transfer it to a unique remote `/tmp` directory by SCP, verify its hash remotely, execute there, return evidence, and remove the temporary remote directory.

## 5. Power and signal ownership

Normal current bench policy:

- target normal power/VBUS comes from the USB cable attached to the Linux USB host (Ubuntu) host;
- ST-LINK provides SWD signals/ground but must not simultaneously provide a conflicting 3.3 V source;
- UART adapter provides TX/RX/ground; adapter VCC remains disconnected when the target is normally USB-powered.

Consequences:

- a **physical target power cycle** means removal/restoration of the target USB power connection on the Linux USB host side;
- disconnecting ST-LINK is not the normal target power-cycle primitive;
- an SWD software reset is initiated from Windows but USB disappearance/re-enumeration must be observed on the Linux USB host (Ubuntu) host;
- physical USB reconnect evidence belongs to the USB-owning host even when the top-level package is orchestrated from Windows.

### 5.1 Target wiring / pin map

The current physical bench must be interpreted together with the target-side pin map below.
These pins are stable hardware ownership facts for the current STM32F103/Blue-Pill-class
target unless a later hardware change explicitly updates this document:

| Function | STM32F103 pin(s) | Physical connection / owner |
| --- | --- | --- |
| Native USB FS | PA11 `USB_DM`, PA12 `USB_DP` | USB data + normal target VBUS/power to Linux USB host (Ubuntu) |
| Emergency UART / CH340 | PA9 `USART1_TX`, PA10 `USART1_RX` | CH340 on Windows, 115200 8N1; TX/RX/GND connected, adapter VCC disconnected while USB powers target |
| SWD debug/recovery | PA13 `SWDIO`, PA14 `SWCLK` | ST-LINK on Windows with shared ground; ST-LINK 3.3 V disconnected while USB powers target |
| OLED / I2C1 | PB6 `I2C1_SCL`, PB7 `I2C1_SDA` | SSD1306-class 128x32 display |
| Heartbeat LED | PC13 | on-board active-low LED |

Consequences for harnesses and product boundaries:

- native USB acceptance must not repurpose PA11/PA12 or look for the current target USB on Windows unless the cable topology is explicitly changed and re-proven;
- UART diagnostics remain on PA9/PA10 and are not a silent fallback product-update carrier;
- SWD remains on PA13/PA14 and is recovery/debug authority, not normal product update;
- bootloader/update code must not seize PB6/PB7 or require the OLED/I2C path for update correctness;
- any pin, cable, VCC or host-owner change is a topology change and requires this document to be updated before the next composed hardware gate.

## 6. Acceptance execution-domain matrix

| Operation | Required execution domain |
| --- | --- |
| Git pre/poststate, temporary index, candidate materialization | Windows canonical repository |
| GNU Arm firmware build | Windows |
| Windows .NET restore/build/tests | Windows, with CWD under `host/` |
| STM32CubeProgrammer / ST-LINK read, erase, program, reset | Windows |
| UART / CH340 diagnostics | Windows |
| Target native USB discovery / `lsusb` | Linux USB host (Ubuntu) |
| Linux libusb management IF2 runtime | Linux USB host (Ubuntu) |
| Linux CDC presence/retention | Linux USB host (Ubuntu) |
| Physical USB disconnect/reconnect / USB-powered retention | Linux USB connection, with remote observation |
| Cross-host coordination | Windows PowerShell -> SSH/SCP -> `<SSH_USER>@<USB_HOST>` |
| Human OLED visual confirmation | physical target/operator |

A composed hardware gate may legitimately span both hosts. It must not collapse these domains into one generic "host".

## 7. Mandatory cross-host harness rules

Any acceptance package that needs both Windows-owned SWD/UART and Linux USB host-owned USB must:

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
CANONICAL_REPOSITORY=<WINDOWS_OPERATOR_PATH>
USB_EXECUTION_DOMAIN=MACMINI_UBUNTU
USB_SSH_TARGET=<SSH_USER>@<USB_HOST>
UART_EXECUTION_DOMAIN=WINDOWS
STLINK_EXECUTION_DOMAIN=WINDOWS
```

Remote and local command records should also carry an explicit host/domain label so identical commands cannot be mistaken for execution on the other machine.

## 8. Reusable primitive baseline

The current environment has already produced accepted executed proofs for several building blocks used by later composed harnesses:

- bounded Windows child-process execution with redirected stdin/stdout/stderr, hard timeout and process-tree cancellation via `Kill(true)`;
- Windows OpenSSH alias/public-key execution and SCP transfer to the Linux USB host Ubuntu domain, including bounded remote temporary-directory workflows;
- Windows .NET locked restore followed by Release build with `host/global.json` in scope; the dedicated repair primitive passed after the corruption-matrix v3 harness exposed a stale restore-state assumption;
- Linux libusb access to the target management interface on the Linux USB execution domain in prior Host Control acceptance.

These proofs authorize reuse of the **primitive semantics**, not blind reuse of stale paths/candidate hashes. Every composed harness still performs lightweight preflight of the actual endpoint/tool/interface it is about to use, and any topology/toolchain change that affects a primitive requires re-proof before destructive composition.

## 9. USB-management rule for the current bench

For current published Asset/Configuration operation and any later acceptance that reuses this topology:

- management IF2 traffic must be exercised through Linux/libusb on the Linux USB host Ubuntu host unless the operator explicitly rewires target USB to Windows and that topology change is re-proven;
- the Linux adapter is a first-class implementation, not a test shim: it discovers `1209:000C`, validates vendor IF2 `FF/00/00`, claims only interface 2 and uses EP4 OUT/IN `0x04/0x84`; it must not detach a kernel driver from IF2;
- production `deus-cp config ...` uses `AssetAccessPolicy.PublishedOnly`; the published target advertises the accepted Asset capability, so ordinary CLI Asset operations are governed by the published capability/identity contract rather than the historical pre-publication gate state;
- historical Asset Gates 1–5 intentionally kept system capability bit 5 OFF and used `AssetAccessPolicy.PrepublicationAcceptance` only inside acceptance-only helpers/runners; that historical mechanism must not be treated as current product policy or reintroduced into the production CLI;
- Windows may still drive ST-LINK fault selectors, Flash corruption/readback and reset when a separately authorized acceptance boundary requires those operations;
- Windows may still observe UART diagnostics;
- after a Windows-driven reset or Flash operation, runtime USB recovery/HELLO/RPC proof must be performed on the Linux USB host;
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
- a bounded Linux payload script/archive to SCP to the Linux USB host;
- exact candidate artifacts required by the gate.

The orchestrator owns the final evidence package. Remote Linux evidence is copied back and embedded in it; the operator must not manually merge Windows and Linux evidence.

The package must follow `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` for timestamps, section banners, categories, process control, evidence hashing, final red/green RESULT presentation and failure classification.

## 11. Topology-sensitive anti-patterns

Do not:

- search Windows for the current target USB device and treat absence as product failure;
- use local Windows WinUSB when the target cable is physically attached to the Linux USB host;
- assume the Linux USB host contains the canonical repository;
- confuse WSL `<WSL_OPERATOR_PATH>` with the remote Linux USB host filesystem;
- run CubeProgrammer on the Linux USB host in the current topology;
- run CH340/UART acceptance on the Linux USB host in the current topology;
- use ST-LINK 3.3 V or UART-adapter VCC as an accidental second target supply;
- call a Windows SWD reset a complete runtime proof without remote USB recovery verification;
- require the operator to manually combine two evidence archives.

## 12. Change control

If any cable ownership or execution host changes, update this document before composing the next hardware acceptance harness.

A topology change invalidates only assumptions that depend on that topology; it does not by itself invalidate accepted protocol/product contracts. The affected hardware-access and SSH/SCP primitives must be re-proven on the new execution domain before a composed gate is run.
