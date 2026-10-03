# Deus OS — Firmware Update / Bootloader Foundation Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED / POST-PUBLICATION PHYSICAL DEPLOYMENT VERIFIED**

Boundary: FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION

Canonical wire/image protocol: `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`.

Published prerequisite:
PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY — Gates 0–7 accepted/published at commit a8f92f83c2ba8917ad183b1a099c9e21199c9463, tree 013c1f472eb9404de94befdcc3f1e1e5acfc5831.

## 1. Purpose

This boundary introduces the minimum recoverable firmware-update owner for the accepted STM32F103C8 64-KiB target. It is not a second operating system and does not authorize a filesystem, package manager, dynamic loader, networking stack, UI subsystem or duplicate Host Control surface.

Gate 0 has two ordered proof steps:

1. Gate 0A — contract/source-boundary freeze: collect already-published Flash, runtime, transport, host-ownership and recovery prerequisites into one canonical design, freeze unresolved decisions, and forbid product mutation.
2. Gate 0B — static linked-size feasibility: build acceptance-only prototypes outside the product source boundary and prove that the selected minimum bootloader responsibility composition fits the frozen 8-KiB bootloader ceiling.

Gate 0A is accepted from `stm32_os_bootloader_gate0a_contract_source_audit_v1_20260927_200959.evidence.zip`, SHA-256 `BB3D9210639564D27E0E9FFF57817A10E619991EC3A1960D4D5D29E82E122002`. It proved the published recovery binding, exact docs-only WIP, current linker/startup/Flash/recovery/host/capability ownership and zero product/target mutation. Its ten-entry open-decision matrix has now been resolved in this contract and `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`; Gate 0B may not start until the docs/read-only decision-freeze consistency audit accepts those exact bytes.

Gate 0B is accepted from `stm32_os_bootloader_gate0b_linked_feasibility_recomposed_v4_20260927_225243.evidence.zip`, SHA-256 `852DF5AB29AB850FDBB250FD582C2C1D59B0052790E1CDAF8759A5E50C05F780`. ARM GNU 15.3.1 linked the complete acceptance-only bootloader at Flash `5756/8192`, conventional SRAM `1684/2048`, max frame `248/256`, longest linked stack path `648/768` with MSP margin `376`; undefined symbols `0`, heap absent. The exact current application also relocates acceptance-only to `0x08002000/52K` at Flash/SRAM `52932/10920`, end `0x0800EEC4`, leaving `316` bytes executable Flash margin and `344` bytes SRAM margin. Gate 0B additionally reconciles the corrected STM32F1 FPEC metadata rule: `0xA55A` is written once from erased `0xFFFF`; no `0xA500` in-place retirement exists. No target, product-source, Git-index, commit or push mutation occurred.

Gate 0 is therefore complete. Gate 1 is now authorized only for the behavior-preserving Host Core ownership split and exact product source authorization. Firmware linker/startup relocation, bootloader installation and target mutation remain forbidden until their later gates.

### Current acceptance disposition after Gate 5

The implementation subsequently completed Gates 1–4 and the complete Gate-5 hardware matrix. Gate 5 now has accepted proof for normal boot, explicit update entry, invalid-application recovery, malformed header, wrong target, bad digest, bad authenticator, rollback/version rejection, deterministic reset interruption, physical Mac-mini USB/VBUS power interruption, full retry from zero, exact application/metadata/bootloader/persistence ownership, deterministic handoff/VTOR, and the applicable Linux/libusb hardware path.

The final hardware state was returned to the accepted repair-candidate whole-Flash SHA-256 `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`. Stage-10 formal continuation SHA-256 is `E6EB019B021AB95DB16CA3B95E1C3F7C2C7F44D862ABF2711F6B42089F35D560`.

A post-Stage-10 host reliability audit then accepted a host-only repair for delayed timed-out response correlation and Windows WinUSB timeout classification. Validation evidence SHA-256 `61D8AEBCA99E712D893474376CA4FEDD63D17D11021E45C0F7701621C712B108` passed Core `39/39`, Transport `12/12`, all Core/Windows/Linux Release builds, exact source/poststate locks and zero target/remote/Git mutation. Formal repair continuation SHA-256 is `253026C36C346C579AD5C1CE0FD528A7F7943A07CC21D792171C14C3350797FD`.

