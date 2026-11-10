# Deus OS — v9 P3 Host updater: source-only Mac mini native readiness

**Date:** 2026-11-09 23:35 Europe/Paris.

**Verdict:** P3_HOST_SOURCE_ONLY_NATIVE_MAC_BUILD=PASS; P3_SIGNED_NORMAL_RESCUE_OFFLINE=PASS; P3_REAL_CSHARP_480_POLICY_MATRIX=PASS; PHYSICAL_V9=NOT_ATTESTED; P3_RESET_FLASH/Gate4=NOT_AUTHORIZED.

**2026-11-10 P3-A PHYSICAL OWNER ENTER/BOOT INFO COMPLETED (LATEST; supersedes earlier NOT_AUTHORIZED P3-A text below):** User explicitly approved **one** physical ENTER/RESET + read-only INFO, still **NOT P3-B Flash**. Original Mac `esp010-v9-p3a-enter-once-1110-a1` receipt SHA `DC901FC9329F6B3EC5AEB44BF2CA7244DE5F9F89E14BA03D5A222BF5448B9BA3` exit0 `ENTER=Ok/Resetting` from physically confirmed signed v8 runtime. Separate actual Mac bootloader000D `INFO=Ok RecoveryIdle FLOOR=8 COMMITTED=8` and `FLASH=NOT_STARTED`, receipt SHA `7424B777F9915C82E90DFF0D09642EE2EC1CE9E8AA5EAAAEE7AC32959110E83D`. Native owner device **currently bootloader**, v8 application signed payload unchanged. Independent four jobs original 12/12 SHA `task_1794266534902_1e9d3d96a6fe5a68` PASS. Historical first preflight CRLF source-assertion FAIL preserved, corrected second preflight PASS; one-shot physical Enter marker 0600 retained. Next P3-B requires separately explicit owner Flash consent and **fresh** actual bootloader info8/8 before one signed normal9 update; no auto retry. Scoped new `docs/STM32_V9_P3A_NATIVE_MAC_ENTER_BOOT_INFO_ACCEPTANCE_20261110.md`.

## Why a v9 Host candidate is necessary

The accepted v8 host BenchFirmwareCommand.cs (SHA256 5885E2FE4B0F802EEE9D5AEA8A03F7E2D2D01DAFC4847BCCF15DA6642EC6B850) supports only normal updates through v8. It pins signed normal8 SHA and bootloader committed7/floor7. Never substitute the signed v9 package under that old command.

FirmwareUpdateClient and the v1 Bootloader protocol are unchanged. Built an **isolated new Mac source candidate** with a separate bench-fw-v9 command, not a replacement for the installed v8 CLI. Source staging: OS/build/esp010_v9_host_source_candidate_20261109/BenchFirmwareV9Command.cs (13485 bytes SHA256 ACC3D74653FDEFFD1E6F9585A5AAF4A669A733F86C7E4FE1F44D4C3B49F84DF1), and private Mac candidate DEUS-MCP/macmini-workspace/automation/host-work/esp010-v9-cli-candidate-20261109. All 56 original signed-v8 Host source inputs were verified against their original Mac SHA index; 55 unchanged in the candidate, one Program.cs has one additive dispatch line; one new C# class. Original Mac v8 Host deus-cp.dll SHA DE842365C0AB7F11FCF48E7115922D4A2A86B50F2082576EF6BDD81DE7DC4936 unchanged. Current OS/host source not edited.

## Original native Mac compile + signed package tests

Mac mini SDK10.0.114 real release build: 0 errors, 0 warnings. Final candidate deus-cp.dll: **65024 bytes SHA256 8DB3CB42AB27CCB6F2C5203CF254D4B6E1A2D87731A0485F2C708132B840B048**.

Original Mac native build-v2 job esp010-v9-sourceonly-mac-build-v2-1109-a1, exact request SHA 05C1EC454FC1DCD43E394F908E2149A6A427C08BD3519F3F66DCBE1C365D45D1, original typed receipt SHA 76890F130E205F12E750D55AB1A0A30B5570583BF94D1E24AF72CE282B029C01, full log SHA E04C26A59144DB9A9185C80DAA9FAC6904CDBFDE476E86D402066F567E536F02, exit0/no timeout.

Both real owner-signed packages were copied into the private Mac candidate signed_inputs dir (0700 folder, 0600 individual files), never into Git or Evidence: normal9 v4 signed package **49100B SHA DFA66DFFAEBABFC75C5D0942A157D07904EADA7D3B7D13A21BB6B5C014C7416B**, rescue9 rev5-content signed package **52092B SHA 4D5EC4FBA74D0E09C0A61330B1834940C3B9CBAD0CD49A9E13B6A26961B02928**. The original ProtectedRecovery and its local Backups/20261109_signed_v9/ still SHA match.

