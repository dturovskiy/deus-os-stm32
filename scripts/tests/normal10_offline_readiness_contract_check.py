#!/usr/bin/env python3
"""Source-only checks for the normal10 prep; never touches ProtectedRecovery or hardware."""
from __future__ import annotations

import hashlib
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LOCK = ROOT / "scripts/normal10_release_source_lock_20261111.json"
SWD = ROOT / "scripts/normal10_swd_readonly_backup.ps1"
BOOT = ROOT / "bootloader/bootloader.c"
SIGNER = ROOT / "scripts/create_firmware_update_package.ps1"


def need(cond: bool, detail: str) -> None:
    if not cond:
        raise AssertionError(detail)
    print("PASS " + detail)


def digest(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest().upper()


def main() -> None:
    lock = json.loads(LOCK.read_text(encoding="utf-8"))
    need(lock["schema"] == "DEUS_NORMAL10_SOURCE_ONLY_RELEASE_LOCK_V1", "schema")
    need(lock["scope"] == "SOURCE_ONLY_NO_SIGNING_NO_HARDWARE", "scope")
    boot_bytes = BOOT.read_bytes()
    need(digest(boot_bytes) == lock["reference_bootloader_source_sha256"], "bootloader_source_sha256")
    boot_text = boot_bytes.decode("utf-8")
    need("if(!explicit_update && committed_version!=0u)handoff_app();" in boot_text,
         "valid_boot_auto_handoff_requires_oob_recovery")
    need("if(cand<=version_floor)return ST_VERSION_REJECTED;" in boot_text and
         "if(cand<version_floor)return ST_VERSION_REJECTED;" in boot_text,
         "version_floor_exact_admission_rules")
    package = lock["package_format"]
    need(package["authenticated_domain"] == "DEUSHDR1", "hmac_domain")
    need(package["header_bytes"] == 48 and package["hmac_tag_bytes"] == 32, "package_geometry")
    cases = lock["candidates"]
    need([c["role"] for c in cases] == ["normal10", "rescue11", "normal12"], "roles")
    need([c["version"] for c in cases] == [10, 11, 12], "version_order")
    for c in cases:
        image_digest = bytes.fromhex(c["image_sha256"])
        need(len(image_digest) == 32, c["role"] + "_image_digest_length")
        image_len = c["image_length"]
        need(image_len % 4 == 0 and 8 <= image_len <= 53248, c["role"] + "_image_bounds")
        header = struct.pack("<IBBHII", 0x534F4544, 1, 0, 0x0410,
                             image_len, c["version"]) + image_digest
        need(len(header) == 48, c["role"] + "_header_length")
        need(digest(header) == c["public_header_sha256"], c["role"] + "_public_header_sha256")
        need(c["expected_package_bytes"] == image_len + 80, c["role"] + "_pkg_size")
    need(cases[1]["image_sha256"] == cases[2]["image_sha256"], "rescue11_normal12_same_proven_image")
    need(cases[1]["image_sha256"] == "6035D50379B5ED5D8BBE26CF02297B82524E96AE43930A71EC5F8EC49C26CF9E",
         "known_good_normal9_image_digest")
    need(lock["execution_safety"]["physical_flash_authorized"] is False and
         lock["execution_safety"]["signing_authorized"] is False, "owner_permissions_not_granted")
    swd = SWD.read_text(encoding="utf-8")
    need("mode=HOTPLUG" in swd and "Hot Plug" in swd, "hotplug_required")
    need("Assert-AllowedCubeArguments -Arguments $Arguments" in swd, "exact_command_allowlist")
    need(swd.count("'--upload'") == 2, "bounded_64k_upload_only")
    need("'0x08000000','0x10000'" in swd, "correct_flash_readback_range")
    need("ProtectedRecovery" in swd and "ReparsePoint" in swd and
         "expectedProtectedRoot" in swd, "workspace_sibling_protected_destination")
    need("Assert-PrivateAcl -Path $root" in swd and
         "Assert-PrivateAcl -Path $runDir" in swd and "S-1-1-0" in swd,
         "fail_closed_acl_root_and_output")
    need("NO_RESET_NO_ERASE_NO_PROGRAM_NO_OPTION_BYTE_WRITE=YES" in swd,
         "no_target_mutation_declared")
    need("POST_SWD_USB_RUNTIME=NOT_ATTESTED" in swd, "post_swd_usb_not_fabricated")
    # Native V1 original Evidence: HOST_PRECHECK rejected PATH/Get-Command lookup.
    # V2 must select the SHA-pinned, already installed sibling Tools executable.
    exact_cli = "269DB404ED3F2CDD7CFCCCC257AB0A907F2364C3D44E39EDAB6B007CB5F668ED"
    def cli_source_contract(src: str) -> bool:
        return (("Tools/STM32Cube/STM32CubeProgrammer/bin" in src)
                and ("STM32_Programmer_CLI.exe" in src)
                and (exact_cli in src)
                and ("CUBE_CLI_PINNED_WORKSPACE_TOOLS_MISSING" in src)
                and ("CUBE_CLI_PINNED_SHA256_MISMATCH" in src)
                and ("CUBE_CLI_RESOLUTION=PINNED_WORKSPACE_TOOLS_SHA256" in src)
                and ("Get-Command -Name STM32_Programmer_CLI.exe" not in src)
                and ("DEUS_STM32_PROGRAMMER_CLI" not in src))
    need(cli_source_contract(swd), "canonical_workspace_cubeprogrammer_sha_pinned_no_path_fallback")
    need(not cli_source_contract(swd.replace(exact_cli, "0" * 64)),
         "negative_mutated_cubeprogrammer_sha_rejected")
    need("CUBE_CLI_BINARY_REPARSE_FORBIDDEN" in swd and
         "CUBE_CLI_DIRECTORY_REPARSE_FORBIDDEN" in swd,
         "cubeprogrammer_executable_path_no_reparse")
    for token in ["'-rst'", "'-e'", "'--erase'", "'-w'", "'-d'", "'-go'",
                  "'--readunprotect'", "'mode=NORMAL'", "'-rdu'"]:
        need(token not in swd, "forbid_arg_" + token)
    sign = SIGNER.read_text(encoding="utf-8")
    need("HMACSHA256" in sign and "DEUSHDR1" in sign and "0x08002000" in sign,
         "reuse_existing_public_signing_format")
    need(not (ROOT / "scripts/normal10_signed.pkg").exists(), "no_fake_signed_pkg")
    print("SOURCE_ONLY_NORMAL10_PREP=PASS_WINDOWS_AST_NATIVE_SWD_AND_SIGNING_NOT_ATTESTED")


if __name__ == "__main__":
    main()
