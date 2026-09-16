# STM32 OS — USB Development Identity Policy

Status: **GATE 1 DEVELOPMENT POLICY — PRIVATE TESTING ONLY**

Boundary:

`NATIVE_USB_DEVICE_CORE_FOUNDATION`

## Assigned development identity

The first native USB Device core uses:

```text
VID = 0x1209
PID = 0x000A
```

This pair is the `pid.codes` private **Test PID** allocation.

The allocation states that anyone may use this VID/PID for private testing, but
it **must not** be used on a device that is redistributed, sold, or
manufactured. It is not globally unique.

Authoritative allocation page:

`https://pid.codes/1209/000A/`

USB-IF remains the authority for USB Vendor IDs. This development policy does
not claim ownership of VID `0x1209`; it uses only the private-test permission
granted by that VID owner for PID `0x000A`.

## Hard restrictions

- private bench/development testing only;
- no redistribution of firmware or devices configured with this identity as a
  product identity;
- no sale or manufacturing with this identity;
- no claim that this identity is globally unique;
- no use of ST, ST-LINK, Arduino, or any unrelated third-party VID/PID;
- production/release identity requires a separately authorized VID/PID before
  release.

## Descriptor policy

The VID/PID constants are centralized in the native USB device-core source and
must match this document exactly.

The initial product string is development-only and must not be treated as a
stable public product identity.

## Gate effect

This policy resolves the Gate 1 requirement that USB identity be explicit
before source mutation. It authorizes only private enumeration testing for the
`NATIVE_USB_DEVICE_CORE_FOUNDATION` boundary.
