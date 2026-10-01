# Deus OS — Current Project State

Status: **SOLE GLOBAL PROJECT-STATE SOURCE OF TRUTH**

This file answers only one question: **where is the project now?**

It intentionally does not duplicate live Git `HEAD`/tree identity. Git itself is authoritative for repository bytes, commit identity, branch state and cleanliness. Documentation-only commits may advance `HEAD` without changing the accepted product boundary.

For architecture, roadmap sequence, boundary contracts, acceptance criteria, evidence policy and historical records, follow the precedence in `docs/DOCUMENTATION_MODEL.md`.

## 1. Latest completed product boundary

`ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED**

Boundary acceptance/publication commit:

`562e786ffa734da055c23144ec4256bc8961bbaf`

Boundary commit tree:

`88720a614794d5aef93cf13ac762a4095cb17baf`

Accepted publication-activation candidate:

- firmware source tree: `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`
- host source tree: `136687e80c42bd8104ad6c37fbbccb915b60fd08`
- BIN SHA-256: `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`
- whole-Flash SHA-256: `CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`
- Flash/SRAM: `54268/11944`
- task stacks remain `1024/512`
- system capability mask: `0x0000003F`
- final accepted persistence wear counter: `46/64`
- final Gate-5 recovery state: `CLEAN_STATE_POST_RESET_BYTE_EXACT`

Published v1 capability is deliberately bounded:

- management-IF2 binary Asset/Configuration transfer;
- object type `0x0001` / `OLED_UI_LAYOUT_CONFIG_V1`;
- exact current consumer payload `8` bytes;
- persistent A/B pages 62/63 with version/integrity/atomic-commit/recovery/wear contracts;
- production `PublishedOnly` host path accepted on Linux/libusb;
- response-loss idempotency, corruption fallback and physical VBUS retention accepted;
- accepted 128x9 status bar remains system-owned; only the console clip is persisted.

Canonical design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

Canonical acceptance:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

The preceding `HOST_CONTROL_APPLICATION_FOUNDATION` remains published at
`e0f49f168542fa1cf49bca451e01b0c077aa8d18`; it is no longer the latest completed
product boundary.

## 2. Current boundary disposition

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–6 ACCEPTED LOCALLY / GATE 7 ORDINARY NON-FORCE PUBLICATION CURRENT**

Current accepted Stage-1 normal-boot repair:

- Gate-3 narrow repair evidence SHA-256 `58E7FA5576DACC5BE636A2ABAD5B82E1BFDA1A463182957E7B039BB697F50292`;
- Gate-4 narrow repair evidence SHA-256 `62596F4066E6C0FEB476FBC412951123A61FED87731C6B48B473B28429BBCF07`;
- accepted repair trees: full `1972d7d59057eae8e89fda6b0028ccf07eca5bb1`, firmware `b3b6d4136106f6b6a195fb987a0c66a4b7812842`, host `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`, recovery tooling `24434b8f19be22251e4fd6129a8d39f06b101b9c`;
- immutable repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`;
- Stage-1 acceptance evidence `stm32_os_stage1_repair_apply_accepted_bundle_v3_20260930_155157.evidence.zip`, SHA-256 `3D3A28F0B0C949CCCFB817B7DC726B1B9F63104348EF954ABFEDD08A0F9F0EC5`;
- accepted post-repair whole-Flash SHA-256 `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`; recovery region `AB23050CE2A5E6CB5FE3638D8F8FAC299A7707F610116891D6BD349AB8A1E6D0`; bootloader BIN `509C40718A45CD64A078DCDAACE6CE3ECD92039CF3DF876B5E689E23277F73AA`; application BIN `C7EB7201906B5CA8DB8767DE6DB8C48F6397DCA3EB1AEB6C94CC6EFA2E95A4F8`;
- runtime acceptance: core running, `VTOR=0x08002000`, `VECTACTIVE=0`, fault magic cleared, kernel ticks advancing, runtime USB `1209:000C` present, bootloader USB `1209:000D` absent;
- physical OLED acceptance: `DEUS OS / DESKTOP / READY` = **PASS**;
- historical authorization note: this Stage-1 result did not itself authorize Stage 2; Stage 2 was later separately accepted by its own evidence/continuation.

Accepted Gate-5 Stage-2 record:

- `stm32_os_bootloader_gate5_stage2_explicit_update_entry_v4_20260930_180321.evidence.zip`, SHA-256 `80C670DF059ED37D0A7E2E7BBCF48F55FCC330B21381B68C93FE5DA385DDF8BA`, ZIP CRC clean and `68/68` hash-owned evidence entries exact;
- exact prestate and final whole-Flash SHA-256 remain `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`; no nonvolatile update command was sent;
- runtime `ENTER_BOOTLOADER` passed as normal user with `ENTER_STATUS=Ok` / `ENTER_STATE=Resetting`; bootloader `1209:000D` enumerated while runtime `1209:000C` disappeared;
- Linux bootloader node was initially root-owned and non-writable by `deus`; one operator-visible privileged action changed only the ephemeral device node to owner `deus`, mode `0600`; bootloader protocol itself remained unprivileged and no persistent host permission rule/configuration was created;
- bootloader `INFO` passed with `STATUS=Ok`, `STATE=RecoveryIdle`, expected offset `0`, version floor `1`, committed version `1`;
- while bootloader was idle, whole Flash remained byte-exact and `BEGIN/AUTHORIZE/DATA/END` were not sent;
- explicit CubeProgrammer `mode=HOTPLUG -rst` returned the target to runtime; `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks advanced `4771 -> 6492`, runtime USB `1209:000C` returned and bootloader USB `1209:000D` disappeared;
- technical Stage 2 is PASS and operator physical reconfirmation is `OLED=PASS` for `DEUS OS / DESKTOP / READY`; Stage 2 is therefore fully accepted. Formal continuation `stm32_os_bootloader_gate5_stage2_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `DA28EC5CD24A31DCE74F996F22D5D774EE1C5E8BD69AA2F7B9F3791608FD3676`, exact-binds the v4 evidence (`68/68`), records the operator physical PASS with zero target I/O/mutation, and authorizes Stage 3 as `GATE5_STAGE3_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Accepted Gate-5 Stage-3 record:

- `stm32_os_bootloader_gate5_stage3_invalid_application_recovery_v1_20260930_182233.evidence.zip`, SHA-256 `A507F73C628DA735FCF6A5C0A4E58ABB7A984397970ACB7BE3C64CAE9E4F7D50`, ZIP CRC clean and `72/72` hash-owned evidence entries exact;
- exact accepted prestate whole-Flash SHA-256 was `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, with erased persistence SHA-256 `D0FF1B294B5288D1AE1421EADF5B2D38A8752B76D472FF30BED9028E25B1C5B8`;
- invalidation was deliberately bounded to application page 59 at `0x0800EC00`: exactly page 59 changed, it became fully erased with SHA-256 `5F4ECDB7B71C3E403983FE405CDDCDC2F2576B655FDB3E80D94A6F7C32E58BC2`, metadata and persistence pages did not change, and the resulting invalid whole-Flash SHA-256 was `8195EFFFC3C8FE990BB8677EA6A9BC20B52B75A30B57CC3B68DF1B98A03DF4B4`;
- after explicit HOTPLUG reset, runtime `1209:000C` was absent and bootloader recovery `1209:000D` was present; bootloader `INFO` returned `Ok / RecoveryIdle`, expected offset `0`, version floor `1`, committed version `0`, proving the authenticated metadata retained the floor while the invalid application was not selected as committed bootable firmware;
- bootloader USB access again used only one operator-visible privileged ownership/mode change on the ephemeral device node; the actual protocol ran as `deus`, with no persistent host permission mutation;
- no USB update `BEGIN/AUTHORIZE/DATA/END` command was sent during this Stage-3 case;
- the accepted `PRESERVE_PERSISTENCE` recovery then completed successfully; recovery-region byte verification passed and final whole Flash returned exact to `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`;
- post-recovery runtime proof passed: runtime `1209:000C` present, bootloader `1209:000D` absent, `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, and kernel ticks advanced `20939 -> 22724`;
- Stage 3 technical evidence is PASS and operator physical reconfirmation is `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage3_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `E587AEC687955E4A5B93B1350D1489E4F6451726A454C09ED091A2F4A3539CA3`, exact-binds the `72/72` Stage-3 technical evidence, records zero target I/O/mutation for the continuation, fully accepts Stage 3, and authorizes Stage 4 as `GATE5_STAGE4_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Accepted Gate-5 Stage-4 record:

- `stm32_os_bootloader_gate5_stage4_malformed_header_rejection_v1_20260930_184648.evidence.zip`, SHA-256 `BD0ABADA1523C4E213A4F2B46997A62FE55124208939786543577D2DD434F281`, ZIP CRC clean and `68/68` hash-owned evidence entries exact;
- exact prestate whole-Flash SHA-256 was `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`; no pre-Stage-4 nonvolatile mutation was present;
- runtime explicit update entry passed with `ENTER_STATUS=Ok` / `ENTER_STATE=Resetting`; runtime USB `1209:000C` disappeared and bootloader `1209:000D` appeared;
- all six structural BEGIN-header rejection cases passed fail-closed: `FORMAT_VERSION_2`, `RESERVED_NONZERO`, `IMAGE_LENGTH_TOO_SMALL`, `IMAGE_LENGTH_TOO_LARGE`, `IMAGE_LENGTH_MISALIGNED`, `FIRMWARE_VERSION_ZERO`; every case returned `BadHeader / RecoveryIdle / offset 0 / floor 1 / committed 1`;
- pre- and post-malformed `INFO` were both `Ok / RecoveryIdle / offset 0 / floor 1 / committed 1`; `AUTHORIZE_HEADER`, `DATA`, and `END` were not sent;
- whole Flash remained byte-exact at `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` while the bootloader was still active after all malformed headers, proving BEGIN structural rejection was non-mutating;
- the bootloader USB permission boundary remained bounded to one operator-visible ownership/mode change on the ephemeral `/dev/bus/usb/...` node; protocol execution itself remained unprivileged and no persistent host permission change was created;
- explicit HOTPLUG reset returned the target to runtime; runtime `1209:000C` present, bootloader `1209:000D` absent, `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks advanced `4806 -> 6555`, and final whole Flash remained exact;
- Stage 4 technical evidence is PASS and operator physical reconfirmation is `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage4_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `AD194B3198D19EF14EA726568DF50207DE7B09EBC4EF2C2FBE2E974974121F5D`, exact-binds the `68/68` Stage-4 technical evidence, records zero target I/O/mutation for the continuation, fully accepts Stage 4, and authorizes Stage 5 as `GATE5_STAGE5_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.

