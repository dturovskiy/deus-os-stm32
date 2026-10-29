#!/usr/bin/env python3
"""DEUS Gate3 permanent owner-consent and source audit (read-only).

This module never builds, releases, installs, resets, or performs device IO.
"""
from __future__ import annotations
import argparse
import base64
from datetime import datetime, timezone, timedelta
import hashlib
import json
from pathlib import Path
import re
import sys
import zipfile

SIGN_NAMESPACE = b"deus-os-gate3-one-reset-v1"
PRINCIPAL = "deus-os-hardware-owner"
OWNER_FP = "SHA256:4Ze7JMXNBsGEmhG/j+VRm9+X+a/C3dVlLSk59MU440M"
TRUST_SHA = "AFF18C4CB818D1637268787FE7F8834CABD56ACDC5615772E374AB907CF15069"
TRUST_EVIDENCE_SHA = "C4E5A40BE4DB9C703F00B0286FDC1E3299DDD4032FBE2D6767005F9399FD3227"
STAGE_ID = "stm32_os_gate3_000d_one_reset_info_macmini_v1_20261027"

class Reject(ValueError):
    pass

def need(ok, name):
    if not ok:
        raise Reject(name)

def sha(data):
    return hashlib.sha256(data).hexdigest().upper()

def json_unique(pairs):
    out = {}
    for k,v in pairs:
        need(k not in out, "DUPLICATE_JSON_KEY")
        out[k]=v
    return out

def parse_json(data):
    need(len(data) < 200_000, "JSON_TOO_LARGE")
    value = json.loads(data.decode("utf-8"), object_pairs_hook=json_unique)
    need(isinstance(value, dict), "OBJECT_REQUIRED")
    return value

def evidence(path, expected):
    need(path.is_file() and not path.is_symlink(), "EVIDENCE_MISSING")
    need(0 < path.stat().st_size < 8_000_000, "EVIDENCE_SIZE")
    need(sha(path.read_bytes()) == expected.upper(), "EVIDENCE_SHA")
    with zipfile.ZipFile(path) as z:
        names = z.namelist()
        need(len(names) == len(set(names)) and len(names) <= 24, "EVIDENCE_DUPLICATES")
        need(all(re.fullmatch(r"[A-Za-z0-9_.-]+", n) for n in names), "EVIDENCE_MEMBER_UNSAFE")
        need(all(x.file_size <= 100_000 for x in z.infolist()) and
             sum(x.file_size for x in z.infolist()) <= 500_000, "EVIDENCE_DECOMPRESSION_BOUNDS")
        need(z.testzip() is None, "EVIDENCE_CRC")
        data = {n:z.read(n) for n in names}
    need("hashes.sha256" in data, "INDEX_MISSING")
    lines = data.pop("hashes.sha256").decode("ascii").splitlines()
    need(len(lines) == len(data), "INDEX_COUNT")
    checked=set()
    for line in lines:
        m = re.fullmatch(r"([A-Fa-f0-9]{64})  ([A-Za-z0-9_.-]+)", line)
        need(m is not None and m[2] in data and m[2] not in checked, "INDEX_ENTRY")
        need(sha(data[m[2]]) == m[1].upper(), "INDEX_HASH")
        checked.add(m[2])
    need(checked == set(data), "UNINDEXED_DATA")
    return data

def ssh_string(value):
    return len(value).to_bytes(4,"big") + value

def take(blob, offset):
    need(offset+4 <= len(blob), "SSHSIG_SHORT")
    size = int.from_bytes(blob[offset:offset+4],"big")
    offset += 4
    need(size <= 4096 and offset+size <= len(blob), "SSHSIG_SIZE")
    return blob[offset:offset+size],offset+size

