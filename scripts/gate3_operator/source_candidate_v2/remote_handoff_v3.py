#!/usr/bin/env python3
"""One approved 000C -> 000D -> one read-only INFO physical session on Ubuntu.
Requires interactive ssh -tt; sudo credential preflight is BEFORE the reset.
No firmware programming, signing, UART/SWD, udev changes, or reset retries."""
import hashlib,importlib.util,json,os,pathlib,pwd,stat,subprocess,sys,threading,time,ctypes.util
BASE=pathlib.Path(__file__).resolve().parent
SYSFS=pathlib.Path("/sys/bus/usb/devices")
PORT="usb:001:8"
SCHEMA="DEUS_G3_000D_ONE_RESET_INFO_PHYSICAL_V1"
RESULT=BASE/"result.json"
EXACT_FILES={
    "macmini_gate3_sysfs_readonly_v2.py": "5541B85C14E00DDA8072A4551CCAFA2E3B392F277BC9DCC171C47715717B2C88",
    "remote_result_guard.py": "D2E047BEC44F580946248A0A6A8330F85E17AD1D78E894D69D5CDA7A9C4A54CD",
}
def require(ok,err):
    if not ok:raise RuntimeError(err)
def attr(p):
    try:return p.read_text(encoding="ascii").strip()
    except (OSError,UnicodeError):return None
def moment():
    return {"monotonic":round(time.monotonic(),6),"utc":time.strftime("%Y-%m-%dT%H:%M:%SZ",time.gmtime())}
R={"schema":SCHEMA,"result":"FAIL","classification":"STOP_BEFORE_RESET",
   "physical_port":PORT,"host":"UBUNTU_MACMINI","firmware_flash":"NO",
   "firmware_erase":"NO","stlink":"NO","uart":"NO","persistent_udev":"NO",
   "protocol_run_as_root":"NO","physical_identity_authenticated":"NO",
   "reset_attempts":0,"info_attempts":0,"gate3_full":"NOT_ACCEPTED",
   "gate4":"NOT_AUTHORIZED","events":[],"permission":{"status":"NOT_RUN"}}
def persist():
    tmp=BASE/"result.json.tmp"
    with tmp.open("w",encoding="utf-8") as handle:
        json.dump(R,handle,sort_keys=True,ensure_ascii=True,separators=(",",":"))
        handle.write("\n");handle.flush();os.fsync(handle.fileno())
    os.replace(tmp,RESULT)
def stamp(kind):
    R["events"].append({"event":kind,**moment()});persist()
def validated_stage():
    meta=json.loads((BASE/"deployment.json").read_text(encoding="utf-8"))
    require(meta.get("schema")=="DEUS_G3_STAGE_HASHES_V1","STAGE_SCHEMA_DRIFT")
    items=meta.get("files")
    require(isinstance(items,dict),"STAGE_MANIFEST_MALFORMED")
    # Exact manifest domain means no unpinned binary or executable substitution.
    required={"DeusOs.Gate3HardwareHandoff.dll","DeusOs.Gate3HardwareHandoff.deps.json",
              "DeusOs.Gate3HardwareHandoff.runtimeconfig.json",
              "DeusOs.Control.Core.dll","DeusOs.Control.Transport.Linux.dll",
              "macmini_gate3_sysfs_readonly_v2.py","node_fixture.py",
              "remote_handoff_v3.py","remote_result_guard.py"}
    require(set(items)==required,"STAGE_INVENTORY_DRIFT")
    for n,sha in items.items():
        require(n==pathlib.Path(n).name and
                isinstance(sha,str) and len(sha)==64 and
                all(c in "0123456789ABCDEF" for c in sha),
                "STAGE_UNSAFE_MEMBER")
        path=BASE/n
        require(path.is_file() and not path.is_symlink(),"STAGE_MEMBER_ABSENT_OR_LINK")
        require(hashlib.sha256(path.read_bytes()).hexdigest().upper()==sha,
                "STAGE_MEMBER_SHA_DRIFT")
    for n,sha in EXACT_FILES.items():
        require(items.get(n)==sha,"SOURCE_IDENTITY_MISMATCH")
    R["stage_sha256"]={n:items[n] for n in sorted(items)}
    persist()