Accepted Gate-5 Stage-5 record:

- `stm32_os_bootloader_gate5_stage5_wrong_target_rejection_v1_20260930_190851.evidence.zip`, SHA-256 `2BAADC493338BA31A26F38F33A0488C7F2B7F092F2ABB8B8F420296D2766043F`, ZIP CRC clean and `72/72` hash-owned evidence entries exact;
- exact accepted prestate whole-Flash SHA-256 was `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`; no pre-Stage-5 nonvolatile mutation was present;
- exact accepted repair bundle SHA-256 `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E` validated before target entry; its frozen signed wrong-target vector SHA-256 was `B82A52EF431017A27C0BAA64D346982A93AB6A7D1240C18C14F11A3EDB2CB5FB`, with product `0x534F4544`, target `0x0411`, firmware version `2`; raw acceptance key was not exported;
- runtime explicit update entry passed with source tree `b3b6d4136106f6b6a195fb987a0c66a4b7812842`, `ENTER_STATUS=Ok` / `ENTER_STATE=Resetting`; runtime USB disappeared and bootloader `1209:000D` appeared;
- pre-case bootloader `INFO` was `Ok / RecoveryIdle / offset 0 / floor 1 / committed 1`; structurally valid signed wrong-target `BEGIN` passed as `Ok / HeaderStaged`, then `AUTHORIZE_HEADER` returned exactly `TargetMismatch / HeaderStaged / offset 0 / floor 1 / committed 1`;
- `DATA` and `END` were not sent; whole Flash remained byte-exact at `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD` while the bootloader was still active after target rejection, proving the target check failed before metadata erase/application mutation;
- Linux permission handling remained bounded to one operator-visible ownership/mode change on the ephemeral `/dev/bus/usb/...` node; protocol execution itself remained unprivileged and no persistent host permission mutation occurred;
- explicit HOTPLUG reset returned runtime `1209:000C`, removed bootloader `1209:000D`, restored `VTOR=0x08002000`, `ICSR=0x0400F000`, `VECTACTIVE=0`, kernel ticks advanced `4732 -> 6451`, and final whole Flash remained exact;
- Stage 5 technical evidence is PASS and operator physical reconfirmation is `OLED=PASS` for `DEUS OS / DESKTOP / READY`. Formal continuation `stm32_os_bootloader_gate5_stage5_acceptance_continuation_v1_20260930.evidence.zip`, SHA-256 `C35A3319B806B21BC42B48EAF84822DF0F721AEE5EF5222BFA45C959F0BB90EE`, exact-binds the `72/72` Stage-5 technical evidence, records zero target I/O/mutation for the continuation, fully accepts Stage 5, and authorizes Stage 6 as `GATE5_STAGE6_AUTHORIZED_BY_THIS_RESULT=YES_AFTER_EXACT_PARENT_EVIDENCE_BIND_AND_OPERATOR_PHYSICAL_PASS`.






Gate-5 closure record after the Stage-5 chronology above:

- Stage 6 bad-digest/corruption case is fully accepted: technical v4 evidence SHA-256 `0C5FD70A063F6E4BB366ED85A45DD4AB060B8B645C5983AF7D3E0E486C2703DA`; formal physical-OLED continuation SHA-256 `D726C04A812BDAB3262E7D352A146481E627894586ADA89A00DA026614175717`. It also exercised the protocol-defined exact immediate DATA retry on two real response timeouts and restored the exact repair-candidate baseline.
- Stage 7 bad-authenticator rejection is fully accepted: technical evidence SHA-256 `43DF41301E005D6E23DEE7C71D3076F97797AF3419232C46A57F3FAA9F916515`; formal continuation SHA-256 `EFA420DA03CB0300937B1DBC99C73CBBC66B7627577E82E92AC3F1252B5D7CA9`. Rejection occurred before DATA/END and before authorization-side Flash mutation.
- Stage 8 rollback/version rejection is fully accepted: technical v2 evidence SHA-256 `D2006CEE054A85ECA59B96288B3CE25AFCF5BF97B5611D994CF8F7F06B3DB61F`; formal continuation SHA-256 `57761DE3C23839566983172E403884F85B1A583AB7C5C9ADC512E6096F22B921`. Exact signed version 1 was rejected as `VersionRejected` at floor/committed version 1 with no DATA/END and byte-exact Flash.
- Stage 9 deterministic reset interruption/retry is fully accepted: technical evidence SHA-256 `2775E2565EA10BCEF8A72EDEE052D698A515B347C6E177D7ABB972D7F36C1EAB`; formal continuation SHA-256 `C0240BA36BD79C5DB8413B50B4885A6ADBBE6FB69BF2907A75153CEE89D333EB`. Reset after AUTHORIZE preserved the bootable v1 state; reset after the first DATA ACK produced exact partial-Flash state and recovery; full restart from offset zero committed version 2 with exact application/metadata/bootloader/persistence ownership.
- Stage 10 physical Mac-mini native-USB/VBUS interruption is fully accepted: technical evidence SHA-256 `24FA0C42644B7E87F88AAD0B775547C7272A591A366D11A4903274C3F47D6CA3`; formal continuation SHA-256 `E6EB019B021AB95DB16CA3B95E1C3F7C2C7F44D862ABF2711F6B42089F35D560`. The target was physically unpowered for at least 2 seconds after the first DATA ACK, returned directly in recovery, successfully retried from zero, and was finally restored to whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`.
- Post-Stage-10 host reliability audit found and repaired two host-only defects without reopening firmware hardware acceptance: delayed-response poisoning across a fresh request-ID DATA retry, and Windows WinUSB timeout `121/1460` misclassification. Validation evidence SHA-256 `61D8AEBCA99E712D893474376CA4FEDD63D17D11021E45C0F7701621C712B108` passed Core `39/39`, Transport `12/12`, Core/Windows/Linux Release builds, exact source/poststate locks and zero target/remote/Git mutation. Formal repair continuation SHA-256 `253026C36C346C579AD5C1CE0FD528A7F7943A07CC21D792171C14C3350797FD`.
- No second firmware-side response-loss mechanism is claimed from the historical INFO/BEGIN/DATA timeout observations. The accepted host repair handles explicitly stale timed-out responses; unknown request-ID mismatches remain fatal. No speculative bootloader patch is authorized.
- Gate-5 hardware functional matrix and technical reliability audit are complete. Gate 6 is locally accepted by the normal acceptance commit containing this record. Firmware-update capability bit 6 is active (`SYSTEM_IDENTITY_CAPABILITIES = 0x0000007F`). Gate-6 validation evidence SHA-256 `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7` passed `92/92` evidence hashes, firmware candidate tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, host candidate tree `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, validated pre-commit full candidate tree `8860661c9bcd5e4424fa5c36b37d48815b8ea7ef`, firmware BIN SHA-256 `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, Flash/SRAM `53212/10956`, Core `41/41`, Transport `12/12`, and zero target/remote/real-index/ref mutation. Gate 7 ordinary non-force publication is current; no push has occurred in Gate 6.

Published prerequisite:

- `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — Gates 0–7 accepted/published at
  `a8f92f83c2ba8917ad183b1a099c9e21199c9463`, tree
  `013c1f472eb9404de94befdcc3f1e1e5acfc5831`;