def verify_sshsig(message, armor, allowed):
    begin=b"-----BEGIN SSH SIGNATURE-----\n"
    end=b"-----END SSH SIGNATURE-----"
    need(armor.startswith(begin) and end in armor and len(armor) < 2048, "SIGNATURE_ARMOR")
    raw=base64.b64decode(armor[len(begin):armor.index(end)].replace(b"\n",b""),validate=True)
    need(raw[:6]==b"SSHSIG" and int.from_bytes(raw[6:10],"big")==1, "SIGNATURE_VERSION")
    key,pos=take(raw,10)
    ns,pos=take(raw,pos)
    reserved,pos=take(raw,pos)
    hash_name,pos=take(raw,pos)
    sig,pos=take(raw,pos)
    need(pos==len(raw) and ns==SIGN_NAMESPACE and reserved==b"" and hash_name in (b"sha256",b"sha512"), "SIGNATURE_NAMESPACE")
    typ,j=take(key,0)
    pub,j=take(key,j)
    need(j==len(key) and typ==b"ssh-ed25519" and len(pub)==32, "SIGNATURE_KEY")
    typ,j=take(sig,0)
    bytesig,j=take(sig,j)
    need(j==len(sig) and typ==b"ssh-ed25519" and len(bytesig)==64, "SIGNATURE_ED25519")
    fp="SHA256:"+base64.b64encode(hashlib.sha256(key).digest()).decode().rstrip("=")
    need(fp==OWNER_FP, "SIGNER_FINGERPRINT")
    need(allowed.decode("ascii").strip()==PRINCIPAL+" ssh-ed25519 "+base64.b64encode(key).decode(), "SIGNER_NOT_IN_ACCEPTED_TRUST")
    from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
    from cryptography.exceptions import InvalidSignature
    v=Ed25519PublicKey.from_public_bytes(pub)
    def signed(data,namespace):
        h=hashlib.sha256 if hash_name==b"sha256" else hashlib.sha512
        return b"SSHSIG"+ssh_string(namespace)+ssh_string(b"")+ssh_string(hash_name)+ssh_string(h(data).digest())
    try:v.verify(bytesig,signed(message,ns))
    except InvalidSignature as err:raise Reject("BAD_OWNER_SIGNATURE") from err
    for m,n in ((message.replace(b"max_device_reset=1",b"max_device_reset=2"),ns),
                (message,b"deus-os-gate3-linux-public-trust-enrollment-v1")):
        try:v.verify(bytesig,signed(m,n))
        except InvalidSignature:continue
        raise Reject("NEGATIVE_SIGNATURE_ACCEPTED")
    return "ED25519_PASS"

def verify_plan(stage, lock_bytes):
    lock=parse_json(lock_bytes)
    need(lock.get("schema")=="DEUS_OS_GATE3_PHYSICAL_000D_SOURCE_LOCK_V1", "LOCK_SCHEMA")
    need(lock.get("operator_stage_package_id")==STAGE_ID and lock.get("operator_execution_profile")=="HARDWARE_MUTATING", "LOCK_PROFILE")
    need(lock.get("release_status")=="QUARANTINED_SOURCE_ONLY_NOT_RELEASABLE", "LOCK_QUARANTINE")
    need(lock.get("owner_signature_namespace")==SIGN_NAMESPACE.decode(), "LOCK_NAMESPACE")
    for k,v in {"allowed_runtime_physical_port":"usb:001:8","expected_before_pid":"000C",
                "expected_after_pid":"000D","allowed_reset_attempts":1,
                "allowed_readonly_info_attempts":1,
                "allowed_per_device_node_permission_fix":"ONE_TIME_NEW_000D_NODE_DEUS_0600_ONLY",
                "flash_erase_update_begin_data_end":"FORBIDDEN",
                "privilege_prompt":"VISIBLE_INTERACTIVE_BEFORE_RESET",
                "automatic_retry":"FORBIDDEN","automatic_return_reset":"FORBIDDEN",
                "persistent_udev":"FORBIDDEN","root_product_usb_protocol":"FORBIDDEN"}.items():
        need(lock.get(k)==v, "LOCK_SCOPE_"+k.upper())
    need(stage.is_dir() and not stage.is_symlink(), "STAGE_MISSING")
    need((stage/"QUARANTINED_NOT_FOR_RUN.txt").is_file(), "STAGE_QUARANTINE_MISSING")
    files=lock.get("source_files",{})
    need(isinstance(files,dict) and len(files)==14, "SOURCE_FILES_COUNT")
    for rel,v in files.items():
        need(rel in ("apply.ps1","bundle_spec.json") or
             (re.fullmatch(r"payload/[A-Za-z0-9_.-]+",rel) is not None), "SOURCE_PATH")
        need(isinstance(v,dict) and set(v)=={"size","sha256"} and
             type(v["size"]) is int and 0 < v["size"] <= 250_000 and
             isinstance(v["sha256"],str) and re.fullmatch("[a-fA-F0-9]{64}",v["sha256"]) is not None,
             "SOURCE_ENTRY_INVALID")
        p=stage/rel
        need(p.is_file() and not p.is_symlink() and not p.parent.is_symlink(), "SOURCE_MISSING")
        need(p.stat().st_size==v["size"] and sha(p.read_bytes())==v["sha256"], "SOURCE_DRIFT_"+rel)
    spec=parse_json((stage/"bundle_spec.json").read_bytes())
    need(spec.get("package_id")==STAGE_ID and spec.get("execution_profile")=="HARDWARE_MUTATING", "SPEC_ID")
    entry=(stage/"apply.ps1").read_text(encoding="utf-8")
    need("$PackageId='"+STAGE_ID+"'" in entry and "gate=$PackageId" in entry
         and "DEUS_RUN_RESULT_PATH" in entry, "RUNNER_CONTRACT")
    return len(files)

