#!/usr/bin/env python3
"""Pure RA-01 deterministic fixtures: no owner signature, no device, no consent."""
import json
import unittest
from datetime import datetime, timezone, timedelta
from dataclasses import replace

import release_approval_contract_v1 as p

NOW=datetime(2026,10,28,18,0,0,tzinfo=timezone.utc)
M=p.TrustedMaterial(
    zip_sha256=p.sha(b"NO_REAL_OPERATOR_ZIP_SYNTHETIC"),
    zip_bytes=29,
    source_lock_sha256="A"*64,
    source_git_tree=p.FUTURE_TREE,
    build_provenance_sha256="B"*64,
    entrypoint_sha256="C"*64,
    readiness_evidence_sha256="D"*64,
    device_usb_port="usb:001:8",
)

def fixture():
    return {
        "schema":p.SCHEMA,"workflow_id":p.WORKFLOW,"package_id":p.PKG_ID,
        "bundle_schema":p.BUNDLE_SCHEMA,"operator_profile":p.PROFILE,
        "operator_zip_sha256":M.zip_sha256,"operator_zip_bytes":M.zip_bytes,
        "source_lock_sha256":M.source_lock_sha256,"source_git_tree":M.source_git_tree,
        "build_provenance_sha256":M.build_provenance_sha256,
        "entrypoint_sha256":M.entrypoint_sha256,"owner_identity":p.OWNER,
        "owner_signer_fingerprint":p.SIGNER_FINGERPRINT,
        "signing_namespace":p.NAMESPACE,
        "valid_from_utc":"2026-10-28T17:59:00Z",
        "valid_until_utc":"2026-10-28T18:09:00Z",
        "nonce":"0123456789abcdef0123456789abcdef",
        "host_identity":"macmini","device_usb_port":M.device_usb_port,
        "expected_old_vid_pid":"1209:000C",
        "expected_old_source_tree":p.INSTALLED_TREE,
        "expected_bootloader_vid_pid":"1209:000D",
        "signed_normal_v6_sha256":p.NORMAL_SHA,
        "signed_rescue_v7_sha256":p.RESCUE_SHA,
        "allowed_enter_count":1,"allowed_info_count":1,"allowed_usb_node_fix_count":1,
        "flash_allowed":False,"erase_allowed":False,
        "retry_allowed":False,"automatic_return_allowed":False,
        "readiness_evidence_sha256":M.readiness_evidence_sha256
    }

def valid():
    return p.canonical_bytes(fixture())

def mutate_field(key,value):
    f=fixture();f[key]=value
    return p.canonical_bytes(f)

class Good(unittest.TestCase):
    def test_valid_still_denied(self):
        result=p.inspect_source_only(valid(),M,NOW)
        self.assertEqual(result["result"],"PASS")
        self.assertEqual(result["classification"],"RA01_CANONICAL_SOURCE_CONTRACT_ONLY")
        self.assertEqual(result["authorization"],"DENIED")
        self.assertEqual(result["signature"],"NOT_VERIFIED")
        self.assertEqual(result["nonce_consumed"],"NO")
        self.assertEqual(result["physical_executor"],"NOT_ADMITTED")
        self.assertEqual(result["device_io"],"NONE")

    def test_matching_whole_digest(self):
        self.assertEqual(p.inspect_source_only(valid(),M,NOW)["statement_sha256"],p.sha(valid()))

    def test_contract_exact_keyset(self):
        self.assertEqual(set(fixture()),p.REQUIRED)

    def test_deterministic_canonicalization(self):
        self.assertEqual(p.canonical_bytes(fixture()),p.canonical_bytes(dict(reversed(list(fixture().items())))))