- Gate-7 evidence SHA-256
  `335013389D8AB6C1B09C4BF720185FED82EC4A0D30A2F3F6237CF8ACCA1DE8CE`;
- accepted application source tree
  `a10182e7d0659b9b161073ad49a8816ecb6e7918`;
- accepted BIN SHA-256
  `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`;
- Flash/SRAM `52908/10920`, task margins `304/432`, MSP margin `1592`.

Canonical design:

`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_PLAN.md`

Canonical acceptance:

`docs/FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION_ACCEPTANCE_PLAN.md`

Canonical wire/image protocol:

`docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`

Gate 0 is intentionally non-destructive. It must freeze and prove the minimum safe
bootloader/update architecture before any product linker/startup/Flash mutation:

- bootloader/recovery pages 0..7, hard ceiling `8192` bytes;
- relocated physical application ownership `0x08002000..0x0800F7FF` split into executable pages 8..59 (`52K`) plus firmware metadata A/B pages 60/61 (`2K`);
- persistence pages 62/63 unchanged and forbidden as implicit update staging;
- explicit recoverable in-place update versus formal Flash-layout reopen;
- exact update-entry mechanism;
- exact product update transport; CDC remains secondary diagnostics, not the default
  Bootloader carrier;
- device-enforced authenticity/trust, integrity, target/version/rollback contract;
- exact image-validity metadata ownership and atomic boot-valid rule;
- application VTOR/handoff contract;
- historical Gate-0 rule: capability bit 6 stayed clear throughout implementation/destructive acceptance; Gate 6 activated it only after complete Gate-5 hardware/reliability acceptance and then passed the local publication-candidate validation.