def verify_consent(message, lock_sha, now):
    need(len(message)<3000 and message.endswith(b"\n") and b"\r" not in message, "CONSENT_FORMAT")
    lines=message.decode("ascii").splitlines()
    need(lines[0]=="DEUS_OS_GATE3_ONE_RESET_NODE_INFO_OWNER_CONSENT_V1", "CONSENT_SCHEMA")
    fields={}
    for line in lines[1:]:
        need(line.count("=")==1, "CONSENT_LINE")
        k,v=line.split("=",1)
        need(re.fullmatch(r"[a-z][a-z0-9_]+",k) is not None and k not in fields, "CONSENT_DUPLICATE")
        fields[k]=v
    expected={"host_alias":"macmini","linux_user":"deus","physical_port":"usb:001:8",
              "usb_before_pid":"1209:000C","usb_after_pid":"1209:000D",
              "max_device_reset":"1","max_bootloader_readonly_info":"1",
              "new_000d_node_permission":"ONE_TIME_DEUS_0600",
              "pre_reset_privilege":"VISIBLE_INTERACTIVE_SUDO_V_BEFORE_RESET",
              "purpose":"ONE_RUNTIME_ENTER_BOOTLOADER_AND_ONE_READONLY_INFO_STOP",
              "firmware_flash":"NO","firmware_erase":"NO",
              "automatic_retry":"NO","automatic_return_reset":"NO",
              "owner_key_fingerprint":OWNER_FP,"signed_source_lock_sha256":lock_sha}
    need(set(fields)==set(expected)|{"expires_utc","nonce"}, "CONSENT_FIELD_SET")
    need(all(fields[k]==v for k,v in expected.items()), "CONSENT_SCOPE")
    need(re.fullmatch(r"[a-f0-9]{32}",fields["nonce"]) is not None, "CONSENT_NONCE")
    try:expiry=datetime.strptime(fields["expires_utc"],"%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
    except ValueError as err:raise Reject("CONSENT_EXPIRY_FORMAT") from err
    need(now<expiry<=now+timedelta(hours=26), "CONSENT_EXPIRED_OR_TOO_FAR")
    return expiry.isoformat()

def audit(stage,lock_path,consent_path,consent_sha,trust_path,now):
    need(now.tzinfo is not None, "NOW_MUST_BE_AWARE")
    need(lock_path.is_file() and not lock_path.is_symlink(), "LOCK_FILE_MISSING")
    lock=lock_path.read_bytes()
    lock_sha=sha(lock)
    count=verify_plan(stage,lock)
    signed=evidence(consent_path,consent_sha)
    trust=evidence(trust_path,TRUST_EVIDENCE_SHA)
    need(set(signed)=={"outcome.txt","native_checks.txt","run.log","signing_probe.txt","signing_probe.txt.sig"}, "OWNER_EVIDENCE_CONTENTS")
    allowed=trust.get("new_candidate_allowed_signers.txt",b"")
    need(sha(allowed)==TRUST_SHA, "OWNER_TRUST_SHA")
    msg=signed["signing_probe.txt"];sig=signed["signing_probe.txt.sig"]
    o=parse_json(signed["outcome.txt"])
    need(o.get("result")=="PASS" and o.get("classification")=="GATE3_000D_PHYSICAL_OWNER_CONSENT_SIGN_PASS", "OWNER_NATIVE_NOT_PASS")
    need(o.get("signature_namespace")==SIGN_NAMESPACE.decode() and o.get("selftest_namespace")==SIGN_NAMESPACE.decode(), "OWNER_NAMESPACE")
    need(o.get("signing_exit")=="0" and o.get("verification_exit")=="0"
         and o.get("tamper_exit")!="0" and o.get("wrong_namespace_exit")!="0", "OWNER_NATIVE_SIGNATURE_TESTS")
    need(o.get("challenge_sha256")==sha(msg) and o.get("signature_sha256")==sha(sig)
         and o.get("physical_source_lock_sha256")==lock_sha, "OWNER_RECEIPT_MISMATCH")
    expires=verify_consent(msg,lock_sha,now)
    verify_sshsig(msg,sig,allowed)
    return {"schema":"DEUS_GATE3_PERMANENT_AUDITOR_V1",
            "result":"PASS","classification":"OWNER_SIGNED_SOURCE_STATIC_ACCEPTED_NOT_HARDWARE_RELEASE",
            "package_id":STAGE_ID,"source_files":count,"source_lock_sha256":lock_sha,
            "owner_evidence_sha256":consent_sha.upper(),"expires_utc":expires,
            "signature":"ED25519_VERIFIED","hardware_release":"DENIED_UNTIL_SEPARATE_NATIVE_GATE",
            "reset":"NOT_EXECUTED","full_gate3":"NOT_ACCEPTED"}

def inspect_candidate(bundle_path, expected_sha, stage, source_lock, guard_root=None):
    """Static-only canonical ZIP shape + byte-exact signed entrypoint check.

    This is NOT a signed executable release decision.
    """
    need(bundle_path.is_file() and not bundle_path.is_symlink(), "CANDIDATE_MISSING")
    need(re.fullmatch(r"[A-Fa-f0-9]{64}", expected_sha) is not None, "CANDIDATE_SHA_REQUIRED")
    need(sha(bundle_path.read_bytes()) == expected_sha.upper(), "CANDIDATE_SHA_MISMATCH")
    import sys as _sys
    _guard = guard_root or Path.cwd() / "scripts/operator_bundle_guard"
    _sys.path.insert(0, str(_guard))
    try:
        import bundle_guard
        validation = bundle_guard.validate_file(bundle_path, expected_sha.upper())
    finally:
        _sys.path.pop(0)
    need(validation.get("profile") == "HARDWARE_MUTATING", "CANDIDATE_WRONG_PROFILE")
    with zipfile.ZipFile(bundle_path) as z:
        names=set(z.namelist())
        allowed={"apply.ps1","PACKAGE_README.txt","hashes.sha256","manifest.json"}
        approved={name for name in source_lock["source_files"] if name.startswith("payload/")}
        need(names == allowed | approved, "CANDIDATE_EXTRA_OR_MISSING_CODE")
        manifest=parse_json(z.read("manifest.json"))
        need(manifest.get("package_id")==STAGE_ID, "CANDIDATE_PACKAGE_ID_DRIFT")
        need(z.read("apply.ps1")==(stage/"apply.ps1").read_bytes(), "CANDIDATE_APPLY_NOT_SIGNED")
        for member in approved:
            need(z.read(member)==(stage/member).read_bytes(), "CANDIDATE_PAYLOAD_NOT_SIGNED")
    return {"candidate_zip_sha256":expected_sha.upper(),
            "candidate_review":"BYTE_EXACT_SIGNED_SOURCE_STATIC_ONLY",
            "physical_release":"STILL_NOT_AUTHORIZED"}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--os-root",type=Path,default=Path(__file__).resolve().parents[2])
    p.add_argument("--stage",type=Path)
    p.add_argument("--source-lock",type=Path)
    p.add_argument("--owner-evidence",type=Path)
    p.add_argument("--owner-evidence-sha256")
    p.add_argument("--policy",type=Path)
    p.add_argument("--trust-evidence",type=Path)
    p.add_argument("--candidate-zip",type=Path)
    p.add_argument("--candidate-sha256")
    a=p.parse_args()
    w=a.os_root.resolve().parent
    try:
        policy_file=a.policy or a.os_root.resolve()/"scripts/gate3_operator/policy.json"
        policy=parse_json(policy_file.read_bytes())
        need(policy.get("schema")=="DEUS_GATE3_OPERATOR_AUDIT_POLICY_V1", "AUDIT_POLICY_SCHEMA")
        pinned=policy.get("current_owner_consent_evidence_sha256")
        need(isinstance(pinned,str) and re.fullmatch(r"[A-Fa-f0-9]{64}",pinned) is not None, "AUDIT_POLICY_SHA")
        consent_sha=a.owner_evidence_sha256 or pinned
        outcome=audit(a.stage or w/"Bundle"/("_stage_"+STAGE_ID),
                a.source_lock or w/"Bundle/_stage_stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027/payload/physical_000d_source_lock.json",
                a.owner_evidence or w/"Evidence/stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027_20261027_234630_623699d0508b48d1a47ad8441565c32b.evidence.zip",
                consent_sha,
                a.trust_evidence or w/"Evidence/stm32_os_gate3_owner_existing_candidate_readonly_sign_v1_20261027_20261027_210632_df8435a546b44d5993fa34b3f0fb2770.evidence.zip",
                datetime.now(timezone.utc))
        if a.candidate_zip is not None:
            need(a.candidate_sha256 is not None, "CANDIDATE_SHA_REQUIRED")
            stage = a.stage or w/"Bundle"/("_stage_"+STAGE_ID)
            source = a.source_lock or w/"Bundle/_stage_stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027/payload/physical_000d_source_lock.json"
            outcome.update(inspect_candidate(a.candidate_zip,a.candidate_sha256,stage,parse_json(source.read_bytes()),a.os_root.resolve()/"scripts/operator_bundle_guard"))
        print(json.dumps(outcome,sort_keys=True));return 0
    except (Reject,OSError,ValueError,UnicodeError,TypeError,KeyError,zipfile.BadZipFile) as ex:
        label=str(ex) if isinstance(ex,Reject) else type(ex).__name__
        print(json.dumps({"result":"FAIL","classification":label,
                          "hardware_release":"DENIED","reset":"NOT_EXECUTED"}));return 2

if __name__=="__main__":
    sys.exit(main())
