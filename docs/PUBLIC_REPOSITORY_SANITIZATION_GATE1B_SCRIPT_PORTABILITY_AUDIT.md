# Deus OS — Gate-1B Script Portability and Hosting Preflight

Status: **READ-ONLY TECHNICAL PREPARATION; NOT OWNER SIGN-OFF, NOT GATE-1/1B CLOSURE, NOT GATE-2 ACCEPTANCE.**

Boundary: `PUBLIC_REPOSITORY_SANITIZATION`. This is a subordinate, non-authorizing evidence record. Global state remains `docs/CURRENT_STATE.md`; file-by-file owner proposal remains `docs/PUBLIC_REPOSITORY_SANITIZATION_GATE1B_OWNER_DECISIONS.md`; acceptance criteria remain `docs/PUBLIC_REPOSITORY_SANITIZATION_ACCEPTANCE_PLAN.md`.

## Verified baseline and limits

- Baseline before this audit: public `main == origin/main == b3d32c8bf435c2facc2750c531e2819a76906870`, clean ahead/behind `0/0`. The public Git tree contains **263 tracked files** and **9 tracked PowerShell files**. No tracked `host/**/obj/**` or `host/**/bin/**` files are present. Locally generated .NET `obj/` metadata observed in broad working-directory searches is **not** published Git content.
- Public GitHub repository reports `PUBLIC`, default branch `main`, one branch, zero forks and no published Releases at audit time. A recent exact-SHA GitHub Actions run completed successfully; its absence of workflow artifacts does not establish absence of artifacts/caches/log contents for all historic runs. Unindexed forks, external mirrors, cloning, caches and third-party archives are **NOT VERIFIED**.
- Original documentation archive reports `PRIVATE`, with **29 selected Markdown originals** and clean local Git state `0/0`. An independent cross-repository readback compared all **29/29 full local archive contents** against the appropriate old public GitHub commit (`f728e494…` for 19, `4588985d…` for nine, `3a5ccfa7…` for IWDG): **29 identical, zero missing/different/truncated**. This is not a claim that all project source/evidence/history is archived. Independent fresh-clone restore under separate credentials and anonymous access-denial proof are **NOT VERIFIED**; a separate GitHub integration cannot retrieve its private content, which does not establish an anonymous test. Do not copy the private archive into public CI or a public ZIP.
- Previous current-tree Markdown screen eliminated the identified model/name, workstation checkout path and literal historic UART-port patterns; historic reachable Git blobs retain workstation/home paths. No actual production signing-key exposure has been proven by the bounded recognized-credential scans.

## Exact seven-script default-path audit (read-only; frozen bytes)

The seven paths below are the approved *audit scope*, **not** authorized mutation scope. SHA-256 values are local file-content fingerprints; they are not evidence of Windows PowerShell parsing, firmware equivalence, safe target behavior or source Git blob SHA.

| Script | SHA-256 of inspected bytes | Default-path role and risk boundary |
| --- | --- | --- |
| `scripts/build_bootloader.ps1` | `B9AF5D69BE7B4A6EF21627CFFD2FF5931CE986031BEE8C7559F54919CD33B453` | Explicit `-ToolchainRoot` parameter with workstation-bound default; tool presence, 32-byte `UpdateKeyFile`, bootloader link ceilings and nondeployable-key test isolation |
| `scripts/build_firmware.ps1` | `78CBFC59A08C5D9115E2B0C52282ADCC67FDDE11B61138C745D3002BDB59506B` | Explicit `-ToolchainRoot`; deterministic firmware build, source-tree identity and fail-closed output-directory ownership |
| `scripts/create_asset_recovery_bundle.ps1` | `A6ECCB9AE263BD43F112A1354895651E64A7904723B251B59F07C1A86E8B29E8` | **Historical-only** Asset generator; emits two wrapper scripts containing a programmer default; preserve 54-KiB-era historical geometry and never select this as current bootloader recovery |
| `scripts/create_bootloader_recovery_bundle.ps1` | `6CC87F969908EF4F8F8D1724B3B53CA485B12EC0D5123C9ADA4AFFBB5A5B617F` | Current recovery generator; emits validate-only, preserve and clean wrappers with programmer defaults; signed-update vector/manifest and 62-page recovery geometry are hash-bound |
| `scripts/stm32_asset_recovery.ps1` | `670667F21BE9890AC3BBC0C4B8CF968D26F241DF84550B97800217EEE092AAFD` | **Historical-only** direct recovery; explicit `-ProgrammerCli`, destructive PRESERVE/CLEAN modes and independent post-readback proof |
| `scripts/stm32_bootloader_recovery.ps1` | `8C199ABFDA4EC8733AAB881F2A29BE7DBBE1B77CF3ED15C0F3FCEE022F3D5E15` | Current direct recovery; explicit `-ProgrammerCli`; VALIDATE_ONLY distinct from target-writing PRESERVE/CLEAN, pages 62/63 and full 64-KiB verification |
| `scripts/stm32_readonly_flash_preflight.ps1` | `CD30E562D6F7B4549214AC2AA8E52E863C8DA5296F28F716C643C12712923542` | Explicit `-ProgrammerCli`; even a read-only preflight touches ST-LINK/target and requires separate hardware permission |

