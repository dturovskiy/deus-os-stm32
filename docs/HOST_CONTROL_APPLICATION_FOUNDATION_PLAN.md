# Deus OS — Host Control Application Foundation Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `e0f49f168542fa1cf49bca451e01b0c077aa8d18`**

Boundary:

`HOST_CONTROL_APPLICATION_FOUNDATION`

Published firmware baseline:

- repository publication before this boundary: `b51e1b144bfbcb93ce1e32e5cfa33a8ccecacf1e`;
- USB-management firmware publication: `1f88083843c6aae9fd228ad2d677f9252b889a11`;
- USB-management publication tree: `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`;
- hardware-accepted firmware candidate tree: `46841b52d351277deb134a6f4709619087b477af`;
- accepted BIN: `50172` bytes / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`;
- binary framing protocol: v1;
- command-service foundation: v2 / 35 RPC methods / IDs `0x0001..0x0023`;
- application runtime ABI: v1;
- Windows management discovery GUID: `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`;
- accepted USB profile: private-test `1209:000C` / `Deus OS Device`, management IF2, EP4 OUT/IN `0x04/0x84` bulk64.

Canonical predecessors:

- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`;
- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`;
- `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md`;
- `docs/USB_MANAGEMENT_DEVICE_FOUNDATION_ACCEPTANCE_PLAN.md`.

Gate 5 accepted candidate entering Gate 6:

- parent repository commit/tree before the boundary acceptance commit: `61d8e138e7ca44aefbad546e0a4ad5561848a260` / `58e2078dc0eadbd76ac12cc3c12b792413555944`;
- hardware-accepted firmware candidate tree: `b895955f7738aceb6fca0272d510cc433378c6ab`;
- accepted host source candidate tree: `2c5afd9914851300aed15e321cf69c3a2c3daeed`;
- BIN `50652` bytes / SHA-256 `FB68993FC998DE77B61FAF9F4949E4E124B95FF867BB456EBA401C9F2709F13F`;
- ELF `483488` bytes / SHA-256 `978CC710497F79E9A3AFBAFE3E0CD3AE10BA2B6CF0FE6AA7518B7AC5BC5E524C`;
- MAP `207952` bytes / SHA-256 `F9806E2A71476D6D2DAC2CD481A6C491BACD18CADD713813BB69AF9DA608C032`;
- Flash/SRAM `50652/11728` within frozen ceilings `54780/11824`; task stacks `1024/512`; undefined symbols `0`; accepted task0/task1 margins `424/424`;
- host target `net10.0`; Windows SDK `10.0.201`; Linux SDK `10.0.112`; Avalonia `12.1.2`; `xunit.v3.mtp-v2` `4.0.1`;
- Core tests `21/21`; Transport tests `5/5`;
- exact HELLO: protocol `1`, service `3`, max args `4`, line capacity `32`, request payload max `132`, data chunk max `48`, registry `36`, protocol capability flags `0x0000003F`;
- exact `sysinfo`: ABI `1`, `DEUS_OS`, `STM32F103C8`, `ARMV7M`, source tree equal to the firmware candidate above, protocol `1`, service `3`, runtime ABI `1`, system capability mask `0x0000001F`, unit-id kind `0`;
- initial applications remain `system.home=0x0001` and `device.info=0x0002`;
- Windows CLI/hardware Gate 3 PASS including malformed-vector recovery; Windows Desktop Gate 4 PASS; Linux Gate 5 real-hardware runtime PASS;
- Windows and Linux physical reconnect both require a fresh HELLO + `sysinfo`; Linux same-port recovery is keyed by stable `usb:<bus>:<physical-port-path>` and passed with enumeration address `007 -> 008` while locator remained `usb:001:8`;
- final Gate 5 diagnostics: USB errors/PMA overruns `0/0`, management RX/TX packets `13/113`, management drops `0/0`, CDC drops `0/0`, scheduler faults `0`;
- Gate-4 target display disposition: `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`;
- final target Flash readback is exactly the accepted BIN;
- Gate 5 final composite evidence/log SHA-256: `564BF507C54D802ACD590012384A6FA2BE7483C40F94654802C5316071702BFE` / `BFF093889489077BBF60B97E083D34082ECF8AE40322E2710EA2B77FF2893591`.

Gate 6 changes documentation only before staging the already hardware-accepted source set. The one normal local acceptance commit is the Gate 6 output; its exact commit/tree identity is recorded by Gate 6 evidence. No amend and no push are authorized in Gate 6.
## 1. Purpose

This boundary creates the first real Windows/Linux host management application for Deus OS.

The host application is a management plane. The STM32 remains independently bootable and operational without the host. Firmware retains ownership of hardware, safety, scheduler/watchdog behavior, application lifecycle, local UI and RPC semantics.

The host owns:

- discovery and connection;
- HELLO/protocol negotiation;
- stable system/build/platform/capability presentation;
- binary-RPC request/response orchestration;
- health/diagnostic presentation;
- application list/start/stop UX;
- reconnect handling;
- later transfer/update orchestration, but not transfer/update implementation in this boundary.

Provisional product name remains **Deus OS CP** / **Deus OS Control Panel**.

## 2. Technology freeze

Host implementation baseline:

- language: **C#**;
- runtime/SDK target: **.NET 10 LTS**, `net10.0`;
- desktop UI: **Avalonia 12.x**;
- command-line reference client: .NET console application;
- tests: `dotnet test` with no hardware dependency for protocol/core unit tests.

The protocol core must not reference Avalonia.

The desktop shell must not contain USB framing, CRC, request correlation or RPC parsing logic. It consumes the same Core API used by the CLI.

The boundary does not authorize Electron, browser-only WebUSB, WPF-only architecture, platform-specific business logic, or a second independent protocol implementation.

## 3. Host project/source ownership

Gate 1 may create exactly this host tree:

```text
host/
  DeusOs.Control.sln
  global.json
  Directory.Build.props
  Directory.Packages.props
  src/
    DeusOs.Control.Core/
    DeusOs.Control.Transport.Windows/
    DeusOs.Control.Transport.Linux/
    DeusOs.Control.Cli/
    DeusOs.Control.Desktop/
  tests/
    DeusOs.Control.Core.Tests/
    DeusOs.Control.Transport.Tests/
