# STM32 OS — Harness, Evidence, Failure Classification & Recovery Playbook

Status: project engineering rulebook
Scope: Windows-orchestrated build/evidence scripts, Windows-owned UART and STM32CubeProgrammer/ST-LINK hardware access, SSH/SCP coordination with the Ubuntu/Mac-mini USB execution domain, Linux libusb runtime acceptance, Git evidence gates.

## 1. Purpose

This playbook exists because repeated failures can come from very different layers:

- the OS/product implementation;
- the PowerShell harness;
- host transport/protocol handling;
- stale assumptions copied from older milestones;
- environment/hardware state.

The first task after any failure is **classification**, not patching.

No source change is allowed until the failure is causally tied to product behavior.
No new acceptance script is allowed until the previous harness failure is understood from its evidence bundle.

---

## 2. Mandatory failure classes

Every failed gate MUST be classified as one of:

### HARNESS
The product did what it should, but the script interpreted it incorrectly.

Examples from this project:

- PowerShell empty pipeline became `$null`.
- Empty string element rejected by a mandatory `string[]` parameter.
- `git grep -E` escaping produced an invalid regex.
- Case-sensitive search for `primask` missed GNU objdump `PRIMASK`.
- Historical fixed Reset vector `0x08000041` was incorrectly reused after the vector table grew.
- Generic substring `_ERR` falsely matched `RX_ERROR_COUNT`.
- Stale console alias `schedrun` was used instead of accepted `schedcoop`.
- Runtime transport token `ERR\r\n` was incorrectly treated as a per-command source token.

### PRODUCT
The hardware/runtime output proves that the current source behavior violates the intended contract.

Current example:

- `schedwaitwake` received UART event correctly but consumed `0x0A` instead of sentinel `0x57`.
- Root cause: the synchronous console executes on CR before the trailing LF of CRLF is drained. The diagnostic consumer assumed “UART event == sentinel payload” instead of re-checking the RX FIFO/condition.

### ENVIRONMENT
The product/harness cannot run because the host or probe state is wrong.

Examples:

- COM3 missing.
- CH340 not `OK`.
- STM32CubeProgrammer GUI owns ST-LINK and CLI returns `DEV_CONNECT_ERR`.
- Wrong ST-LINK identity or firmware.
- Target powered only through unintended signal/reference backfeed while the board's normal supply is absent; an ST-LINK voltage reading alone is not proof that the MCU is validly powered. Hardware gates must separately record host USB presence (when USB is the normal board supply), target voltage, and SWD core access before classifying a physical-link failure.
- Tool executable/path missing.

### EVIDENCE / STATE DRIFT
The repository, candidate, source hashes, branch, remote, or accepted evidence no longer match the required precondition.

Examples:

- wrong candidate path (`build\os.bin` vs milestone-specific candidate);
- dirty set differs from the exact accepted files;
- origin/main moved unexpectedly;
- evidence file hash mismatch.

---

## 3. Evidence-first rule

For every failure:

1. Read the `.log`.
2. Open the `.zip`.
3. Inspect the exact failing command stdout/stderr.
4. Inspect source/build artifacts already captured.
5. Compare with the previous accepted workflow for the same subsystem.
6. Only then classify the failure.
7. Only PRODUCT failures may trigger source changes.

If the ZIP does not contain enough information to classify the failure, the **next script is a collector**, not a patch.

Build/link collectors MUST persist the highest-value diagnostic artifacts before asserting success. For GNU Arm application/bootloader links this means, when emitted, copy the linker MAP, raw linker stdout/stderr, command metadata and relevant object/NM attribution before `ExitCode == 0` or artifact-existence assertions. A failed link must never be followed by a generic `os.bin missing` classification that discards the already-emitted MAP. Measurement collectors must report failed-link absence of ELF/BIN as measurement data, not as a second product failure. Failure classes: `HARNESS-FAILURE-ARTIFACT-ORDER-01` and `HARNESS-MEASUREMENT-POSTFAIL-ARTIFACT-01`.

Artifacts that intentionally embed candidate identity (for example `DEUS_FIRMWARE_SOURCE_TREE_HEX`) MUST NOT be raw-SHA-pinned across an authorized change to that identity domain. The harness must separately prove the current embedded identity equals the current candidate identity and, when behavior-preserving equivalence to an older accepted artifact is required, normalize only the documented identity field and compare the normalized artifact hash to the older accepted hash. Raw artifact SHA drift caused solely by an authorized embedded-identity update is not a product drift. Failure class: `HARNESS-IDENTITY-BEARING-ARTIFACT-BASELINE-01`.

### Evidence surface ownership

Do not assume that every accepted fact is repeated in every evidence file.

Each evidence surface has an owner role:

- `run.log` — chronological commands, stdout/stderr, runtime diagnostics and terminal PASS/FAIL marker;
- `outcome.txt` — structured final state such as `OUTCOME`, runtime failure count, mutation flags, candidate identity and policy flags;
- hash manifest / candidate files — byte identity;
- Git status/diff evidence — repository mutation state.

A gate must assert a fact against the evidence surface that actually owns that fact.

Example from this project:

- `RUNTIME_FAILURE_COUNT=0` is written to the hardware acceptance `outcome.txt`;
- it is **not** guaranteed to appear in the external `run.log`;
- therefore a later docs gate must read `outcome.txt` from the accepted ZIP instead of searching the hardware log for that token.

Never promote a token from one evidence surface into a required token on another surface unless the producing script explicitly guarantees both.

When a recovery/composite harness consumes structured prior evidence, it MUST validate each file against that file's actual emitted schema. Do not derive expected key names from a human-readable `run.log` line and then search for those prose tokens inside a structured proof file. For example, if `gate2-static-resource.txt` owns `STATIC_SRAM_DATA_BSS=9856`, a recovery harness must parse that exact key/value; it must not require the `run.log` rendering `static_sram=9856/12288`. Before delivery, replay every prior-evidence assertion against the exact embedded ZIP bytes. Failure class: `EVIDENCE-SCHEMA-SURFACE-MISMATCH-01`.

Static source assertions in a harness MUST prove stable contract semantics or public diagnostic surfaces, not incidental implementation-private identifiers. When a footprint/refactor optimization intentionally deletes or renames a private field/helper while preserving the contract, the harness must be regenerated and replayed against the exact candidate source snapshot before delivery. A stale assertion such as requiring a deleted private `mutation_start_cycles` field after the mutation-start timestamp was intentionally stored in the public volatile `last_mutation_cycles` diagnostic is a harness defect, not a product failure. Failure class: `HARNESS-STALE-SOURCE-ASSERTION-01`.

When an acceptance package already locks an exact path set and SHA-256 for the candidate, that byte-identity authority MUST NOT be redundantly reimplemented as broad whole-file substring checks. A check such as `header.Contains('uint32_t wire_capacity);')` is invalid if the token can legally belong to a different declaration in the same file. If a source-level semantic assertion is genuinely required beyond exact byte identity, it must be scoped to the owning syntactic unit or derived from compiler/linker/test evidence, and the generator MUST replay that exact assertion against the exact candidate bytes before delivery. Documentation prose is not a stable machine oracle unless the document explicitly defines a machine-readable token as normative. Failure classes: `HARNESS-SOURCE-ASSERTION-SCOPE-01` and `HARNESS-DOC-PROSE-ORACLE-01`.

Every generated operator-run package MUST pass an offline assertion replay before delivery. The replay executes every deterministic pre-build/pre-hardware assertion against the exact candidate/materialized inputs that the operator package will consume. A package is not deliverable merely because its PowerShell AST parses and its ZIP/hash inventory is valid. If any deterministic package assertion has not been exercised against the exact candidate bytes, generator audit is incomplete. Failure class: `HARNESS-UNREPLAYED-DETERMINISTIC-ASSERTION-01`.

---

## 4. Collector rule for unavailable host/hardware facts

ChatGPT cannot directly execute the operator's installed Windows GNU toolchain, STM32CubeProgrammer/ST-LINK/CH340 hardware or the physical USB connection owned by the remote Ubuntu/Mac-mini host.

Therefore:

- never guess those facts;
- first recover them from canonical project documents and accepted evidence;
- if still unknown, generate a **read-only collector** on the execution domain that physically owns the fact;
- the default return surface is one self-contained evidence ZIP containing `run.log`, `outcome.txt`, raw stdout/stderr and hashes; a loose log is optional compatibility output only;
- a collector must not edit source/docs, build unless explicitly needed, flash, commit or push.

The canonical operational topology is `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`. Current stable facts include:

- canonical repository `D:\Projects\STM32\OS` on Windows;
- PowerShell 7 orchestrates operator-run packages;
- Arm GNU Toolchain 15.3.Rel1;
- STM32CubeProgrammer 2.23;
- CH340 / USART1 `115200 8N1` on Windows; historical COM number `COM3` is an enumeration fact, not a permanent identity;
- ST-LINK V2J48S7 on Windows, accepted SWD speed `950 kHz`, read-only fallback ladder `950/480/240/125 kHz`;
- target native USB is physically attached to Ubuntu on the Mac mini and is exercised there through libusb/`cdc_acm`;
- Windows reaches that USB execution domain via `deus@macmini` / SSH alias `macmini`;
- the target USB cable is also the normal target power/VBUS source in the current bench.

Do not ask the operator to repeat stable facts already recorded. Do not infer that a physical interface is absent merely because it is not visible on the wrong host.

---

## 5. Script preflight before handing a script to the operator

Every new script must be validated against **current artifacts**, not memory.

### Primitive-first composition contract — mandatory

Acceptance harnesses MUST be built **primitive-first**, not by repeatedly patching a monolithic `v1 -> v2 -> v3 -> ...` collector after each operator-visible failure.

