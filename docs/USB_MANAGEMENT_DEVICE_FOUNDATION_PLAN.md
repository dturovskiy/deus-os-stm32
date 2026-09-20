# Deus OS — USB Management Device Foundation Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `1f88083843c6aae9fd228ad2d677f9252b889a11`**

Boundary ID:

`USB_MANAGEMENT_DEVICE_FOUNDATION`

Published baseline:

- commit `fa75307fb392718a1d10d52770a6a111c97208e7`;
- subject `refactor: decompose kernel composition root`;
- accepted source candidate tree `883cecc8d78306fa28b252332dc9d654fde95b5a`;
- accepted BIN `48636` bytes / SHA-256 `51083C63652DCFCCC479604CA09E191EAB43561C496E2D6F1E5DAABC10CC9766`;
- Flash `48636 / 65536`;
- SRAM `10032 / 20480`;
- final task0/task1 margins `384 / 424` bytes;
- branch `main`, clean, published ahead/behind `0/0`.

## 1. Objective

Add the production-facing Windows USB management function without duplicating the already accepted binary RPC semantics.

The target becomes one composite USB device:

```text
Deus OS Device
  |
  +-- CDC ACM diagnostic function
  |     interface 0  CDC control
  |     interface 1  CDC data
  |     EP1 IN      notification
  |     EP2 OUT     bulk
  |     EP3 IN      bulk
  |
  +-- Deus OS management function
        interface 2  vendor-specific / WinUSB
        EP4 OUT      bulk 64
        EP4 IN       bulk 64
```

WinUSB becomes the primary Windows management transport. CDC remains a secondary diagnostic/compatibility transport; UART remains the emergency text path.

The management interface reuses binary framed protocol v1 and the current command-service registry exactly. It does not define a second command ABI.

## 2. Frozen development USB identity

Private-development identity for this boundary:

```text
VID       0x1209
PID       0x000C
Product   Deus OS Device
Use       private bench / development only
```

`0x1209:0x000C` is a pid.codes Test PID. It is not a production identity and must not be used for redistributed, sold or manufactured devices.

A real product/release requires a separately authorized VID/PID.

This boundary also changes host-visible USB topology and therefore must not silently reuse the CDC-only `0x000B` development identity.

## 3. Composite descriptor topology

Device descriptor:

```text
bcdUSB          0x0210
bDeviceClass    0xEF
bDeviceSubClass 0x02
bDeviceProtocol 0x01
bMaxPacketSize0 64
idVendor        0x1209
idProduct       0x000C
iProduct        "Deus OS Device"
one configuration
```

The `EF/02/01` device-class tuple advertises IAD-aware composite enumeration to Windows.

Configuration has exactly three interfaces.

### 3.1 CDC function

An Interface Association Descriptor groups interfaces 0 and 1.

CDC semantics remain the already accepted implementation:

- interface 0: CDC Communications / ACM;
- interface 1: CDC Data;
- EP1 IN interrupt notification, 16 bytes;
- EP2 OUT bulk, 64 bytes;
- EP3 IN bulk, 64 bytes;
- existing CDC text + binary compatibility path retained.

### 3.2 Management function

Exactly one vendor-specific interface:

```text
interface number  2
class             0xFF
subclass          0x00
protocol          0x00
bulk OUT          0x04
bulk IN           0x84
max packet        64 bytes each
```

EP4 is one bidirectional bulk endpoint register with distinct RX/TX PMA buffers.

No interrupt endpoint is added for management v1.

## 4. Frozen PMA map

STM32F103 USB FS exposes endpoint registers 0..7 and 512 logical PMA bytes.

Accepted CDC map remains:

```text
BTABLE   0x000
EP0 TX   0x040  64
EP0 RX   0x080  64
EP1 TX   0x0C0  16
EP2 RX   0x100  64
EP3 TX   0x140  64
```

Management consumes the remaining 128 bytes exactly:

```text
EP4 RX   0x180  64
EP4 TX   0x1C0  64
end      0x200
```

Compile-time overlap/bound checks are mandatory.

