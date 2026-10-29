#!/usr/bin/env python3
"""Separate scoped Ed25519 owner SIGNED admission for exactly ONE user-local
Mac mini initial public trust installation. No generic hardware mutating release.
NO remote calls, no private keys, no USB, sudo, Flash or reset.
"""
from pathlib import Path
from datetime import datetime,timezone,timedelta
import base64,hashlib,importlib.util,json,sys,zipfile
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from cryptography.exceptions import InvalidSignature

sys.path.insert(0,str(Path("scripts/operator_bundle_guard").resolve()))
from bundle_guard import Invalid,validate_file,release

pkg="stm32_os_gate3_macmini_owner_public_trust_enrollment_v1_20261027"
archive=Path("../Bundle")/(pkg+".zip")
expected_zip="8A4202D8E5353FF7949DA7EE4A2BE2560F3E86F1DD6C26336414F72F90E9C8A9"
expected_apply="D95A204F66B21B24B0B56F4F134519FC22553FA998FF2E474BFF54692F1BBF70"
source_sha="CC2A49A3776CAF47996D0EE262912DB0CB21CBCFD9662F943469E10DBAE220B9"
consent_sha="7F0D964BE452989A374D9241806DC66FE5616D2394BEB06244C2E62D5D0217C7"
signature_sha="A10C69307702CF4C3718FF6BB6C4D46E3E8B886A00F86B1D3524BE196DDA1269"
signed_evidence=Path("../Evidence/stm32_os_gate3_macmini_public_trust_owner_consent_sign_offline_v1_20261027_20261027_222153_151bbb8fd3794865af81d8e54c8906de.evidence.zip")
evidence_sha="81EC2C3CFE13F51B86F5B0C6FD35CC2284B84A71C2B6FB7D057EF0D189544B72"
public_evidence=Path("../Evidence/stm32_os_gate3_owner_existing_candidate_readonly_sign_v1_20261027_20261027_210632_df8435a546b44d5993fa34b3f0fb2770.evidence.zip")
public_evidence_sha="C4E5A40BE4DB9C703F00B0286FDC1E3299DDD4032FBE2D6767005F9399FD3227"

def sha(b):return hashlib.sha256(b).hexdigest().upper()
def need(cond,label):
    if not cond:raise AssertionError(label)
def pack(b):return len(b).to_bytes(4,"big")+b
def pop(b,pos):
    need(pos+4<=len(b),"TRUNCATED_SSHSIG")
    size=int.from_bytes(b[pos:pos+4],"big");pos+=4
    need(0<=size<4096 and pos+size<=len(b),"SSHSIG_BOUNDS")
    return b[pos:pos+size],pos+size
def parse_public_signature(signature):
    need(signature.startswith(b"-----BEGIN SSH SIGNATURE-----\n"),"SSHSIG_ARMOR")
    payload=signature.split(b"-----BEGIN SSH SIGNATURE-----\n",1)[1].split(b"-----END SSH SIGNATURE-----",1)[0].replace(b"\n",b"")
    decoded=base64.b64decode(payload,validate=True)
    need(decoded[:6]==b"SSHSIG" and int.from_bytes(decoded[6:10],"big")==1,"SSHSIG_VERSION")
    key,pos=pop(decoded,10)
    ns,pos=pop(decoded,pos)
    reserved,pos=pop(decoded,pos)
    h,pos=pop(decoded,pos)
    sigblob,pos=pop(decoded,pos)
    need(pos==len(decoded) and reserved==b"" and h in (b"sha256",b"sha512"),"SSHSIG_METADATA")
    alg,q=pop(key,0)
    keybytes,q=pop(key,q)
    need(q==len(key) and alg==b"ssh-ed25519" and len(keybytes)==32,"PUBLIC_KEY_FORMAT")
    sigalg,q=pop(sigblob,0)
    sigbytes,q=pop(sigblob,q)
    need(q==len(sigblob) and sigalg==b"ssh-ed25519" and len(sigbytes)==64,"SSHSIG_ED25519")
    return key,ns,h,keybytes,sigbytes

