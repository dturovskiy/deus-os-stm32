# STM32 OS — USB CDC ACM Diagnostic/Command Console Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT NEXT**

Boundary ID:

`USB_CDC_ACM_CONSOLE_FOUNDATION`

## 1. Authority and published baseline

This boundary starts from the fully published native USB Device core:

- commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`;
- tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`;
- subject `feat: add native USB device core foundation`;
- accepted core candidate tree `4b798382ef843ef8f488115624d48c0cd1506c75`;
- accepted core firmware `43812` bytes / `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`.

Roadmap sequence remains:

```text
native USB Device core
    -> CDC ACM diagnostic/command console   <- this boundary
    -> transport-neutral shell/RPC
    -> binary transport
    -> host control/update tooling
```

## 2. Boundary objective

Add a standards-based USB CDC ACM virtual serial transport carrying the existing diagnostic/command console over native micro-USB while preserving UART as an independent emergency console.

The first host target is Windows inbox `usbser.sys`; no project-specific kernel driver or custom INF belongs in this boundary. This is a transport boundary, not a shell/RPC redesign.

## 3. Development USB identity

Use a distinct private-test profile so Windows does not reuse the previous vendor-specific device-node/driver cache:

```text
VID      0x1209
PID      0x000B
product  Deus OS CDC Console
scope    private development/testing only
```

The previous accepted core profile remains historical:

```text
1209:000A  Deus OS USB Core
```

`1209:000B` is a pid.codes private Test PID and carries the same hard restriction as `000A`: no redistribution, sale, manufacturing, or claim of global uniqueness. Production/release identity remains a separate future decision.

Canonical identity policy: `docs/USB_IDENTITY_POLICY.md`.

## 4. Windows compatibility target

For automatic Microsoft `usbser.sys` binding, the Device Descriptor targets:

```text
bDeviceClass      0x02
bDeviceSubClass   0x02
bDeviceProtocol   0x00
bMaxPacketSize0   64
idVendor          0x1209
idProduct         0x000B
bNumConfigurations 1
```

No serial-number descriptor is required in the first CDC boundary.

References:

- Microsoft USB serial driver / compatible-ID guidance;
- USB-IF CDC class definitions.

## 5. Descriptor topology

One configuration, two interfaces:

```text
interface 0: Communications / CDC Control
  class     0x02
  subclass  0x02 ACM
  protocol  0x00
  Header Functional Descriptor
  Call Management Functional Descriptor
  ACM Functional Descriptor
  Union Functional Descriptor: master 0, slave 1
  EP1 IN interrupt notification

interface 1: CDC Data
  class     0x0A
  subclass  0x00
  protocol  0x00
  EP2 OUT bulk
  EP3 IN bulk
```

Accepted configuration tree length: `67` bytes.

The ACM functional descriptor advertises only implemented semantics. Accepted `bmCapabilities=0x02`: line coding/control-line state plus serial-state notification semantics. `SEND_BREAK` is not advertised or required.

## 6. Endpoint and PMA ownership

Retain accepted BTABLE/EP0 ownership and extend deterministically:

```text
BTABLE          local 0x000
EP0 TX          local 0x040, 64 bytes
EP0 RX          local 0x080, 64 bytes
EP1 notif IN    local 0x0C0, 16 bytes
EP2 bulk OUT    local 0x100, 64 bytes
EP3 bulk IN     local 0x140, 64 bytes
```

Endpoint addresses:

```text
0x81  CDC notification IN, interrupt, MPS 16
0x02  CDC data OUT, bulk, MPS 64
0x83  CDC data IN, bulk, MPS 64
```

Highest planned PMA byte stays below local `0x180`, within the STM32F103 local `0x200`-byte packet-memory limit. Gate 1 must re-prove the exact endpoint/BTABLE semantics before mutation; no speculative generic allocator is authorized.

## 7. EP0 extensions required by CDC

The accepted core supports bounded standard control IN transfers and zero-length control OUT status handling. CDC requires one deliberate new core capability: a **bounded control OUT data stage**.

Required CDC ACM class requests:

```text
SET_LINE_CODING          0x20, host -> device, 7-byte OUT data stage
GET_LINE_CODING          0x21, device -> host, 7 bytes
SET_CONTROL_LINE_STATE   0x22, host -> device, no data stage
```

Default virtual line coding is `115200 8N1`. Changing CDC line coding must never reconfigure USART1 or couple USB settings to the emergency UART. `SET_LINE_CODING` stores validated virtual state; `GET_LINE_CODING` returns it. DTR/RTS are retained as CDC state/telemetry, not scheduler policy.

Unsupported class requests stall deterministically. Standard request handling is extended only as required for interfaces 0/1 and endpoints, including endpoint halt/status recovery where applicable.

## 8. Configuration lifecycle

`SET_CONFIGURATION(1)` becomes a real transition:

- configure/arm EP1 notification IN;
- configure/arm EP2 bulk OUT;
- configure EP3 bulk IN;
- initialize class transfer state/rings according to the accepted reset policy;
- publish configuration `1` only after endpoint state is valid.

`SET_CONFIGURATION(0)` disables class endpoints and returns to addressed state. USB bus reset returns class state to unconfigured/default state. Disconnect/reconnect and IWDG reset must recover without reflashing or manual driver repair.

