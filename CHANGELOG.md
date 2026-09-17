## 2026-09-17

### Documentation consistency finalization

Post-publication documentation audit completed after `OS_APPLICATION_AND_UI_MODEL_FOUNDATION` Gate 7. Canonical current-state docs now record Gates 0–7 as published at `3dac2c4528fc77e87e1374ff47f56223d2b44e2c`, distinguish the accepted firmware/source baseline from the later docs-only architecture publication, and identify `BOOT_DESKTOP_UI_FOUNDATION` as the exact next boundary. Historical C3.6/C3.7/C3.8/C3.9/C4.0 plan/acceptance headers that still claimed implementation/publication was pending were corrected to their actual published commits; historical plan bodies remain unchanged. No firmware/source/header/linker/script/build mutation is part of this cleanup.

### Published — Boot / desktop UI foundation — Gates 0–7 accepted

Boundary: `BOOT_DESKTOP_UI_FOUNDATION`.

The accepted revision-2 implementation delivers `BOOT_SPLASH -> DESKTOP_HOME`: bootstrap renders `DEUS OS / STARTING / PLEASE WAIT`, task0 transitions to `DEUS OS / DESKTOP / READY` only after a minimum 1000 ms visible dwell plus production readiness, and the accepted status bar now carries real SYSTEM/USB/NETWORK plus monotonic uptime semantics. Task0 uses the existing timed wait with a 250 ms service bound; semantic snapshot suppression prevents idle polling redraws. A hardware-observed first-candidate defect was corrected by separating one-time/recovery SSD1306 initialization from steady-state panel refresh, so routine minute/status transitions never issue `display off`.

Accepted Gate 2 candidate: tree `41e0c7cd345dd64d3b5336abf2fc46d446f19ecb`; BIN `41520` bytes / `A9E3A929118C32A836CE069FC0D18828A8776A9A648EB4B228060D2336E5CC42`; ELF `70700` bytes / `62A827893CC1EF44B18025E87B8299792636CA2D93093ED569F27F08A0B09F02`; MAP SHA-256 `D051AB4EBDAD4609B7961E5F7441FF90C4AF56766F5A977023B0A58D1E9208A8`; Flash `41520 / 65536`; SRAM `9792 / 20480`. Gate 2 evidence SHA-256 `62EA3D8DCA364F178D8D0B649740DCF551D095FC33F5E21AA424C3E23C8FF729`.

Gate 3 hardware acceptance passed on the exact candidate: no reflash was needed because target Flash already matched; boot/scheduler/IWDG startup, task margins (`640` / `424` bytes), CDC text pressure `128/128` with zero drops, UART pressure `128/128` with zero drops/errors, binary pressure `128/128` unique IDs, malformed/CRC/oversize/split-frame recovery, physical USB reconnect, authorized IWDG reboot/recovery, three USB enumerations and final exact Flash readback all passed. Gate 3 log SHA-256 `9481BEFADB5A8F1D17AF6FC0ACDE238FE66B6956940F56DC86A828CBFAA7F900`; Gate 3 evidence SHA-256 `2B9EA2BB00671E829C5F4718FD63EC68889B25347EABF1D7FEF65191E5C3C0CD`.

Gate 4 physical OLED review is `PHYSICAL_OLED=PASS`: displayed digits and text update correctly, no unintended content is drawn, and no flicker/blank pulse/stale-pixel artifact is visible. Gate 5 documentation/evidence finalization changed docs only. Gate 6 committed the accepted 12-path source/docs candidate as `d1d2230ef70c3e7ffc6e8e01eec82e17dbf8a6e8`, tree `d27cf8246fb7563b2327955ffc06428b9d843b2a`, subject `feat: add boot desktop UI foundation`; Gate 7 published it by ordinary non-force fast-forward and final local/remote ahead-behind is `0/0`.

### Accepted through Gate 5 — OLED dirty-region optimization

Boundary: `OLED_DIRTY_REGION_OPTIMIZATION`.

The accepted implementation keeps one 512-byte framebuffer, adds bounded 16-bit dirty X spans, uses change-aware framebuffer writes, sends exact SSD1306 page/column windows, records bounded present telemetry and renders status components independently so ordinary minute/SYSTEM/USB changes do not clear/recompose the whole framebuffer or rerasterize console content. No shadow framebuffer, heap, DMA, new task/SVC/IPC primitive, geometry change, command/RPC renumbering or USB redesign was introduced.

Accepted Gate 2/3 candidate: tree `75f05f689970b760604112b30346b0c328bfaff2`; BIN `44560` bytes / SHA-256 `93D999CC3C6B3EA7AE3B7FED991E0FCFDFA6C7AC2445412E801226869C6DD677`; ELF `71308` bytes / SHA-256 `E0CB04772160A7906E235C7E6C4984E3BACB1C84AB4C39B244A1995DE1B1950C`; MAP SHA-256 `00AF5C8ADCC1A824BBD43D61805ADB3024BE9FD4D114C13EC63957F2364A8C09`; Flash `44560 / 65536`; SRAM `9848 / 20480`; named persistent optimization metadata increment `55` bytes. Gate 2 evidence SHA-256 `FF1156B29A7BBF8D4F843B4A9AEECD2A9E6402B89BF9CF8C92D2CACAFB9DE7FC`.

Gate 3 hardware acceptance passed on the exact candidate. The historical full semantic refresh measured `572` I2C payload bytes / `36` writes; clean present measured zero traffic; a single changed byte measured `9` payload bytes; deterministic `00:00 -> 00:01` measured span `x=123..125` / `11` payload bytes; USB indicator transition measured `x=9..11` / `11` payload bytes. Task0 margin remained `328` bytes after both `oledstatus` and `oleddirty`; task1 margin remained `424` bytes. CDC/UART/binary pressure `128/128`, malformed-frame recovery, physical USB reconnect, authorized IWDG reboot/recovery and final exact Flash readback all passed. Gate 3 log SHA-256 `887E0D78E025C0EB44B7C69B8C9E19A81D70EB95D5A76FFEE7C4D88EC04C7EE6`; Gate 3 evidence SHA-256 `76A49D4483033708542A7F6F14CA3B2FED90B77F1035C08513A3610E9ED34214`.

Gate 4 is `PHYSICAL_OLED=PASS`: splash/home, real minute rollover, USB indicator transition and `uiruntime` restore were visually accepted with no blank/off pulse, flicker, stale pixels, dirty-span clipping or console corruption. Gate 4 evidence SHA-256 `1C1982E6685D995082B61E99195AE83AC4CFFC537A65875D9B71460CE3C25EB6`.

Gate 5 finalizes documentation/evidence, adds repository LF policy through `.gitattributes`, and records the deferred measured-optimization / robustness / observability / test-profile / storage policy in `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`. Gate 6 local acceptance commit is next; publication has not occurred yet.

## 2026-09-16

### Published — Native STM32F103 USB Device core foundation

