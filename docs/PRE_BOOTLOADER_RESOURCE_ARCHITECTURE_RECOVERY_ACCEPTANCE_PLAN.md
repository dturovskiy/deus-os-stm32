# Deus OS — Pre-Bootloader Resource / Architecture Recovery Acceptance Plan

Status: **GATES 0–7 ACCEPTED / PUBLISHED `a8f92f83c2ba8917ad183b1a099c9e21199c9463` — COMPLETE**

Boundary:

`PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY`

Canonical design:

`docs/PRE_BOOTLOADER_RESOURCE_ARCHITECTURE_RECOVERY_PLAN.md`

Published parent:

`562e786ffa734da055c23144ec4256bc8961bbaf`

## Gate 0 — documentation/source-boundary freeze

Acceptance: **PASS / DOCS + READ-ONLY AUDIT ONLY**.

Frozen facts:

- parent Asset boundary is published;
- accepted Asset activation firmware is `54268/54272` Flash and
  `11944/12288` static SRAM;
- current acceptance headroom is `4` Flash bytes and `344` SRAM bytes;
- physical application region remains 54 KiB; reset/vector relocation is not part of
  this boundary;
- current build already owns section GC and mostly uses `-O2`;
- substantial diagnostic/self-test code remains reachable through the production
  command registry;
- normal production tasks use separately bound `1024/512` stacks while scheduler
  core also reserves a 1-KiB default stack store;
- `src/kernel.c` remains a measured multi-responsibility composition-root hotspot;
- no heap or lower-layer driver/gfx -> kernel dependency inversion was found;
- the Asset implementation itself remained modular and did not materially regrow the
  composition root.

Gate 0 authorizes only Gate 1 build/measurement work. It does not authorize firmware
source mutation, target I/O, Flash mutation, linker relocation, USB-ring resizing,
public ABI removal or Bootloader implementation.

## Gate 1 — build-only measurement matrix

Required evidence:

- exact published baseline rebuild identity or an explicit explanation of any
  debug/path-only ELF/MAP variance;
- BIN exactness where reproducibility contract requires it;
- section/object/symbol size attribution;
- all `.su` stack-usage files;
- controlled per-family `-Os` deltas;
- exact linked contribution of scheduler diagnostics/self-tests and other top symbols;
- resource delta table;
- current USB ring high-water/drop evidence from accepted hardware runs;
- proposed smallest source/profile recovery set.

Target I/O: **NONE**.

No product-source mutation in this gate.

Acceptance: **PASS** from
`stm32_os_preboot_resource_gate1_measurement_v2_20260926_175102.evidence.zip`.

Accepted measurement facts:

- exact baseline BIN SHA-256
  `7EDB650B78D6778C57BA477EC466B31E3AC5CE693ECF933E04B42AC3F0B23F9F`;
- baseline Flash/SRAM `54268/11944`;
- `kernel.c -Os`: `52620` Flash, `-1648`;
- scheduler core `-Os`: `52908` Flash, `-1360`;
- USB device `-Os`: `52668` Flash, `-1600`;
- gfx/OLED presentation `-Os`: `52760` Flash, `-1508`;
- scheduler diagnostics `-Os`: `LINK_BLOCKED_COMPILER_MEMSET`, not selected;
- all linkable variants: SRAM unchanged at `11944`;
- `RING_RESIZE_AUTHORIZED=NO`.

Gate-2 source/profile authorization is therefore frozen narrowly to:

- add only `src/kernel.c` to the normal `-Os` profile;
- remove the scheduler-owned 1024-byte default task-stack store;
- preserve scheduler/self-test behavior by explicit borrowed-stack binding using
  already-existing production stack storage while the scheduler is inactive;
- make no USB-ring reduction, diagnostic command/RPC removal, diagnostic-profile
  split, libc/memset shim, linker relocation or Bootloader change.

Expected SRAM after removal of the duplicate default store is `10920` bytes from the
current baseline arithmetic. This is a Gate-2 selection invariant; Gate 3 must prove
the fresh linked bytes and actual final totals.

## Gate 2 — behavior-preserving source/profile recovery

