# Deus OS — Application Stop-Failure Hardening Acceptance Plan

Status: **FDC-07 / GATES 0–7 ACCEPTED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`**

Canonical design: `docs/APPLICATION_STOP_FAILURE_HARDENING_PLAN.md`

Gate 1 requires exact source scope and direct review of the transition ordering: stop failure must return before `active_id` clear, replacement start or home fallback.

Gate 2 requires warning-clean fresh firmware build, unchanged public IDs/ABI/resources, and a deterministic synthetic stop-failure self-test proving `FAILED + active_id preserved + one fault + no replacement`.

Gate 3 requires hardware regression for the existing built-ins: Home initial ownership, Device Info start, repeated same-app start idempotence, normal stop/home fallback, invalid start no mutation, semantic-event/view behavior and final Flash identity. No artificial resource-owning product app is added merely for the test.

Gate 4 is expected `PHYSICAL_OLED=N/A_UNCHANGED_UI_AUTOMATED_REGRESSION_PASS` if no visible rendering semantics change.

Gates 5–7: reconcile docs/evidence, one normal local commit, ordinary non-force publication, fresh-fetch clean `0/0`.

Accepted Gates 1–5: exact candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`; Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788` proves the deterministic `FAILED + active_id preserved + one fault + no replacement` self-test; hardware Gate-4 evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01` proves the existing built-in lifecycle/idempotence regression and exact final candidate Flash. Gate 5 reconciliation is complete; Gates 6–7 are accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8` with fresh-fetch clean `0/0`.

Failure classes: `FDC07_SOURCE_SCOPE_DRIFT`, `FDC07_STOP_FAILURE_OWNERSHIP_LOST`, `FDC07_REPLACEMENT_AFTER_FAILED_STOP`, `FDC07_SELF_TEST_FAILURE`, `FDC07_EXISTING_LIFECYCLE_REGRESSION`, `FDC07_BUILD_RESOURCE_FAILURE`.
