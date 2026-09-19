# Deus OS — USB Management Device Foundation Acceptance Plan

Status: **GATE 0 ACCEPTED — GATE 1 SOURCE IMPLEMENTATION NEXT**

Boundary:

`USB_MANAGEMENT_DEVICE_FOUNDATION`

This plan accepts or rejects the exact implementation defined by
`docs/USB_MANAGEMENT_DEVICE_FOUNDATION_PLAN.md`.

## Gate 0 — architecture/source freeze

Gate 0 is accepted when all of the following are frozen before source mutation:

- published parent `fa75307fb392718a1d10d52770a6a111c97208e7`;
- private-test identity `1209:000C`, product `Deus OS Device`;
- composite `EF/02/01`, `bcdUSB 0x0210`;
- CDC interfaces 0–1 retained under one IAD;
- management interface 2 = `FF/00/00`;
- EP4 OUT `0x04` and IN `0x84`, bulk 64;
- PMA RX/TX management buffers `0x180/0x1C0`;
- Microsoft OS 2.0 BOS/vendor-request strategy;
- compatible ID `WINUSB` only for interface 2;
- `DeviceInterfaceGUIDs` REG_MULTI_SZ with
  `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`;
- binary frame/RPC semantics reused unchanged;
- independent management parser/RPC state;
- 512-byte RX / 1024-byte TX management rings;
- task0 execution ownership;
- exact initial five-path source boundary;
- resource ceilings and gate order.

No source mutation, build or flash belongs to Gate 0.

## Gate 1 — source implementation

Authorized initial paths:

NEW:
- `include/kernel/usb_management.h`
- `src/kernel/usb_management.c`

MODIFY:
- `include/drivers/usb_device.h`
- `src/drivers/usb_device.c`
- `src/kernel.c`

Reject Gate 1 if any unreviewed scheduler, startup, linker, IWDG, OLED/UI, application runtime, command-service, binary-frame or binary-RPC source is changed.

### Descriptor checks

Must prove exact values:

```text
bcdUSB          0x0210
bDeviceClass    0xEF
bDeviceSubClass 0x02
bDeviceProtocol 0x01
VID             0x1209
PID             0x000C
Product         Deus OS Device

CDC IAD first    0
CDC IAD count    2
management IF    2
management class FF/00/00
management OUT   0x04 bulk 64
management IN    0x84 bulk 64
```

BOS and MS OS 2.0 descriptor byte counts must be compile-time or deterministic static checks.

### PMA checks

Must prove:

```text
EP0 TX 0x040..0x07F
EP0 RX 0x080..0x0BF
EP1 TX 0x0C0..0x0CF
EP2 RX 0x100..0x13F
EP3 TX 0x140..0x17F
EP4 RX 0x180..0x1BF
EP4 TX 0x1C0..0x1FF
```

No overlap and no PMA byte above `0x1FF`.

### Windows binding checks

Source must contain exactly one management MS OS 2.0 function subset for interface 2 with:

- `WINUSB` compatible ID;
- `DeviceInterfaceGUIDs`;
- REG_MULTI_SZ data type;
- exact GUID
  `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`.

The CDC interfaces must not be marked `WINUSB`.

### Runtime ownership checks

Must prove:

- USB IRQ performs endpoint/ring work only;
- management RX wake enters the existing production task0 path;
- parsing/RPC execution happens Thread/PSP only;
- management parser state is not the CDC parser state;
- no cross-origin partial-frame state;
- response bytes return through the originating WinUSB management transport;
- no IWDG reload from USB/management code;
- no new task/SVC/heap/queue/mutex/timer.

### Protocol compatibility checks

Must prove unchanged:

- binary protocol version 1;
- frame types;
- CRC;
- request-ID rules;
- destructive flag;
- request payload limit;
- response chunk limit;
- command-service version 2;
- registry 35;
- RPC IDs `0x0001..0x0023`.

Gate 1 performs no flash.

## Gate 2 — fresh build/static candidate

Build from exact Gate 1 candidate source using Arm GNU:

