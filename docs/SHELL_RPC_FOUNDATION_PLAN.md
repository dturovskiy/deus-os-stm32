# STM32 OS — Transport-Neutral Shell/RPC Foundation Plan

Status: **GATES 0–5 ACCEPTED — GATE 6 LOCAL ACCEPTANCE COMMIT NEXT**

Boundary ID:

`SHELL_RPC_FOUNDATION`

## 1. Published baseline

This boundary starts from the fully published USB CDC ACM console foundation:

- commit `5a8a45618b87b3069fd7cbac6119035b6ac4ad2c`;
- tree `bbc6b24e28279075c41410e8abdace89c4b805b7`;
- subject `feat: add USB CDC ACM console foundation`;
- accepted firmware `58548` bytes / SHA-256 `D01AC5B281DA4D0E97BB778918F39684C4E8160AD690F04881B45395BDA8F0AE`;
- UART and native USB CDC are both hardware-accepted command transports;
- Windows `usbser.sys`, physical USB reconnect, software-reset attach and post-IWDG automatic CDC recovery are accepted.

Roadmap sequence remains:

```text
native USB Device core
    -> CDC ACM console
    -> transport-neutral shell/RPC foundation   <- this boundary
    -> binary framed transport
    -> host control application
    -> USB firmware update / recoverable bootloader
```

## 2. Exact current command path

The published implementation in `src/kernel.c` currently has:

- a `32`-byte command line buffer per transport;
- one parser state for UART and one parser state for USB CDC;
- CR/LF command termination and backspace/delete handling;
- a `console_transport_t` UART/CDC enum;
- a global `console_output_transport` selecting the active response sink;
- `console_execute_named()` dispatching exact command strings through chained comparisons;
- scheduler diagnostic dispatch separated from the safe command surface;
- `wdogtrip` as a destructive non-returning command;
- `schedtimed` deriving its wake event from the active transport;
- task0 draining the UART and CDC RX rings and executing all command policy in Thread/PSP context.

Published source fingerprints before this boundary:

```text
src/kernel.c                    cb78cbb9ad1ca0513e7673760a9fc3fb4ed5fe7f67847d9f984400e6b17215f7
src/drivers/usb_device.c        a7fe1ccb71ea4c36a306607e4a11aedebca6ababa3d1356584a61e2eeafb5af3
include/drivers/usb_device.h    69915df563cb259830ee2f473764eb32c9ac4bb30e94bb846101914bd76a7fd5
src/kernel/scheduler.c          b59b08373662b841c2cc077c92de18d7fa21da6dce4e1dee435f86ab58566c88
include/kernel/scheduler.h      a48dcbb90d08fad03f2d426aa2a129a8d8858204380d4a518d7094a318f5d841
src/startup.s                   b72063bd58ee01fc6ee42d10c765eac9b47381e18d01051e1557c7fda80d6461
src/drivers/iwdg.c              dfbcc9479135064b778f952b9e9adce370affd870b5be44ea6af3b4d0ecee3c0
```

## 3. Boundary objective

Separate **command semantics** from **text framing** and from **physical UART/USB transports** without changing the accepted scheduler, USB, watchdog, or OLED architecture.

The result must support two front ends over one command service:

```text
UART RX ring -> text shell adapter --+
                                    |
USB CDC RX -> text shell adapter ----+--> command service --> handlers
                                    |          |
future binary RPC adapter -----------+          +--> response sink
```

This boundary does not add the future binary wire format. It creates the in-process service contract that the binary adapter will later call.

## 4. Command service contract

Gate 1 should introduce a small static command-service module, expected as:

```text
include/kernel/command_service.h
src/kernel/command_service.c
```

The service contract is deliberately bounded and allocation-free.

A request contains:

- canonical method name;
- `argc` and bounded `argv` tokens;
- execution context supplied by the caller.

Initial limits:

```text
text line capacity   32 bytes including terminator reserve
maximum argv tokens  4
heap allocation      none
```

The existing line capacity is retained in this foundation. Increasing command payload capacity belongs to a consumer-driven later change, not this refactor.

## 5. Execution context and response ownership

The command service must not depend on UART or USB identifiers.

The execution context carries only capabilities needed by command semantics:

- response writer callback;
- opaque writer context;
- source RX-event mask for diagnostics such as `schedtimed` that must wait on the same originating stream.

UART and CDC adapters bind their existing write path and scheduler event into this generic context.

All command execution remains serialized in production task0, so one active execution context at a time is an explicit invariant. IRQ/Handler mode may publish bytes/events only; it never dispatches a command.

## 6. Stable status taxonomy

The service layer should return a small semantic status rather than embedding transport policy:

```text
OK
NOT_FOUND
BAD_ARGS
BUSY
INTERNAL_ERROR
```

The service status describes dispatch/execution of the registered method, not the physical success of every method-specific diagnostic. `OK` therefore means the service path completed; commands that perform hardware/self-tests continue to report their operational result through their established payload tokens such as `OLED_*_OK/ERR` and `SCHED_*_OK/ERR`. A failed response writer is latched in the execution context and is promoted to `INTERNAL_ERROR` when the handler returns.

The text shell keeps backward compatibility:

