# Deus OS — P3-A original Mac mini: one owner-authorized ENTER/RESET + live bootloader INFO8/8

**Date:** 2026-11-10 00:16–00:20 Europe/Paris. **Scope:** Owner explicitly replied `так` to the request for **exactly one physical STM32 v8 ENTER/RESET and subsequently read-only bootloader INFO on Mac mini, no Flash, no CH340**. This permission **does not** authorize P3-B update, additional RESET/ENTER, recovery firmware, CH340, ESP power/EN/BOOT, signing, git stage/commit/push or Gate4.

**Result:** `P3_A_MAC_NATIVE_ONE_ENTER_RESET=PASS`; `P3_A_MAC_NATIVE_BOOT_INFO_8_8=PASS`; `P3_A_ORIGINAL_MAC_EVIDENCE_12_12_SHA=PASS`; `P3_B_FLASH=NOT_STARTED_NOT_AUTHORIZED`; `PHYSICAL_ACTIVE_MODE=BOOTLOADER_1209_000D_RECOVERYIDLE`; `INSTALLED_SIGNED_NORMAL_V8_PAYLOAD_UNCHANGED` (no erase or update commands were sent), `ESP_APP_V2_UNTOUCHED`.

## 1. Real Mac worker: exactly four original jobs

Actual device is the Mac mini (Ubuntu) under the unprivileged `deus` worker `deus-command-worker-v2`, real sysfs USB physical path **`/sys/bus/usb/devices/1-8`**, logical port **`usb:001:8`**. All commands ran in the *isolated, source-locked candidate* with native Mac .NET 10 `deus-cp.dll`, **65024B SHA256 `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`**, with no deployment/replacement of accepted original signed-v8 host binary. No scripts opened the other USB CH340 at `1-7`.

| Phase | Unique genuine Mac job (in `DEUS-MCP/macmini-workspace/automation/`) | Original queue SHA256 | Original typed receipt SHA256 | Original full log SHA256 | Genuine result |
|---|---|---|---|---|---|
| Read-only preflight 1 | `esp010-v9-p3a-preflight-1110-a1` | `774F4008268E02F93C938EF43A667A04DBE5AC21D954FB5DC7B2D3FBD35228BC` | `773E85A8419B1CE3A659D22C98C14D4388D22B293DC8083ACF0013CA0C38597F` | `5B605C717862721CF798D4C3A179C6BDBA9A93E54857A599254ED7F3160B296F` | **REAL FAIL exit1**, Bash text assertion only; runtime v8/USB/health all output PASS; **no RESET** |
| Read-only preflight 2 | `esp010-v9-p3a-preflight-v2-1110-a1` | `BF4F357A9D5025CDE7993EF53BF7D854B81B5ADD7A7938BF2E4EEF3E5556D5BD` | `73944997257388B01156BD7D85E947157451C973330C9767D9495A60F0173C4E` | `13CEE2EA6BB00E1DB5D8B0BBD6A5FB912EA584B8B2584FA0EA35C17F99E32F24` | **PASS exit0**, explicit runtime healthy, still **no RESET** |
| ONE owner-authorized physical ENTER | `esp010-v9-p3a-enter-once-1110-a1` | `887A4A7855F5D186F80F89D34CF6675E6CE98A3BD70E9E9C2F0BBF41B896C625` | `DC901FC9329F6B3EC5AEB44BF2CA7244DE5F9F89E14BA03D5A222BF5448B9BA3` | `36C3925B212374EA1A78AF897B2B3782AECC9B3486119F4E1D6592100DFB3D5F` | **PASS exit0**, exactly one `ENTER=Ok/Resetting`; one-shot marker created |
| Read-only bootloader INFO | `esp010-v9-p3a-boot-info-1110-a1` | `679693F222A8EBADB38FA6781EE533126B354BC5DD73C46DEC1599DC47EEEA56` | `7424B777F9915C82E90DFF0D09642EE2EC1CE9E8AA5EAAAEE7AC32959110E83D` | `279277EA4C277F490DA4596F5D2FF71A8218166D14601023A31FCF3D841C3F29` | **PASS exit0**, real bootloader `FLOOR=8 COMMITTED=8 RecoveryIdle` and **FLASH NOT STARTED** |

**Original files are not reconstructed/synthetic.** They remain at original typed worker queue, receipt and full native log paths. Independent reader `Tools/scripts/bench/audit_original_mac_p3a_v9_enter_boot_info_20261110.py`, task **`task_1794266534902_1e9d3d96a6fe5a68` exit0 PASS**, verified original **12/12 SHA**, original request↔receipt binding, worker Mac identity, exact original stdout/log bytes, timeouts/exit codes, the three-pass/one-actual-script-failure classification, source script SHA4/4 and one-shot marker 0600/SHA.

## 2. Pre-RESET original signed-v8 proof

The real exact preflight saw:

- `/sys/bus/usb/devices/1-8` **VID 1209 PID 000E**, USB `usb:001:8`, node `/dev/bus/usb/001/098`, ACL `deus:deus:600`, one runtime, zero bootloader000D.
- `Protocol=1 Service=3 Registry=39`, `SourceTree=2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926`, `PONG`.
- `WDOG_ACTIVE=1`, `IWDG_RESET=0`, `RESET_FLAGS=0x14000000`, `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS`, valid PSP guards/highwater. No CH340 activity. Final preflight `P3A_PREFLIGHT_RUNTIME_V8_HEALTH_AND_IDENTITY=PASS`.