Before a composed acceptance collector is handed to the operator, every non-trivial external primitive used by that collector MUST be proven independently on the **actual execution domain where it will run**. Static lint, AST parsing, ZIP/hash verification and source inspection are necessary preflight, but they are **not sufficient proof of runtime semantics**.

At minimum, prove the primitives that the collector actually depends on, including where applicable:

- Windows / PowerShell process invocation, argument passing, stdout/stderr capture, timeout, cancellation, elevation and child-process lifetime semantics on the real Windows host;
- OpenSSH alias resolution, non-interactive authentication, `ssh` command execution, `scp` transfer, timeout behavior and remote cleanup across the actual Windows -> Linux endpoint path;
- Git archive creation from the canonical repository plus extraction and path semantics on every target OS that will consume the archive;
- .NET SDK / Microsoft.Testing.Platform / xUnit invocation using the exact installed SDK/runtime family on each execution host, including the real test entrypoint and accepted exit-code/result-count semantics;
- UART, ST-LINK, WinUSB/libusb or other hardware-access primitives on the host that physically owns that interface whenever the acceptance collector will exercise them.

A primitive proof is an **executed proof**, not an inferred one. It must use a representative real artifact/input and capture enough evidence to establish at least the command/arguments, execution host/domain, exit code, relevant stdout/stderr or result tokens, and bounded timeout/failure behavior. When portability is part of the collector contract, the primitive must be proven on both sides of that portability boundary rather than assumed from one host.

### Physical-interface execution-domain ownership — mandatory

Before composing any hardware gate, map each operation to the host that physically owns its interface. For the current bench, the binding in `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md` is authoritative:

- target USB / Linux libusb / Linux CDC observation -> Ubuntu on Mac mini via `deus@macmini`;
- ST-LINK / SWD / STM32CubeProgrammer -> Windows;
- CH340 / UART -> Windows;
- canonical repository, candidate construction and final evidence ZIP -> Windows.

A current-bench harness MUST NOT probe local Windows WinUSB/CDC and classify absence as target/environment failure when the USB cable is attached to the Mac mini. Likewise it must not try to run CubeProgrammer or CH340 acceptance on the Mac mini. A Windows-driven Flash/reset step that requires runtime proof must subsequently prove USB recovery/HELLO/RPC on the Mac-mini USB domain. Failure class: `HARNESS-EXECUTION-DOMAIN-OWNERSHIP-01`.

During pre-publication Asset Gates 1–5, system capability bit 5 remains intentionally OFF. The production CLI therefore uses `AssetAccessPolicy.PublishedOnly` and must reject normal `config` operations until publication. Acceptance harnesses that intentionally exercise the management carrier before publication must use the same transport-neutral Core with `AssetAccessPolicy.PrepublicationAcceptance` in an acceptance-only helper/runner. Do not patch the production CLI to bypass this policy and do not classify its expected pre-publication rejection as a product failure. Failure class for using the wrong access policy in an acceptance harness: `HARNESS-ASSET-ACCESS-POLICY-01`.

The WSL path `/home/deus/projects/deus-os-stm32/OS` is a Windows-side bind/view of `D:\Projects\STM32\OS`; it is not proof that the remote Mac mini has the same path or repository. Unless a remote repository is explicitly re-proven, cross-host harnesses materialize the exact required candidate/payload on Windows, copy it to a unique remote `/tmp` directory by bounded SCP, verify hashes remotely, execute there, copy raw evidence back, and clean the remote temporary directory. The final authoritative evidence ZIP is assembled on Windows.

Only after all required primitives pass independently may they be composed into one acceptance collector.

If the composed collector then fails in the HARNESS layer:

1. freeze the composed collector; do **not** repair only the last failing line and immediately issue another version;
2. classify the failure and isolate the smallest failing primitive or interaction between already-proven primitives;
3. reproduce and correct that primitive in a dedicated bounded proof on the real execution domain;
4. re-run the primitive proof until it passes with trustworthy evidence;
5. regenerate/recompose the acceptance collector from the proven primitives;
6. replay downstream assertions against existing evidence where possible before asking the operator for another full run.

Repeated `v1 -> v2 -> v3 -> ...` patch trains that discover one basic runtime-semantic defect per operator run are prohibited. A version increment is not evidence that a harness defect was understood. The required recovery unit is the failed primitive, not the monolithic bundle.

Artifact generators that run against a materialized clean candidate MUST treat source/provenance identity as explicit caller-owned input. They MUST NOT rediscover `HEAD`, candidate tree or other VCS identity through `.git` inside the clean candidate, because the canonical clean candidate intentionally contains no `.git`. A generator that needs `base_head`, `candidate_tree`, or `firmware_source_tree` receives and validates those values as parameters and records them verbatim in its manifest. This keeps artifact generation a deterministic function of explicit candidate inputs rather than the host repository topology. Failure class: `HARNESS-CLEAN-CANDIDATE-HIDDEN-VCS-DEPENDENCY-01`.

Byte identity MUST be compared within the same ownership domain. A live working-tree file hash is not interchangeable with the hash of the same logical path after `git add` normalization and materialization from a candidate tree. With attributes such as `* text=auto`, a Windows checkout may contain CRLF bytes while the Git blob/candidate archive contains LF-normalized bytes. Any manifest field that claims the hash of a build/recovery script used from the clean candidate MUST be checked against the bytes materialized from that exact candidate tree, not against the live checkout file. Conversely, WIP/source-lock evidence continues to own and check the live working-tree bytes. Primitive proofs for clean-candidate generators must therefore construct the representative root through the same `temporary index -> write-tree -> git archive -> extract` path used by the composed gate; direct `Copy-Item` from the working tree is insufficient when Git normalization can change bytes. Failure class: `HARNESS-CANDIDATE-BYTE-DOMAIN-MISMATCH-01`.

### Accepted-candidate freeze during destructive acceptance

Once a Gate-2-or-later candidate, recovery bundle or destructive campaign has been accepted and downstream evidence is cryptographically bound to it, repository-owned firmware/recovery script bytes that contribute to that identity are frozen for the remainder of that candidate's campaign. Do not "clean up" or modernize such scripts in place during Gate 3–5 merely because the global harness rulebook has since become stricter. If a required product/recovery script must change, stop the downstream campaign and regenerate/re-accept the source candidate, clean build, recovery bundle and every downstream precondition invalidated by that change. An outer acceptance harness may wrap a frozen candidate-owned utility with newer bounded process/evidence handling only when the wrapper does not alter the candidate-owned bytes or semantics.

### PowerShell native-argument cardinality — mandatory

When building a `string[]` for `ProcessStartInfo.ArgumentList`, every computed argument inside an array literal MUST be explicitly parenthesized or precomputed, for example `@('hash-object', ('--path=' + $path), $path)` and `@('-c','port=SWD', ('freq=' + $SwdKHz))`. Do not rely on PowerShell comma/`+` precedence inside a multi-item array literal. The harness must capture the final argument vector in the native command metadata and validate output cardinality when a command is expected to return exactly one token/line. Example: `git hash-object --path=<path> <path>` must produce exactly one 40-hex line; duplicated identical lines are a harness argument-construction failure, not evidence-state drift. Failure class: `HARNESS-POWERSHELL-ARGUMENT-CARDINALITY-01`.

Structured evidence records built as PowerShell arrays (for example `outcome.txt`) must likewise parenthesize every computed element so one key/value is emitted per physical line. A flattened single-line outcome is `HARNESS-EVIDENCE-RECORD-CARDINALITY-01`.

### Product-assertion authority and semantic-oracle rule — mandatory

A `PRODUCT_*` failure may be emitted only for an assertion that is directly required by a frozen product/acceptance contract and whose observer has already been proven trustworthy in that run. Every composed acceptance package must maintain an explicit assertion-authority map (in code comments, manifest data or a dedicated oracle table) linking each product-gating assertion to its owning contract requirement. Internal implementation diagnostics, convenience counters, RAM instrumentation and incidental symbol values that are not acceptance requirements may be recorded as evidence, but they MUST NOT become independent product gates merely because they are easy to read.

Do not encode a diagnostic transition counter as a state-identity constant. Derive expected runtime state from the contract-owned state machine first (for example `persisted valid record -> selected payload`, `no valid record -> compiled default`), then validate the contract-owned observable state. A counter such as `oled_ui_layout_revision` is implementation diagnostics: its value depends on whether activation changed the compiled default and is not, by itself, a Gate-5 product requirement. If such a diagnostic is retained, its expectation must be derived from the source-bound transition semantics and a mismatch is HARNESS/oracle drift unless a frozen contract explicitly makes that counter normative.

Before any destructive composed campaign, run the package's complete offline semantic oracle against all deterministic generated cases and all expected terminal states. The oracle must model the same validity/selection rules as the accepted candidate and prove the expected fallback/selection result without target mutation. Failure class: `HARNESS-SEMANTIC-ORACLE-01`.

A composed readiness poll MUST test only transient readiness conditions such as physical discovery/enumeration and, after presence is established, one bounded negotiation/identity/liveness probe. Deep acceptance assertions (persistence shape, scheduler margins, application lifecycle, fault counters, etc.) are not readiness predicates and MUST NOT be retried under a generic `NOT_READY` label. Once discovery succeeds, a deterministic contract assertion failure is surfaced immediately with its real error. Failure class for conflating deep acceptance with readiness and thereby masking the causal error: `HARNESS-READINESS-ORACLE-CONFLATION-01`.

Acceptance preconditions MUST be derived from the bound canonical prestate, not from a convenient state that would make a later test easier. In particular, optional persistent state must follow its accepted state machine (`valid committed record -> selected payload`; `no valid record -> compiled default`) and a harness MUST NOT require or create a committed record when the accepted prestate is clean/erased merely to enable a no-op-write check. Such an invalid assumption is harness oracle drift, not a product regression. Failure class: `HARNESS-ACCEPTANCE-PRECONDITION-01`.

