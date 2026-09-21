# Deus OS — Flash Ownership / Memory Map Decision

Status: **CANONICAL DOCS-ONLY ARCHITECTURE DECISION — NO FLASH/LINKER/STARTUP MUTATION AUTHORIZED BY THIS FILE**

Decision ID:

`FLASH_OWNERSHIP_LAYOUT_DECISION_V1`

## 1. Purpose

This decision freezes the future physical ownership of the 64 KiB internal Flash on the accepted STM32F103C8 target before either persistent Asset/Configuration writes or a firmware-update bootloader is implemented.

It exists to prevent Asset/Configuration persistence from consuming pages that a future recovery/update path needs and to prevent the future bootloader from forcing an ad-hoc migration of already-persisted configuration.

This document is a shared architecture contract. It is not an implementation boundary and does not itself authorize erase/program operations, linker relocation, vector relocation, bootloader code or target mutation.

## 2. Authoritative hardware basis

Accepted physical-board preflight already proves:

- STM32 device ID `0x410`;
- factory Flash size `64 KiB`;
- RDP disabled;
- write protection inactive.

ST hardware documentation retained as the silicon basis for this decision:

- STM32F103C8 / DS5319: the x8 device provides 64 KiB Flash and 20 KiB SRAM;
- RM0008, embedded Flash organization for medium-density devices: main Flash erase-page size is 1 KiB;
- PM0056, Cortex-M3 vector table / `SCB_VTOR`: reset begins with the reset vector table at address zero; privileged software may relocate the vector table, and `TBLOFF[8:0]` must be zero.

The selected application origin below is therefore both Flash-page aligned and stricter than the Cortex-M3 VTOR alignment requirement.

## 3. Current state versus future owned layout

The current accepted firmware remains a standalone image linked from:

```text
FLASH ORIGIN = 0x08000000
FLASH LENGTH = 64 KiB
```

and currently owns the reset vector at the beginning of Flash.

This decision does **not** retroactively change that accepted binary.

The ownership target in Section 4 is reached in **two implementation phases**. Asset/Configuration may reserve and use the top A/B persistence pages while the application remains the standalone reset owner at `0x08000000`. The application moves to `0x08002000` only when a real bootloader is installed at Flash base. This staged rule prevents an Asset-first implementation from producing an unbootable device.

## 4. Frozen future physical map

STM32F103C8 internal Flash:

```text
0x08000000  +----------------------------------+
            | Bootloader / recovery            |
            | pages 0..7                       |
            | 8 KiB                            |
0x08002000  +----------------------------------+
            | Application                      |
            | pages 8..61                      |
            | 54 KiB maximum                   |
            |                                  |
            | current 50652-byte image would   |
            | leave 4644 bytes inside this     |
            | application region after         |
            | relocation                       |
0x0800F800  +----------------------------------+
            | Persistent config slot A         |
            | page 62 / 1 KiB                  |
0x0800FC00  +----------------------------------+
            | Persistent config slot B         |
            | page 63 / 1 KiB                  |
0x08010000  +----------------------------------+  exclusive end
```

Exact ownership:

| Region | Start | End inclusive | Size | Erase pages |
| --- | --- | --- | ---: | --- |
| Bootloader / recovery | `0x08000000` | `0x08001FFF` | 8192 B | 0..7 |
| Application | `0x08002000` | `0x0800F7FF` | 55296 B | 8..61 |
| Persistent slot A | `0x0800F800` | `0x0800FBFF` | 1024 B | 62 |
| Persistent slot B | `0x0800FC00` | `0x0800FFFF` | 1024 B | 63 |

No region may erase or program a page owned by another region.

### 4.1 Phased realization before the bootloader exists

The final map above is the steady-state ownership target, but the roadmap intentionally implements persistence before the bootloader.

Therefore the Asset/Configuration phase uses this transitional physical execution map:

```text
0x08000000  +----------------------------------+
            | Standalone application           |
            | reset owner                       |
            | maximum linked span: 54 KiB      |
0x0800D800  +----------------------------------+
            | Reserved relocation headroom     |
            | pages 54..61 / 8 KiB             |
0x0800F800  +----------------------------------+
            | Persistent config slot A / 1 KiB |
0x0800FC00  +----------------------------------+
            | Persistent config slot B / 1 KiB |
0x08010000  +----------------------------------+
```

During this phase:

- the application vector table remains at `0x08000000`;
- no bootloader is implied or fabricated;
- the application linker may remain at origin `0x08000000`, but its Flash length/ceiling becomes **54 KiB**, ending before `0x0800D800`;
- pages 54..61 remain deliberately unused as relocation headroom;
- pages 62/63 are the only self-programmable Asset/Configuration pages;
- the current 50652-byte application fits the 54 KiB ceiling with 4644 bytes remaining.

When `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` is later implemented, the same 54 KiB application budget shifts upward by exactly 8 KiB:

```text
standalone phase: application 0x08000000..0x0800D7FF
bootloader phase: application 0x08002000..0x0800F7FF
```

Only that bootloader phase changes reset ownership, application linker origin and VTOR/handoff semantics. Persistence pages do not move.

## 5. Why the bootloader reservation is 8 KiB

The 8 KiB value is a **budget ceiling derived from the current 64 KiB device constraint**, not an assertion that an unimplemented bootloader already has a proven linked size.

Page accounting:

- physical Flash: `64` pages;
- current accepted application: `50652` bytes -> `50` 1-KiB pages when rounded up;
- persistent atomic A/B storage: `2` pages;
- explicit application growth reserve before the persistence boundary is considered boxed in: `4` pages;
- remaining lower pages available for reset/recovery ownership: `64 - 50 - 2 - 4 = 8` pages.

Therefore:

```text
bootloader/recovery ceiling = 8 KiB
application region          = 54 KiB
persistent region           = 2 KiB
```

The future `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` Gate 0 must produce a static linked-size feasibility proof against this 8 KiB ceiling **before implementation is authorized**.

If the minimum accepted update/recovery/security responsibility cannot fit in 8 KiB, this decision must be explicitly reopened. The project must then choose among reducing application footprint, reducing another reserved budget, changing the update architecture, adding external staging storage, or changing target hardware. Silent region encroachment is forbidden.

## 6. Minimum future bootloader responsibility model

The reservation is for a deliberately small recovery/update owner, not a second operating system.

The future bootloader is expected to own only responsibilities necessary to make executable update safe and recoverable:

1. own reset at `0x08000000`;
2. distinguish normal boot from recovery/update entry;
3. validate that an application candidate is structurally bootable before handoff;
4. enforce the future executable-image integrity/authenticity/target/version contract;
5. provide the minimum accepted update transport needed by the future update boundary;
6. erase/program/verify **application pages only**;
7. remain recoverable when application programming is interrupted or the application is invalid;
8. hand off deterministically to the application at `0x08002000`.

The bootloader must not own:

- OLED presentation;
- the production scheduler/application runtime;
- the general Host Control feature surface;
- a filesystem/package manager;
- configuration semantics;
- application UI policy;
- unrelated diagnostics.

Exact transport framing, update image format, cryptographic/authenticity mechanism and boot-validity metadata remain future `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` design work.

## 7. Reset and vector ownership

After bootloader implementation:

- the bootloader vector table remains at Flash base and is the reset owner;
- the application vector table begins at `0x08002000`;
- the application linker origin must become `0x08002000`;
- the application must not assume vectors remain at `0x08000000`;
- bootloader handoff must establish the application vector-table/MSP/reset-handler contract;
- application startup must establish/verify `SCB_VTOR = 0x08002000` before normal interrupt-dependent runtime proceeds.

The current `src/startup.s` does not perform that relocation, so this is a future implementation change and must be accepted together with the linker migration.

No application image linked for `0x08000000` may be treated as bootloader-layout compatible merely because its bytes otherwise validate.

## 8. Persistence ownership

Pages 62 and 63 are reserved as the generic bounded persistent A/B transaction substrate.

They are intentionally separate erase pages:

```text
slot A = page 62
slot B = page 63
```

The reopened Asset/Configuration Gate 0 must freeze the exact record envelope, generation/selection rules, integrity field, commit validity and power-loss behavior before either page is programmed.

Required recovery result after reset at any transaction point remains exactly one of:

- newly committed valid record;
- previous valid record;
- compiled default when no valid persistent record exists.

Half-valid/ambiguous authority is forbidden.

The bootloader must not reuse these pages for firmware-update staging or bootloader metadata without reopening this shared decision.

## 9. Application-region invariants

The application budget is **54 KiB in both phases**.

### Asset/Configuration phase — standalone reset owner

Before a bootloader exists:

```text
ORIGIN = 0x08000000
LENGTH = 54K
```

The build must fail if application loadable sections exceed `0x0800D800`. Pages 54..61 remain unused relocation headroom; pages 62/63 remain persistence.

### Bootloader phase — relocated application

After the bootloader becomes reset owner:

```text
ORIGIN = 0x08002000
LENGTH = 54K
```

The build must fail if application loadable sections exceed `0x0800F800`.

The relevant linker migration in each phase must add build-failing assertions that prevent:

- application overlap into persistence pages;
- use of the reserved relocation-headroom pages during the standalone Asset phase;
- application overlap into bootloader pages after relocation;
- persistence symbols outside pages 62..63.

The repository-owned reproducible build entrypoint remains authoritative and must be updated/re-accepted at each linker-contract transition.

## 10. Programming ownership rules

Future self-programming code must operate fail-closed:

- Asset/Configuration may erase/program only pages 62..63;
- before bootloader installation, the standalone application may execute from lower Flash but its self-programming path still owns only pages 62..63;
- after bootloader installation, Bootloader/update may erase/program only pages 8..61;
- normal application self-programming code may never erase/program pages 0..7;
- no boundary may perform mass erase as part of normal product operation;
- ST-LINK remains recovery/debug tooling outside normal self-programming ownership.

Any test that intentionally violates these ownership ranges must be a separately authorized destructive acceptance diagnostic and must not ship as a normal product path.

## 11. Sequencing unlocked by this decision

The product dependency order becomes:

```text
accepted runtime / USB / RPC / host / reproducible-build substrate
        |
        v
FLASH_OWNERSHIP_LAYOUT_DECISION_V1
        |
        v
concrete bounded persistent consumer
        |
        v
ASSET_CONFIGURATION_TRANSFER_FOUNDATION Gate 0 reactivation
        |
        v
Asset/Configuration implementation + acceptance
        |
        v
FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION
        |
        v
network/security/remote management
```

Host-only packaging, CLI distribution and local-only Web presentation remain orthogonal unless they introduce target Flash mutation or remote trust exposure.

## 12. Change rule

This map is intentionally conservative and page-granular.

Changing any of the following requires an explicit new architecture decision before implementation:

- bootloader region size or origin;
- application origin or maximum end;
- number/location of persistent pages;
- page ownership;
- reset-owner model.

A future change must include migration consequences for already-installed firmware and already-persisted configuration.

## 13. Non-goals

This decision does not:

- implement or link a bootloader;
- move the current application;
- modify `SCB_VTOR`;
- add Flash program/erase code;
- define executable update authenticity;
- define the Asset transfer wire protocol;
- write persistent data;
- flash/reset/reconnect the target.

It freezes ownership so later boundaries can be designed against one physical truth.
