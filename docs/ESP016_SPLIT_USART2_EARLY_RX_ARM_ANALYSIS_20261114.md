# ESP01-016 — Split CH340 GPIO control vs STM32 USART2 data; early RX arming

**Актуальне на 2026-11-14 19:25:** normal14 перевірено на Windows-native ARM, підписано тим самим ключем, оригінальний Evidence та BIN незалежно звірено. Реальна плата лишається normal13 із FAULTED=1, оновлення normal14 потребує окремого фізичного запуску. Формулювання «NOT signed / Windows-native not executed» у датованих підрозділах нижче стосуються **попередніх фаз**, а не поточного стану.

**2026-11-14 update:** The previously isolated normal14 candidate was promoted byte-exact into live `src/kernel/usb_management.c` SHA `B5FD61565EFD8F7B8BA9B73CE5A56EFCA48C4B1415659B48797FD7D773ED2385` (task `task_1794623226496_f82903608d28ddaa`). Strict original signed-normal13 negative control and new positive source O0/Os/UBSan PASS (task `task_1794623288559_24a78ce1db190454`); original split USB dispatcher O0/Os/UBSan PASS, bridge core and NodeLink integration 3/3 each. Normal14 93/93 SHA source index `28B6DB3D2DD0EDB35457A4EEC55CA166AB6B08507CB03866ACCBAC422281055D` / SourceTree `28b6db3d2dd0edb35457a4eec55ca166ab6b0850`. Mac native ARM source-bound **UNSIGNED** build PASS: Flash 50,856B, SRAM 8,596B, BIN SHA `5F39C54792392C1CB1BE4E9AEEEB2238051C5CBF43996C919A8BEAA4AE076621`. Canonical Windows OFFLINE ARM build ZIP SHA `ABC7F18CFFD65712861CAE60441A84DF59B6B08500665A8C8FF8266858180FDC`, Guard+93/93 static PASS; **Windows native not executed, no HMAC or physical update**. Host ROM error annotation distinguishes next TX fragment/RX poll on future separately scoped run (Mac .NET build PASS). Installed signed normal13 remains `FAULTED=1` and previous ROM marker consumed. Sections below describe the earlier isolated candidate state historically.

**2026-11-14 — OFFLINE ROOT-CAUSE CANDIDATE; no physical change, no new firmware installed.**

## Actual owner wiring and transport

- ESP8266 **TX → STM32 PA3/USART2_RX**.
- STM32 **PA2/USART2_TX → ESP8266 RX**.
- Mac mini controls ESP8266 **GPIO0/BOOT and EN/CH_PD through independent CH340 DTR and RTS**. CH340 UART TX/RX are **not used for ESP data in the present split wiring**.
- Mac mini connects to STM32 USB Management to carry ESP UART bridge `OPEN/TX/RX/CLOSE`; no direct CH340 serial-data request is valid.

This is the existing intended split design. User correctly restated it. No PB0/PB1 physical wiring exists.

## Original error, exact evidence

Signed normal13 STM32 SourceTree `288b25818a250c79bd81b678dfebd61142bd139c` accepts `ESP_BRIDGE_OPEN`, clean CLOSE without any UART (previous physical PASS). A distinct owner CH340 ROM handover entered the bridge and returned `UartUncertain` during `WAIT_ROM_SYNC`, .NET child exit 2; exact failing TX or RX opcode **not captured**. Original child JSON SHA `5102A30BADB23F9562D4A39A382F40442C71E90C8144174C838F12D46E054B5E`, one-shot marker SHA `4DDF1430EADCC0E02CF863E0311D8D2A31A60E39489B256EEE9DEDDCE23D636C`. Independent passive-only post-stop status `FAULTED=1`, `ACTIVE=0`, `UART_ATTEMPTED=0`, watchdog healthy. `uart_attempted=0` is cleared on abort and therefore NOT a no-UART-activity proof. No ROM response evidence, no Flash.