Every terminal `FINAL_OUTCOME=FAIL`, regardless of PRODUCT/HARNESS/ENVIRONMENT classification, MUST return a nonzero process exit code to its launcher. Returning `0` while the structured outcome is FAIL is invalid harness evidence and may cause automation to promote a failed gate. Failure class: `HARNESS-TERMINAL-EXIT-CODE-01`.

Generated PowerShell source MUST distinguish Markdown backticks from PowerShell escape characters. Do not embed Markdown code-span backticks inside double-quoted PowerShell runtime strings; a trailing Markdown backtick before the closing quote escapes that quote and can turn a visually balanced script into a parser failure. Prefer plain assertion tokens or single-quoted literals for documentation text. Static authoring audit must reject a backtick immediately preceding a double-quote in generated PowerShell. Failure class: `HARNESS-POWERSHELL-MARKDOWN-BACKTICK-01`.

For destructive watchdog acceptance, observing `WDOG_TRIP_ARMED` proves only that reload starvation has begun; it does **not** prove that reset has already occurred. A composed collector must synchronize on an actual reset-completion event before opening a post-IWDG management session: use an independent accepted channel (for example UART recovery plus `IWDG_RESET=1`) or an equivalently explicit USB session-generation transition. A fixed short sleep followed by mere USB presence is invalid because the pre-reset USB device may still be enumerated while the console task is intentionally spinning; the ensuing reset can then interrupt the first post-IWDG transfer and create a false transport timeout. Failure class: `HARNESS-IWDG-RESET-SYNCHRONIZATION-01`.

### Endpoint-protection / AMSI compatibility — mandatory

A PowerShell script rejected by AMSI/endpoint protection during parse or load is a HARNESS delivery failure, never a product result. No acceptance package may instruct the operator to disable Defender/AMSI, add exclusions, change antimalware policy, obfuscate the script, encode it to evade inspection, or otherwise bypass the security control. Preserve the exact blocked package/hash and inspect the endpoint-protection event/detection record to identify the rule family. If the accepted workflow has grown into a large process-orchestration state machine, recompose it into a conventional hash-bound executable (prefer the already accepted .NET/`ProcessStartInfo` model) with PowerShell limited to a small package-integrity/bootstrap surface. Failure class: `HARNESS-AMSI-DELIVERY-01`.

### Generated .NET compiler gate — mandatory

A generated `.NET` acceptance engine is not delivery-ready merely because its source passed lexical/static scans. Locked restore and Release compilation are part of the harness proof. Where the authoring environment has a compatible SDK, compile before packaging. Where it does not, the operator-side bootstrap must perform locked restore/build **before any SSH, ST-LINK, USB, UART or target I/O**, must persist full compiler stdout/stderr on failure, and must terminate with zero target mutation. After any compile failure, audit the complete generated source for the same defect class rather than patching only the first reported line. Failure class: `HARNESS-DOTNET-COMPILE-01`.

Downstream missing tokens/files/results caused solely by an earlier harness abort MUST NOT be reported as independent product failures. A collector crash or uncontrolled host-process termination makes that run **invalid harness evidence**; it cannot be promoted to PRODUCT failure or acceptance PASS.

Failure class for violating this construction rule: `HARNESS-UNPROVEN-PRIMITIVE-COMPOSITION-01`.

Required preflight:

- exact expected HEAD / origin / remote;
- exact dirty file set;
- exact source hashes;
- exact candidate path;
- exact evidence hashes where applicable;
- every console command name checked against current `src/kernel.c`;
- success/error tokens checked against current source;
- any ELF/vector assertion checked against current `nm` / vector image;
- any disassembly assertion tested against current `objdump`;
- any build-size assertion checked against current `size`/map;
- all future gates after the previously failing gate must be replayed against existing evidence when possible.

A fix is not considered ready if only the last failing line was corrected.

---

## 6. PowerShell harness rules

### Empty collections and strings

Functions that may receive empty values must use appropriate attributes:

- `[AllowNull()]`
- `[AllowEmptyString()]`
- `[AllowEmptyCollection()]`

Always materialize pipeline results explicitly with `@(...)` when an empty result is valid.

Never assume an empty pipeline becomes an empty array.

A helper that returns a raw collection does **not** establish a stable collection-valued API contract in PowerShell: success-pipeline enumeration makes zero returned elements caller-visible as `$null`, one element as a scalar, and multiple elements as an array. When downstream code needs stable cardinality semantics, return exactly one non-enumerable record that owns the collection explicitly, for example a `PSCustomObject` with scalar `Count` and `List[string] Items`, and consume `Count`/`Items` from that record. Do not repair this defect by adding `@(...)` independently around each downstream `.Count`; prove the collection primitive once on the actual PowerShell execution domain and then recompose its consumers. This exact 0/1/N behavior and the record-based correction were executed successfully on PowerShell 7.6.6 during the pre-Bootloader resource-recovery Gate-2 harness recovery on 2026-09-26. Failure class: `PWR-FUNCTION-OUTPUT-CARDINALITY-01`.

External Windows tools routinely emit legitimate blank stdout/stderr lines. Any generic logging/capture helper that accepts external command output must permit empty strings (`[AllowEmptyString()]`) and preferably serialize them explicitly as `<blank>` in the chronological log. A blank line from STM32CubeProgrammer, Git, a compiler, or a host bridge must never become a parameter-binding failure. Capture the complete native-command output to its own evidence file before interpreting exit status or replaying lines to the human-readable log. This failure class is `PWR-EMPTY-OUTPUT-LOGGING-01`.

Do not recursively `ConvertTo-Json` arbitrary `Get-PnpDeviceProperty`, registry, WMI/CIM or provider objects in an acceptance harness. Their `Data`/provider metadata can contain deeply nested objects or cycles, causing truncation warnings, excessive CPU/memory use, or apparent hangs. Flatten evidence first to primitive strings/arrays (CSV/TSV or explicit ordered scalar objects), bound any SetupAPI log scan to a recent tail/window, and keep host-diagnostic collection time-bounded. This failure class is `PWR-RECURSIVE-PROVIDER-SERIALIZATION-01`.

USB bulk transfer boundaries are transport boundaries, never application-frame boundaries. A WinUSB `ReadPipe` may return a partial protocol frame, exactly one frame, or bytes spanning the end of one frame and the beginning of the next. Hardware acceptance harnesses for framed protocols must use a persistent stream decoder that accumulates arbitrary read chunks, extracts complete frames by protocol length/CRC, and preserves surplus bytes for the next frame. Never pass one raw USB read directly to a whole-frame parser or claim that every 64-byte transfer must equal one frame. This failure class is `PWR-USB-BULK-TRANSFER-FRAME-ASSUMPTION-01`.

Do not declare PowerShell parameters or working variables using names that collide case-insensitively with automatic variables. The hard blacklist for acceptance harness declarations/assignments includes at least `$args`, `$input`, `$error`, `$matches`, `$pwd`, `$home`, `$host`, `$pid`, `$profile`, `$pshome`, `$psscriptroot`, `$pscommandpath`, `$myinvocation`, `$lastexitcode`, `$executioncontext`, `$foreach`, `$switch`, `$this`, `$true`, `$false`, and `$null`. In particular, protocol helpers must not use a formal `$Args` parameter, and ordinary state variables must not use names such as `$home`: PowerShell automatic-variable names are case-insensitive and some are read-only. Use explicit names such as `$RpcArgs` and `$homeState`, invoke helpers with named parameters for nontrivial protocol requests, statically scan every formal parameter and assignment target against the blacklist, and run exact host-side self-tests before any target I/O. This failure class is `PWR-POWERSHELL-AUTOMATIC-VARIABLE-COLLISION-01`.

PowerShell evaluates ordinary function-call argument expressions before entering the callee. Therefore an assertion helper such as `Assert-Gate $condition ("...{0}" -f $possiblyNull.Type)` can itself fail while constructing the failure message even when the condition is true. Under StrictMode, any diagnostic message that dereferences an object must be guarded before property access or constructed lazily. Never dereference the object that is expected to be `$null` in a success-path no-response assertion. This failure class is `PWR-EAGER-ASSERT-MESSAGE-DEREFERENCE-01`.

Acceptance harnesses must pin `Set-StrictMode` to a deterministic version (currently `3.0`) rather than `Latest`; Microsoft documents `Latest` as intentionally non-deterministic across future PowerShell releases. Unexpected uncategorized PowerShell exceptions are harness failures until proven otherwise, not product failures.

Before a long hardware-runtime gate, preflight every host resource required later in the same run. If UART acceptance is mandatory, prove that a non-target CDC serial adapter is enumerated before SWD/programming/reconnect/IWDG work begins; otherwise stop early with an environment classification. Keep adapter VCC disconnected when the board has its own normal supply.

A retryable probe must not poison the final failure classification. If a temporary `Open-DeusWinUsb`, serial-port, SWD or enumeration attempt is expected to fail and be retried, either use a non-asserting probe helper or save/restore the global classification inside the retry boundary. Only the terminal failed condition may set the final class.

Do not collapse layered boot readiness into one signal. Transport recovery (`USB enumerated`, `WinUSB opens`, `ping works`) does not prove higher-level runtime readiness. If product code has an explicit boot/splash/state-machine delay, a post-reset acceptance test must poll the actual higher-level state with a bounded deadline derived from that contract. For the current STM32 application runtime, reset sets `active_id=0`, the boot UI holds SPLASH for at least 1000 ms, and `system.home` is started only when the UI reaches HOME; therefore an immediate `APP_ACTIVE_ID=1` assertion after the first recovered ping is a harness race. Capture every polled application state and fail only after the bounded home deadline.