```

Ownership:

### `DeusOs.Control.Core`

Owns:

- binary frame encoder/stream decoder;
- CRC-16 implementation matching firmware protocol v1;
- HELLO parser;
- RPC request/response aggregation;
- request-ID allocator;
- timeout/cancellation policy;
- `sysinfo` parser and capability model;
- typed health/rpcinfo/application models;
- connection/session state machine;
- transport-neutral device/session interfaces.

It must have no WinUSB, libusb, Avalonia or OS-specific dependency.

### `DeusOs.Control.Transport.Windows`

Owns:

- discovery by the accepted device-interface GUID;
- SetupAPI/CfgMgr32 enumeration as needed;
- WinUSB open/configuration;
- bulk EP4 OUT/IN I/O;
- Windows transport errors and cancellation mapping.

No custom kernel driver or custom INF is permitted. The already accepted inbox `winusb.inf` binding is authoritative.

### `DeusOs.Control.Transport.Linux`

Owns:

- libusb-1.0 enumeration of `VID=0x1209 PID=0x000C`;
- exact configuration/interface verification;
- claim of management interface 2 only;
- bulk EP4 OUT/IN I/O;
- Linux transport errors and cancellation mapping.

It must not detach or claim CDC interfaces 0/1.

### `DeusOs.Control.Cli`

Owns the reference host workflow and acceptance surface.

Initial commands:

```text
devices
info
ping
health
rpcinfo
apps
app start <id>
app stop
```

The CLI is the reference integration client. Hardware protocol acceptance must pass through it before the desktop UI can be accepted.

### `DeusOs.Control.Desktop`

Owns only human-facing desktop presentation and interaction.

Initial v1 surfaces:

- device list / connection state;
- system identity and source/build identity;
- platform/architecture/capability summary;
- health/reset/watchdog state;
- application list;
- start selected application;
- stop active application;
- reconnect/error state.

No transfer, update, network management, arbitrary command console, scheduler stress UI or destructive watchdog-trip button is part of the normal v1 desktop surface.

## 4. Transport discovery contract

### Windows

Discovery key:

`{C8B05EDE-1683-5002-81F0-95636B89CEC6}`

The host opens the management interface exposed through inbox WinUSB. It must not locate the product through COM-port enumeration.

### Linux

Discovery requires all of:

- VID `0x1209`;
- PID `0x000C`;
- one vendor management interface numbered `2`;
- interface class/subclass/protocol `FF/00/00`;
- EP4 OUT `0x04`, bulk, max packet 64;
- EP4 IN `0x84`, bulk, max packet 64.

The host claims only interface 2.

### Session locator versus unit identity

The accepted USB descriptor has no serial number.

Therefore v1 host discovery explicitly distinguishes:

- **session locator**: OS device path / Linux bus-port topology used to open one current device instance;
- **stable unit identity**: not available in this boundary.

The host may manage multiple simultaneously connected units by current session locator, but it must not persist that locator as a permanent physical-device identity.

MCU unique-ID exposure is **not** authorized in this boundary. A later requirement may add a privacy/stability-reviewed unit identity contract.

## 5. Binary protocol host contract

Binary framing remains protocol v1 unchanged.

The host decoder must treat every bulk read as an arbitrary byte chunk. USB reads are not frame boundaries.

The decoder must support:

- zero, partial, one or multiple complete frames per transport read;
- every split position across header/payload/CRC;
- bounded resynchronization according to the accepted binary protocol;
- CRC rejection;
- malformed length rejection;
- request-ID correlation;
- `RPC_DATA` aggregation followed by exactly one `RPC_END`;
- `PROTOCOL_ERROR` handling;
- transport disconnect at any byte boundary.

The initial host session permits **one in-flight RPC request per device**. This matches the current task0-serialized firmware execution model and avoids speculative host concurrency. Request correlation remains implemented correctly so the limit can be relaxed later without redesigning framing.

Request IDs:

- nonzero 16-bit values;
- monotonically advance modulo 65536;
- skip zero;
- a new connection session may restart allocation;
- responses with a different request ID must not complete the active request.

Normal RPC timeout baseline: 2 seconds.

Reconnect/discovery windows are separate from RPC timeout and may be longer.

## 6. Connection state machine

Required host states:

```text
DISCONNECTED
DISCOVERED
OPENING
NEGOTIATING
READY
RECOVERING
FAULTED
```

Successful connection sequence:

1. discover/open transport;
2. clear decoder/request state;
3. receive/obtain exact HELLO;
4. verify protocol v1;
5. verify command-service compatibility;
6. execute `sysinfo`;
7. validate mandatory identity/capability fields;
8. enter READY.

On physical disconnect:

- active request fails as transport-disconnected;
- decoder partial bytes are discarded;
- cached READY state becomes invalid;
- reconnect creates a fresh session;
- HELLO + `sysinfo` must run again before READY.

No application command may be issued merely because an old UI model says the device used to be connected.

## 7. System identity/capability firmware contract

The current HELLO and `rpcinfo` do not provide sufficient long-term system/build/platform identity.

Gate 1 therefore appends one safe zero-argument RPC:

```text
name      sysinfo
RPC ID    0x0024
class     SAFE
argc      0
```

Consequences:

- binary frame protocol remains **v1**;
- command-service foundation version advances **2 -> 3**;
- registry count advances **35 -> 36**;
- existing RPC IDs `0x0001..0x0023` remain unchanged;
- application runtime ABI remains **v1**.

### Exact `sysinfo` v1 output

The command emits these four CRLF-terminated lines in this order:

```text
SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M
SOURCE_TREE=<40 lowercase hex or UNBOUND>
PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001
CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000
```

Mandatory parser rules:

- required keys must appear exactly once;
- unknown additional keys are ignored for forward compatibility;
- malformed hexadecimal values fail negotiation;
- `OS_ID`, `PLATFORM_ID`, `ARCH_ID` are stable machine tokens, not localized labels;
- hardware acceptance forbids `SOURCE_TREE=UNBOUND`.

### Source/build identity binding

Firmware source may provide a development fallback `UNBOUND` when no build identity is supplied so local ad-hoc compilation remains possible.

Gate 2/3 acceptance builds must define `DEUS_FIRMWARE_SOURCE_TREE_HEX` as the exact 40-hex temporary-index **firmware-only candidate tree** at compile time. That tree is computed from the Gate-0 repository baseline plus only the authorized firmware source changes, excluding host/docs paths. The running target must return that exact value through `sysinfo`.

This value identifies the accepted firmware source candidate; it is not required to equal the later documentation/publication commit tree.

## 8. Capability bits

`CAPABILITIES` is a 32-bit versioned host-facing feature mask under `SYSINFO_ABI=1`.

Frozen bits:

```text
bit 0  0x00000001  SYSTEM_HEALTH
bit 1  0x00000002  APPLICATION_RUNTIME
bit 2  0x00000004  APPLICATION_CONTROL
bit 3  0x00000008  DIAGNOSTICS
bit 4  0x00000010  LOCAL_UI
bit 5  0x00000020  ASSET_CONFIGURATION_TRANSFER
bit 6  0x00000040  FIRMWARE_UPDATE
bit 7  0x00000080  NETWORK_SERVICES
```

Current accepted value for this boundary is `0x0000001F`.

Bits 5–7 must remain clear. The host must use capability bits to disable/hide unavailable future surfaces rather than infer support from product name or VID/PID.

## 9. Firmware source boundary for Gate 1

Authorized firmware additions/modifications:

```text
new      include/kernel/system_identity.h
new      src/kernel/system_identity.c
modify   include/kernel/command_service.h
modify   src/kernel/command_service.c
modify   src/kernel.c
```

No other firmware source path is authorized without reopening Gate 0.

One repository-support path is additionally authorized in Gate 1: `.gitignore`, only to add host-scoped generated-artifact exclusions (`host/**/bin/`, `host/**/obj/`, `host/**/TestResults/` and host IDE scratch if generated). It may not hide source, package manifests, lock files or acceptance evidence.

`host/global.json` owns the .NET 10 SDK major-line pin/roll-forward policy. `host/Directory.Packages.props` owns reviewed central package versions; per-project ad-hoc package-version drift is forbidden.

`system_identity` owns stable machine identity tokens, capability constants and `sysinfo` emission.

`command_service` owns the additive RPC descriptor/ID and service-version bump.

`kernel.c` may only route the new safe method to the system-identity owner. It must not regain identity-formatting logic or host-specific policy.

No new task, SVC, queue, mutex, timer, heap, filesystem, DMA, persistence mechanism, USB endpoint or USB descriptor change is authorized.

## 10. Firmware resource constraints

The new target identity service must be effectively read-only/static.

Frozen acceptance ceilings remain:

- Flash <= `54780` bytes;
- SRAM <= `11824` bytes;
- task stacks exactly `1024 / 512`;
- runtime task0/task1 margins >= `384 / 384` bytes;
- management RX/TX rings remain `512 / 1024`;
- management drops remain zero under accepted host pressure.

The identity implementation must not allocate writable persistent buffers merely to format `sysinfo`.

## 11. Host error model

Core exposes typed categories at minimum:

```text
Discovery
Open
TransportDisconnected
Timeout
Protocol
Crc
RequestCorrelation
RpcStatus
IncompatibleProtocol
IncompatibleService
InvalidSystemInfo
Cancelled
```

UI text may be human-friendly, but error classification belongs to Core.

Transport-native error numbers may be attached diagnostically without becoming the portable API.

## 12. RPC behavior exposed by v1 host application

Normal user-facing operations:

- `ping`;
- `health`;
- `rpcinfo`;
- `sysinfo`;
- `applist`;
- `appstart`;
- `appstop`.

Diagnostics may expose selected safe existing methods later inside this same boundary only after the Core API is typed.

Destructive `wdogtrip` remains excluded from normal UI. The presence of the existing protocol destructive-intent flag is not authentication and must not be presented as a security feature.

## 13. Desktop architecture

Desktop uses a small presentation layer over Core.

Required separation:

```text
Avalonia Views
    |
