"""Engineering ONLY: replay real v3 frame lengths against source 1024-byte Mgmt TX ring.

Original evidence ZIP remains on Windows; list contains only non-sensitive
wire-frame byte counts reconstructed from v3 RPC payload, not raw RPC output.
USB IRQ drain timing is UNKNOWN. This is NOT evidence of physical TX drops.
"""
import unittest

# 33 RPC_DATA frames extracted by canonical 'newline or 48 data bytes' rules;
# exact v3 observed payload=1183 and frames=33. Wire frame has 16-byte overhead.
FRAME_WIRE_BYTES = (46,45,50,47,43,52,51,54,57,57,51,54,56,52,54,54,57,
                    55,56,55,58,54,56,56,50,50,50,51,53,55,56,45,31)
RPC_END_WIRE_BYTES = 22
CAPACITY = 1024
MAX_IN_PACKET = 64

def simulate(drained_bytes_per_frame=0):
    """Thread-mode atomic enqueue and a synthetic bounded USB-IRQ drain.

    Not an STM32 timing model. Returns first no-space event; never retries a frame.
    """
    depth=0
    for number, wire in enumerate((*FRAME_WIRE_BYTES, RPC_END_WIRE_BYTES), 1):
        if depth+wire > CAPACITY:
            return (False, number, depth, wire)
        depth+=wire
        depth-=min(depth,drained_bytes_per_frame)
    return (True, number, depth, 0)

class RingPressureTests(unittest.TestCase):
    def test_01_exact_real_v3_frame_metadata(self):
        self.assertEqual(len(FRAME_WIRE_BYTES),33)
        self.assertEqual(sum(FRAME_WIRE_BYTES)-16*33,1183)
        self.assertEqual(sum(FRAME_WIRE_BYTES)+RPC_END_WIRE_BYTES,1733)

    def test_02_atomic_failure_without_irq_drain(self):
        ok,frame,depth,next_len=simulate(0)
        self.assertFalse(ok)
        self.assertEqual(frame,20)
        self.assertEqual(depth,991)
        self.assertEqual(next_len,55)

    def test_03_irq_drain_can_make_same_response_succeed(self):
        self.assertTrue(simulate(MAX_IN_PACKET)[0])

    def test_04_aggregate_wire_exceeds_queue_without_drain(self):
        self.assertEqual(sum(FRAME_WIRE_BYTES)+RPC_END_WIRE_BYTES-CAPACITY,709)

    def test_05_model_never_fabricates_product_failure(self):
        self.assertFalse(simulate(0)[0])
        self.assertTrue(simulate(64)[0])
        # Neither branch proves what the installed device's IRQ actually did.

if __name__=="__main__":
    unittest.main(verbosity=2)
