"""Native-independent negative contract tests for libusb per-RPC diagnostics.

Never opens USB or claims acceptance. The framing sizes of the long response
are pinned to actual 2026-10-22 000C sample-0 schedprod wire-length metadata.
"""
import struct
import unittest

from libusb_frame_primitive import (
    RpcReader,Failure,crc16,wire
)
from test_tx_pressure import FRAME_WIRE_BYTES,RPC_END_WIRE_BYTES

class Fake:
    def __init__(self, responses):
        self.queue=list(responses)
        self.timeouts=[]
    def read(self,max_bytes,timeout_ms):
        self.timeouts.append(timeout_ms)
        if not self.queue:
            return (-7,b"",0)
        rc,data=self.queue.pop(0)
        return rc,data,len(data)

def frames(request=12,rpc=7):
    # Exact data-frame lengths captured in sample0 v3. Preserves total bytes
    # and framing boundaries, but contents are SYNTHETIC, not evidence bytes.
    parts=[]
    total=0
    for seq,frame_size in enumerate(FRAME_WIRE_BYTES):
        n=frame_size-16
        total+=n
        body=struct.pack("<HH",rpc,seq)+b"x"*n
        assert len(wire(0x82,request,body))==frame_size
        parts.append(wire(0x82,request,body))
    assert total==1183
    end=struct.pack("<HHIBB",rpc,len(parts),total,0,0)
    assert len(wire(0x83,request,end))==RPC_END_WIRE_BYTES
    parts.append(wire(0x83,request,end))
    return parts

def packetize(parts,packet=64):
    b=b"".join(parts)
    return [(0,b[n:n+packet]) for n in range(0,len(b),packet)]

