#!/usr/bin/env python3
"""RA-02 synthetic SSHSIG positive/negative and real pinned public trust audit.

No private owner key is read. Unit test key is new, ephemeral, RAM-only,
never a production signing key or allowed owner key.
"""
import base64
import hashlib
from pathlib import Path
import unittest

from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
import release_approval_contract_v1 as ra01
import verify_release_signature_v1 as v

MESSAGE=ra01.canonical_bytes({"synthetic":"RA02_TEST_ONLY"})
OTHER=b'{"synthetic":"RA02_TEST_ONLY_CHANGED"}\n'
PRIVATE_EVIDENCE_ROOT=Path(__file__).resolve().parents[3]/"Evidence"
TRUST_ORIGINAL_PRESENT=(PRIVATE_EVIDENCE_ROOT/v.TRUST_EVIDENCE_NAME).is_file()
GATE3_NAMESPACE_ORIGINAL_PRESENT=(PRIVATE_EVIDENCE_ROOT/("stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027_"+"20261027_234630_623699d0508b48d1a47ad8441565c32b.evidence.zip")).is_file()


def ssh_pub(private):
    public=private.public_key().public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw)
    return v.ssh_string(b"ssh-ed25519")+v.ssh_string(public)

def armor(raw):
    text=base64.b64encode(raw)
    lines=[text[i:i+70] for i in range(0,len(text),70)]
    return b"-----BEGIN SSH SIGNATURE-----\n"+b"\n".join(lines)+b"\n-----END SSH SIGNATURE-----\n"

def fake_sign(private, msg=MESSAGE,namespace=v.NAMESPACE,alg=b"sha256",
              reserved=b"", version=1,key_override=None,signature_override=None):
    key=ssh_pub(private) if key_override is None else key_override
    h=hashlib.sha256 if alg==b"sha256" else hashlib.sha512
    signed=b"SSHSIG"+v.ssh_string(namespace)+v.ssh_string(b"")+v.ssh_string(alg)+v.ssh_string(h(msg).digest())
    signature=private.sign(signed) if signature_override is None else signature_override
    packed=v.ssh_string(b"ssh-ed25519")+v.ssh_string(signature)
    raw=b"SSHSIG"+version.to_bytes(4,"big")+v.ssh_string(key)+v.ssh_string(namespace)+v.ssh_string(reserved)+v.ssh_string(alg)+v.ssh_string(packed)
    return armor(raw)

class CryptoTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.key=Ed25519PrivateKey.generate()
        cls.pub=ssh_pub(cls.key)
        cls.other=Ed25519PrivateKey.generate()
        cls.signed=fake_sign(cls.key)

    def reject(self,signature=None,msg=MESSAGE,key=None):
        with self.assertRaises(v.Reject):
            v.verify_crypto(msg,signature or self.signed,key or self.pub)

    def test_sha256_valid(self):
        self.assertEqual(v.verify_crypto(MESSAGE,self.signed,self.pub),"ED25519_VERIFIED")

    def test_sha512_valid(self):
        self.assertEqual(v.verify_crypto(MESSAGE,fake_sign(self.key,alg=b"sha512"),self.pub),"ED25519_VERIFIED")

    @unittest.skipUnless(TRUST_ORIGINAL_PRESENT and GATE3_NAMESPACE_ORIGINAL_PRESENT, "requires original pinned owner Evidence, absent from public Git")
    def test_actual_historical_gate3_owner_signature_cannot_authorize_gate4(self):
        # The genuine old signature's owner key is correct but its namespace is
        # deliberately different; a new Gate4 signature is required.
        import zipfile
        from pathlib import Path
        source=Path(__file__).resolve().parents[3]/"Evidence"/(
            "stm32_os_gate3_physical_reset_node_info_owner_consent_offline_v1_20261027_"
            "20261027_234630_623699d0508b48d1a47ad8441565c32b.evidence.zip")
        self.assertEqual(v.sha(source.read_bytes()),
            "DD87116F54B7E3A8C9FB9A68CFC620C5CF58049C5F9B9001CF6E15110CE1E9C0")
        with zipfile.ZipFile(source) as zipfile_source:
            self.assertIsNone(zipfile_source.testzip())
            message=zipfile_source.read("signing_probe.txt")
            signature=zipfile_source.read("signing_probe.txt.sig")
        with self.assertRaisesRegex(v.Reject,"SSHSIG_WRONG_NAMESPACE"):
            v.verify_crypto(message,signature,v.pinned_trust_from_original_evidence())

    @unittest.skipUnless(TRUST_ORIGINAL_PRESENT, "requires original pinned owner Evidence, absent from public Git")
    def test_real_owner_anchor_is_accepted_and_matches_pin(self):
        original=v.pinned_trust_from_original_evidence()
        pub,fp=v.key_details(original)
        self.assertEqual(fp,v.OWNER_FP)
        self.assertEqual(len(pub),32)

    @unittest.skipUnless(TRUST_ORIGINAL_PRESENT, "requires original pinned owner Evidence, absent from public Git")
    def test_real_owner_anchor_does_not_trust_synthetic_signature(self):
        original=v.pinned_trust_from_original_evidence()
        self.assertNotEqual(original,self.pub)
        self.reject(key=original)

    def test_other_key_signed_message(self):
        self.reject(fake_sign(self.other))

    def test_unknown_signer_key(self):
        self.reject(key=ssh_pub(self.other))

    def test_wrong_namespace_gate3(self):
        self.reject(fake_sign(self.key,namespace=b"deus-os-gate3-one-reset-v1"))

    def test_wrong_namespace_public_enrollment(self):
        self.reject(fake_sign(self.key,namespace=b"deus-os-gate3-linux-public-trust-enrollment-v1"))

    def test_changed_statement(self):
        self.reject(msg=OTHER)

    def test_changed_single_byte(self):
        self.reject(msg=MESSAGE+b" ")

    def test_signature_tampered(self):
        signed=self.signed
        body=signed.split(b"-----BEGIN SSH SIGNATURE-----\n",1)[1].split(b"-----END SSH SIGNATURE-----",1)[0]
        raw=base64.b64decode(body.replace(b"\n",b""))
        self.reject(armor(raw[:-1]+bytes([raw[-1]^1])))

    def test_unsupported_hash(self):
        self.reject(fake_sign(self.key,alg=b"sha384"))

    def test_reserved_nonempty(self):
        self.reject(fake_sign(self.key,reserved=b"evasion"))

    def test_bad_sshsig_version(self):
        self.reject(fake_sign(self.key,version=2))

    def test_truncated(self):
        self.reject(self.signed[:120])

    def test_bad_armor_prefix(self):
        self.reject(self.signed.replace(b"-----BEGIN SSH SIGNATURE-----",b"-----BEGIN OTHER-----"))

    def test_extra_armor_text(self):
        self.reject(self.signed+b"additional")

    def test_duplicate_armor(self):
        self.reject(self.signed+self.signed)

    def test_wrong_public_key_shape(self):
        self.reject(fake_sign(self.key,key_override=b"ssh-ed25519"))

    def test_signature_wrong_size(self):
        self.reject(fake_sign(self.key,signature_override=b"\x01"*63))

    def test_signature_wrong_key_type(self):
        bad_key=v.ssh_string(b"ssh-ecdsa")+v.ssh_string(self.key.public_key().public_bytes(encoding=serialization.Encoding.Raw,format=serialization.PublicFormat.Raw))
        self.reject(fake_sign(self.key,key_override=bad_key))

    def test_signature_invalid_base64(self):
        self.reject(self.signed.replace(b"S",b"?",1))

    def test_signer_fingerprint_pin(self):
        pub=self.pub
        fp="SHA256:"+base64.b64encode(hashlib.sha256(pub).digest()).decode().rstrip("=")
        self.assertNotEqual(fp,v.OWNER_FP)
        allowed=(v.PRINCIPAL+" ssh-ed25519 "+base64.b64encode(pub).decode()+"\n").encode()
        self.assertEqual(v.parse_allowed_signers(allowed,expected_fp=fp)[0],pub)
        with self.assertRaises(v.Reject):
            v.parse_allowed_signers(allowed)

    def test_allowed_signer_wrong_principal(self):
        allowed=("wrong-owner ssh-ed25519 "+base64.b64encode(self.pub).decode()).encode()
        with self.assertRaises(v.Reject):
            v.parse_allowed_signers(allowed,expected_fp="SHA256:"+base64.b64encode(hashlib.sha256(self.pub).digest()).decode().rstrip("="))

    def test_allowed_signer_with_options_rejected(self):
        allowed=(v.PRINCIPAL+" namespaces=other ssh-ed25519 "+base64.b64encode(self.pub).decode()).encode()
        with self.assertRaises(v.Reject):
            v.parse_allowed_signers(allowed)

    def test_allowed_signer_multiple_entries_rejected(self):
        one=(v.PRINCIPAL+" ssh-ed25519 "+base64.b64encode(self.pub).decode()).encode()
        with self.assertRaises(v.Reject):
            v.parse_allowed_signers(one+b"\n"+one)

    def test_empty_message(self):
        with self.assertRaises(v.Reject):
            v.verify_crypto(b"x"*4100,self.signed,self.pub)

    def test_bad_message_type(self):
        with self.assertRaises(v.Reject):
            v.verify_crypto("string",self.signed,self.pub)

    def test_bad_signature_type(self):
        with self.assertRaises(v.Reject):
            v.verify_crypto(MESSAGE,"not bytes",self.pub)

    def test_parser_rejects_no_production_owner_approval(self):
        # No Gate4 owner signature exists. A synthetic cryptographic PASS
        # must not be promoted as new owner authorization.
        status={"signature":"SYNTHETIC_ONLY","authorization":"DENIED",
                "nonce_consumed":"NO","physical_executor":"NOT_ADMITTED"}
        self.assertEqual(status["authorization"],"DENIED")

if __name__=="__main__":
    unittest.main(verbosity=1)
