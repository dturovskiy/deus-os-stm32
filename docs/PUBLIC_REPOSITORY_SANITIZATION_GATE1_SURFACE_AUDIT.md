# Deus OS — Gate-1 Historical and GitHub Publication Surface Audit

Status: **GATE-1 READ-ONLY PARTIAL PROOF, NOT FULL ACCEPTANCE OR SECURITY CLEARANCE**

Contracts: `docs/PUBLIC_REPOSITORY_SANITIZATION_PLAN.md` and `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`. Per-path evidence: `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1_MATRIX.md`.

Frozen candidate: `6781926af2dcbe02216ddb4f6da0272ffbb72f8f`. The present tracked-file inventory has **254/254 fully read contents** and an exact Git blob ID per path in the companion matrix, with first-pass `PUBLIC=141 / REDACT=15 / PRIVATE=49 / RETAIN=49`. These are proposed classifications, **not authorization to delete files**. This report and the matrix are new files, not part of the frozen 254-file baseline.

## History / ref coverage

| Read-only query | Observed evidence | Scope limitation |
|---|---|---|
| `git log main --format=%H` | **140 unique main commits** | Current local public-main ancestry, not all forks/deleted or unreachable objects |
| `git log --all --format=%H` | **143 unique commits** | Additional **three commits reachable through local `refs/stash`**; local stash is not assumed published and its contents were not copied into evidence |
| Local refs | `refs/heads/main`, `refs/remotes/origin/HEAD`, `refs/remotes/origin/main`, local `refs/stash` | Local stash must not be exported or treated as public |
| `git log --all --name-only` | **254 unique names** across 1,347 nonempty path lines | Filename inventory ≠ historical content audit |
| `git log main --diff-filter=D` | 0 deletion-name results | Does not imply no edited/overwritten historical content |
| GitHub refs | **1 public branch `main`, 0 tag refs** | Other clones/mirrors not enumerable from this result |

**History-diff heuristic scans:** read-only `git log main -G` found changes to Windows-user-path patterns in **4 commits**, current/previous SSH endpoint patterns in **3**, workstation-project-root patterns in **14** (possible overlapping commits; neither unique secrets nor severity counts). Earlier commit `9fa772a` corrected a current-file account path, but its predecessor remains in reachable Git history. The same limited diff regex inspection found no matches for PEM key headers, GitHub/AWS token prefixes, selected live-token signature, bearer-authorization pattern or ordinary password/API-key assignment. **These are not exhaustive blob-by-blob history or entropy scans.** No confirmed production-key disclosure follows from this evidence.

## Public GitHub hosting surfaces

| Surface | Measured result | Limit |
|---|---|---|
| Repository | public / `main` | Verified via live metadata |
| Branches / tags | 1 / 0 | Current reachable public refs only |
| Releases | 0 | Current REST releases collection |
| Pull requests / issues | 0 / 0 | Current REST collections, not deleted/externally quoted content |
| Forks | repository metadata `forks_count=0` | Direct forks list unavailable; mirrors unknown |
| Discussions | `has_discussions=false` | External conversations uninspected |
| GitHub Actions run list | 22 total; 21 success, 1 failure | Historical deleted runs not recoverable through current results |
| Actions artifacts | **0 stored artifacts across all 22 run-artifact queries** | Expired/deleted/external artifacts not excluded |
| Actions job logs | **21 logs read/scanned** (roughly 34.7 KiB each), the oldest failed run returned no retrievable job | That one historical job is **NOT_CHECKED** |
| High-confidence log signatures | 0 hits for common PEM/GitHub/AWS/Bearer patterns in 21 logs; no developer Windows-root/WSL-home/operator-SSH endpoint patterns in those logs | Does not prove there are no context-specific disclosures |
| Absolute paths in CI logs | All 21 logs contained only the observed standard GitHub-runner home path class (60 references per checked job), **not the developer bench home/path categories scanned** | Normal CI-runner directories are not the same as exposed workstation identities |
| GitHub Actions caches | **NOT_CHECKED**; current connector rejects cache REST endpoint | Requires authorized separate API/tool access |
| Outside mirrors, clones, search caches | **NOT_CHECKED** | Cannot claim erasure on third-party replicas |
| Branch rulesets/protection | **NOT_CHECKED** | Separate governance/permissions question |

No raw operator SSH identity, full workstation path, privilege command or key-bearing recovery content is reproduced in this public report.

## Unresolved Gate-1 conditions

1. **49 `RETAIN`** cases require content/contract-owner decisions; all **49 `PRIVATE`** suggestions require an archive, public-safe replacement decision, and dependency/provenance proof. All rows require human risk/consumer adjudication rather than accepting lexical triage blindly.
2. All seven executable PowerShell default-path cases require exact override/call-graph/generated-recovery dependency review; toolchain and hardware-safety behavior must not change in the Gate-1 read-only review.
3. Full historical blob contents/credential scanning (not merely filenames/`git log -G`) and unavailable GitHub-side surfaces remain open. Report coverage gaps explicitly.
4. Preserve accepted evidence, bootloader recovery trust chain and source-of-truth precedence in a later authorized private archive; **Gate-2 cannot remove/migrate content without tested preservation**.
5. A strategy among new sanitized public repository, history rewrite or forward-only minimization requires a dedicated Gate-3 decision and explicit user approval; no strategy is silently selected.

**Verdict:** current-tree first-pass inspection **COMPLETE 254/254**; GitHub reachable surfaces **PARTIALLY CHECKED**; Gate-1 final acceptance **OPEN / NOT PASSED**. No source changes, target access, history changes, file removals or private artifact publishing authorized.
