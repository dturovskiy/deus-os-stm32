#!/usr/bin/env python3
"""Read-only, non-attesting USB sysfs locator for DEUS MCP execution domain.
No libusb, no open(), reset, claim, detach, endpoints, UART or SWD.
"""
from pathlib import Path
import json

def get(p,default="?"):
    try:return p.read_text(encoding="ascii").strip()
    except (OSError,UnicodeError):return default

root=Path("/sys/bus/usb/devices")
devices=[]
for node in root.glob("*"):
    if not (node/"idVendor").exists():continue
    vid=get(node/"idVendor")
    pid=get(node/"idProduct")
    if vid.lower()!="1209":continue
    iface=[]
    for child in root.glob(node.name+":*"):
        iface.append(dict(number=get(child/"bInterfaceNumber"),cls=get(child/"bInterfaceClass"),
                         subcls=get(child/"bInterfaceSubClass"),protocol=get(child/"bInterfaceProtocol")))
    devices.append(dict(vid=vid,pid=pid,interfaces=iface))
print(json.dumps({"schema":"DEUS_OS_DEUS_MCP_USB_SYSFS_READONLY_V1",
                  "usb_sysfs_present":root.is_dir(),
                  "device_1209_count":len(devices),"devices":devices,
                  "opened_usb":False,"hardware_source_attested":False,
                  "firmware_modified":False,"gate4_authorized":False},sort_keys=True))
