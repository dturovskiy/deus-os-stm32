# Deus Node Link v0 — portable C11 codec (ESP01-007)

This standalone source module implements the **owner-frozen offline wire contract**
`A000016 / E002 / T006 / G001`: Node Link **REV2** candidate
SHA-256 `899E2FBF4366CE91C31D6B1E6B481195DEECE5DF21AD4495F78E1D450DE406BA`.
The exact accepted private specification is outside this Git checkout in
`../Coordination/tasks/ESP01-006/artifacts/A000016__E002__T006__G001__NODE_LINK_V0_CANDIDATE_REV2_20261101.md`.
Its accepted owner source-only receipt is under sibling `Coordination/receipts/`.
These are **local workspace paths, not relative links from this README**.

## Files, ownership, and integration boundary

- `include/deus_node_link.h` — public transport-independent C API.
- `src/deus_node_link.c` — pure C11 CRC16 CCITT-FALSE, IEEE CRC32, wire encode/decode,
  bounded incremental stream parser, minimal STM32-controller/ESP-worker reference FSM.
- `tests/node_link_tests.c` — deterministic C host tests, ten byte-identical REV2
  frames, negative framing/sequence/session/correlation/retry cases.
- Private reproducible host runner:
  sibling `Tools/scripts/esp8266/build_test_node_link_c_host.py`.
  Host binary is built in an external temporary directory and deleted after testing.

The module **does not** initialize UART, GPIO, USB or any SDK/HAL peripheral, and
contains no dynamic allocation or MCU-specific register access. It **is not**
included automatically in the existing STM32 firmware source list and does not
modify any of the accepted bootloader, USB, IWDG, OLED or USART1 build artifacts.
The standalone ESP8266 SDK v3.4 app in `OS/nodes/esp8266/firmware/` already
builds this exact shared codec through `main/node_link_impl.c`; owner native
Xtensa v4 source build was accepted as ESP01-008 and the offline resource/link
emulator as ESP01-009. **STM32 USART2 has not been implemented or linked**:
its future source/object list, IRQ38 vector, budget and hardware admission
remain separate. Neither accepted off-device step authorized ESP Flash.

## Wire / memory limits

Frame starts `D5 4E`, major=1, node=1, little-endian sequence (1..65535),
payload 0..24, REV2 `session_id:u32` at offset 10, CRC16 over bytes 2..13+N.
Total `16+N`, maximum **40 bytes**. HELLO carries zero session and nonzero
challenge; HELLO_ACK and subsequent messages carry session=challenge. A controller
only accepts reply type, sequence, session and (for CRC32) request length that
match one outstanding request. Worker returns cached reply for the last
byte-identical request and does not repeat CRC jobs on duplicates. The stream
parser uses a fixed 40-byte buffer and a 50-ms partial-frame deadline.
Controller response deadlines are **provided as values** (200/500 ms);
they must be scheduled by a future nonblocking transport adapter. No ISR or
real-time hardware guarantee is inferred from host testing.

## Host build example (outside repo output)

Run from `OS/shared/node_link` with a user-selected temporary output location:

```sh
cc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror \
  -I include src/deus_node_link.c tests/node_link_tests.c \
  -o /tmp/deus_node_link_host_tests
/tmp/deus_node_link_host_tests
```

Do not track compiled binaries. In the managed DEUS MCP workspace, use the
existing off-Git Python runner to compile at `-O0` and `-O2` and capture
source SHA plus original host test output; the runner requires an accepted
source-only freeze receipt.

**Current evidence boundary (2026-11-01):** Portable codec Linux host GCC
15.2.0 actual `-O0`/`-O2` C11 `-Werror` PASS 20/20 each and UBSan `-O1`
PASS 20/20. AddressSanitizer could not reserve shadow memory and **ASan is
not claimed PASS**. Separately, the ESP8266 app native Xtensa SDK v3.4 build
(ESP01-008) and the SDK-config-derived resource/link emulator audit
(ESP01-009) were accepted off-device. No STM32 target build including this
codec, live STM32 ↔ ESP UART traffic, ESP boot/Flash or runtime free heap
has been verified.
The current operator task disposition and detailed source receipts are
exclusively in sibling `Coordination/INDEX.md`; global product milestone
truth remains in `OS/docs/CURRENT_STATE.md`.