Actual compiled candidate CLI offline executions: bench-fw-v9 plan, check-normal, check-rescue **3 positives PASS**; cross-swapped packages, missing/corrupted files, bad argument/device locator, missing ENTER/UPDATE/recovery confirmations **12 negatives PASS** including legacy bench-fw check-v8 refusing the v9 package. Job esp010-v9-offline-pkg-negative-tests-v2-1109-a1, original request SHA 15F29DAA27E87BE4A46F59D5DC693C1DCD12F2057D70E627A27A2195C43A31ED; original receipt SHA 68C5FCF33BE394A85CECF623AA23698533979029A7BBFAB92C4C014AE6A0BCE7; original log SHA 2AFD5B8D2F67C4EFD84637ECDA5C8DE4265394914A3BFBF2AB62AD6CDFB114F9; exit0/no timeout.

Independent audit of six original Mac request/receipt/log files, full SHA, worker identity, exit/timeout, exact native compiled class and DLL, 56 original v8 input SHA, signed private pkg SHA: Tools/scripts/bench/audit_original_mac_v9_host_native_build_offline_tests_20261109.py task **task_1794263476838_234a1efc11761968 PASS**.

## Recovery logic fixed before any physical deployment

Real bootloader.c scan_metadata uses authenticated metadata to derive **floor**, but only matching image digest to derive **committed**. If a v8→v9 update aborts before final END, v8 application may become invalid while metadata still has floor8: committed0/floor8. If installed normal9 later becomes invalid, metadata is floor9: committed0/floor9.

The initial pre-release host candidate incorrectly required committed0/floor9 for rescue. It was never deployed, and its original compiled bytes were preserved as a historical unreleased DLL (SHA 77D2317270D68BF1FC6E3974AAA837BB6202B54B9991F693E1E510F13A99E6A6). Final host **only** allows signed rescue9 when committed0 with floor8 OR floor9, plus INFO Ok/RecoveryIdle/offset0 and exact signed package SHA/version/target. Normal9 is allowed **only** for committed8/floor8 in that same idle state.

A standalone actual C# native test references the compiled final Host and invokes both internal predicates via reflection over **480 combinations** of opcode/status/state/offset/floor/committed: exactly **1 normal** and **2 rescue** combinations accepted, 477 denied. It also executes actual signed package validators: 2 valid / 4 invalid. Native source-only Mac job esp010-v9-real-csharp-policy-probe-1109-a1: request SHA ED59CC00421A943EEF9B31AA4E46E81C50601FFD227E11D1C437CFB0CE9A9F96, original receipt SHA 019767A4B51726559370462EF677051DA342D993EAD36407EC9FBD27A58879CF, original log SHA 4FB34A9B4797E1DD296A7DFF4594E42964D81532A8E4B5A438AD62EFB3B76454, exit0, 0 warnings/errors. Test DLL SHA CFD50076512D1019A1BE0F796A702F65C915630139F9F1C00CF1CE7197379F7D. Independent third original Mac audit: Tools/scripts/bench/audit_original_mac_v9_policy_probe_20261109.py task **task_1794263695991_b430b588d665d971 PASS**.

## Prepared physical sequence, NOT executed and NOT authorized

1. **P3-A separately owner-authorized ENTER/RESET:** check physical single 1209:000E STM32 Management device and exact installed v8 SourceTree 2efe9374f2fe2aa2fec47e8c7bc0ca9203e1f926, Service3 Registry39. One explicitly confirmed ENTER, no auto-retry. Reenumerated 1209:000D bootloader INFO must be read separately.
2. **P3-B separately authorized FLASH:** require actual bootloader INFO status Ok/RecoveryIdle, committed8/floor8/offset0 on exact same Mac USB owner; exact signed normal9 49100B SHA above; one explicitly confirmed normal-v9 transfer. On timeout/ambiguity STOP, inspect hardware, never auto-retry. No auto-ENTER/Flash chain.
3. **P3-C after a successful commit:** actual 1209:000E source identity, PONG, registry 39, watchdog/no resets, MSP/console/heartbeat canaries/high-water, OLED I2C physical output, USART2 NodeLink ESP v2 traffic, negative/flood regression. Legacy v8 real physical console PSP margin 404B is baseline only. Reenumerated USB access may need separately scoped permission repair, not auto-sudo.
4. **P3-R emergency only, separately authorized:** if after interrupted transfer real bootloader INFO has committed0/floor8 or 0/floor9, separately confirmed signed rescue9 recovery can be attempted exactly once. The rev5-content rescue runtime is 1209:000C, distinct from normal9 000E. A committed valid rescue9 at floor9 cannot be upgraded back to same-version normal9; normal10+ would need a new separate signed candidate and explicit version/return decision.

No new signed normal10/rescue10 created, no physical P3 reset/Flash/USB, no CH340, no Gate4 authority, no hardware-mutating operator ZIP. This is complete SOURCE-ONLY HOST READINESS, not an installed firmware claim.

**Next real missing evidence:** owner-specific P3-A real entering the Bootloader and obtaining actual INFO8/8, before a separate P3-B Flash decision. Do not introduce a separate-drive backup condition: local ProtectedRecovery/Backups v9 copy already verified and is adequate for the current one-board BENCH custody selection.
