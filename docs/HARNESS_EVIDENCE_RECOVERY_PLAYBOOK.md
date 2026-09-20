# STM32 OS — Harness, Evidence, Failure Classification & Recovery Playbook

Status: project engineering rulebook
Scope: Windows host scripts, GNU Arm build gates, UART diagnostics, STM32CubeProgrammer/ST-LINK hardware acceptance, Git evidence gates.

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

---

## 4. Collector rule for unavailable Windows/hardware facts

ChatGPT cannot directly execute the user's installed Windows GNU toolchain, COM3, STM32CubeProgrammer, ST-LINK or physical MCU.

Therefore:

- never guess those facts;
- first recover them from prior project chats/evidence;
- if still unknown, generate a **read-only collector**;
- collector emits `.log + .zip`;
- collector must not edit source/docs, build unless explicitly needed, flash, commit or push.

The project already has authoritative host facts from the first project chat:

- PowerShell 7;
- Arm GNU Toolchain 15.3.Rel1;
- STM32CubeProgrammer 2.23.0;
- COM3 / CH340;
- UART 115200 8N1;
- ST-LINK V2J48S7;
- SWD 4000 kHz;
- STM32F101/F102/F103 medium-density target;
- fixed project/tool paths.

Do not ask the operator to repeat stable facts already recorded.

---

## 5. Script preflight before handing a script to the operator

Every new script must be validated against **current artifacts**, not memory.

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

### Gate outcome vs process failure

An expected acceptance result such as `PRODUCT_RESOURCE_BUDGET`, protocol rejection, hardware mismatch, or other successfully classified gate FAIL is not a harness crash. Once `run.log`, `outcome.txt` and the evidence archive have been written successfully, the operator-facing script must terminate normally and print the gate outcome/classification. Do not `throw` merely to convert an accepted `OUTCOME=FAIL` into process exit code 1; that can cause terminal wrappers to close/restart and obscures the distinction between product failure and harness execution failure. Reserve process-level failure for cases where trustworthy evidence could not be produced or package/AST/prestate integrity itself failed before normal evidence finalization. This failure class is `HARNESS-EXIT-SEMANTICS-01`.

### Operator-facing terminal structure

The chronological `run.log` remains authoritative, but the terminal must not present a wall of undifferentiated command lines. Every acceptance harness must emit visible section headers for at least `PRESTATE`, `BUILD`, `STATIC`, `RESOURCES`, `ARTIFACTS`, and `RESULT`; hardware gates add `TARGET`, `ENUMERATION`, `RUNTIME`, and `READBACK` as applicable. Each section prints concise PASS/FAIL status while detailed command stdout/stderr continues into `run.log`.

The final terminal result must be visually dominant and color-coded with `Write-Host`: green foreground for PASS, red foreground for FAIL, yellow for classified warnings/skips. The final block must include gate name, `RESULT=PASS|FAIL`, classification, candidate identity and the most important resource/runtime figures, followed by the result log/evidence paths. Do not rely on an uncolored prose line buried after command output. If `$Host.UI` coloring is unavailable, print an ASCII banner such as `========== GATE 2 PASS ==========` as a non-color fallback.

A categorized summary is presentation only; evidence ownership remains unchanged: raw chronological commands stay in `run.log`, structured final state stays in `outcome.txt`, artifact identity stays in hash/candidate files.

Reserve the words `PASS` and `FAIL` for the final gate result. Intermediate successful checks print `[OK]`; recoverable/nonfatal differences print `[WARN]`; an intermediate blocking check prints `[ERROR]`. This prevents a log from appearing to contain both a gate PASS and a gate FAIL when an early subcheck succeeds but the gate later stops.

For a hardware gate that consumes an already accepted build candidate, source identity is owned by the accepted Git candidate tree, not by raw filesystem SHA-256 of individual working-tree files. Recompute the candidate with a temporary Git index over the authorized source paths and require the exact accepted tree hash. Raw filesystem hashes may be captured diagnostically, but must not independently fail the hardware gate when the normalized candidate tree, dirty set and real index are exact; line-ending/encoding filters can change raw bytes without changing Git content identity. This failure class is `PRESTATE-RAW-HASH-OWNER-01`.

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

Bundle should contain, as applicable:

- run log;
- outcome classification;
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

---

## 13. Current known-gotcha registry

| ID | Symptom | Root cause | Required prevention |
|---|---|---|---|
| PWR-EMPTY-01 | null/empty array binding error | empty pipeline materialization | `@(...)`, AllowNull/AllowEmptyCollection |
| PWR-EMPTY-02 | empty string rejected in string array | mandatory binding rejects empty string | AllowEmptyString or skip empties |
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
| PWR-PARSER-01 | package runner fails before line 1 execution | interpolated `$variable:` parsed as scoped-variable syntax | AST-parse `apply.ps1` before execution; use `${variable}:` or format operator |
| DOC-CHECK-01 | correct finalized document rejected by package validator | validator searched case-sensitive prose (`normal-boot` vs `Normal-boot`) even though payload SHA was exact | validate exact post-SHA plus unique structural milestone markers; do not re-parse prose semantics with brittle substrings |
| UNTRACKED-CHECK-01 | finalization passes `git diff --check`, commit gate later finds whitespace in a new file | ordinary `git diff --check` does not include untracked files | validate the full would-be commit through a temporary Git index and run `git diff --cached --check` there before accepting finalization |
| DOC-CURRENT-STATE-01 | new chat reads contradictory current state | several documents independently claimed global current/next authority | `docs/CURRENT_STATE.md` is the sole global project-state source; other docs keep scoped roles and link to it rather than duplicating current-state blocks |
| PWR-CMDTOKEN-01 | AST parse passes but runner fails with “term is not recognized” for a helper/built-in command | PowerShell command name was immediately adjacent to its first argument (`Write-Host"..."`, `L'...'`, `Sha$p`, `SaveEvidence'PASS'`), so tokenization produced a different command name | require whitespace between a command name and every argument; generator statically rejects adjacent-argument calls for all package helper commands and critical built-ins |

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

Use a self-contained ZIP runner when the task materially requires the operator's local Windows environment or physical target, for example:

- firmware build/toolchain execution unavailable through the connected repo tool;
- STM32CubeProgrammer / SWD / UART / USB hardware acceptance;
- Windows-specific WinUSB/Desktop acceptance;
- evidence collection that must occur on the operator machine;
- a required operation that the connected repository tool cannot execute.

For such packages the operator workflow remains download -> one PowerShell command -> return `.log` + evidence `.zip`. The operator must not manually merge or edit payload files.

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
