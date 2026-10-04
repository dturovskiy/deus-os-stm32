# Deus OS — Semantic System / Service State Acceptance Plan

Status: **FDC-06 / GATE 0 ACCEPTED CONTRACT**

Canonical design: `docs/SEMANTIC_SYSTEM_SERVICE_STATE_PLAN.md`

Gate 1 requires exact source scope and a static dependency proof that `application_service_snapshot_t` and OLED indicator/time composition consume semantic state directly; no code may derive health/USB/network truth from `OLED_STATUS_INDICATOR_*` values.

Gate 2 requires fresh warning-clean firmware build, unchanged public ABI, bounded static state, resource/stack ceilings retained, and deterministic proof for equal-state/no-event, minute change, system-health change, USB change and network change.

Gate 3 requires hardware proof that application semantic-event counters/types remain correct, unchanged semantic state does not create spurious events/rerenders, USB and scheduler health truth remain correct, and the final Flash equals the Gate-2 candidate.

Gate 4 requires physical OLED regression because the same upstream state now feeds the status presentation. Visible SYSTEM/USB/NETWORK/uptime behavior must remain accepted with no blank pulse/flicker/stale state.

Gates 5–7 are docs/evidence reconciliation, one normal local acceptance commit and ordinary non-force publication with fresh-fetch clean `0/0` proof.

Failure classes: `FDC06_SOURCE_SCOPE_DRIFT`, `FDC06_PRESENTATION_AUTHORITY_RETAINED`, `FDC06_SEMANTIC_EVENT_REGRESSION`, `FDC06_RERENDER_REGRESSION`, `FDC06_BUILD_RESOURCE_FAILURE`, `FDC06_OLED_REGRESSION`.
