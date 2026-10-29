# DEUS Gate 3 — permanent signed-operator audit module

This is a **persistent project component** (not a one-off Bundle and not the runner).
It is installed at `OS/scripts/gate3_operator/` and is intended to be reused
for every Gate 3 physical operator candidate by passing the latest **original**
owner-consent evidence and exact whole SHA.

## One stable entrypoint

From the OS Git repository root:

```text
python scripts/gate3_operator/gate3_auditor.py
```

On the DEUS MCP host, the same file is invoked using the existing
`run_script_start` Python runtime. Its default immutable original-evidence
SHA is read from adjacent version-controlled `policy.json`; for a new
accepted owner consent, update only that policy and pass the new exact
original Evidence path. The Python implementation stays unchanged.
It does not require an extra `deus-run` bundle to be copied to Downloads. Python 3.10+ and the `cryptography` package
are required; if the cryptographic verifier is missing, the check fails closed.

For a new, separately native-accepted owner consent, pass:

`--owner-evidence /absolute/path/to/ORIGINAL.evidence.zip --owner-evidence-sha256 EXACT_WHOLE_SHA256`

If the signed plan or stage changes, also pass `--source-lock` and `--stage`
for the exact plan whose hash is bound into that signature. Key rotation requires
a separately accepted public-trust anchor update; never trust a key supplied
only by the same unverified consent.

When a future actual V2 candidate operator ZIP is available, add:

`--candidate-zip /absolute/path/to/EXACT_OPERATOR.zip --candidate-sha256 EXACT_ZIP_SHA256`

That additional phase runs canonical `bundle_guard.validate_file`, verifies
the whole supplied ZIP SHA, exact package ID, root member set, entrypoint, and
payload bytes against the owner's signed 14-file source-lock. A candidate must
have only one entrypoint and exactly the signed payload. This is **static
candidate inspection**, NOT permission to release or execute.

## Invariants (not optional)

- Owner trust anchor is pinned to the previously independently accepted
  Ed25519 owner key; the code verifies real binary OpenSSH SSHSIG under
  `deus-os-gate3-one-reset-v1`, plus tamper/namespace negatives.
- Only the complete original hashed evidence ZIP is accepted: whole SHA,
  member CRC, internal SHA-index and no missing/duplicate files.
- Exact 14 signed stage file names, sizes and SHA-256 are checked. The
  original source remains **QUARANTINED_NOT_FOR_RUN**.
- Fixed host `macmini`, Linux account `deus`, port `usb:001:8`, one
  `1209:000C → 1209:000D` reset, one INFO, precisely one new USB-node
  `deus:0600` privilege action, no Flash/erase/retry/auto-return.
- Every consent is time-bound and nonce-bound. A persistent tool **cannot**
  make an expired one-time owner signature permanently valid.
- The module has no SSH, sudo, USB, device or hardware mutation functions,
  does not issue a physical command, and does not create operator ZIPs.
  A PASS here states only
  `OWNER_SIGNED_SOURCE_STATIC_ACCEPTED_NOT_HARDWARE_RELEASE`.
- The project's `scripts/operator_bundle_guard/bundle_guard.py` remains the
  only canonical V2 ZIP builder. The generic HARDWARE_MUTATING `release`
  remains **DENIED**. Independent native Windows AST / runner receipt,
  physical-host proof and a distinct owner-authorized release lane are
  necessary before `deus-run` may be offered for real reset.

## Offline remote-result safety guard (2026-10-28)

`remote_result_guard.py` is a **pure in-memory** policy module intended for a future **separately approved** Windows result owner. It is **not connected** to the original signed `apply.ps1`, not a `deus-run` operator, and cannot start SSH, USB, sudo, reset or Flash.

It distinguishes `STOP_BEFORE_RESET` when the remote execution boundary is genuinely not entered, `STOP_AFTER_ONE_ENTER_NO_RETRY` on a confirmed post-ENTER failure, and **`RESET_POSSIBLY_ATTEMPTED`** when SSH/result transport or the original receipt is absent, malformed, inconsistent or ambiguous. A syntactically positive remote receipt must carry the old installed rev5 runtime source-tree pin, a unique and ordered USB detach/new 000D attach, owner-only device permissions, unique exact ENTER/INFO tokens, and a bounded firmware version floor. Even if the receipt is accepted **within this narrow scope**, the policy always returns `retry_authorized=False`, `flash_authorized=False`, and `full_gate3_accepted=False`. External evidence provenance and signature remain independent.

