# G3-4b — source-bound non-destructive recovery readiness PRE-ADJUDICATION
Date: 2026-10-28. **Result: PASS_SOURCE_ONLY; FULL_GATE3=NOT_ACCEPTED.**
Instrument: `scripts/gate3_operator/g3_4b_readiness_source_proof.py`; source SHA256 `D4C1A3B43210CD0C1C236D5ACF0CA98953CFE57214B1EB556ABCCB59491ABEE0`, 20,593 bytes.

## Verified in live DEUS MCP today

- Managed Python `--selftest`: **39/39 PASS**, process exit **0**, with `USB=NO SSH=NO RESET=NO FLASH=NO`, including six single-source tamper rejects, wrong Git tree, oracle drift, signed SHA/receipt boundary tamper, malformed image lengths, and negative/positive state-machine cases for both recovery branches.
- Managed Python read-only `inspect`: **35/35** strict source/accepted-record consistency checks, process exit **0**. The script rehashes and enforces exact bytes of **six** firmware/Host/transport/protocol source files and **four** original accepted records/receipts. No files written by the script.
- The accepted previous native Windows evidence remains **historical original acceptance**, not bytes re-read by this Linux tool: signed `000E` v6 49,072 B SHA256 `2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED` and signed protected rescue v7 52,092 B SHA256 `C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A`; earlier Windows native exact signed-pair proof eight affirmative checks plus two explicit NOT_ATTESTED boundaries. The accepted closed-world source-tree reconstruction `081aac70c31909bf681254393e2288917cb9b8b2` and the 31/31 source oracle are SHA-verified.
- Source checks bind actual `bootloader.c` (INFO read-only flags 0, metadata HMAC before version-floor decision, no-valid-app recovery branch, destructive header authorization), `FirmwareUpdateClient.cs` (signed header/payload SHA, Info vs Update split, exact page-safe transfer and limited DATA retry), `DeusDeviceClient.cs` (runtime capability gate), Linux 000D IF0 EP OUT01/IN81, Windows WinUSB GUID caveat, and protocol no persistent partial resume.
- Future normal v6 plan **1,052** even DATA chunks over **48** 1-KiB application pages; rescue v7 plan **1,117** even chunks over **51** pages; `max DATA bytes=48`; no chunk crosses a 1-KiB page. Both package lengths = 80-byte signed header/tag + image.
- The source-only route model explicitly covers (A) hypothetical **valid committed v6 / floor6**: 000E runtime → separately authorized ENTER → 000D → non-destructive INFO → v7 stricter-than-floor eligibility, and (B) **no valid application / committed0 / floor7**: owner-authorized reset → boot_main remains in 000D → INFO → v7 equal-to-floor permitted. Both STOP on wrong USB host, PID, source tree, rescue SHA, source-only physical GO request, missing/contradictory floor or mode, and reject implicit runtime entry from the no-app case. These values are **hypothetical future test states**, not observed device metadata.
- After a *hypothetical* successful v7 restore, any future new normal update needs version **>=8**, not the old signed v6. No actual update, HMAC signing, key operation, SSH, 000D transaction, reset, erase or Flash was performed.

## What this does NOT prove

1. **This new script is not a physically executable recovery operator, nor a separately accepted original Windows-native evidence ZIP** for an integrated owner-controlled readiness route. Its output is an offline machine-readable source pre-adjudication, not final G3-4b attestation. Reusing older signed-pair original evidence establishes the package/source subproofs only.
2. The exact future 000E source tree and v6 signed artifact are previously attested, but **000E is not installed**. There is no real observed 000E→000D USB handoff, and the no-app branch is hypothetical; this is **intentional** under the Gate-3 non-destructive scope.
3. INFO reports a bootloader-computed HMAC-derived floor but is **not** cryptographic end-to-end host/USB device identity authentication. Windows interface GUID alone cannot prove physical ownership. Source pins/Hub ownership/STOP are mandatory at a future permitted session.
4. No new native Windows execution, retained original `Evidence/*.evidence.zip`, independent acceptance/signature or full `deus-run` operator result was obtained by this attempt. Do not invent one or mark the Gate3 checkbox.
5. The earlier release-controller plan RA-01..08 is **separate optional physical-run governance**, not a new mandatory prerequisite to finish the original non-destructive G3-4b.

## One remaining precise acceptance step

An independently reviewed **single source-bound recovery-readiness owner procedure/attestation**, using both **already accepted** real v6 and v7 signed file identities and both route models, must be natively qualified and produce **original indexed PASS/FAIL Windows evidence**, with clean source pre/post and true STOP/ownership. No device IO or reset is needed for this *Gate-3 readiness attestation*.

The source-only verifier here is a valid component to review/reuse without repeating unchanged 168 Host tests, 31 source-oracle tests, Ed25519 signer proof or Windows v6/v7 signing. **Do not represent its 39 tests as the native independent acceptance still required**. A future operator may not be substituted with an unadmitted physical executor. Candidate authorship and execution must follow existing owner/bundle/evidence restrictions, including prior client-layer blocked runnable generator calls.

**Formal disposition:** `G3_4B_SOURCE_RECONCILIATION=PASS_35_OF_35`; `G3_4B_ROUTE_POLICY_TESTS=PASS_39_OF_39`; `G3_4B_INDEPENDENT_NATIVE_INTEGRATED_ATTESTATION=PENDING`; `FULL_GATE3=NOT_ACCEPTED`; `GATE4=NOT_AUTHORIZED`.