Gate 1 Host Core ownership split is accepted from evidence SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`: `DeusDeviceClient.cs` is a 377-line compatibility/application facade; `DeviceProtocolChannel` owns the single transport/decoder/request-correlation/gate; `DeusRpcClient` owns generic RPC; `AssetTransferClient` owns Asset transactions; existing `DeusDeviceSession` remains reconnect/discovery owner. Core tests passed `28/28`, Transport `5/5`.

Gate 2 is accepted from evidence SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`. Accepted firmware tree `637ea07b10cf84882e19cbb8239f31b7f48856a7`; application Flash/SRAM `52932/10920`, end `0x0800EEC4`, vector origin `0x08002000`, metadata/persistence boundaries exact, undefined `0`. Gate 3 and Gate 4 are accepted; the later Gate-5 hardware matrix is now complete. Resource attribution proved: pre-refactor canonical load image `53340/53248` (+92); pre-refactor isolated `usb_management.c -Os` `53292/53248` (+44); shared-finalizer canonical `53332/53248` (+84); shared-finalizer + isolated `usb_management.c -Os` `53276/53248` (+28). Linked-symbol comparison localizes the entire Gate-2 -> Gate-3 canonical growth to `usb_management`: accepted Gate-2 `runtime_init + runtime_service = 268` bytes, Gate-3 runtime-entry/response/reset plus enlarged init/service = `668` bytes, exactly `+400` bytes, matching the full application load-image growth `52932 -> 53332`. The authorized Gate-3 boundary is 21 paths so RPC, Asset and Firmware Update responses share one Binary Framed envelope/CRC owner. Gate 2 is now explicitly reopened only for the measured build-profile change that adds `src/kernel/usb_management.c` to the existing `$SizeOptimizedSources` set; `linker/stm32f103c8.ld`, `src/startup.s`, origin, 52-KiB limit, metadata/persistence geometry and resource ceilings remain frozen. The shared finalizer is preconditioned so caller-proven bounds are not redundantly rechecked, while `binary_frame_encode()` retains checked compatibility semantics. This narrow reopen is accepted from `stm32_os_bootloader_gate2_reopen_gate3_resource_acceptance_recomposed_v1_20260929_134328.evidence.zip`, SHA-256 `891C3075432F5540E521DABA60E18FD7228E8943725BBF1A92A398D118DD852A`: canonical Arm GNU build PASS, application Flash/SRAM `53212/10956`, margins `36/308`, load-image end `0x0800EFDC`, MSP/reset `0x20005000/0x080020D9`, BIN/ELF/MAP SHA-256 `32E225DF6705E5F79ABE433FBA92AFF50353D1802D90B67508667B9C070ECB69` / `1BD4A9E4CC3B8AF99351B9FCD89E513CBD987236C46D1F370347C63DE3CA5136` / `ABB04D59130CEA2E6ECD8FE28F56D338DC2EFE59E510AD896923B129E299D69D`, full candidate tree `9a9646b67a035d16cc1d164fd4ff9a6a7ab3006d`, firmware candidate tree `c8212ef6de086da50d3f37465273f78d12c329bb`, exact poststate and no target I/O. Gate 3 full non-target implementation acceptance is **PASS** from `stm32_os_bootloader_gate3_implementation_acceptance_recomposed_v12_20260929_155027.evidence.zip`, SHA-256 `E016B495CECE75F8BD2A4C120EC0E5506EFF31ED984566007EA6363CC6D35846`. Pre-repair Gate-3 candidate trees were full `34bee09f7e69e3bf55eee3ac6ca274bd70f137d4`, firmware `8a0d64817de2d0eb9a3b465d3cd57c392259178a`, host `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`. Current accepted normal-boot repair candidate trees are full `1972d7d59057eae8e89fda6b0028ccf07eca5bb1`, firmware `b3b6d4136106f6b6a195fb987a0c66a4b7812842`, host unchanged `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4`; recovery-tooling tree `24434b8f19be22251e4fd6129a8d39f06b101b9c`. Pre-repair application Flash/SRAM were `53212/10956`, with BIN/ELF/MAP SHA-256 `D799F9FCAE1663ADE59F9783A01499E9FBA0FD954E50A09A62A94C6AA2CBB607` / `0002C39AED7FA405797F056E7DEA31F5D0A63D435C345D5802657D7A39F8D9F4` / `36DD69B8CDEC1AEA3F9E2527BBD5D75E349EFC5488334E4E9D0C2D12171E0BCF`; the accepted normal-boot repair application BIN is now `C7EB7201906B5CA8DB8767DE6DB8C48F6397DCA3EB1AEB6C94CC6EFA2E95A4F8` at the same `53212/10956` resource footprint. Bootloader acceptance is Flash `5896/8192`, `.data+.bss=660/1024`, conventional SRAM `1684/2048`, max frame `248/256`, longest linked stack `648/768`, MSP margin `376`; deterministic package-generator preflight and independent HMAC/package oracle passed; Host Core/Windows/Linux Release builds passed with zero warnings/errors, direct MTP Core `33/33` and Transport `8/8`; exact poststate passed and target I/O/Flash/reset/UART/target USB/index/commit/push mutations were not performed. Gate 4 product revalidation is **PASS** from `stm32_os_bootloader_gate4_fresh_revalidation_recovery_acceptance_v2_20260929_190357.evidence.zip`, SHA-256 `974E482D098973FEE1F56DAA9A767E228A7EA7DBA98D747FCF5E5AD632F905B7`; the recovery artifact is under a narrow tooling-only reopen before destructive Gate 5 because Stage-0 hardware preflight exposed a PowerShell UInt32 comparison defect in `stm32_bootloader_recovery.ps1` (`FLASH_WRPR` raw hardware value `FFFFFFFF` was parsed correctly but compared to a signed bare literal). Recovery-tooling repair v1 then failed before any repair work on a harness-only PowerShell argument-vector binding bug (`Invoke-Git([string[]]$Args)` called with a bare array expression), falsely reporting HEAD drift even though repository HEAD remained `a8f92f83c2ba8917ad183b1a099c9e21199c9463`; repair-v1 evidence SHA-256 is `5DABB31CFFA090DC8EFD14B62DBB70B062572135B5096A1D0D203662B96ECA9E`, target I/O zero. Recovery-tooling repair v2 is **PASS** from `stm32_os_bootloader_gate4_recovery_tooling_repair_v2_20260929_211058.evidence.zip`, SHA-256 `4FE642BD172ACAE390EB4BCCE1A8ABA7E2C6B5FFCD7D28319A2AAFAC4E9B0E31`; internal evidence hashes `7/7` exact, poststate exact, target I/O zero. It produced repaired tooling tree `a29b27aebd8f30fcd0448d561a7820a50a4cbb61`, fixed recovery-common SHA-256 `8180084FA67423ADD3DEEA56F83C85D94AE120319058CC10E22557DA2E829CFC`, and sensitive recovery ZIP v2 `stm32_os_bootloader_gate4_immutable_recovery_bundle_v2_20260929_211058.zip`, SHA-256 `E4D8928E17C809B2A0BE7736379C4ACE3274B0B4AEB47621D9FBFBA0BC2FD3AE`. Evidence proves directory and extracted-ZIP `VALIDATE_ONLY` PASS and byte-identical recovery region/package vectors. The recovery ZIP v2 has now also been independently byte-audited: outer SHA-256 `E4D8928E17C809B2A0BE7736379C4ACE3274B0B4AEB47621D9FBFBA0BC2FD3AE`, ZIP CRC clean, inventory exactly 11 expected entries, all 10 hash-owned entries exact, bundle manifest/hash-index byte-identical to repair-v2 evidence, recovery region geometry exact, metadata A/B exact, package payloads exact, fixed recovery-common SHA exact to repo. At that checkpoint Gate 4 had been fully accepted again; Gate-5 Stage 0 read-only preflight was the next required step and destructive Stage 1 remained blocked until Stage-0 PASS. Stage-0 v4 evidence `stm32_os_bootloader_gate5_stage0_readonly_preflight_v4_20260929_212350.evidence.zip`, SHA-256 `B16F46E0A43C2790D80F65DB78AC9E1BC2BA6132942DFB6A9EE08B9BD92C6A55`, is a harness connect-mode false positive: recovery v2 validation, Mac-mini runtime USB before SWD, CH340 discovery, UInt32 replay, ST-LINK device/Flash/OBR/WRPR/option-byte checks and full 64-KiB readback all passed; poststate passed and no nonvolatile mutation/explicit reset/update-entry/UART-TX command was issued. Runtime USB disappeared only after CubeProgrammer `Connect mode: Normal / Reset mode: Software reset`, which resets and halts the core by design. Correct classification is `HARNESS_STLINK_CONNECT_MODE_SCOPE`; Stage-0 read-only probes must use and verify `mode=HOTPLUG`. Stage-0 v5 evidence `stm32_os_bootloader_gate5_stage0_readonly_preflight_v5_20260929_213433.evidence.zip`, SHA-256 `EA6414953C72A4A0DF7583B2FE599FD0182EC2E4E4B3BCFF77CAD8FC83F939B1`, is technically complete: ZIP CRC clean, internal hashes `52/52` exact, recovery v2 validation PASS, runtime USB present before/after SWD, all six acceptance SWD reads reported `Connect mode: Hot Plug`, Flash/OBR/WRPR and full 64-KiB baseline matched exactly, poststate exact, and no nonvolatile mutation/update-entry/UART-TX occurred. However the harness violated the operator-facing terminal contract by emitting an uncolored RESULT block and then printing a green `EVIDENCE_ZIP_SELF_TEST=PASS` line after it. The run is therefore frozen as underlying technical PASS with `HARNESS_TERMINAL_STRUCTURE_01`; it does not itself authorize destructive Stage 1. Formal Stage-0 acceptance is now provided by `stm32_os_bootloader_gate5_stage0_acceptance_continuation_v1_20260929_230244.evidence.zip`, SHA-256 `31FBFF9ABD04EF343B0617CA5149F0ED076B338993F54817E42C63526F2F1B1D`: ZIP CRC clean, `37/37` hash-owned entries exact, v5 technical evidence replayed `52/52`, runtime USB `1209:000C` present before/after current HOTPLUG verification, OBR/WRPR exact, full-Flash SHA-256 still `17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`, poststate exact, authorization record cardinality correct, and the final colored RESULT block is last. Gate-5 Stage 0 is accepted. Stage-1 v1 performed the first destructive `CLEAN_STATE` and proved the canonical Flash image byte-exact, but its post-recovery runtime check is **not yet attributable to product code**: `stm32_os_bootloader_gate5_stage1_clean_baseline_v1_20260930_114012.evidence.zip`, SHA-256 `AA10BD268339EF3251D397E761B72375B133DF93590919306EAA8C45631BF4B1`, has clean ZIP CRC and `58/58` internal hashes; pre-recovery Flash was exact Stage-0 `17B48E...`, explicit pages `0..63` erase/program/verify succeeded, post-recovery whole Flash is exact canonical CLEAN SHA-256 `B8D61449ED029E3F93869656C94BB85929509F01311686A7ECD5AD110421E69C` with exact bootloader/application/metadata/persistence regional hashes, and repository poststate is exact. The subsequent recovery `FINAL_SOFTWARE_RESET` used default CubeProgrammer `Connect mode: Normal`, which resets then halts on attach, after which neither runtime USB `1209:000C` nor bootloader USB `1209:000D` appeared in ten probes. Therefore the recorded `PRODUCT_GATE5_STAGE1_NORMAL_BOOT_FAILURE` is not accepted as a product attribution; that original Stage-1 v1 run is frozen at a reset/execute attribution gap under `HARNESS_STLINK_CONNECT_MODE_SCOPE`. Current work has now attributed the failure to a real product handoff defect plus a separate recovery-tool final-reset defect. Reset-attribution evidence `stm32_os_bootloader_gate5_stage1_reset_attribution_diag_v1_20260930_115228.evidence.zip`, SHA-256 `14A44E5290593C316F8AC320E95A92A20030E61E27757F61D2CBCC667F2F2426`, has clean ZIP CRC and `72/72` internal hashes. Raw hardware facts are valid: CLEAN Flash remains exact `B8D61449ED029E3F93869656C94BB85929509F01311686A7ECD5AD110421E69C`, initial and post-reset core state are `RUNNING`, initial `VTOR=0x08002000`, `DHCSR=0x01010000`, and neither runtime `1209:000C` nor bootloader `1209:000D` enumerates before or after one explicit `mode=HOTPLUG -rst`. The diagnostic's structured `outcome.txt`/terminal RESULT is not an acceptance record because a case-insensitive PowerShell `$outcome` / `$script:Outcome` collision spliced the frozen Stage-1 outcome text into `FINAL_OUTCOME` and the RESULT banner; freeze that presentation defect as `HARNESS_POWERSHELL_STATE_VARIABLE_COLLISION_01` while retaining the raw native measurements. Source attribution is now causal: `bootloader/handoff_app()` executes `cpsid i` before VTOR/MSP branch, while application `Reset_Handler` never executes `cpsie i`; `usb_device_init()` only enables USB IRQ20 in NVIC, so PRIMASK remains set and USB/SysTick/UART IRQ delivery never starts despite the application core running. The narrow product fix removes the leaked interrupt mask from normal boot handoff. Independently, `stm32_bootloader_recovery.ps1` now performs its final software reset with explicit `mode=HOTPLUG` instead of default `NORMAL`. Gate 3/4 bootloader/recovery identity is therefore narrowly reopened; this is **not** a wholesale rerun of accepted Gate 3 or Gate 4. The Gate-3 21-path list is an authorization/change boundary, not the definition of the accepted full/firmware/host candidate-tree inventories. Host product bytes are not reopened by the PRIMASK/recovery-reset repair: accepted host identity `96d5f1f73ddd65783c4e7f6b10ae61b6a72d0ef4` and its accepted build/test evidence carry forward unless an authorized host source byte changes. The narrow Gate-3 reopen owns `bootloader/bootloader.c` plus the consequential firmware-source-tree identity change embedded in the otherwise byte-equivalent application, and must reuse the accepted Gate-3 v12 candidate-tree derivation semantics rather than synthesize trees from an empty index containing only authorized change paths. The narrow Gate-4 reopen owns recovery-tooling bytes plus fresh immutable-bundle generation/validation only. That block is now closed by the accepted narrow repair. Gate-3 narrow repair evidence SHA-256 is `58E7FA5576DACC5BE636A2ABAD5B82E1BFDA1A463182957E7B039BB697F50292`; Gate-4 narrow repair evidence SHA-256 is `62596F4066E6C0FEB476FBC412951123A61FED87731C6B48B473B28429BBCF07`; exact accepted repair bundle SHA-256 is `8871F0E7D770A3CA45A942638552B53B0BABE3EECE946ECB4C0E74DA64616B9E`. Stage-1 normal-boot repair is accepted from `stm32_os_stage1_repair_apply_accepted_bundle_v3_20260930_155157.evidence.zip`, SHA-256 `3D3A28F0B0C949CCCFB817B7DC726B1B9F63104348EF954ABFEDD08A0F9F0EC5`: ZIP CRC clean, `41/41` hash-owned evidence entries exact, post-repair whole Flash SHA-256 `A9F279C2DEB70937A4A3BED1B0A2919D50374B8BCE4B1C15948B8244F17062CD`, recovery region `AB23050CE2A5E6CB5FE3638D8F8FAC299A7707F610116891D6BD349AB8A1E6D0`, bootloader BIN `509C40718A45CD64A078DCDAACE6CE3ECD92039CF3DF876B5E689E23277F73AA`, application BIN `C7EB7201906B5CA8DB8767DE6DB8C48F6397DCA3EB1AEB6C94CC6EFA2E95A4F8`, persistence preserved erased, core running with `VTOR=0x08002000`, `VECTACTIVE=0`, fault magic cleared, advancing kernel ticks, runtime USB `1209:000C` present and bootloader USB `1209:000D` absent. Physical OLED acceptance is `DEUS OS / DESKTOP / READY` = PASS. The evidence explicitly states `GATE5_STAGE2_AUTHORIZED_BY_THIS_RESULT=NO`; Stage 2 remains not yet authorized. Exact accepted Gate-3 product trees were reproduced; recovery-tooling tree is `77b8824fb3b83f12c7b7fa082907fbad3bdddc34`. Application remained exact at BIN `D799F9FCAE1663ADE59F9783A01499E9FBA0FD954E50A09A62A94C6AA2CBB607`, Flash/SRAM `53212/10956`; same-toolchain debug-stripped Gate-3/Gate-4 ELFs were byte-identical at SHA-256 `698D88EF4C09F2C9A4D33F7B0A333128A725AB4A799BF0893368E9EA34743729`. Bootloader resource/stack replay remained `5896` Flash, `.data+.bss=660`, conventional SRAM `1684`, clone/max/longest/margin `48/248/648/376`; Host builds/tests passed Core `33/33`, Transport `8/8`. The immutable sensitive recovery ZIP is `stm32_os_bootloader_gate4_immutable_recovery_bundle_v1_20260929_190539.zip`, SHA-256 `00992EC3CF8F31243B322CE2BEF0668CE94889BF47F56DFBEE28EBF14A6AC50E`; recovery region SHA-256 `30642D7947B6C776B1550E8EECA185B8A2BB285A4AFBC3F6E7F01ED0CE6C8088`; signed baseline/update/wrong-target package SHA-256 `FD4AF4D085CDB585C13BC332F729EF86300EED54FF69A42F78F3CC46F16B5387` / `17553B21CA295348394726F943E84245CEA69CC6C51019F2121FB84DF44D9520` / `EE58CE64C33A7B4BFEC3EEC15DB36FE14BE733AC9B63E184579495589BBEE581`. Bundle source-directory and extracted-ZIP `VALIDATE_ONLY` both passed; raw acceptance key is not persisted; Gate-4 target I/O remained zero. Gate-4 v1 is retained as a harness artifact-identity false positive caused by path-bearing debug ELF/MAP metadata. Gate-3 v11 passed package-generator preflight, application identity/build, bootloader build, stack `48/248/648/376`, real package generation and HMAC oracle, then failed only at Host locked restore because an inherited temporary NuGet package cache contained an incomplete locked `xunit.v3.mtp-v2/4.0.1` entry without its nuspec. Host acceptance now owns per-run `DOTNET_CLI_HOME` and `NUGET_PACKAGES` under WorkRoot. Gate 5 may perform target Flash/reset only inside its bounded hardware acceptance matrix after exact Gate-4 recovery-bundle SHA binding and local `VALIDATE_ONLY`.

