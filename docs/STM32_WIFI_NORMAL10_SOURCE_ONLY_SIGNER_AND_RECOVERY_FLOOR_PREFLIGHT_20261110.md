# STM32 Wi-Fi normal10 — public signer / recovery-floor preflight

**2026-11-10, after 23:50 original ARM stack-floor PASS. Scope: SOURCE_ONLY, no signing, no protected package/key reads, no device actions.** Authority: docs/CURRENT_STATE.md. Status: NORMAL10_PUBLIC_VERSION_FLOOR_FEASIBLE_CONDITIONALLY_RECOVERY_UNREADY.

## Frozen inputs

- Original unsigned ARM V2 os.bin: 52,052 B, SHA256 6F7378619B7631114926FDFC31B881B06CA49A2660FA2D79E563F4D506E24B73. Native original Windows compile and later existing-output read-only audit PASSED; current source-index SHA prefix160 d892610a4b842fe84b17c68da2546bd29c5698b6 is **not** a signed release commit or an image signature.
- Actual public bootloader source OS/bootloader/bootloader.c was read again, full SHA256 A7A52D4962F48D7C95506FB97613BC6A5F7DA661BC07CF3CE850D86247EE25E3, equal to the previously GCC O0/O2/Os/UBSan 98-case-tested public source from 2026-11-08. The test was **not** executed anew for this preflight.
- In public bootloader.c handle_authorize, candidate is read from header+12. When committed_version != 0, candidate <= version_floor is rejected. When committed_version == 0, candidate < version_floor is rejected. Commit raises floor to max(existing,candidate) and sets committed_version=candidate. These source predicates alone do not authenticate HMAC or attest physical current bootloader INFO.
- Firmware protocol v1 public header: 48 B, HMAC tag 32 B, image length 8..53248 B and multiple of four, version nonzero; product 0x534F4544, target 0x0410, application origin 0x08002000. The exact unsigned 52,052 B ARM image passes length/alignment, and a **hypothetical** correctly signed version10 package would be 52,132 B (48+32+52052). **No signed normal10 package was generated** and these numeric checks are not key continuity proof.
- The real Mac one-shot normal9 update reported UPDATE_COMMITTED=Ok/Committed VERSION=9 on 2026-11-10. Current live bootloader INFO floor9/committed9 was **not** freshly reread here. The protected normal9 package (49,100 B, SHA DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B) and protected rescue9 package (52,092 B, SHA 4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928) had prior accepted owner Windows signer evidence and local same-store backups. **Neither package nor protected escrow/key was opened now**. Their one-shot v9 signer is archived/consumed; do not rerun it or relabel as a normal10 signer.

## Public version-floor state matrix (source rules only)

| Physical state ASSUMED for predicate evaluation | Candidate version | Public admission only |
| --- | ---: | --- |
| Valid normal9; floor9 | normal9 = 9 | REJECT: candidate <= floor |
| Valid normal9; floor9 | normal10 = 10 | VERSION-ELIGIBLE: 10 > 9 |
| Invalid application; floor9 | rescue9 = 9 | VERSION-ELIGIBLE: 9 >= 9 |
| Valid committed rescue9; floor9 | normal9 = 9 | REJECT: equal floor |
| Valid committed rescue9; floor9 | normal10 = 10 | VERSION-ELIGIBLE: 10 > 9 |
| Valid normal10; floor10 | old rescue9 = 9 | REJECT: below floor |
| Invalid application; floor10 | old rescue9 = 9 | REJECT: below floor |
| Invalid application; floor10 | prospective signed rescue10 = 10 | VERSION-ELIGIBLE: equal floor permitted only when invalid |
| Valid committed rescue10; floor10 | normal10 = 10 | REJECT: equal floor with valid app |
| Valid committed rescue10; floor10 | prospective normal11 = 11 | VERSION-ELIGIBLE: higher than floor |

**Critical recovery consequence:** It is unsafe to infer that prior signed rescue9 will remain usable after a normal10 commit raises floor10. At floor10 with an invalid app, rescue9 is rejected by version policy. A separately verified rescue10 (or explicitly approved equivalent compatible strategy), plus a normal11-or-newer post-rescue10 return path, must be planned before any physical floor10 change. The matrix does not attest signing, host admission, rescue runtime, actual current INFO, protected package restore, or any authorization to flash.

## Gate state

| Requirement | Current scoped status |
| --- | --- |
| Unsigned ARM V2 binary, linker origin, Flash/SRAM limit | Native Windows PASS, source-only |
| Original ELF/MAP and 38 GCC .su SHA/frame validation | Native Windows read-only PASS |
| Complete nested console 1024 B / heartbeat 512 B stack call graph and IRQ bound | NOT_ATTESTED; max 240 B per-function does not establish it |
| Release-source identity: Git tree, 84-file SHA manifest, exact unsigned BIN binding | SOURCE-INDEX ACCEPTED; publishable signed release identity NOT_FROZEN |
| New normal10 same-key HMAC signer / native owner-only protected ACL | NOT_AUTHORIZED / NOT_ATTESTED |
| Future-floor10-compatible rescue and >10 normal return route | NOT_PREPARED |
| Fresh hardware bootloader INFO / new update consent | NOT_RUN / NOT_AUTHORIZED |
| Actual normal10 signing, USB/CH340, Flash, RESET, ESP radio, Gate4 | NOT_AUTHORIZED |

**Next safe task:** analyze nested task0/heartbeat call paths with ISR preemption and scheduler context against 1024/512 B PSP reserves, and specify a separate owner-approved normal10/rescue10/normal11 signing+recovery sequence without reading protected key material into logs, Bundle or Git. Keep existing signed normal9 and rescue9 custody untouched; do not invent a mandatory second disk for this one-board BENCH.

ESP01-010 PARTIAL. Gate4 NOT_AUTHORIZED. No staging/commit/push or physical device action in this preflight.
