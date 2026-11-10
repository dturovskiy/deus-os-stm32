"""OFFLINE ESP-01 Flash Bridge v0 reference contract, NOT device firmware.

Proposed Management frame types 0x05/0x87 are NOT in installed STM32 v8.
This pure-Python model never opens UART, USB, CH340, ST-LINK or flashes.
A future STM32 implementation requires explicit GPIO ownership, a new
firmware resource proof, signed update and separate physical authorization.
"""
from __future__ import annotations

import struct
from dataclasses import dataclass, field

MAGIC = b"\xA5\x5A"
REQUEST_TYPE = 0x05  # PROPOSED, not assigned in the deployed protocol
RESPONSE_TYPE = 0x87  # PROPOSED, not assigned in the deployed protocol
MAX_DATA = 40         # current USART2 TX cap; also fits USB IN 64B
MAX_REPLY_WIRE = 64
OPEN, TX, RX, CLOSE = 1, 2, 3, 4
OK, DENIED, BAD_SESSION, BAD_SEQUENCE, BAD_LENGTH, UART_UNCERTAIN = range(6)


class BridgeError(ValueError):
    pass


def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = (((crc << 1) ^ 0x1021) if crc & 0x8000 else crc << 1) & 0xFFFF
    return crc


def encode_frame(kind: int, request_id: int, payload: bytes) -> bytes:
    if not (0 <= kind <= 255 and 1 <= request_id <= 65535
            and len(payload) <= 132):
        raise BridgeError("BAD_FRAME_ARGUMENT")
    header = struct.pack("<BBBBHH", 1, kind, 0, 0, request_id, len(payload))
    content = header + payload
    return MAGIC + content + struct.pack("<H", crc16(content))


def decode_frame(wire: bytes, *, expected_type: int) -> tuple[int, bytes]:
    if len(wire) < 12 or len(wire) > 144 or wire[:2] != MAGIC:
        raise BridgeError("BAD_WIRE")
    version, kind, flags, reserved, request_id, n = struct.unpack(
        "<BBBBHH", wire[2:10])
    if (version, kind, flags, reserved) != (1, expected_type, 0, 0):
        raise BridgeError("BAD_HEADER")
    if request_id == 0 or n > 132 or len(wire) != n + 12:
        raise BridgeError("BAD_LENGTH")
    if int.from_bytes(wire[-2:], "little") != crc16(wire[2:-2]):
        raise BridgeError("BAD_CRC")
    if expected_type == RESPONSE_TYPE and len(wire) > MAX_REPLY_WIRE:
        raise BridgeError("OVERSIZED_USB_REPLY")
    return request_id, wire[10:-2]


def request(op: int, session: int, seq: int, data: bytes = b"",
            *, request_id: int = 1) -> bytes:
    if (op not in (OPEN, TX, RX, CLOSE) or
        not 1 <= session <= 0xFFFFFFFF or
        not 0 <= seq <= 0xFFFF or len(data) > MAX_DATA or
        (op != TX and len(data) != 0) or
        (op == TX and not data)):
        raise BridgeError("INVALID_BRIDGE_REQUEST")
    payload = struct.pack("<BIHB", op, session, seq, len(data)) + data
    return encode_frame(REQUEST_TYPE, request_id, payload)


def response(op: int, session: int, seq: int, status: int,
             data: bytes = b"", *, request_id: int = 1) -> bytes:
    if (len(data) > MAX_DATA or not 0 <= status <= UART_UNCERTAIN):
        raise BridgeError("INVALID_BRIDGE_RESPONSE")
    payload = struct.pack("<BIHBB", op, session, seq, status, len(data)) + data
    result = encode_frame(RESPONSE_TYPE, request_id, payload)
    if len(result) > MAX_REPLY_WIRE:
        raise BridgeError("USB_REPLY_OVERFLOW")
    return result


def parse_response(wire: bytes, *, request_id: int, op: int,
                   session: int, seq: int) -> tuple[int, bytes]:
    got_id, payload = decode_frame(wire, expected_type=RESPONSE_TYPE)
    if got_id != request_id or len(payload) < 9:
        raise BridgeError("BAD_REPLY_ID_OR_SIZE")
    got_op, got_session, got_seq, status, count = struct.unpack(
        "<BIHBB", payload[:9])
    if (got_op, got_session, got_seq) != (op, session, seq):
        raise BridgeError("STALE_OR_CROSSED_REPLY")
    if count > MAX_DATA or len(payload) != 9 + count or status > UART_UNCERTAIN:
        raise BridgeError("BAD_REPLY_PAYLOAD")
    return status, payload[9:]


@dataclass
class OfflineBridgeModel:
    """Deterministic mock only; no physical reset/boot/flash functionality."""

    control_owned: bool = False
    node_quiescent: bool = False
    approved: bool = False
    active: bool = False
    faulted: bool = False
    session: int = 0
    last_seq: int = 0
    tx_bytes: bytearray = field(default_factory=bytearray)
    rx_bytes: bytearray = field(default_factory=bytearray)
    fail_next_write: bool = False

    def handle(self, wire: bytes) -> bytes:
        request_id, payload = decode_frame(wire, expected_type=REQUEST_TYPE)
        if len(payload) < 8:
            raise BridgeError("MALFORMED_BRIDGE_PAYLOAD")
        op, sid, seq, count = struct.unpack("<BIHB", payload[:8])
        if (op not in (OPEN, TX, RX, CLOSE) or sid == 0 or
            count > MAX_DATA or len(payload) != count + 8 or
            (op != TX and count != 0) or
            (op == TX and count == 0)):
            raise BridgeError("MALFORMED_BRIDGE_PAYLOAD")
        data = payload[8:]
        status, result = DENIED, b""
        if op == OPEN:
            if (seq == 0 and not self.active and not self.faulted and
                self.control_owned and self.node_quiescent and self.approved):
                self.active, self.session, self.last_seq = True, sid, 0
                status = OK
        elif not self.active or self.faulted or sid != self.session:
            status = BAD_SESSION
        elif seq != self.last_seq + 1:
            status = BAD_SEQUENCE
        elif op == TX:
            if self.fail_next_write:
                self.fail_next_write = False
                self.faulted = True
                status = UART_UNCERTAIN  # partial transmit cannot be replayed
            else:
                self.tx_bytes.extend(data)
                self.last_seq = seq
                status = OK
        elif op == RX:
            result = bytes(self.rx_bytes[:MAX_DATA])
            del self.rx_bytes[:len(result)]
            self.last_seq = seq
            status = OK
        elif op == CLOSE:
            self.active = False
            self.session = 0
            self.last_seq = 0
            # This does NOT reset ESP, restore firmware mode, or mark Node ready.
            status = OK
        return response(op, sid, seq, status, result, request_id=request_id)
