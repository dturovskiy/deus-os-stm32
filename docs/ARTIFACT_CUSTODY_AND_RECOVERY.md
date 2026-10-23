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

### 2026-10-23 — Deterministic historical rev5 source and rebuild route

**Native harness correction:** On Windows 23:22, the v1 preparation tool failed `MISSING_TOOL_REV5_BUILD` **after** proving exact source Git tree, `git archive`, `tar`, and real Arm GNU GCC 15.3.1. It had checked an unusable child PowerShell executable string via `[IO.File]::Exists`, not a missing compiler. Original evidence SHA-256 `08DF9C834A46A170EC50A71621B55205DCA31EAAACF6C2E32C2275151421845F`, 7/7 indexed SHA PASS. Repaired v2 bundle SHA-256 `5EC5774CD3B4EDFD07A592D3209856264F1A68D82EF7EA6A19EA5FC9F7026082` uses running `$PSHOME\pwsh.exe`, performs a real noninteractive child PowerShell probe, and records no private absolute path. Source rebuild is **still pending native execution**, as is production signer/custody validation. No reason to search for/install an Arm compiler based on this error alone.


**Proven Git identity:** published RDC-06 commit `7bc5f27ce3447f0c907d2643e7db67669371210a` resolves via live Git `rev-parse` to the exact accepted rev5 tree `ccbd50200ec7f0ec53da6b1048d7377271dfb7bd`. Historical application image was **52,012 B**, SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`. The published Git commit recovers original **source**, not an already built or signed recovery artifact.

**Offline Windows operator check:** bundle ZIP SHA-256 `E57F9F82D83F1D599E92EEA667E54796B5F783094134C07321E6C756CAD3FA67`, 10,221 bytes, original package ID `stm32_os_mgmt_000e_gate3_recovery_rebuild_signer_qualification_v1_20261023` archives only the exact old commit into a disposable TEMP root, compiles with that archived commit's canonical ARM build script and Arm GCC 15.3.1, and requires the historical BIN size+SHA. Separately, the current canonical signing script processes an **inert 16-byte dummy image with a completely synthetic non-production key**, and the wrapper independently checks its HMAC/header/digest. The real DPAPI signer is only checked for file presence; no actual device key is accessed or decrypted. All temporary BIN, synthetic key and dummy `.pkg` are deleted; no firmware images are exported or retained. Only operator-local evidence metadata is produced.

**Qualification boundary:** source/tree confirmation is accepted, but Windows native rebuild, actual production signer operability, protected BIN custody, independently restorable floor-compatible signed rescue, live bootloader floor and Gate 3 remain **UNVERIFIED** until their own original evidence is accepted. The package is an offline qualification, not permission to sign, publish raw firmware, reset/flash or run Gate 4.

### 2026-10-23 23:38 +02:00 — Successful original Windows exact rebuild; custody remains separate

The original Windows V2 evidence ZIP SHA-256 `24C1245584A4E1DFDED88446BC686E12BD8FE9A6F632C0E9444055672528D7FD` passes ZIP CRC and **13/13** indexed SHA-256. Exact source Git archive plus Arm GCC **15.3.1** rebuilt historical `000C` application **52,012 B / SHA-256 `AA4F83C1F33858D6ADF381DDF3CE5B3EC8A5A7C72B6C254EEDC4D8518B7F6210`** in isolated Windows TEMP; 32/32 C compiles and link/objcopy PASS. Current package-builder synthetic test with nonproduction 32-byte key and 16-byte inert input independently checked v1 HMAC/header; real signer/vault decrypt remained **NOT_TESTED**. Temporary actual application BIN was removed by design, and no BIN or key is inside evidence. This is an **accepted reproducibility proof**, not verified protected off-Git custody or a signed recovery capability.

**Mandatory next durable step:** in a separately approved no-device, no-production-key Windows operator custody workflow, reproduce the same bin and store it **outside project/Tools/Downloads/Git** in an explicitly owner-controlled restricted Windows folder. Verify the destination file's SHA/size by reopening it **after** the write; reject existing nonmatching files, reparse points, weak/unverifiable ACLs, and partial/failed deposits; record only a stable locator alias + relative name and proof digest in private metadata, never the raw BIN/secret in Git or evidence. Keep a second backup and restoration capability separately unverified until proved. After custody, complete real signer, authenticated live floor and monotonic-version signed rescue acceptance through distinct gates; do not present successful compiler or synthetic HMAC tests as recovery-ready.

## 5. Actual open acceptance predicates

- Original rev5 payload bytes (or reproducible deterministic equivalent) are **NOT YET PROVEN AVAILABLE** in protected custody.
- A verified floor-compatible, signed **post-000E rescue package**, access controls, copy and recovery path are **NOT PRODUCED / NOT ACCEPTED**.
- Present live authenticated bootloader floor, target power/isolation and complete Gate-3 readiness remain **NOT ACCEPTED**.
- Gate 2 remains **14/14 OFFLINE ACCEPTED**; Gate 3 **NOT_ACCEPTED**; Gate 4 **NOT_AUTHORIZED**. The inventory never modifies these statuses.