When a hardware gate has already completed an irreversible or operator-heavy acceptance step (for example a required physical USB disconnect/reconnect) on the exact same source/Flash candidate and later fails solely because of a proven harness defect, a continuation run may consume that earlier step instead of repeating it. The continuation must embed or otherwise cryptographically bind the prior evidence/log, verify the exact candidate/Flash again, explicitly list which earlier assertions are inherited, execute all remaining assertions, and produce a composite final record. Do not use continuation to bypass an unresolved product/environment failure.

Do not then re-wrap an already materialized array as a single pipeline object when returning from a helper. In particular, `return ,$output` turns a command result array into one nested `Object[]`; downstream regex/line parsers then see the array object instead of individual output lines. Command-capture helpers must return `$output`, while callers that require array semantics use `@(...)` at the call site. This failure class is `PWR-ARRAY-MATERIALIZATION-01`.

Do not construct dynamic assertion-token lists with unparenthesized string concatenation inside an array literal, for example `@('A='+$a, 'B='+$b)`. PowerShell expression binding can make the comma-separated values become the right-hand operand of `+`, yielding one nested/array-valued expression; converting that value to text then produces a single space-joined string instead of independent tokens. For acceptance assertions, prefer separate scalar assertion calls. If a dynamic array is genuinely required, parenthesize each complete expression explicitly, for example `[string[]]@(('A={0}' -f $a),('B={0}' -f $b))`, and execute-prove the construction on the target PowerShell host before composition. Failure class: `PWR-DYNAMIC-ARRAY-CONCAT-BINDING-01`.

### Gate outcome vs process failure

An expected acceptance result such as `PRODUCT_RESOURCE_BUDGET`, protocol rejection, hardware mismatch, or other successfully classified gate FAIL is not a harness crash. Once `run.log`, `outcome.txt` and the evidence archive have been written successfully, the operator-facing script must terminate normally and print the gate outcome/classification. Do not `throw` merely to convert an accepted `OUTCOME=FAIL` into process exit code 1; that can cause terminal wrappers to close/restart and obscures the distinction between product failure and harness execution failure. Reserve process-level failure for cases where trustworthy evidence could not be produced or package/AST/prestate integrity itself failed before normal evidence finalization. This failure class is `HARNESS-EXIT-SEMANTICS-01`.

Failure finalization must be phase-aware. `run.log`, `outcome.txt` and the evidence hash index are unconditional. Phase-owned artifacts such as a Flash summary, MAP, transport transcript, recovery binding or hardware matrix report are required only if that phase completed or if the overall run is PASS. On an earlier FAIL, absence of a later phase artifact records the execution boundary reached and must not trigger a second exception. If evidence finalization encounters a defect after a primary classified failure, preserve the primary classification/error and record the finalization defect as secondary evidence instead of masking it. Failure class: `HARNESS-PHASE-ARTIFACT-FINALIZATION-01`.

### Operator-facing terminal structure

The chronological `run.log` remains authoritative, but the terminal must not present a wall of undifferentiated command lines. Every acceptance harness must emit visually separated major sections for at least `PRESTATE`, `BUILD`, `STATIC`, `RESOURCES`, `ARTIFACTS`, and `RESULT`; hardware gates add `TARGET`, `ENUMERATION`, `RUNTIME`, and `READBACK` as applicable. Each major section MUST use the established three-line project banner form, with the same emission timestamp on all three lines:

```text
[yyyy-MM-dd HH:mm:ss.fff] ========================================================================
[yyyy-MM-dd HH:mm:ss.fff] SECTION=<NAME>
[yyyy-MM-dd HH:mm:ss.fff] ========================================================================
```

The separator/header is operator presentation and must also be persisted in `run.log`. A single inline token such as `[SECTION] SECTION=BUILD` is not sufficient section separation for an operator-facing acceptance harness. Intermediate section content uses concise `[OK]`, `[WARN]`, `[ERROR]` and, when needed, `[INFO]` records while detailed native stdout/stderr remains in dedicated evidence files; the words `PASS` and `FAIL` are reserved for the final RESULT block.

The final terminal result must be visually dominant and color-coded with `Write-Host`: green foreground for PASS, red foreground for FAIL, yellow for classified warnings/skips. The final block must include gate name, `RESULT=PASS|FAIL`, classification, candidate identity and the most important resource/runtime figures, followed by the result log/evidence paths. On FAIL it must also print the exact `FAILURE_PHASE` and concise `ERROR` in red in that same final block. Do not rely on an uncolored prose line buried after command output. If `$Host.UI` coloring is unavailable, print an ASCII banner such as `========== GATE 2 PASS ==========` as a non-color fallback. **The final RESULT block is the last operator-visible block. Nothing after it may be printed green/red or contain the reserved words `PASS`/`FAIL`; evidence/archive self-checks that occur before RESULT render as timestamped `[OK]`/`[ERROR]` lines. Package-generation lint must reject any `ForegroundColor Green|Red`, `RESULT=`, `FINAL_OUTCOME=`, or bare `PASS|FAIL` emission after the final RESULT block, and must reject any green emission reachable when the overall result is FAIL.**

Every operator-visible section header, categorized summary line (`[OK]`, `[WARN]`, `[ERROR]`) and final RESULT line MUST carry an emission-time local timestamp prefix in the exact form `[yyyy-MM-dd HH:mm:ss.fff]`. `run.log` uses the same timestamp format for chronological harness events. Raw native stdout/stderr files remain byte-faithful and need no injected timestamp; when native output is summarized into `run.log`, the harness timestamps the summary rather than rewriting the raw capture. `outcome.txt` MUST record `RUN_STARTED_AT` and `RUN_FINISHED_AT` as ISO-8601 timestamps with offset. Missing timestamp/category/final-color structure is a harness presentation defect even when the underlying product result is otherwise classifiable. Failure class: `HARNESS-TERMINAL-STRUCTURE-01`.

A categorized summary is presentation only; evidence ownership remains unchanged: raw chronological commands stay in `run.log`, structured final state stays in `outcome.txt`, artifact identity stays in hash/candidate files.

### .NET 10 / Microsoft.Testing.Platform working-directory rule

Host-test harnesses using .NET 10 + Microsoft.Testing.Platform v2 MUST prove that the intended `global.json` is in scope before running tests. The .NET CLI resolves `global.json` from the current working directory and its ancestors; passing a `.csproj` path below `host/` does not make `host/global.json` active when the process CWD is the repository root.

For this repository:

1. run `dotnet --info`, locked restore, build and `dotnet test` with CWD set to the materialized candidate `host/` directory; or execute the already-built MTP test application directly when that is the accepted primitive;
2. capture `dotnet --info` and fail HARNESS preflight if it reports `global.json file: Not found` or otherwise proves that the expected host policy is not active;
3. verify `host/global.json` semantically selects `"test": { "runner": "Microsoft.Testing.Platform" }` before test execution;
4. never classify the MTP-v2/.NET-10 `VSTest target is no longer supported` diagnostic as a unit-test assertion failure;
5. test-count discovery is not test execution: a matching `[Fact]` count cannot substitute for an executed passing test summary and process exit code.

Failure class: `HARNESS-DOTNET-MTP-RUNNER-01`.

Host build restore state MUST be established explicitly in the same bounded harness run. A `dotnet build ... --no-restore` invocation is permitted only after a successful bounded `dotnet restore --locked-mode` for the exact host project/solution under test, executed with CWD inside the candidate `host/` tree so the intended `host/global.json` is in scope. Merely assuming that a previous run left `obj/project.assets.json` behind is forbidden; a clean checkout or cleaned `obj/` tree is a valid execution state. If the harness skips restore and then fails with `NETSDK1004` / missing `project.assets.json`, classify it as `HARNESS-DOTNET-RESTORE-STATE-01`. The host-acceptance harness must own the NuGet restore domain: do not inherit an operator shell's `DOTNET_CLI_HOME` or `NUGET_PACKAGES` as package-cache authority. Launch each `dotnet` process with per-run `DOTNET_CLI_HOME` and `NUGET_PACKAGES` directories under WorkRoot and pass the owned package directory explicitly to restore. This per-process environment must not mutate the parent shell. After restore, prove the locked direct test-runner package has a real `.nuspec` in the owned cache before `--no-restore` builds/tests. An `NU5037` missing-nuspec failure under an unrelated prior-harness temp cache is `HARNESS-DOTNET-CACHE-DOMAIN-01`, not a product failure. If an explicitly isolated locked restore itself fails because a required SDK/feed/network is unavailable, preserve its stdout/stderr and classify that environment/dependency failure.

Reserve the words `PASS` and `FAIL` for the final gate result. Intermediate successful checks print `[OK]`; recoverable/nonfatal differences print `[WARN]`; an intermediate blocking check prints `[ERROR]`. This prevents a log from appearing to contain both a gate PASS and a gate FAIL when an early subcheck succeeds but the gate later stops.

For a hardware gate that consumes an already accepted build candidate, source identity is owned by the accepted Git candidate tree, not by raw filesystem SHA-256 of individual working-tree files. Recompute the candidate with a temporary Git index over the authorized source paths and require the exact accepted tree hash. Raw filesystem hashes may be captured diagnostically, but must not independently fail the hardware gate when the normalized candidate tree, dirty set and real index are exact; line-ending/encoding filters can change raw bytes without changing Git content identity. This failure class is `PRESTATE-RAW-HASH-OWNER-01`.

### Line endings and structured native-output records

For line-oriented native output, especially `KEY=value` build/status records, do not use multiline-regex end anchors directly on raw process text. Windows `CRLF` and Unix `LF` must be treated equivalently. Normalize `\r\n` and lone `\r` to `\n`, split into records, select the exact `KEY=` prefix, and require the contract-defined cardinality (normally exactly one record) before parsing the value. A parser correction after a false failure must be replayed against the exact raw stdout captured by the failed evidence before a recomposed package is released. Failure class: `HARNESS-LINE-ENDINGS-PARSER-01`.

