# Deus OS — USB Management Device Foundation Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `1f88083843c6aae9fd228ad2d677f9252b889a11`**

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
- Microsoft OS 2.0 BOS/vendor-request strategy with vendor code `0x20` and exact descriptor-set length `178` (`0x00B2`) bytes;
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

Source must contain exactly one management MS OS 2.0 function subset for interface 2. BOS/vendor retrieval must use `bMS_VendorCode=0x20`, `bmRequestType=0xC0`, `wValue=0x0000`, `wIndex=0x0007`, Windows-version floor `0x0A000000`, and descriptor-set length `178` (`0x00B2`) bytes. The function subset must contain:

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

Acceptance: **PASS**. The exact five-path source boundary is implemented: modified `include/drivers/usb_device.h`, `src/drivers/usb_device.c`, `src/kernel.c`; added `include/kernel/usb_management.h`, `src/kernel/usb_management.c`. The composite descriptor is `bcdUSB=0x0210`, `EF/02/01`, VID/PID `1209:000C`, product `Deus OS Device`, CDC interfaces 0–1 under one IAD, and vendor management interface 2 `FF/00/00` over EP4 OUT/IN `0x04/0x84` bulk 64. PMA is statically bounded through `0x200` with EP4 RX/TX at `0x180/0x1C0`. BOS/MS OS 2.0 uses Windows floor `0x0A000000`, vendor code `0x20`, retrieval `0xC0/0x20/wValue=0/wIndex=0x0007`, exact set length `178`, configuration subset selector `0` for the first configuration (while the standard USB configuration descriptor remains value `1`), exactly one function subset for IF2, `WINUSB`, REG_MULTI_SZ `DeviceInterfaceGUIDs` and GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`. Management owns independent 512-byte RX / 1024-byte TX rings and independent binary parser/RPC state; USB IRQ performs EP4/PMA/ring/notification work only, while task0 performs parsing and existing command/RPC execution in Thread/PSP. Protocol v1, command service v2 / 35 methods / RPC IDs `0x0001..0x0023`, scheduler/startup/linker/IWDG ownership and existing CDC state remain unchanged. Management parser state is reset on USB bus generation change as well as disconnect/reconnect observation, preventing partial-frame carryover across fast reset/re-enumeration. `git diff --check` passes. No build or flash was performed in Gate 1.

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

Gate 2 current result: **PASS**. After the Gate 3 Windows Code 28 investigation proved that Windows requested the MS OS 2.0 set but did not bind `WINUSB`, the configuration-subset selector was corrected to `0` for the first configuration and `bcdDevice` advanced to `1.02`. Fresh Gate 2 rebuild evidence accepts candidate tree `46841b52d351277deb134a6f4709619087b477af` at Flash/SRAM `50172/11728`, within frozen ceilings `54780/11824`; task stacks remain `1024/512`, undefined symbols are `0`, forbidden libc/heap symbol check is clean, stack-usage files are `21/21`, and the real Git index is unchanged. Candidate BIN is `50172` bytes / SHA-256 `FD0A8049193772892C2A3DC1CF2B24FA17BCC83FC4B0F55A22AA6A4962C864FB`; ELF SHA-256 `3A1C7863C94AA24A13520E50EF51367EC7A26631F8FC54EF9F76A143A4630DE0`; MAP SHA-256 `D9C44E1B77C47242CAE9FAD1DB059E1132088D0D63D9C51D75A6572EB8D5709C`. Gate 2 evidence/log SHA-256 are `B6101C2FAD615DC41856BD1DE88C46E92517759EC3AC1F56ACB2030B89252AF3` / `245F15303355C47473C15CB05838DB495764F97D228DB1113B3C7693765D429F`; no Flash. Gate 3 current result is **PASS / COMPOSITE V11+V14** on the same exact candidate. Accepted Gate 3 evidence/log SHA-256 are `1EF8595E85F088F0D3870CA5D880631342AD795FDB94C05EAB9BBC3566A3DCC6` / `083C9B66B7225D3FF37845996B62991C7DE8E84332BD3C8B059F1B8A6569797B`. Hardware/runtime acceptance proves `REV_0102`, automatic inbox `WINUSB` binding on interface 2, WinUSB pressure `128/128`, UART pressure `32/32`, management RX/TX drops `0/0`, physical USB reconnect recovery, authorized IWDG recovery with bounded return to `system.home`, task0/task1 margins `448/424`, intact canaries/zero scheduler faults, and final Flash exactly equal to the Gate-2 BIN; the final continuation performed no flash and no repository mutation.

## Gate 3 — Windows + target hardware runtime

Use the exact Gate 2 BIN only.

Before target access, preflight every host resource required later in Gate 3: STM32CubeProgrammer/ST-LINK availability, target USB presence under normal microUSB power, WinUSB host bridge integrity, and a non-target UART serial adapter (historically COM3 / CH340) enumerated on the host with adapter VCC disconnected. Missing UART host enumeration is an environment failure and must stop the run before SWD/reconnect/IWDG work.

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
- bad CRC executes nothing, proven with a safe state-mutating request against a known application state and a post-rejection state check, not merely by observing no response;
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
- application runtime returns to normal boot/home behavior after the normal boot splash/lifecycle delay; the harness must poll boundedly for `APP_ACTIVE_ID=0x00000001` rather than assert it immediately on the first post-reset WinUSB response;
- the post-IWDG home proof must also require zero application faults and `system.home` RUNNING/ACTIVE while `device.info` is not active;
- final Flash readback equals Gate 2 target exactly.

### Scheduler/resource runtime checks

After full management activity:

- task0/task1 canaries intact;
- each margin >=256 bytes;
- production scheduler fault counters zero;
- IWDG liveness ownership unchanged.

## Gate 4 — OLED disposition

Accepted disposition:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

The accepted source diff does not modify OLED/UI/application-rendering logic and Gate 3 found no visible-regression signal; no explicit physical OLED rerun is required.

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

Acceptance: **Gate 5 PASS / Gate 6 PASS / Gate 7 PASS**. Gate 6 commit `1f88083843c6aae9fd228ad2d677f9252b889a11`, commit tree `931f1cbce8bc7c043bf27626c6127ac7cab9acb9`, contains the exact accepted source/docs set. Gate 7 used an ordinary non-force push; fresh post-push fetch verified `HEAD == origin/main == FETCH_HEAD == 1f88083843c6aae9fd228ad2d677f9252b889a11` with a clean repository and ahead/behind `0/0`.

Then next boundary is:

`HOST_CONTROL_APPLICATION_FOUNDATION`
