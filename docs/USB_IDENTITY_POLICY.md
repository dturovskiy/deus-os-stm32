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

The next CDC ACM console boundary uses a distinct profile:

```text
Boundary  USB_CDC_ACM_CONSOLE_FOUNDATION
VID       0x1209
PID       0x000B
Product   Deus OS CDC Console
Status    hardware accepted CDC profile / publication pending
```

Both `0x000A` and `0x000B` are pid.codes private **Test PID** allocations under VID `0x1209`. They may be used for private testing only; they must not be used for redistributed, sold, or manufactured devices and are not globally unique product identities.

Authoritative allocation references:

- `https://pid.codes/1209/000A/`
- `https://pid.codes/1209/` (Test PID list including `0x000B`)

USB-IF remains the authority for USB Vendor IDs. This project does not claim ownership of VID `0x1209`; it uses only the private-test permission granted by that VID owner for these Test PIDs.

## Why the CDC boundary uses a different PID

The CDC ACM descriptor/class topology and Windows driver binding differ materially from the published vendor-specific core profile. Reusing `1209:000A` could cause host-side device-node/descriptor/driver cache ambiguity during development. `1209:000B` gives the CDC profile a clean private-test identity while preserving the published `000A` evidence unchanged.

## Hard restrictions

- private bench/development testing only;
- no redistribution of firmware/devices configured with either Test PID as a product identity;
- no sale or manufacturing with these identities;
- no claim that either profile is globally unique;
- no use of ST, ST-LINK, Arduino, or unrelated third-party VID/PID values;
- production/release identity requires a separately authorized VID/PID before release.

## Descriptor policy

VID/PID constants must be centralized in the native USB descriptor/policy source and match the active boundary documented here exactly.

Product strings are development identifiers, not stable public product identities.

A boundary that changes USB class topology in a way that changes host binding should use an explicitly documented development profile rather than silently reusing a previous profile.

## Gate effect

This document authorizes private testing only. For `USB_CDC_ACM_CONSOLE_FOUNDATION`, Gate 1 source mutation may use `VID=0x1209`, `PID=0x000B`, product `Deus OS CDC Console`; it does not authorize any production/distribution use.