No double buffering is introduced.

## 5. Windows WinUSB binding

Windows 10 is the acceptance host baseline.

Use Microsoft OS 2.0 descriptors, not a custom INF, for the private-development path.

The device exposes a BOS descriptor containing the Microsoft OS 2.0 Platform Capability UUID:

`D8DD60DF-4589-4CC7-9CD2-659D9E648A9F`.

The BOS capability declares:

- Windows version floor appropriate to Windows 10;
- fixed vendor request code `0x20` owned by this boundary;
- exact MS OS 2.0 descriptor-set length `178` bytes (`0x00B2`);
- alternate enumeration disabled.

The vendor control request for the descriptor set is exactly `bmRequestType=0xC0`, `bRequest=0x20`, `wValue=0x0000`, `wIndex=0x0007`; the device returns at most the host-requested prefix of the 178-byte descriptor set.

The MS OS 2.0 descriptor set targets management interface 2 only and contains:

1. set header;
2. configuration subset header with `bConfigurationValue = 0` to select the first configuration subset; the standard USB configuration descriptor remains `bConfigurationValue = 1`;
3. function subset header for first interface 2;
4. compatible-ID feature descriptor with `WINUSB`;
5. registry-property feature descriptor:
   - property name `DeviceInterfaceGUIDs`;
   - data type `REG_MULTI_SZ`;
   - one stable GUID.

Frozen management device-interface GUID:

`{C8B05EDE-1683-5002-81F0-95636B89CEC6}`

The GUID is the stable host discovery ABI for management interface v1. If the host-visible management protocol is intentionally broken in a future incompatible version, that future boundary must explicitly review whether a new GUID is required.

## 6. Transport semantics

The management bulk interface is transport only.

It reuses:

- binary frame protocol version `1`;
- magic `A5 5A`;
- CRC-16/CCITT-FALSE;
- request-ID correlation;
- `ALLOW_DESTRUCTIVE`;
- bounded argument decoding;
- `RPC_DATA` chunking;
- `RPC_END` completion;
- current stable RPC IDs `0x0001..0x0023`;
- current command-service version `2`;
- current registry count `35`.

HELLO is generated from current runtime constants, not from historical hard-coded documentation values.

No management-specific command registry is permitted.

## 7. Parser and execution ownership

Management interface gets an independent parser/runtime state so partial frames on CDC and management can never corrupt each other.

Required ownership:

```text
USB IRQ
  -> bounded management OUT copy into management RX ring
  -> signal existing production task0 with a dedicated internal wake bit
  -> task0 drains management bytes
  -> management binary parser
  -> existing binary RPC adapter
  -> existing command service / handlers
  -> management-originating response writer
  -> management TX ring
  -> EP4 IN
```

Rules:

- no command parsing/execution in IRQ/Handler mode;
- no application callbacks in IRQ;
- no IWDG reload ownership change;
- no new production task;
- no new SVC;
- no public exposure of scheduler event-bit values;
- transport origins remain isolated.

## 8. Management RX/TX buffering

This is management/RPC traffic, not the later high-volume transfer boundary.

Initial bounded rings:

```text
management RX ring  512 bytes
management TX ring  1024 bytes
```

Both are power-of-two SPSC rings.

Rationale:

- inbound binary frame maximum remains bounded by current frame/parser storage;
- outbound RPC wire frames are at most 64 bytes each;
- 1024-byte TX ring permits multiple response frames without allocating whole responses;
- this adds only 1536 bytes static SRAM and avoids speculative 1024/2048 duplication of CDC.

TX frame enqueue is all-or-none for each binary wire frame and never busy-waits.

If Gate 2 resource accounting proves this SRAM budget is undesirable, reducing ring sizes is allowed only before candidate freeze and with explicit pressure proof. Increasing them requires Gate 1 review.

## 9. USB driver API boundary

The USB driver owns:

- descriptors;
- EP0 standard/class/vendor request handling;
- endpoint/PMA configuration;
- management RX/TX rings;
- endpoint interrupt servicing;
- bounded management RX notification callback;
- management configured state;
- nonblocking management byte/span APIs;
- transport diagnostics.

