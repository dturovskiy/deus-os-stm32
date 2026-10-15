# Public Repository Sanitization — owner stop decision and forward-only release view

Status: **OWNER POLICY / NON-DESTRUCTIVE CONTINUATION DECISION (2026-10-14)**. This document records the programme's **2026-10-14 owner-decision checkpoint**; the programme subsequently closed under the same bounded forward-only policy. Final result: `docs/PUBLIC_REPOSITORY_SANITIZATION_FINAL_ACCEPTANCE.md`. `docs/CURRENT_STATE.md` remains the sole global status authority.

## Gate-2 factual disposition and owner exception

- Independent source Git/current HEAD restoration: **PASS** (private bundle restored 175 commits, 268 tracked paths; 36/36 selected originals). Historical 58 deleted evidence archives are not required by the owner's earlier 2026-10-14 scope decision.
- V10 local Windows encrypted recovery material creation and disk reopen: **scoped PASS**, proven by actual nonsecret evidence. It is **not** proof of independent off-host storage, a second-account restore, or formal custodial attestation.
- The owner explicitly directed **stop further work on recovery-key / escrow proofs** on 2026-10-14 and proceed with the pre-existing sanitization plan. `OWNER_STOP_ADDITIONAL_CUSTODY_WORK=YES`. The unmet independent custody/restore checks are recorded as **`NOT_VERIFIED / OWNER_ACCEPTED_DEFERRED_RISK`**, not falsely marked PASS. Do not create more key, firmware, SWD, passphrase, backup or restore operator bundles in this programme.
- Owner-authorized practical disposition: `GATE2=OWNER_CLOSED_WITH_RECORDED_LIMITS` **for the non-destructive forward-only repository hygiene path only**, substituting an explicit owner risk exception for the original independent-custody proof. This is not `GATE2_TECHNICAL_PASS`; a separate future production release/security gate cannot inherit the missing proof. Do not alter the historical V10 result or previously accepted firmware gates.

## Gate-3 evaluated release strategy and authority

The owner already approved `ACCEPT_RESIDUAL_HISTORY` on 2026-10-11, for known historical operator-path exposure, and reaffirmed priority to continue the project. Rechecked the existing public release view:

| Strategy | Evaluation | Decision |
|---|---|---|
| A — create another public repository | Additional remote, link/CI/issue migration, changed provenance, potential historic replicas remain | Rejected for this maintenance gate |
| B — rewrite public Git history | Commit/tag/clone provenance breaks, forced non-fast-forward update, old copies/caches remain | Rejected; no force |
| C — prospective-only cleanup in existing public repo | Preserves original public history and project links; known old operator paths stay reachable | **Selected: C** |

This is the technical plan's **option C** and the previously owner-named **strategy A (forward-only)**: the labels belong to different decision tables, but the selected behavior is identical. No new public remote, visibility change, deletion, history rewrite, re-tagging or force push.

**Public release scope frozen for next implementation review:**

- `.gitignore`: add protection against accidentally staged `*.dpapi` and `*.pkg` artifacts. Ignore rules are preventive, not protection against an intentional `git add -f`.
- `.github/workflows/ci.yml`: extend forbidden tracked-output checks to those extensions and prove representative synthetic positive/negative cases within CI. No production secret files or binaries included in tests.
- Existing published docs and current source/protocol/Host contracts remain unchanged unless a narrowly necessary public status/acceptance reference is appended by the normal governance gate.
- Preexisting 113/113 grouped owner-disposition outcomes, 36/36 preserved source originals and the accepted historic redactions/script portability are retained; no second mass cleanup or reclassification without evidence.

**Residual limits:** current `.gitignore` is not an exfiltration guarantee; known historical operator references are deliberately retained in old commits; unobservable clones/caches/expired workflow logs are not certified absent; detection of a concrete new secret would trigger a separate incident response rather than quietly changing this selection.

## Gate-4 / Gate-5 execution contract

Keep source firmware, ARM build inputs, bootloader, host projects, protected operator files, private Git and target hardware untouched. Apply the exact two-path hygiene scope, run deterministic local negative fixtures and non-vacuous staged path/whitespace review, publish only by ordinary non-force commit/push after a fresh fetch/direct-parent proof, and require hosted exact-SHA CI (Core >=84, Transport >=24, Release warning/error free) as applicable. A Gate-4 source diff or CI result does not imply final sanitized-release acceptance; Gates 5–7 require their own checked proofs.

**Gate-2 provenance is preserved, not replayed. Gate-3 strategy is selected. Next authorized practical work: the two-path forward-only CI/ignore hardening candidate.**