Gate 0A is accepted from `stm32_os_bootloader_gate0a_contract_source_audit_v1_20260927_200959.evidence.zip`, SHA-256 `BB3D9210639564D27E0E9FFF57817A10E619991EC3A1960D4D5D29E82E122002`. It proved exact published recovery binding, docs-only WIP, linker/startup/Flash/recovery/host/capability ownership and zero product/target mutation.

The ten Gate-0 architecture decisions are now frozen in the canonical plan: management-IF2/BKP reset entry, minimal vendor/bulk WinUSB+libusb bootloader transport on private-test `1209:000D`, fixed 48-byte image header, HMAC-SHA-256 trust/authentication, exact header-authenticated bytes and payload-digest binding, product/device/origin binding, monotonic rollback floor, firmware metadata A/B pages 60/61, full-retry in-place update ordering, and a `<=2048`-byte conventional SRAM budget with 1024-byte MSP.

Gate 0B is accepted from `stm32_os_bootloader_gate0b_linked_feasibility_recomposed_v4_20260927_225243.evidence.zip`, SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`. The complete acceptance-only bootloader links at `5756/8192` Flash and `1684/2048` conventional SRAM; max frame `248`, longest linked stack path `648`, MSP margin `376`, undefined symbols `0`. The exact current application relocates to `0x08002000/52K` at Flash/SRAM `52932/10920`, end `0x0800EEC4`, without touching metadata/persistence.

Historical Gate-3 implementation record (retained for chronology; the later Gate-5 hardware matrix is now complete): bootloader/update transport/security implementation within the exact 21-path authorized Gate-3 source boundary, including the measured shared Binary Framed finalizer composition repair. Gate-2 linker/startup and all relocation/resource ceilings remain byte-frozen; `scripts/build_firmware.ps1` is the already-accepted narrow compiler-profile reopen. Gate-3 v5 reached the real bootloader build but exposed a build-observer defect: GNU `size` aggregate BSS included the dedicated 1024-byte NOLOAD `.boot_stack`, so `build_bootloader.ps1` misreported `.data+.bss` as `1684/1024` and would then have double-counted MSP. The linker had already accepted the real `.data+.bss <=1024` contract. Bootloader SRAM metrics are now derived from linker symbols (`_sdata/_edata/_sbss/_ebss/_sboot_stack/_eboot_stack`) so stack is counted exactly once. Gate-3 v6 then rebuilt the application at the same `53212/10956` and `0x0800EFDC` layout but correctly embedded the new firmware candidate tree `96ba126520f5d2fbc66390b3ae6a788c1751e1ca`, changing BIN SHA from the resource-baseline `32E225...` to `2B50C095...`. Byte-level normalization proves this drift is only the single 40-byte source-tree identity string at BIN offset `0xCF20`: replacing the old tree with the current tree in the accepted BIN produces exactly `2B50C09587ED64DC10D4A5B78ABA83E9C5430EBF99C0977CD6E07A5EDB077943`. Full Gate-3 acceptance therefore binds current `BUILD_SOURCE_TREE` and normalized application equivalence instead of the stale raw resource-baseline BIN SHA. Gate-3 v7 then completed application identity and bootloader build/resource checks (`5896/8192` Flash, real `.data+.bss=660/1024`, MSP `1024`, conventional SRAM `1684/2048`) but the stack oracle stopped on the sole GCC IPA clone `metadata_authenticated.constprop.0`, because source `.su` names did not contain the linked `.constprop.0` suffix. Final disassembly proves that clone's own frame is 48 bytes (`push {r4,r5,lr}` = 12 plus `sub sp,#36`), the reachable graph has 31 functions and no unresolved callees, and an independent linked-disassembly stack calculation reproduces the accepted Gate-0B bounds exactly: max frame `248`, longest linked stack `648`, MSP margin `376`. The stack oracle is being corrected to resolve recognized GCC clone names through the compiler's canonical `.su` stem (for example `.constprop.0` -> `.constprop`) and accept the clone only when its own balanced final-disassembly frame equals that `.su` frame; ordinary frames remain exact `.su`-owned. Gate-3 v8 reached that parser with valid `.su`/disassembly data but failed before analysis because `Get-AsmText([string]$Line)` was mandatory and PowerShell rejected a legitimate empty disassembly line during parameter binding. Exact v8 artifact replay with empty lines accepted completes the entire graph and reproduces clone frame `48`, max frame `248`, longest linked stack `648`, MSP margin `376`; this is a harness line-consumer binding defect, not a stack/product defect. Gate-3 v9 corrected the empty-string parameter contract but placed its `Get-AsmText -Line ''` self-test at script scope while `Get-AsmText` is a nested helper local to `Analyze-BootStack`; PowerShell therefore correctly reported the helper as undefined before stack analysis. A full scope audit of the generated v9 harness found no other nested-helper call escaping its owner. The self-test is moved inside `Analyze-BootStack`, and nested-helper scope auditing is now a required generator check. Gate-3 v10 then passed the corrected stack oracle exactly (`clone=48`, max frame `248`, longest linked stack `648`, MSP margin `376`) and reached the real package generator. `create_firmware_update_package.ps1` failed on C#-style `$array.AsSpan(...)`: PowerShell does not provide that extension-method call as a reliable instance API. The package generator is corrected to write v1 little-endian header/origin fields explicitly into byte arrays, preserving package bytes/crypto semantics, and full acceptance must first run a deterministic host-only package-generator self-test with an independent expected package hash. Gate 5 may perform target Flash/reset only inside its bounded hardware acceptance matrix after exact Gate-4 recovery-bundle SHA binding and local `VALIDATE_ONLY`.