The driver must not own:

- binary frame parsing;
- RPC decoding;
- command lookup/execution;
- application lifecycle policy;
- scheduler policy;
- host identity/capability semantics above USB transport.

## 10. Management runtime module

Expected new module:

```text
include/kernel/usb_management.h
src/kernel/usb_management.c
```

It owns:

- independent binary frame parser state;
- independent `binary_rpc_state_t`;
- management transport binding;
- task0 drain/service function;
- management transport counters that are not low-level endpoint counters.

It reuses `binary_frame`, `binary_rpc` and `command_service` rather than copying them.

## 11. Gate 1 authorized source boundary

Initially authorized source paths:

NEW:
- `include/kernel/usb_management.h`
- `src/kernel/usb_management.c`

MODIFY:
- `include/drivers/usb_device.h`
- `src/drivers/usb_device.c`
- `src/kernel.c`

No change is initially authorized to:

- `include/kernel/binary_frame.h`
- `src/kernel/binary_frame.c`
- `include/kernel/binary_rpc.h`
- `src/kernel/binary_rpc.c`
- command-service registry/IDs;
- scheduler core;
- startup/vector table;
- linker script;
- IWDG;
- OLED/gfx/UI/application runtime.

If implementation proves one of those frozen paths must change, stop and review the blocker before mutation.

## 12. Gate 1 source proof

Gate 1 must prove:

- composite device descriptor exactly `EF/02/01`, `bcdUSB 0x0210`;
- private-test identity exactly `1209:000C`, product `Deus OS Device`;
- CDC IAD groups interfaces 0–1;
- management interface exactly 2 / `FF/00/00`;
- EP4 OUT `0x04`, EP4 IN `0x84`, 64-byte bulk;
- PMA exact map through `0x200` with static overlap checks;
- BOS + MS OS 2.0 descriptor request path exact;
- compatible ID `WINUSB` targets interface 2 only;
- stable GUID exact;
- management parser/RPC state independent from CDC;
- current binary protocol remains v1;
- command-service version remains 2;
- registry remains 35 and RPC IDs remain `0x0001..0x0023`;
- no command execution in IRQ;
- no new task/SVC/heap/queue/mutex/timer/DMA;
- CDC and UART compatibility paths retained.

No flash in Gate 1.

## 13. Gate 2 build/static acceptance

Build exact Gate 1 source candidate using Arm GNU with:

`-Wall -Wextra -Werror -fstack-usage`

Required proof:

- all translation units compile and link;
- undefined symbols 0;
- no heap symbols;
- exact descriptor byte lengths/static checks;
- exact PMA bounds;
- endpoint ownership 0–4 only;
- management ring power-of-two checks;
- binary protocol constants unchanged;
- command registry/IDs unchanged;
- no scheduler/startup/linker/IWDG/OLED source changes;
- exact candidate tree/BIN/ELF/MAP hashes;
- real Git index untouched by candidate-tree build proof.

Resource ceilings for the first candidate:

```text
published Flash  48636
Gate 2 Flash max 54780   (+6144)
published SRAM   10032
Gate 2 SRAM max  11824   (+1792)
```