Offline test (not a hardware test):

```text
python scripts/gate3_operator/test_remote_result_guard.py
```

Latest genuine DEUS MCP Python execution (2026-10-28): **59/59 PASS**, zero hardware accesses. Source SHA-256 `D2E047BEC44F580946248A0A6A8330F85E17AD1D78E894D69D5CDA7A9C4A54CD`; offline tests SHA-256 `80EB7ACB9D583F36FF306737D73B19F9E17864BF6559D2F46784B7A53BE88F80`. This supersedes the previous 47/47 test count, without superseding the signed original physical runner. A remote `STOP_BEFORE_RESET` claim after SSH child launch is now treated as **`RESET_POSSIBLY_ATTEMPTED`**; only a locally established non-start can be classified `STOP_BEFORE_RESET`. The `ENTER` claim must include exactly the five expected fields and a firmware-update capability bit (`SystemCapability.FirmwareUpdate = 1u << 6`); INFO must have precisely six expected fields, without duplicates or extras. Nonfinite sysfs event timestamps are rejected. The signed physical runner is unchanged; its early PowerShell error handling and the missing actual result-guard wiring are **still blocking** and require a separately accepted, freshly signed source candidate, real Windows failure-path evidence and an authorized hardware execution lane. Do not turn this module's mock PASS into physical authorization.

## Windows result-owner source-only qualification candidate (2026-10-28)

`windows_result_owner_offline.psm1` implements a fail-only Windows PowerShell 7 result/evidence writer with all fallible workspace and evidence-root setup inside `try/catch/finally`. It **has no live device, SSH, sudo, signing, reset or firmware call**. `test_windows_result_owner_offline.ps1` runs 10 purely synthetic filesystem/error paths including preinit failure, child-not-started, SSH timeout, missing/invalid result, post-ENTER failure, evidence seal failure, invalid repository, missing Evidence directory and duplicate result rejection. The test creates and deletes a uniquely named TEMP workspace; it **does not** write a physical/evidence package to the owner's real Evidence or flash the STM32. It checks FAIL-evidence ZIP SHA-index/reopen and fail-closed result semantics when evidence cannot be sealed.

Owner Windows PowerShell 7 **native AST and 10/10 targeted offline failure tests PASS** on 2026-10-28. Exact source pins: `.psm1` SHA-256 `65495FEDB861874CA06E6CAE69C093C05EBADE9E6DC9CFF36A763BF2E80832C9`; test `.ps1` SHA-256 `870304DC8DE8994BC413798C3234E27E88A9C6CE1ABDE4A50B6BF38E59D7FEAE`. Owner console showed `SOURCE_AND_NATIVE_AST=PASS`, ten individual `G3_WINDOWS_OWNER_CASE=PASS`, final `GATE3_WINDOWS_RESULT_OWNER_OFFLINE=PASS COUNT=10 USB=NO SSH=NO RESET=NO FLASH=NO` and `WINDOWS_RESULT_OWNER_OFFLINE=PASS`. This is terminal-only native qualification of the isolated offline component, **not** an original sealed Windows Evidence ZIP, a real remote-child failure simulation, or a native physical runner receipt. Do not repeat this suite unless its sources change. The original signed `apply.ps1` and its 14-file source lock remain unchanged; this offline module is not wired into them. It cannot output physical PASS or authorize a second reset.

From the canonical OS root on Windows PowerShell 7, after an independent AST parse:

```powershell
& .\scripts\gate3_operator\test_windows_result_owner_offline.ps1
```

No `deus-run` operator ZIP or hardware command is created by that line. Final operator acceptance still requires integrating a re-reviewed Windows result-owner with exact C# runtime pin, fresh signed source-lock, native evidence on PASS/FAIL and distinct release admission.

## Test and maintenance

```text
python scripts/gate3_operator/test_gate3_auditor.py
python scripts/operator_bundle_guard/test_bundle_guard.py
```

The first suite covers actual original signature success, signature, payload,
trust, consent, expiration, ZIP/index, quarantine and candidate-negative
cases. It uses temporary paths for tamper cases; no STM32 operations.

This directory belongs to **OS Git** (the durable source of truth), not
`Bundle` (ephemeral generated operator inputs) or `Evidence` (immutable
native receipts). It was created in the existing working tree and is
**not automatically committed or pushed**.

Current scoped baseline: original owner reset consent expiry
`2026-10-28T22:45:18Z`; `FULL_GATE3=NOT_ACCEPTED`,
`GATE4=NOT_AUTHORIZED`.