def main():
    gate=validate_file(archive,expected_zip)
    need(gate["profile"]=="REMOTE_PUBLIC_TRUST_MUTATING" and gate["apply_sha256"]==expected_apply,"ZIP_AND_APPLY_SCOPE_DRIFT")
    with zipfile.ZipFile(archive) as z:
        names=set(z.namelist())
        need(names=={"apply.ps1","manifest.json","hashes.sha256","PACKAGE_README.txt",
                    "payload/macmini_initial_owner_public_trust_install_source_only.py",
                    "payload/signed_owner_consent.txt","payload/signed_owner_consent.sshsig"},"ZIP_SCOPE_MEMBER_DRIFT")
        manifest=json.loads(z.read("manifest.json"))
        need(manifest["workflow_id"]=="gate3_macmini_owner_public_trust_enrollment","WRONG_WORKFLOW")
        need(manifest["owner_authorization_namespace"]=="deus-os-gate3-linux-public-trust-enrollment-v1","WRONG_NAMESPACE")
        need(manifest["authorization_profile"]=="SIGNED_OWNER_APPROVAL_REQUIRED","NO_SIGNED_APPROVAL")
        raw=z.read("payload/macmini_initial_owner_public_trust_install_source_only.py")
        msg=z.read("payload/signed_owner_consent.txt")
        sig=z.read("payload/signed_owner_consent.sshsig")
        need(sha(raw)==source_sha and sha(msg)==consent_sha and sha(sig)==signature_sha,"SIGNED_CONTENT_SHA_CHANGED")
    need(sha(signed_evidence.read_bytes())==evidence_sha,"OWNER_ORIGINAL_EVIDENCE_SHA_DRIFT")
    with zipfile.ZipFile(signed_evidence) as z:
        need(z.testzip() is None,"OWNER_EVIDENCE_CRC_FAIL")
        need(z.read("signing_probe.txt")==msg and z.read("signing_probe.txt.sig")==sig,"SIGNED_PROOF_BYTES_NOT_ORIGINAL")
        owner_out=json.loads(z.read("outcome.txt"))
        need(owner_out["result"]=="PASS" and owner_out["signature_namespace"]=="deus-os-gate3-linux-public-trust-enrollment-v1","OWNER_CONSENT_NATIVE_NOT_ACCEPTED")
    need(sha(public_evidence.read_bytes())==public_evidence_sha,"OWNER_PUBLIC_EVIDENCE_SHA_DRIFT")
    with zipfile.ZipFile(public_evidence) as z:
        public=z.read("new_candidate_allowed_signers.txt")
    need(sha(public)=="AFF18C4CB818D1637268787FE7F8834CABD56ACDC5615772E374AB907CF15069","PUBLIC_TRUST_BYTES_NOT_ACCEPTED")
    spec=importlib.util.spec_from_file_location("deus_remote_install",Path("build/gate3_mutating_owner_authorization_v1/macmini_initial_owner_public_trust_install_source_only.py"))
    module=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    need(sha(raw)==sha(Path(spec.origin).read_bytes()),"REMOTE_SOURCE_PIN_DRIFT")
    consent=module.validate_consent(msg,source_sha,public)
    expiration=datetime.strptime(consent["expires_utc"],"%Y-%m-%dT%H:%M:%SZ").replace(tzinfo=timezone.utc)
    need(datetime.now(timezone.utc)<expiration<=datetime.now(timezone.utc)+timedelta(hours=26),"OWNER_APPROVAL_EXPIRED")
    key,namespace,hashalg,keybytes,sigbytes=parse_public_signature(sig)
    need(namespace==b"deus-os-gate3-linux-public-trust-enrollment-v1","WRONG_BINARY_SSHSIG_NAMESPACE")
    pub_fingerprint="SHA256:"+base64.b64encode(hashlib.sha256(key).digest()).decode().rstrip("=")
    need(pub_fingerprint==module.FINGERPRINT,"PUBLIC_OWNER_KEY_DIFFERENT")
    encoded_key=base64.b64encode(key).decode()
    need(public.decode("ascii").strip()=="deus-os-hardware-owner ssh-ed25519 "+encoded_key,"PUBLIC_TRUST_SIGNATURE_KEY_MISMATCH")
    def signed_bytes(m,ns):
        d=hashlib.sha512 if hashalg==b"sha512" else hashlib.sha256
        return b"SSHSIG"+pack(ns)+pack(b"")+pack(hashalg)+pack(d(m).digest())
    verifier=Ed25519PublicKey.from_public_bytes(keybytes)
    verifier.verify(sigbytes,signed_bytes(msg,namespace))
    for altered,other_ns in [(msg.replace(b"linux_user=deus",b"linux_user=root"),namespace),
                             (msg,b"deus-os-gate3-one-reset-v1")]:
        try:verifier.verify(sigbytes,signed_bytes(altered,other_ns))
        except InvalidSignature:pass
        else:raise AssertionError("OWNER_SCOPE_OR_NAMESPACE_TAMPER_ACCEPTED")
    # Generic release MUST remain a red gate even with this exact signed package.
    try:release(archive,expected_zip,Path("/this_is_not_a_receipt"))
    except Invalid as e:need(str(e)=="MUTATING_RELEASE_REQUIRES_SEPARATE_SIGNED_GATE","GENERIC_MUTATING_GATE_DRIFT")
    else:raise AssertionError("UNSIGNED_GENERIC_RELEASE_BYPASS")
    print(json.dumps({
        "result":"PASS",
        "classification":"EXACT_OWNER_SIGNED_PUBLIC_TRUST_SCOPE_ADMITTED_STATIC_NOT_NATIVE",
        "package_id":pkg,
        "zip_sha256":expected_zip,
        "apply_sha256":expected_apply,
        "public_install_source_sha256":source_sha,
        "original_signed_evidence_sha256":evidence_sha,
        "signed_consent_message_sha256":consent_sha,
        "signed_consent_signature_sha256":signature_sha,
        "ssh_signature":"ED25519_INDEPENDENT_PASS",
        "tamper_and_reset_namespace":"REJECTED",
        "expires_utc":consent["expires_utc"],
        "generic_release":"STILL_DENIED",
        "windows_native_ast":"NOT_VERIFIED_BY_THIS_GATE",
        "linux_installed":"NO",
        "usb_reset_flash":"NO"},sort_keys=True))
if __name__=="__main__": main()
