"""Pure, fail-closed Linux libusb framed-IN primitive (no direct USB access).

The injected reader interface is read(max_bytes:int,timeout_ms:int)
 -> (libusb_rc:int, received_bytes:bytes, transferred:int).
The native adaptor MUST supply actual libusb values without flattening them.
A single transport attempt never retries, reopens, resets or clears a USB halt.

No physical collector may be released until this exact primitive is executed on
the Linux USB host and correlated to raw native evidence.
"""
from __future__ import annotations
from dataclasses import dataclass
import struct
import time
from typing import Callable, Protocol

MAX_FRAME_PAYLOAD = 132
MAX_RPC_OUTPUT = 8192
MAX_STASH = 4096
LIBUSB_CODES = {
    0:"SUCCESS", -1:"IO", -2:"INVALID_PARAM", -3:"ACCESS",
    -4:"NO_DEVICE", -5:"NOT_FOUND", -6:"BUSY", -7:"TIMEOUT",
    -8:"OVERFLOW", -9:"PIPE", -10:"INTERRUPTED", -11:"NO_MEM",
    -12:"NOT_SUPPORTED"
}
FRAME_TYPES = {0x81:"HELLO",0x82:"RPC_DATA",0x83:"RPC_END",0x84:"PROTOCOL_ERROR"}

@dataclass(frozen=True)
class ReadEvent:
    index:int
    libusb_rc:int
    libusb_reason:str
    requested_ms:int
    transferred:int
    buffer_bytes:int
    stash_bytes:int
    frames_completed:int
    elapsed_ms:int
    at_ms:int  # monotonic milliseconds since this RPC started

@dataclass(frozen=True)
class Result:
    request_id:int
    rpc_id:int
    chunks:int
    output_bytes:int
    final_domain:int
    final_code:int
    events:tuple[ReadEvent,...]

class Failure(RuntimeError):
    def __init__(self,reason,*,request_id,rpc_id,chunks,output_bytes,
                 end_domain=None,end_code=None,events=(),remaining_stash=0,
                 partial_tail_hex="",frame_summaries=()):
        super().__init__(reason)
        self.reason=reason
        self.request_id=request_id
        self.rpc_id=rpc_id
        self.chunks=chunks
        self.output_bytes=output_bytes
        self.end_domain=end_domain
        self.end_code=end_code
        self.events=tuple(events)
        self.remaining_stash=remaining_stash
        self.partial_tail_hex=partial_tail_hex
        self.frame_summaries=tuple(frame_summaries)

    def receipt(self):
        return {
            "schema":"DEUS_LIBUSB_FRAME_FAILURE_V1",
            "result":"FAIL",
            "reason":self.reason,
            "request_id":self.request_id,
            "rpc_id":self.rpc_id,
            "chunks":self.chunks,
            "output_bytes":self.output_bytes,
            "end_domain":self.end_domain,
            "end_code":self.end_code,
            "remaining_stash":self.remaining_stash,
            "partial_tail_hex":self.partial_tail_hex,
            "recent_frame_headers":list(self.frame_summaries),
            "read_events":[e.__dict__ for e in self.events],
            "source_authenticated":False,
            "physical_gate_accepted":False
        }


class Reader(Protocol):
    def read(self,max_bytes:int,timeout_ms:int)->tuple[int,bytes,int]:...


def crc16(data:bytes)->int:
    x=0xffff
    for byte in data:
        x ^= byte << 8
        for _ in range(8):
            x=((x<<1)^0x1021)&0xffff if x&0x8000 else (x<<1)&0xffff
    return x


def wire(ft:int,rid:int,body:bytes)->bytes:
    if not 0 < rid <= 0xffff or len(body)>MAX_FRAME_PAYLOAD:
        raise ValueError("INVALID_WIRE_FIXTURE")
    head=struct.pack("<2sBBBBHH",b"\xa5\x5a",1,ft,0,0,rid,len(body))
    return head+body+struct.pack("<H",crc16(head[2:]+body))