For parsed hardware registers and other fixed-width unsigned fields, comparison must stay in the declared numeric domain. A `[uint32]` register containing `FFFFFFFF` is `4294967295`; do not compare it to an untyped/bare `0xFFFFFFFF` literal whose PowerShell interpretation can be signed `-1`. Use `[uint32]::MaxValue`, an explicit validated `[uint32]` expected value, or compare canonical fixed-width hex strings. The harness must include at least one high-bit-set sentinel in parser/comparison replay when the accepted hardware state can contain such a value. Failure class: `HARNESS-UINT32-LITERAL-DOMAIN-01`.

PowerShell helper functions that accept an argument vector must not name the formal parameter `$Args` and must not be called with a bare array expression in command position. `$args` is an automatic variable and a call such as `Invoke-Git @('rev-parse','HEAD')` can bind/unroll positionally instead of passing the intended argument vector, silently changing the native command. Use an explicit non-reserved parameter name such as `-Arguments` and call it as `Invoke-Git -Arguments @('rev-parse','HEAD')`; generated harness audits must verify every wrapper call uses the named vector parameter. Failure class: `HARNESS-POWERSHELL-ARGUMENT-VECTOR-BINDING-01`.

Read-only ST-LINK/CubeProgrammer probes that claim to preserve live runtime execution must explicitly request `mode=HOTPLUG` and verify the tool reports `Connect mode: Hot Plug`. CubeProgrammer `NORMAL` is not read-only with respect to CPU execution state: it resets the target and halts the core before access. Therefore a harness must not infer a device/runtime failure from USB disappearance after `NORMAL` SWD attachment. For a nonvolatile read-only preflight, use `HOTPLUG`, forbid erase/program/explicit-reset/start/RDU/option-byte-write commands, and re-prove the runtime interface after SWD reads. Failure class: `HARNESS-STLINK-CONNECT-MODE-SCOPE-01`.

PowerShell variables are case-insensitive. A harness must not use a local name that case-collides with authoritative script state (for example `$outcome` beside `$script:Outcome`): assigning or reading the local can overwrite/read the script-level value and splice arbitrary multi-line evidence text into the terminal RESULT. Authoritative state variables must use collision-resistant names, and generator lint must reject case-insensitive duplicates across local/script scopes for `Outcome`, `Classification`, `FailurePhase`, and `FailureError`. Failure class: `HARNESS-POWERSHELL-STATE-VARIABLE-COLLISION-01`.

The same rule applies to the **final reset/start boundary of a destructive recovery tool**. A recovery utility may use `NORMAL` while it intentionally owns/halt-controls the target for erase/program/verify, but it must not claim successful runtime return after a final `-rst` that first reconnects in default `NORMAL` mode. The final execution-release primitive must use an explicit non-halting connection mode (`mode=HOTPLUG`) or another separately proven start/reset sequence, and the enclosing harness must verify runtime return afterward. If Flash is byte-exact but USB/runtime is absent after a `NORMAL`-attached final reset, classification is an attribution/tooling gap (`HARNESS-STLINK-CONNECT-MODE-SCOPE-01`), not a product boot failure, until core state is independently observed.

### String matching

Do not use broad substring checks for protocol errors.

Bad:

```powershell
if ($text.Contains('_ERR')) { ... }
```

because valid fields such as `RX_ERROR_COUNT` contain `_ERR`.

Use exact command-specific tokens:

```text
RX_IRQ_RING_ERR
MSP_STACK_ERR
SCHED_COOP_ERR
```

and parse numeric fields independently.

### Source semantic tokens vs transport tokens

Keep separate structures:

- source semantic contract:
  command name, success token, semantic error tokens;
- transport/runtime contract:
  timeout, CR/LF framing, generic unknown-command `ERR\r\n`.

Do not require transport tokens to exist inside each command's source implementation.

### Regex / escaping

Avoid multilayer regex when direct source parsing is sufficient.

PowerShell → Git → POSIX ERE escaping has already caused false failures.

Prefer:

- direct file reads;
- exact tokens;
- simple case-normalized line scanning.

### Case sensitivity

GNU disassembly may print special registers differently (`PRIMASK`).

Normalize case for mnemonic/register presence checks.

---

## 7. ELF/vector rules

Never hard-code handler addresses from an old milestone.

Bad:

```text
Reset vector must equal 0x08000041
```

Correct rule:

1. read current linked handler symbol address from `nm`;
2. set Thumb bit;
3. compare vector entry to that current symbol.

Fixed architectural addresses are allowed only for true linker/MCU invariants such as reserved MSP top/bottom when those are explicitly frozen.

---

## 8. Candidate identity rules

Milestone candidates live in milestone-specific build directories.

Never substitute another `build\os.bin`.

Always bind:

- path;
- byte count;
- SHA-256;
- ELF/map when relevant.

After an accepted build, hardware scripts should:

1. read back target Flash first;
2. compare exact candidate identity;
3. reuse the already-correct target;
4. reflash only on mismatch.

This avoids unnecessary erase/program cycles.

---

## 9. UART / console / event rules

### CRLF is two bytes

The console executes a command when it consumes CR **or** LF.

For a normal `command\r\n` line, execution can start on CR while LF is still queued.

Any diagnostic that reuses the same RX FIFO while executing synchronously must account for this.

### Event is notification, not payload

Scheduler event semantics must be treated like a condition-variable notification:

> event means “re-check the condition/data source”.

It does **not** mean:

> the next queued byte is necessarily the byte that caused the event.

A correct event consumer must:

1. inspect/drain the relevant FIFO condition;
2. ignore protocol framing where appropriate;
3. block only when the condition is not satisfied;
4. after wake, re-check the FIFO/condition;
5. tolerate event/data races using the scheduler's pending-event mechanism.

### Wait/wake diagnostic handshake

For the UART IRQ wait/wake proof:

- `SCHED_WAIT_WAKE_ARMED` must be emitted from the scheduler-side task after scheduler activation, not from MSP before `scheduler_start_preemptive()`;
- CR/LF framing must not be treated as diagnostic payload;
- host waits briefly after `ARMED` so all runnable tasks can disappear and MSP can enter WFE;
- host then sends raw sentinel `0x57`;
- task must wake via USART1 IRQ event and consume sentinel from RX FIFO;
- evidence must show `idle_wait_count > 0`, intact PSP canaries and expected byte/event mask.

---

## 10. Git / rollback rules

### Before Flash

For source/build gates:

- back up exact files before mutation;
- on any failure before hardware flash, restore exact previous bytes;
- verify hashes;
- verify expected Git dirty set.

### After Flash begins

Do not automatically rewrite source to an older state after the MCU has been programmed with the new candidate.

Keep source and target aligned for diagnosis.

Do not commit or push until failure is classified and accepted.

---

## 11. Evidence bundle minimum

Important gates produce ZIP on PASS **and** FAIL.

The default operator-return surface is **one self-contained evidence ZIP**. `run.log` MUST be inside that ZIP together with `outcome.txt`, hashes and the raw command evidence. Do not require the operator to return a loose family of duplicate evidence files.

A loose `<gate>.log` beside the ZIP may be emitted as a convenience for immediate reading, but it is optional compatibility output only. If emitted, it MUST be a byte-identical copy of the archived `run.log` and its SHA-256 MUST be recorded in the evidence manifest/outcome. The ZIP remains the authoritative archival artifact. Other detailed stdout/stderr/artifact files belong inside the ZIP unless a later workflow has a specific external-consumer requirement.

Bundle should contain, as applicable:

- `run.log` — chronological log whose harness-emitted lines carry `[yyyy-MM-dd HH:mm:ss.fff]`, with explicit `SECTION=<name>` markers and textual categories `[OK]`, `[WARN]`, `[ERROR]` plus the final `RESULT` block;
- `outcome.txt` — structured final classification/result;
- hash manifest covering the evidence payload and any optional loose convenience log;
- Git status/diff;
- exact source files or source diff;
- candidate `.bin`;
- `.elf`;
- `.map`;
- `nm`;
- disassembly;
- vector image;
- stack-usage files;
- tool stdout/stderr;
- target readback;
- prior accepted/failed evidence chained by hash.

Terminal coloring is presentation and is not serialized into the plain-text log. The persistent log equivalent is the explicit section/category token structure above; the terminal still follows the color rules in Section 6.

If a future failure cannot be explained from the bundle, the bundle is incomplete.

---

## 12. Stop-the-line rule for repeated harness failures

After two harness failures in the same workflow family:

- do not issue another full hardware/build acceptance script immediately;
- inspect all previous failed ZIPs;
- replay all remaining assertions against available artifacts;
- compare with at least the previous two accepted workflows;
- add self-tests for the failure class;
- only then produce the next script.

A repeated class of harness failure must be added to this playbook before continuing.

Linked stack oracles must treat compiler-generated IPA clone symbols as a distinct identity domain from source-level `-fstack-usage` names. A reachable linked symbol such as `function.constprop.N`, `function.isra.N` or `function.part.N` must not be assigned the base function's `.su` frame by name alias alone. A fallback is allowed only when: (1) the suffix is a recognized compiler-clone form; (2) the compiler's canonical stack-usage stem for that clone has a parsed `.su` record (for GCC 15.3, a linked `foo.constprop.0` is expected to resolve to a stack-usage name such as `foo.constprop`); (3) the clone's own final linked disassembly has an unambiguous static stack prologue; (4) a matching epilogue restores the same stack bytes; and (5) the disassembly-derived frame equals the canonical `.su` frame. The linked clone frame comes from the final disassembly, the `.su` row is an independent consistency check, the resolution is reported explicitly in evidence, and all other missing reachable frames remain `HARNESS_STACK_ORACLE_UNSUPPORTED`. Bootloader `.su` files used by the oracle should be retained in evidence when they contain no secret material. Failure class: `HARNESS-STACK-ORACLE-COMPILER-CLONE-01`.