Two other tracked PowerShell dependencies, `scripts/create_firmware_update_package.ps1` and `scripts/output_directory_safety.ps1`, are **outside the seven default-path changes**; preserve their behavior, input contracts and output safety. All nine PowerShell files are currently unchanged.

## Required behavior-preserving resolver boundary before code authorization

1. Preserve explicit command-line override precedence (`ToolchainRoot` or `ProgrammerCli` as appropriate). Any future declared environment/config fallback must be explicit, validated and deterministic; path discovery must fail closed on missing or ambiguous installations. Do not log host-local absolute paths into public manifests, committed logs or emitted wrappers.
2. Test Windows PowerShell 7 parsing and invocation from a *disposable* copy, including quoted paths with spaces, missing/ambiguous installations, command-line override precedence and negative-path exit semantics. No Windows execution proof has been obtained in this audit.
3. Prove deterministic ARM firmware/bootloader builds against accepted candidate/source identity and expected byte/hash equivalence where required. Use **synthetic/nondeployable keys only**, keep production key files and signed deployment payloads out of public archives and CI.
4. Preserve wrapper inheritance of the chosen programmer path in both PRESERVE and CLEAN modes, plus VALIDATE_ONLY behavior, stable bundled file list and manifest/hash policy. If wrapper bytes change, require explicit new recovery-bundle candidate proof rather than claiming equivalence.
5. Preserve the `Reset-DeusGeneratedOutputDirectory` anti-clobber owner/marker policy, transport/persistence and Flash geometry, exact tool versions, readback assertions, the accepted SWD fallback `950/480/240/125 kHz`, and no mass erase/read-unprotect/option-byte modification.
6. First run isolated mock/offline negative/positive tests with **no physical STM32 access**. A separately approved Windows/target Gate must authorize real ARM build/recovery, ST-LINK/Flash/reset/USB/UART activity; ordinary Host CI Core `84/84` + Transport `24/24` does not prove these PowerShell contracts.

## Outstanding approvals / Gate interpretation

- **OPEN** — owner's per-category public/private publication disposition for the 49 PRIVATE + 49 RETAIN original rows and the 15 REDACT items. A docs-only cleanup directive does not approve script byte modifications or historic/visibility operations.
- **OPEN** — independently verifiable private restore and anonymous denial, plus exhaustive accessible external GitHub/clone/cache/artifact surface review (enumerate unavailable surfaces as NOT_CHECKED).
- **OPEN** — coordinated seven-script Windows offline parity acceptance and any separate hardware gate. These seven executable files must not be rewritten or staged merely to remove a recognizable local path.
- **OPEN** — explicit approval of future-only publication versus history rewrite/new repository/visibility change. No `push --force`, history rewrite, source removal or automatic Gate-2 promotion.

Result: **Gate-1B technical script/default-path inventory is scoped and source-fingerprinted; implementation authorization is not established. Gate-1 and Gate-1B remain OPEN, Gate-2 BLOCKED.** This record supplements rather than replaces the canonical 113-row decision matrix.