**Historical preflight1 FAIL:** The device emitted some stack report lines with `CRLF`; the first script used `grep -Fxq` against `LF` expected strings and rejected its own text despite real `MSP_STACK_OK`, `SCHED_PROD_OK`, `STACKS_STATUS=PASS` being present. The original FAILED receipt was preserved. The *new v2 preflight script* first removed CR for the text assertions only, kept all original device safety checks, and **PASSED**. This was an actual validation-script bug, **not** device watchdog/stack/USB failure and **not a reset attempt**.

## 3. Exactly one ENTER/RESET; genuine bootloader state

Owner scope was executed via exact C# candidate command `bench-fw-v9 enter-v9 --device usb:001:8 --confirm-enter yes`, only after a **new** runtime sysfs/live signed-v8 Service/Registry/SourceTree recheck. Source-lock and previous successful preflight receipt SHA pinned. The private one-shot marker `DEUS-MCP/macmini-workspace/automation/host-work/esp010-v9-cli-candidate-20261109/.p3a_20261110_once_enter_attempted` is **75B SHA `DE03DF0C51BCDAEDF7CAC7FCE25B6BD2F8F8F122730F85B9D97CF50BF6422228`**, Mac mode **0600**. **Never rerun** consumed `p3a_v9_enter_once_20261110.sh`, whether successful or failed; do not delete the marker.

Real output:
```text
P3A_FRESH_STM32_V8_RUNTIME_IDENTITY=PASS PORT=usb:001:8 NODE=/dev/bus/usb/001/098
P3A_ENTER_ATTEMPT_MARKER_CREATED=YES NO_REPLAY=YES
V8_IDENTITY=PASS SOURCE=2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926 PORT=usb:001:8
ENTER=Ok/Resetting STOP=RECHECK_BOOTLOADER_INFO DO_NOT_AUTO_RETRY=YES
P3A_SINGLE_ENTER_OK_RESETTING=PASS; NEXT_READONLY_BOOTLOADER_INFO_ONLY=YES
```

The *separate*, **read-only** physical job then found real STM32 bootloader `VID=1209 PID=000D` at **same sysfs 1-8 / logical port usb:001:8**, USB node **`/dev/bus/usb/001/100`**, actual Linux node **`root:plugdev:660`**. This node **was accessible without sudo or ACL change**, one bootloader000D, zero runtime000E/000C.

```text
BOOT_INFO=PASS FLOOR=8 COMMITTED=8 STATE=RecoveryIdle
FLASH=NOT_STARTED
P3A_REAL_STM32_BOOTLOADER_INFO_8_8=PASS
P3A_ONE_ENTER_ONLY_NO_UPDATE_NO_FLASH_NO_CH340_NO_ESP_ACTION=YES
```

Important current physical-state statement: **STM32 is presently in 000D bootloader RecoveryIdle, rather than running normal v8 runtime**. The installed signed normal8 image/metadata are still intact with floor8/committed8. No firmware image was transmitted or Flash written. If bootloader later resets/re-enumerates independently, that will require *new* read-only verification, not assuming this 00:20 sample is timeless. No extra RESET/ENTER/reboot was requested or executed.

## 4. Exact executed script custody

Four original private Mac sources (immutable after run):

- `p3a_v9_preflight_ro_20261110.sh` 2457B SHA `BB25A9007B27393D36A4B82A6A681B73623F87AD5C4AC52CB4ECE67ABA77EFD5` (historical script-only CRLF FAIL).
- `p3a_v9_preflight_ro_v2_20261110.sh` 2618B SHA `FBCE47AD3591363F465255EE23ACE5C16319AD54F720A6507AE24BD3DEE4574D` (source-corrected, native PASS).
- `p3a_v9_enter_once_20261110.sh` 2715B SHA `F8BA937C671F03B5E21B0DE69D8E02CF66D6511FBCA801115EE58A954EEE746E` (one owner-authorized physical RESET).
- `p3a_v9_boot_info_readonly_20261110.sh` 2110B SHA `D1B08337088793C96DED66F66BEF3B95928307449B0923EEB428955079566586` (real INFO8/8 read only).

The independent auditor SHA4/4 and source search confirm **no `update-normal-v9`, `recover-rescue-v9`, `sudo`, Flash or other transfer calls** in these scripts. Both source-locked v9 signed package originals and local `ProtectedRecovery/Backups/20261109_signed_v9` remain untouched; legacy accepted host original unchanged.

## 5. Actual next owner decision, no imaginary blocker

**P3-A=ACCEPTED_SCOPED.** Physical bootloader Info 8/8 now establishes eligibility for the *previously signed normal v9* under v1 policy and exact new isolated Host candidate, but **does not authorize flashing**. The necessary separate **P3-B permission** would cover exactly one actual signed normal v9 49100B package SHA `DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B` on the exact Mac USB/000D device with **fresh** INFO8/8 and one unique owner update marker; no automatic resend on timeout. A current recheck must precede any update because port identity/boot state may change. Postcommit real v9 smoke tests remain separate.

Emergency rescue v9 only if *actual* bootloader reports invalid committed app/floor8 or9 and it is independently, separately authorized; signed backup copies already exist under `ProtectedRecovery/Backups/20261109_signed_v9` and are verified. Do **not** invent an off-drive media requirement; do not re-sign, restore v8 by downgrading, or auto-Flash rescue. No normal10 was created or authorized.

**P3-B=NOT_AUTHORIZED**, **P3-R=NOT_AUTHORIZED**, **Gate4=NOT_AUTHORIZED**.
