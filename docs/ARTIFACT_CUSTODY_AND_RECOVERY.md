# DEUS OS — Artifact Custody and Recovery Location Contract

**Scope:** Operational artifact lifecycle, **not** a second project-state authority. Effective: 2026-10-23. The sole project-state authority remains `docs/CURRENT_STATE.md`; signing, update ABI, and Gate authority remain in the respective protocol/acceptance plans. The operator ZIP/evidence rules remain in `docs/HARNESS_EVIDENCE_RECOVERY_PLAYBOOK.md`.

## 1. Where each class lives (one role per source)

| Artifact class | Authoritative custody | Proof / disclosure |
|---|---|---|
| Source and build inputs | Public `deus-os-stm32` Git + accepted exact full source tree | Git tree and original build evidence; **not** a recoverable signed image |
| Sanitized runnable `deus-run` ZIP | Private `deus-os-stm32-private-docs/operator_artifacts/` | ZIP SHA/size/manifest **and independently verified remote-byte readback**, never just a link/import intention |
| Recovery package **file-identity inventory** | Private `operator_artifacts/RECOVERY_CUSTODY_LEDGER_V1.json` | File ID, expected size/SHA, last confirmed custody status and locator *alias*; no secrets or user paths |
| Signed app `.pkg`, candidate/rescue `.bin`, recovery image | Protected, owner-controlled **off-Git** store | Exact bytes, SHA/length, signing and usable recovery receipts; private Git **not** a firmware vault |
| Device signer (DPAPI) / encrypted escrow | Owner Windows protected store + independently authorized secure backup | Presence/custodian-only metadata in docs; do **not** inspect/decrypt/export with an inventory tool |
| Original `.evidence.zip`, native logs | Windows operator `Downloads` | Original SHA/CRC/hash-index, local retention; only concise metadata in Git |
| Actual Gate/source/installation state | `docs/CURRENT_STATE.md` | Accepted source-and-hardware evidence, not artifact ledger fields |

This is **not** an instruction to write signed firmware into any Git repository. A prior rejected import due to file-mode `0600` is a blocker, **not** proof of private remote custody. Old historical receipts and their FAIL states must not be altered.

## 2. Machine-readable exact locator (no more blanket searches)

Read-only implementation: `scripts/artifact_ledger.py`; negative/unit tests: `scripts/test_artifact_ledger.py`. It consumes `DEUS_OS_RECOVERY_ARTIFACT_REGISTRY_V1` from the private artifact repository and optional explicit aliases such as `WIN_DOWNLOADS=<actual absolute Downloads folder>`.

It checks **only pre-registered relative `.pkg`, `.bin`, or `.zip` filenames** under the authorized supplied alias. It does **not** recursively walk other disks, follow symlinks, open network/hardware interfaces, read key/escrow/DPAPI content, write files, sign, flash or reset. It reports `EXACT_MATCH`, `NOT_VERIFIED`, `MISMATCH` with per-locator SHA/size statuses. Unknown location remains unknown. Non-verified or missing entries never become `RECOVERY_READY`.

Example for a **developer's local read-only shell** (not a new `deus-run` hardware bundle):

```text
python scripts/artifact_ledger.py --registry ../private_docs_archive/operator_artifacts/RECOVERY_CUSTODY_LEDGER_V1.json --root WIN_DOWNLOADS=<ABSOLUTE_AUTHORIZED_DOWNLOADS_DIRECTORY>
```

Git only stores alias IDs and relative filenames; actual Windows profile names, credentials, passwords, vault paths and protected storage roots remain owner-local. Source/digest without an actual verified stored file is a historical reference **only**.

## 3. Mandatory lifecycle and future automation

For *every new* signed/backup/recovery artifact, the reviewed build/signing/transfer process must eventually enforce all six obligations **in order**:

1. Build from one exact accepted source tree. Record source tree, original firmware BIN SHA/size, target, linked origin and intended revision.
2. Sign in the separately authorized, protected signing domain. Require current authenticated live floor; record candidate package SHA/size plus trustworthy signer/HMAC receipt. Digest or nonzero HMAC-looking bytes alone do not authenticate the package.
3. Store in a stable access-controlled **off-Git** location, record a locator alias and exact filename, independently re-open the **stored destination** and compare the full SHA/size. Do not rely solely on an in-memory build or staging SHA.
4. Preserve a separately controlled second copy or explicitly mark `NOT_ATTESTED`. **Never delete the only verified copy**. An encrypted key escrow is **not** a backup of the firmware package or full recovery procedure.
5. Record custodian role/date, proof reference, independent restore check and an exact compatible recovery route. A stored SHA match is **not** equivalent to signed legitimacy, live version-floor suitability or ability to recover a bricked device.
6. Before any future Gate-4 installation, prove the *post-installation* recovery capability against the **expected future revision floor**, record the non-destructive preflight, and require separate explicit owner GO.

**Current automation coverage:** deterministic exact-file verification and negative tests are implemented. **Not implemented:** automatic call from firmware builder/signer, protected-store creation, remote backup/restore, new signing, native Windows deployment, or firmware installation. Integration into the signing workflow requires its own reviewed and owner-authorized tooling gate: never silently weaken source scope or let a document checkmark authorize key handling.

## 4. Recorded historical recovery identities and consequences

Historical exact signed rev5 `.pkg`: SHA-256 `61AE3D54838356A79916EFEFF715B8E16451ECC6482AB8DADC8F7378B5503D8C`, **52,092 bytes**; accepted embedded application BIN: SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`, **52,012 bytes**, source tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. Historical primary location instruction was Windows Downloads (including browser `(1)` duplicate name). Source: private `Tools/DEUS_OS_RDC06_STAGING_V3_REVALIDATION_20261009.md`.

The 2026-10-23 bounded Windows search reached four active roots, 182 directories, 1,724 files, 249 inspected ZIP candidates; it found **no** matching `.pkg` or old operator ZIP, with 4 policy-skipped files. **Not found in bounded search != lost from every possible store**. There is **no** attested current destination for these bytes, so the ledger intentionally marks them unlocated. STOP repeated unfocused hunts or attempts to restore already-discarded historical ZIPs.

Bootloader v1 protocol §9 requires signed version **greater than floor** for a normal update with a valid app; only recovery without a valid committed app permits **version >= floor**. Old signed v5 is not a universal fallback after management-only `000E` increases authenticated floor. A **hypothetical** future 000E version 6 would require a separately signed, compatible legacy 000C-content package **at least version 7** for ordinary rollback. These numbers are an example, not an authorized revision assignment or permission to sign/reflash. A reconstructed source/old BIN SHA alone does not prove bootability, package authentication or a usable recovery channel.

## 5. Actual open acceptance predicates

- Original rev5 payload bytes (or reproducible deterministic equivalent) are **NOT YET PROVEN AVAILABLE** in protected custody.
- A verified floor-compatible, signed **post-000E rescue package**, access controls, copy and recovery path are **NOT PRODUCED / NOT ACCEPTED**.
- Present live authenticated bootloader floor, target power/isolation and complete Gate-3 readiness remain **NOT ACCEPTED**.
- Gate 2 remains **14/14 OFFLINE ACCEPTED**; Gate 3 **NOT_ACCEPTED**; Gate 4 **NOT_AUTHORIZED**. The inventory never modifies these statuses.
