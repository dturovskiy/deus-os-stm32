# Deus OS — Application Stop-Failure Hardening Plan

Status: **FDC-07 / GATES 0–7 ACCEPTED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`**

Baseline: `21e45b1dd7b1b5b701b087f44d1b29676ce0e6d9`

## Purpose

Freeze fail-closed lifecycle/resource ownership before any resource-owning application is accepted.

## Confirmed audit finding

Current `application_runtime_start()` marks a current application `FAILED` when its `stop()` callback fails, increments the fault counter, but then clears `active_id` and proceeds toward starting the replacement. This loses unresolved ownership and is exactly the FDC-07 defect.

## Frozen semantics

When stopping the current non-home application:

- enter `STOPPING`;
- if `stop()` succeeds: transition to `STOPPED`, clear `active_id`, then a requested replacement may start;
- if `stop()` fails or is absent: transition current app to `FAILED`, increment fault count, **preserve `active_id` as the unresolved owner**, return failure immediately, and do not start a replacement or home fallback;
- a failed unresolved owner may not be silently restarted/replaced through the normal start path;
- successful existing built-in start/stop/repeated-start behavior remains unchanged.

The initial built-ins still own no independent peripheral resources; this boundary freezes semantics before such ownership exists.

## Frozen source boundary

Authorized:

- `src/kernel/application_runtime.c`
- `include/kernel/application_runtime.h`
- `src/kernel.c` only to execute one deterministic boot-time synthetic self-test before normal application ownership begins.

No public RPC ID, command-service ID, application ID, lifecycle numeric value or application ABI version changes.

## Deterministic synthetic proof

Add a bounded allocation-free self-test that exercises the same internal stop-transition primitive with a synthetic failing stop callback. It must prove:

- state becomes `FAILED`;
- active ownership remains the failing current ID;
- fault count increments once;
- replacement/home is not started;
- no production global runtime state is retained from the test.

## Gates

0 contract freeze; 1 implementation/static proof; 2 fresh build/resource/stack + deterministic self-test; 3 hardware regression of both built-ins and idempotence; 4 unchanged-UI disposition unless visible behavior changes; 5 docs; 6 local commit; 7 publication.

## Gate 5 accepted state

Exact candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e` implements fail-closed stop failure: the failing owner becomes `FAILED`, fault count increments once, `active_id` is preserved, and replacement/home start is forbidden until recovery. Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` includes the deterministic synthetic stop-failure proof and build/static acceptance. Hardware Gate-4 evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` binds the accepted built-in start/idempotence/normal-stop/invalid-start regression and exact final Flash. Gate 5 documentation reconciliation is complete; Gates 6–7 are accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8` with fresh-fetch clean `0/0`.

## Exit criterion

No failed resource release can silently transfer application ownership, and the exact source plus hardware regression is accepted.