Boundary:
`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Accepted candidate:

- source tree `4b798382ef843ef8f488115624d48c0cd1506c75`;
- binary `43812` bytes;
- SHA-256 `1DD1B1528AFD9CB037AE54B873D6DBEAE94BC04DFA0047037DD6037D0BE7CFA6`.

Accepted architecture:

- direct-register STM32F103 USB FS Device on PA11/PA12;
- accepted 72 MHz SYSCLK retained; RCC `USBPRE=0` gives 48 MHz USB clock;
- IRQ20 `USB_LP_CAN1_RX0`;
- explicit BTABLE/PMA/EP0 ownership;
- bounded standard-control request engine with delayed `SET_ADDRESS` semantics;
- development identity `VID 0x1209 / PID 0x000A`, private testing only;
- minimal vendor-specific interface with no non-control endpoints;
- no CDC ACM, new task, SVC, IPC, generic timer subsystem, runtime-statistics subsystem or OLED change.

Hardware proof:

- exact device/configuration descriptors read by Windows through EP0 control transfers;
- initial addressed state `20`, physical reconnect addressed state `21`;
- physical disconnect/reconnect recovered without reflashing;
- post-IWDG USB recovery passed;
- destructive IWDG reboot `8748 ms`, post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- safe surface pre/post `20/20`;
- timed event controlled RX wake `34 ms` pre-IWDG and `35 ms` post-IWDG;
- invasive scheduler diagnostics `8/8 BUSY`;
- UART race `128/128 PONG`, zero RX drops/errors;
- final Flash readback exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Evidence:

- Gate 2 build evidence `5E261F473EA48CAA7ABD2B080E22732D95EEEA24BEA76349A4A10E66088FC81F`;
- Gate 3 hardware log `F612D710A6029ADACD0AE51BF5F85B2F5983F22003A6C09787C99045C520ECEC`;
- Gate 3 hardware evidence `14E7751AFBACEED96DF5816B81969AB33C499FAF182F19BBA89A22A21E2C6B43`;
- Gate 4 disposition `FCC6B971D7CCBDDA7866CBD92B56D33955D328EF1D368690B132E06A6801F676`.

Published commit `3f55f624b72b4c5266ec0e4b0006839c4478bec8`, tree `52c2a0efacf9c533d7664316dbfcac344cb2d742`, subject `feat: add native USB device core foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — USB CDC ACM diagnostic/command console

Boundary: `USB_CDC_ACM_CONSOLE_FOUNDATION`.

Accepted candidate:

- tested source tree `d815a8357b9f77850c08ff071f47be8d2b5d4b53`;
- binary `58548` bytes;
- SHA-256 `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- Flash used `58548` bytes; SRAM used `9200` bytes.

Accepted architecture and hardware proof:

- private-test identity `1209:000B`, product `Deus OS CDC Console`;
- Windows inbox `usbser.sys`, no custom INF, configuration `1`;
- CDC Control interface 0 + CDC Data interface 1;
- EP1 `0x81` interrupt IN, EP2 `0x02` bulk OUT, EP3 `0x83` bulk IN;
- bounded EP0 OUT data stage plus `SET_LINE_CODING`, `GET_LINE_CODING`, and `SET_CONTROL_LINE_STATE`;
- CDC RX/TX rings `1024` / `2048` bytes;
- task0 owns UART + CDC command work with independent parser state and origin-bound responses; no third task/SVC/IPC;
- corrected USB init forces a bounded PA12/D+ low disconnect pulse so MCU/IWDG reset produces a fresh host-visible attach;
- automatic CDC reopen after software reset PASS; physical micro-USB reconnect PASS; post-IWDG automatic `usbser` reopen PASS;
- CDC safe surface `20/20` pre-IWDG and `20/20` post-IWDG;
- scheduler diagnostics `8/8 BUSY`, packet-boundary proof `38/38`, CDC pressure `128/128` with zero drops, UART pressure `128/128`, dual-transport parser/response isolation PASS;
- real IWDG reboot `9028 ms`, post-reset UART + CDC recovery PASS;
- final Flash readback exact.

Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- corrected Gate 2 evidence `49AAFBAFEA0B895DFBEBA7311335069EFD512640A13F200D1F3B9D65D6774869`;
- Gate 3 hardware log `D009F0D7E12C10C3FBA40037C6EC485E9FA27674901C5C3D6C4AAE72C0D42F35`;
- Gate 3 hardware evidence `6035C6FD5C0718252B28AAC07CD67C879AA1F14A639DD9A7854D3B74F54D49CB`.

Published commit `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`, tree `bbc6b24e28279075c41410e8abdace89c4b805b7`, subject `feat: add USB CDC ACM console foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — transport-neutral shell/RPC foundation

Boundary: `SHELL_RPC_FOUNDATION`.

Gate 0 establishes a static allocation-free command service above the already accepted UART and USB CDC transports: one deterministic registry, semantic status codes, generic response sink/context, bounded argument tokenization, exact legacy command compatibility, and `help`/`rpcinfo` introspection. Binary framing, host control application, firmware update, bootloader, new tasks/SVC/IPC, and OLED changes remain out of scope.

Canonical planning: `docs/SHELL_RPC_FOUNDATION_PLAN.md` and `docs/SHELL_RPC_FOUNDATION_ACCEPTANCE_PLAN.md`.