No second firmware-side response-loss mechanism is claimed from the historical INFO/BEGIN/DATA timeout observations and no speculative bootloader patch is authorized. Gate 6 reconciled the canonical documentation, activated `SYSTEM_IDENTITY_CAP_FIRMWARE_UPDATE` (`SYSTEM_IDENTITY_CAPABILITIES = 0x0000007F`) and passed build/test/source-consistency validation. Gate-6 validation evidence SHA-256 `104A93DDCED3F6B86B41E6476DF5D47CE6FD532216DC4BD14FA848FE3FD44FD7` passed `92/92` evidence hashes, firmware tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, host tree `c95019bedfb6223705e6eba4f4c6d310b1701cdc`, firmware BIN `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, Flash/SRAM `53212/10956`, Core `41/41`, Transport `12/12`, all host Release builds and zero target/remote/real-index/ref mutation. Gate 6 was accepted by commit `27fb10288ef45dcc9292287603e5ab8a26bf1fcb`; Gate 7 then published that commit/tree `0eca476d84eb1f06b633a7780b7883a3fadd8adb` by ordinary non-force fast-forward. Gate-7 publication evidence SHA-256 is `B59E3628731AB78143A5E4B4918AFEFA60CC448C4C6065EB190697A4A0480F95`. Gate 6/7 performed zero target I/O, so the board initially remained on the Stage-10 restored baseline `5033A8FDE3F1962E0AA63F8343E6761730628ECE12AA0BCD3AE12CB2001283CA`.

That publication/physical-state gap is now closed. Post-publication deployment evidence SHA-256 `77F42EE22978A52FC60AE03D14D10BAC27D38B9FE8E6647D095F646349D75706` proves exact signed package `8BD8952AB994011438B55EF56DE6FBEDE3264F375DE97CF6BAABD1AA4D568C7D`, exact published application `2CB6423F9E8752772256BCDDEBB116EEE5C907CED94911EEF4D5AC5CBF6C64BA`, successful `Ok/Committed` END with version floor/committed version `2/2`, committed metadata B version `2` / marker `0xA55A`, unchanged bootloader and persistence, runtime source tree `8323c68c931894441ae4db9138ba3838f35bb8b6`, capability `0x0000007F`, PONG/health PASS, and no rollback. Current whole-Flash SHA-256 is `FB85D953CAC213DCE3662FA8F2008D42E89AE11A8667FD77E6B3F300958440D6`. The earlier post-publication END-timeout investigation remains historical: its old version-1 Flash observation was caused by an executed post-failure restore, not by proof that the published package had never transferred or committed.

## 2. Published baseline entering Gate 0

- published commit: a8f92f83c2ba8917ad183b1a099c9e21199c9463
- published tree: 013c1f472eb9404de94befdcc3f1e1e5acfc5831
- recovery firmware source tree: a10182e7d0659b9b161073ad49a8816ecb6e7918
- BIN SHA-256: FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2
- accepted Flash/SRAM: 52908/10920
- task0/task1 margins: 304/432
- MSP margin: 1592
- capability mask: 0x0000003F; firmware-update bit 6 remains clear

The pre-Bootloader recovery exists precisely so this boundary does not begin from the prior 4-byte Flash / 344-byte SRAM margin.

## 3. Frozen physical Flash ownership

Authoritative steady-state map:

    0x08000000..0x08001FFF  bootloader/recovery       8 KiB  pages 0..7
    0x08002000..0x0800F7FF  relocated application    54 KiB pages 8..61
    0x0800F800..0x0800FBFF  persistence slot A        1 KiB page 62
    0x0800FC00..0x0800FFFF  persistence slot B        1 KiB page 63

Hard rules:

- bootloader/recovery loadable image <= 8192 bytes;
- application physical region remains exactly 55296 bytes;
- application origin changes to 0x08002000 only in this boundary;
- persistence pages 62/63 do not move and are never implicit update staging;
- application update may erase/program only pages 8..61;
- bootloader self-update is outside this foundation unless explicitly reopened.

The accepted forward application discipline remains Flash <=53248, SRAM <=11264, task margins >=256/256 and MSP margin >=1024 unless Gate 0 explicitly reopens a ceiling.

Resource-freeze sequencing rule: a gate must not freeze a forward application ceiling using only the pre-feature image when the next authorized gate must add mandatory resident application code. Before freezing such a ceiling, either link the mandatory resident path in the feasibility prototype or reserve and document an explicit byte budget for it. This foundation violated that sequencing once: accepted Gate-2 Flash was `52932/53248` (316-byte margin), while the first complete Gate-3 runtime-entry implementation increased linked application load image by 400 bytes. Subsequent Gate-3 recovery must therefore be measured against exact linked deltas, and future foundations must not treat pre-feature headroom as proof of post-feature feasibility.

## 4. Reset/vector ownership migration

Current accepted application owns vectors at 0x08000000. Bootloader steady state requires:

- bootloader vector table/reset owner at 0x08000000;
- application vector table/link origin at 0x08002000;
- application SCB_VTOR = 0x08002000 before interrupt-dependent runtime;
- application MSP loaded from vector[0];
- application reset handler taken from vector[1];
- invalid application never receives control.

Application structural checks must include SRAM-range/alignment for MSP, Thumb reset handler and handler address inside the application region. The bootloader must quiesce its interrupt/peripheral state before handoff.

No image linked for 0x08000000 is bootloader-layout compatible.

## 5. Recovery/update state model

The bootloader must remain executable when the application is absent, invalid, partially programmed, rejected by trust policy or structurally unbootable.

The frozen map has no spare room for a second complete 54-KiB application image. Gate 0 must therefore choose explicitly:

A. recoverable in-place update: stream/program pages 8..61 while bootloader pages remain intact; interruption may invalidate the application, after which bootloader recovery accepts a complete retry; or
B. reopen the Flash/storage decision if full-image staging or executable A/B is required.

Silent use of pages 62/63, another owner’s budget or undocumented external storage is forbidden.

Gate 0 must freeze the exact application-valid/commit rule so partially programmed bytes cannot become bootable.

## 6. Update entry — decision required

Gate 0 must select the exact application-requested reboot/update entry mechanism and define behavior across software reset, IWDG reset, power loss, invalid application and cold boot.

A transient RAM mailbox, retained register or other mechanism may be evaluated, but ownership, lifetime and false-positive behavior must be proven. Invalid/missing application must always provide independent recovery entry.

## 7. Product update transport

Published runtime management remains WinUSB on Windows and libusb on Linux through management IF2. CDC remains secondary diagnostics; UART remains emergency/development diagnostics.

### 7.1 Current bench wiring / pin map

Execution-host and cable ownership is canonical in `docs/DEVELOPMENT_ENVIRONMENT_TOPOLOGY.md`.
This boundary additionally freezes the target-side wiring assumptions that Gate 5 must
preserve and re-prove before destructive hardware acceptance:

| Function | STM32F103 pin(s) | Current bench owner / connection | Bootloader rule |
| --- | --- | --- | --- |
| Native USB FS | PA11 `USB_DM`, PA12 `USB_DP` | USB data + normal VBUS/power are physically attached to Ubuntu on the Mac mini | runtime and bootloader may own USB only through the accepted USB device contract; do not repurpose PA11/PA12 |
| Emergency UART / CH340 | PA9 `USART1_TX`, PA10 `USART1_RX` | CH340 on Windows, 115200 8N1; adapter VCC disconnected while USB powers target | update transport must not silently move to UART; PA9/PA10 remain diagnostics |
| OLED / I2C1 | PB6 `I2C1_SCL`, PB7 `I2C1_SDA` | SSD1306-class 128x32 display | bootloader does not own OLED/I2C and must not reconfigure PB6/PB7 |
| Heartbeat LED | PC13 | on-board active-low LED | bootloader does not rely on PC13 for update correctness |
| SWD debug/recovery | PA13 `SWDIO`, PA14 `SWCLK` | ST-LINK/SWD on Windows; shared ground; ST-LINK 3.3 V disconnected while USB powers target | ST-LINK remains recovery/debug authority, not the normal product update carrier |

Power discipline is part of the map, not an operator convention:

- normal target power comes from the Mac-mini USB VBUS connection;
- ST-LINK supplies SWD signals/ground only in the normal bench topology, not a competing 3.3 V rail;
- CH340 supplies TX/RX/ground only; adapter VCC remains disconnected;
- a physical target power cycle therefore belongs to the Mac-mini USB domain;
- a Windows-driven SWD reset is not by itself a complete USB recovery proof.

Gate 3 is non-target and must not probe or mutate these interfaces. Gate 5 must record the
execution domain for every USB/UART/SWD operation and reject any harness that looks for
the current target USB device on Windows without an explicit rewiring/re-proof of topology.

CDC is not the default Bootloader update carrier.

Gate 0B first evaluates the smallest dedicated vendor/bulk USB recovery/update transport compatible with the existing host-management model and the 8-KiB ceiling. The bootloader does not need to reproduce the runtime composite device, CDC interfaces or application RPC registry.

If USB + recovery + security cannot fit, Gate 0 reports that result and may evaluate an explicitly documented fallback. It may not silently substitute UART or ST-LINK as the product update feature. ST-LINK remains operator/debug recovery authority.

## 8. Executable image trust contract

CRC or an unkeyed digest provides integrity but not authenticity.

Before implementation Gate 0 must freeze:

- threat model;
- device-enforced authenticity mechanism;
- trust/public material ownership and update policy;
- exact authenticated bytes;
- integrity digest;
- target/product binding;
- image origin and length bounds;
- image-format ABI/version;
- firmware version / rollback policy;
- malformed/unknown-field rejection rules.

Current RDP is disabled and SWD is available. The initial authenticity claim may protect against unauthorized images delivered through the supported update transport, but must not claim resistance to a physical attacker able to rewrite Flash through unrestricted debug access. Any RDP/debug-lock policy is a separate explicit security/recovery decision.

Gate 0B must measure the selected real verifier; a CRC-only placeholder is invalid.

## 9. Image/validity metadata ownership

Gate 0 freezes the exact location and format of all validity/update metadata.

Forbidden implicit locations:

- persistence pages 62/63;
- undocumented bytes beyond the application region;
- stale RAM as durable validity state;
- metadata that marks a partial image bootable before complete verification.

Metadata may live inside bootloader ownership or application ownership only when budget, erase semantics and atomic validity behavior are explicitly accepted.

## 10. Application relocation contract

The repository linker currently uses ORIGIN 0x08000000, LENGTH 54K. src/startup.s does not establish relocated application VTOR.

Implementation cannot begin until Gate 0 freezes the authorized relocation source set, including at least the application linker, early startup/VTOR establishment, build/image assertions and bootloader vector/reset/handoff owner.

Relocation must not move persistence or expand the 54-KiB application physical region.

## 11. Host ownership trigger is now active

The deferred Host Core decomposition trigger is now satisfied.

DeusDeviceClient.cs already combines session negotiation, frame correlation, generic RPC, application facade and Asset transaction orchestration. Update orchestration must not be added directly to that monolith.

Before update-client behavior, this boundary performs a behavior-preserving host-only ownership split that:

- retains one shared session/channel;
- retains one framing/correlation/reconnect authority;
- separates generic RPC, Asset transactions and update transactions into coherent owners;
- does not duplicate decoders, locks or reconnect state;
- preserves existing behavior/tests.

This is a prerequisite substep inside this Bootloader boundary, not another product boundary inserted into the roadmap.

## 12. Capability publication

SYSTEM_IDENTITY_CAP_FIRMWARE_UPDATE is reserved as bit 6.

It remained clear throughout implementation/destructive acceptance. After complete Gate-5 hardware/reliability acceptance and Gate-6 documentation reconciliation, Gate 6 activates the bit in the publication candidate; that candidate must pass build/test/source-consistency validation before the Gate-6 acceptance commit.

## 13. Gate 0A source audit

Gate 0A is read-only/docs-only. It inventories:

- linker/startup/vector assumptions;
- Flash erase/program ownership;
- build/recovery scripts that encode standalone 0x08000000 geometry;
- reset/IWDG/reboot controls;
- USB management ownership;
- host session/client ownership;
- system identity capability bit 6;
- exact Asset recovery artifacts that become stale when reset ownership changes.

No linker/startup/product source mutation, target I/O, reset, Flash write, RDP/option-byte change, commit or push is allowed.

## 14. Gate 0B linked-size feasibility

Gate 0B may compile acceptance-only prototype code in a temporary directory, not product source.

The selected prototype must contain the real minimum responsibility set:

- reset/vector owner + normal/recovery decision;
- application structural validation + handoff;
- selected update transport;
- application-page erase/program/verify;
- selected integrity/authenticity verifier;
- selected image/target/version parser;
- recovery behavior sufficient to accept a full retry after interruption.

Acceptance:

- bootloader loadable Flash <=8192 bytes and end <=0x08002000;
- explicit RAM/stack report;
- no heap;
- undefined symbols = 0;
- reviewed compiler helpers only;
- source/object attribution and complete build argv captured.

A skeleton that omits security or transport is not a feasibility proof.

Gate 0B must also perform a temporary application-link experiment at ORIGIN `0x08002000`, executable LENGTH `52K`, with firmware metadata fixed in pages 60/61, and prove no overlap with metadata/persistence, no hidden absolute-address assumptions, and a defined future VTOR owner.

## 15. Gate 0 architecture decision freeze

The ten Gate-0A open rows are frozen as follows. These are architecture decisions only;
they do not authorize product-source or target mutation.

### 15.1 Update entry

Normal runtime entry is management-IF2-only and uses the additive firmware-update
request frame, opcode `ENTER_BOOTLOADER`, with the destructive flag asserted. CDC and
UART do not own product update entry.

The application writes a one-shot backup-domain token:

- `BKP_DR1 = 0xD35A`;
- `BKP_DR2 = 0x2CA5` (bitwise complement);
- then requests Cortex-M `SYSRESETREQ`.

The bootloader reads the pair before normal application handoff and clears both registers
before acting. Exact pair -> explicit recovery/update mode. Any other pair is ignored and
cleared. A missing token never prevents recovery from an invalid/missing application.

The token is allowed to survive system reset. Retention across loss of VDD depends on the
board's VBAT/backup-domain supply and is not required for correctness: after complete
power loss a valid application may boot normally and the host may request update entry
again.

### 15.2 Product update transport

Bootloader v1 uses a deliberately minimal USB FS vendor/bulk device:

- runtime private bench identity remains `VID 0x1209 / PID 0x000C`;
- bootloader uses distinct private-test identity `VID 0x1209 / PID 0x000D` to avoid runtime/bootloader Windows descriptor and interface-binding cache collisions;
- product string: `Deus OS Bootloader`;
- one vendor-specific interface, number 0;
- bulk OUT `0x01`, bulk IN `0x81`, 64-byte maximum packets;
- Microsoft OS 2.0 WinUSB compatible-ID descriptors for Windows;
- direct libusb claim on Linux;
- no CDC, IAD, application RPC registry, OLED, scheduler or composite-device surface.

Production redistribution still requires an independently authorized product VID/PID.

Firmware update reuses the accepted Binary Framed Transport v1 **envelope only**:
`A5 5A`, protocol version 1, request ID and CRC-16/CCITT-FALSE. Every bootloader
firmware-update frame must fit in one 64-byte bulk packet, therefore payload length is
`<=52` bytes. The bootloader implements only the firmware-update request/response
frame family and rejects unrelated runtime frame types.

Reserved frame types:

- `0x04 FIRMWARE_UPDATE_REQUEST`;
- `0x86 FIRMWARE_UPDATE_RESPONSE`.

Request payload byte 0 is the update opcode. v1 opcodes:

- `0x01 ENTER_BOOTLOADER` — runtime management IF2 only;
- `0x02 INFO` — bootloader state/floor/current-version query;
- `0x03 BEGIN` — submit exact 48-byte image header;
- `0x04 AUTHORIZE_HEADER` — submit 32-byte header authenticator;
- `0x05 DATA` — exact sequential application bytes;
- `0x06 END` — finalize and verify the authenticated header digest.

No resumable persistent transfer session is introduced. Exact request/response layouts, DATA offset/idempotency semantics and status codes are canonical in `docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md`.

### 15.3 Executable image envelope

Firmware package v1 is:

```text
48-byte fixed header
32-byte header_auth_tag
image_length raw relocated application bytes
```

All integers are little-endian. Exact 48-byte header:

```text
offset size field
0      4    product_id = 0x534F4544
4      1    image_format_version = 1
5      1    reserved = 0
6      2    target_device_id = 0x0410
8      4    image_length
12     4    firmware_version
16     32   SHA-256 of exactly image_length application bytes
```

Application origin is fixed to `0x08002000` by format v1 and is included explicitly as little-endian bytes in the header HMAC input. This keeps `BEGIN` at `1 + 48 = 49` payload bytes inside the single-packet `<=52` limit.

`image_length` must be `>=8`, `<=53248`, and a multiple of 4. The raw application
image must be linked for `0x08002000`; a standalone `0x08000000` image is rejected.

### 15.4 Authenticity and trust material

v1 device-side authenticity is `HMAC-SHA-256` with one 256-bit update key.

The key:

- is not committed to Git;
- is not emitted into logs/evidence;
- is injected into the bootloader build from an external secret source;
- is present in bootloader Flash and therefore is **not** claimed secure against a
  physical attacker with unrestricted SWD while RDP remains disabled;
- has no in-band rotation in this foundation; key replacement requires an explicitly
  controlled bootloader/recovery operation outside normal application update.

The supported claim is bounded: an actor that can only use the supported update transport
without the key cannot authorize destructive application programming or commit an
unauthenticated executable image.

A future production/security boundary may replace symmetric authentication with an
asymmetric release-signature scheme without changing the application/persistence map.

### 15.5 Exact authenticated bytes

One HMAC authenticates the fixed header, including its payload SHA-256 digest, with a fixed 8-byte domain:

```text
header domain = 44 45 55 53 48 44 52 31
application_origin_le32 = 00 20 00 08

