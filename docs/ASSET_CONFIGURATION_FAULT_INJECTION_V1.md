# Deus OS — Asset / Configuration Deterministic Fault-Injection Matrix v1

Status: **GATE 0 CONTRACT FROZEN — DOCS ONLY — NO FAULT HOOK OR TARGET MUTATION AUTHORIZED**

Boundary:

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION`

Contract:

`ASSET_CONFIGURATION_FAULT_INJECTION_V1`

## 1. Purpose

This contract freezes deterministic reset/power-loss recovery acceptance for the A/B persistence transaction.

The goal is not to prove one lucky cable pull.

The goal is to force reset at exact state-machine boundaries and prove that every reachable persistent state resolves to exactly one of:

- previous committed record;
- newly committed record;
- compiled default when no valid record exists.

Ambiguous authority is a failure.

## 2. Deterministic injection mechanism

Gate 1 may add one narrow acceptance hook:

`asset_persistence_fault_injection_control`

Properties:

- exactly one volatile 32-bit selector in ordinary RAM/BSS;
- default value `0` = disabled;
- not persisted;
- not exposed through Asset transfer frames;
- not exposed through normal Host Control UI/CLI;
- not a new configuration object;
- armed only by the hardware acceptance harness through SWD/debug memory access;
- checked only at frozen persistence checkpoints;
- matching checkpoint requests a Cortex-M system reset;
- after reset normal BSS initialization clears the selector back to `0`.

Selector encoding:

```text
bits 31..16  arm magic = 0xA55A
bits 15..8   reserved = 0
bits 7..0    checkpoint id
```

Any value without exact arm magic is ignored.

A selected checkpoint can therefore reset only if execution actually reaches that checkpoint.

Static review must prove the hook is not reachable through ordinary product protocol input.

## 3. Reset primitive

The deterministic injection action is a Cortex-M system reset using the SCB AIRCR `SYSRESETREQ` mechanism.

The hook must not:

- deliberately trip IWDG;
- reload IWDG;
- erase additional Flash;
- write a special persistent “test result” record;
- alter option bytes.

The expected injected reset classification is software/system reset, not watchdog reset.

## 4. Frozen transaction checkpoints

Checkpoint IDs:

```text
0  DISABLED