def load_guard():
    spec=importlib.util.spec_from_file_location("gate3_result_guard",BASE/"remote_result_guard.py")
    module=importlib.util.module_from_spec(spec)
    sys.modules[spec.name]=module
    spec.loader.exec_module(module)
    return module

def load_probe():
    spec=importlib.util.spec_from_file_location("gate3sysfs",BASE/"macmini_gate3_sysfs_readonly_v2.py")
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module
def scan(mod):
    items=mod.collect(SYSFS)
    require(len(items)<=1,"AMBIGUOUS_TARGETS")
    if not items:return None
    item=items[0]
    require(item["port"]==PORT,"WRONG_PHYSICAL_PORT")
    devnode=SYSFS/"1-8"
    require(devnode.is_dir(),"EXPECTED_USB_SYSFS_NODE_ABSENT")
    dn=attr(devnode/"devnum")
    require(dn is not None and dn.isdecimal() and 1<=int(dn)<=127,"DEVNUM_INVALID")
    item["devnum"]=int(dn)
    return item
def helper(subcommand,args,timeout):
    cmd=["dotnet",str(BASE/"DeusOs.Gate3HardwareHandoff.dll"),subcommand,*args]
    try:
        p=subprocess.run(cmd,input="",capture_output=True,encoding="utf-8",
                         errors="replace",timeout=timeout,cwd=BASE)
        require(len(p.stdout)<5000 and len(p.stderr)<4000,"HELPER_OUTPUT_LIMIT")
        return {"exit":p.returncode,"stdout":p.stdout,"stderr_empty":not p.stderr.strip(),
                "timeout":False}
    except subprocess.TimeoutExpired:
        return {"exit":124,"stdout":"","stderr_empty":False,"timeout":True}
def user_preflight(mod):
    require(sys.platform.startswith("linux"),"HOST_NOT_LINUX")
    require(os.geteuid()!=0,"ROOT_PRODUCT_PROTOCOL_FORBIDDEN")
    require(pwd.getpwuid(os.geteuid()).pw_name=="deus","UNEXPECTED_UNPRIVILEGED_USER")
    require(sys.stdin.isatty() and sys.stdout.isatty(),"INTERACTIVE_SSH_TTY_REQUIRED")
    require(ctypes.util.find_library("usb-1.0") is not None,"LIBUSB_ABSENT")
    require(SYSFS.is_dir(),"SYSFS_ABSENT")
    dotnet=subprocess.run(["dotnet","--version"],capture_output=True,text=True,timeout=10)
    require(dotnet.returncode==0 and dotnet.stdout.strip().startswith("10."),"DOTNET10_ABSENT")
    check=helper("selftest",[],10)
    require(check["exit"]==0 and "GATE3_HELPER_SELFTEST=PASS" in check["stdout"],
            "COMPILED_HELPER_SELFTEST_FAIL")
    old=scan(mod)
    require(old is not None and old["pid"]=="000c","SINGLE_RUNTIME_000C_NOT_PRESENT")
    decision,_=mod.evaluate([old])
    require(decision=="PASS_RUNTIME_000C_OBSERVED","RUNTIME_DESCRIPTOR_INVALID")
    R["before"]=old
    persist()
    # Visibility and sudo capability MUST be proven BEFORE reset. This call
    # inherits ssh -tt stdin/stdout/stderr; password is NOT piped or logged.
    print("DEUS Gate3: sudo credential check BEFORE reset; password stays on this terminal.",
          flush=True)
    credential=subprocess.run(["sudo","-p","DEUS Gate3 sudo (no logging): ","-v"],
                              timeout=50,stdin=None,stdout=None,stderr=None)
    require(credential.returncode==0,"SUDO_INTERACTIVE_PREFLIGHT_FAILED")
    cached=subprocess.run(["sudo","-n","-v"],timeout=8,capture_output=True)
    require(cached.returncode==0,"SUDO_NONINTERACTIVE_CACHE_NOT_READY")
    R["permission"]["status"]="SUDO_INTERACTIVE_PREFLIGHT_PASS"
    stamp("INTERACTIVE_PRIVILEGE_PREFLIGHT_PASS_BEFORE_RESET")
    return old
