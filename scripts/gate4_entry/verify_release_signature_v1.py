#!/usr/bin/env python3
"""Gate4 RA-02 SOURCE ONLY: verify Ed25519 SSHSIG on exact RA-01 canonical bytes.

No signing, no nonce consumption, no hardware admission or I/O. The public trust
anchor is recovered only from a SHA-pinned ORIGINAL Gate3 Windows evidence ZIP,
NOT from the signed statement, signature file, bundle or passed-in public key.
"""
from __future__ import annotations
import base64
import binascii
import hashlib
from pathlib import Path
import re
import zipfile

import release_approval_contract_v1 as contract

TRUST_EVIDENCE_NAME = (
    "stm32_os_gate3_owner_existing_candidate_readonly_sign_v1_20261027_"
    "20261027_210632_df8435a546b44d5993fa34b3f0fb2770.evidence.zip"
)
TRUST_EVIDENCE_SHA = "C4E5A40BE4DB9C703F00B0286FDC1E3299DDD4032FBE2D6767005F9399FD3227"
ALLOWED_SIGNERS_SHA = "AFF18C4CB818D1637268787FE7F8834CABD56ACDC5615772E374AB907CF15069"
PRINCIPAL = "deus-os-hardware-owner"
OWNER_FP = "SHA256:4Ze7JMXNBsGEmhG/j+VRm9+X+a/C3dVlLSk59MU440M"
NAMESPACE = b"deus-os-gate4-hardware-release-v1"

class Reject(contract.Reject):
    pass

def need(ok, code):
    if not ok:
        raise Reject(code)

def sha(data):
    return hashlib.sha256(data).hexdigest().upper()

def ssh_string(data):
    return len(data).to_bytes(4, "big") + data

def take(blob, position, limit=4096):
    need(position+4 <= len(blob), "SSHSIG_TRUNCATED")
    length=int.from_bytes(blob[position:position+4], "big")
    position+=4
    need(length<=limit and position+length<=len(blob), "SSHSIG_FIELD_SIZE")
    return blob[position:position+length], position+length

def key_details(ssh_blob):
    kind,pos=take(ssh_blob,0,64)
    pub,pos=take(ssh_blob,pos,32)
    need(pos==len(ssh_blob) and kind==b"ssh-ed25519" and len(pub)==32,
         "OWNER_PUBLIC_KEY_FORMAT")
    fingerprint="SHA256:"+base64.b64encode(hashlib.sha256(ssh_blob).digest()).decode("ascii").rstrip("=")
    return pub,fingerprint

def parse_allowed_signers(blob, expected_fp=OWNER_FP):
    need(type(blob) is bytes and 1<=len(blob)<512, "ALLOWED_SIGNERS_SIZE")
    try:
        line=blob.decode("ascii").strip()
    except UnicodeError as ex:
        raise Reject("ALLOWED_SIGNERS_ENCODING") from ex
    match=re.fullmatch(r"([A-Za-z0-9_-]+) ssh-ed25519 ([A-Za-z0-9+/]+={0,2})",line)
    need(match is not None and match[1]==PRINCIPAL,"ALLOWED_SIGNERS_SCOPE")
    try:
        ssh_blob=base64.b64decode(match[2].encode("ascii"),validate=True)
    except binascii.Error as ex:
        raise Reject("ALLOWED_SIGNERS_BASE64") from ex
    pub,fp=key_details(ssh_blob)
    need(fp==expected_fp,"OWNER_FINGERPRINT")
    return ssh_blob,pub,fp

def pinned_trust_from_original_evidence(root=None):
    """Byte-verify the original independent Windows evidence first."""
    if root is None:
        root=Path(__file__).resolve().parents[3]
    root=Path(root)
    evidence=root/"Evidence"/TRUST_EVIDENCE_NAME
    need(evidence.is_file() and not evidence.is_symlink(),"TRUST_ORIGINAL_MISSING")
    need(0<evidence.stat().st_size<1_000_000,"TRUST_ORIGINAL_SIZE")
    need(sha(evidence.read_bytes())==TRUST_EVIDENCE_SHA,"TRUST_ORIGINAL_SHA")
    with zipfile.ZipFile(evidence) as z:
        infos=z.infolist()
        names=[v.filename for v in infos]
        need(len(names)<=24 and len(names)==len(set(names)) and
             all(re.fullmatch(r"[A-Za-z0-9_.-]+",name) for name in names),
             "TRUST_MEMBER_NAMES")
        need(sum(v.file_size for v in infos)<250_000 and
             all(v.file_size<100_000 and not v.is_dir() and not(v.flag_bits&1)
                 for v in infos),"TRUST_MEMBER_SIZE")
        need(z.testzip() is None,"TRUST_CRC")
        data={name:z.read(name) for name in names}
    need("hashes.sha256" in data and "new_candidate_allowed_signers.txt" in data,
         "TRUST_MEMBERS_REQUIRED")
    rows=data["hashes.sha256"].decode("ascii").splitlines()
    seen=set()
    need(len(rows)==len(data)-1,"TRUST_INDEX_COUNT")
    for row in rows:
        m=re.fullmatch(r"([A-Fa-f0-9]{64})  ([A-Za-z0-9_.-]+)",row)
        need(m is not None and m[2] in data and m[2]!="hashes.sha256"
             and m[2] not in seen,"TRUST_INDEX_NAME")
        need(sha(data[m[2]])==m[1].upper(),"TRUST_INDEX_SHA")
        seen.add(m[2])
    need(seen==set(data)-{"hashes.sha256"},"TRUST_INDEX_COVERAGE")
    allowed=data["new_candidate_allowed_signers.txt"]
    need(sha(allowed)==ALLOWED_SIGNERS_SHA,"TRUST_ALLOWED_SHA")
    key,pub,fp=parse_allowed_signers(allowed)
    need(fp==OWNER_FP,"TRUST_OWNER_FINGERPRINT")
    return key