1  BEFORE_ERASE
2  AFTER_ERASE_VERIFY
3  AFTER_ENVELOPE_METADATA
4  AFTER_FIRST_PAYLOAD_HALFWORD
5  AFTER_ALL_PAYLOAD
6  AFTER_HEADER_CRC_AND_READBACK_VERIFY
7  AFTER_COMMIT_MARKER_WRITE_READBACK
8  AFTER_FULL_COMMITTED_RECORD_REVALIDATION
9  AFTER_RUNTIME_ACTIVATION_BEFORE_RESPONSE
```

Exact meanings follow.

### FI-1 BEFORE_ERASE

Reached after:

- candidate/session/consumer validation;
- inactive slot selection;
- unchanged-data suppression.

Reached before:

- setting page erase in progress;
- any Flash mutation for this commit.

### FI-2 AFTER_ERASE_VERIFY

Reached after:

- inactive page erase completed;
- erased-page verification passed.

Reached before any envelope/payload programming.

### FI-3 AFTER_ENVELOPE_METADATA

Reached after all non-CRC metadata halfwords that are intended to be programmed have completed.

Commit marker remains erased.

Payload remains unprogrammed.

### FI-4 AFTER_FIRST_PAYLOAD_HALFWORD

Reached after the first payload halfword has completed and before the second payload halfword.

This is the deterministic **partial-programming** state.

For v1 acceptance it represents interruption during the multi-halfword programming sequence; it does not claim to electrically interrupt the internal programming pulse inside one 70-us halfword operation.

### FI-5 AFTER_ALL_PAYLOAD

Reached after all declared payload halfwords have completed.

Header CRC and commit marker are still erased.

### FI-6 AFTER_HEADER_CRC_AND_READBACK_VERIFY

Reached after:

- header CRC programmed;
- envelope/payload readback complete;
- pre-commit CRC/consumer validation passed.

Reached immediately before final commit-marker programming.

This is the last state in which the new record must remain non-authoritative.

### FI-7 AFTER_COMMIT_MARKER_WRITE_READBACK

Reached after:

- `0xA55A` commit marker programmed;
- marker read back successfully.

Reached before the normal full committed-record validator executes.

A structurally complete valid record may already be authoritative after reboot because boot selection validates persistent bytes independently of interrupted runtime control flow.

### FI-8 AFTER_FULL_COMMITTED_RECORD_REVALIDATION

Reached after the new slot passes the complete normal committed-record validator.

Reached before runtime consumer activation.

After reset, the new committed record must be selected.

### FI-9 AFTER_RUNTIME_ACTIVATION_BEFORE_RESPONSE

Reached after runtime state was activated from the new committed record.

Reached before successful COMMIT response is sent to host.

After reset, persistent selection must still choose the same new record.

Host timeout/retry semantics must not cause a second Flash mutation.

## 5. Mandatory campaign A — no previous persistent record

Precondition:

- both persistence pages are erased/invalid;
- runtime uses compiled `OLED_UI_LAYOUT_CONFIG_V1` default;
- no valid committed generation exists.

Use one non-default valid 8-byte candidate.

Required injections:

| Checkpoint | Expected after reset |
| --- | --- |
| FI-1 BEFORE_ERASE | compiled default |
| FI-2 AFTER_ERASE_VERIFY | compiled default |
| FI-4 AFTER_FIRST_PAYLOAD_HALFWORD | compiled default |
| FI-6 BEFORE_COMMIT_MARKER | compiled default |
| FI-7 AFTER_COMMIT_MARKER | new record if and only if complete validator passes |

For every pre-marker injection:

- no valid persistent record may appear;
- boot performs zero repair writes;
- runtime is exact compiled default.

FI-7 proves that persistent authority is byte/validator based, not dependent on returning from the original COMMIT call.

## 6. Mandatory campaign B — previous valid record exists

Precondition:

- slot A contains generation `N`, valid old payload;
- slot B is inactive/invalid;
- runtime uses generation `N`;
- candidate payload differs and would produce generation `N+1`.

Run every checkpoint FI-1 through FI-9 in controlled trials.

Expected result:

| Checkpoint | Expected authoritative record after reset |
| --- | --- |
| FI-1 | previous generation N |
| FI-2 | previous generation N |
| FI-3 | previous generation N |
| FI-4 | previous generation N |
| FI-5 | previous generation N |
| FI-6 | previous generation N |
| FI-7 | new generation N+1 |
| FI-8 | new generation N+1 |
| FI-9 | new generation N+1 |

No trial may produce a mixed payload, ambiguous slot selection or partially activated consumer.

## 7. Trial isolation

Each trial starts from an explicitly proven precondition.

The harness must not assume that the prior trial left the desired slot state.

Before each trial it records:

- selected active slot;
- generation;
- object type/schema/length;
- payload CRC;
- exact readback payload;
- candidate inactive-slot validity.

If the required precondition is absent, the harness restores it through the accepted persistence protocol or the separately accepted recovery procedure before arming the next injection.

A trial is not accepted merely because the device rebooted.

## 8. Required post-reset proof for every trial

After every injected reset:

1. device enumerates/returns to normal runtime;
2. reset classification is compatible with intentional system reset and not an unexpected IWDG reset;
3. management HELLO/RPC baseline still works;
4. persistence STATUS/readback reports the expected selected generation or NOT_FOUND;
5. runtime OLED consumer state equals the selected persisted payload or compiled default;
6. status bar remains frozen/system-owned;
7. boot caused zero persistence erase/program activity;
8. management/CDC/UART drop/error state remains within accepted reset/reconnect expectations;
9. scheduler faults/canaries remain healthy;
10. firmware Flash outside pages 62/63 remains exact.

## 9. COMMIT-response-loss idempotency trial

FI-9 intentionally allows the host to lose the successful response because reset occurs before it is transmitted.

After reboot the host must:

- query STATUS/readback first;
- recognize that the desired generation/payload is already committed;
- treat the original operation as successful/recovered;
- not issue a blind duplicate changed commit.

If COMMIT is retried explicitly with identical committed identity, the target's idempotency rule must return the existing generation with:

```text
erase delta    0
program delta  0
generation delta 0
```

## 10. Corruption matrix

Reset injection is supplemented by deterministic invalid-record construction.

At minimum prove boot fallback for:

1. valid-looking metadata + erased commit marker;
2. valid commit marker + bad header CRC;
3. valid commit marker/header + bad payload CRC;
4. unsupported schema;
5. unsupported object type;
6. payload length inconsistent with consumer;
7. conflicting equal-generation valid-looking A/B records.

Expected fallback follows `ASSET_CONFIGURATION_PERSISTENCE_V1`.

Boot must not repair any of these states automatically.

Construction of corrupt states is an acceptance-only destructive operation restricted to pages 62/63 and must be followed by recovery.

## 11. Fault-injection wear bound

The complete deterministic campaign must declare before execution the maximum number of page erases it can consume.

Frozen acceptance limit for the full Gate-5 fault campaign:

`<= 64 deliberate persistence-page erase attempts total`

This is <1% of the 10k datasheet minimum per-page endurance even in the pathological case that every erase hit one page.

The harness records actual **page 62/63** erase-attempt deltas, including those caused by CLEAN_STATE recovery during the campaign. PRESERVE_PERSISTENCE is required to contribute zero page-62/page-63 erases.

Erases of relocation-headroom pages 54..61 or application sectors are recovery-tool evidence and are recorded separately; they do not consume this persistence-page endurance counter.

An accidental unbounded loop is a harness failure and must be stopped.

## 12. Physical power-loss corroboration

Repeated manual cable removal is **not** required for systematic coverage.

The deterministic matrix above is authoritative because it covers exact transaction boundaries.

After it passes, one physical power-removal/power-return retention proof is required for the final selected valid configuration:

- commit a known non-default valid object;
- verify readback/runtime state;
- remove target power once;
- restore power;
- require the same committed generation/payload and normal runtime.

This single physical proof confirms nonvolatile retention.

It does not replace FI-1..FI-9 and does not need to race a 70-us programming pulse.

## 13. Optional asynchronous destructive corroboration

If later automated power-control hardware exists, acceptance may additionally cut power asynchronously during repeated changed commits.

That evidence is supplemental.

It must not become a prerequisite for v1 and must stay inside the 64-erase fault-campaign budget unless Gate 0 is explicitly reopened.

## 14. Harness evidence

Each deterministic trial records:

- exact firmware candidate identity;
- checkpoint ID;
- injected selector value;
- active/inactive slot prestate;
- generation prestate;
- candidate payload/CRC;
- observed reset/reconnect;
- post-reset slot validation;
- selected generation/payload;
- runtime consumer values;
- erase/program diagnostic deltas;
- final PASS/FAIL classification.

The harness must distinguish:

- product recovery failure;
- expected injected reset;
- host reconnect failure;
- harness inability to arm/observe the checkpoint.

A harness failure does not become a persistence product failure.

## 15. Gate blocking rule

Gate 5 cannot pass unless:

- every mandatory FI checkpoint campaign result matches the matrix;
- corruption matrix passes;
- COMMIT-response-loss idempotency passes;
- one physical retention power cycle passes;
- erase-attempt campaign budget is respected;
- recovery procedure can restore a known-good firmware/persistence state.

The last item depends on the separately frozen recovery-bundle/ST-LINK contract.

## 16. Non-goals

This matrix does not attempt to characterize:

- analog brownout waveforms;
- exact internal Flash-cell state during a sub-halfword power collapse;
- malicious infinite local power cycling;
- electromagnetic fault injection;
- security/authentication bypass.

Those require separate hardware/security qualification.