Gates 0–7 are accepted and published. Gate 2 produced candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`, BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`, Flash `37196` bytes and SRAM `9216` bytes. Gate 3 proved the shared command service over both UART and CDC: deterministic 32-method registry, `help`, `help ping`, `rpcinfo`, exact legacy `ERR` behavior, tabs/backspace/DEL/overflow recovery, origin-bound replies, UART+CDC `schedtimed`, retained `20/20` CDC safe surface pre/post IWDG, `8/8` scheduler BUSY, `38/38` packet-boundary PONG, `128/128` CDC pressure, `128/128` UART pressure, physical USB reconnect, real IWDG reboot with automatic CDC reopen, and final exact Flash readback. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`. Published commit `0c33304d2db86e54d715905393f49147bb6dd2ea`, tree `19ac9b95caca09e991842f6f9963864934b0a334`, subject `feat: add transport-neutral shell RPC foundation`; local/remote ahead-behind `0/0`.

### Published — binary framed transport foundation

Boundary: `BINARY_FRAMED_TRANSPORT_FOUNDATION`.

Gates 0–7 are accepted and published. Protocol v1 runs over the existing USB CDC stream with `A5 5A` magic, little-endian fields, request-ID correlation, CRC-16/CCITT-FALSE, stable explicit 16-bit RPC IDs for all 32 command-service methods, bounded four-argument adaptation, 48-byte response-data chunks, structured final status, explicit destructive authorization, deterministic text/binary coexistence, and nonblocking all-or-none CDC frame enqueue.

Accepted tested candidate:

- source candidate tree `c2c3d9743c23ab02329a9652714862fafb5bb17c`;
- BIN `40720` bytes / SHA-256 `AE24F039C2CE24866C900E46EEF09179439E9E93B1F51C97AF9590B7165C2022`;
- ELF `65876` bytes / SHA-256 `4DAFEF92ED58F72BF2C4A29B8E3B1131579BD9ADC4002DB2EAB0F1387BFF634B`;
- Flash `40720 / 65536`, SRAM `9752 / 20480`.

Gate 3 hardware proof:

- Windows `usbser` and retained CDC class controls PASS;
- binary HELLO/request-ID echo and exact `help` chunking/accounting PASS;
- binary safe core `21/21`, scheduler diagnostics `8/8 BUSY`;
- unknown ID, BAD_ARGS, destructive deny/allow, bad CRC, oversize resync and arbitrary split writes PASS;
- partial text preservation and binary/text parser isolation PASS;
- binary pressure `128/128` unique IDs with zero CDC drops;
- retained CDC pressure `128/128`, UART race `128/128`;
- physical micro-USB reconnect restores text + binary without reflash;
- authorized binary `wdogtrip` produced a real IWDG reboot in `7433 ms`; post-reset UART + CDC text + binary recovered automatically;
- final Flash readback exactly matched the Gate 2 candidate.

Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- Gate 2 evidence `D1EAB3470A88A802314B9F9A735CA49799FBD0F30D0413CA99F8698503CF8E3A`;
- Gate 3 hardware log `F8DE91AE43AA2731C26828FF4A993597E4FD940794D0BEE03D661B0DB771758B`;
- Gate 3 hardware evidence `8A38E6E60A3B6B2EAE0F35835E6BBE06AF5E38513A492BA2184662243E1B6565`.

Gate 5 documentation/evidence finalization is accepted. Canonical protocol/design/acceptance: `docs/BINARY_FRAMED_TRANSPORT_PROTOCOL.md`, `docs/BINARY_FRAMED_TRANSPORT_PLAN.md`, and `docs/BINARY_FRAMED_TRANSPORT_ACCEPTANCE_PLAN.md`.

Published commit `2fde9025a51021511e73a76b561f7983ca655e2f`, tree `27248c5ac81c60cc898083b09ea73b95aa1e1ff1`, subject `feat: add binary framed transport foundation`. Ordinary non-force publication complete; local/remote ahead-behind `0/0`.

### Published — Deus OS product/application/UI model foundation

Boundary: `OS_APPLICATION_AND_UI_MODEL_FOUNDATION`.

This documentation-only architecture boundary defines Deus OS as an independently operating embedded device runtime, freezes the first static application/lifecycle model, defines boot splash -> desktop/home -> application-view ownership, assigns real SYSTEM/USB/NETWORK and uptime semantics to the current status bar, separates target applications from host Control Panel plugins/packages, and keeps native dynamic ARM loading, filesystem, update protocol and bootloader deferred.

Canonical planning:

- `docs/OS_APPLICATION_AND_UI_MODEL_PLAN.md`;
- `docs/OS_APPLICATION_AND_UI_MODEL_ACCEPTANCE_PLAN.md`.

Gates 0–7 are accepted. The docs-only foundation was committed as `3dac2c4528fc77e87e1374ff47f56223d2b44e2c` (`docs: freeze application and ui model foundation`), tree `ff63c349a54a508725a460ce2c0d23d28fe1ec33`, and published by ordinary non-force fast-forward. After this foundation, exact firmware order is `BOOT_DESKTOP_UI_FOUNDATION` -> `APPLICATION_RUNTIME_FOUNDATION` -> `HOST_CONTROL_APPLICATION_FOUNDATION`.

A foundation completeness review is now canonical in `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`. It records that no kernel blocker requires speculative RTOS work before that sequence, while explicitly retaining later contracts for semantic application events, system/build/platform capability identity, bounded persistent-state safety, retained crash/reset observability, structured telemetry, security/trust before network mutation or executable update, controlled update reboot handoff, and portability layering. Generic timers, message queues, synchronization and runtime statistics remain consumer-driven; heap/filesystem/RTC/DMA/MPU/general power-management facilities remain deferred until a real product requirement justifies them.

## 2026-09-15


### Published — C4.0 IWDG production liveness foundation

Published commit:
`3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca`

Published tree:
`e024d425e97f878c170ccfc41f0505dce79277a3`

Subject:
`feat: add IWDG liveness foundation`

Accepted firmware:
`25192` bytes /
`4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D`.

Publication:
ordinary non-force fast-forward; local/remote ahead-behind `0/0`.

### Planned — Native STM32F103 USB Device core foundation

Boundary:
`NATIVE_USB_DEVICE_CORE_FOUNDATION`

Roadmap basis:

- minimal STM32F103 USB Device core on PA11/PA12 comes before CDC ACM;
- CDC ACM remains the next class/transport slice after the core;
- generic timer callbacks remain deferred until a real consumer;
- queues/synchronization remain deferred until a real shared-ownership boundary;
- runtime statistics remain a later observability slice;
- OLED/status-bar remains frozen.

Initial core scope:

- direct-register USB FS device support, no HAL;
- preserve 72 MHz SYSCLK and derive the required USB 48 MHz clock;
- USB reset + low-priority interrupt ownership;
- PMA/BTABLE foundation;
- endpoint 0 control-transfer state machine;
- standard control requests needed for minimal enumeration;
- centralized USB descriptor identity with no arbitrary third-party VID/PID;
- UART/ST-LINK recovery paths retained.

Power constraint:
during native USB hardware testing, do not power the board simultaneously from
ST-LINK 3.3 V and micro-USB VBUS.

No source implementation is authorized by this planning gate.

### Published — C4.0 IWDG production liveness foundation

Boundary:
`IWDG_LIVENESS_FOUNDATION_C4_0`

Published parent:
`39ea5b3d1b72fca8d15e22e7544870ab0704c274` (`feat: add production heartbeat task`)

Accepted candidate:

```text
build\iwdg_liveness_foundation_v2\os.bin
25192 bytes
4FAAF278A90540931F67F2A70E3354A4A8E78A8E3ACBAED6CAABBDE99E30D74D
```

Accepted architecture:

- register-level STM32F103 IWDG on independent LSI;
- prescaler `/256` (code `6`), reload `1249`, nominal approximately `8 s`;
- repaired sequence `START -> unlock -> PR/RLR -> wait PVU/RVU -> reload`;
- reset cause captured before reset flags are cleared;
- watchdog starts immediately before normal scheduler ownership;
- reload only from concrete Thread/PSP production progress;
- SysTick/USART IRQ/Handler/fault paths never reload;
- destructive `wdogtrip` never reloads;
- no scheduler-core change, no new SVC, no IPC, no OLED/gfx/status-bar edit.

Hardware proof:

- normal reload count `39 -> 50`;
- real IWDG reboot after `7294 ms`;
- post-reset `RESET_FLAGS=0x24000000`, `IWDG_RESET=1`;
- post-reset heartbeat delta `3`;
- safe `20/20`, timed `4/4`, diagnostics `8/8 BUSY`;
- UART race `128/128 PONG`;
- task0 stack `604 used / 420 margin`;
- task1 stack `80 used / 432 margin`;
- MSP `348 used / 1636 margin`;
- RX drop/error/depth `0/0/0`;
- final Flash exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Evidence:

- build log `D7034BB4597EEF3C5CF1CA6025BAA148BCAD1F21D9534009EF242EAD334A0421`;
- build evidence `96773CADE1440BA844FE1455E049109C4099318F57A8F906D7F28148471E5709`;
- hardware log `6C613D31AEF61968021288F41EED3D28EE8B14DBB8C7A129B21636858676B2D9`;
- hardware evidence `08410EF7BC5F330CF2D18BD7CEDF5E85D83FCD4A825C3D5BD0D0C1E4F7D24F66`;
- Gate 4 disposition `3B69B950D80AB8402D873E02567BEF4976E846CC585EE680CCD852B4E96778AE`.

Gates 0–7 are accepted. Published at `3a8b1b5d0dbfa33e0ced1f02164f1761d21277ca` by ordinary non-force fast-forward.

### Published — C3.9 production heartbeat task ownership

Status: Gates 0–7 accepted and published at `39ea5b3d1b72fca8d15e22e7544870ab0704c274`.

Accepted candidate:

```text
build\production_heartbeat_task_v1\os.bin
24648 bytes
4DA8EBCA998D81F4AA2BDAB9990AB5A62A9A83D8B2081940E701E94D10932A86
```

Accepted architecture:

- task0 remains console/runtime, priority `128`, stack `1024 B`;
- task1 owns normal-runtime PC13 heartbeat, priority `255`, stack `512 B`;
- heartbeat blocks with `scheduler_sleep_ms(500)`;
- SysTick retains only kernel time + `scheduler_tick()`;
- fatal-fault PC13 blink remains an out-of-band diagnostic exception;
- no IPC, no new SVC, cooperative production retained.

Hardware proof:

- heartbeat count `3 -> 10`, delta `7`;
- PC13: `5` transitions, both ODR states observed;
- heartbeat stack `80 used / 432 margin`;
- console stack `616 used / 408 margin`;
- fixed-priority self-test `0x0000003F`;
- task0 `128 -> 128`, task1 `255 -> 255`;
- safe surface `20/20`;
- timed blocking `4/4`;
- invasive diagnostics `8/8 BUSY`;
- retained `4 x 32 = 128/128 PONG`;
- RX `drop/error/depth = 0/0/0`;
- MSP `340 used / 1644 margin`;
- `OLED_RUNTIME_UI_OK`;
- final Flash identity exact.

OLED Gate 4:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

Publication completed at `39ea5b3d1b72fca8d15e22e7544870ab0704c274` by ordinary non-force fast-forward.
### Accepted — C3.8 static fixed-priority scheduling

Accepted candidate:

- `build\scheduler_fixed_priority_v1\os.bin`
- `23792` bytes
- SHA-256 `492F149551F2E638801F8AF31B1B1C1F6723082FE6032E79F6C1CEDE7E79228F`
- `src/kernel.c` `235813864C83E7E813A31288DBE45635AF9948213CC3352A039FC4AC31D82A8D`
- `src/kernel/scheduler.c` `B59B08373662B841C2CC077C92DE18D7FA21DA6DCE4E1DEE435F86AB58566C88`
- `include/kernel/scheduler.h` `A48DCBB90D08FAD03F2D426AA2A129A8D8858204380D4A518D7094A318F5D841`

Architecture accepted:

- static priority range `0..255`, default `128`;
- lower value = higher priority;
- inactive-only `scheduler_task_priority_set()`;
- one shared priority-aware READY selector;
- equal priorities retain round-robin tie behavior;
- cooperative production remains cooperative;
- task0 DEFAULT, task1 UNUSED;
- no new SVC;
- timed/event/WFE/PRIMASK architecture retained.

Hardware accepted:

- `schedprio` `1/1`;
- selector self-test `0x0000003F`;
- active mutation rejected and task0 remains `128`;
- safe surface `20/20`;
- timed blocking `4/4`;
- `8/8` invasive diagnostics BUSY;
- retained `4 x 32`, `128/128 PONG`;
- RX drops/errors/depth `0/0/0`;
- production stack `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact.

