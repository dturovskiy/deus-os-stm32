# Deus OS / STM32 OS

A small bare-metal operating-system project for STM32F103 / ARM Cortex-M3.

The target firmware is written without STM32 HAL, Arduino, FreeRTOS, a heap, or a dynamic process loader. The project uses direct-register drivers, a bounded static scheduler/runtime, native USB, UART diagnostics, an SSD1306 local UI, and a C#/.NET host control application.

## Current published state

Live repository HEAD is intentionally not hard-coded in this file because documentation-only reconciliation commits may advance it without changing the accepted product boundary. Verify live Git state directly when exact repository identity is required.

Latest completed product boundary commit:

`e0f49f168542fa1cf49bca451e01b0c077aa8d18` — `feat: add host control application foundation`

Latest completed boundary:

`HOST_CONTROL_APPLICATION_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED**

Accepted target firmware:

- firmware candidate tree: `b895955f7738aceb6fca0272d510cc433378c6ab`
- BIN: `50652` bytes
- BIN SHA-256: `FB68993FC998DE77B61FAF9F4949E4E124B95FF867BB456EBA401C9F2709F13F`
- Flash/SRAM: `50652 / 11728`
- task stacks: `1024 / 512`

Accepted host foundation:

- C# / `net10.0`
- transport-neutral Core
- Windows WinUSB adapter
- Linux libusb adapter
- Avalonia `12.1.2` desktop
- Core tests `21/21`
- Transport tests `5/5`

Published management transport:

- USB identity `1209:000C` / `Deus OS Device`
- CDC interfaces 0–1 retained for diagnostics
- management interface 2 over EP4 bulk64
- Windows binding: WinUSB
- stable management GUID `{C8B05EDE-1683-5002-81F0-95636B89CEC6}`
- binary framing protocol v1
- command service v3 / registry 36
- system capability mask `0x0000001F`

Final Host foundation hardware acceptance includes Windows CLI/Desktop, real Linux libusb runtime, application lifecycle, reconnect with fresh negotiation, 128 unique management pings, zero management/CDC drops, and exact final Flash readback.

## Next boundary

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **Gate 0 contract freeze next**

Before any persistent target write, Gate 0 must freeze a bounded persistence contract covering schema/version, exact size and Flash ownership, CRC/integrity, atomic commit and power-loss recovery, default/previous recovery, erase/program alignment, wear budget, incompatible-version policy, and strict separation of configuration/assets from firmware/update state.

A general filesystem remains deferred until a concrete consumer requires one.

## Target

- MCU: STM32F103 medium-density
- CPU: ARM Cortex-M3 / ARMv7-M
- Flash: 64 KiB
- SRAM: 20 KiB
- debug/recovery: SWD / ST-LINK V2
- emergency diagnostics: USART1
- primary host management: native USB WinUSB
- secondary USB diagnostics: CDC ACM
- local display: SSD1306-class 128x32 OLED

## Implemented foundations

- startup/vector table/reset and fault handling
- clock + monotonic kernel time
- static PSP scheduler with block/event wake and IWDG liveness
- USART1 IRQ/ring diagnostic console
- native USB Device + CDC ACM
- transport-neutral command service
- binary framed RPC v1
- boot splash / desktop UI
- OLED dirty-region optimization
- static application runtime
- kernel composition-root decomposition
- USB management device
- Windows/Linux host control application

## Project structure

```text
OS/
├── docs/
├── host/
├── include/
├── linker/
├── scripts/
└── src/
```

## Documentation

Authoritative current state:

- `docs/ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/MASTER_EXECUTION_CHECKLIST.md`
- `docs/PROJECT_HANDOFF.md`

Historical changes:

- `CHANGELOG.md`

Current Host foundation design/acceptance:

- `docs/HOST_CONTROL_APPLICATION_FOUNDATION_PLAN.md`
- `docs/HOST_CONTROL_APPLICATION_FOUNDATION_ACCEPTANCE_PLAN.md`

Persistence prerequisites for the next boundary:

- `docs/FOUNDATION_ARCHITECTURE_GAP_REVIEW.md`

## Design principles

- Bare metal first.
- Keep boot, IRQ, task and ownership paths explicit.
- Prefer bounded static state over speculative dynamic machinery.
- Keep platform/register code below kernel/service/application semantics.
- Measure before optimizing.
- Do not invent connectivity, identity or runtime state that hardware cannot truthfully report.
- Add filesystem, networking, update/security and other large subsystems only against a concrete product consumer.

## License

MIT
