"""DEUS immutable ZIP producer/validator regression matrix (offline, no device)."""
import io
import json
import os
from pathlib import Path
import tempfile
import unittest
import zipfile
from bundle_guard import Invalid, build, validate_file, validate_blob, digest, release

PID="stm32_os_bundle_contract_offline_smoke_v1"
SCRIPT=b"Set-StrictMode -Version 3.0\n$report=[Environment]::GetEnvironmentVariable('DEUS_RUN_RESULT_PATH','Process')\nif (!$report) { throw 'NO_REPORT' }\n"
SPEC={"schema":"DEUS_OS_OPERATOR_BUNDLE_V2","package_id":PID,
      "runner_result_contract":"DEUS_OS_RUNNER_RESULT_V1","execution_profile":"OFFLINE"}

class BundleGuardTests(unittest.TestCase):
    def setUp(self):
        tmp=tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root=Path(tmp.name)
        self.src=self.root/"source"
        self.src.mkdir()
        (self.src/"apply.ps1").write_bytes(SCRIPT)
        (self.src/"PACKAGE_README.txt").write_text("offline smoke only\n")
        (self.src/"bundle_spec.json").write_text(json.dumps(SPEC))
        (self.src/"payload").mkdir()
        (self.src/"payload"/"offline.py").write_text("print('offline')\n")
        self.zip=self.root/(PID+".zip")

    def make(self):
        build(self.src,self.zip)
        return self.zip.read_bytes()

    def repack(self,blob,fn):
        with zipfile.ZipFile(io.BytesIO(blob)) as z:
            data={n:z.read(n) for n in z.namelist()}
        fn(data)
        out=io.BytesIO()
        with zipfile.ZipFile(out,"w",zipfile.ZIP_DEFLATED) as z:
            for name,value in data.items():z.writestr(name,value)
        return out.getvalue()

    def reject(self,blob,reason):
        with self.assertRaises(Invalid) as e:validate_blob(blob,PID+".zip")
        self.assertEqual(str(e.exception),reason)

    def test_01_build_and_verify(self):
        blob=self.make()
        self.assertEqual(validate_file(self.zip)["sha256"],digest(blob))

    def test_02_deterministic_rebuild(self):
        a=self.make();self.zip.unlink()
        self.assertEqual(a,self.make())

    def test_03_v2_apply_manifest_stale_even_when_index_updated(self):
        b=self.make()
        def mutate(d):
            d["apply.ps1"]+=b"# changed after manifest\n"
            old=digest(SCRIPT).encode()
            d["hashes.sha256"]=d["hashes.sha256"].replace(old,digest(d["apply.ps1"]).encode())
        self.reject(self.repack(b,mutate),"MANIFEST_APPLY_SHA_DRIFT")

    def test_04_index_wrong(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.__setitem__("PACKAGE_README.txt",b"changed")),
                    "INDEX_HASH_DRIFT:PACKAGE_README.txt")

    def test_05_missing_index_member(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.pop("payload/offline.py")),"INDEX_SET_MISMATCH")

    def test_06_duplicate_index_path(self):
        b=self.make()
        def m(d):d["hashes.sha256"]+=d["hashes.sha256"].splitlines(keepends=True)[0]
        self.reject(self.repack(b,m),"DUPLICATE_HASH_ENTRY")

    def test_07_wrong_runner_contract(self):
        b=self.make()
        def m(d):
            info=json.loads(d["manifest.json"])
            info["runner_result_contract"]="LEGACY"
            d["manifest.json"]=json.dumps(info).encode()
        self.reject(self.repack(b,m),"RUNNER_CONTRACT_DRIFT")

    def test_08_parent_traversal(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.__setitem__("../escape.py",b"x")),"PATH_TRAVERSAL")

    def test_09_abs_path(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.__setitem__("/tmp/escape",b"x")),"PATH_TRAVERSAL")

    def test_10_duplicate_case_insensitive_member(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.__setitem__("APPLY.ps1",b"x")),
                    "DUPLICATE_MEMBER_CASE")

    def test_11_missing_apply(self):
        b=self.make()
        self.reject(self.repack(b,lambda d:d.pop("apply.ps1")),"MISSING_ROOT_FILES")

    def test_12_old_strict_mode_prohibited(self):
        (self.src/"apply.ps1").write_bytes(SCRIPT.replace(b"3.0",b"Latest"))
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"STRICTMODE_3_REQUIRED")

    def test_13_contract_path_not_used(self):
        (self.src/"apply.ps1").write_bytes(SCRIPT.replace(b"DEUS_RUN_RESULT_PATH",b"WRONG"))
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"RESULT_CONTRACT_NOT_USED")

    def test_14_duplicate_manifest_key(self):
        b=self.make()
        def m(d):d["manifest.json"]=d["manifest.json"].rstrip()[:-1]+b',"schema":"DEUS_OS_OPERATOR_BUNDLE_V2"}'
        self.reject(self.repack(b,m),"DUPLICATE_JSON_KEY")

    def test_15_no_overwrite(self):
        self.make()
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"REFUSE_OVERWRITE")

    def test_16_name_mismatch(self):
        b=self.make()
        with self.assertRaises(Invalid) as e:validate_blob(b,"stm32_os_different.zip")
        self.assertEqual(str(e.exception),"PACKAGE_NAME_DRIFT")

    def test_17_external_sha_mismatch(self):
        b=self.make()
        with self.assertRaises(Invalid) as e:validate_blob(b,expected_sha="0"*64)
        self.assertEqual(str(e.exception),"WHOLE_ZIP_SHA_MISMATCH")

    def test_18_crc_failure(self):
        blob=self.make()
        with zipfile.ZipFile(io.BytesIO(blob)) as z:
            item=z.getinfo("PACKAGE_README.txt")
            at=item.header_offset+30+len(item.filename)+len(item.extra)
        broken=bytearray(blob);broken[at]^=0xff
        self.reject(bytes(broken),"CORRUPT_ZIP_OR_CRC")

    def test_19_index_self_reference(self):
        b=self.make()
        def m(d):d["hashes.sha256"]+=b"0"*64+b"  hashes.sha256\n"
        self.reject(self.repack(b,m),"SELF_HASH_ENTRY")

    def test_20_builder_must_own_manifest(self):
        (self.src/"manifest.json").write_text("{}")
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"BUILDER_OWNS_MANIFEST")

    def test_21_private_payload_disallowed(self):
        (self.src/"payload"/"secret.pem").write_text("not a real secret")
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"UNSAFE_PAYLOAD")

    def test_22_runner_owned_banner(self):
        (self.src/"apply.ps1").write_bytes(SCRIPT+b'Write-Host "RESULT=PASS"\n')
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"RUNNER_OWNS_RESULT")

    def test_23_defender_bypass(self):
        (self.src/"apply.ps1").write_bytes(SCRIPT+b"Set-MpPreference -DisableRealtimeMonitoring $true\n")
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"DEFENDER_BYPASS_FORBIDDEN")

    def test_24_script_final_bytes_pinned(self):
        updated=SCRIPT+b'Write-Host "OFFLINE_ONLY"\n'
        (self.src/"apply.ps1").write_bytes(updated)
        self.make()
        with zipfile.ZipFile(self.zip) as z:
            self.assertEqual(json.loads(z.read("manifest.json"))["apply_sha256"],digest(updated))

    def test_25_execution_profile_required(self):
        sp=SPEC.copy();sp.pop("execution_profile")
        (self.src/"bundle_spec.json").write_text(json.dumps(sp))
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"EXECUTION_PROFILE_REQUIRED")

    def test_26_native_release_does_not_run_on_linux(self):
        b=self.make()
        if os.name=="nt":self.skipTest("Windows release tested separately")
        with self.assertRaises(Invalid) as e:release(self.zip,digest(b),self.root/"none.json")
        self.assertEqual(str(e.exception),"NATIVE_WINDOWS_REQUIRED")

    def test_28_hardware_bundle_requires_exact_workflow_id(self):
        spec=SPEC.copy()
        spec["execution_profile"]="HARDWARE_READONLY"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"HARDWARE_WORKFLOW_ID_REQUIRED")

    def test_29_open_000c_incidents_block_composed_release(self):
        from bundle_guard import check_hardware_incident_blockers
        spec=SPEC.copy()
        spec["execution_profile"]="HARDWARE_READONLY"
        spec["workflow_id"]="000c_session_collector"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        b=self.make()
        with self.assertRaises(Invalid) as e:check_hardware_incident_blockers(b)
        self.assertTrue(str(e.exception).startswith("OPEN_NATIVE_PRIMITIVE_BLOCKERS:"))

    def test_30_real_windows_ast_no_execution(self):
        from bundle_guard import windows_ast_parse
        if os.name!="nt":self.skipTest("Native Windows-only AST primitive")
        windows_ast_parse(SCRIPT)
        with self.assertRaises(Invalid) as e:
            windows_ast_parse(SCRIPT+b"\n$b=@(\n")
        self.assertEqual(str(e.exception),"WINDOWS_POWERSHELL_AST_FAILED")

    def test_31_mutating_release_cannot_bypass_separate_signed_gate(self):
        spec=SPEC.copy()
        spec["execution_profile"]="HARDWARE_MUTATING"
        spec["workflow_id"]="other_approved_workflow"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        (self.src/"apply.ps1").write_bytes(SCRIPT+b"$PackageId='"+PID.encode()+b"'\n")
        b=self.make()
        with self.assertRaises(Invalid) as e:
            release(self.zip,digest(b),self.root/"fabricated_receipt.json")
        self.assertEqual(str(e.exception),"MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE")

    def test_27_incident_registry_links_all_guarded_defects(self):
        db=json.loads((Path.cwd()/"scripts/operator_bundle_guard/incidents.json").read_text())
        self.assertEqual(db["schema"],"DEUS_HARNESS_INCIDENT_REGISTRY_V1")
        self.assertGreaterEqual(len(db["entries"]),10)
        ids=[r["id"] for r in db["entries"]]
        self.assertEqual(len(ids),len(set(ids)))
        for r in db["entries"]:
            if r["status"]=="GUARDED_STATIC":
                self.assertTrue(hasattr(self,r["test"]))
            else:
                self.assertIn(r["status"],("PENDING_REAL_PRIMITIVE","OPEN_UNPROVEN_CAUSE"))

    def test_32_guard_ci_runs_without_path_filters(self):
        # A pushed ZIP outside scripts/ must not skip the public repository guard.
        workflow=(Path(__file__).resolve().parents[2]/
                  '.github/workflows/operator-bundle-guard.yml').read_text()
        self.assertIn('  push:\n',workflow)
        self.assertIn('  pull_request:\n',workflow)
        self.assertNotIn('    paths:',workflow)
        # Negative fixture: the old path-filtered trigger must fail the guard.
        with self.assertRaises(AssertionError):
            self.assertNotIn('    paths:',workflow.replace('  push:\n',
                                          '  push:\n    paths:\n      - scripts/**\n'))

    def public_scope(self):
        from bundle_guard import PUBLIC_TRUST_PACKAGE, PUBLIC_TRUST_WORKFLOW, PUBLIC_TRUST_NAMESPACE
        spec=SPEC.copy()
        spec.update(package_id=PUBLIC_TRUST_PACKAGE,
                    execution_profile="REMOTE_PUBLIC_TRUST_MUTATING",
                    workflow_id=PUBLIC_TRUST_WORKFLOW,
                    owner_authorization_namespace=PUBLIC_TRUST_NAMESPACE,
                    authorization_profile="SIGNED_OWNER_APPROVAL_REQUIRED")
        self.zip=self.root/(PUBLIC_TRUST_PACKAGE+".zip")
        return spec

    def test_33_public_trust_mutating_build_only_is_static(self):
        spec=self.public_scope()
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        blob=self.make()
        self.assertEqual(validate_file(self.zip)["profile"],"REMOTE_PUBLIC_TRUST_MUTATING")
        self.assertEqual(validate_file(self.zip)["qualification"],
                         "STATIC_ONLY_NOT_NATIVE_OR_HARDWARE")

    def test_34_public_trust_release_requires_separate_signed_gate(self):
        spec=self.public_scope()
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        blob=self.make()
        with self.assertRaises(Invalid) as error:
            release(self.zip,digest(blob),self.root/"forged_receipt.json")
        self.assertEqual(str(error.exception),"MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE")

    def test_35_public_trust_wrong_workflow_rejected(self):
        spec=self.public_scope()
        spec["workflow_id"]="other_linux_mutation"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_WORKFLOW_NOT_ALLOWED")

    def test_36_public_trust_wrong_package_rejected(self):
        spec=self.public_scope()
        spec["package_id"]=PID
        self.zip=self.root/(PID+".zip")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_PACKAGE_NOT_ALLOWED")

    def test_37_public_trust_missing_owner_namespace_rejected(self):
        spec=self.public_scope()
        del spec["owner_authorization_namespace"]
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_OWNER_NAMESPACE_REQUIRED")

    def test_38_public_trust_wrong_owner_namespace_rejected(self):
        spec=self.public_scope()
        spec["owner_authorization_namespace"]="deus-os-gate3-one-reset-v1"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_OWNER_NAMESPACE_REQUIRED")

    def test_39_public_trust_missing_signed_gate_declaration_rejected(self):
        spec=self.public_scope()
        spec["authorization_profile"]="NONE"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_OWNER_SIGNED_GATE_REQUIRED")

    def test_40_public_trust_cannot_be_relabelled_offline(self):
        spec=self.public_scope()
        spec["execution_profile"]="OFFLINE"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_PROFILE_REQUIRED")

    def test_41_public_trust_cannot_be_relabelled_readonly(self):
        spec=self.public_scope()
        spec["execution_profile"]="HARDWARE_READONLY"
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as error: self.make()
        self.assertEqual(str(error.exception),"PUBLIC_TRUST_PROFILE_REQUIRED")

    def test_42_existing_usb_hardware_mutating_remains_denied(self):
        spec=SPEC.copy()
        spec.update(execution_profile="HARDWARE_MUTATING",
                    workflow_id="stm32_physical_reset")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        (self.src/"apply.ps1").write_bytes(SCRIPT+b"$PackageId='"+PID.encode()+b"'\n")
        blob=self.make()
        with self.assertRaises(Invalid) as error:
            release(self.zip,digest(blob),self.root/"fabricated_receipt.json")
        self.assertEqual(str(error.exception),"MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE")

    def test_43_mutating_hardware_top_level_gate_id_mismatch_rejected(self):
        spec=SPEC.copy()
        spec.update(execution_profile="HARDWARE_MUTATING",
                    workflow_id="gate3_physical_identity_probe")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        (self.src/"apply.ps1").write_bytes(
            SCRIPT+b"$PackageId='stm32_os_other_inner_runner_v1'\n")
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),"HARDWARE_RESULT_GATE_ID_DRIFT")

    def test_44_mutating_hardware_nested_second_result_writer_rejected(self):
        spec=SPEC.copy()
        spec.update(execution_profile="HARDWARE_MUTATING",
                    workflow_id="gate3_physical_identity_probe")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        (self.src/"apply.ps1").write_bytes(
            SCRIPT+b"$PackageId='"+PID.encode()+b"'\n"+
            b"& (Join-Path $PSScriptRoot 'physical_original_apply.ps1')\n")
        (self.src/"physical_original_apply.ps1").write_bytes(
            b"Set-StrictMode -Version 3\n"+
            b"$PackageId='stm32_os_gate3_000d_one_reset_info_macmini_v1_20261027'\n"+
            b"$x=$env:DEUS_RUN_RESULT_PATH\n")
        with self.assertRaises(Invalid) as e:build(self.src,self.zip)
        self.assertEqual(str(e.exception),
                         "HARDWARE_MULTIPLE_RUNNER_RESULT_WRITERS:physical_original_apply.ps1")

    def test_45_hardware_single_result_writer_static_only_still_not_released(self):
        spec=SPEC.copy()
        spec.update(execution_profile="HARDWARE_MUTATING",
                    workflow_id="gate3_physical_identity_probe")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        (self.src/"apply.ps1").write_bytes(
            SCRIPT+b"$PackageId='"+PID.encode()+b"'\n")
        blob=self.make()
        self.assertEqual(validate_file(self.zip)["qualification"],
                         "STATIC_ONLY_NOT_NATIVE_OR_HARDWARE")
        with self.assertRaises(Invalid) as e:
            release(self.zip,digest(blob),self.root/"not_a_signed_receipt.json")
        self.assertEqual(str(e.exception),
                         "MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE")

    def test_46_mismatch_and_nested_writer_do_not_affect_offline_profile(self):
        (self.src/"apply.ps1").write_bytes(
            SCRIPT+b"$PackageId='stm32_os_other_offline_probe_v1'\n")
        (self.src/"physical_original_apply.ps1").write_bytes(
            b"$x=$env:DEUS_RUN_RESULT_PATH\n")
        blob=self.make()
        self.assertEqual(validate_file(self.zip)["profile"],"OFFLINE")
    def test_47_hardware_mutating_requires_exact_declared_result_owner(self):
        spec=SPEC.copy()
        spec.update(execution_profile="HARDWARE_MUTATING",
                    workflow_id="gate3_physical_identity_probe")
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        # A manifest can be valid even when its entrypoint never declares
        # the actual result gate. This must not be hardware-admissible.
        with self.assertRaises(Invalid) as err:
            self.make()
        self.assertEqual(str(err.exception),
                         "HARDWARE_RESULT_GATE_ID_REQUIRED")
    def test_48_quarantined_source_directory_never_builds(self):
        (self.src/"QUARANTINED_NOT_FOR_RUN.txt").write_text(
            "QUARANTINED / SOURCE ONLY / NO RUN / NO PACKAGE / NO PUBLICATION\n")
        with self.assertRaises(Invalid) as err:
            self.make()
        self.assertEqual(str(err.exception),"QUARANTINED_SOURCE_NOT_FOR_BUILD")
        self.assertFalse(self.zip.exists())

    def test_49_quarantine_marker_in_zip_never_validates(self):
        blob=self.make()
        updated=self.repack(blob,lambda d:d.__setitem__(
            "QUARANTINED_NOT_FOR_RUN.txt",b"NO RUN\n"))
        with self.assertRaises(Invalid) as err:
            validate_blob(updated,PID+".zip")
        self.assertEqual(str(err.exception),"QUARANTINED_SOURCE_NOT_FOR_BUILD")


    def test_50_evidence_prefix_only_traceback_rejected_before_build(self):
        # Reproduces the exact A-05 v4 lost-terminal-exception source pattern.
        (self.src/"apply.ps1").write_bytes(
            SCRIPT + b"$one=$one.Substring(0,600)\n")
        with self.assertRaises(Invalid) as err:
            self.make()
        self.assertEqual(str(err.exception),
                         "EVIDENCE_PREFIX_ONLY_TRUNCATION_FORBIDDEN")
        self.assertFalse(self.zip.exists())

    def test_51_evidence_prefix_only_traceback_rejected_when_validated(self):
        blob=self.make()
        def update(data):
            changed=SCRIPT + b"$one=$one.Substring(0,600)\n"
            data["apply.ps1"]=changed
            manifest=json.loads(data["manifest.json"])
            manifest["apply_sha256"]=digest(changed)
            data["manifest.json"]=(json.dumps(manifest,sort_keys=True)+"\n").encode()
            data["hashes.sha256"]="".join(
                digest(v)+"  "+n+"\n"
                for n,v in sorted(data.items()) if n != "hashes.sha256").encode()
        with self.assertRaises(Invalid) as err:
            validate_blob(self.repack(blob,update),PID+".zip")
        self.assertEqual(str(err.exception),
                         "EVIDENCE_PREFIX_ONLY_TRUNCATION_FORBIDDEN")

    def test_52_evidence_tail_preserving_version_not_preemptively_rejected(self):
        # Static allow does NOT qualify the writer; Windows fail-path proof
        # is still mandatory under the canonical playbook.
        safe=SCRIPT + (b"$one=$one.Substring(0,120) + "
                       b"' [MIDDLE_TRUNCATED_TAIL_PRESERVED] ' + "
                       b"$one.Substring($one.Length-430)\n")
        (self.src/"apply.ps1").write_bytes(safe)
        self.assertEqual(self.make() is not None, True)
        self.assertEqual(validate_file(self.zip)["qualification"],
                         "STATIC_ONLY_NOT_NATIVE_OR_HARDWARE")

    def test_53_independent_cli_rejects_genuine_v4_diagnostic_loss(self):
        from bundle_guard import main
        from contextlib import redirect_stdout, redirect_stderr
        evidence=Path.cwd().parent/"Evidence"/(
            "stm32_os_gate4_a05_controller_adapter_native_offline_v4_20261028_"
            "20261028_211601_84e61bbdc92345d781a6023698b84f2d.evidence.zip")
        if not evidence.is_file():
            self.skipTest("Original owner Windows Evidence resides outside Git; synthetic negatives remain mandatory")
        err=io.StringIO()
        with redirect_stdout(io.StringIO()),redirect_stderr(err):
            rc=main(["evidence",str(evidence),"--sha256",
                "471563D0FC49AD7DFE091FF1158C1F1B1A8E8173014B32C8348D0AD117AD5972"])
        self.assertEqual(rc,2)
        output=json.loads(err.getvalue())
        self.assertEqual(output["result"],"FAIL")
        self.assertIn("DIAGNOSTIC_INCOMPLETE_PREFIX_ONLY",output["reason"])


    def test_54_network_download_profile_exact_only(self):
        from bundle_guard import WHEEL_DOWNLOAD_PACKAGE, WHEEL_DOWNLOAD_WORKFLOW
        spec=SPEC.copy()
        spec.update({"package_id":WHEEL_DOWNLOAD_PACKAGE,
                     "execution_profile":"HOST_NETWORK_DOWNLOAD",
                     "workflow_id":WHEEL_DOWNLOAD_WORKFLOW,
                     "network_scope":"PYPI_JSON_AND_FILESHOSTED_HTTPS_ONLY",
                     "host_mutation":"DOWNLOAD_ONLY_NO_INSTALL"})
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        renamed=self.root/(WHEEL_DOWNLOAD_PACKAGE+".zip")
        result=build(self.src,renamed)
        self.assertEqual(result["profile"],"HOST_NETWORK_DOWNLOAD")
        self.assertEqual(validate_file(renamed)["package_id"],WHEEL_DOWNLOAD_PACKAGE)

    def test_55_network_download_unrelated_package_rejected(self):
        spec=SPEC.copy()
        spec.update({"execution_profile":"HOST_NETWORK_DOWNLOAD",
                     "workflow_id":"gate4_a05_pypi_wheel_acquisition",
                     "network_scope":"PYPI_JSON_AND_FILESHOSTED_HTTPS_ONLY",
                     "host_mutation":"DOWNLOAD_ONLY_NO_INSTALL"})
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as err:
            self.make()
        self.assertEqual(str(err.exception),"NETWORK_DOWNLOAD_PACKAGE_NOT_ALLOWED")

    def test_56_network_download_wrong_domain_scope_denied(self):
        from bundle_guard import WHEEL_DOWNLOAD_PACKAGE, WHEEL_DOWNLOAD_WORKFLOW
        spec=SPEC.copy()
        spec.update({"package_id":WHEEL_DOWNLOAD_PACKAGE,
                     "execution_profile":"HOST_NETWORK_DOWNLOAD",
                     "workflow_id":WHEEL_DOWNLOAD_WORKFLOW,
                     "network_scope":"UNRESTRICTED",
                     "host_mutation":"DOWNLOAD_ONLY_NO_INSTALL"})
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as err:
            build(self.src,self.root/(WHEEL_DOWNLOAD_PACKAGE+".zip"))
        self.assertEqual(str(err.exception),"NETWORK_DOWNLOAD_DOMAIN_SCOPE_REQUIRED")

    def test_57_network_download_cannot_pretend_offline(self):
        from bundle_guard import WHEEL_DOWNLOAD_PACKAGE, WHEEL_DOWNLOAD_WORKFLOW
        spec=SPEC.copy()
        spec.update({"package_id":WHEEL_DOWNLOAD_PACKAGE,
                     "execution_profile":"OFFLINE",
                     "workflow_id":WHEEL_DOWNLOAD_WORKFLOW})
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as err:
            build(self.src,self.root/(WHEEL_DOWNLOAD_PACKAGE+".zip"))
        self.assertEqual(str(err.exception),"NETWORK_DOWNLOAD_PROFILE_REQUIRED")

    def test_58_network_download_cannot_request_install(self):
        from bundle_guard import WHEEL_DOWNLOAD_PACKAGE, WHEEL_DOWNLOAD_WORKFLOW
        spec=SPEC.copy()
        spec.update({"package_id":WHEEL_DOWNLOAD_PACKAGE,
                     "execution_profile":"HOST_NETWORK_DOWNLOAD",
                     "workflow_id":WHEEL_DOWNLOAD_WORKFLOW,
                     "network_scope":"PYPI_JSON_AND_FILESHOSTED_HTTPS_ONLY",
                     "host_mutation":"DOWNLOAD_AND_INSTALL"})
        (self.src/"bundle_spec.json").write_text(json.dumps(spec))
        with self.assertRaises(Invalid) as err:
            build(self.src,self.root/(WHEEL_DOWNLOAD_PACKAGE+".zip"))
        self.assertEqual(str(err.exception),"NETWORK_DOWNLOAD_NO_INSTALL_REQUIRED")


if __name__=="__main__":
    unittest.main(verbosity=2)