OLED Gate 4:
`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`.

Evidence:

- build log `306AC5D97125F5BEFB4B2DB95E602ED8F6541E32CF364AA61ACD3D93A0E946D0`;
- build evidence `33E699282A456B79A0F15C8B2B6DF756BAD5979867091863599575A851CEA000`;
- hardware log `005D646FD613506896BC1A3961DDA752D9D424AD7F4AD0CFAAEE377C50E05222`;
- hardware evidence `41665A892477FB195D00DD722A093F69B09AB2A66669EB872DDB7A3518EACC6C`.

Published as `0312bb376c365c0235b9cafe22e927254528f2c4` (`feat: add fixed-priority scheduler policy`).
## 2026-09-14

### Accepted — C3.7 SysTick-backed timed blocking / sleep foundation

Accepted candidate:

- `build\scheduler_timed_blocking_v1\os.bin`
- `22900` bytes
- SHA-256 `366D92BB36E021A3595ED5F35F78ADA05CA7989E11E295D60B126758801B5D3A`
- `src/kernel.c` `44056ABC0371E75DA242D68FBB530B90C56512C11916533982BD543BCF59ACEC`
- `src/kernel/scheduler.c` `90B53B43D53EB53A0BADC97DC5EEF3D2434A32A999F3940B01AE4B0F8129E618`
- `include/kernel/scheduler.h` `F1EF9AB65CF95C68872E09CDB91BAAF87F01D8EACC2F86B483577A75C5FCF547`

Architecture accepted:

- one existing `BLOCKED` state with deadline metadata; no sleeping state;
- `scheduler_sleep_ms()` and `scheduler_wait_events_timeout()`;
- SVC 4 timed blocking;
- `kernel_ticks` remains time authority;
- wrap-safe signed-delta expiry with maximum timeout `0x7FFFFFFF ms`;
- event/timeout arbitration is first atomic `BLOCKED -> READY` transition wins;
- timeout wake publishes READY before `SEV`;
- production UART wait remains untimed;
- ring is payload; event is notification.

Hardware accepted:

- `schedtimed` `4/4` phases;
- sleep `50 ms`, timeout `50 ms`;
- external UART event wake `164 ms`, event `0x00000001`;
- timed RX delta `26`, host idle delta `2904`;
- no double wake; injected `ping` survives in ring and yields `PONG`;
- safe surface `19/19`;
- invasive scheduler diagnostics `8/8` BUSY;
- retained `4 x 32` burst regression, `128/128 PONG`;
- RX drops/errors/depth `0/0/0`;
- production stack `580 used / 444 margin`;
- MSP `340 used / 1644 margin`;
- final Flash exact;
- OLED runtime restore PASS;
- physical OLED PASS.

Evidence:

- build log `6B523A798B4C801B041D54C039064B8675B7AAD43A1766B6559084B370DF6D84`;
- build evidence `431B01CD3CBC62E4EB7BD0718CCDFABA90F6B8511DCED3A9EB50069DA39D5199`;
- hardware log `3B0BC09AA70F58974B77AE03F8AC754C871545E766851C16C1806487810E4769`;
- hardware evidence `ACF91D141F3789A7F42556046574EA13585A9BC46F50DF95FDD948E5F51D8EF4`.

Published as `90df6a690230c9800c0d8497d5597f87a5ae0409` (`feat: add scheduler timed blocking foundation`).
### Accepted — normal-boot production task ownership + atomic host-idle race closure

Accepted C3.6 candidate:

- path: `build\normal_boot_production_ownership_atomic_racefix_v1\os.bin`
- bytes: `21832`
- SHA-256: `4E3C82B68C7E5B2D6EEE72BFD12FD282F944A32934FB891E5C24B585FE2695BB`
- scheduler core: `src/kernel/scheduler.c` SHA-256 `FA649EF24569AEE653A1CA022778237F76FF236A3F0B1B38FDDA8FB06F867649`
- production ownership source: `src/kernel.c` SHA-256 `FE046521F7A017B3DE204E984ECC392CA471C82824530B00EFCBFDFA433658F1`