class RpcReader:
    def __init__(self,reader:Reader,*,clock:Callable[[],float]=time.monotonic,
                 per_read_ms:int=2300,rpc_budget_ms:int=6000):
        if not 50<=per_read_ms<=5000 or not per_read_ms<=rpc_budget_ms<=30000:
            raise ValueError("UNBOUNDED_OR_INVALID_TIMEOUT")
        self.reader=reader
        self.clock=clock
        self.per_read_ms=per_read_ms
        self.rpc_budget_ms=rpc_budget_ms
        self.stash=bytearray()
        self._events:list[ReadEvent]=[]
        self._frames=0
        self._frame_summaries=[]
        self._poisoned=False
        self._rpc_start=0.0

    def _read_frame(self,deadline:float,fail):
        while True:
            if len(self.stash)>=10:
                if self.stash[:2]!=b"\xa5\x5a":
                    fail("FRAME_MAGIC_MISMATCH")
                n=struct.unpack_from("<H",self.stash,8)[0]
                if n>MAX_FRAME_PAYLOAD:
                    fail("FRAME_PAYLOAD_TOO_LONG")
                if len(self.stash)>=n+12:
                    frame=bytes(self.stash[:n+12])
                    del self.stash[:n+12]
                    if crc16(frame[2:-2])!=struct.unpack_from("<H",frame,len(frame)-2)[0]:
                        fail("FRAME_CRC_MISMATCH")
                    ver,ft,flags,res,rid,_=struct.unpack_from("<BBBBHH",frame,2)
                    if ver!=1 or flags!=0 or res!=0 or rid==0:
                        fail("FRAME_INVALID_HEADER")
                    self._frames+=1
                    return ft,rid,frame[10:-2]

            now=self.clock()
            left=deadline-now
            if left<=0:
                fail("RPC_ABSOLUTE_DEADLINE")
            timeout_ms=max(1,min(self.per_read_ms,int(left*1000)))
            start=self.clock()
            rc,data,n=self.reader.read(64,timeout_ms)
            finished=self.clock()
            elapsed=max(0,int((finished-start)*1000))
            at_ms=max(0,int((finished-self._rpc_start)*1000))
            # An adapter returning a short read or partial on TIMEOUT must
            # preserve the actual byte count. Never silently pretend success.
            if type(rc) is not int or type(n) is not int or type(data) is not bytes:
                fail("LIBUSB_ADAPTER_CONTRACT_INVALID")
            self._events.append(ReadEvent(
                len(self._events)+1,rc,LIBUSB_CODES.get(rc,"UNKNOWN"),
                timeout_ms,n,len(data),len(self.stash),self._frames,elapsed,at_ms
            ))
            if not 0<=n<=64 or n!=len(data):
                fail("LIBUSB_ADAPTER_LENGTH_MISMATCH")
            if len(self.stash)+n>MAX_STASH:
                fail("FRAME_STASH_OVERFLOW")
            # RFC: preserve bytes returned with a timeout even though the RPC
            # FAILS. They are crucial forensic evidence, not a valid frame.
            self.stash.extend(data)
            if rc!=0:
                fail("LIBUSB_"+LIBUSB_CODES.get(rc,"UNKNOWN")+"_RC_"+str(rc))
            if n==0:
                fail("LIBUSB_SUCCESS_ZERO_BYTES")

    def rpc(self,request_id:int,rpc_id:int)->Result:
        if not 0<request_id<=65535 or not 0<rpc_id<=65535:
            raise ValueError("INVALID_RPC_CONTEXT")
        if self._poisoned:
            raise RuntimeError("RPC_READER_POISONED_AFTER_FAILURE")
        self._events=[]
        self._frames=0
        self._frame_summaries=[]
        chunks=0
        output_bytes=0
        end_domain=None
        end_code=None
        self._rpc_start=self.clock()
        deadline=self._rpc_start+self.rpc_budget_ms/1000
        def fail(reason):
            self._poisoned=True
            raise Failure(reason,request_id=request_id,rpc_id=rpc_id,
                          chunks=chunks,output_bytes=output_bytes,
                          end_domain=end_domain,end_code=end_code,
                          events=self._events,remaining_stash=len(self.stash),
                          partial_tail_hex=bytes(self.stash[-64:]).hex(),
                          frame_summaries=self._frame_summaries[-12:])
        while True:
            ft,rid,body=self._read_frame(deadline,fail)
            summary={"frame_type":FRAME_TYPES.get(ft,"UNKNOWN"),
                     "request_id":rid,"payload_bytes":len(body)}
            if ft in (0x82,0x83) and len(body)>=4:
                summary["rpc_id"],summary["sequence"]=struct.unpack_from("<HH",body)
            self._frame_summaries.append(summary)
            if rid!=request_id:
                fail("RPC_REQUEST_ID_MISMATCH")
            if ft==0x82:
                if len(body)<4:
                    fail("RPC_DATA_SHORT")
                rid2,seq=struct.unpack_from("<HH",body)
                if rid2!=rpc_id or seq!=chunks or len(body)-4>48:
                    fail("RPC_DATA_CORRELATION")
                output_bytes+=len(body)-4
                chunks+=1
                if output_bytes>MAX_RPC_OUTPUT or chunks>200:
                    fail("RPC_UNBOUNDED_OUTPUT")
            elif ft==0x83:
                if len(body)!=10:
                    fail("RPC_END_BAD_LENGTH")
                rid2,seq,total,end_domain,end_code=struct.unpack("<HHIBB",body)
                if rid2!=rpc_id or seq!=chunks or total!=output_bytes:
                    fail("RPC_END_CORRELATION")
                if end_domain!=0 or end_code!=0:
                    fail("RPC_END_STATUS_NONZERO")
                # No unsolicited/pipelined frames exist in this one-request
                # transport contract. Trailing bytes must not be carried into
                # the next request and misreported as a successful response.
                if self.stash:
                    fail("RPC_TRAILING_BYTES_AFTER_END")
                return Result(request_id,rpc_id,chunks,output_bytes,
                              end_domain,end_code,tuple(self._events))
            elif ft==0x84:
                fail("PROTOCOL_ERROR_FRAME")
            else:
                fail("UNEXPECTED_FRAME_TYPE")