## Source-level early RX-arm defect candidate

In **original installed normal13** `src/kernel/usb_management.c` SHA `4F8AC015C900D3BBA8E37D3188DF3A82A921D19F95A9A40E234A735D9E989080`, `usb_bridge_enter()` invokes `node_transport_maintenance_enter()` (disarms USART2 RX), then **immediately** `usart2_node_rx_arm()` when OPEN is accepted. BUT split `split_control_handover_v0.py` waits until OPEN has succeeded, **then** manipulates CH340 DTR/RTS to reset ESP into ROM, and only later commands the first UART ROM SYNC TX. Thus STM32 PA3/USART2 RX is listening throughout asynchronous ESP boot output.

In `src/drivers/usart2_node.c`, USART2 is configured 115200 8N1; `usart2_node_irq_service()` treats `PE/FE/NE/ORE` as `rx_loss=1`. The first `usart2_node_try_read()` after loss returns -1 and discards partial ring. `usb_bridge_rx()` propagates -1; `esp_bridge_core_v0_handle()` replies `UartUncertain` and leaves `faulted=1` latched. Prior physical ESP boot records include **74880-baud boot logs** (historical, not proof of their precise timing or behavior in the latest attempt). This is a plausible mismatch between UART RX-gate timing and external BOOT control; **not proof** that the specific latest failure occurred on RX rather than TX.

Separate unverified risk: `CH340BootEnController` assumes `port.dtr=True` drives ESP GPIO0 low and `port.rts=True` drives EN low; the actual split mapping's electrical polarity remains unmeasured. This does NOT imply the wiring is wrong.

## Isolated normal14 candidate and strict negative/positive regression

No mutation of installed normal13 source or signed package. New **source-only** isolated candidate:

- `OS/nodes/esp8266/bridge/usb_management_normal14_deferred_rx_split_source_only.c` SHA `B5FD61565EFD8F7B8BA9B73CE5A56EFCA48C4B1415659B48797FD7D773ED2385`
- `OS/nodes/esp8266/bridge/test_usb_bridge_normal14_late_rx_gate_offline.c` SHA `9305FEAF777D68E79F166F7A94A6809E92F9941B6B9D857D697192E03EA7F52B`
- `OS/nodes/esp8266/bridge/run_usb_bridge_normal14_late_rx_gate_offline.py` (see live SHA), original managed test `task_1794622544438_f66f019f92151875` exit0

The candidate keeps USART2 RX interrupts **OFF during bridge OPEN and external CH340 ESP reset**. On the **first** accepted bridge TX it calls `usart2_node_rx_arm()` once to flush stale SR/DR/ring/loss, then sends data. On later TX fragments, RX remains armed and is not flushed. On leave/abort, it disarms and resets the private armed flag. RX before first TX fail-closes instead of collecting unqualified boot logs. Full-STM32-ownership non-split branch still arms RX at OPEN.

**Regression proof in mock (NOT on real hardware):** Exact original signed normal13 USB source compiled against this stricter harness and **failed at RX armed during OPEN**, as expected. Candidate compiled/run under host GCC `-O0`, `-Os`, UBSan, all **3/3 PASS**, including simulated ESP boot chatter while CH340 controls reset, first TX after handover, two-fragment ROM SYNC 40+6 without losing response, RX, CLOSE, and baseline USB state. ARM-only reset asm neutralized in temporary host test source, not candidate or installed source. No device or signing access.

## Required next decision

Current hardware **remains normal13 with target `FAULTED=1`**. Do not repeat consumed ESP ROM operators, reset STM32, blindly retry CH340/EN, or ESP Flash. The deferred-RX candidate is **NOT signed, NOT ARM/Windows-native built, NOT installed, NOT proven as the physical root cause**. Further work: offline native ARM resource budget and failure-phase diagnostic version/source-index tests, then separately authorize an exact signed successor and physically scoped recovery only after all safety gates. Preserve original owner Evidence and markers.
