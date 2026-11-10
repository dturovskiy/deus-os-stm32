"""Offline end-to-end host Node Link tests with in-memory transport only."""
from __future__ import annotations
import binascii
import struct
import unittest

import node_link_smoke_v2 as link

class MemoryEspNode:
    def __init__(self,*,noise=False,lost_first_hello=False,bad_crc=False):
        self.out=bytearray()
        self.sid=0
        self.last_job=None
        self.done=0
        self.received=[]
        self.noise=noise
        self.lost_first_hello=lost_first_hello
        self.bad_crc=bad_crc
        self.hello_count=0
    def write(self,request:bytes):
        parsed=link.decode(request)
        if parsed is None:raise AssertionError("HOST_WRONG_FRAME")
        kind,seq,sid,payload=parsed
        self.received.append(request)
        if kind==1:
            self.hello_count+=1
            assert seq==1 and sid==0 and len(payload)==4
            self.sid=struct.unpack("<I",payload)[0]
            self.done=0
            self.last_job=None
            reply=link.encode(0x81,1,self.sid,payload+link.CAPS)
        else:
            assert sid==self.sid and self.sid!=0
            if kind==2:
                reply=link.encode(0x82,seq,self.sid)
            elif kind==3:
                assert payload==b"\x04DEUS"
                if request!=self.last_job:
                    self.done+=1
                    self.last_job=request
                expected=bytes((4,))+struct.pack("<I",binascii.crc32(b"DEUS")&0xffffffff)
                if self.bad_crc:expected=b"\x04\0\0\0\0"
                reply=link.encode(0x83,seq,self.sid,expected)
            elif kind==4:
                reply=link.encode(0x84,seq,self.sid,b"\x02\x00"+struct.pack("<H",self.done))
            else:raise AssertionError("UNEXPECTED_KIND")
        if kind==1 and self.lost_first_hello and self.hello_count==1:
            return len(request)
        if self.noise:
            self.out.extend(b"noise\xD5")
            stale=link.encode(0x81 if kind==1 else (kind|0x80),seq,self.sid^1)
            self.out.extend(stale)
        self.out.extend(reply)
        return len(request)
    def read(self,n:int):
        take=bytes(self.out[:n])
        del self.out[:n]
        return take
    def flush(self):pass

class SoftwareTests(unittest.TestCase):
    def test_crc16_known_reference(self):
        self.assertEqual(link.crc16(b"123456789"),0x29B1)
    def test_frame_encode_decode(self):
        f=link.encode(1,1,0,b"\x78\x56\x34\x12")
        self.assertEqual(f[:14],b"\xD5\x4E\x01\x01\x01\x00\x01\x00\x04\x00\x00\x00\x00\x00")
        self.assertEqual(link.decode(f),(1,1,0,b"\x78\x56\x34\x12"))
        broken=bytearray(f);broken[-1]^=1
        self.assertIsNone(link.decode(bytes(broken)))
    def test_all_five_cases_and_duplicate_once(self):
        node=MemoryEspNode()
        self.assertEqual(len(link.smoke(node,0xC0FFEE01)),5)
        self.assertEqual(node.done,1)
        self.assertEqual(len(node.received),5)
        self.assertEqual(node.received[2],node.received[3])
    def test_noise_and_stale_reply_are_discarded(self):
        node=MemoryEspNode(noise=True)
        self.assertEqual(len(link.smoke(node,0x1A2B3C4D)),5)
    def test_hello_retry_idempotent(self):
        node=MemoryEspNode(lost_first_hello=True)
        self.assertEqual(len(link.smoke(node,0xC0FFEE02)),5)
        self.assertEqual(node.hello_count,2)
        self.assertEqual(node.received[0],node.received[1])
    def test_bad_result_is_fail_closed(self):
        node=MemoryEspNode(bad_crc=True)
        with self.assertRaisesRegex(link.NodeLinkError,"BAD_CRC32_RESULT"):
            link.smoke(node,0xC0FFEE03)
    def test_no_explicit_execute_no_device_open(self):
        self.assertEqual(link.main([]),0)

if __name__=="__main__":
    unittest.main(verbosity=2)