class TestPrimitive(unittest.TestCase):
    def assertReject(self,responses,reason,*,request=12,rpc=7,expected_chunk=None):
        reader=RpcReader(Fake(responses))
        with self.assertRaises(Failure) as ctx:
            reader.rpc(request,rpc)
        e=ctx.exception
        self.assertEqual(e.reason,reason)
        self.assertFalse(e.receipt()["physical_gate_accepted"])
        self.assertEqual(e.request_id,request)
        self.assertEqual(e.rpc_id,rpc)
        if expected_chunk is not None:
            self.assertEqual(e.chunks,expected_chunk)
        return e

    def test_01_complete_long_v3_sized_response(self):
        packets=packetize(frames())
        read=Fake(packets)
        o=RpcReader(read).rpc(12,7)
        self.assertEqual((o.chunks,o.output_bytes),(33,1183))
        self.assertEqual(sum(len(x[1]) for x in packets),1733)
        self.assertEqual(len(o.events),len(packets))
        self.assertTrue(all(1<=ms<=2300 for ms in read.timeouts))

    def test_02_multi_frame_usb_packet_coalescing(self):
        # Multiple complete frames inside ONE LEGAL 64-byte USB packet.
        small=[wire(0x82,12,struct.pack("<HH",7,i)+b"xyz") for i in range(3)]
        coalesced=b"".join(small)
        self.assertLessEqual(len(coalesced),64)
        end=wire(0x83,12,struct.pack("<HHIBB",7,3,9,0,0))
        o=RpcReader(Fake([(0,coalesced),(0,end)])).rpc(12,7)
        self.assertEqual((o.chunks,o.output_bytes),(3,9))

    def test_03_one_byte_per_usb_read(self):
        o=RpcReader(Fake(packetize(frames(),1))).rpc(12,7)
        self.assertEqual(o.output_bytes,1183)

    def test_04_timeout_preserves_29_partial_bytes_and_rc(self):
        chunks=packetize(frames())
        fail=chunks[:7]+[(-7,b"y"*29)]
        e=self.assertReject(fail,"LIBUSB_TIMEOUT_RC_-7")
        self.assertEqual(e.events[-1].transferred,29)
        self.assertEqual(e.events[-1].libusb_reason,"TIMEOUT")
        self.assertGreater(e.chunks,0)
        self.assertGreater(e.remaining_stash,0)

    def test_05_device_disconnect_distinguished(self):
        e=self.assertReject([(-4,b"",)],"LIBUSB_NO_DEVICE_RC_-4",expected_chunk=0)
        self.assertEqual(e.events[0].libusb_rc,-4)

    def test_06_usb_pipe_stall_distinguished(self):
        self.assertReject([(-9,b"",)],"LIBUSB_PIPE_RC_-9")

    def test_07_io_error_distinguished(self):
        self.assertReject([(-1,b"",)],"LIBUSB_IO_RC_-1")

    def test_08_busy_distinguished(self):
        self.assertReject([(-6,b"",)],"LIBUSB_BUSY_RC_-6")

    def test_09_unknown_libusb_code_not_environment_automatically(self):
        e=self.assertReject([(-100,b"",)],"LIBUSB_UNKNOWN_RC_-100")
        self.assertEqual(e.events[0].libusb_reason,"UNKNOWN")

    def test_10_positive_libcode_zero_zero_data_fails(self):
        self.assertReject([(0,b"")],"LIBUSB_SUCCESS_ZERO_BYTES")

    def test_11_nonzero_rpc_end_keeps_status_domain_code(self):
        parts=frames()
        parts[-1]=wire(0x83,12,struct.pack("<HHIBB",7,33,1183,0,4))
        e=self.assertReject(packetize(parts),"RPC_END_STATUS_NONZERO",expected_chunk=33)
        self.assertEqual((e.end_domain,e.end_code),(0,4))
        self.assertEqual(e.output_bytes,1183)

    def test_12_bad_request_correlation(self):
        self.assertReject(packetize(frames(request=18)),"RPC_REQUEST_ID_MISMATCH")

    def test_13_bad_sequence(self):
        p=frames();p[2]=wire(0x82,12,struct.pack("<HH",7,25)+b"x"*(FRAME_WIRE_BYTES[2]-16))
        self.assertReject(packetize(p),"RPC_DATA_CORRELATION")

    def test_14_bad_crc(self):
        x=bytearray(b"".join(frames()))
        x[24]^=0x01
        self.assertReject(packetize([bytes(x)]),"FRAME_CRC_MISMATCH")

    def test_15_missing_end_waits_then_fails_with_partial_context(self):
        p=frames()[:-1]
        e=self.assertReject(packetize(p),"LIBUSB_TIMEOUT_RC_-7",expected_chunk=33)
        self.assertEqual(e.output_bytes,1183)

    def test_16_truncated_data_packet_then_timeout(self):
        truncated=b"".join(frames())[:75]
        e=self.assertReject(packetize([truncated]),"LIBUSB_TIMEOUT_RC_-7")
        self.assertEqual(e.chunks,1)
        self.assertGreater(e.remaining_stash,0)

    def test_17_short_rid_zero_rejected(self):
        with self.assertRaises(ValueError):
            RpcReader(Fake([])).rpc(0,7)

    def test_18_short_unexpected_adapter_len(self):
        class BadReader:
            def read(self,max_bytes,timeout_ms):return (0,b"123",4)
        with self.assertRaises(Failure) as ex:
            RpcReader(BadReader()).rpc(12,7)
        self.assertEqual(ex.exception.reason,"LIBUSB_ADAPTER_LENGTH_MISMATCH")

    def test_19_negative_timeout_validation(self):
        with self.assertRaises(ValueError):
            RpcReader(Fake([]),per_read_ms=10000)

    def test_21_error_receipt_keeps_bounded_wire_tail_and_recent_headers(self):
        p=frames()
        partially=b"".join(p)[:70]
        e=self.assertReject(packetize([partially]),"LIBUSB_TIMEOUT_RC_-7",expected_chunk=1)
        receipt=e.receipt()
        self.assertTrue(0<len(receipt["partial_tail_hex"])<=128)
        self.assertEqual(len(receipt["recent_frame_headers"]),1)
        self.assertEqual(receipt["recent_frame_headers"][0]["frame_type"],"RPC_DATA")
        self.assertEqual(receipt["read_events"][-1]["libusb_rc"],-7)

    def test_22_libcode_with_partial_data_cannot_be_mislabeled_pass(self):
        p=frames()
        cut=b"".join(p)[:15]
        e=self.assertReject([(-7,cut)],"LIBUSB_TIMEOUT_RC_-7",expected_chunk=0)
        self.assertEqual(e.remaining_stash,15)
        self.assertEqual(e.partial_tail_hex,cut.hex())

    def test_23_fake_over_64_byte_usb_packet_is_rejected(self):
        e=self.assertReject([(0,b"x"*65)],"LIBUSB_ADAPTER_LENGTH_MISMATCH")
        self.assertEqual(e.events[0].transferred,65)

    def test_20_absolute_deadline_clamps_transfer_budget(self):
        class Clock:
            def __init__(self):self.now=0.0
            def __call__(self):return self.now
        clock=Clock()
        class TimeoutFake:
            def read(self,max_bytes,timeout_ms):
                clock.now+=timeout_ms/1000
                return (0,b"x",1)
        e=self.assertReject([(-7,b"")],"LIBUSB_TIMEOUT_RC_-7")
        fake=TimeoutFake()
        reader=RpcReader(fake,clock=clock,rpc_budget_ms=2400,per_read_ms=2300)
        with self.assertRaises(Failure) as ex:reader.rpc(12,7)
        self.assertEqual(ex.exception.reason,"RPC_ABSOLUTE_DEADLINE")
        self.assertLessEqual(sum(ev.requested_ms for ev in ex.exception.events),2400)

if __name__=="__main__":
    unittest.main(verbosity=2)