View Models / presentation state
    |
DeusOs.Control.Core
    |
IDeviceTransport
   / \
WinUSB  libusb
```

Views must never call native USB APIs directly.

No plugin loader is implemented in this boundary. Plugin architecture remains a later host feature after the stable Core API exists.

## 14. Security/trust position

This is a local USB management boundary.

No authentication protocol is added here.

CRC-16 protects framing integrity only.

The existing destructive flag expresses operator intent only.

Before networking or executable firmware update is exposed, a separate authorization/authenticity/trust boundary remains mandatory.

The host must not label an unauthenticated local USB session as authenticated or secure.

## 15. Acceptance order

### Gate 0 — contract/source-boundary freeze

Docs only.

Freeze:

- .NET 10/C#/Avalonia architecture;
- host source tree;
- Windows/Linux transport ownership;
- `sysinfo=0x0024`;
- command-service v3 / registry 36;
- identity fields and capability bits;
- session/reconnect/error semantics;
- firmware exact five-path source boundary;
- resource ceilings;
- Windows and Linux hardware acceptance requirements.

No source implementation.

### Gate 1 — source implementation/static review

Implement only the authorized host tree and five firmware paths.

Static review must prove layering and no duplicated protocol parser in CLI/Desktop.

### Gate 2 — fresh build/unit/resource acceptance

Firmware:

- clean full GNU build;
- exact source candidate identity bound into `sysinfo`;
- unchanged binary frame v1;
- Flash/SRAM/stacks within frozen ceilings.

Host:

- .NET 10 restore/build/test;
- Core golden-frame tests;
- arbitrary-chunk/split/CRC/malformed/request-correlation/reconnect tests;
- Windows and Linux transport projects compile;
- Desktop and CLI compile.

No target Flash in Gate 2.

### Gate 3 — Windows hardware/CLI acceptance

Using the exact accepted Gate-2 BIN and current board:

- automatic discovery by interface GUID;
- no COM-first discovery;
- HELLO exact;
- `sysinfo` exact and source tree matches Gate 2;
- ping/health/rpcinfo/apps;
- app start/stop lifecycle;
- 128 unique pings;
- malformed transport test vectors from Core where safe;
- physical USB reconnect and host automatic session recovery;
- management drops zero;
- final Flash exact.

### Gate 4 — Windows desktop acceptance

Desktop must:

- discover and connect without manual device-path entry;
- show exact identity/platform/source/capabilities;
- show health;
- show applications;
- start `device.info`;
- stop back to normal application behavior;
- represent disconnect/reconnect without stale READY state;
- use the same Core session as CLI.

No target source change is authorized merely to satisfy UI presentation.

### Gate 5 — Linux hardware/cross-platform acceptance

Required before claiming the host foundation fully cross-platform:

- `dotnet test` / Release build on Linux;
- libusb discovery of `1209:000C`;
- verify IF2 and EP4 exactly;
- claim IF2 only;
- HELLO + `sysinfo` + ping/health/apps;
- application start/stop;
- 128 unique pings;
- physical reconnect/recovery;
- CDC interfaces remain available/untouched;
- final target Flash exact.

If a real Linux host is unavailable, Gate 5 remains pending; a Windows-only result must not be described as full Windows/Linux acceptance.

### Gate 6 — documentation/final acceptance commit

Synchronize exact hashes/results, then one normal commit containing only accepted source/docs/host paths.

### Gate 7 — ordinary non-force publication

Fresh fetch, direct-parent proof, ordinary non-force push, fresh post-push fetch, clean ahead/behind `0/0`.

## 16. Explicit non-goals

Not part of this boundary:

- asset/configuration transfer;
- filesystem/persistent target configuration;
- firmware update or bootloader;
- network transport;
- authentication/authorization protocol;
- uploaded native target code;
- host plugin loader;
- stable physical unit identity/MCU UID exposure;
- telemetry subscriptions/streaming;
- scheduler/runtime-statistics framework;
- replacing UART/ST-LINK recovery paths;
- removing CDC diagnostics;
- changing OLED layout or local UI geometry.

## 17. Next boundary after publication

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

That boundary may begin only after this host foundation is published and the bounded persistence contract is frozen.