Gate 3 hardware evidence proved that a pure MCU/IWDG reset on the Blue Pill-class target can reset the USB macrocell while the board's fixed external D+ pull-up keeps the host attachment asserted. Windows may then retain a stale `usbser` session: EP0/configuration can recover while COM open fails. The corrective implementation therefore forces a host-visible disconnect during every `usb_device_init()` by temporarily owning PA12/D+ as a low open-drain GPIO for a bounded >=20 ms interval before enabling the USB macrocell, then restores the prior PA12 GPIO configuration and starts normal USB initialization. This is USB lifecycle recovery, not scheduler policy.

## 9. CDC data plane

CDC is a byte stream; USB packet boundaries are not command boundaries.

```text
EP2 OUT IRQ/PMA
    -> bounded CDC RX ring
    -> publish USB_CDC_RX readiness event

production task0 / Thread-PSP
    -> drain CDC RX ring
    -> existing command semantics
    -> bounded CDC TX queue

CDC TX queue
    -> EP3 IN packetizer
    -> USB IRQ advances completed packets
```

RX/TX rings are static and bounded. Gate 1 sizes them from measured command/output requirements; no heap.

Backpressure is explicit: disconnect or a host that stops reading must never trap the cooperative production task in an indefinite spin. Queue-full/drop/error counters are required; hardware acceptance requires zero drops/errors under the planned regression load.

EP1 notification IN may publish the minimum CDC `SERIAL_STATE` notification required by the advertised capability. It is not a second data channel.

## 10. Scheduler and IRQ ownership

No third production task is introduced. Existing topology remains:

```text
task0  console/runtime  priority 128  PSP 1024 B
task1  heartbeat        priority 255  PSP 512 B
host   Thread/MSP WFE when both tasks block
```

Task0 wait mask expands conceptually from `UART_RX_EVENT` to `UART_RX_EVENT | USB_CDC_RX_EVENT`.

USB IRQ may move bounded endpoint/PMA data and publish readiness/completion only after authoritative state/ring update. It must not execute command policy, OLED work, or IWDG reload.

Low-level USB core must not own scheduler policy. If wake notification is needed, use a narrow class-to-kernel adapter/callback whose sole purpose is event publication after data/state commit.

## 11. Console semantics and transport isolation

There remains **one command language**, not separate UART and USB command implementations.

Required console refactor:

- parser state is per transport, preventing simultaneous UART/CDC bytes from corrupting one shared line buffer;
- command execution logic is shared;
- response sink is bound to the originating transport;
- USB command responses are not blindly broadcast to UART;
- UART command responses do not depend on USB configuration;
- boot/fatal emergency UART output remains independently usable.

Existing OLED/UI configuration semantics are reused; CDC must not create a second UI-control API.

Only the minimal transport/sink abstraction required for reuse is authorized here. A generalized shell/RPC framework remains the next boundary.

## 12. IWDG and liveness

Accepted watchdog ownership is unchanged:

- no USB IRQ/Handler reload;
- endpoint completion alone is not production liveness;
- Thread/PSP task progress remains reload authority;
- `wdogtrip` must still produce a real IWDG reset;
- after IWDG reset both UART and USB CDC must recover.

## 13. UART/ST-LINK/power discipline

UART remains emergency diagnostics and an independent acceptance/control path. ST-LINK remains recovery/debug.

Power invariant:

**Never power the board simultaneously from ST-LINK 3.3 V and micro-USB VBUS.**

For CDC runtime acceptance, keep micro-USB VBUS/data as the stable sole target power source, ST-LINK 3.3 V disconnected, and UART adapter VCC disconnected. Avoid repeated source switching unless a specific gate requires physical USB disconnect/reconnect.

## 14. Expected Gate 1 source scope

Gate 1 must investigate exact published source before mutation. Expected, not pre-authorized, source set:

```text
include/drivers/usb_device.h
src/drivers/usb_device.c
include/drivers/usb_cdc_acm.h      new if layering review confirms
src/drivers/usb_cdc_acm.c          new if layering review confirms
src/kernel.c
```

`src/startup.s` should remain unchanged unless exact investigation finds a real vector requirement; IRQ20 is already published. Scheduler core/header, linker, and OLED/gfx/status-bar remain guards unless evidence explicitly reclassifies scope.

Gate 1 must reject speculative generic USB class registration, new scheduler states, new SVCs, or IPC unless a concrete CDC correctness requirement proves them necessary.

## 15. Non-goals

Not in this boundary:

- shell/RPC protocol redesign;
- binary framed transport;
- file transfer;
- firmware update/bootloader;
- USB Host;
- HID;
- vendor-specific bulk performance mode;
- custom Windows kernel driver or custom INF;
- generic timers;
- new IPC/queue/semaphore/mutex subsystem;
- third production task;
- runtime-statistics subsystem;
- OLED/gfx/status-bar redesign.

## 16. Gate order

```text
Gate 0  planning/docs synchronization
Gate 1  exact source investigation + CDC implementation
Gate 2  fresh GNU build/link/static validation
Gate 3  real Windows usbser/COM-port + dual-transport hardware acceptance
Gate 4  OLED conditional N/A review
Gate 5  docs/evidence finalization
Gate 6  local acceptance commit
Gate 7  ordinary non-force publication
```

Never combine planning, source mutation, build, hardware acceptance, commit, and push into one gate.
