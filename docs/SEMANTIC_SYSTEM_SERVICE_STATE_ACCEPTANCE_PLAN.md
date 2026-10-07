# Deus OS — Semantic System / Service State Acceptance Plan

Status: **FDC-06 / GATES 0–7 ACCEPTED / PUBLISHED `6aa2df19ab02c14bde38833e738fe825008102e8`**

Canonical design: `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md`

Gate 1 requires exact source scope and a static dependency proof that `application_service_snapshot_t` and OLED indicator/time composition consume semantic state directly; no code may derive health/USB/network truth from `OLED_STATUS_INDICATOR_*` values.

Gate 2 requires fresh warning-clean firmware build, unchanged public ABI, bounded static state, resource/stack ceilings retained, and deterministic proof for equal-state/no-event, minute change, system-health change, USB change and network change.

Gate 3 requires hardware proof that application semantic-event counters/types remain correct, unchanged semantic state does not create spurious events/rerenders, USB and scheduler health truth remain correct, and the final Flash equals the Gate-2 candidate.

Gate 4 requires physical OLED regression because the same upstream state now feeds the status presentation. Visible SYSTEM/USB/NETWORK/uptime behavior must remain accepted with no blank pulse/flicker/stale state.

Gates 5–7 are docs/evidence reconciliation, one normal local acceptance commit and ordinary non-force publication with fresh-fetch clean `0/0` proof.

Accepted Gates 1–5: exact candidate tree `bb99acf111dfa3a78193b4e5d3376fa077defa1e`, application SHA-256 `0EC605A42511C9E71BE9B0D9BE96B5F0B0EBAFC12E81FFC509A416E5FCE14446`; Gate-1..3 evidence SHA-256 `E5A9545F0FEAFB601234E8BE8B2D2D184D1614F7B96BC44C99C88BF26B113788`; final hardware evidence SHA-256 `72DD52218DF50D5DEFFEDB796855666DED92D00D1053488CC3B68C93D89AFC01`. The hardware proof accepts semantic event/no-rerender behavior, scheduler/USB health, exact Flash identity and physical OLED PASS. Gate 5 reconciliation is complete; Gates 6–7 are accepted/published at `6aa2df19ab02c14bde38833e738fe825008102e8` with fresh-fetch clean `0/0`.

Failure classes: `FDC06_SOURCE_SCOPE_DRIFT`, `FDC06_PRESENTATION_AUTHORITY_RETAINED`, `FDC06_SEMANTIC_EVENT_REGRESSION`, `FDC06_RERENDER_REGRESSION`, `FDC06_BUILD_RESOURCE_FAILURE`, `FDC06_OLED_REGRESSION`.