Gate 2 initially reproduced the published baseline exactly at Flash/SRAM `48636/10032` and measured the first management candidate at `50740/12232`, which exceeded the frozen SRAM ceiling by `408` bytes. The authorized SRAM remediation extended the source set only into `include/kernel/binary_rpc.h` and `src/kernel/binary_rpc.c`: per-transport persistent RPC state remains independent, while request/output scratch storage is moved into one bounded task0-owned workspace because RPC execution is synchronous and serialized by the single production task0 owner. The response builder reuses one 64-byte wire buffer in place instead of duplicating 48-byte data-chunk + 52-byte payload + 64-byte wire scratch per transport. After hardware Gate 3 exposed Windows Code 28 on management interface 2, the MS OS 2.0 first-configuration subset selector was corrected to `0` and `bcdDevice` advanced to `1.02`. Gate 2 rebuild evidence passes with candidate tree `46841b52d351277deb134a6f4709619087b477af`, BIN `50172` / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`, Flash/SRAM `50172/11728` against frozen ceilings `54780/11824`, task stacks `1024/512`, undefined symbols `0`, stack-usage files `21/21`, no Flash and no real Git-index mutation. Evidence SHA-256 `B6101C2FAD615DC41856BD1DE88C46E92517759EC3AC1F56ACB2030B89252AF3`; log SHA-256 `245F15303355C47473C15CB05838DB495764F97D228DB1113B3C7693765D429F`. Gate 3 composite hardware/runtime acceptance (`v11+v14`) now also passes: evidence `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6`, log `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`, WinUSB `128/128`, UART `32/32`, drops `0/0`, task margins `448/424`, physical reconnect PASS, IWDG recovery PASS, final Flash exact.

Task stacks stay exactly 1024 / 512 bytes. Hardware minimum margins remain >=256 bytes.

No flash in Gate 2.

## 14. Gate 3 Windows/hardware acceptance

Flash only the exact Gate 2 candidate.

Windows 10 must prove after a physical USB reconnect:

- device enumerates as composite `1209:000C`;
- product is `Deus OS Device`;
- CDC function still binds to `usbser` and exposes diagnostic COM port;
- interface 2 binds automatically to `WinUSB` without custom INF;
- stable device interface GUID is discoverable exactly;
- management bulk endpoints are 64-byte OUT/IN;
- binary HELLO over WinUSB reports protocol 1, command-service 2, registry 35;
- binary `ping`, `rpcinfo`, `applist`, `appstart 2`, `appstop` work through WinUSB;
- invalid RPC ID, bad argc, bad flags, bad CRC, oversized/malformed/split frames recover without command execution leakage;
- destructive authorization semantics remain exact;
- >=128 management `ping` requests complete with unique request IDs;
- management RX/TX drop/error counters remain zero under accepted pressure;
- concurrent/sequential CDC and WinUSB binary origins do not cross-contaminate parser/request state;
- CDC text remains usable before/after WinUSB activity;
- UART text remains usable;
- real USB disconnect/reconnect restores both functions;
- deliberate IWDG reset restores composite enumeration, CDC and WinUSB automatically;
- application runtime remains home after reboot;
- scheduler production diagnostics, stack canaries and margins stay green;
- final Flash readback equals Gate 2 BIN exactly.

## 15. Gate 4 visual acceptance

OLED/UI source is frozen by this boundary.

Accepted Gate 4 disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Gate 1/2/3 source diffs do not modify OLED/UI/application-rendering logic, and Gate 3 found no visible-regression signal. No additional physical OLED run is required.

## 16. Gates 5–7

Gate 5:
- synchronize canonical docs with exact accepted hashes/results;
- no source mutation.

Gate 6:
- one exact acceptance commit containing only the accepted source/docs set;
- no rebuild drift after hardware acceptance.

Gate 7:
- fresh fetch;
- prove remote is direct parent;
- ordinary non-force push;
- fresh post-push fetch;
- prove `HEAD == origin/main`, clean, ahead/behind `0/0`.

Accepted publication result: Gate 5 PASS (docs-only, accepted source unchanged); Gate 6 PASS at commit `1f88083843c6aae9fd228ad2d677f9252b889a11`, commit tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`; Gate 7 PASS by ordinary non-force push with fresh post-push `HEAD == origin/main == FETCH_HEAD` and clean ahead/behind `0/0`.

Next boundary after publication:

`HOST_CONTROL_APPLICATION_FOUNDATION`

## 17. Explicit non-goals

Not part of this boundary:

- removing UART emergency console;
- removing CDC diagnostics;
- changing binary RPC semantics or stable IDs;
- host GUI/Control Panel implementation;
- file/asset/config transfer;
- firmware update/bootloader;
- streaming telemetry subscriptions;
- network transport;
- authentication/cryptography;
- USB High Speed;
- DMA;
- dynamic allocation;
- generic HAL;
- generic IPC/timer/synchronization work.