Gate 2 is authorized only after Gate 1 selects exact changes.

Preferred order:

1. scheduler default-stack production ownership;
2. measured compiler-size profile changes;
3. acceptance-only diagnostic separation where justified;
4. coherent `kernel.c` ownership extraction only as required.

Reject the gate for speculative frameworks, ABI pruning for size alone, unmeasured
buffer cuts or Bootloader feature creep.

Acceptance: **PASS** from
`stm32_os_preboot_resource_gate2_recovery_recomposed_v1_20260926_210150.evidence.zip`,
SHA-256 `D84AC967C204FC9D09B94EC8AD7B472B173BCDCE4593320F88601B062681E325`.
The accepted normalized source diff SHA-256 is
`D1EC010F55313DA61DC1366E99E68A619CF4E678EDD2EE443E7D53AB08AD6A06`.
The real Git index remained empty; target I/O, Flash mutation, staging, commit and push
were all absent. The accepted live WIP changes exactly the six authorized firmware
paths and leaves USB-ring sizing, linker layout and external protocol/application ABI
outside the mutation set.

## Gate 3 — fresh build/resource/static acceptance

Required:

- Flash <= `53248` and SRAM <= `11264`, unless Gate 1 explicitly reopened the
  objectives with documented evidence before mutation;
- task stacks and stack-usage checks complete;
- task0/task1 hardware margins remain targeted at >= `256/256`;
- MSP measurable margin >= `1024`;
- no undefined/forbidden heap/libc drift;
- public ABI/version checks exact;
- final production BIN/ELF/MAP and source tree locked.

Acceptance: **PASS** from
`stm32_os_preboot_resource_gate3_build_static_v1_20260926_212215.evidence.zip`,
SHA-256 `9FA982530FAB4B703AF6D92936080BD13A0E2FF346D294F6376F5CAFE814EFE2`.
The accepted firmware-only candidate tree is
`a10182e7d0659b9b161073ad49a8816ecb6e7918`; production BIN is `52908` bytes with
SHA-256 `FE1CB8AF32063C0336D276EDAAB0583E6F269C953DCF0E68D4FB6F9B55D583C2`,
ELF SHA-256 `5BF8D5CE66049CCBD7EF2D77D6D569B980CB96B33E0FFDBFDB1A626A6A101247`, and
MAP SHA-256 `AE45CF669031AC4CB29DB2FFB72161ABA2249D9A6558B80E6B40757EC162C29E`.
Fresh linked totals are Flash/SRAM `52908/10920`; stack-usage is complete `26/26`,
undefined symbols and forbidden heap/libc drift are zero, production task stacks remain
`1024/512`, and public protocol/application ABI diff is zero. Runtime task/MSP margins
remain owned by Gate 4 hardware acceptance.

No target mutation before Gate 3 accepts the exact candidate.

## Gate 4 — hardware/runtime equivalence

Run the exact production candidate and retain:

- boot/startup;
- scheduler/task/IWDG liveness;
- UART emergency/text diagnostics;
- primary management IF2 and production host policy through the canonical management carrier (WinUSB when Windows owns target USB, Linux libusb on the current Mac-mini-owned USB topology); CDC remains secondary diagnostic/compatibility transport and is not an independent Gate-4 blocker when CDC implementation/descriptor/ring ownership is unchanged;
- binary framing/RPC;
- Asset STATUS/read/write/readback and persistence continuity as applicable without
  unnecessary wear. The current accepted physical prestate is canonical CLEAN from the
  Asset foundation: persistence pages erased, generation `0`, and compiled-default
  runtime. Therefore this recovery Gate 4 must prove CLEAN -> CLEAN continuity and
  compiled-default runtime without creating a persistence record merely to exercise a
  no-op write; write/readback is applicable only when a valid committed record already
  exists in the bound prestate;
- reconnect/reset recovery;
- UI/application runtime;
- zero unexpected production/application/scheduler/USB faults or drops;
- exact final Flash readback outside intentionally exercised persistence state;
- physical OLED disposition when presentation implementation changes.