### Published Asset acceptance record

The original `DEFERRED_NO_REAL_CONSUMER` Gate 0 result remains historical proof that implementation was correctly blocked when no real consumer existed.

That blocking condition was resolved by the promoted narrow consumer:

- consumer: `OLED_UI_LAYOUT_CONFIG_V1`;
- canonical consumer contract: `docs/OLED_UI_LAYOUT_CONFIG_V1_CONSUMER.md`;
- target object type: `0x0001`;
- exact consumer payload: `8 bytes`;
- scope: persisted OLED **console clip only**;
- accepted status bar remains frozen/system-owned at `128x9`;
- compiled default remains console `x=1, y=10, w=126, h=22`.

Gates 0–7 are accepted and the boundary is published at
`562e786ffa734da055c23144ec4256bc8961bbaf`.

Frozen Gate-0 contracts:

- first-class bounded binary transfer ABI — `docs/ASSET_CONFIGURATION_TRANSFER_PROTOCOL_V1.md`;
- persistent A/B record envelope + atomic commit/recovery — `docs/ASSET_CONFIGURATION_PERSISTENCE_V1.md`;
- resource budget — `docs/ASSET_CONFIGURATION_RESOURCE_BUDGET_V1.md`: Flash `<=54272`, SRAM `<=12288`, task margins `>=256/256`, MSP margin `>=1024`;
- Flash wear/timing/watchdog/USB continuity — `docs/ASSET_CONFIGURATION_FLASH_OPERATION_POLICY_V1.md`;
- deterministic reset/power-loss fault-injection matrix — `docs/ASSET_CONFIGURATION_FAULT_INJECTION_V1.md`;
- recovery bundle / ST-LINK restoration — `docs/ASSET_CONFIGURATION_STLINK_RECOVERY_V1.md`.

All six Gate-0 design contracts are frozen and the 2026-09-22 cross-contract closure audit is **PASS**.

Closure found and corrected two issues before authorization:

1. HELLO Asset bit 6 is carrier-specific: CDC remains `0x3F`, management IF2 candidate becomes `0x7F`;
2. ST-LINK recovery uses explicit page erases followed by CubeProgrammer `--skiperase`; PRESERVE never erases pages 62/63.

Current accepted Gate-5 candidate source tree is `f945045221adb52e1aa7a13f4e89b3dbd1ddb5de`.

Gate-5 status:

- deterministic Campaign A — **ACCEPTED**;
- deterministic Campaign B with previous valid record — **ACCEPTED**, including FI-1..FI-9 and the FI-9 response-loss/idempotency proof;
- deliberate persistence-page erase attempts accumulated through Campaign B: `20/64`;
- accepted Campaign-B final generation: `2`, payload `01100A6016000000`, payload CRC `838D1F51`;
- accepted Campaign-B whole-Flash SHA-256: `CCE75D6B5C75934F5E4FB5788BB3BB63639B1B5D7D0FDF552085830C1300341D`;
- accepted Campaign-B persistence-region SHA-256: `F5375013643DD5E8D3C31D44836BE1B2FB06844A9C99D751CCAB594E61922233`;
- corruption matrix — **ACCEPTED 7/7** by `stm32_os_asset_gate5_corruption_matrix_contract_oracle_v1_20260925_203206.evidence.zip`, SHA-256 `51BA4472AE94C1ADDFE552AEF245E8CF7DC39FDE6C855C2377455CBD74838255`;
- the accepted matrix independently proved all seven deterministic corrupt-state fallbacks, exact compiled-default runtime and frozen status bar, Linux/libusb management health, zero boot repair mutation by exact pre-reset/post-boot persistence byte equality, and exact firmware bytes outside pages 62/63. Case 7 additionally proved conflicting equal-generation valid A/B records resolve to no record/default as required;
- accepted matrix preflight first proved the previously uncertain post-v2 live target state was already canonical CLEAN, then consumed `16` deliberate persistence-page erases (14 for seven cases + 2 final CLEAN), bringing the Gate-5 cumulative counter to **40/64**;
- final matrix recovery is **CLEAN_STATE_POST_RESET_BYTE_EXACT**: whole-Flash SHA-256 before and after reset is `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957`, persistence pages are erased, Asset status is generation `0`/no-record, runtime is exact compiled default, and scheduler/IWDG/USB/fault health passes;
- historical failed corruption-matrix attempts remain diagnostic history only: v3 restore-state/obsolete Windows-USB composition, cross-host v1 PowerShell argument-cardinality, and cross-host v2 semantic-oracle revision misuse are superseded by the accepted contract-oracle evidence above;
- historical Gate-5 final substep was the physical target USB/VBUS power-removal/power-return retention proof; it is now accepted by the v3 evidence below;
- first retention package `stm32_os_asset_gate5_physical_retention_v1_20260925.zip` was blocked by Microsoft Defender AMSI while `apply.ps1` was being parsed, before any harness code or target I/O executed. Defender Operational events `1116/1117` identify `HackTool:PowerShell/ApexToolkit.A`, Threat ID `2147749462`, detection source `AMSI`, process `pwsh.exe`, action `Quarantine`, action success `True`, security-intelligence version `1.459.398.0`, engine `1.1.26080.3`. This is `HARNESS-AMSI-DELIVERY-01`: zero target mutation, zero additional persistence erases. The blocked package is frozen; no Defender exclusion, AMSI bypass, obfuscation or signature-avoidance rewrite is authorized;
- replacement `.NET` retention run `stm32_os_asset_gate5_physical_retention_dotnet_v1_20260925_222446.evidence.zip`, SHA-256 `A496C86EDDE741358C32B12968172E8FF978AF122BC3A3329EEE18C8415915D0`, proved the executable/AMSI architecture works and reached the physical-power phase. It committed generation `1`, then timed out waiting for USB disappearance: 81 fresh remote helper processes each created a new Linux libusb discovery context and all reported exactly one Deus device at stable locator `usb:001:8`; therefore no actual Linux USB data-path disappearance was observed. Classification `ENVIRONMENT / USB_PRESENCE_TIMEOUT`, not PRODUCT. Failure cleanup restored `CLEAN_STATE_POST_RESET_BYTE_EXACT`;
- that failed retention run consumed `+3` persistence-page erases (`+1` retention commit, `+2` cleanup), so cumulative Gate-5 wear reached **`43/64`** at that checkpoint. The subsequent composition sequence introduced the required zero-write physical-path rehearsal with operator-synchronized USB removal, fresh Linux libusb absence, SWD target-unavailable corroboration, USB/SWD return and exact CLEAN re-attestation before the retained commit; the accepted v3 run below satisfied that requirement;
- `stm32_os_asset_gate5_physical_retention_dotnet_v2_20260925.zip` failed safely during `DOTNET_BUILD_RELEASE` before the .NET engine started. Root cause is `HARNESS-DOTNET-COMPILE-01`: six newly added failed-v1 evidence regexes used `\s` inside ordinary C# string literals, producing Roslyn invalid-escape compile errors. Bootstrap ordering proved zero SSH/ST-LINK/USB/UART/target I/O and zero persistence erases; cumulative wear therefore remained **`43/64`** at that checkpoint. Full-source audit found exactly those six invalid normal-string escapes and no others;
- physical retention — **ACCEPTED** by `stm32_os_asset_gate5_physical_retention_dotnet_v3_20260925_235521.evidence.zip`, SHA-256 `239735C43309870DC820ED0B3FF2C6D0E331BF805FE9B1BCA2B68ED685A249A7`. The zero-write rehearsal proved real Mac-mini Linux USB disappearance plus independent SWD target loss, then exact CLEAN return. The retained test committed generation `1`, payload `01100A6016000000`, CRC `838D1F51`; after one real VBUS removal/return the same generation/payload/runtime returned and the entire 64-KiB Flash plus 2-KiB persistence region were byte-identical pre/post power cycle. Final persistence-only cleanup restored canonical CLEAN whole-Flash SHA `15061F971CFCF48A9EAA8F0AB8C6E630D14A6428B96E19D576C02C5E74859957` before and after reset;
- accepted retention evidence integrity: ZIP CRC clean, `412/412` evidence hashes exact, all 24 source locks exact, repository pre/post status identical, real index empty, no commit/push/mass erase/option-byte mutation. Of 121 bounded native processes, the only two nonzero/timeouts are the intentional SWD power-off probes during rehearsal and retained power removal;
- the accepted retention run consumed `+3` persistence-page erases (`+1` retained commit + `+2` final CLEAN), bringing the final Gate-5 wear counter to **`46/64`**. This is within the frozen `<=64` campaign budget;
- **GATE 5 = ACCEPTED**: Campaign A/B, FI-9 response-loss idempotency, 7/7 corruption matrix, one physical retention power cycle, wear budget and known-good recovery are all satisfied. Final Gate-5 target state was `CLEAN_STATE_POST_RESET_BYTE_EXACT`. Gate 6 then performed the single capability-publication activation described below;
- Gate-6 harness v2 reached clean locked restore/build but failed before target I/O because its `git status --porcelain=v1` parser trimmed the fixed status prefix before slicing the path, turning tracked names such as `README.md` into `EADME.md`; its operator/evidence presentation also violated the canonical terminal-structure contract. This is diagnostic HARNESS history only and consumed zero persistence erases;
- Gate-6 harness v3 corrected the porcelain/presentation lifecycle and produced trustworthy classified evidence, but falsely reported `BUILD_TOKEN: BUILD_FLASH` after a successful firmware build because its multiline regex was CRLF-sensitive. Evidence `stm32_os_asset_gate6_publication_activation_smoke_v3_20260926_120207.evidence.zip`, SHA-256 `4C58503D25033BFCEAAE973746489908E0011B21A52EED00F784CDAD21DC2862`, proves `BUILD_OUTCOME=PASS`, Flash `54268`, SRAM `11944`, no target I/O and unchanged wear `46/64`; failure class `HARNESS-LINE-ENDINGS-PARSER-01`;
- Gate-6 v4 replayed the corrected normalized/cardinality parser against the exact v3 raw build stdout before execution, then passed the complete activation smoke. This v4 result supersedes the failed Gate-6 harness attempts.

The current bench is split-host: target native USB/power is owned by Ubuntu on the Mac mini and exercised through Linux libusb via `deus@macmini`; ST-LINK/SWD and CH340/UART are owned by Windows. The canonical operational contract is `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.

The Asset Flash erase/program implementation and 54-KiB linker boundary are active in the published Asset boundary. Application reset origin remains `0x08000000`; bootloader relocation/VTOR migration is still deferred. Gate 5 accepted the implementation with system capability bit 5 intentionally off. Gate 6 added the single publication-activation source change `SYSTEM_IDENTITY_CAP_ASSET_CONFIGURATION_TRANSFER`, changing the published system capability mask from `0x0000001F` to `0x0000003F`; that activation is now hardware-accepted by `stm32_os_asset_gate6_publication_activation_smoke_v4_20260926_134106.evidence.zip`, SHA-256 `7A65CA036E29FE06E39C5ADA2B70461292DBFD3DC584D28C3563A7E498322C60`. Accepted activation firmware tree `12f0a0ffaa4597d9ada8b78ecee324d77db79d84`, host tree `136687e80c42bd8104ad6c37fbbccb915b60fd08`, BIN SHA-256 `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`, whole-Flash SHA-256 `CD31D49985753E08F3AE123F0AF7BF136AC510AA2D40E3D5CB4FDF5EBEF9D737`. Fresh build/tests pass at Flash `54268/54272` (4-byte headroom) and SRAM `11944/12288`; production `PublishedOnly` `config status/read`, ping8 and health pass on Mac-mini Linux libusb. Persistence remains erased and Gate-5 wear remains `46/64`. Generic storage/filesystem/package infrastructure remains unauthorized.

Canonical Asset design:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_PLAN.md`

Canonical Asset acceptance:

`docs/ASSET_CONFIGURATION_TRANSFER_FOUNDATION_ACCEPTANCE_PLAN.md`

The broader configurable OLED layout/preset/assets roadmap remains deferred. Only `OLED_UI_LAYOUT_CONFIG_V1` has been promoted as the concrete persistence consumer.

Historical repository-readiness prerequisites that supported the reopened Asset Gate 0:

1. repository-owned firmware build entrypoint — **ACCEPTED**: the exact historical Host Control Gate-2 compiler/startup/link/objcopy invocation was recovered from acceptance evidence rather than guessed, the generated `deus_build_identity.h` was recovered identically from 20/20 surviving copies (SHA-256 `F6EAA98172FEE67338BFD978F208BD95731063A1160B0C9C1282306A5D6658A5`), and two independent temporary builds with Arm GNU Toolchain `15.3.1` reproduced the accepted 50652-byte firmware BIN byte-for-byte at SHA-256 `FB68993FC998DE77B61FAC9F4949E4E124B95FF867BB456EBA401C9F2709F13F` with exact `text/data/bss = 50540/112/11616` and Flash/SRAM `50652/11728`; the versioned entrypoint is `scripts/build_firmware.ps1` (accepted script SHA-256 `BC7B91825B9BB80394C28643848EE3AEF75A4B62017432BB1070190088B8CDFB`), final reproducibility evidence ZIP SHA-256 `1FDB3C647ACCF4B9A67FDC02514009495F60D1F3E3B4E00C551FC0B4A3F4F847`; ELF/MAP debug artifacts remain path-dependent and are not the firmware identity; no target I/O or Flash mutation occurred;
2. read-only real-device MCU/Flash preflight — **ACCEPTED** on the current physical board: `DEV_ID=0x410`, numeric `REV_ID=0x2003`, factory Flash size `64 KiB`, `FLASH_OBR=0x000003FC`, `FLASH_WRPR=0xFFFFFFFF`, RDP disabled, WRP0..31 inactive, SWD accepted at 950 kHz; immutable source-log SHA-256 `B112A754E82ECC01BDC509B2D8FB359D652D74DDC565B3DE046C30C181672C13`, finalized evidence ZIP SHA-256 `BB1F928A2E716F2C8D0FA6B160FC7F41B3187A75CF869DD139DF0C450D6B42D8`;
3. host package-lock/SDK restore reproducibility — **ACCEPTED**: seven project-local `packages.lock.json` files are now the reviewed dependency graph; `host/global.json` remains the accepted `.NET 10` policy (`10.0.100`, `rollForward=latestFeature`, prerelease disabled), selecting Windows SDK `10.0.201` and Linux SDK `10.0.112`; locked restore, Release build and direct Core/Transport tests pass on both Windows (`21/21`, `5/5`) and Ubuntu/Linux (`21/21`, `5/5`), WSL proves the DEUS path and `D:` path are the same checkout, and all seven normalized lock hashes are identical across Windows/Linux. Final acceptance evidence ZIP SHA-256 `CE7E3BB09A24913D5374A875CCE49556A2A186F114E36128E42AD3BB26F42165`; no target I/O, Flash mutation, reset or reconnect occurred.

