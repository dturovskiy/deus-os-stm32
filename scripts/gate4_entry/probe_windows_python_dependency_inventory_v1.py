#!/usr/bin/env python3
"""Windows CPython dependency INVENTORY only: no pip, install, network or keys.

Executed by canonical read-only OFFLINE Windows deus-run operator with a hard
subprocess timeout; this source is separately SHA-pinned in bundle + live Git.
"""
import importlib
import importlib.metadata as metadata
import json
import os
import platform
import struct
import sys
import sysconfig

PACKAGES=("cryptography","cffi","pycparser","pip")
def version_of(name):
    try:
        return metadata.version(name)
    except metadata.PackageNotFoundError:
        return None
def module_probe(name):
    try:
        importlib.import_module(name)
        return {"status":"IMPORT_PASS"}
    except BaseException as exc:
        # No user paths or PII. The full failure classification and final
        # message are preserved as separate bounded structured data.
        msg=str(exc).replace("\r"," ").replace("\n"," ")
        if len(msg)>350:
            msg=msg[:80]+" [MIDDLE_TRUNCATED_TAIL_PRESERVED] "+msg[-240:]
        return {"status":"IMPORT_FAIL","exception":type(exc).__name__,"message":msg}
def main():
    if os.name!="nt":
        raise RuntimeError("WINDOWS_ONLY_INVENTORY")
    ver=sys.version_info
    impl=platform.python_implementation()
    bits=struct.calcsize("P")*8
    gilvar=sysconfig.get_config_var("Py_GIL_DISABLED")
    gil_disabled=bool(gilvar)
    machine=platform.machine().upper()
    tag_prefix="cp"+str(ver.major)+str(ver.minor)
    # pip vendored packaging is only advisory, never a prerequisite or used to
    # install anything. If pip isn't importable, record that explicitly.
    try:
        from pip._vendor.packaging.tags import sys_tags
        tags=[str(x) for _,x in zip(range(30),sys_tags())]
    except BaseException as exc:
        tags=[]
        tag_failure=type(exc).__name__+": "+str(exc)[:180]
    else:
        tag_failure=None
    versions={k:version_of(k) for k in PACKAGES}
    imports={k:module_probe(k) for k in ("cryptography","cffi","pycparser")}
    tag_candidates=(
        ["cryptography-50.0.2-cp314-cp314t-win_amd64.whl",
         "cffi-2.1.1-cp314-cp314t-win_amd64.whl"]
        if gil_disabled else
        ["cryptography-50.0.2-cp311-abi3-win_amd64.whl",
         "cffi-2.1.1-cp314-cp314-win_amd64.whl"]
    )
    proposed=tag_candidates+["pycparser-3.12-py3-none-any.whl"]
    platform_accepted=(
       impl=="CPython" and ver.major==3 and ver.minor==14 and
       bits==64 and machine in ("AMD64","X86_64") and
       sys.platform=="win32"
    )
    result={
      "schema":"DEUS_A05_PYTHON314_WINDOWS_DEP_INVENTORY_V1",
      "source_scope":"READ_ONLY_HOST_INVENTORY",
      "platform":sys.platform,
      "implementation":impl,
      "version":f"{ver.major}.{ver.minor}.{ver.micro}",
      "architecture_bits":bits,
      "machine":machine,
      "sysconfig_platform":sysconfig.get_platform(),
      "abi_flags":str(getattr(sys,"abiflags","")),
      "gil_disabled":gil_disabled,
      "py_gil_disabled_config":str(gilvar),
      "python_executable_path":"REDACTED",
      "pip_tags_first_30":tags,
      "pip_tags_error":tag_failure,
      "package_versions":versions,
      "module_imports":imports,
      "missing_packages":[x for x in ("cryptography","cffi","pycparser")
                          if versions[x] is None],
      "recommended_wheel_filenames_METADATA_ONLY":proposed,
      "python314_win_amd64_candidate_match":platform_accepted,
      "wheel_files_verified_on_owner":"NO",
      "dependency_install_performed":"NO",
      "network_access":"NO",
      "usb":"NO","ssh":"NO","flash":"NO","reset":"NO"
    }
    print("G4_A05_PYTHON_INVENTORY_JSON="+json.dumps(result,sort_keys=True,separators=(",",":")),flush=True)
    if not platform_accepted:
        print("G4_A05_PYTHON_INVENTORY=NONMATCHING_INTERPRETER",flush=True)
    else:
        print("G4_A05_PYTHON_INVENTORY=PASS",flush=True)
if __name__=="__main__":
    main()