---

## 13. Current known-gotcha registry

| ID | Symptom | Root cause | Required prevention |
|---|---|---|---|
| PWR-EMPTY-01 | null/empty array binding error | empty pipeline materialization | `@(...)`, AllowNull/AllowEmptyCollection |
| PWR-EMPTY-02 | empty string rejected in string array | mandatory binding rejects empty string | AllowEmptyString or skip empties |
| PWR-EMPTY-03 | disassembly/line parser helper throws before parsing a blank line | normalized/raw disassembly legitimately contains empty physical lines, but a mandatory scalar string parameter rejects `""` during PowerShell binding | every line-consumer helper that may receive raw/normalized physical lines must either declare `[AllowEmptyString()]` or filter empties before invocation; replay the parser against the exact captured artifact before delivery |
| PWR-SCOPE-01 | nested helper self-test/call reports command not recognized | helper function is defined inside an owning function, but a call/self-test was emitted at script scope where the nested helper does not exist | generated harness audit must identify nested function definitions and reject any call site outside the owning lexical function; helper self-tests must execute inside that owner or the helper must be intentionally promoted to script scope |
| PWR-SPAN-01 | PowerShell package/tool script fails on `array.AsSpan(...)` / span-based .NET API | C# extension-method/span invocation semantics were assumed to work as instance methods through PowerShell binding | do not use C# extension-method syntax such as `AsSpan` in operator-facing PowerShell; prefer explicit byte-array indexing/copying or a PowerShell-proven static API, and run a deterministic standalone self-test of the script before embedding it in a full acceptance harness |
| ARTIFACT-DEBUG-PATH-01 | fresh build has exact BIN/loadable semantics but ELF/MAP raw SHA differs | debug-enabled ELF and linker MAP embed absolute temporary source/object paths; a new run root/GUID or output-directory name legitimately changes DWARF path strings, debug-section sizes/offsets and raw ELF/MAP hashes without changing product bytes | never use raw debug-enabled ELF or MAP SHA as cross-run product identity unless the build path is itself canonicalized/frozen. For fresh revalidation, require exact source tree + exact loadable BIN, compare a debug-stripped ELF from both runs byte-for-byte (using the same ARM toolchain), and separately re-prove symbol table, alloc-section geometry, vectors, resources and linker invariants. Retain raw ELF/MAP hashes only as run-local evidence identities. |
| REGEX-01 | git grep exit 128 | escaping across PowerShell/Git/ERE | direct source/token inspection |
| PATH-01 | candidate byte mismatch | wrong milestone candidate path | bind exact path + SHA-256 |
| DISASM-01 | `primask` “missing” | case-sensitive search vs `PRIMASK` | case-normalized disassembly checks |
| VECTOR-01 | Reset vector mismatch | stale historical fixed address | bind vector to current linked symbol |
| UART-TOKEN-01 | valid RX stats treated as error | `_ERR` substring matched `RX_ERROR_COUNT` | exact failure tokens + numeric field parsing |
| UART-CMD-01 | firmware returns `ERR` | stale alias `schedrun` vs `schedcoop` | source-bound command preflight |
| CONTRACT-01 | source preflight wants `ERR` per command | semantic and transport contracts mixed | separate contract layers |
| UART-FRAME-01 | wait/wake reads `0x0A` not sentinel | trailing CRLF delimiter remains in RX FIFO | framing-aware consumer + scheduler-side ARM handshake |
| EVENT-01 | event assumed to identify payload | notification confused with data | re-check FIFO/condition after wake |
| EVIDENCE-LOCATION-01 | accepted token reported missing although acceptance passed | consumer searched the wrong evidence surface (`run.log` vs `outcome.txt`) | define evidence-surface ownership and assert each fact only where the producer guarantees it |
| EVIDENCE-SCHEMA-SURFACE-MISMATCH-01 | prior structured proof is present and valid but recovery harness reports a missing token such as `static_sram=9856` | recovery harness copied a human-readable `run.log` field name into validation of a different structured evidence file whose real key is different (for example `STATIC_SRAM_DATA_BSS=9856`) | parse the exact schema of each owning evidence file and replay every prior-evidence assertion against the exact embedded ZIP before delivery |
| HARNESS-STALE-SOURCE-ASSERTION-01 | candidate source is hash-correct but a pre-build static guard reports a deleted private implementation token such as `mutation_start_cycles` | harness assertions were not regenerated after a contract-preserving footprint/refactor optimization | assert stable contract/public diagnostic surfaces, not private identifiers; replay every static source assertion against the exact candidate snapshot before delivery |
| HARNESS-CLEAN-CANDIDATE-HIDDEN-VCS-DEPENDENCY-01 | a recovery/artifact generator succeeds in the live repo but fails in a clean materialized candidate with `not a git repository` | the primitive secretly rediscovered provenance through `.git` instead of consuming explicit candidate identity | make provenance (`base_head`, `candidate_tree`, firmware tree, hashes) required validated inputs; execute-prove the generator in a gitless representative root before composing it into a gate |
| PWR-DYNAMIC-ARRAY-CONCAT-BINDING-01 | a proof reports one impossible long “missing token” containing several intended tokens separated by spaces | unparenthesized `+` expressions inside `@(...)` bound to a comma-created array, so the loop received one array-valued token | use separate scalar assertions, or explicitly parenthesize every dynamic element and host-execute-prove the resulting array before composition |
| HARNESS-CANDIDATE-BYTE-DOMAIN-MISMATCH-01 | recovery manifest contains a different script SHA than the live WIP hash even though the candidate tree and generator are correct | validator compared Windows working-tree bytes with Git-normalized candidate-tree bytes (`* text=auto`, CRLF vs LF) | compare candidate-owned hashes only against files materialized from the exact candidate tree; keep live WIP hashes in the separate working-tree evidence domain; prove clean-candidate primitives through temp-index + tree + archive materialization rather than direct copies |
| CUBEPROG-WRITE-VERIFY-ORDER-01 | CubeProgrammer reports `Wrong verify command use` after `File download complete` and exits 1 | `--skiperase` was placed between `-w <file> <address>` and `-v`; CubeProgrammer requires verify immediately after the write command | place `--skiperase` before `-w`, keep `-v` immediately after the write address, and primitive-prove this exact token order before composing recovery/runtime acceptance |
| PWR-PARSER-01 | package runner fails before line 1 execution | interpolated `$variable:` parsed as scoped-variable syntax | AST-parse `apply.ps1` before execution; use `${variable}:` or format operator |
| DOC-CHECK-01 | correct finalized document rejected by package validator | validator searched case-sensitive prose (`normal-boot` vs `Normal-boot`) even though payload SHA was exact | validate exact post-SHA plus unique structural milestone markers; do not re-parse prose semantics with brittle substrings |
| UNTRACKED-CHECK-01 | finalization passes `git diff --check`, commit gate later finds whitespace in a new file | ordinary `git diff --check` does not include untracked files | validate the full would-be commit through a temporary Git index and run `git diff --cached --check` there before accepting finalization |
| DOC-CURRENT-STATE-01 | new chat reads contradictory current state | several documents independently claimed global current/next authority | `docs/CURRENT_STATE.md` is the sole global project-state source; other docs keep scoped roles and link to it rather than duplicating current-state blocks |
| PWR-CMDTOKEN-01 | AST parse passes but runner fails with “term is not recognized” for a helper/built-in command | PowerShell command name was immediately adjacent to its first argument (`Write-Host"..."`, `L'...'`, `Sha$p`, `SaveEvidence'PASS'`), so tokenization produced a different command name | require whitespace between a command name and every argument; generator statically rejects adjacent-argument calls for all package helper commands and critical built-ins |
| HARNESS-DOTNET-MTP-RUNNER-01 | Core/Transport test process exits before executing tests with `VSTest target is no longer supported` while `[Fact]` discovery counts still match | .NET 10 was invoked outside the `host/` policy scope, so `host/global.json` did not select Microsoft.Testing.Platform v2 | set CWD to candidate `host/` (or run the accepted direct MTP test app), capture/verify `dotnet --info` sees `global.json`, and require an executed passing test summary rather than source test-count discovery |
| HARNESS-DOTNET-RESTORE-STATE-01 | host build stops with `NETSDK1004` because `obj/project.assets.json` is absent | harness invoked `dotnet build --no-restore` without establishing restore state in the same bounded run | run bounded `dotnet restore --locked-mode` first with candidate `host/` CWD/global.json in scope, capture stdout/stderr, then build with `--no-restore`; classify an actual restore dependency/feed failure separately |
| HARNESS-TERMINAL-STRUCTURE-01 | operator sees an uncategorized wall of output or a raw exception instead of a timestamped colored result | harness omitted required section/category/timestamp/final-result presentation or used `throw` after evidence finalization | timestamp all harness summary lines, use `[OK]/[WARN]/[ERROR]`, print final PASS green or FAIL red with classification/phase/error/evidence path, and terminate normally once trustworthy evidence is finalized |
| HARNESS-EXECUTION-DOMAIN-OWNERSHIP-01 | target USB appears "missing" on Windows, or a harness tries CubeProgrammer/UART on the Mac mini | composed gate ignored the physical split-host topology and executed a hardware primitive on a host that does not own that interface | bind every hardware step to `DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`; current USB/libusb/CDC is Mac-mini Ubuntu, ST-LINK/UART is Windows; coordinate by bounded SSH/SCP and retain host labels in evidence |
| HARNESS-ASSET-ACCESS-POLICY-01 | pre-publication Asset runtime proof is rejected even though management HELLO advertises the carrier | harness used production `PublishedOnly` policy while system capability bit 5 is intentionally OFF | keep production CLI fail-closed; use Core `PrepublicationAcceptance` only inside the bounded acceptance helper/runner through Gate 5 |
| HARNESS-POWERSHELL-ARGUMENT-CARDINALITY-01 | a native command receives a split/duplicated computed argument; e.g. `git hash-object` emits the same blob twice | unparenthesized string concatenation was embedded in a multi-item PowerShell array literal | parenthesize/precompute each computed argument, persist the exact argument vector, and validate expected output cardinality before semantic comparison |
| HARNESS-EVIDENCE-RECORD-CARDINALITY-01 | `outcome.txt` or another structured evidence file collapses multiple key/value records onto one physical line | computed expressions inside a PowerShell array literal were not individually grouped | parenthesize each computed record and validate one key/value per physical line before packaging |
| HARNESS-SEMANTIC-ORACLE-01 | a healthy target is classified PRODUCT because the harness hard-coded an implementation diagnostic or otherwise modeled the accepted state machine incorrectly | product assertion was not tied to a frozen contract or the expected state was guessed instead of derived from candidate semantics | keep an explicit assertion-authority map; derive persistent-selection/runtime state from frozen contracts/source-bound semantics; keep non-normative diagnostics observational or classify their oracle mismatch as HARNESS |
| HARNESS-AMSI-DELIVERY-01 | PowerShell/AMSI or endpoint protection blocks the acceptance script during parse/load before the harness starts | the delivery artifact presents a monolithic or otherwise security-sensitive scripting surface that the endpoint protection policy rejects | perform zero target mutation; preserve the blocked artifact; inspect the endpoint-protection detection/event to identify the rule; do not disable, bypass, obfuscate or weaken security controls; recompose the acceptance engine into a conventional signed/hash-bound executable or otherwise security-compatible architecture before rerun |
| HARNESS-DOTNET-COMPILE-01 | generated acceptance harness reaches `dotnet build` but does not compile | delivery was treated as ready after lexical/static inspection without a compiler proof, or generated C# contains a compile-time defect such as invalid string escapes | no target I/O is allowed before successful locked restore/build; inspect exact compiler diagnostics; fix the source-level cause; scan the whole generated source for the same defect class; persist restore/build stdout+stderr on failure; require compiler success before any hardware stage |
| HARNESS-LINE-ENDINGS-PARSER-01 | a successful native command is misclassified because a harness token parser depends on host line endings | line-oriented records such as `KEY=value` are parsed directly from raw CRLF/LF text with regex anchors or trimming assumptions | normalize `CRLF` and lone `CR` to `LF` first, split into records, require exact prefix and exact cardinality, then parse the value; replay the corrected parser against the raw failed-run stdout before recomposition |
| HARNESS_STLINK_CONNECT_MODE_SCOPE | runtime USB disappears or a post-reset state is attributed to product code after a CubeProgrammer connection/reset sequence | the harness used default `NORMAL` connect semantics or failed to prove the actual connect mode; `NORMAL` may reset/halt the core, so absence of runtime after that sequence is not product evidence | every read-only acceptance SWD attachment and every acceptance final-reset command must explicitly request `mode=HOTPLUG`, persist the native CubeProgrammer stdout, require `Connect mode: Hot Plug`, and re-prove post-reset execution state before any product classification; a run that used default/`NORMAL` is frozen as harness attribution evidence only |
| HARNESS_POWERSHELL_STATE_VARIABLE_COLLISION_01 | structured `outcome.txt` or the final RESULT contains unrelated prior outcome text even though raw measurements are valid | PowerShell variable names are case-insensitive, so a local name such as `$outcome` aliases script-scoped `$script:Outcome` and corrupts the authoritative final-state accumulator | reserve distinct script-scoped names such as `$script:FinalOutcome`, `$script:FinalClassification`, `$script:FinalFailurePhase`, and `$script:FinalError`; do not reuse their case-insensitive basenames for locals; generator lint must reject such collisions, and offline replay must validate single-line key/value cardinality plus exact final RESULT fields before delivery |
| HARNESS_EXTERNAL_ACCEPTED_ARTIFACT_DEPENDENCY_01 | a fresh acceptance/recovery run fails before testing product behavior because a previously accepted ZIP is no longer present in the operator Downloads directory | the harness treated an old operational artifact as a required local prerequisite even though the accepted product identity is reproducible from frozen source/toolchain/tree inputs | prefer self-reproduction of the accepted identity: rebuild with the frozen accepted source-tree identity and require the published artifact SHA/resource tuple, then rebuild the current candidate and prove normalized equivalence; require an old ZIP only when it contains irreducible state that cannot be independently reconstructed from accepted inputs |
| HARNESS_CANDIDATE_TREE_SCOPE_RECONSTRUCTION_01 | an unchanged identity domain is falsely reported as tree drift during a narrow reopen | the harness confused an authorized change-path list with the candidate-tree inventory and synthesized a new Git tree from an empty index containing only changed/authorized paths; such a tree is not comparable to the accepted full/firmware/host identity | treat authorization sets and identity inventories as separate contracts; reuse the exact accepted candidate-tree derivation semantics from the owning accepted harness, carry forward identity domains whose bytes are not reopened, and never compare an empty-index subset tree to an accepted candidate tree |
| HARNESS_COMMENT_SENSITIVE_SOURCE_ORACLE_01 | a valid narrow source repair is rejected because a static regex matches an explanatory comment rather than executable code | the harness searches raw source text for semantic tokens such as `PRIMASK` after already proving an exact deterministic source delta; comments therefore become false product evidence | never classify executable behavior from comment-sensitive token scans; prefer exact old-to-new deterministic source transforms plus final linked disassembly, or strip comments with a parser-aware method before any source-token oracle; redundant raw-token checks must not override stronger byte/source-delta and machine-code evidence |
| HARNESS_RECOVERY_EVIDENCE_DIR_OWNERSHIP_COLLISION_01 | recovery exits before the first SWD command with `Recovery EvidenceDir must not already exist` | the outer harness pre-created the same evidence directory that the recovery primitive contractually owns and requires to be absent before it creates it | assign single-owner lifecycle to child evidence directories: the outer harness may create only the parent, must assert the child path is absent immediately before launch, and must never pre-create it; freeze a failed run with zero target I/O when child stdout/stderr shows failure before the first recovery `Invoke-Cube` section |
| HARNESS_TARGET_MUTATION_PRECLAIM_01 | final evidence reports `TARGET_MUTATION_STARTED=True` even though the recovery child exited before any destructive target command | the outer harness sets the mutation flag before launching the child rather than observing the real destructive boundary | mutation state must be evidence-derived: set STARTED only after child evidence records launch of the first destructive command (for this recovery, `SECTION=EXPLICIT_PAGE_ERASE`), and set COMPLETED only after byte-exact post-readback plus `RECOVERY_FLASH_PHASE=PASS`; process launch is not target mutation |
| HARNESS_SENSITIVE_RAW_FLASH_EVIDENCE_01 | a shareable evidence ZIP contains a full target Flash dump even though bootloader Flash embeds a firmware-update verification key | the harness writes a 64-KiB readback directly under the shareable evidence root and archive finalization sweeps it in | full Flash/RAM captures are private artifacts: store them only outside the shareable evidence root, record hashes/derived structural facts in shareable evidence, and delete or separately mark raw captures `DO NOT UPLOAD`; package-generation lint must reject `*.bin` full-Flash captures under the shareable evidence root |
| HARNESS_POWERSHELL_COMMAND_BOOLEAN_GROUPING_01 | launcher AST parse fails with `MissingEndParenthesisAfterStatement` / `UnexpectedToken` before execution | a command-form expression such as `Test-Path -LiteralPath ... -PathType Leaf` is combined directly with `-and`/`-or`, so PowerShell remains in command argument mode instead of parsing the boolean operator | every command invocation used inside a boolean expression must be parenthesized as its own expression, e.g. `if ((Test-Path ... -PathType Leaf) -and (-not (Test-Path ...)))`; generator lint must reject `Test-Path ... -and/-or` without a closing expression parenthesis, and the operator-facing launcher must always AST-parse before execution |
| HARNESS_SINGLE_SHOT_HOTPLUG_FLASH_READBACK_01 | a full-Flash prestate gate reports immutable drift from one 64-KiB HOTPLUG upload even though targeted rereads prove the allegedly changed page is byte-exact baseline | a single SWD/HOTPLUG bulk readback is treated as authoritative without retry/reproducibility proof; one transient 1-KiB read corruption therefore becomes a false target-drift classification | never classify Flash drift from one bulk readback alone; an exact expected SHA may pass immediately, but any mismatch must be reproduced by at least two further independent HOTPLUG readbacks before it is treated as target state; if both retries return the exact expected SHA, classify the first mismatch as `TRANSIENT_READBACK_ANOMALY` and continue; apply the same rule to post-program verification before declaring target corruption |
| HARNESS_POWERSHELL_INLINE_EXPRESSION_PARSE_01 | a read-only classifier is blocked by launcher AST parse with repeated `MissingEndParenthesisInExpression` | dense inline helper calls and bitwise/format expressions such as custom `U16/U32` functions inside parenthesized `-f` expressions were authored without an authoritative PowerShell parser available and escaped static delimiter checks | keep operator classifiers structurally simple: prefer .NET `BitConverter`/`Array.Copy` calls and precomputed variables over nested inline arithmetic/function calls; do not claim PowerShell AST PASS unless an actual PowerShell parser executed; launcher AST parse remains mandatory before execution |
| HARNESS_DOTNET_WARNINGS_AS_ERRORS_UNUSED_HELPER_01 | a temporary acceptance helper restores successfully but Release build fails with `CS8321` before any target action | generated helper source contains an unused local function while the project correctly enforces `TreatWarningsAsErrors=true`; the harness package was not compiled in an equivalent .NET environment before delivery | generated .NET helpers must be minimal and warning-clean by construction; remove unused scaffolding, bind helper source/template SHA-256 inside the package, and treat a build failure before target entry as harness-only with zero target mutation |
| HOST_LINUX_BOOTLOADER_LIBUSB_PERMISSION_01 | explicit runtime update entry returns `Ok/Resetting` and `1209:000D` enumerates, but the non-root Linux host then fails opening the bootloader with `LIBUSB_ERROR_ACCESS (-3)` | the bootloader uses a distinct USB PID from the already accepted runtime `1209:000C`; enumeration does not imply permission to open `/dev/bus/usb/...`, and the bench had no repository-owned or harness-owned permission rule for private-test `1209:000D` | distinguish enumeration from open/claim authority; before a Linux bootloader transaction, provision or verify an explicit bounded permission policy for `1209:000D`, record device-node ownership/mode, and execute the actual protocol as the normal `deus` user; acceptance must not silently use root for the protocol itself. A harness may use a temporary root-created udev rule or ACL solely to establish the test fixture, provided it is recorded, removed after the run, and the subsequent libusb open/claim occurs unprivileged |
| HARNESS_PASSWORDLESS_SUDO_ASSUMPTION_01 | a hardware harness fails in host precheck before target work with `sudo: interactive authentication is required` | the harness promoted passwordless `sudo -n` from a convenience into an undocumented bench prerequisite | do not require passwordless sudo unless it is an explicit frozen environment contract; when one bounded privileged host setup action is unavoidable, keep the product protocol unprivileged and use a clearly operator-visible interactive privilege boundary or a previously accepted persistent rule. Prefer ephemeral device-node ownership/ACL changes over persistent udev configuration when the permission is needed only for one enumerated test device |