header_auth_tag =
    HMAC-SHA-256(key, header_domain || application_origin_le32 || header[48])
```

The header HMAC authenticates the payload SHA-256 field; the programmed application must hash to that authenticated digest. A second whole-image HMAC is intentionally absent because it would duplicate the same authenticity property while requiring another streaming crypto state. Tag comparisons are constant-time. Header authentication must pass **before any Flash mutation**.

### 15.6 Target/product binding

The accepted v1 target tuple is exact:

- product ID `0x534F4544`;
- STM32 device ID `0x0410`;
- application origin `0x08002000`;
- image format version `1`;
- physical Flash geometry remains the published 64-KiB map.

No physical unit identity is invented. The bootloader checks its compiled target and
device-ID register against the package target before authorizing mutation.

### 15.7 Firmware version and rollback policy

`firmware_version` is an unsigned 32-bit monotonic release number; `0` is invalid.

The bootloader derives a version floor from authenticated firmware-metadata records:

- with a valid committed application, a normal update requires
  `candidate_version > floor`;
- in recovery with no valid committed application, retry/recovery requires
  `candidate_version >= floor`;
- `candidate_version < floor` is always rejected;
- when no authenticated metadata exists, floor is `0`.

This allows retry after an interrupted update while preventing rollback below the highest
previously committed version retained in metadata. Version `0xFFFFFFFF` exhausts the v1
counter and requires a future explicit contract rather than wraparound.

### 15.8 Firmware metadata and atomic validity

The existing 54-KiB physical application ownership is refined without moving any outer
boundary:

```text
0x08002000..0x0800EFFF  executable application payload  52 KiB  pages 8..59
0x0800F000..0x0800F3FF  firmware metadata slot A          1 KiB  page 60
0x0800F400..0x0800F7FF  firmware metadata slot B          1 KiB  page 61
0x0800F800..0x0800FBFF  configuration persistence A       1 KiB  page 62
0x0800FC00..0x0800FFFF  configuration persistence B       1 KiB  page 63
```

Thus the relocated application linker maximum becomes exactly `52K / 53248` bytes,
matching the already accepted forward Flash ceiling.

Each firmware metadata slot stores:

- the exact 48-byte image header;
- the exact 32-byte `header_auth_tag`;
- a 16-bit state marker at offset `0x50`;
- all remaining bytes erased `0xFF`.

State markers:

- erased/uncommitted `0xFFFF`;
- authenticated metadata record present `0xA55A`.

STM32F1 standard Flash programming requires the destination halfword to be erased before programming (apart from the special `0x0000` case), so v1 never attempts an in-place `0xA55A -> other-value` marker transition. `0xA55A` is programmed exactly once from erased `0xFFFF`, and only as the final commit write for a newly prepared metadata page.

An `0xA55A` slot contributes its authenticated `firmware_version` to the rollback floor when its header structure, target tuple and HMAC pass. It is **bootable** only when, in addition, the current application SHA-256 and vectors validate against that header. Therefore an older authenticated slot may remain physically marked `0xA55A` after a later update while being non-bootable because its payload digest no longer matches the current executable image.

If more than one authenticated `0xA55A` slot matches the current application (for example, the same executable bytes published at a higher version), the highest `firmware_version` is authoritative.

### 15.9 Interruption/retry state machine

Update is recoverable in-place; there is no second application image.

Ordered mutation:

1. receive/validate the exact 48-byte header — no mutation;
2. verify `header_auth_tag`, target and version policy — no mutation before PASS;
3. select the metadata target page that does **not** contain the highest authenticated version-floor record (A when neither slot is authenticated); erase only that target metadata page;
4. preserve the other authenticated metadata page byte-for-byte as rollback-floor evidence; do not reprogram its marker;
5. accept exact sequential DATA chunks; on the first chunk for each page 8..59, erase that page, then program aligned halfwords, verify readback and update the payload SHA-256;
6. accept only `byte_offset == expected_offset`, except an exact immediately-previous DATA retry which is acknowledged idempotently without reprogramming or rehashing;
7. on END, verify exact byte count and payload SHA-256 against the digest authenticated by `header_auth_tag`;
8. re-read and structurally validate application MSP/reset vectors;
9. program/verify new metadata header + header auth tag in the inactive slot;
10. program `0xA55A` **last** as the only boot-valid commit transition;
11. reset/handoff through the normal boot decision.

Any reset, USB loss, parse error, Flash error, digest/HMAC failure or vector failure before the final `0xA55A` write leaves no newly committed metadata record. If executable bytes are still unchanged, the preserved older record may remain bootable; once any executable page has been erased/programmed, its old payload digest no longer validates and the bootloader remains in recovery. Retry always restarts from BEGIN and reprograms the complete application; the preserved authenticated metadata record retains the version floor. Persistent partial-transfer resume is intentionally absent.

Pages 62/63 remain byte-for-byte outside this state machine.

### 15.10 Bootloader SRAM/stack ceiling

Gate 0B hard RAM acceptance:

- `.data + .bss <= 1024` bytes, measured from linker-owned section bounds (`_edata-_sdata` plus `_ebss-_sbss`), not from GNU `size` aggregate BSS because that aggregate also includes the NOLOAD `.boot_stack` reservation;
- dedicated bootloader MSP reservation = `1024` bytes, measured independently from `_eboot_stack-_sboot_stack`;
- statically reviewed worst-case stack chain `<=768` bytes;
- required stack margin `>=256` bytes;
- conventional SRAM total `<=2048` bytes, excluding USB PMA peripheral memory;
- heap/dynamic allocation = forbidden.

If the selected USB + SHA/HMAC + Flash/update composition cannot satisfy both the 8-KiB
Flash ceiling and these RAM limits, Gate 0 is blocked and the decision is reopened.

### 15.11 Host ownership split frozen for Gate 1

Gate 1 is behavior-preserving and must preserve the already-existing public reconnect boundary while creating one shared low-level protocol-channel owner rather than duplicate transport state:

- existing `DeusDeviceSession` remains unchanged as discovery/open/recovery lifecycle owner around a `DeusDeviceClient`;
- new internal `DeviceProtocolChannel` owns exactly one `IDeviceTransport`, frame decoder, request-ID allocator, queued frames, serialization gate, common response validation and matching-frame reads;
- new internal `DeusRpcClient` owns generic RPC transaction framing/aggregation over that shared channel;
- new internal `AssetTransferClient` owns Asset transaction semantics and the independent Asset transfer-ID allocator over that same channel;
- `DeusDeviceClient` remains the public compatibility/application facade: negotiation, capability validation, application convenience methods and transport-disconnect state translation only; it must not retain its own transport, decoder, request allocator or serialization gate;
- `FirmwareUpdateClient` is **not** created in the behavior-preserving split. Its ownership slot is reserved for the later update-feature gate after this split is accepted, so Gate 1 introduces no firmware-update feature behavior.

Exact Gate-1 Host Core source boundary:

- modified: `host/src/DeusOs.Control.Core/DeusDeviceClient.cs`;
- new: `host/src/DeusOs.Control.Core/DeviceProtocolChannel.cs`;
- new: `host/src/DeusOs.Control.Core/DeusRpcClient.cs`;
- new: `host/src/DeusOs.Control.Core/AssetTransferClient.cs`.

`DeusDeviceSession.cs`, transports, tests, firmware, linker, startup and build scripts remain source-unchanged. Gate 1 is accepted from `stm32_os_bootloader_gate1_host_ownership_split_acceptance_v3_20260927_233619.evidence.zip`, SHA-256 `D0EE276FB34965AB229E21A53F3A6317159F82EF1AD6B6C1F993132692C7C808`: Core build/test-project builds passed with zero warnings/errors, direct MTP Core tests passed `28/28`, Transport tests passed `5/5`, exact 22-path byte lock and poststate passed, and firmware/linker/startup remained unchanged.

Gate 2 exact product source boundary is:

- modified `linker/stm32f103c8.ld` — application `ORIGIN=0x08002000`, executable `LENGTH=52K`, explicit metadata A/B and persistence boundaries/assertions;
- modified `src/startup.s` — application writes `SCB_VTOR=0x08002000` with `DSB/ISB` at reset entry before C/runtime initialization;
- modified `scripts/build_firmware.ps1` — forward application acceptance ceilings become Flash `53248` and SRAM `11264`, with Bootloader-boundary terminology.

No other firmware/include/host source is authorized in Gate 2. The published Asset recovery scripts remain historical Asset-phase tooling and are forbidden for the relocated Bootloader-boundary image; Gate 4 must create the new immutable recovery bundle/tooling.

Gate 2 is accepted from `stm32_os_bootloader_gate2_relocated_application_build_acceptance_v2_20260928_145108.evidence.zip`, SHA-256 `CF8492497990C799602E53AAD59490E9EBDA399C40EB81BB13A74422002D0531`. The exact firmware-only candidate tree is `637ea07b10cf84882e19cbb8239f31b7f48856a7`; ARM GNU 15.3.1 produced Flash/SRAM `52932/10920`, BIN end `0x0800EEC4`, vector VMA/LMA and `g_pfnVectors` at `0x08002000`, metadata/persistence symbols at the frozen addresses, undefined symbols `0`, stack-usage files `26/26`, and exact poststate. Accepted artifact hashes: BIN `82567F621ED393395810DEB40382EE8477B2975BA1824A407CA9CDDD7B721324`, ELF `DDE555984DA6926CE24C3EAC329E6AB4DDDA4F980E33A05C87258A181036179A`, MAP `0E075E8DC4B3CD64FF89D0B2119FF228CC4834614B15F5006CF3518508DF15B2`.

Gate 3 full non-target implementation acceptance is complete; Gate 4 fresh revalidation + immutable recovery acceptance is complete; Gate 5 hardware acceptance is now current. Gate-2 **relocation owners and invariants** remain frozen inputs to the accepted Gate-3/Gate-4 candidate: `linker/stm32f103c8.ld`, `src/startup.s`, origin `0x08002000`, executable ceiling `52K`, metadata/persistence boundaries and the accepted resource ceilings may not drift. `scripts/build_firmware.ps1` carries only the already-accepted narrow `usb_management.c` size-profile reopen. Gate 4 freshly re-proved the accepted Gate-3 candidate and created the immutable Bootloader-boundary recovery bundle/tooling without changing product semantics. Accepted Gate-4 evidence SHA-256 is `974E482D098973FEE1F56DAA9A767E228A7EA7DBA98D747FCF5E5AD632F905B7`; recovery-tooling tree `77b8824fb3b83f12c7b7fa082907fbad3bdddc34`; immutable sensitive recovery ZIP SHA-256 `00992EC3CF8F31243B322CE2BEF0668CE94889BF47F56DFBEE28EBF14A6AC50E`. Gate 5 must bind and locally validate that exact recovery ZIP before any target mutation.

Gate-3 exact source boundary is **21 paths**:

1. `include/kernel/binary_frame.h` — reserve firmware-update request/response frame types `0x04/0x86` and expose the shared in-place Binary Framed finalizer;
2. `src/kernel/binary_frame.c` — single application-side Binary Framed envelope/CRC finalization authority used by RPC, Asset and Firmware Update response paths;
3. `src/kernel/binary_rpc.c` — reuse the shared Binary Framed finalizer instead of owning a duplicate envelope/CRC writer;
4. `src/kernel/asset_transfer.c` — reuse the shared Binary Framed finalizer while preserving the accepted Asset payload/session ABI;
5. `src/kernel/usb_management.c` — runtime management-IF2 `ENTER_BOOTLOADER`, BKP token and post-response reset owner, with Firmware Update payload construction delegated to the same shared frame finalizer;
6. `bootloader/bootloader.c` — bootloader state machine, USB vendor/bulk transport, Flash writer, SHA/HMAC, metadata/version/recovery/handoff and MS OS 2.0 descriptor set; it is intentionally outside application `src/` so the frozen Gate-2 application source inventory remains exact;
7. `bootloader/startup.s` — bootloader reset/vector owner;
8. `linker/stm32f103c8_bootloader.ld` — bootloader `0x08000000/8K` + SRAM/MSP ceilings;
9. `scripts/build_bootloader.ps1` — external-key bootloader build; secret bytes are temporary and never logged;
10. `scripts/create_firmware_update_package.ps1` — external-key release/operator package signing for `header+tag+image`;
11. `host/src/DeusOs.Control.Core/ProtocolConstants.cs` — host frame-type ABI;
12. `host/src/DeusOs.Control.Core/DeusDeviceClient.cs` — public runtime update-entry facade with published/prepublication policy;
13. `host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs` — package validation and runtime/bootloader update transaction owner over one shared `DeviceProtocolChannel`;
14. `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbDiscovery.cs` — reusable interface-path enumeration primitive;
15. `host/src/DeusOs.Control.Transport.Windows/WindowsWinUsbTransport.cs` — parameterized interface/endpoints while retaining runtime defaults;
16. `host/src/DeusOs.Control.Transport.Windows/WindowsBootloaderWinUsbDiscovery.cs` — bootloader GUID + IF0/01/81 profile;
17. `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbDiscovery.cs` — parameterized libusb topology profile while retaining runtime constants;
18. `host/src/DeusOs.Control.Transport.Linux/LinuxLibUsbTransport.cs` — parameterized interface/endpoints;
19. `host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs` — `1209:000D`, IF0, OUT01/IN81 profile;
20. `host/tests/DeusOs.Control.Core.Tests/FirmwareUpdateTests.cs` — package/framing/update-sequence tests;
21. `host/tests/DeusOs.Control.Transport.Tests/TransportContractTests.cs` — bootloader GUID/topology + non-platform inertness tests.

The expansion from 18 to 21 paths is a measured Gate-3 composition repair. The first exact candidate proved application load-image pressure: canonical `-O2` load image `53340/53248` (over by `92`); an isolated `usb_management.c -O2 -> -Os` measurement reduced that candidate to `53292/53248` (over by `44`). After the shared Binary Framed finalizer repair, the exact canonical image measured `53332/53248` (over by `84`) and the isolated `usb_management.c -Os` variant measured `53276/53248` (over by `28`). The shared-owner refactor therefore removes design duplication but is not sufficient by itself.

Gate 2 is narrowly reopened for one measured compiler-profile owner change only: add `src/kernel/usb_management.c` to the existing repository-owned `$SizeOptimizedSources` profile in `scripts/build_firmware.ps1`. No compiler flag set is changed, no source is removed, and `linker/stm32f103c8.ld`, `src/startup.s`, origin `0x08002000`, the 52-KiB application region, metadata/persistence geometry and acceptance ceilings remain frozen. The Binary Framed finalizer is additionally tightened to a preconditioned internal transport primitive so caller-proven null/payload/capacity bounds are not redundantly rechecked; the checked compatibility encoder retains validation. This is the selected measured resource recovery path and must be re-accepted before composed Gate 3 acceptance.

The bootloader Windows discovery ABI is `{F08907B7-BEC4-5FCF-BC4C-B446ED345D87}` and must appear identically in its MS OS 2.0 `DeviceInterfaceGUIDs` property and `WindowsBootloaderWinUsbDiscovery`. System capability bit 6 remains clear through Gate 4 and throughout destructive Gate-5 acceptance; it is activated only after complete hardware acceptance, documentation and publication readiness. Production update entry defaults to `PublishedOnly`, while Gate-5 acceptance may explicitly use `PrepublicationAcceptance`.

### 15.1 Gate-4 immutable recovery campaign contract

Gate 4 does not reopen the accepted Gate-3 product candidate. The exact accepted Gate-3 full/firmware/host candidate trees remain the product authority. Gate 4 may add exactly two recovery-tooling files outside those product trees: `scripts/create_bootloader_recovery_bundle.ps1` and `scripts/stm32_bootloader_recovery.ps1`. They are owned by a separate recovery-tooling identity and must not change application `DEUS_FIRMWARE_SOURCE_TREE_HEX`.

Gate 4 creates an acceptance-campaign key transiently and uses that same key to build the bootloader and signed package vectors. Raw key bytes are forbidden from repository, logs, evidence and the final recovery bundle. Because the compiled bootloader necessarily contains the verification key, the immutable recovery bundle is a sensitive operational artifact and is SHA-bound by, but not embedded in, the evidence ZIP.

The immutable recovery image covers exactly Flash pages 0..61 (`0x08000000..0x0800F7FF`, 63488 bytes): pages 0..7 contain the accepted bootloader padded with erased bytes; pages 8..59 contain the exact accepted relocated application padded to 52 KiB; page 60 contains authenticated baseline-v1 metadata with header bytes 0..47, tag bytes 48..79 and commit marker `0xA55A` at metadata offset `0x50`; page 61 is erased. Persistence pages 62/63 are never part of this image. The bundle also contains signed `baseline_v1.pkg`, signed `update_v2.pkg`, and a valid-HMAC wrong-target-v2 acceptance vector. This lets Gate 5 exercise success, rollback and target/authenticity paths without retaining the raw signing key.

Recovery has three modes: `VALIDATE_ONLY`, `PRESERVE_PERSISTENCE`, and `CLEAN_STATE`. The two target-mutating modes use Windows ST-LINK/CubeProgrammer, explicit page erase only, no mass erase/read-unprotect/RDP/option-byte mutation, pre-read full Flash, full post-readback, byte-exact pages 0..61 verification, and either byte-exact persistence preservation or erased pages 62/63. CubeProgrammer mode is never implicit: pre-mutation/read-only probes use and verify `mode=HOTPLUG`; authorized erase/program operations use and verify `mode=NORMAL` to halt the core deliberately; post-program readback and final software-reset/release use and verify `mode=HOTPLUG`. Gate 5 must bind the accepted Gate-4 recovery ZIP SHA and pass `VALIDATE_ONLY` before its first target mutation.

## 16. Gate sequence

1. Gate 0A contract/source audit.
2. Gate 0B linked-size/relocation feasibility against the already-frozen architecture decisions.
3. Gate 1 behavior-preserving host ownership split + exact product source boundary.
4. Gate 2 application relocation/build/handoff foundation.
5. Gate 3 bootloader/update transport/security implementation.
6. Gate 4 fresh build/static/host tests + immutable recovery bundle.
7. Gate 5 hardware normal-boot/update/interruption/authenticity/recovery acceptance.
8. Gate 6 docs + one normal local acceptance commit.
9. Gate 7 ordinary non-force publication.

Primitive isolation may split a gate further but may not weaken security, recovery or publication requirements.

## 17. Gate 0 exit

Gate 0 passes only when update entry, transport, authenticity/trust, image validity/metadata and host ownership decisions are frozen and the selected real minimum composition fits 8 KiB.

Fail closed as BLOCKED_8K, BLOCKED_SECURITY_CONTRACT, BLOCKED_STORAGE_MODEL or BLOCKED_HOST_OWNERSHIP rather than silently removing a required responsibility.