def node_path(devnum):
    return pathlib.Path(f"/dev/bus/usb/001/{devnum:03d}")
def permission_for_bootloader(item):
    require(item["pid"]=="000d" and item["port"]==PORT,"BOOTLOADER_PORT_NOT_EXACT")
    devnum=item["devnum"];node=node_path(devnum)
    sb=node.lstat()
    require(stat.S_ISCHR(sb.st_mode) and os.major(sb.st_rdev)==189 and
            os.minor(sb.st_rdev)==devnum-1,"BOOTLOADER_DEVICE_NODE_NOT_USB")
    current=scan(load_probe())
    require(current is not None and current["pid"]=="000d" and
            current["devnum"]==devnum,"BOOTLOADER_DEVICE_NODE_CHANGED")
    if sb.st_uid==os.getuid() and stat.S_IMODE(sb.st_mode)==0o600:
        R["permission"]={"status":"ALREADY_OWNER_0600","node":f"001/{devnum:03d}"}
        persist();return
    stamp("ONE_TIME_EPHEMERAL_NODE_FIXTURE_BEGIN")
    # -n cannot prompt or capture password after reset; the credential check
    # was attached to the visible operator TTY BEFORE any device reset.
    cmd=["sudo","-n","--",sys.executable,str(BASE/"node_fixture.py"),
         str(os.getuid()),str(devnum)]
    result=subprocess.run(cmd,capture_output=True,text=True,timeout=12)
    require(result.returncode==0,"EPHEMERAL_NODE_FIXTURE_FAILED")
    fixture=json.loads(result.stdout.strip())
    require(fixture.get("status")=="PASS" and fixture.get("devnum")==devnum and
            fixture.get("uid")==os.getuid() and fixture.get("mode")=="0600",
            "EPHEMERAL_NODE_FIXTURE_RECEIPT_INVALID")
    updated=node.lstat()
    require(updated.st_uid==os.getuid() and stat.S_IMODE(updated.st_mode)==0o600,
            "EPHEMERAL_NODE_PERMISSIONS_WRONG")
    R["permission"]={"status":"EPHEMERAL_NODE_OWNER_0600_PASS","node":fixture["node"]}
    stamp("ONE_TIME_EPHEMERAL_NODE_FIXTURE_PASS")