- Normal boot now automatically starts one cooperative production console/runtime task on PSP.
- Slot 0 owns production console/runtime work; slot 1 remains `UNUSED`.
- MSP is bootstrap-complete scheduler host/idle + exception stack and uses `WFE` when no task is READY.
- The hardware-discovered READY/BLOCKED terminal-classification race was closed by PRIMASK-atomic host classification in `scheduler_start_mode()`.
- Full safe production command surface: `18/18` PASS.
- Invasive scheduler commands: `8/8` exact `SCHED_DIAG_BUSY`.
- Atomic regression: `4 x 32` unpaced `ping`, `128/128 PONG`, no scheduler return/fatal path.
- RX drops/errors/final depth = `0/0/0`; burst high-water = `4`.
- Production stack = `580 used / 444 free`, canary intact.
- MSP = `320 used / 1664 free`, canary intact.
- Final Flash readback exactly matched the accepted candidate.
- Automated OLED regression + `uiruntime` PASS.
- Physical frozen OLED `DEUS OS / BOOT OK / READY`: `OLED PASS`.
- Local acceptance commit and non-force publication are the only remaining gates.

## 2026-09-13

### Published — scheduler steady-state wait/wake foundation

- Published commit `bfed76e0de1c52bf65e60f66c029cc41710fabd0` (`feat: add scheduler steady-state wait/wake foundation`) by normal non-force fast-forward.
- Published candidate `19932 bytes`, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`.
- Added explicit BLOCKED task state, SVC event wait, ISR-safe event wake, pending-event race closure and host-MSP WFE idle.
- UART RX event publication occurs only after ring insertion; event consumers re-check FIFO/condition after wake.
- Hardware acceptance: 4/4 UART IRQ wait/wake rounds PASS; lifecycle/RX/MSP/OLED regression PASS; physical OLED PASS.
- Worktree/index clean after publication.

### Hardware discovery / scope reclassification — normal-boot production ownership

- Gate 1 source implementation completed in `src/kernel.c`; Gate 2 fresh GNU validation PASS.
- Initial production candidate: `21804 bytes`, SHA-256 `609BFB2216B178C59E6BC7D0F04F4986571465A24C820E3A34E575DB768858E3`.
- Hardware v1 proved automatic cooperative production ownership, PSP execution, host-MSP WFE idle, all 18 safe commands, and all 8 invasive diagnostics blocked with `SCHED_DIAG_BUSY`.
- Before burst pressure: production stack high-water `580 / 1024`, free margin `444`, canary intact; RX drops/errors were `0`.
- UART burst then exposed a scheduler host-idle TOCTOU race: a task can transition `BLOCKED -> READY` between the host's first READY scan and its BLOCKED scan, causing a false `scheduler_abort_run()` and `scheduler_start()` return `0`.
- Observed failure signature: 14 `PONG`, then `SCHED_PROD_RETURN=0x00000000`, `SCHED_PROD_UNEXPECTED_RETURN`, `SCHED_PROD_FATAL`.
- This is a real scheduler-core correctness blocker, not a harness failure.
- C3.6 source scope is therefore reclassified from `src/kernel.c` only to `src/kernel.c` plus a minimal `src/kernel/scheduler.c` host-idle race fix.
- Gate 2B linked the first-pass second READY scan, but static review found a smaller remaining IRQ window before `scheduler_abort_run()`. Final closure requires atomic READY/BLOCKED terminal classification under `PRIMASK`, with terminal abort committed before interrupt restore.
- `include/kernel/scheduler.h`, startup, linker and frozen OLED implementation remain guards.
- Regression hardware acceptance must repeat burst rounds and prove no unexpected scheduler return/fatal path.

### Planning baseline — normal-boot production task ownership migration

- Canonical next boundary is `NORMAL_BOOT_PRODUCTION_TASK_OWNERSHIP_MIGRATION`.
- Added detailed ownership design and acceptance plans.
- Chosen first production topology: one cooperative console/runtime task on PSP, slot 1 UNUSED.
- Reuse accepted 1024-byte console PSP allocation; migration must re-prove >=256-byte free margin.
- Preserve USART1 IRQ sole-DR-reader/ring ownership and drain-first event consumer semantics.
- MSP becomes scheduler host/idle + exception stack after bootstrap; no dummy idle task.
- Initial OLED UI remains bootstrap work; runtime OLED/I2C application calls move with console ownership to PSP.
- All eight console-visible invasive scheduler diagnostics remain blocked with `SCHED_DIAG_BUSY` while production scheduler is active.
- `sleep()`/timed waits, priorities, second production task, OLED task and IPC remain deferred.
- Original target source boundary was `src/kernel.c` only; hardware v1 reclassified C3.6 to `src/kernel.c` plus a minimal `src/kernel/scheduler.c` host-idle race fix. Scheduler header, startup/linker and frozen OLED modules remain guards.

### Hardware + physical accepted — production scheduler lifecycle / diagnostic isolation

- Published baseline before this delta: `8bc1510265d0377adafda663d320c1e55f5b2c3a` (`feat: add console PSP stack-budget probe`).
- `scheduler_init()` is now status-returning and rejects reset while active before any scheduler-global mutation.
- Added read-only `scheduler_is_active()`.
- Centralized scheduler diagnostics behind an active-lifecycle gate; six published invasive diagnostics return exact `SCHED_DIAG_BUSY` while active and preserve idle behavior.
- Added offline `schedisolate` real-preemption diagnostic.
- Accepted source delta remains exactly `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- Candidate `18324 bytes`, SHA-256 `9AFDE9AC5AF196E98A2896BAC0DD7AE610414FB5A2888F1D499D2A5E0A12794B`.
- `.bss=5248 bytes`, `_ebss=0x20000C80`, RAM gap below MSP `15232 bytes`, `fault_record=0x20000764`, probe stack `0x20000060 / 1024 bytes`.
- `4/4` isolation rounds PASS; every round had six busy lines, `INIT_REJECT=1`, `BLOCKED_DIAGNOSTICS=6`, `ACTIVE_PRESERVED=1`, `OVERLAP=1`, both canaries intact.
- Isolation stacks: `160 / 512` and `88 / 512`; switches `16`.
- All six published scheduler diagnostics PASS before and after isolation.
- Console PSP regression: standalone `2/2`; composite `4/4`; exact `+105` IRQ / `+105` byte deltas in every composite.
- Console PSP high-water `560 / 1024`, minimum margin `464`; peer `88 / 512`; maximum switches `693`.
- RX high-water `80 / 128`, zero drops/errors, depth zero after stress.
- MSP high-water `320 / 1984`, minimum margin `1664`, canary intact.
- Fresh reset isolation + console probe PASS; final exact flash identity PASS; runtime failures `0`.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Normal boot remains MSP-owned and production scheduler remains inactive during normal boot.
- Next active scheduler gate: steady-state wait/wake foundation; normal-boot migration remains later.

### Hardware + physical accepted — console PSP stack-budget foundation

