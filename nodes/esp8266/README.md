# Deus OS — ESP8266 node 1

Status: **ESP01-008 accepted off-device native Xtensa SDK v3.4/GCC8.4 build; ESP01-009 accepted off-device link-emulator and config-derived resource audit (2026-11-01).** ESP01-008 original Evidence SHA `4F2FDAC6D76B3E6F275F3183673E989C970CFEBE2CD3F4ABCB2CEB0A49B4D48D`; ESP01-009 original owner v3 Evidence SHA `787A9BA39F8594BA22FCB3217BC5BA1E6317F51D6787AE98C90F3739E083B378` verified CRC6/6 and indexed SHA5/5. **Next: ESP01-010 STM32 USART2 source-only feasibility, PARTIAL pending fresh ARM resource/stack budget.** No physical ESP boot/Flash, STM32 UART link or runtime free-heap proof. Current global state is `OS/docs/CURRENT_STATE.md`.

## Ownership

- Target: physical ESP-01 / ESP8266EX, SPI Flash JEDEC `ef:4014`, **1 MiB** confirmed by owner via Ubuntu CH340 and existing `esptool v4.7.0`.
- Planned independent C firmware: `firmware/`; target tests: `tests/`.
- Shared bounded transport-neutral Node Link REV2 **is now implemented** under `../../shared/node_link/` as pure C11 (header/source/tests/README), frozen owner source SHA `899E2FBF4366CE91C31D6B1E6B481195DEECE5DF21AD4495F78E1D450DE406BA`; actual host C11 GCC O0/O2 20/20 each and UBSan 20/20 PASS. No automatic STM32 source-list inclusion, HAL coupling, or device integration.
- Linux **Ubuntu Mac mini** runs SDK/Xtensa and hardware serial; Windows `deus-run` orchestrates SSH/SCP. DEUS MCP Windows/WSL repo access is **not** an Ubuntu shell.
- Exact SDK/GCC provenance is `toolchain.lock`. Installed SDK, compiler, Python environments, build directories and generated ELF/BIN never belong in `OS/`. Keep them in a dedicated **Ubuntu off-Git** directory. Operator builders/sources live under Windows workspace sibling `Tools/scripts/`, executable ZIPs under sibling `Bundle/`, original run evidence under `Evidence/`, original Flash backups under protected `ProtectedRecovery/`.

## Current safe next step

**ESP01-010 STM32 source-only feasibility** is active; its first read-only scan passed, but current-source ARM Flash/SRAM/stack budget remains unmeasured. ESP01-008's pinned native Xtensa build and ESP01-009's pinned native 1MiB image-offset/static-ELF review are already accepted in their bounded off-device scopes; **do not rerun unchanged builds or audits**. ESP01-009 original Evidence ZIP SHA256 `787A9BA39F8594BA22FCB3217BC5BA1E6317F51D6787AE98C90F3739E083B378`. Original external `ESP-01 Lamp Control` firmware is not replaced; no ESP Flash, USB/UART/reset, live STM32 link or runtime heap measurement has occurred. The current step is to obtain a fresh exact-source STM32 build budget and design USART2 integration without altering existing device firmware.

**Historical milestone, not current next-step instruction (2026-11-01):** first 1MiB no-device `hello_world` native Ubuntu Make build PASS via Windows v4 operator (original Evidence SHA `98CE311143E7B730AF5A25300AB763FEC9640708B03168C42324202432B8F781`). Toolchain, Python checker and hello_world were accepted before Node Runtime design. **Standalone Node Runtime has now also passed real Xtensa SDK build** under ESP01-008; current next offline task is ESP01-009 Flash geometry/resource and emulator verification, not repeated source implementation or installing tools. ESP Flash backup/UART/reset/Flash still require explicit separate owner approval.

**Historical next step (now completed 2026-11-01):** isolated Ubuntu Python3.11.14 and ESP8266 SDK hello_world 1MiB build were completed by owner native v4 PASS; original Evidence SHA `98CE311143E7B730AF5A25300AB763FEC9640708B03168C42324202432B8F781` independently checked. This previous pending instruction is superseded. Hardware ESP-01 remains untouched, and the next active engineering step is firmware/protocol source plus offline unit tests, not repeated provisioning or Flash.

Scoped design: `docs/ESP01_DISTRIBUTED_NODE_V0_PLAN.md`; global status `docs/CURRENT_STATE.md`; sequencing `docs/ROADMAP.md`. Do not create a second per-node global state file.