---

## 14. Definition of a clean next step

A next step is clean only if:

- its preconditions are proven from evidence;
- current source/command/tool contracts are inspected;
- assertions are replayed on existing artifacts where possible;
- harness and product contracts are separated;
- failure evidence will be sufficient for causal diagnosis;
- rollback policy matches whether Flash has started;
- no already-known project fact is re-requested from the operator.

---

## 15. Delivery / execution standard

Use the narrowest execution path that can prove the required result.

### Direct repository operations

When `@DEUS MCP` (or an equivalent authorized repository tool) can safely perform a repo-only task, use it directly for:

- documentation edits/reconciliation;
- source edits that do not require the operator's local hardware/toolchain;
- Git inspection, staging, commits, fetch and ordinary non-force publication;
- read-only repository audits.

Do **not** generate a ZIP merely to make the operator apply changes that the connected repository tool can perform directly.

Direct repo mutation still requires:

- exact prestate validation;
- exact changed/staged path review;
- `git diff --check` / staged check as appropriate;
- clean commit scope;
- fresh-fetch/direct-parent proof before publication;
- ordinary non-force push only;
- fresh post-push fetch and clean `0/0` proof.

### Operator-run package

Use a self-contained ZIP runner when the task materially requires the operator's Windows environment, the remote Mac-mini USB execution domain, or the physical target, for example:

- firmware build/toolchain execution unavailable through the connected repo tool;
- STM32CubeProgrammer / SWD / UART hardware acceptance on Windows;
- current-bench target USB/libusb/CDC acceptance through bounded SSH/SCP execution on `deus@macmini`;
- Windows-specific WinUSB/Desktop acceptance when the target is actually attached to Windows;
- evidence collection that must occur on the operator machine;
- a required operation that the connected repository tool cannot execute.

For such packages the operator workflow remains download -> one PowerShell command -> return the self-contained evidence `.zip`. A loose `.log` may also be emitted for convenience, but it is optional and must duplicate the `run.log` archived in the ZIP exactly. The operator must not manually merge or edit payload files.

Unless the operator explicitly requests another form, the launch command is delivered as **one physical PowerShell line**. Its mandatory order is: locate the intended ZIP in Downloads -> verify the exact package SHA-256 supplied with the delivery -> expand into a unique GUID-named temporary directory -> resolve `apply.ps1` -> AST-parse it and require zero parser errors -> invoke only that parsed package script. The launcher must not silently select a differently named package, skip package hashing, extract over a fixed shared directory, or bypass the AST check. Package-internal native execution remains subject to the bounded-process rules above.

Every delivery ZIP must contain:

- `apply.ps1` — package runner/orchestrator;
- `manifest.json` — package identity, expected repository prestate, payload file hashes and roles;
- `payload/` when repository files must be installed;
- `PACKAGE_README.txt` — short human-readable identity and scope.

Package application rules remain fail-closed: validate package/prestate, mutate only declared scope, validate poststate, restore exact prestate on pre-Flash failure where possible, and produce sufficient PASS/FAIL evidence.

Delivery mechanics do not weaken the separation of build, hardware acceptance, documentation finalization, commit and publication gates.

### Mandatory PowerShell parser preflight

Before `apply.ps1` is executed, the operator command must parse it with the PowerShell AST parser:

```powershell
[System.Management.Automation.Language.Parser]::ParseFile(...)
```

Execution is permitted only when the parser returns **zero syntax errors**.

This preflight exists specifically to catch package-runner syntax defects before repository mutation, including interpolation hazards such as an unbraced variable immediately followed by a colon (`$Label:`). In interpolated strings, use `${Label}:` or the format operator instead.

The package-generation side must also statically reject obvious unbraced `$variable:` interpolation hazards before producing the ZIP.

### Payload post-validation policy

A payload file that is already bound by exact SHA-256 must not be rejected by a second, weaker prose-substring interpretation.

For finalized documentation:

1. verify exact payload SHA-256 before install;
2. verify exact installed SHA-256 after install;
3. verify unique structural milestone markers where a structural presence check is useful;
4. do **not** use case-sensitive prose fragments, wording variants, punctuation, capitalization, or stylistic phrasing as semantic acceptance gates.

This rule prevents a correct document from failing because `Normal-boot ...` and `normal-boot ...` differ only by capitalization.

### Full commit-candidate whitespace validation

`git diff --check` on the working tree is insufficient when the accepted file set contains untracked files, because untracked files are not part of that diff.

Before a source/document finalization gate is declared commit-ready:

1. create a temporary Git index from `HEAD`;
2. stage the exact accepted file set into that temporary index;
3. verify the temporary staged path set is exact;
4. run `git diff --cached --check` against the temporary index;
5. destroy the temporary index without touching the real index.

This check must cover tracked modifications and newly added files together.

### Canonical current-state synchronization

`docs/CURRENT_STATE.md` is the sole global project-state document.

When a product boundary is published:

1. update `docs/CURRENT_STATE.md` with the newly completed boundary and the newly active next boundary;
2. update `CHANGELOG.md` for chronology;
3. update `docs/ROADMAP.md` only if sequencing/completion history materially changes;
4. update `docs/ARCHITECTURE.md` only when stable architecture/invariants changed;
5. update the completed boundary plan/acceptance with its scoped accepted facts where required;
6. update historical ledgers/handoff only when useful, without creating another current-state authority.

`README.md`, `MASTER_EXECUTION_CHECKLIST.md`, `IMPLEMENTATION_PLAN.md` and `PROJECT_HANDOFF.md` must link to `CURRENT_STATE.md` rather than independently repeating the current boundary/next gate.

Specialized subsystem acceptance documents remain unchanged when their accepted contract is preserved; do not churn unrelated docs merely to touch every file.

### PowerShell command-token spacing

A zero-error AST parse is necessary but not sufficient.

PowerShell may parse an adjacent command/argument sequence as a different command token, for example:

- `Write-Host"message"`
- `Log'PASS'`
- `Hash$Path`
- `SaveEvidence'FAILED'`

Package-generation validation must therefore enforce:

1. a command name is followed by whitespace before its first argument;
2. package helper commands are not invoked in compressed adjacent-token form;
3. critical built-ins such as `Write-Host` are checked for the same defect;
4. the runner is written in readable statement-per-line form rather than dense semicolon-compressed command chains.

This lint is in addition to the mandatory PowerShell AST parser preflight.


## Hardware-race and disassembly-proof lessons — 2026-09-14

### HARN-CFG-01 — disassembly text order is not control-flow order

Do not prove IRQ masking / restore ordering by comparing only the nearest textually preceding instruction in `objdump` output when branches are present.

The atomic scheduler build initially produced a false negative because a READY-path `msr PRIMASK` appeared textually before a terminal abort block, while the terminal conditional branch jumped directly over that restore.

Required practice:
- parse instruction addresses;
- parse branch targets;
- prove the actual control-flow edge to the target block;
- then verify ordering inside that block/path.

A linear disassembly heuristic is acceptable only for truly branch-free local sequences.

### ARCH-RACE-01 — repeated reads do not close an interrupt race

When task state can change in IRQ context, a second unlocked READY check before terminal abort is still racy. The classification and terminal action must share one interrupt-masked critical section when correctness depends on the state remaining stable until abort.

For blocked idle, restore interrupts before `WFE`; retain the established `SEV` producer contract so an event racing between restore and `WFE` remains observable.

### EVID-HW-01 — hardware race regression must reproduce pressure, not only nominal wake

After a timing race is found under burst traffic, the acceptance regression must preserve that pressure shape. For this milestone the permanent pattern was multiple unpaced bursts with exact response counts, exact RX byte/IRQ accounting, zero drop/error/depth, and scheduler telemetry after every round.
