# Deus OS — Host Dead-Surface Cleanup Plan

Status: **CLOSED / PUBLISHED `09a432f6c0b73ef2425add5950b6f6d5dee3733d` — GATES 0–5 ACCEPTED**

Boundary ID:

`HOST_DEAD_SURFACE_CLEANUP`

Residual-debt ID:

`RDC-02`

Baseline:

`bc562f5030990d4b4e69af35d0ad1a3e4cb301bb`

## 1. Purpose

Remove only re-proven unreachable Host transport remnants left by the accepted runtime/bootloader Linux USB profile migration, without deleting compatibility/operator/protocol APIs merely because their current call count is low.

This is a Host-only maintainability boundary. It changes no behavior, wire protocol, discovery profile, target firmware, scripts, Flash or hardware state.

## 2. Historical ownership proof

The original Host Control foundation introduced the runtime-only Linux libusb constants/helpers.

Firmware Update / Bootloader publication commit `27fb10288ef45dcc9292287603e5ab8a26bf1fcb` generalized the Linux adapter around `LinuxUsbProfile` so runtime and bootloader share:

- profile-aware `EnumerateCandidates(profile)`;
- profile-aware `ValidateTopology(device, profile)`;
- profile-aware `LinuxLibUsbTransport.Open(locator, profile)`.

Current runtime and bootloader discovery call the profile-aware owners directly.

## 3. Confirmed dead surface

Exact deletion set:

### `LinuxLibUsbDiscovery.cs`

1. `LinuxLibUsbNative.ExpectedVendorId`;
2. `LinuxLibUsbNative.ExpectedProductId`;
3. `LinuxLibUsbNative.ManagementInterface`;
4. `LinuxLibUsbNative.OutEndpoint`;
5. `LinuxLibUsbNative.InEndpoint`;
6. `LinuxLibUsbNative.ValidateManagementTopology(IntPtr)`;
7. P/Invoke `libusb_get_device_address(IntPtr)`.

Each has declaration/definition only and no current source/test/doc consumer.

The public `LinuxLibUsbDiscovery.VendorId/ProductId/InterfaceNumber/OutEndpoint/InEndpoint` constants remain because they define the accepted runtime profile and are covered by Transport contract tests.

### `LinuxLibUsbTransport.cs`

8. one-argument `LinuxLibUsbTransport.Open(string locator)`.

The containing transport type is `internal sealed`; current runtime and bootloader discovery both call `Open(locator, profile)` directly. The one-argument wrapper has no consumer and is not an external assembly API.

## 4. Explicit retained surface

Do **not** remove:

- `LinuxUsbProfile`;
- `RuntimeProfile` / bootloader profile;
- profile-aware `EnumerateCandidates(profile)`;
- profile-aware `ValidateTopology(device, profile)`;
- profile-aware `Open(locator, profile)`;
- public runtime USB identity/interface/endpoint constants;
- `SysInfoAsync()`: published low-level raw/operator compatibility API;
- `AssetTransferProtocol.EncodeAbort()`: active protocol API used by RDC-01;
- any Windows transport surface;
- any target/firmware API.

## 5. Frozen source boundary

Authorized product source only:

- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbDiscovery.cs`;
- `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs`.

No test mutation is required unless deterministic validation exposes a missing contract assertion. Existing Transport contract tests already cover accepted runtime/bootloader USB identities/endpoints/profile behavior.

Authorized docs:

- this plan;
- `docs/HOST_DEAD_SURFACE_CLEANUP_ACCEPTANCE_PLAN.md`;
- `docs/CURRENT_STATE.md`;
- RDC closure reconciliation docs after acceptance.

## 6. Proof requirements

Static:

- all eight deleted symbols have zero remaining definitions/references;
- retained profile/public constants still resolve;
- no unrelated Linux/Windows/Core source drift;
- `git diff --check` PASS.

Deterministic Host validation:

- Transport suite remains exactly `24/24` PASS;
- Core suite remains exactly `84/84` PASS;
- Linux transport Release build PASS with zero warnings/errors;
- Core Release build PASS with zero warnings/errors;
- exact candidate path/hash and poststate proof;
- target I/O NONE / Flash mutation NONE.

No real-platform USB rerun is required for deletion-only unreachable surface when compiled profile-aware code and the accepted contract suites remain unchanged and PASS.

## 7. Gates

- Gate 0 — this exact dead/retained/source-boundary freeze.
- Gate 1 — two-file deletion/static ownership review.
- Gate 2 — deterministic Host build/test validation.
- Gate 3 — evidence/hash/closure reconciliation.
- Gate 4 — one normal local acceptance commit.
- Gate 5 — ordinary non-force publication and fresh-fetch clean `0/0`.

## 8. Exit criterion

RDC-02 closes only when all eight confirmed remnants are absent, retained compatibility/profile owners are unchanged, Transport `24/24` and Core `84/84` remain PASS, warning-clean Release builds pass, and the exact deletion candidate is normally published.