These were infrastructure prerequisites for the now-published Asset boundary; they are retained here as acceptance provenance.

## 3. Planned product sequencing

The shared docs-only Flash ownership decision is now frozen by `docs/FLASH_OWNERSHIP_LAYOUT_DECISION.md`:

- steady-state bootloader/recovery: pages 0..7, `0x08000000..0x08001FFF`, 8 KiB ceiling;
- steady-state relocated application: pages 8..61, origin `0x08002000`, 54 KiB maximum;
- persistent slot A/B: pages 62/63 at `0x0800F800` / `0x0800FC00`, 1 KiB each;
- historical sequence: Asset/Configuration was first implemented in a transitional standalone phase with the application still reset owner at `0x08000000` and persistence on pages 62/63;
- current accepted firmware-update architecture has completed the later relocation: bootloader owns pages 0..7 and reset at `0x08000000`, application owns pages 8..59 with vectors/VTOR at `0x08002000`, update metadata owns pages 60/61, and persistence remains pages 62/63.

The historical Asset reactivation condition was satisfied by `OLED_UI_LAYOUT_CONFIG_V1`; the full Asset/Configuration boundary is now accepted and published at `562e786ffa734da055c23144ec4256bc8961bbaf`.

Forward dependency order is:

1. `FLASH_OWNERSHIP_LAYOUT_DECISION_V1` — **ACCEPTED DOCS-ONLY SHARED CONTRACT**;
2. `OLED_UI_LAYOUT_CONFIG_V1` — **PROMOTED / FROZEN CONSUMER CONTRACT**;
3. `ASSET_CONFIGURATION_TRANSFER_FOUNDATION` — **GATES 0–7 ACCEPTED / PUBLISHED `562e786ffa734da055c23144ec4256bc8961bbaf`**;
4. `PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY` — **GATES 0–7 ACCEPTED / PUBLISHED `a8f92f83c2ba8917ad183b1a099c9e21199c9463`**;
5. `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION` — **GATES 0–6 ACCEPTED LOCALLY / GATE 7 ORDINARY NON-FORCE PUBLICATION CURRENT**;
6. networking/service/security extensions.

The pre-Bootloader recovery boundary is deliberately behavior-preserving. It does not
implicitly authorize firmware-update logic, linker relocation or public-ABI removal.

Firmware update requires separate authenticity/security, controlled reboot/update handoff and recoverable Flash transaction design. Networking remains a later service boundary and does not authorize speculative TCP/IP expansion on the STM32F103.

Canonical sequence:

`docs/ROADMAP.md`

## 4. Completed/published foundation sequence

The following major foundations are complete/published and are **not active work**:

- scheduler timed blocking;
- scheduler fixed priority;
- production heartbeat task;
- IWDG production liveness;
- normal-boot production task ownership;
- native USB device core;
- USB CDC ACM diagnostics/console;
- transport-neutral shell/RPC;
- binary framed transport v1;
- OS application/UI model foundation;
- boot/desktop UI foundation;
- OLED dirty-region optimization;
- application runtime foundation;
- kernel composition-root decomposition;
- USB management device foundation;
- Host control application foundation;
- Asset / Configuration transfer foundation.

Their `*_PLAN.md` and `*_ACCEPTANCE_PLAN.md` files are retained as immutable scoped design/proof records, not as competing global current-state documents.

## 5. Deferred / trigger-driven work

The following items exist as possible future work but are **not active roadmap boundaries unless explicitly promoted**:

### Performance and kernel/runtime

- I2C async/IRQ/DMA after measured bus pressure;
- scheduler ready-set acceleration after measured task-count/CPU pressure;
- CRC acceleration after measured streaming cost;
- bounded minimal-copy/backpressure refinements for transfer/update streaming;
- tickless/low-power after a real power requirement;
- generic timers/queues/synchronization/runtime statistics only for real consumers;
- further composition-root convergence when ownership pressure warrants it.

### Robustness and observability

- previous-boot crash/reset retention;
- bounded structured binary event tracing;
- soak/fault-injection campaigns;
- explicit application stop-failure semantics before resource-owning apps require them.

### UI/configuration ideas

- alternate OLED layout presets;
- runtime custom layout editing;
- PC layout configurator/import;
- persisted UI layout only through an accepted persistence contract;
- RTC wall-clock;
- notification rendering/queue semantics;
- optional kernel-log presentation.

`docs/OLED_UI_LAYOUT_PLAN.md` and `docs/OLED_STATUS_BAR_PLAN.md` are deferred future-consumer designs, not active implementation plans.

### Host management presentation direction

- `DeusOs.Control.Cli` is the canonical first-class management surface for automation, diagnostics, acceptance and headless Linux operation;
- the existing Avalonia Desktop remains an optional workstation frontend rather than a Linux-server dependency;
- a future Web UI should sit above the same `DeusOs.Control.Core` / bounded host-management service API rather than implement the STM32 binary protocol independently in the browser;
- a local Web/service surface does not imply STM32 networking and may be promoted separately only when a concrete host-management consumer requires it;
- LAN/Wi-Fi/remote exposure remains a later networking/security boundary and requires explicit authentication/authorization/trust design before activation.

Forward presentation reference: `docs/HOST_MANAGEMENT_PRESENTATION_MODEL.md`.

This direction is **not an active implementation boundary**.

### Storage/platform expansion

- general filesystem;
- external storage/block-device layers without a concrete consumer;
- MPU/user-kernel isolation;
- broad power-management framework;
- stable physical unit identity/MCU UID until a real multi-unit/privacy requirement exists.

Canonical deferred policy:

`docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`

## 6. Known architecture state

The initial Asset/Configuration Gate 0 review found no substrate blocker and was historically deferred because no concrete consumer with a frozen persistence schema existed. That blocker was subsequently resolved by promoting `OLED_UI_LAYOUT_CONFIG_V1`; implementation and hardware acceptance have since advanced to Gate 5.

Resolved gaps:

- semantic application events are separate from scheduler wake bits;
- system identity/capability discovery is provided through `sysinfo`;
- USB management and host transport are separated from CDC diagnostics;
- Windows/Linux host control share one protocol/Core model;
- kernel composition-root risk was materially reduced without introducing a god context/service locator.

Still intentionally unresolved until a consumer requires them:

- stable physical unit identity;
- live physical “UART peer connected” semantics;
- product-level ST-LINK attachment state;
- general filesystem;
- network mutation security;
- richer observability/runtime-statistics framework.

Firmware-update authenticity is resolved and accepted inside `FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`: device-side HMAC authentication, target/product binding, version-floor rollback rejection and fail-closed recovery are covered by the accepted Gate-5 matrix. The remaining items above stay consumer- or boundary-specific future concerns and are not implicitly activated by the Bootloader boundary.

## 7. Source-of-truth map

- live repository bytes/commit/tree/branch/cleanliness: **Git**
- global current project/product state: **this file**
- stable architecture/invariants: `docs/ARCHITECTURE.md`
- development/acceptance host and physical-interface topology: `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`
- forward ordering: `docs/ROADMAP.md`
- active/completed boundary design contract: matching `*_PLAN.md`
- boundary proof/acceptance contract: matching `*_ACCEPTANCE_PLAN.md`
- actual acceptance proof: accepted evidence/log artifacts
- deferred improvements: `docs/DEFERRED_OPTIMIZATION_ROBUSTNESS_BACKLOG.md`
- historical change chronology: `CHANGELOG.md`
- execution history/checklists: `docs/MASTER_EXECUTION_CHECKLIST.md`
- operator/handoff reference: `docs/PROJECT_HANDOFF.md`
- historical umbrella implementation notes: `docs/IMPLEMENTATION_PLAN.md`

If another document disagrees with this file about **which boundary is current or what work is next**, this file wins. If this file disagrees with Git about repository identity/bytes, Git wins.
