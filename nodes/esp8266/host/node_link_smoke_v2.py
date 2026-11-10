#!/usr/bin/env python3
"""Deus ESP-01 REV2 host Node Link test with explicit safe CH340 open policy.

No automatic hardware operations on import or test. Actual UART open requires
--execute from an authorized operator. No reset, esptool, boot or Flash ops.
"""
from __future__ import annotations
import argparse
import binascii
import secrets
import struct
import sys
import time
from serial_policy import open_esp_uart, ESP_CH340_BY_ID, SerialPolicyError

MAGIC=b"\xD5\x4E"
CAPS=bytes((1,3,16,2))
MAX_FRAME=40

class NodeLinkError(RuntimeError):
    pass

def crc16(data: bytes)->int:
    crc=0xffff
    for b in data:
        crc ^= b<<8
        for _ in range(8):
            crc=(((crc<<1)^0x1021) if crc&0x8000 else (crc<<1))&0xffff
    return crc

def encode(kind:int,seq:int,session:int,payload:bytes=b"")->bytes:
    if not (1<=seq<=65535 and 0<=session<=0xffffffff and len(payload)<=24):
        raise NodeLinkError("INVALID_REQUEST")
    hdr=struct.pack("<BBBBHBBI",1,kind,1,0,seq,len(payload),0,session)
    frame=MAGIC+hdr+payload
    return frame+struct.pack("<H",crc16(frame[2:]))

def decode(raw:bytes):
    if not (16<=len(raw)<=MAX_FRAME and raw[:2]==MAGIC):
        return None
    version,kind,node,reserved,seq,n,reserved2,sid=struct.unpack("<BBBBHBBI",raw[2:14])
    if (version,node,reserved,reserved2)!=(1,1,0,0) or not seq or n>24 or len(raw)!=16+n:
        return None
    if int.from_bytes(raw[-2:],"little")!=crc16(raw[2:-2]):
        return None
    return kind,seq,sid,raw[14:-2]

def receive(port,expected_kind:int,seq:int,session:int,timeout_s:float):
    deadline=time.monotonic()+timeout_s
    buf=bytearray()
    while time.monotonic()<deadline:
        buf.extend(port.read(64))
        if len(buf)>1024:
            del buf[:-64]
        while buf:
            i=buf.find(MAGIC)
            if i<0:
                last=buf[-1] if buf else -1
                buf.clear()
                if last==MAGIC[0]:buf.append(last)
                break
            if i:del buf[:i]
            if len(buf)<14:break
            n=buf[8]
            if n>24:
                del buf[0]
                continue
            size=16+n
            if len(buf)<size:break
            raw=bytes(buf[:size]);del buf[:size]
            parsed=decode(raw)
            if parsed is None:continue
            kind,sq,sid,payload=parsed
            if (kind,sq,sid)==(expected_kind,seq,session):
                return payload
    raise NodeLinkError("NO_VALID_MATCHED_REPLY")

def transact(port,request:bytes,expected_kind:int,seq:int,sid:int,
             timeout_s:float=1.0,retries:int=0):
    for attempt in range(1+retries):
        wrote=port.write(request)
        port.flush()
        if wrote!=len(request):
            raise NodeLinkError("SHORT_TX")
        try:
            return receive(port,expected_kind,seq,sid,timeout_s)
        except NodeLinkError as e:
            if str(e)!="NO_VALID_MATCHED_REPLY" or attempt>=retries:raise
    raise NodeLinkError("UNEXPECTED_CONTROL_FLOW")

def smoke(port,challenge:int):
    """Full five-case check, safe to unit-test with an in-memory transport."""
    if not (1<=challenge<=0xffffffff):raise NodeLinkError("INVALID_CHALLENGE")
    hello=encode(1,1,0,struct.pack("<I",challenge))
    ack=transact(port,hello,0x81,1,challenge,retries=1)
    if ack!=struct.pack("<I",challenge)+CAPS:raise NodeLinkError("BAD_HELLO_ACK")
    ping=encode(2,2,challenge)
    if transact(port,ping,0x82,2,challenge,retries=1):raise NodeLinkError("BAD_PONG")
    data=b"DEUS"
    request=encode(3,3,challenge,bytes((len(data),))+data)
    expected=bytes((len(data),))+struct.pack("<I",binascii.crc32(data)&0xffffffff)
    first=transact(port,request,0x83,3,challenge,timeout_s=1.5)
    if first!=expected:raise NodeLinkError("BAD_CRC32_RESULT")
    replay=transact(port,request,0x83,3,challenge,timeout_s=1.5)
    if replay!=first:raise NodeLinkError("BAD_DUPLICATE_RESULT")
    status=encode(4,4,challenge)
    result=transact(port,status,0x84,4,challenge,retries=1)
    if result!=b"\x02\x00\x01\x00":raise NodeLinkError("BAD_STATUS")
    return {"hello":True,"ping":True,"crc32":True,"idempotence":True,"status":True}

def main(argv=None):
    a=argparse.ArgumentParser(description="ESP-01 Node Link v2 host smoke; no reset/Flash")
    a.add_argument("--execute",action="store_true",
                   help="Explicitly authorize opening the existing ESP CH340 UART")
    a.add_argument("--port",default=ESP_CH340_BY_ID)
    args=a.parse_args(argv)
    if not args.execute:
        print("NO_DEVICE_OPEN=YES; use --execute only under owner hardware authorization")
        return 0
    challenge=secrets.randbits(32) or 1
    with open_esp_uart(port=args.port) as port:
        time.sleep(.45)
        port.reset_input_buffer()
        result=smoke(port,challenge)
    for name,ok in result.items():
        print("PHYSICAL_NODE_"+name.upper()+"="+("PASS" if ok else "FAIL"),flush=True)
    print("ESP01_NODE_LINK_V2_RESULT=PASS FLASH_RESET=NO",flush=True)
    return 0

if __name__=="__main__":
    try:sys.exit(main())
    except (NodeLinkError,SerialPolicyError,OSError) as exc:
        print("ESP01_NODE_LINK_V2_RESULT=FAIL REASON="+str(exc)[:140],flush=True)
        sys.exit(2)
