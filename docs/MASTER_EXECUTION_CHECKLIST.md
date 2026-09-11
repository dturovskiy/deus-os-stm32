# Master Execution Checklist

This file is the canonical execution gate for the STM32 OS project.

## Current hardware baseline

MCU:

- STM32F103C8T6 / medium-density Cortex-M3
- 64 KiB flash
- 20 KiB SRAM

OLED:

- native `128x32`
- SSD1306-compatible
- I2C address `0x3C`
- framebuffer `512 bytes`
- four pages
- `A8=0x1F`
- `DA=0x02`
- `D3=0`
- start line `0`
- `A1/C8`
- full window columns `0..127`, pages `0..3`

The former 128x64 assumption is obsolete.

## Accepted OLED milestones

- I2C scan / address proof.
- SSD1306 command proof.
- visible output proof.
- SSD1306 driver extraction.
- framebuffer/font extraction.
- raw mapping calibration.
- native 128x32 geometry proof.
- 1x font/native full-frame baseline.

Accepted native baseline evidence:

- image size `3412 bytes`
- SHA-256
  `0758C49D3EA10447C684987D74481C8B6D7F7002F7CF79B6E804D18A9399F4F0`
- framebuffer `512 bytes`
- physical thin `DEUS OS`
- full clean frame

## OLED console canonical plan

<!-- OLED_CONSOLE_ARCHITECTURE_PLAN_CANONICAL -->

Read together:

- `docs/OLED_SSD1306_HARDWARE_PROFILE.md`
- `docs/OLED_CONSOLE_ARCHITECTURE.md`
- `docs/OLED_CONSOLE_API_CONTRACT.md`
- `docs/OLED_CONSOLE_IMPLEMENTATION_PLAN.md`
- `docs/OLED_CONSOLE_ACCEPTANCE_PLAN.md`

Canonical framed text geometry:

```text
safe graphics interior: x=1..126, y=1..30
font: 5x7
cell advance: 6x8
text y origins: 8, 16, 24
capacity: 21x3
```

No hidden Y remap.
No 2x text workaround.
No 128x64 assumptions.

## Current implementation sequence

- [x] Slice 0: restore known-good baseline.
- [x] Slice 1: isolate SSD1306 driver.
- [x] Slice 2: isolate monochrome framebuffer and font.
- [x] Hardware correction: identify and prove native 128x32 panel profile.
- [ ] Slice 3A: commit native 128x32 source + canonical documentation.
- [ ] Slice 3B: implement and physically accept generic opaque text renderer.
- [ ] Slice 3C: add framebuffer-equivalent aligned fast path.
- [ ] Slice 4: retained 21x3 console without scrolling.
- [ ] Slice 5: circular 3-row scroll.
- [ ] Slice 6: dirty-page presentation optimization.
- [ ] Slice 7: optional kernel-log integration.

## Development loop

For every hardware-visible change:

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART
 -> physical visual
 -> PASS/FAIL
 -> evidence
```

Do not skip physical acceptance for OLED rendering changes.

## Build discipline

- `-Wall -Wextra -Werror`
- `git diff --check`
- no warning suppression
- no implicit package installation
- no unrelated Git changes
- dynamic ELF symbol resolution
- fail closed on unexpected tree state

## Flash discipline

Close STM32CubeProgrammer GUI before CLI access.

SWD fallback order:

```text
4000 KHz
1000 KHz
400 KHz
```

Once flashing begins, source must remain matched to the flashed image.

## UART regression

Minimum:

```text
BOOT OK
ping -> PONG
```

For OLED work also require:

```text
i2cscan -> 0x3C
oledping -> OLED_CMD_OK
```

Use the specific rendering command and require its `*_OK` response before
physical display judgment.

## Fault diagnostics

`fault_record` resides in BSS and is cleared on reset.

Its address is build-dependent. Tooling must resolve it from the current ELF.

OLED must never be the only fault-reporting sink.