def execute():
    import sys
    require(len(sys.argv)==2 and
            sys.argv[1]=="--owner-approved-reset-once-and-node-000d-only",
            "OWNER_SCOPE_TOKEN_MISSING")
    validated_stage()
    module=load_probe()
    old=user_preflight(module)
    result_guard=load_guard()
    old_session=f"usb001_8_dev{old['devnum']}"
    watcher_events=[];watcher_errors=[]
    watchstop=threading.Event()
    def watch():
        previous="000c:"+str(old["devnum"])
        end=time.monotonic()+23
        while not watchstop.is_set() and time.monotonic()<end:
            try:
                item=scan(module)
                state="none" if item is None else item["pid"]+":"+str(item["devnum"])
                if state!=previous:
                    watcher_events.append({"event":state,"ordinal":len(watcher_events)+1,
                                           **moment()})
                    previous=state
            except Exception as exc:
                watcher_errors.append(type(exc).__name__);return
            time.sleep(.02)
    worker=threading.Thread(target=watch,daemon=True)
    worker.start()
    stamp("RUNTIME_LIVE_PRESTATE_AND_SUDO_PREFLIGHT_CONFIRMED")
    R["reset_attempts"]=1  # Conservative boundary BEFORE sending opcode 01.
    stamp("ONE_ENTER_BOOTLOADER_ATTEMPT_STARTED")
    # Helper emits exactly one ENTER_BOOTLOADER request; NO second call even if
    # native response is lost after STM32 acted on the command.
    entry=helper("enter",[PORT],16)
    R["enter"]={k:v for k,v in entry.items() if k!="stdout"}
    R["enter"]["tokens"]=entry["stdout"].splitlines()
    persist()
    result_guard.stage_receipt(R["enter"],"ENTER")
    deadline=time.monotonic()+8
    boot=None
    while time.monotonic()<deadline:
        item=scan(module)
        if item is not None and item["pid"]=="000d" and item["devnum"]!=old["devnum"]:
            desc,_=module.evaluate([item])
            if desc=="PASS_BOOTLOADER_000D_SYSFS_ONLY":
                boot=item;break
        time.sleep(.055)
    watchstop.set();worker.join(timeout=1)
    R["usb_events"]=watcher_events
    persist()
    require(not watcher_errors,"USB_WATCHER_FAILED")
    require(boot is not None,"NEW_BOOTLOADER_000D_NOT_FOUND")
    gone=[e for e in watcher_events if e["event"]=="none"]
    attached=[e for e in watcher_events
              if e["event"]=="000d:"+str(boot["devnum"])]
    require(len(gone)==1 and len(attached)==1 and
            gone[0]["ordinal"]<attached[0]["ordinal"],
            "ORDERED_USB_DETACH_ATTACH_NOT_ESTABLISHED")
    R["after"]=boot
    R["observed_detach_ordinal"]=gone[0]["ordinal"]
    R["observed_attach_ordinal"]=attached[0]["ordinal"]
    stamp("NEW_BOOTLOADER_000D_SAME_PORT_DIFFERENT_DEVNUM")
    permission_for_bootloader(boot)
    require(scan(module)["devnum"]==boot["devnum"],"BOOTLOADER_DEVICE_SESSION_STALE")
    new_session=f"usb001_8_dev{boot['devnum']}"
    R["info_attempts"]=1
    stamp("ONE_BOOTLOADER_INFO_ATTEMPT_STARTED")
    info=helper("info",[PORT,old_session,new_session,
                        str(gone[0]["ordinal"]),str(attached[0]["ordinal"])],12)
    R["info"]={k:v for k,v in info.items() if k!="stdout"}
    R["info"]["tokens"]=info["stdout"].splitlines()
    persist()
    result_guard.stage_receipt(R["info"],"INFO")
    R["result"]="PASS"
    R["classification"]="SCOPED_PHYSICAL_ONE_ENTER_ONE_INFO_STOP_NO_FLASH"
    try:
        result_guard.success_is_exact(R)
    except Exception:
        R["result"]="FAIL"
        raise
    stamp("READONLY_INFO_GATE_STOP")
    return 0
def main():
    persist()
    try:return execute()
    except Exception as err:
        R["result"]="FAIL"
        code=str(err)
        R["error"]=code if code.isupper() and len(code)<=96 else "SANITIZED_REMOTE_FAILURE"
        R["classification"]=("STOP_AFTER_ONE_ENTER_NO_RETRY" if R["reset_attempts"]
                             else "STOP_BEFORE_RESET")
        persist()
        return 9
    finally:
        # Never attempt a second reset/return-to-application or device write.
        if "watchstop" in globals():watchstop.set()
        persist()
        print("DEUS_G3_REMOTE_FINAL="+R["classification"],flush=True)
if __name__=="__main__":
    raise SystemExit("SOURCE_ONLY_NOT_FOR_HARDWARE_EXECUTION")
    # Admission must be separate; do not remove this STOP as a workaround.
    # sys.exit(main())
