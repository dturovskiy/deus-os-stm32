# Deus OS — Application Stop-Failure Hardening Acceptance Plan

Status: **FDC-07 / GATE 0 ACCEPTED CONTRACT**

Canonical design: `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md`

Gate 1 requires exact source scope and direct review of the transition ordering: stop failure must return before `active_id` clear, replacement start or home fallback.

Gate 2 requires warning-clean fresh firmware build, unchanged public IDs/ABI/resources, and a deterministic synthetic stop-failure self-test proving `FAILED + active_id preserved + one fault + no replacement`.

Gate 3 requires hardware regression for the existing built-ins: Home initial ownership, Device Info start, repeated same-app start idempotence, normal stop/home fallback, invalid start no mutation, semantic-event/view behavior and final Flash identity. No artificial resource-owning product app is added merely for the test.

Gate 4 is expected `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS` if no visible rendering semantics change.

Gates 5–7: reconcile docs/evidence, one normal local commit, ordinary non-force publication, fresh-fetch clean `0/0`.

Failure classes: `FDC07_SOURCE_SCOPE_DRIFT`, `FDC07_STOP_FAILURE_OWNERSHIP_LOST`, `FDC07_REPLACEMENT_AFTER_FAILED_STOP`, `FDC07_SELF_TEST_FAILURE`, `FDC07_EXISTING_LIFECYCLE_REGRESSION`, `FDC07_BUILD_RESOURCE_FAILURE`.
