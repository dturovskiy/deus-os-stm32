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

if __name__=="__main__":
    unittest.main(verbosity=2)
