# Deus OS — Host Management Presentation Model

Status: **PLANNING / ARCHITECTURE REFERENCE — NOT AN ACTIVE IMPLEMENTATION BOUNDARY**

This document defines the intended relationship between the accepted host-management Core, the existing CLI/Desktop presentation surfaces, and a future browser-based management surface.

It does **not** activate networking, a web server, a new transport, packaging/distribution work, or any target-firmware change by itself. Active implementation authority remains owned by `docs/CURRENT_STATE.md` and by a dedicated promoted boundary with its own plan/acceptance pair.

## 1. Existing accepted foundation

`HOST_CONTROL_APPLICATION_FOUNDATION` already provides:

- `DeusOs.Control.Core` as the transport-neutral host protocol/session/application model;
- `DeusOs.Control.Cli` as a working command-line management client;
- `DeusOs.Control.Desktop` as an existing Avalonia desktop presentation;
- Windows management through WinUSB;
- Linux management through libusb;
- shared HELLO / `sysinfo` negotiation, typed protocol/session errors and application control;
- no stable physical unit identity and no network transport in the accepted v1 host foundation.

The accepted CLI and Desktop are consumers of the same Core. A future Web surface must preserve that separation rather than implementing a second protocol stack.

## 2. Presentation decision

The long-term host-management model is:

```text
                         presentation
             +---------------+---------------+
             |               |               |
            CLI        Avalonia Desktop      Web
             |               |               |
             +---------------+---------------+
                             |
                    DeusOs.Control.Core
                             |
                    IDeviceTransport
                    /              \
               WinUSB             libusb
                 Windows            Linux

Future network work may add another transport/service path only through a
separately reviewed networking/security boundary.
```

The surfaces have different operational roles rather than replacing one another.

## 3. CLI role

The CLI is the **canonical first-class management surface** for automation, diagnostics, headless operation and acceptance.

Reasons:

- it works without a graphical desktop;
- it is appropriate for Ubuntu Server/macmini-style execution hosts;
- it is scriptable and suitable for reproducible acceptance;
- it exposes the portable Core contract without adding desktop presentation state;
- it remains usable as a recovery/operator interface even if richer UI surfaces fail.

The CLI must not become a separate protocol implementation. It remains a thin presentation/orchestration layer over `DeusOs.Control.Core`.

## 4. Avalonia Desktop role

The existing Avalonia application remains a valid **optional workstation frontend**.

It is useful when:

- a local Windows/Linux desktop environment exists;
- the operator wants graphical device/session/application state;
- native desktop discovery/reconnect behavior is desirable.

It is not a prerequisite for headless Linux support and does not define the canonical protocol semantics.

Desktop views must continue to consume Core/session models rather than call WinUSB/libusb directly.

No decision in this document requires deleting or expanding the accepted Avalonia frontend.

## 5. Future Web role

A browser-based UI is the preferred future portable rich-management surface for headless hosts and later network-capable deployments.

For local USB management, the intended topology is:

```text
Browser
   |
HTTP / WebSocket on local host
   |
Deus OS host management service
   |
DeusOs.Control.Core
   |
WinUSB / libusb
   |
STM32
```

The browser must not implement the STM32 binary USB/RPC protocol directly.

The local host service owns:

- device discovery/open/reconnect through the accepted Core/transports;
- translation from host-management models/events into a bounded web API;
- browser-session lifecycle;
- local authorization/exposure policy once that boundary is activated.

A future Web frontend therefore reuses the accepted host-management implementation instead of duplicating it in JavaScript/TypeScript.

## 6. Headless Linux position

Linux support does not imply a graphical desktop requirement.

A headless Ubuntu host may operate through:

1. `deus-cp` CLI directly; or
2. a future local host management service, with the Web UI opened remotely or locally in a browser.

Avalonia installation is optional on such systems.

This distinction must be preserved in future packaging/distribution work.

## 7. Future network transport position

Future ESP/Wi-Fi work must not couple the Web UI directly to ESP-specific framing.

The architecture target is to keep presentation independent from the physical management path:

```text
CLI / Desktop / Web
        |
DeusOs.Control.Core or a bounded host-management service API
        |
   transport/service adapters
        |
  USB today / network later
```

Whether a future network path is implemented as another `IDeviceTransport`, as a host-side service adapter, or as a different bounded abstraction must be frozen by the future networking boundary. This document does not pre-decide wire-level networking architecture.

## 8. Security boundary

The current accepted local USB management session is unauthenticated. CRC-16 is framing integrity, not authentication.

A Web surface bound only to localhost over a trusted local operator session is still host software and must not be described as authenticated merely because it uses HTTP/WebSocket.

Before any management API is exposed over LAN/Wi-Fi or another remotely reachable interface, a dedicated security/trust boundary must define at minimum:

- peer/device authentication;
- operator/user authorization;
- credential/key provisioning and rotation;
- transport confidentiality/integrity as appropriate;
- destructive-operation authorization;
- replay/session handling;
- firmware-update authenticity interaction;
- network exposure/default-bind policy.

Networking must not be enabled merely because a browser UI exists.

## 9. Packaging/distribution implications

Future host distribution should treat presentation packages independently where practical:

- Core + CLI are the minimum portable management capability;
- Avalonia Desktop is an optional graphical workstation component;
- a future host management service/Web package is a separate deployable surface;
- Linux server/headless installation must not require Avalonia or a desktop environment;
- end-user deployment should not require the .NET SDK; self-contained/runtime-packaged distribution may be evaluated in a dedicated distribution boundary.

Exact installer/package formats, signing, update/uninstall behavior and OS integration are not frozen here.

## 10. Stable invariants

Future host-management work must preserve:

1. one portable host protocol/session model in Core;
2. no duplicated binary STM32 protocol parser per presentation surface;
3. CLI remains first-class and headless-capable;
4. Avalonia Desktop remains optional rather than a Linux-server dependency;
5. Web is a presentation over Core/service APIs, not direct USB protocol logic;
6. physical transport selection is not presentation policy;
7. network exposure requires a separate authentication/authorization/security contract;
8. CDC/UART/ST-LINK remain distinct diagnostics/recovery domains rather than UI backdoors;
9. capability discovery controls unavailable surfaces; presentation must not infer features from product name alone.

## 11. Promotion / implementation rule

This document is intentionally non-authorizing.

Implementation requires a concrete promoted boundary named by `docs/CURRENT_STATE.md`.

Possible future boundaries include, depending on actual need:

- host application distribution/packaging;
- local host-management service/Web frontend;
- network service/transport foundation;
- remote-management security/authentication foundation.

A future boundary may refine this model, but it must explicitly document any departure from these invariants.

## 12. Relationship to existing documents

- Current project state: `docs/CURRENT_STATE.md`
- Stable system architecture: `docs/ARCHITECTURE.md`
- Future sequencing: `docs/ROADMAP.md`
- Accepted host foundation: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`
- Accepted host proof: `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`
- Documentation governance: `docs/DOCUMENTATION_MODEL.md`

The accepted Host Control plan remains historical/immutable. This document only defines the forward presentation model built on top of that accepted foundation.