- unknown method -> exact legacy `ERR`;
- invalid argument count/syntax -> exact legacy `ERR`;
- scheduler lifecycle rejection -> exact legacy `SCHED_DIAG_BUSY` where already required;
- successful and failed legacy method-specific diagnostics retain their accepted payload tokens.

`wdogtrip` remains a deliberate destructive/non-returning method after emitting `WDOG_TRIP_ARMED`.

## 7. Static method registry

Replace the monolithic string-comparison dispatch with one deterministic static registry.

Each descriptor owns at minimum:

- method name;
- command class/flags;
- minimum argument count;
- maximum argument count;
- handler binding or internal dispatch key.

Initial command classes:

```text
SAFE
DIAGNOSTIC
DESTRUCTIVE
```

The registry is a source-level service catalog, not yet a public binary protocol ABI. No numeric wire method IDs are committed in this boundary.

All existing published command names remain valid.

Two foundation introspection methods are authorized:

- `help` — zero arguments lists registered method names; one argument returns metadata for that method;
- `rpcinfo` — reports foundation version, registry count, line capacity and argument limit.

`help <method>` is the first bounded argument-bearing shell operation and is the acceptance proof for tokenization without inventing a product-control feature.

## 8. Text shell adapter

UART and CDC keep independent parser state.

The text adapter retains:

- CR or LF termination;
- empty-line ignore;
- backspace and DEL editing;
- bounded overflow rejection;
- no heap;
- per-stream partial-line isolation.

It adds bounded in-place tokenization:

- method is token 0;
- arguments are separated by ASCII space or tab;
- repeated separators collapse;
- maximum `4` argument tokens;
- quotes/escaping/globbing/environment expansion are explicitly not implemented.

This is an embedded diagnostic shell, not a POSIX shell.

## 9. Output compatibility

Existing response payloads are acceptance contracts for this refactor. Gate 1 must avoid gratuitous output changes.

The current misleading helper names such as `uart_write*()` may be renamed or wrapped only as needed to make response ownership transport-neutral. The critical rule is semantic: command handlers write through the active generic response sink, not by selecting UART/CDC directly.

Boot/fatal emergency output may remain UART-specific because it is outside normal command execution and is an intentional recovery path.

## 10. Scheduler and watchdog ownership

No new production task, scheduler state, priority, SVC, queue, mutex, semaphore, or timer subsystem is authorized.

Task0 remains the sole normal command executor.

Task1 remains heartbeat.

IWDG ownership remains unchanged:

- no command parser/registry/transport IRQ reload;
- no USB/UART Handler reload;
- production progress in Thread/PSP remains the reload authority;
- `wdogtrip` still intentionally stops progress/reload and causes a real reset.

## 11. USB/UART ownership

USB CDC and UART are already accepted transports and should not be redesigned here.

Expected guards:

- `src/drivers/usb_device.c` unchanged;
- `include/drivers/usb_device.h` unchanged;
- USART1 register/ring/IRQ ownership unchanged;
- USB endpoint/PMA/descriptor/class code unchanged;
- physical USB reconnect and post-IWDG auto-attach remain accepted regressions.

## 12. OLED scope

OLED/gfx/status-bar source remains frozen.

If exact guard hashes remain unchanged and `uiruntime` passes in Gate 3, Gate 4 remains:

`PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`

## 13. Expected Gate 1 source scope

Expected source changes:

```text
include/kernel/command_service.h     new
src/kernel/command_service.c         new
src/kernel.c                         adapt registry/parser/response context
```

This is an expected scope, not permission to mutate unrelated files if implementation discovery finds a different need. Any scheduler/USB/startup/linker/OLED scope expansion must be justified before mutation.

## 14. Non-goals

Not in this boundary:

- binary framing or packet protocol;
- numeric public RPC method IDs;
- file transfer;
- structured telemetry streaming;
- host GUI/control-panel implementation;
- firmware update protocol;
- bootloader;
- USB Host/HID;
- networking/ESP8266;
- heap allocation;
- generic timers;
- message queues/semaphores/mutexes;
- new production tasks or SVCs;
- runtime-statistics subsystem;
- OLED UI redesign.

## Accepted implementation record — 2026-09-16

Gate 1 produced the explicit-context allocation-free command service described above. Gate 2 accepted candidate tree `e136814480ac0760bc5dd62a78ebca4e07f0ba98`, BIN `37196` bytes / SHA-256 `90534921EA966235D3F3C72AE65F1684D62FA6A122762E64BCF4972A5C39EA60`, with Flash `37196` bytes and SRAM `9216` bytes. Gate 3 accepted the full UART/CDC shared-service contract and retained scheduler/watchdog/USB regressions, including physical reconnect and automatic post-IWDG `usbser` recovery. Gate 4 is `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS`; Gate 5 docs/evidence finalization is complete. Gate 6 local acceptance commit is next.

## 15. Gate order

```text
Gate 0  planning/docs synchronization
Gate 1  exact source investigation + command-service implementation
Gate 2  fresh GNU build/link/static validation
Gate 3  UART + CDC shell/RPC hardware acceptance and retained regressions
Gate 4  OLED conditional N/A review
Gate 5  docs/evidence finalization
Gate 6  local acceptance commit
Gate 7  ordinary non-force publication
```

Never combine source mutation, build, hardware acceptance, commit and push into one gate.
