# Deus OS — Host Control Application Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `e0f49f168542fa1cf49bca451e01b0c077aa8d18`**

Boundary:

`HOST_CONTROL_APPLICATION_FOUNDATION`

Canonical design:

`docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`

## 1. Gate 0 accepted baseline

Repository prestate at Gate 0 freeze:

- `HEAD == origin/main == b51e1b144bfbcb93ce1e32e5cfa33a8ccecacf1e`;
- working tree clean;
- USB-management firmware publication `1f88083843c6aae9fd228ad2d677f9252b889a11`;
- hardware-accepted candidate tree `46841b52d351277deb134a6f4709619087b477af`;
- accepted BIN SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`;
- binary protocol v1;
- command service v2 / 35 methods;
- application runtime ABI v1;
- Windows WinUSB management binding accepted;
- current target Flash exact to the accepted BIN.

Gate 0 is architecture/docs only. No firmware/host source implementation is part of Gate 0.

## 2. Gate 0 frozen decisions

Acceptance requires the canonical plan to freeze all of:

- C# / .NET 10 host baseline;
- Avalonia 12.x desktop shell;
- protocol-independent Core project;
- Windows direct WinUSB adapter;
- Linux libusb-1.0 adapter;
- CLI as reference integration client;
- Desktop consuming the same Core API;
- one in-flight RPC per device;
- arbitrary bulk-read chunk decoding;
- reconnect as a fresh HELLO + `sysinfo` session;
- no stable physical unit identity in v1;
- `sysinfo=0x0024`;
- command-service version `3`;
- registry count `36`;
- binary protocol remains `1`;
- exact `sysinfo` fields/order;
- capability mask ABI;
- exact firmware source boundary;
- exact host source tree;
- unchanged target USB descriptors/endpoints/rings;
- no transfer/update/network/plugin work.

## 3. Exact Gate 1 source boundary

Firmware allowed paths only:

```text
include/kernel/system_identity.h
src/kernel/system_identity.c
include/kernel/command_service.h
src/kernel/command_service.c
src/kernel.c
```

Host allowed tree only:

```text
host/DeusOs.Control.sln
host/global.json
host/Directory.Build.props
host/Directory.Packages.props
host/src/DeusOs.Control.Core/**
host/src/DeusOs.Control.Transport.Windows/**
host/src/DeusOs.Control.Transport.Linux/**
host/src/DeusOs.Control.Cli/**
host/src/DeusOs.Control.Desktop/**
host/tests/DeusOs.Control.Core.Tests/**
host/tests/DeusOs.Control.Transport.Tests/**
```

Canonical docs may be updated as acceptance evidence requires. Gate 1 may also modify `.gitignore` only for host-scoped generated-artifact exclusions; source/package/lock/evidence files must remain visible to Git.

Any additional firmware source path requires Gate 0 reopening before implementation.

## 4. Static architecture checks

Gate 1 must prove:

- Core has no Avalonia reference;
- Core has no direct Windows/Linux native USB call;
- CLI has no frame decoder/CRC implementation;
- Desktop has no frame decoder/CRC/native USB implementation;
- Windows native interop is isolated to Windows transport project;
- Linux native interop is isolated to Linux transport project;
- target `system_identity` owns machine identity strings/capability mask;
- target `kernel.c` only routes the method;
- no new target task/SVC/queue/mutex/timer/heap/filesystem/persistence/DMA;
- no USB descriptor, endpoint, PMA or ring-size mutation.

## 5. `sysinfo` exact acceptance

RPC:

```text
ID=0x0024
NAME=sysinfo
CLASS=SAFE
MIN_ARGS=0
MAX_ARGS=0
```

Expected lines:

```text
SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M
SOURCE_TREE=<exact Gate-2 firmware candidate tree>
PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001
CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000
```

Gate 2/3 reject:

- missing/duplicate mandatory field;
- `SOURCE_TREE=UNBOUND`;
- source-tree mismatch;
- service version other than 3;
- registry count other than 36;
- protocol version other than 1;
- unexpected capability bits 5–7 set;
- nonzero `UNIT_ID_KIND`.

Existing RPC IDs `0x0001..0x0023` must remain unchanged.

## 6. Host Core unit acceptance

Required deterministic tests:

- CRC known vectors match firmware;
- encode one request frame exactly;
- decode one complete frame;
- decode byte-at-a-time;
- decode every two-chunk split position;
- decode multiple frames in one input chunk;
- retain partial frame across reads;
- reject bad CRC without completing a request;
- reject malformed/oversized frame;
- correlate only matching nonzero request ID;
- aggregate multiple `RPC_DATA` frames;
- terminate exactly on matching `RPC_END`;
- map `PROTOCOL_ERROR`;
- timeout active request;
- cancellation active request;
- disconnect clears partial decoder state;
- reconnect does not reuse old READY state;
- request-ID wrap skips zero;
- `sysinfo` parser requires mandatory keys once;
- `sysinfo` parser ignores unknown future keys;
- capability mask maps known bits exactly.

No hardware is needed for these tests.

## 7. Gate 2 firmware build/resource acceptance

Fresh GNU build must prove:

- all C translation units compile;
- startup/link/objcopy succeed;
- undefined symbol count zero;
- forbidden heap/libc dependency check clean;
- stack-usage artifacts complete;
- binary protocol v1;
- command service v3 / registry 36;
- exact `sysinfo` strings/capability constants in candidate;
- `DEUS_FIRMWARE_SOURCE_TREE_HEX` build binding equals the temporary-index firmware-only candidate tree computed from the Gate-0 baseline plus only authorized firmware source paths;
- Flash <= `54780`;
- SRAM <= `11824`;
- task stacks exactly `1024/512`;
- real Git index untouched;
- no Flash.

Candidate BIN/ELF/MAP hashes and sizes become authoritative for Gates 3–5.

## 8. Gate 2 host build acceptance

Required:

- `dotnet --info` records .NET 10 SDK/runtime;
- restore succeeds;
- Release build succeeds;
- all unit tests pass;
- Windows transport project builds;
- Linux transport project builds;
- CLI builds;
- Desktop builds;
- Core test coverage includes every protocol condition in section 6;
- no generated secrets or machine-specific absolute paths enter the candidate.

NuGet lock/central package version policy must make package resolution reviewable and repeatable.

## 9. Windows Gate 3 preflight

Before target mutation:

- current repo prestate exact;
- accepted Gate-2 firmware candidate identity exact;
- accepted Gate-2 host candidate identity exact;
- Windows management interface present;
- interface 2 service = `WINUSB`;
- no custom INF;
- exact management GUID discoverable;
- non-Deus UART adapter enumerated if UART regression pressure is part of the run;
- target normally powered by micro-USB;
- SWD readable;
- target Flash read first.

If Flash already equals Gate-2 BIN, do not reflash.

If programming is required, backup first, program exact once, verify exact.

## 10. Windows CLI Gate 3 runtime acceptance

The CLI must prove through the product Core/Windows transport:

- `devices` discovers by management GUID;
- no COM-port path is used for primary discovery;
- open management interface;
- exact HELLO: protocol 1, service 3, registry 36;
- exact `sysinfo`;
- returned source tree equals Gate-2 candidate;
- `ping` request-ID echo;
- `health`;
- `rpcinfo`;
- `applist` exactly two applications;
- start `device.info`;
- repeated start remains idempotent according to target contract;
- stop returns to normal application behavior;
- 128 unique sequential pings;
- management RX/TX drops remain zero;
- CDC diagnostics still enumerate;
- scheduler/application faults remain zero;
- task margins >= `384/384`.

## 11. Windows reconnect acceptance

Perform one physical board micro-USB disconnect/reconnect while leaving ST-LINK/UART wiring untouched.

Host requirements:

- current session transitions out of READY;
- active operation fails as transport-disconnected, not protocol corruption;
- old partial decoder/request state cleared;
- device rediscovered without manual path entry;
- WinUSB reopened;
- HELLO reruns;
- `sysinfo` reruns;
- READY only after both succeed;
- subsequent ping/health/apps succeed.

No target reflash due solely to reconnect.

## 12. Windows desktop Gate 4 acceptance

Desktop must visibly and functionally prove:

- device appears from discovery;
- connection state is accurate;
- exact OS/platform/arch/source tree displayed;
- capability-supported surfaces enabled;
- future transfer/update/network surfaces absent or disabled;
- health visible;
- app list visible;
- `system.home` / `device.info` states represented correctly;
- start `device.info`;
- stop active app;
- disconnect is shown without stale READY;
- reconnect restores live state after fresh negotiation;
- errors come from typed Core classifications.

Desktop must not expose `wdogtrip` as an ordinary action.

## 13. Linux Gate 5 acceptance

A real Linux host is required before the boundary can claim full cross-platform hardware acceptance.

Required:

- .NET 10 Release build/tests pass on Linux;
- libusb-1.0 available;
- discover `1209:000C`;
- verify exact management IF2/EP4 topology;
- claim IF2 only;
- do not detach CDC IF0/1;
- HELLO exact;
- `sysinfo` exact/source tree exact;
- ping/health/rpcinfo/apps;
- application start/stop;
- 128 unique pings;
- management drops zero;
- physical USB reconnect/recovery;
- CDC remains available;
- final Flash exact Gate-2 BIN.

If Linux hardware execution is unavailable, record `GATE5_ENVIRONMENT_BLOCKED`; do not convert Windows success into Linux PASS.

## 14. Host multi-device behavior

Tests must allow two synthetic/mock device sessions concurrently.

Core state, request IDs, decoder bytes and cancellation must be per-device/session.

Because no stable unit identity exists:

- UI may distinguish simultaneous devices by current session locator;
- persistence across USB-port moves/reboots is forbidden;
- no serial-number fabrication;
- no hash of OS device path presented as physical identity.

## 15. Regression requirements

Target regression retention:

- CDC diagnostics;
- UART emergency path;
- WinUSB management;
- application lifecycle;
- IWDG ownership;
- scheduler canaries/fault counters;
- local OLED/UI behavior;
- exact Flash final readback.

No physical OLED observation is required if firmware changes only add `sysinfo` and do not alter local UI rendering. In that case Gate-4 target disposition remains:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

## 16. Failure classification

At minimum distinguish:

```text
HARNESS_OR_BUILD_FAILURE
HOST_BUILD_FAILURE
HOST_UNIT_TEST_FAILURE
WINDOWS_DISCOVERY_FAILURE
LINUX_DISCOVERY_FAILURE
TRANSPORT_OPEN_FAILURE
PRODUCT_PROTOCOL_FAILURE
SYSTEM_IDENTITY_FAILURE
PRODUCT_RUNTIME_FAILURE
HOST_RECONNECT_FAILURE
HOST_UI_FAILURE
RESOURCE_BUDGET_FAILURE
ENVIRONMENT_BLOCKED
```

Classified gate FAIL must still finalize trustworthy logs/evidence and terminate normally. Process-level failure is reserved for inability to produce trustworthy evidence/package/prestate.

Gate 5 accepted result entering Gate 6:

- firmware candidate tree `b895955f7738aceb6fca0272d510cc433378c6ab`;
- host candidate tree `2c5afd9914851300aed15e321cf69c3a2c3daeed`;
- BIN/ELF/MAP sizes `50652 / 483488 / 207952`;
- BIN/ELF/MAP SHA-256 `FB68993FC998DE77B61FAF9F4949E4E124B95FF867BB456EBA401C9F2709F13F` / `978CC710497F79E9A3AFBAFE3E0CD3AE10BA2B6CF0FE6AA7518B7AC5BC5E524C` / `F9806E2A71476D6D2DAC2CD481A6C491BACD18CADD713813BB69AF9DA608C032`;
- Flash/SRAM `50652/11728`, ceilings `54780/11824`, task stacks `1024/512`, task margins `424/424`, scheduler faults `0`;
- .NET target `net10.0`, Windows SDK `10.0.201`, Linux SDK `10.0.112`, Avalonia `12.1.2`, xUnit MTP `4.0.1`;
- Core `21/21`; Transport `5/5`;
- Windows Gate 3 hardware/CLI acceptance evidence/log `50CB913F5E09CBF0531583B109B3650899AC39209A6F172DAD87B6EAD3C0C139` / `863F482CE40311E82F4C9D92B6DFD877DDE6024EFAD7C022CBF6B0F2037D70AA`;
- malformed-vector amendment evidence/log `5C537438A2353621A627708A7F15ECCBBD0A60BD23ABF4116326AEC2B535362` / `020796469F67878FD13763D222B966BA05C9F8BF5DE9D0FBEE5BA497981ACC4C`;
- Windows Gate 4 Desktop evidence/log `8D58EAE457C2A2DCD19B581D445CA192D9F190CF2BA8A34FF95CC2C3B8B9E8EC` / `58AD1BA2B6AC95C1BA85FC088ACF1527721018BF364EBA04F68C51255CB53A88`;
- Linux Gate 5 runtime evidence/log `1FAAFDFE40B34D20D69AE59369CABBCCDB07567A276B2E3ADAA7B819335F379D` / `00E3310932CC2115B8728FAD4484B3D2FCCA698304C2C4186B7F5BE52DC75218`;
- final Gate 5 composite evidence/log `564BF507C54D802ACD590012384A6FA2BE7483C40F94654802C5316071702BFE` / `BFF093889489077BBF60B97E083D34082ECF8AE40322E2710EA2B77FF2893591`;
- Windows/Linux reconnect PASS with fresh negotiation; Linux enumeration address changed `007 -> 008` while stable locator remained `usb:001:8`;
- final management/CDC RX/TX drops all zero; USB error/PMA counters zero; final Flash exact accepted Gate-2 BIN;
- `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.
## 17. Gate 6 documentation finalization

After Windows/Linux acceptance, update canonical docs with exact:

- firmware candidate tree;
- BIN/ELF/MAP hashes and sizes;
- host source candidate/commit identity;
- .NET/Avalonia package versions;
- Core test counts;
- Windows runtime results;
- Linux runtime results;
- system identity/capability values;
- reconnect results;
- target margins/drops/faults;
- target Gate-4 OLED disposition.

No source mutation during finalization.

## 18. Gate 6 local acceptance commit phase

Requirements:

- exact accepted firmware + host + docs set only;
- `git diff --cached --check` clean;
- no unstaged/untracked residue;
- source equals hardware-accepted candidate;
- one normal local commit;
- no amend drift after hardware acceptance.

## 19. Gate 7 publication

Before push:

- fresh `git fetch origin main`;
- prove remote/FETCH_HEAD equals direct parent;
- clean repo;
- ahead/behind `1/0`.

Publish by ordinary non-force push only.

After push:

- fresh fetch;
- `HEAD == origin/main == FETCH_HEAD`;
- clean;
- ahead/behind `0/0`.

Next boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`
