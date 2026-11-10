# Deus OS — P3-C native Mac postcommit read-only USB discovery: runtime enumerated, USB ACL blocks protocol

**2026-11-10 subsequent resolution note:** This report is the genuine *historical pre-ACL-fix observation*, not current P3-C status. Owner later executed the exact guarded one-USB-node Mac permission script, and a new independent native Mac unprivileged P3-C management read-only smoke accepted `P3_C_REAL_MAC_POSTINSTALL_NORMAL_V9_RUNTIME_SMOKE=PASS_SCOPED` (USB SourceTree/PONG/WDOG/stacks) with original receipt SHA `977CAFAA268AB41C429F9647D9687AC09074AA0F45887F6CCA05D7DD165BCCB3`. Keep original blocker/receipt statements preserved as dated history; current decision is in `docs/STM32_V9_P3C_NATIVE_MAC_POSTACL_RUNTIME_SMOKE_ACCEPTANCE_20261110.md` and `docs/CURRENT_STATE.md`.

**2026-11-10 01:46–01:48 Europe/Paris. Scope:** Owner replied `тоді продовжуємо` after P3-B completion and P3-C read-only offer. Authorized only hardware **READ-ONLY** discovery/protocol smoke, not USB permission mutation, physical RESET/ENTER, Flash/UPDATE, rescue, CH340, ESP action, or Git staging/commit.

**Verdict: `P3_C_USB_RUNTIME_ENUMERATION=PASS_SCOPED`; `P3_C_PROTOCOL_HEALTH=BLOCKED_HOST_USB_ACL_NOT_ATTESTED`.** Signed normal9 update P3-B remained accepted as `UPDATE_COMMITTED=Ok/Committed VERSION=9` (original Mac receipt SHA `FABE432FE9FDCE1FC5CFFB5FCBD6337003256FFDB6E411B2114BEC5A59D6E78A`). This P3-C run obtained real fresh **post-commit** physical Linux sysfs: `/sys/bus/usb/devices/1-8`, `VID=1209 PID=000e BUS=1 DEVNUM=101`, node `/dev/bus/usb/001/101`, `root:root:664`. Hence STM32 re-enumerated to **Management runtime USB mode 000E**, rather than remaining in 000D bootloader. **It does NOT yet prove that v9 app SourceTree, USB protocol, PONG, watchdog, MSP/PSP, OLED or ESP bridge work**, because `deus` worker lacks permission to write new USB node. Do not equate USB PID with full firmware identity.

## Original native Mac evidence, exact immutable files

`esp010-v9-p3c-discovery-ro-1110-a1` command worker: `completed`, `exit_code=0`, `timed_out=false`, host `macmini`, worker `deus-command-worker-v2`, elapsed 0.065s. The exit0 means successful classification of the access blocker, **not** completed P3-C health PASS.

- Original queue JSON SHA256 `A40BD004448335F86397B134B3236238E04FEC33B7AF83E637EA9B5A8233EADF`.
- Original typed result JSON SHA256 `E6F690CCB4A9C300C46DCF3D93187ECC63356CB85648ABD68B3958F7F2707987`.
- Original complete Mac stdout/stderr log SHA256 `95B6A25D852F9B60A8974C322B0AE7CA8369CA9AE2FFD598B9603A488614BA0E`, 197 B.
- Source-locked Mac script `p3c_v9_postcommit_discovery_readonly_20261110.sh`, 2414 B SHA256 `B4DBBC345211136FDCCA8A253AE3144C2DE245FAF7997448BB36901EB76C7DF0`. It verified exact earlier P3-B original receipt SHA, permanent P3-B marker SHA and host DLL SHA `8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048`, read sysfs/node ACL, and stopped *before* CLI USB protocol access. No USB permission modification.
- Independent DEUS MCP original 3/3 SHA readback and original request-ID, request SHA, receipt, complete log semantic/length checks **PASS**.

```text
P3C_PREVIOUS_COMMIT_EVIDENCE_AND_HOST_PINS=PASS
P3C_SYSFS=PORT_1-8 VID=1209 PID=000e BUS=1 DEVNUM=101
P3C_USB_NODE=/dev/bus/usb/001/101 ACL=root:root:664
P3C_USB_ACCESS=BLOCKED_NO_PRIVILEGE_CHANGE
```

## Minimal next action, separate owner privilege boundary

Same known symptom as the earlier v8 postboot USB device re-enumeration: the current runtime node `root:root:664` denies the unprivileged `deus` account write permission for USB libusb; **not evidence of a v9 firmware failure**. No repeated P3-B UPDATE and no extra ENTER or RESET.

For optional future **owner-run**, exact node permission-only resolution, a dedicated guarded Mac file was prepared **but NOT EXECUTED**: `p3c_runtime_usb_permission_only_OWNER_NOT_EXECUTED_20261110.sh`, 1316 B SHA256 `A4071980E50B8AD268712A4794EC439FCE1476DD79D5233E31129E79E8089662`. It requires interactive root, `macmini`, uid1000 `deus`, unique `1209:000E` at sysfs `1-8`, devnum **101**, no bootloader and original node ACL `root:root:664`; it only `chown deus:deus` + `chmod 600` **on /dev/bus/usb/001/101**. It refuses changed/re-enumerated node, does not affect CH340, firmware, hardware reset or other device ACL. This script requires a separate owner manual decision and must not be invoked as part of P3-C read-only authorization. Native Mac `bash -n` source check only original job `esp010-v9-p3c-permission-helper-syntax-1110-a1` `completed exit0` receipt SHA `FADBF48AB2E35AFE181E267DCF0F964555B6A202F4F3E3153C7363CF24F4C2F0`; exact output `P3C_PERMISSION_HELPER_NATIVE_SYNTAX_ONLY=PASS_NOT_EXECUTED`.

**After manual owner-granted permission-only fix:** repeat independent read-only USB identity and `deus-cp` v9 runtime `info`, `ping`, `health`, `stacks`, check 39-method Registry/Service3 and compare actual runtime SourceTree with trusted v4/v9 build source provenance (do not assume it equals old v8). If ACL is not changed, **STOP**, do not bypass through sudo USB access in automation. P3-C stays **INCOMPLETE**.