`-Wall -Wextra -Werror -fstack-usage`

Acceptance:

- C/ASM build success;
- link success;
- undefined symbols 0;
- no heap;
- no unexpected libc synthesis;
- descriptor and PMA compile-time checks pass;
- no duplicate endpoint/interface ownership;
- no change to startup/linker/vector ownership;
- binary/command ABI guards exact;
- exact candidate tree/BIN/ELF/MAP hashes recorded;
- Flash <= `54780`;
- SRAM <= `11824`;
- production task stacks remain `1024/512`;
- candidate generation does not mutate the real Git index.

No flash in Gate 2.

## Gate 3 — Windows + target hardware runtime

Use the exact Gate 2 BIN only.

If current Flash readback already equals the exact target candidate, flashing may be skipped. Otherwise flash once and verify.

### Enumeration

After real microUSB reconnect:

- one composite device `1209:000C`;
- product `Deus OS Device`;
- CDC diagnostic function enumerates;
- WinUSB management interface enumerates;
- no driver warning/error;
- no custom INF required;
- exact management GUID discoverable.

### WinUSB RPC

Using interface 2 bulk endpoints:

- HELLO response exact;
- protocol version 1;
- command-service version 2;
- registry 35;
- `ping` success with request-ID echo;
- `rpcinfo` success;
- `applist` returns exactly two applications;
- `appstart 0x0002` -> device.info;
- repeated start remains idempotent;
- `appstop` -> system.home;
- invalid application ID does not mutate active app.

### Error/recovery

Prove:

- unknown RPC ID -> NOT_FOUND;
- malformed args -> BAD_ARGS;
- destructive method without flag rejected;
- bad CRC executes nothing;
- oversized/malformed frames recover;
- arbitrary header/payload split points recover;
- TX ring full never emits partial binary wire frame;
- subsequent valid frame succeeds after each rejection class.

### Pressure/isolation

Prove at least 128 successful unique-request-ID WinUSB pings.

Also prove:

- no management RX/TX drops in accepted test;
- CDC parser remains independent;
- a partial CDC text line survives management binary traffic;
- management partial frame survives unrelated CDC traffic;
- CDC text and binary remain usable;
- UART pressure path remains usable.

### Recovery

Prove:

- physical USB disconnect/reconnect restores CDC + WinUSB;
- deliberate authorized IWDG reset restores CDC + WinUSB without reflashing;
- application runtime returns to normal boot/home behavior;
- final Flash readback equals Gate 2 target exactly.

### Scheduler/resource runtime checks

After full management activity:

- task0/task1 canaries intact;
- each margin >=256 bytes;
- production scheduler fault counters zero;
- IWDG liveness ownership unchanged.

## Gate 4 — OLED disposition

If Gate 1 did not touch OLED/UI/application rendering:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

If any visible behavior changed or regression is suspected, require explicit physical OLED pass before proceeding.

## Gate 5 — documentation finalization

Update canonical docs with exact:

- accepted candidate tree;
- BIN/ELF/MAP hashes;
- Flash/SRAM;
- endpoint/PMA proof;
- Windows enumeration/binding result;
- management GUID;
- WinUSB RPC/pressure/recovery results;
- task margins;
- Gate 4 disposition.

No source mutation.

## Gate 6 — local acceptance commit

Requirements:

- exact accepted source/docs set only;
- `git diff --cached --check` clean;
- no unstaged/untracked residue;
- source in commit equals hardware-accepted candidate;
- one normal local commit;
- no amend drift after hardware acceptance.

## Gate 7 — publication

Before push:

- fresh `git fetch origin main`;
- prove `FETCH_HEAD == HEAD^`;
- clean repo;
- ahead/behind `1/0`.

Publish with ordinary non-force push only.

After push:

- fresh fetch;
- `HEAD == origin/main == FETCH_HEAD`;
- clean;
- ahead/behind `0/0`.

Then next boundary is:

`HOST_CONTROL_APPLICATION_FOUNDATION`
