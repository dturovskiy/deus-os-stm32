# STM32 OS — USB Development Identity Policy

Status: **PRIVATE DEVELOPMENT/TESTING POLICY — NOT A PRODUCT IDENTITY**

## Development profiles

The published native USB Device core foundation uses the historical profile:

```text
Boundary  NATIVE_USB_DEVICE_CORE_FOUNDATION
VID       0x1209
PID       0x000A
Product   Deus OS USB Core
Status    published / historical core profile
```

The published CDC ACM console boundary uses a distinct profile:

```text
Boundary  USB_CDC_ACM_CONSOLE_FOUNDATION
VID       0x1209
PID       0x000B
Product   Deus OS CDC Console
Status    published CDC profile / private-test identity
```

`0x000A`, `0x000B`, `0x000C`, `0x000D` and proposed `0x000E` are pid.codes private **Test PID** reservations under VID `0x1209`. They may be used only for private in-house testing and never for redistributed, sold or manufactured devices; they are not globally unique product identifiers. Naming an experimental PID here **does not** constitute Gate-0 approval or authorization to flash it.

Authoritative allocation references:

- `https://pid.codes/1209/000A/`
- `https://pid.codes/1209/` (Test PID list including `0x000B`, `0x000C`, `0x000D` and `0x000E`)
- `https://pid.codes/1209/000E/` (official private Test PID confirmation for the proposed Management-only candidate)

USB-IF remains the authority for USB Vendor IDs. This project does not claim ownership of VID `0x1209`; it uses only the private-test permission granted by that VID owner for these Test PIDs.

## Why the CDC boundary uses a different PID

The CDC ACM descriptor/class topology and Windows driver binding differ materially from the published vendor-specific core profile. Reusing `1209:000A` could cause host-side device-node/descriptor/driver cache ambiguity during development. `1209:000B` gives the CDC profile a clean private-test identity while preserving the published `000A` evidence unchanged.

## USB management-device development profile

The `USB_MANAGEMENT_DEVICE_FOUNDATION` uses a third, distinct private-test topology identity:

```text
Boundary  USB_MANAGEMENT_DEVICE_FOUNDATION
VID       0x1209
PID       0x000C
Product   Deus OS Device
Status    private-test composite CDC + WinUSB management profile
```

The new PID is required because the host-visible topology changes from CDC-only to a composite device with a vendor-specific WinUSB management interface. Reusing `0x000B` would risk Windows device-node/driver-cache ambiguity.

The stable management device-interface GUID is not the USB product identity. It is a host discovery ABI for the WinUSB management interface:

`{C8B05EDE-1683-5002-81F0-95636B89CEC6}`

## Accepted Gate-0 Management-only Test PID design — **published, NOT installed**

The **owner-approved and Gate-0-published** `MANAGEMENT_ONLY_RUNTIME_MIGRATION` scoped design selects **VID:PID `1209:000E`** for a **private test-only** runtime with a single vendor-specific Management **IF0 `FF/00/00`**, EP `0x04` bulk OUT / `0x84` bulk IN (64 B), and **no CDC ACM interfaces/IAD**. The existing physical runtime remains published composite `1209:000C` Management IF2 + CDC IF0/IF1; the independent published bootloader remains `1209:000D`. The new candidate preserves the established runtime Management discovery GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`, while matching Microsoft OS 2.0 WINUSB function-subset numbering to IF0. The new PID prevents accidental OS driver/cache reuse of the different composite topology; the host must reject mismatched VID/PID/interface/endpoint identities and must not confuse bootloader IF0 with runtime IF0.

**Identity provenance:** pid.codes expressly lists [1209/000E as a Test PID](https://pid.codes/1209/000E/) reserved for in-house/private experiments. The published [Test PID usage restriction](https://pid.codes/pids/) excludes manufactured, redistributed and sold devices; it is not an assigned project/product identity. **This section records an owner-approved Gate-0 design published in `7f73ebd89eabfcc515331c6985cf1e9a32ed9d09` with [exact-SHA CI 39452032151](https://github.com/dturovskiy/deus-os-stm32/actions/runs/39452032151) SUCCESS**, not installed runtime behavior, Windows driver installation, Gate-4 approval or distribution authorization. The private-test identity is frozen **for isolated offline candidate qualification only**; any physical firmware write requires a separate authorized Gate 4. Existing `000C` and `000D` allocations and historical acceptance remain unchanged.

## Bootloader development profile

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` froze and now publishes a fourth private-test identity because the bootloader topology is a single vendor/bulk interface rather than the runtime composite device:

```text
Boundary  FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION
VID       0x1209
PID       0x000D
Product   Deus OS Bootloader
Status    published Firmware Update / Bootloader private-test vendor-bulk profile
```

Using a distinct PID prevents the runtime `000C` composite CDC/management descriptor and Windows function-binding history from being reused for the bootloader's different interface-0 topology. Windows bootloader binding is WinUSB through its own Microsoft OS 2.0 descriptor; Linux uses libusb.

The stable bootloader device-interface GUID is a separate host discovery ABI from the runtime management IF2 GUID:

`{F08907B7-BEC4-5FCF-BC4C-B446ED345D87}`

The bootloader Microsoft OS 2.0 descriptor set publishes that value through the `DeviceInterfaceGUIDs` `REG_MULTI_SZ` property for interface 0. Windows bootloader discovery and the descriptor must match exactly.

## Hard restrictions

- private bench/development testing only;
- no redistribution of firmware/devices configured with these Test PIDs as a product identity;
- no sale or manufacturing with these identities;
- no claim that any private-test profile is globally unique;
- no use of ST, ST-LINK, Arduino, or unrelated third-party VID/PID values;
- production/release identity requires a separately authorized VID/PID before release.

## Descriptor policy

VID/PID constants must be centralized in the native USB descriptor/policy source and match the active boundary documented here exactly.

Product strings are development identifiers, not stable public product identities.

A boundary that changes USB class topology in a way that changes host binding should use an explicitly documented development profile rather than silently reusing a previous profile.

## Gate effect

This document permits **private testing for accepted/published development profiles only**; it does not independently authorize proposed Gate-0 or Gate-4 changes. The historical CDC profile remains `VID=0x1209`, `PID=0x000B`, product `Deus OS CDC Console`. The accepted/published `USB_MANAGEMENT_DEVICE_FOUNDATION` uses `VID=0x1209`, `PID=0x000C`, product `Deus OS Device`. The published `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` uses `VID=0x1209`, `PID=0x000D`, product `Deus OS Bootloader`. Gate-0-published test design `1209:000E` remains **NOT FLASHED / NOT PHYSICALLY ACCEPTED / GATE4 NOT AUTHORIZED**. No Test PID authorizes production/distribution use.