- Added inactive-only external task-stack binding while preserving the legacy internal scheduler stack pool at `2 x 512 bytes`.
- Added a dedicated aligned `1024-byte` console PSP measurement stack.
- Split safe named console dispatch from scheduler diagnostics; the accepted PSP surface is exactly `17` non-scheduler commands.
- Increased `UART_COMMAND_CAPACITY` from `16` to `32` after hardware v1 proved the 17-character `schedconsoleprobe` command could not reach dispatch with the old parser capacity.
- Accepted source delta: `include/kernel/scheduler.h`, `src/kernel.c`, `src/kernel/scheduler.c`.
- Candidate `17028 bytes`, SHA-256 `13539E4F0C422FF0E3C373EF4167F3238FFA6D9A1B09F309C1A07787F2596C7F`.
- `.bss=5224 bytes`, `_ebss=0x20000C68`, RAM gap below MSP reservation `15256 bytes`, `fault_record=0x2000074C`, probe stack `0x20000048 / 1024 bytes`.
- Hardware v3: `4/4` standalone and `8/8` composite probes passed.
- PSP high-water `600 / 1024 bytes`; minimum margin `424 bytes` >= `256-byte` acceptance floor.
- Peer high-water `88 / 512 bytes`; maximum switches `693`; overlap `1`; both canaries intact.
- Exact safe console surface `17/17` completed.
- RX high-water `80 / 128`; drops/errors `0`; depth returned to zero; all `8/8` composites produced exact `+105` IRQ / `+105` byte deltas.
- MSP high-water `360 / 1984`; minimum margin `1624`; canary intact.
- Fresh-reset probe and exact fresh `+105` delta passed.
- Legacy scheduler, health, I2C/OLED regressions and final exact flash identity passed.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Accepted sizing decision: `1024 bytes` for the exact tested 17-command safe PSP surface; 512-byte full-console size remains rejected.
- Normal boot remains MSP-owned; lifecycle/diagnostic isolation is now accepted above; steady-state wait/wake is the next scheduler gate.

### Planned host application naming

- Native USB direction remains STM32F103 USB Device -> CDC ACM -> shell/RPC -> binary transport -> host control/update tooling.
- Provisional cross-platform Windows/Linux host application name: **Deus OS CP** (`Deus OS Control Panel`).
- The name may be changed later without changing the protocol/transport architecture.
- UART remains the emergency console and ST-LINK remains recovery/debug access.

### Hardware + physical accepted — MSP runtime high-water / guard foundation

- Reserved the upper `2048 bytes` of SRAM as a dedicated MSP region:
  - bottom `64 bytes` are the guard/canary area
  - upper `1984 bytes` are the measurable watermark capacity
  - `_smsp_stack=0x20004800`
  - `_emsp_guard=0x20004840`
  - `_estack=0x20005000`.
- `Reset_Handler` initializes the MSP guard and watermark before its first `BL kernel_main`, so boot/runtime Thread-mode MSP usage is included in the measurement.
- Added read-only `mspstat -> MSP_STACK_OK` telemetry:
  - reserved size
  - guard size
  - usable capacity
  - high-water used bytes
  - remaining margin
  - current MSP use
  - canary state.
- Accepted source delta is exactly `src/kernel.c` + `src/startup.s` + `linker/stm32f103c8.ld`.
- Candidate:
  - `15620 bytes`
  - SHA-256 `C15634184CAB7BA3C5CE503773EB7BA4BB45DDB4B32A3D399F6BE587C518D907`
  - MSP reserved `2048 bytes`
  - MSP usable capacity `1984 bytes`
  - SRAM gap below MSP reservation `16320 bytes`.
- Hardware proof:
  - initial MSP high-water `400 bytes`; margin `1584 bytes`
  - OLED/I2C regression raised high-water to `568 bytes`
  - accepted maximum high-water `596 bytes`
  - minimum observed margin `1388 bytes`
  - canary remained intact
  - `12/12` composite nested-pressure rounds passed
  - pressure classes: `oledstatus`, `schedpreempt`, `schedworkload`
  - each composite included `16 x ping`
  - RX ring high-water reached `80 / 128 bytes`
  - RX drops `0`, errors `0`, depth `0` after accepted composites
  - fresh-reset MSP pressure accepted at `572 bytes` used / `1412 bytes` margin.
- Scheduler/workload/health/I2C/OLED regressions remained PASS.
- Final exact flash readback matched the candidate.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- Console ownership is still MSP; the production scheduler is still not started during normal boot.

### Hardware + physical accepted — USART1 RX IRQ / 128-byte ring-buffer foundation

- Replaced direct polling reads of `USART1_DR` with IRQ-driven receive ownership:
  - `USART1_IRQHandler` is the sole `USART1_DR` reader
  - external IRQ37 is wired in the real vector table
  - RXNE interrupt enabled; USART1 NVIC priority `0x80`.
- Added a 128-byte single-producer/single-consumer RX ring.
- Kept the existing `uart_try_getc()` API as the consumer path, so the console remains MSP-owned in this slice.
- Restored `WFI` idle in `kernel_main()`; SysTick and USART1 interrupts wake the core.
- Added read-only `rxstat -> RX_IRQ_RING_OK` telemetry:
  - capacity
  - IRQ count
  - received byte count
  - drop count
  - error count
  - high-water
  - current depth.
- Accepted source delta is exactly `src/kernel.c` + `src/startup.s`.
- Candidate:
  - `15084 bytes`
  - SHA-256 `E25DC54C149EB9DA7B26F5378868DAA790847A1C4C7B4CFB970F294CFB738EFB`
  - `.bss=2112 bytes`
  - `_ebss=0x20000840`
  - SRAM headroom `18368 bytes`
  - linked `fault_record=0x20000320`
  - USART1 IRQ own static frame `12 bytes`.
- Hardware burst proof:
  - `4 x 32` `ping` commands: every round returned exact `32/32` `PONG`
  - each burst plus its `rxstat` snapshot produced exact `+167` IRQ and `+167` byte deltas
  - observed RX ring high-water `29 / 128 bytes`
  - zero drops
  - zero RX errors
  - depth returned to zero after every accepted burst
  - fresh reset repeated exact `167` IRQ / `167` bytes with high-water `29`.
- Scheduler, substantive PSP workload, health, I2C and full OLED regressions remained PASS.
- Final flash readback exactly matched the accepted candidate.
- Physical OLED remained `DEUS OS / BOOT OK / READY`.
- This slice does **not** start the production scheduler or migrate the normal console to PSP.

### Architecture decision after substantive PSP workload

- Substantive PSP workload is published as `8f6b922a7d2e55abc3133702e7571057f995da5d`.
- Direct normal-boot migration was rejected until prerequisite ownership/stack issues are separated.
- The full current console linked feasibility estimate is `524 bytes`; with the 64-byte context reserve it is `588 bytes`, so a 512-byte console PSP stack is not accepted.
- Scheduler self-tests currently reinitialize global scheduler state and cannot safely run nested inside an active production scheduler.
- Kernel/MSP runtime stack budget was unproven at the decision-audit boundary.
- USART1 polling RX was selected as the first prerequisite and is closed by the IRQ/ring-buffer hardware acceptance above.
- The separate MSP runtime budget prerequisite is now closed by the measured guard/high-water acceptance above.

### Next

- Prove the console PSP workload stack budget explicitly; the prior `524 + 64 = 588-byte` feasibility result still rejects a 512-byte full-console PSP stack.
- Keep console ownership on MSP until that runtime workload-sizing gate passes.
- Keep production scheduler lifecycle/diagnostic isolation and normal-boot task migration as separate later gates.

### Accepted / published — substantive preemptive PSP OLED workload