The first Gate-4 hardware collector attempts are frozen as harness history, not
product regressions. V1 failed during generated .NET compilation before target I/O.
V2 evidence `stm32_os_preboot_resource_gate4_hardware_runtime_v2_20260927_140603.evidence.zip`
(SHA-256 `F9D61845494A023922E26EB3282A72CCFD721D464D33740AB441E420F93047F6`)
then successfully built the probe, programmed and verified the exact 54-KiB candidate
application region, and booted the exact candidate. From the second remote poll onward
USB discovery, negotiation, source-tree identity, Home application state and health/IWDG
liveness all succeeded. The collector nevertheless retried and finally reported
`REMOTE_NOT_READY` because its `quick` action incorrectly required an Asset committed
record. The same evidence's pre-flash full-64-KiB readback proves both persistence pages
were erased before Gate 4, which is the canonical CLEAN state required by the accepted
Asset foundation. V2 also returned process exit `0` for a final FAIL. Accordingly V2 is
invalid harness evidence classified as acceptance-oracle/readiness/terminal-status drift;
it is not a `PRODUCT_HARDWARE_RUNTIME_FAILURE` and must not be repaired by a one-line
assertion deletion or timeout change.

The recomposed Gate-4 v1 run `stm32_os_preboot_resource_gate4_hardware_runtime_recomposed_v1_20260927_143650.evidence.zip` (SHA-256 `E0C0018489A9998725320EC6DC6DA44510D8E1366ED093EA0E953DB18DA7D64B`) is frozen as partial harness evidence, not a product regression. Before its non-authoritative CDC check it proved exact candidate/Flash identity, canonical CLEAN persistence, boot/source-tree/Home/health and Windows UART `32/32`. The subsequent Linux `/dev/ttyACM0` permission failure is outside the authoritative risk set for this recovery because CDC is the published secondary diagnostic/compatibility transport, its implementation/descriptor/ring ownership is unchanged, and the canonical primary management path is IF2 over WinUSB/libusb. The passed prerequisites from this evidence should be replayed rather than repeated; Gate 4 remains open only for the still-unproven recovery-specific runtime assertions.

The first runtime-continuation attempt `stm32_os_preboot_resource_gate4_runtime_continuation_v1_20260927_150947.evidence.zip` (SHA-256 `3C8540D271A67D74651A01B8686C9FE6C8BD23276E106E4955CE90DB8AD2E1DE`) is also frozen as partial PASS plus harness-sequencing failure, not a product regression. It accepted runtime margins `304/432`, MSP margin `1592`, management IF2 pressure `128/128`, application lifecycle, canonical CLEAN Asset state, scheduler active/BUSY policy, passive CDC configured/zero-drops, zero USB errors/PMA overruns, physical OLED confirmation and physical reconnect recovery. It then observed `WDOG_TRIP_ARMED` at 15:12:20.590, slept only two seconds, treated the still-enumerated pre-reset USB session as post-IWDG readiness at 15:12:22.946, and the reset occurred during the next libusb IN, producing `LIBUSB_ERROR_TIMEOUT`. The accepted project history shows IWDG reboot completion around 7.4–9.0 seconds for this watchdog configuration; therefore this run is `HARNESS-IWDG-RESET-SYNCHRONIZATION-01`. Gate 4 must resume from an independently observed reset completion, not by increasing an arbitrary sleep.

The reset-synchronization proof `stm32_os_iwdg_reset_sync_primitive_proof_v1_20260927_152644.evidence.zip` (SHA-256 `BA1F678D212CA4B610B5443A879C121709B7534A5B207A8070B6C6CCB52092FB`) then proves the IWDG reset primitive itself: UART recovery occurred at `7575 ms`; post-reset `health` carried `IWDG_RESET=1`; tick changed from `877119` before arm to fresh tick `127`. Its subsequent IF2 check started only about 14 ms after that health sample, while the product contract still requires the minimum 1000-ms boot splash before application Home service; accepted Application Runtime history likewise records first post-IWDG Home observation as `PRE_HOME_ACTIVE_0` followed by Home PASS on the next readiness attempt. Therefore the final `system.home active` assertion in this primitive is `HARNESS-POSTRESET-APPLICATION-READINESS-01`, not a product regression. Gate 4 may close with a read-only tail from this already completed IWDG reset state.