def parse_armor(armor):
    need(type(armor) is bytes and 100<=len(armor)<=2048,"SSHSIG_ARMOR_SIZE")
    begin=b"-----BEGIN SSH SIGNATURE-----\n"
    end=b"-----END SSH SIGNATURE-----"
    need(armor.startswith(begin) and armor.count(begin)==1 and armor.count(end)==1
         and (armor.endswith(end+b"\n") or armor.endswith(end))
         and b"\r" not in armor,"SSHSIG_ARMOR")
    body=armor[len(begin):].split(end,1)[0]
    lines=body.split(b"\n")
    need(lines[-1]==b"" and all(0<len(row)<=80 and
         re.fullmatch(rb"[A-Za-z0-9+/]+={0,2}",row) is not None
         for row in lines[:-1]),"SSHSIG_BASE64_LINES")
    try:
        raw=base64.b64decode(b"".join(lines[:-1]),validate=True)
    except binascii.Error as ex:
        raise Reject("SSHSIG_BASE64_INVALID") from ex
    need(len(raw)<=1024 and raw[:6]==b"SSHSIG" and
         int.from_bytes(raw[6:10],"big")==1,"SSHSIG_MAGIC_VERSION")
    key,pos=take(raw,10,128)
    namespace,pos=take(raw,pos,128)
    reserved,pos=take(raw,pos,128)
    hash_alg,pos=take(raw,pos,32)
    signature,pos=take(raw,pos,256)
    need(pos==len(raw) and reserved==b"" and
         hash_alg in (b"sha256",b"sha512"),"SSHSIG_ENVELOPE")
    kind,j=take(signature,0,64)
    sig,j=take(signature,j,64)
    need(j==len(signature) and kind==b"ssh-ed25519" and len(sig)==64,
         "SSHSIG_ED25519")
    return key,namespace,hash_alg,sig

def verify_crypto(message,armor,expected_key,namespace=NAMESPACE):
    """Pure verifier testable with ephemeral unit-test keys; no release decision."""
    need(type(message) is bytes and len(message)<=4096,"SSHSIG_MESSAGE_SIZE")
    key,scope,hash_alg,sig=parse_armor(armor)
    need(scope==namespace and namespace==NAMESPACE,"SSHSIG_WRONG_NAMESPACE")
    need(key==expected_key,"SSHSIG_SIGNER_NOT_TRUSTED")
    pub,_=key_details(key)
    h=hashlib.sha256 if hash_alg==b"sha256" else hashlib.sha512
    signed=b"SSHSIG"+ssh_string(scope)+ssh_string(b"")+ssh_string(hash_alg)+ssh_string(h(message).digest())
    try:
        from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
        from cryptography.exceptions import InvalidSignature
        Ed25519PublicKey.from_public_bytes(pub).verify(sig,signed)
    except ImportError as ex:
        raise Reject("CRYPTOGRAPHY_MISSING") from ex
    except (InvalidSignature,ValueError) as ex:
        raise Reject("INVALID_OWNER_SIGNATURE") from ex
    return "ED25519_VERIFIED"

def audit_source_only(message,armor,expected_material,now):
    """No authorization path: even successful owner SSHSIG is not a release."""
    contract.inspect_source_only(message,expected_material,now)
    trusted_key=pinned_trust_from_original_evidence()
    verify_crypto(message,armor,trusted_key)
    return {
       "result":"PASS","classification":"RA02_SIGNED_SOURCE_FORMAT_ONLY",
       "statement_sha256":sha(message),"signer_fingerprint":OWNER_FP,
       "signature":"ED25519_VERIFIED","authorization":"DENIED",
       "nonce_consumed":"NO","controller":"NOT_INSTALLED",
       "physical_executor":"NOT_ADMITTED","device_io":"NONE"
    }