- Added command-gated `schedworkload -> SCHED_WORKLOAD_OK`.
- Added public `scheduler_start_preemptive()` wrapper and read-only PendSV switch-count telemetry.
- Workload task 0 executes the frozen OLED runtime full render/present path on PSP.
- Workload task 1 is CPU-only, performs no UART/I2C/OLED access, and does not voluntarily yield.
- Task stacks remain exactly `2 x 512 bytes`.
- Static feasibility evidence:
  - planning estimate for `oled_runtime_ui_show()`: `212 + 64 = 276 bytes`; margin `236 bytes`
  - source-build workload task 0 estimate: `220 + 64 = 284 bytes`; margin `228 bytes`
  - source-build workload task 1 estimate: `12 + 64 = 76 bytes`; margin `436 bytes`.
- Real hardware high-water:
  - workload task 0: `328 bytes`; measured free margin `184 bytes`
  - workload task 1: `80 bytes`; measured free margin `432 bytes`
  - capacity: `512 bytes` per task
  - canaries intact for both tasks in every accepted run.
- Real preemption proof:
  - `WORKLOAD_UI_RESULT=1`
  - `WORKLOAD_PEER_OVERLAP=1`
  - PendSV switches observed `124..126`
  - first workload PASS
  - 32/32 workload stress commands PASS
  - 4/4 return-to-kernel checkpoint pings PASS
  - final workload PASS
  - total accepted workload commands: `34`.
- Full scheduler/UART/I2C/OLED regression remained valid and final exact flash identity matched the candidate.
- Hardware v1 boot checks were false negatives caused by stale harness expectation `FAULTREC=0x20000270`.
- Linked `fault_record` is `0x20000284`; recovery v2:
  - proved the only two v1 false results were the boot matcher
  - revalidated both captured v1 boot frames
  - passed two fresh corrected boot checks
  - revalidated `schedworkload`, scheduler regressions, I2C, health, runtime UI, and exact target readback without reflashing.
- Accepted candidate binary: `14292 bytes`.
- SHA-256: `8C124B0954D65E0F698AD1C62525E72FC4F569D5133EF8F0CE67A8297A80A2CF`.
- `.bss=1952 bytes`; `_ebss=0x200007A0`; SRAM headroom `18528 bytes`.
- Physical frozen OLED output confirmed unchanged: `DEUS OS / BOOT OK / READY`.

The runtime `328-byte` task-0 high-water exceeds the `.su`-based `284-byte` source-build estimate. Therefore that static call-chain method is a feasibility estimate, not a conservative stack upper bound. Runtime watermark/canary evidence is authoritative for sizing.

For the exact tested frozen OLED render/present workload, the current 512-byte PSP stack has `184 bytes` (35.9%) measured margin. Console-task and MSP/kernel stack requirements remain separate open proofs.

### Next

- Finalize the production task ownership/stack budget from the accepted runtime evidence.
- Prove any console-task stack and MSP/kernel stack budgets separately.
- Keep normal-boot scheduler ownership migration deferred until those budgets and the migration design are explicit.
- Preserve `schedtest`, `schedcoop`, `schedpreempt`, `schedstack`, and `schedworkload` as regression gates.

## 2026-09-12


### Accepted — OLED runtime lifecycle (Slice 8)

- Frozen 128x32 OLED UI is initialized as part of the normal boot/runtime lifecycle.
- Boot UART contract includes `OLED_RUNTIME_UI_OK`.
- Full UART/OLED regression passed.
- Physical OLED result accepted.
- Accepted binary: `10148 bytes`.
- SHA-256: `FC8AC07A35A0FA83F4F2F8A06EBCC5C8E603C7B843DDE30E827FD7FD815E5321`.
- Acceptance commit: `4217865403d9725707f4c17e572317caf7fe733f`.

### Accepted — scheduler foundation (Slice 9A)

- Added `include/kernel/scheduler.h` and `src/kernel/scheduler.c`.
- Added two static TCBs, two 512-byte static task stacks, and synthetic initial Cortex-M task frames.
- Added deterministic foundation self-test:
  - `schedtest -> SCHED_FOUNDATION_OK`.
- Accepted binary: `10752 bytes`.
- SHA-256: `29CA6F248B94A861497D2A97C723B4E945208FC4002C363956753599AE38BFBC`.
- Acceptance commit: `1a57f79cda42674219e774900ce07a0da8fedaf4`.

### Accepted / published — cooperative scheduler activation (Slice 9B)

- Corrected initial PC / stacked LR Thumb semantics.
- Activated task Thread-mode execution on PSP.
- SVC `#0/#1/#2` provide start / voluntary yield / task-return exit.
- Added kernel/MSP parking and restoration.
- `schedcoop -> SCHED_COOP_OK` validates `0x10 -> 0x20 -> 0x11 -> 0x21`.
- Real normal task-return path passed across `34` complete cooperative runs.
- Candidate binary: `11792 bytes`.
- SHA-256: `27C5327125BFAC97526F80F83248620152893F7AD92B46F6C56542843F29B885`.
- Acceptance commit: `1114621e9a6bc57d5471cf51a922c216b76bebe2`.

### Accepted / published — PendSV timer-driven preemption

- Activated `PendSV_Handler`.
- `SysTick_Handler()` calls `scheduler_tick()`.
- Scheduler tick requests PendSV only during the command-gated preemptive run.
- PendSV priority is lowest.
- PendSV saves/restores `r4-r11` on PSP.
- EXC_RETURN/SPSEL guard prevents MSP-origin PendSV from touching PSP.
- Abort and final-exit paths clear stale pending PendSV before kernel/MSP restoration.
- Added `schedpreempt -> SCHED_PREEMPT_OK`.
- Preemption acceptance tasks are CPU-bound and contain no voluntary yield.
- Expected sequence: `0x30 -> 0x40 -> 0x31 -> 0x41 -> 0x42 -> 0x32`.
- Hardware path passed across `34` complete preemptive runs with full regression.
- Accepted binary: `12740 bytes`.
- SHA-256: `E1D02C22AF7739DB3EE71E9CB9FF65D0A5F78F8C0ED61D632EFC1444040A7E4A`.
- `.bss=1920 bytes`; `_ebss=0x20000780`; SRAM headroom `18560 bytes`.
- Acceptance commit: `44c9d1c44dc9ce95fde77e68588cc98b5cd8aab4`.

### Accepted / published — scheduler stack canary / high-water instrumentation

- Added command-gated `schedstack -> SCHED_STACK_WATER_OK`.
- Existing task stacks remain exactly `2 x 512 bytes`.
- Canary/high-water telemetry is reset at the start of each scheduler run.
- High-water is recorded on SVC yield, SVC exit, and PendSV switch paths.
- Stack scanning executes in Handler mode/MSP, so the measurement does not consume the measured PSP stack.
- `schedstack` runs the cooperative and preemptive self-tests and reports per-task usage.
- Real hardware measurements:
  - cooperative task 0: `72 bytes`
  - cooperative task 1: `72 bytes`
  - preemptive task 0: `72 bytes`
  - preemptive task 1: `72 bytes`
  - capacity: `512 bytes`
  - observed free margin: `440 bytes`.