Final Gate-4 closure is **PASS / COMPOSITE**. The read-only closure evidence
`stm32_os_preboot_resource_gate4_readonly_closure_v1_20260927_190303.evidence.zip`
has SHA-256 `05A8F13552656D3C2AE5B05F8BC0FC927140BF0CBAC964B12721E46BABB2357B`.
It binds the previously accepted partial runtime evidence and IWDG reset-synchronization
proof, performs no Flash programming/IWDG trip/reconnect/active CDC path, and proves:
post-IWDG UART `IWDG_RESET=1` with watchdog active; primary management IF2/libusb exact
source-tree identity, `system.home` active, application faults `0`, health and ping; final
read-only full-64-KiB Flash SHA-256
`17B48E7743F0AD1811FF431FA8C328E88C7CFBAD6ADA874F03C1392C2E94D726`; unchanged
HEAD/origin, empty real index and exact six-file firmware WIP. Across the bound Gate-4
chain the accepted runtime margins are task0/task1 `304/432` and MSP `1592`, IF2 pressure
is `128/128`, persistence remains canonical CLEAN without added wear, USB errors/PMA
overruns/drops are zero, and physical OLED plus physical reconnect are accepted.

If an acceptance-only diagnostic image also exists, it may provide additional proof,
but it cannot substitute for production-image hardware acceptance.

## Gate 5 — documentation/evidence finalization

Synchronize:

- `CURRENT_STATE.md`;
- `ROADMAP.md`;
- architecture/deferred-debt records;
- exact resource deltas;
- accepted production identities;
- remaining deferred host-client decomposition and Bootloader prerequisites.

No new product behavior in Gate 5.

Gate 5 docs/evidence finalization is **PASS** from
`stm32_os_preboot_resource_gate5_docs_finalization_v1_20260927_192658.evidence.zip`,
SHA-256 `FD2ACCA48E8051356668933FBA9E0A5E451E1E2C97ADADCF3BB3302FC87748B4`.
The read-only gate accepted the exact 28-path worktree set, exact six-path firmware diff
SHA-256 `D1EC010F55313DA61DC1366E99E68A619CF4E678EDD2EE443E7D53AB08AD6A06`,
empty real index, unchanged host source, clean `git diff --check`, and canonical state
synchronization. Its normalized complete documentation diff SHA-256 is
`397E449A64EED8160A21BC484F8217636B55E45819AD1FED59C867CAB67EFCA9`.
No build, target I/O, Flash mutation, index mutation, commit or push occurred.

## Gate 6 — local acceptance commit

Gate 6 is satisfied by the single normal local commit that stages exactly the accepted
28-path Gate-5 worktree plus these final Gate-6/Gate-7 state lines. Its harness must
record the parent, staged path set, staged tree, resulting commit/tree, empty post-commit
index/worktree, and `origin/main` still at the published parent. No amend and no push are
authorized in Gate 6.

Exactly one normal local acceptance commit after:

- full candidate validation;
- temp-index / whitespace validation as required by the harness playbook;
- clean real index before staging;
- exact accepted source/docs set only.

No push in Gate 6.

## Gate 7 — ordinary non-force publication

Gate 7 is **PASS** from `stm32_os_preboot_resource_gate7_nonforce_publish_v1_20260927_194416.evidence.zip`, SHA-256 `335013389D8AB6C1B09C4BF720185FED82EC4A0D30A2F3F6237CF8ACCA1DE8CE`. The ordinary non-force push published commit `a8f92f83c2ba8917ad183b1a099c9e21199c9463`, tree `013c1f472eb9404de94befdcc3f1e1e5acfc5831`; post-push fetch proved `HEAD == origin/main == FETCH_HEAD`, clean worktree/index and ahead/behind `0/0`.

After Gate 7, next boundary is exactly:

`FIRMWARE_UPDATE_BOOTLOADER_FOUNDATION`

Bootloader work must begin with its own Gate 0 contract; it is not implicitly
authorized by this recovery boundary.
