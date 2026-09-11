# OLED Console Acceptance Plan

Status: CANONICAL / NATIVE 128x32

## 1. Global hardware invariants

Every OLED-related production slice must preserve:

```text
address      = 0x3C
resolution   = 128x32
framebuffer  = 512 bytes
pages        = 4
A8           = 0x1F
DA           = 0x02
D3           = 0x00
start line   = 0
segment map  = A1
COM scan     = C8
window       = columns 0..127, pages 0..3
```

The previous 128x64 configuration is explicitly rejected.

## 2. Build gate

Required:

- `-Wall -Wextra -Werror`;
- `git diff --check`;
- valid initial MSP;
- exactly one `oled_framebuffer` symbol;
- `oled_framebuffer` size exactly `512`;
- resolve `fault_record` dynamically from ELF;
- no hardcoded RAM addresses in acceptance tooling.

## 3. Boot/UART gate

After every flash:

- `BOOT OK`;
- boot `FAULTREC` address matches the current ELF;
- `ping -> PONG`.

## 4. I2C/OLED gate

Required:

- I2C scan sees `ADDR=0x0000003C`;
- count remains one for the current bench setup;
- `oledping -> OLED_CMD_OK`.

After reset/flash, explicitly issue the OLED rendering command before judging
physical output.

## 5. Native baseline regression

`oledtext` must return:

```text
OLED_TEXT_OK
```

Physical result:

- full frame at `x=0..127`, `y=0..31`;
- centered 1-pixel-thick `DEUS OS`;
- no doubled vertical pixels;
- no alternating missing rows.

## 6. Generic text renderer acceptance

Before any fast path is introduced:

- use opaque 6x8 cells;
- render aligned rows at y `8`, `16`, `24`;
- verify blank spacer column overwrites stale pixels;
- verify blank spacer row overwrites stale pixels only inside clip;
- overwrite one glyph with another and confirm no ghost pixels;
- verify bottom frame at `y=31` survives row origin `y=24`.

Clip rectangle:

```text
x=1, y=1, width=126, height=30
```

## 7. Fast-path acceptance

The page-aligned fast path must not be accepted merely because it looks good.

Required:

- same input string/cell sequence;
- generic renderer framebuffer snapshot;
- optimized renderer framebuffer snapshot;
- byte-for-byte equality for the affected framebuffer;
- identical physical display.

## 8. Console no-scroll acceptance

Canonical framed capacity:

```text
21 columns x 3 rows
```

Expected text origins:

```text
row 0 -> y=8
row 1 -> y=16
row 2 -> y=24
```

Must verify:

- three readable rows;
- no text corruption;
- no frame corruption;
- wrap at column 21;
- newline semantics;
- opaque overwrite behavior.

UART protocol success is necessary but not sufficient. Physical inspection is
mandatory.

## 9. Scroll acceptance

Write at least five logical lines into the 3-row console.

Verify:

- newest three logical lines remain;
- row order is correct;
- no framebuffer ghosting;
- no hardware scroll mode is used;
- border remains intact.

## 10. Dirty-page acceptance

Dirty-page mask uses bits 0..3 only.

Required:

- correct page marking;
- no false clearing after failed transfer;
- successful partial present matches full-frame output;
- full-frame fallback remains functional.

## 11. Safety regression

UART must remain responsive after OLED operations.

Fault diagnostics must remain available even if OLED/I2C fails.

## 12. Git gate

For every rendering behavior change:

```text
source
 -> build
 -> validate
 -> flash
 -> verify
 -> reset
 -> UART
 -> physical visual PASS
 -> commit
 -> push
```

Never commit an OLED rendering change before physical acceptance.
