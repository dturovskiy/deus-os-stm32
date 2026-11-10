"""Focused, hardware-free checks for the proposed v0 USB/USART2 bridge."""
import unittest
import esp_flash_bridge_v0 as bridge


class BridgeV0OfflineTests(unittest.TestCase):
    def setup_model(self):
        model = bridge.OfflineBridgeModel(
            control_owned=True, node_quiescent=True, approved=True)
        self.call(model, bridge.OPEN, 0)
        return model

    def call(self, model, op, seq, data=b"", session=0xC0FFEE01, rid=None):
        rid = (seq + 1) if rid is None else rid
        frame = bridge.request(op, session, seq, data, request_id=rid)
        reply = model.handle(frame)
        return bridge.parse_response(reply, request_id=rid,
                                     op=op, session=session, seq=seq)

    def test_crc_exact_standard_reference(self):
        self.assertEqual(bridge.crc16(b"123456789"), 0x29B1)

    def test_frame_roundtrip_and_crc_rejection(self):
        raw = bridge.request(bridge.TX, 5, 1, b"\x00\xC0\xDB", request_id=7)
        self.assertEqual(bridge.decode_frame(
            raw, expected_type=bridge.REQUEST_TYPE)[0], 7)
        damaged = bytearray(raw)
        damaged[-1] ^= 0x01
        with self.assertRaisesRegex(bridge.BridgeError, "BAD_CRC"):
            bridge.decode_frame(bytes(damaged),
                                expected_type=bridge.REQUEST_TYPE)

    def test_boot_control_and_explicit_approval_required(self):
        for kwargs in (
            {}, {"control_owned": True},
            {"control_owned": True, "node_quiescent": True},
            {"node_quiescent": True, "approved": True},
        ):
            model = bridge.OfflineBridgeModel(**kwargs)
            status, _ = self.call(model, bridge.OPEN, 0)
            self.assertEqual(status, bridge.DENIED)
            self.assertFalse(model.active)
            self.assertEqual(len(model.tx_bytes), 0)

    def test_no_flash_tx_without_open_session(self):
        model = bridge.OfflineBridgeModel()
        status, _ = self.call(model, bridge.TX, 1, b"\xC0")
        self.assertEqual(status, bridge.BAD_SESSION)
        self.assertEqual(model.tx_bytes, b"")

    def test_proposed_40byte_uart_and_64byte_reply_bounds(self):
        model = self.setup_model()
        raw = bytes(range(40))
        self.assertEqual(self.call(model, bridge.TX, 1, raw)[0], bridge.OK)
        self.assertEqual(bytes(model.tx_bytes), raw)
        model.rx_bytes.extend(raw)
        frame = bridge.request(bridge.RX, model.session, 2, request_id=3)
        reply = model.handle(frame)
        self.assertLessEqual(len(reply), 64)
        self.assertEqual(bridge.parse_response(
            reply, request_id=3, op=bridge.RX,
            session=model.session, seq=2), (bridge.OK, raw))

    def test_41byte_tx_rejected_before_io(self):
        model = self.setup_model()
        with self.assertRaisesRegex(bridge.BridgeError, "INVALID_BRIDGE_REQUEST"):
            self.call(model, bridge.TX, 1, bytes(range(41)))
        self.assertEqual(model.tx_bytes, b"")

    def test_rx_drain_chunked_and_no_data_loss(self):
        model = self.setup_model()
        model.rx_bytes.extend(bytes(range(80)))
        self.assertEqual(self.call(model, bridge.RX, 1)[1], bytes(range(40)))
        self.assertEqual(self.call(model, bridge.RX, 2)[1], bytes(range(40, 80)))
        self.assertEqual(self.call(model, bridge.RX, 3)[1], b"")

    def test_duplicate_tx_not_replayed(self):
        model = self.setup_model()
        self.assertEqual(self.call(model, bridge.TX, 1, b"DEUS")[0], bridge.OK)
        self.assertEqual(self.call(model, bridge.TX, 1, b"DEUS")[0],
                         bridge.BAD_SEQUENCE)
        self.assertEqual(bytes(model.tx_bytes), b"DEUS")

    def test_out_of_order_sequence_denied(self):
        model = self.setup_model()
        status, _ = self.call(model, bridge.TX, 2, b"\xC0")
        self.assertEqual(status, bridge.BAD_SEQUENCE)
        self.assertFalse(model.tx_bytes)

    def test_wrong_session_denied(self):
        model = self.setup_model()
        status, _ = self.call(model, bridge.TX, 1, b"\x00",
                              session=0xC0FFEE02)
        self.assertEqual(status, bridge.BAD_SESSION)
        self.assertFalse(model.tx_bytes)

    def test_partial_uart_write_latches_fault_and_no_retry(self):
        model = self.setup_model()
        model.fail_next_write = True
        status, _ = self.call(model, bridge.TX, 1, b"\xC0")
        self.assertEqual(status, bridge.UART_UNCERTAIN)
        self.assertTrue(model.faulted)
        self.assertEqual(self.call(model, bridge.TX, 1, b"\xC0")[0],
                         bridge.BAD_SESSION)
        self.assertEqual(self.call(model, bridge.CLOSE, 1)[0],
                         bridge.BAD_SESSION)
        self.assertEqual(self.call(model, bridge.OPEN, 0)[0],
                         bridge.DENIED)
        self.assertEqual(model.tx_bytes, b"")

    def test_close_does_not_claim_esp_normal_boot(self):
        model = self.setup_model()
        self.assertEqual(self.call(model, bridge.CLOSE, 1)[0], bridge.OK)
        self.assertFalse(model.active)
        self.assertEqual(model.session, 0)
        self.assertEqual(self.call(model, bridge.TX, 2, b"\xC0")[0],
                         bridge.BAD_SESSION)

    def test_request_and_response_correlation_rejects_stale_reply(self):
        raw = bridge.response(bridge.RX, 0x1234, 3, bridge.OK, b"data",
                              request_id=9)
        with self.assertRaisesRegex(bridge.BridgeError, "BAD_REPLY_ID_OR_SIZE"):
            bridge.parse_response(raw, request_id=10, op=bridge.RX,
                                  session=0x1234, seq=3)
        with self.assertRaisesRegex(bridge.BridgeError, "STALE_OR_CROSSED_REPLY"):
            bridge.parse_response(raw, request_id=9, op=bridge.RX,
                                  session=0x1234, seq=4)

    def test_corrupt_reply_and_unknown_opcode_fail_closed(self):
        raw = bridge.response(bridge.RX, 5, 1, bridge.OK, b"x", request_id=2)
        broken = bytearray(raw)
        broken[-2] ^= 0x40
        with self.assertRaisesRegex(bridge.BridgeError, "BAD_CRC"):
            bridge.parse_response(bytes(broken), request_id=2,
                                  op=bridge.RX, session=5, seq=1)
        with self.assertRaisesRegex(bridge.BridgeError, "INVALID_BRIDGE_REQUEST"):
            bridge.request(99, 5, 1)

    def test_zero_session_and_payload_mismatch_rejected(self):
        with self.assertRaisesRegex(bridge.BridgeError, "INVALID_BRIDGE_REQUEST"):
            bridge.request(bridge.OPEN, 0, 0)
        with self.assertRaisesRegex(bridge.BridgeError, "INVALID_BRIDGE_REQUEST"):
            bridge.request(bridge.RX, 4, 1, b"x")


if __name__ == "__main__":
    unittest.main(verbosity=2)