- Static worst-case for the current synthetic test tasks was also `72 bytes`, matching the hardware high-water result.
- Canary remained intact in every accepted run.
- Hardware acceptance:
  - exact flash program/verify/readback PASS
  - first real stack-water command PASS
  - 32/32 stack-water stress commands PASS
  - 4/4 return-to-kernel ping checkpoints PASS
  - post-stress SysTick health PASS
  - `schedtest`, `schedcoop`, and `schedpreempt` regressions PASS
  - full I2C/OLED legacy regression PASS
  - final reset + `schedstack` + cooperative + preemptive + health PASS
  - final exact flash identity PASS
  - total accepted stack-water commands: `34`
  - total underlying scheduler runs: `68`
  - physical frozen OLED output confirmed unchanged.
- Accepted candidate binary: `13408 bytes`.
- SHA-256: `4A57F4559AAC3BDAE8FEF5FD3B51F3DEA9033FC917DA19032754796459959D42`.
- `.bss=1936 bytes`; `_ebss=0x20000790`; SRAM headroom `18544 bytes`.
- This acceptance proves the current synthetic scheduler test tasks fit comfortably in 512-byte stacks; workload-specific proof remains required for substantive production paths.
- Acceptance commit: `4eaa4f1845fd973ec7ac4393e2f4354fdaf7c66c`.

### Transition

- The next gate after this milestone is a representative substantive PSP workload with real high-water/canary measurement.
- Normal boot, console, and OLED remain on the current kernel/MSP path until that workload-specific proof is complete.
<!-- END STM32_OS_CHANGELOG_2026_09_13 -->

All notable project milestones are recorded here.

This project is currently pre-release; entries are milestone-oriented rather than semantic-version release notes.

## [Unreleased]

<!-- BEGIN STM32_OS_CHANGELOG_2026_09_10 -->
## 2026-09-10

### Added
- Custom Cortex-M3 startup, vector table, .data initialization, and .bss clearing.
- Direct-register 72 MHz clock configuration and SysTick timebase.
- Fault diagnostics for HardFault, MemManage, BusFault, and UsageFault with stacked-context capture.
- Hardware-proven controlled UsageFault diagnostic path and LED panic signaling.
- USART1 polling TX console on PA9 at 115200 8N1.
- UART boot banner and 32-bit hexadecimal output helper.
- Stable kernel time API: kernel_time_now().
- Wraparound-safe deadline and elapsed-time helpers.
- Automated PowerShell hardware loop for build/preflight, ST-LINK flash/verify/reset, UART capture, and acceptance logging.

### Verified
- ST-LINK SWD programming and verification.
- SysTick-driven PC13 heartbeat.
- UART boot output through external USB-UART adapter.
- Time API build and hardware regression image SHA-256: E672398FACBB3BA83E8F05DF4D7165ACFC1849D34BF3589EA81E114ED2DC72E9.
- Dynamic fault_record ELF symbol resolution after its RAM address moved to 0x20000004.

- Time-API hardware regression passed on the 1000-byte image; UART banner matched dynamic `fault_record=0x20000004`, and the PC13 SysTick heartbeat was physically reconfirmed.

- USART1 bidirectional console hardware milestone passed: PA10 RX + PA9 TX at 115200 8N1, automated `ping` -> `PONG`, image SHA-256 `13715B1E206AAD6A8F2EE82E96584623954905C7CAB03611E87514E1D37BE954`, PC13 heartbeat reconfirmed.

- Console `uptime` hardware milestone passed: live `kernel_time_now()` values advanced from `1936 ms` to `2938 ms` (delta `1002 ms`), `ping` regression passed, dynamic `fault_record` matched ELF, image SHA-256 `67C08D182D7CF152BD01F449420C9C5445F0D40F3A49D43B6E1A82BBBFCCC988`.

- Console `health` hardware milestone passed: 10 automated samples showed `3399 ms` SysTick progression, both PC13 output-latch states, and 7 state transitions; image SHA-256 `5BA9FED465FD9F7988B3156CA612EFB88021B1071C6137ABF1D7D1B82042C203`.

- Read-only `fault` console milestone passed: clean fault_record/CFSR/HFSR, SHCSR `0x00070000`, dynamic ELF-matched FAULTREC, and full ping/uptime/health regression; image SHA-256 `185899CE66C3B8683088284939BED267C9E473EFC9317386913595EE15271072`.

- Phase 6 I2C1 hardware scan passed: B6/B7 at 100 kHz, one device ACKed at 7-bit address `0x3C`, with full console/fault/health regression; image SHA-256 `B8F894631076F34EA259E36C0A6744D4FAA6A3B69657F58CEFED72893D25945B`.

- First SSD1306 command transaction passed: `oledping` sent control `0x00` + NOP `0xE3` to address `0x3C`, returned `OLED_CMD_OK`, and the device remained visible to `i2cscan`; image SHA-256 `AA5F092F9C25E6ED07B2F23DEAFF40AF358F07F89B8544EB1BAE2D47605CC9E6`.

- First visible SSD1306 output passed: `oledtest` initialized the 128x64 panel at `0x3C`, wrote a full 1024-byte `0xAA/0x55` checkerboard, returned `OLED_TEST_OK`, remained responsive, and the user physically confirmed the visible pattern; image SHA-256 `46D97925518CD8924278D6A27709E2BEF96AB89E2C49E98E01E88BA40625A5D7`.

### Planned
- USART1 RX and bidirectional kernel command console.
- Native USB Device support on PA11/PA12.
- USB CDC console, binary/RPC transport, host-rendered UI path, and later USB firmware update/bootloader.
- OLED SSD1306 system/status console.
- Scheduler/tasks and PendSV context switching.
<!-- END STM32_OS_CHANGELOG_2026_09_10 -->

<!-- BEGIN STM32_OS_SCHED_WAIT_WAKE_CHANGELOG_20260913 -->
## 2026-09-13 — production scheduler steady-state wait/wake foundation accepted

### Added

- `SCHEDULER_TASK_BLOCKED` lifecycle state and per-task wait/wake event metadata.
- `scheduler_wait_events()` SVC wait path and ISR-safe `scheduler_event_signal()`.
- Host-MSP `WFE` scheduler idle ownership when all incomplete tasks are blocked.
- `schedwaitwake` UART-IRQ hardware diagnostic.
- `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md` for harness/evidence/failure classification and recovery rules.

### Corrected

- Wait/wake diagnostic now treats UART events as notifications to re-check the RX FIFO instead of assuming the event identifies the next payload byte.
- CR/LF console framing is filtered from the wait/wake payload proof.
- `SCHED_WAIT_WAKE_ARMED` is emitted from the active scheduler task so the host can prove blocked -> WFE idle -> IRQ wake -> PSP resume.

### Accepted evidence

- source/build candidate: `19932` bytes, SHA-256 `C21915F3DFA898C8E9F2FC601BC9E0FDA4ABE8EBB23528BF14F25D82FE28CE81`;
- four real UART IRQ wait/wake rounds passed with payload `0x57`, positive idle-WFE counts, and intact PSP canaries;
- lifecycle isolation passed with seven invasive diagnostics blocked while the scheduler was active;
- all six existing scheduler diagnostics passed before and after wait/wake proof;
- USART1 RX ring remained at zero drops and zero errors;
- MSP canary and margin remained healthy;
- OLED runtime regression and final physical OLED appearance passed.

Normal boot remains MSP-owned. Production scheduler normal-boot ownership/migration is the next implementation boundary; timer sleep semantics and priorities remain deferred.
<!-- END STM32_OS_SCHED_WAIT_WAKE_CHANGELOG_20260913 -->