class Bad(unittest.TestCase):
    def run_invalid(self,blob,material=M,now=NOW):
        with self.assertRaises(p.Reject):
            p.inspect_source_only(blob,material,now)

    def test_missing_field(self):
        f=fixture();del f["nonce"];self.run_invalid(p.canonical_bytes(f))

    def test_unknown_field(self):
        f=fixture();f["unexpected"]="x";self.run_invalid(p.canonical_bytes(f))

    def test_duplicate_fields(self):
        blob=valid().rstrip(b"\n")
        self.run_invalid(blob[:-1]+b',"nonce":"00112233445566778899aabbccddeeff"}\n')

    def test_noncanonical_whitespace(self):
        self.run_invalid(json.dumps(fixture(),indent=2).encode()+b"\n")

    def test_noncanonical_crlf(self):
        self.run_invalid(valid()[:-1]+b"\r\n")

    def test_noncanonical_no_newline(self):
        self.run_invalid(valid().rstrip(b"\n"))

    def test_bom(self):
        self.run_invalid(b"\xef\xbb\xbf"+valid())

    def test_null_byte(self):
        self.run_invalid(valid()+b"\x00")

    def test_oversize(self):
        self.run_invalid(valid()+b" " * 4097)

    def test_invalid_utf8(self):
        self.run_invalid(valid()+b"\xff")

    def test_invalid_json(self):
        self.run_invalid(b"not json\n")

    def test_wrong_top_level_array(self):
        self.run_invalid(b"[]\n")

    def test_nonfinite(self):
        self.run_invalid(valid().replace(b'"operator_zip_bytes":29',b'"operator_zip_bytes":NaN'))

    def test_time_without_timezone(self):
        self.run_invalid(valid(),now=NOW.replace(tzinfo=None))

    def test_time_nonutc(self):
        self.run_invalid(valid(),now=NOW.astimezone(timezone(timedelta(hours=1))))

    def test_expired_approval(self):
        self.run_invalid(valid(),now=NOW+timedelta(hours=1))

    def test_not_yet_valid(self):
        self.run_invalid(valid(),now=NOW-timedelta(hours=1))

    def test_bad_calendar_date(self):
        self.run_invalid(mutate_field("valid_until_utc","2026-02-30T18:00:00Z"))

    def test_inverted_window(self):
        self.run_invalid(mutate_field("valid_until_utc","2026-10-28T17:58:00Z"))

    def test_window_too_long(self):
        self.run_invalid(mutate_field("valid_until_utc","2026-10-28T18:20:01Z"))

    def test_wrong_date_format(self):
        self.run_invalid(mutate_field("valid_until_utc","2026-10-28T18:09:00+00:00"))

    def test_noncano_escapes(self):
        self.run_invalid(valid().replace(b'"macmini"',b'"mac\\u006dini"'))

    def test_trust_data_wrong_sha(self):
        self.run_invalid(valid(),material=replace(M,entrypoint_sha256="ab"*32))

    def test_trust_source_tree_wrong(self):
        self.run_invalid(valid(),material=replace(M,source_git_tree="0"*40))

    def test_trust_port_changed(self):
        self.run_invalid(valid(),material=replace(M,device_usb_port="usb:002:9"))

    def test_trust_port_malformed(self):
        self.run_invalid(valid(),material=replace(M,device_usb_port="127.0.0.1:8000"))

    def test_trust_host_drift(self):
        self.run_invalid(valid(),material=replace(M,host_identity="windows"))

    def test_trust_bytes_bool(self):
        self.run_invalid(valid(),material=replace(M,zip_bytes=True))

MUTATIONS={
    "schema":"UNRELATED_V1",
    "workflow_id":"gate3_enter_info_once",
    "package_id":"stm32_os_gate3_enter_info_once_v1",
    "bundle_schema":"DEUS_OS_OPERATOR_BUNDLE_V3",
    "operator_profile":"OFFLINE",
    "operator_zip_sha256":"0"*64,
    "operator_zip_bytes":30,
    "source_lock_sha256":"0"*64,
    "source_git_tree":"0"*40,
    "build_provenance_sha256":"0"*64,
    "entrypoint_sha256":"0"*64,
    "owner_identity":"not-an-owner",
    "owner_signer_fingerprint":"SHA256:wrong",
    "signing_namespace":"deus-os-gate3-one-reset-v1",
    "host_identity":"windows",
    "device_usb_port":"usb:001:9",
    "expected_old_vid_pid":"1209:000D",
    "expected_old_source_tree":"0"*40,
    "expected_bootloader_vid_pid":"1209:000E",
    "signed_normal_v6_sha256":"0"*64,
    "signed_rescue_v7_sha256":"0"*64,
    "allowed_enter_count":2,
    "allowed_info_count":0,
    "allowed_usb_node_fix_count":2,
    "flash_allowed":True,
    "erase_allowed":True,
    "retry_allowed":True,
    "automatic_return_allowed":True,
    "readiness_evidence_sha256":"0"*64,
    "nonce":"0"*32,
}

def make_bad(field,value):
    def check(self):
        self.run_invalid(mutate_field(field,value))
    return check
for field,value in MUTATIONS.items():
    setattr(Bad,"test_reject_"+field,make_bad(field,value))

if __name__=="__main__":
    unittest.main(verbosity=1)
