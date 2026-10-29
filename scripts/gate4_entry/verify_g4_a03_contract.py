#!/usr/bin/env python3
"""Offline-only A-03 source/transport/result contract verifier and negatives.

This instrument reads local source files plus one static JSON contract. It has
no USB, SSH, subprocess, driver, privileged operation, signer or device path.
A PASS never admits physical execution or closes a Gate4 hardware predicate.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
import re
from pathlib import Path

ROOT=Path.cwd().resolve()  # DEUS managed runtime executes source from /proc/self/fd
CONTRACT=ROOT/"scripts/gate4_entry/g4_a03_source_contract_v1.json"
TREE="081aac70c31909bf681254393e2288917cb9b8b2"
V6="2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED"
V7="C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A"
SOURCES={
"bootloader/bootloader.c":"A7A52D4962F48D7C95506FB97613BC6A5F7DA661BC07CF3CE850D86247EE25E3",
"host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs":"CFBFF53BDE5D0C49303F0733448AEF61AC63172CB2478A558E92062A79E8256B",
"host/src/DeusOs.Control.Core/DeusDeviceClient.cs":"CEDB67F7314977BB8F8D6FB6CDEC45A15E88AFA3923C7807DA238FCCD0906CA8",
"host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs":"6453EDCE4EDEC8B827E87B771D9E8081004D05051F9A24662940E0217DCEED82",
"host/src/DeusOs.Control.Transport.Windows/WindowsBootloaderWinUsbDiscovery.cs":"A4E6481CD5D7FB16E05ECFC29A2530911D641005E4C78742584A393BF59B59CD",
"docs/FIRMWARE_UPDATE_BOOTLOADER_PROTOCOL_V1.md":"1777AF7572F3A27091C0F53726329166B5908FF9E64D02F6A9EF68EF64CC1A73",
"scripts/gate3_operator/source_candidate_v2/Program.cs":"373AA8B98BBA01170C3CCADB7C127D1ED96C220CAB81F04FC1C38C61170A5B38",
"scripts/gate3_operator/source_candidate_v2/remote_result_guard.py":"D2E047BEC44F580946248A0A6A8330F85E17AD1D78E894D69D5CDA7A9C4A54CD",
"scripts/gate3_operator/source_candidate_v2/QUARANTINED_NOT_FOR_RUN.txt":"A8CE27D51A9037178B4E25CC989CFC42D10687BF79D355838DC7A69D7D692BC2",
}
PHASES=[
("P00_OFFLINE_SOURCE_AND_PACKAGE","WINDOWS_OWNER","OFFLINE",None,False),
("P01_REAL_OWNER_AND_POWER_PRESTATE","UBUNTU_USB_AND_WINDOWS_OWNER","READ_ONLY","READONLY_GO_REQUIRED",False),
("P02_ENTER_BOOTLOADER_ONCE","UBUNTU_MACMINI","HARDWARE_MUTATING","FRESH_ENTER_GO",True),
("P03_SAME_PORT_000D_ENUMERATION","UBUNTU_MACMINI","READ_ONLY","ENTER_GO_CONTEXT",False),
("P04_SINGLE_BOOTLOADER_INFO","UBUNTU_MACMINI","READ_ONLY","ENTER_GO_CONTEXT",False),
("P05_SIGNED_V6_UPDATE","UBUNTU_MACMINI","HARDWARE_MUTATING","SEPARATE_FRESH_FLASH_GO",True),
("P06_EXPECTED_NEW_RUNTIME_OBSERVATION","UBUNTU_MACMINI","READ_ONLY","FLASH_GO_CONTEXT",False),
("P07_ORIGINAL_EVIDENCE_AND_SINGLE_RESULT","WINDOWS_OWNER","OFFLINE",None,False),
]
REQUIRED_STOP={
"before_any_physical_child":"STOP_BEFORE_PHYSICAL_ACTION",
"after_child_start_ambiguous":"AMBIGUOUS_POSSIBLY_ATTEMPTED",
"after_enter_before_flash":"STOP_AFTER_ENTER_NO_RETRY",
"during_or_after_flash_uncertain":"FLASH_POSSIBLY_PARTIAL_NO_RETRY",
"original_evidence_unsealed":"EVIDENCE_SEAL_FAILURE",
"release_or_consent_invalid":"RELEASE_STOP",
}
REQUIRED_EXCLUDED={
"PRODUCTION_KEY_EXTRACTION","SIGNING_AGAIN","GATE3_RUNNER_REUSE",
"AUTOMATIC_USB_REPLUG","AUTOMATIC_RESCUE_FLASH","GENERIC_RPC",
"STLINK_WRITE_OR_ERASE","ACTUAL_DEVICE_COMMANDS","SHELL_OR_SSH_EXECUTION",
}
class Stop(ValueError): pass
def need(cond,why):
    if not cond: raise Stop(why)
def strict_json_pairs(pairs):
    d={}
    for k,v in pairs:
        need(k not in d,"DUPLICATE_JSON_KEY:"+k)
        d[k]=v
    return d
def file_bytes(relative,sha=None):
    p=ROOT/relative
    need(p.is_file() and not p.is_symlink(),"MISSING_OR_SYMLINK:"+relative)
    b=p.read_bytes()
    need(len(b)>0 and len(b)<=250000,"SIZE_BAD:"+relative)
    if sha: need(hashlib.sha256(b).hexdigest().upper()==sha,"SOURCE_SHA_DRIFT:"+relative)
    return b
def marker(code,text,items,checks):
    need(all(item in text for item in items),"SOURCE_MARKER_DRIFT:"+code)
    checks.append(code)
def verify(c,check_sources=True):
    passes=[]
    def check(label,condition):
        need(condition is True,"CONTRACT_REJECT:"+label)
        passes.append(label)
    check("EXACT_ROOT_KEYS",set(c)=={"schema","status","gate4_authorized",
        "physical_execution_admitted","source_root","source_pins","candidate",
        "ownership","phase_order","go_boundaries","result_contract",
        "stop_classification","deferred_risks","out_of_scope"})
    check("STRICT_OFFLINE_IDENTITY",c["schema"]==
          "DEUS_OS_GATE4_A03_SOURCE_TRANSPORT_RESULT_CONTRACT_V1"
          and c["status"]=="SOURCE_ONLY_NON_EXECUTABLE_NOT_A_RELEASE"
          and c["source_root"]=="OS"
          and c["gate4_authorized"] is False
          and c["physical_execution_admitted"] is False)
    check("PINNED_NINE_SOURCE_FILES",c["source_pins"]==SOURCES)
    x=c["candidate"]
    check("CANDIDATE_SCHEMA",set(x)=={
        "selected_git_tree","old_runtime","old_runtime_version",
        "normal_package_name","normal_package_bytes","normal_package_sha256",
        "normal_signed_version","rescue_package_name","rescue_package_bytes",
        "rescue_package_sha256","rescue_signed_version",
        "post_rescue_min_normal_version"})
    check("ACTUAL_SELECTED_TREE",x["selected_git_tree"]==TREE and
          x["old_runtime"]=="1209:000C" and type(x["old_runtime_version"]) is int and
          x["old_runtime_version"]==5)
    check("SIGNED_V6_PIN",x["normal_package_name"]==
          "deus_os_000e_management_only_tree081aac70_signed_v6.pkg" and
          type(x["normal_package_bytes"]) is int and x["normal_package_bytes"]==49072 and
          x["normal_package_sha256"]==V6 and
          type(x["normal_signed_version"]) is int and x["normal_signed_version"]==6)
    check("SIGNED_V7_PIN",x["rescue_package_name"]==
          "deus_os_rev5_000c_recovery_future_floor7_v1.pkg" and
          type(x["rescue_package_bytes"]) is int and x["rescue_package_bytes"]==52092 and
          x["rescue_package_sha256"]==V7 and
          type(x["rescue_signed_version"]) is int and x["rescue_signed_version"]==7 and
          type(x["post_rescue_min_normal_version"]) is int and
          x["post_rescue_min_normal_version"]==8)
    h=c["ownership"]
    check("EXACT_OWNER_KEYS",set(h)=={"result_owner","native_usb_owner",
        "historical_usb_locator_not_authoritative","windows_other_transports",
        "runtime_initial","bootloader","runtime_after_install",
        "original_evidence_root","bundle_root"})
    check("USB_AND_RESULT_OWNERSHIP",h["result_owner"]=="WINDOWS_DEUS_RUN_V1_3_1" and
          h["native_usb_owner"]=="UBUNTU_MACMINI" and
          h["historical_usb_locator_not_authoritative"]=="usb:001:8" and
          h["windows_other_transports"]==["CH340_UART","STLINK_SWD"])
    check("EXACT_DESCRIPTOR_EXPECTATIONS",
          h["runtime_initial"]=={"pid":"1209:000C","management_interface":2} and
          h["bootloader"]=={"pid":"1209:000D","interface":0,"out_ep":"0x01","in_ep":"0x81"} and
          h["runtime_after_install"]=={"pid":"1209:000E","management_interface":0,"out_ep":"0x04","in_ep":"0x84"})
    check("RESULT_PATH_OFF_GIT",h["original_evidence_root"]=="OS_PARENT/Evidence"
          and h["bundle_root"]=="OS_PARENT/Bundle")
    actual=c["phase_order"]
    check("EIGHT_ORDERED_PHASES",type(actual) is list and len(actual)==len(PHASES)
          and all(type(item) is dict and set(item)==
                  {"phase","domain","action_class","go","side_effect"}
                  for item in actual))
    for i,(phase,domain,cls,go,side) in enumerate(PHASES):
        check("PHASE_"+str(i)+"_EXACT",
              actual[i]=={"phase":phase,"domain":domain,"action_class":cls,
                          "go":go,"side_effect":side})
    g=c["go_boundaries"]
    check("NO_GO_OR_RELEASE",set(g)=={"fresh_owner_go","enter_go","flash_go",
        "release_admitted","release_controller_implemented",
        "past_gate3_consent_reusable","data_timeout_retry_in_host",
        "whole_update_retry","enter_retry","authorize_header_retry",
        "firmware_session_reset_retry"} and
        all(g[k] is False for k in ("fresh_owner_go","enter_go","flash_go",
             "release_admitted","release_controller_implemented",
             "past_gate3_consent_reusable")))
    check("RETRY_BOUNDARIES_NO_SESSION_REPLAY",
          type(g["data_timeout_retry_in_host"]) is int and
          g["data_timeout_retry_in_host"]==1 and
          all(type(g[k]) is int and g[k]==0 for k in
              ("whole_update_retry","enter_retry","authorize_header_retry",
               "firmware_session_reset_retry")))
    result=c["result_contract"]
    check("RESULT_SCHEMA_AND_SINGLE_WRITER",set(result)=={
        "schema","single_owner","write_once","original_zip_indexed",
        "zip_on_fail","zip_on_pass","independent_reopen_required",
        "success_without_original_evidence_allowed","pass_claim_scope",
        "source_only_success_may_promote_gate4","git_pre_post_required",
        "device_identity_observation_required"} and
        result["schema"]=="DEUS_OS_RUNNER_RESULT_V1" and
        result["single_owner"]=="WINDOWS" and result["write_once"] is True and
        all(result[k] is True for k in ("original_zip_indexed","zip_on_fail",
           "zip_on_pass","independent_reopen_required","git_pre_post_required",
           "device_identity_observation_required")) and
        result["success_without_original_evidence_allowed"] is False and
        result["source_only_success_may_promote_gate4"] is False and
        result["pass_claim_scope"]=="G4_PHYSICAL_SCOPED_ONLY_AFTER_REAL_NATIVE_EVIDENCE")
    check("STOP_CLASSIFICATION_COMPLETE",c["stop_classification"]==REQUIRED_STOP)
    check("DEFERRED_RISK_NOT_SILENT_PASS",c["deferred_risks"]==
          {"rescue_second_copy":"NOT_ATTESTED_OWNER_DEFERRED",
           "independent_restore":"NOT_VERIFIED_OWNER_DEFERRED"})
    check("NO_SCOPE_EXPANSION",type(c["out_of_scope"]) is list and
          set(c["out_of_scope"])==REQUIRED_EXCLUDED and
          len(c["out_of_scope"])==len(REQUIRED_EXCLUDED))
    if check_sources:
        for rel,sha256 in SOURCES.items():
            file_bytes(rel,sha256)
            passes.append("FILE_SHA_"+Path(rel).name)
        fw=file_bytes("host/src/DeusOs.Control.Core/FirmwareUpdateClient.cs").decode("utf-8")
        client=file_bytes("host/src/DeusOs.Control.Core/DeusDeviceClient.cs").decode("utf-8")
        boot=file_bytes("bootloader/bootloader.c").decode("utf-8")
        linux=file_bytes("host/src/DeusOs.Control.Transport.Linux/LinuxBootloaderLibUsbDiscovery.cs").decode("utf-8")
        program=file_bytes("scripts/gate3_operator/source_candidate_v2/Program.cs").decode("utf-8")
        marker("FW_NATIVE_EXPLICIT_PHASES",fw,[
            "public async Task<FirmwareUpdateResponse> InfoAsync(",
            "public async Task<FirmwareUpdateResponse> UpdateAsync(",
            "OpcodeBegin,","OpcodeAuthorizeHeader,","OpcodeData,","OpcodeEnd,",
            "EnsureOk(end, FirmwareUpdateState.Committed);"],passes)
        marker("FW_HAS_ONE_TIMEOUT_DATA_RETRY_ONLY",fw,[
            "private async Task<FirmwareUpdateResponse> SendDataWithSingleTimeoutRetryAsync(",
            "when (exception.Kind == HostErrorKind.Timeout)",
            "OpcodeData,\n                DestructiveFlag,",
            "return await SendRequestCoreAsync("],passes)
        # Explicitly pin the only retry method block and count its DATA calls.
        retry=fw.split("private async Task<FirmwareUpdateResponse> SendDataWithSingleTimeoutRetryAsync(")[1].split("private static FirmwareUpdateResponse ParseResponse(")[0]
        check("DATA_RETRY_EXACT_TWO_SEND_BRANCHES",retry.count("OpcodeData,")==2 and
              retry.count("catch (DeusHostException exception)")==1 and
              "when (exception.Kind == HostErrorKind.Timeout)" in retry)
        marker("FW_VERIFY_ECHO_STATUS_STATE_OFFSET",fw,[
            "response.Status != FirmwareUpdateStatus.Ok",
            "response.State != expectedState",
            "data.ExpectedOffset != expectedOffset",
            "payload[0] != expectedOpcode"],passes)
        marker("RUNTIME_PUBLISHED_ENTER",client,[
            "public async Task<FirmwareUpdateResponse> EnterBootloaderAsync(",
            "FirmwareUpdateAccessPolicy.PublishedOnly",
            "SystemCapability.FirmwareUpdate"],passes)
        marker("000D_LINUX_USB_ENDPOINTS",linux,[
            "public const ushort ProductId = 0x000D;",
            "public const int InterfaceNumber = 0;",
            "public const byte OutEndpoint = 0x01;",
            "public const byte InEndpoint = 0x81;"],passes)
        marker("BOOT_AUTH_FLOOR_AND_NO_VALID_APP",boot,[
            "if(!explicit_update && committed_version!=0u)handoff_app();",
            "hmac_header(staged_header,calc);",
            "if(cand<=version_floor)return ST_VERSION_REJECTED;",
            "if(cand<version_floor)return ST_VERSION_REJECTED;"],passes)
        marker("SOURCE_CANDIDATE_HARDWARE_DENIED",program,[
            "private static readonly bool HardwareExecutionAdmitted = false;",
            "return Fail(\"SOURCE_CANDIDATE_HARDWARE_NOT_RELEASED\");"],passes)
    return passes

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--selftest",action="store_true")
    args=ap.parse_args()
    raw=CONTRACT.read_bytes()
    need(0<len(raw)<=20000,"CONTRACT_SIZE")
    need(not raw.startswith(b"\xef\xbb\xbf"),"CONTRACT_UTF8_BOM")
    c=json.loads(raw.decode("utf-8"),object_pairs_hook=strict_json_pairs)
    pass_checks=verify(c)
    if args.selftest:
        tests=0
        changes=[
          ("PRETEND_GATE4_GO",lambda x:x.update(gate4_authorized=True)),
          ("PRETEND_RELEASE",lambda x:x.update(physical_execution_admitted=True)),
          ("TRUST_OLD_GATE3_CONSENT",lambda x:x["go_boundaries"].update(past_gate3_consent_reusable=True)),
          ("FAKE_OWNER_GO",lambda x:x["go_boundaries"].update(fresh_owner_go=True)),
          ("FAKE_ENTER_GO",lambda x:x["go_boundaries"].update(enter_go=True)),
          ("FAKE_FLASH_GO",lambda x:x["go_boundaries"].update(flash_go=True)),
          ("FAKE_RELEASE",lambda x:x["go_boundaries"].update(release_admitted=True)),
          ("WRONG_CANDIDATE_TREE",lambda x:x["candidate"].update(selected_git_tree="0"*40)),
          ("WRONG_V6_SHA",lambda x:x["candidate"].update(normal_package_sha256="0"*64)),
          ("WRONG_V7_SHA",lambda x:x["candidate"].update(rescue_package_sha256="0"*64)),
          ("WRONG_NORMAL_VERSION",lambda x:x["candidate"].update(normal_signed_version=7)),
          ("WRONG_RESCUE_VERSION",lambda x:x["candidate"].update(rescue_signed_version=8)),
          ("WRONG_SOURCE_FILE_HASH",lambda x:x["source_pins"].update({"bootloader/bootloader.c":"0"*64})),
          ("WRONG_USB_OWNER",lambda x:x["ownership"].update(native_usb_owner="WINDOWS")),
          ("WRONG_USB_PORT_ASSUMPTION",lambda x:x["ownership"].update(historical_usb_locator_not_authoritative="CURRENT_VERIFIED")),
          ("WRONG_BOOTLOADER_PID",lambda x:x["ownership"]["bootloader"].update(pid="1209:000E")),
          ("WRONG_BOOTLOADER_ENDPOINT",lambda x:x["ownership"]["bootloader"].update(out_ep="0x04")),
          ("SWAP_ENTER_INFO",lambda x:x["phase_order"].__setitem__(2,x["phase_order"][4])),
          ("REMOVE_FLASH_GO",lambda x:x["phase_order"][5].update(go=None)),
          ("CLASSIFY_INFO_AS_MUTATING",lambda x:x["phase_order"][4].update(side_effect=True)),
          ("REPLAY_ENTER",lambda x:x["go_boundaries"].update(enter_retry=1)),
          ("REPLAY_UPDATE",lambda x:x["go_boundaries"].update(whole_update_retry=1)),
          ("DATA_RETRY_UNBOUNDED",lambda x:x["go_boundaries"].update(data_timeout_retry_in_host=9)),
          ("TWO_RESULT_WRITERS",lambda x:x["result_contract"].update(single_owner="WINDOWS_AND_LINUX")),
          ("REUSE_FAKE_EVIDENCE",lambda x:x["result_contract"].update(independent_reopen_required=False)),
          ("ACCEPT_SOURCE_ONLY_GATE4",lambda x:x["result_contract"].update(source_only_success_may_promote_gate4=True)),
          ("OMIT_EVIDENCE_FAIL",lambda x:x["result_contract"].update(zip_on_fail=False)),
          ("EMPTY_STOP",lambda x:x["stop_classification"].update(after_child_start_ambiguous="")),
          ("MISCLASSIFY_FLASH_AMBIGUITY",lambda x:x["stop_classification"].update(during_or_after_flash_uncertain="SAFE_RETRY")),
          ("FAKE_SECOND_COPY",lambda x:x["deferred_risks"].update(rescue_second_copy="PASS")),
          ("ALLOW_RESCUE_AUTOFLASH",lambda x:x["out_of_scope"].remove("AUTOMATIC_RESCUE_FLASH")),
          ("SCHEMA_DOWNGRADE",lambda x:x.update(schema="DEUS_OS_GATE4_A03_SOURCE_TRANSPORT_RESULT_CONTRACT_V0")),
          ("EXTRA_PERMISSION",lambda x:x.update(permission_granted=True)),
        ]
        for name,edit in changes:
            b=copy.deepcopy(c)
            edit(b)
            try: verify(b,False)
            except Stop: pass
            else: raise Stop("NEGATIVE_ACCEPTED:"+name)
            tests+=1
            print("G4_A03_NEGATIVE=PASS CASE="+name)
        invalids=[
            ("DUPLICATE_JSON_KEY",'{"a":1,"a":2}'),
            ("JSON_NON_OBJECT",'[]'),
        ]
        for name,value in invalids:
            try:
                o=json.loads(value,object_pairs_hook=strict_json_pairs)
                verify(o,False)
            except (Stop,TypeError,AttributeError): pass
            else: raise Stop("NEGATIVE_ACCEPTED:"+name)
            tests+=1
            print("G4_A03_NEGATIVE=PASS CASE="+name)
        print(f"G4_A03_CONTRACT_SELFTEST=PASS NEGATIVE={tests} SOURCE_CHECKS={len(pass_checks)} USB=NO SSH=NO RESET=NO FLASH=NO")
    else:
        print(f"G4_A03_SOURCE_CONTRACT=PASS CHECKS={len(pass_checks)} SOURCE_ONLY=YES HARDWARE_RELEASE=NO GATE4=NOT_AUTHORIZED")
    return 0

if __name__=="__main__":
    try: raise SystemExit(main())
    except (Stop,ValueError,KeyError,TypeError,UnicodeError,OSError) as exc:
        print("G4_A03_SOURCE_CONTRACT=FAIL "+type(exc).__name__+" "+str(exc))
        raise SystemExit(2)
