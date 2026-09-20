using System.Buffers.Binary;

namespace DeusOs.Control.Core;

public sealed class BinaryFrameDecoder
{
    private enum DecoderState
    {
        Idle,
        Magic1,
        Header,
        Payload,
        CrcLow,
        CrcHigh,
        Resync,
        ResyncMagic1,
    }

    private readonly byte[] _header = new byte[ProtocolConstants.HeaderBytesAfterMagic];
    private readonly byte[] _payload = new byte[ProtocolConstants.MaxPayloadBytes];
    private DecoderState _state;
    private int _headerIndex;
    private int _payloadIndex;
    private int _payloadLength;
    private ushort _receivedCrcLow;

    public BinaryFrameDecoder()
    {
        Reset();
    }

    public void Reset()
    {
        _state = DecoderState.Idle;
        _headerIndex = 0;
        _payloadIndex = 0;
        _payloadLength = 0;
        _receivedCrcLow = 0;
    }

    public FrameDecodeBatch Feed(ReadOnlySpan<byte> bytes)
    {
        var frames = new List<BinaryFrame>();
        var errors = new List<FrameDecodeError>();

        foreach (var value in bytes)
        {
            FeedByte(value, frames, errors);
        }

        return new FrameDecodeBatch(frames, errors);
    }

    private void FeedByte(
        byte value,
        List<BinaryFrame> frames,
        List<FrameDecodeError> errors)
    {
        switch (_state)
        {
            case DecoderState.Idle:
                if (value == ProtocolConstants.Magic0)
                {
                    _state = DecoderState.Magic1;
                }
                break;

            case DecoderState.Magic1:
                if (value == ProtocolConstants.Magic1)
                {
                    _state = DecoderState.Header;
                    _headerIndex = 0;
                    _payloadIndex = 0;
                    _payloadLength = 0;
                }
                else
                {
                    errors.Add(new FrameDecodeError(
                        FrameDecodeErrorKind.Sync,
                        $"second magic byte mismatch: 0x{value:X2}"));
                    _state = value == ProtocolConstants.Magic0
                        ? DecoderState.Magic1
                        : DecoderState.Idle;
                }
                break;

            case DecoderState.Header:
                _header[_headerIndex++] = value;
                if (_headerIndex == _header.Length)
                {
                    _payloadLength = BinaryPrimitives.ReadUInt16LittleEndian(
                        _header.AsSpan(6, 2));

                    if (_payloadLength > ProtocolConstants.MaxPayloadBytes)
                    {
                        errors.Add(new FrameDecodeError(
                            FrameDecodeErrorKind.Length,
                            $"payload length {_payloadLength} exceeds {ProtocolConstants.MaxPayloadBytes}"));
                        _state = DecoderState.Resync;
                    }
                    else if (_payloadLength == 0)
                    {
                        _state = DecoderState.CrcLow;
                    }
                    else
                    {
                        _payloadIndex = 0;
                        _state = DecoderState.Payload;
                    }
                }
                break;

            case DecoderState.Payload:
                _payload[_payloadIndex++] = value;
                if (_payloadIndex == _payloadLength)
                {
                    _state = DecoderState.CrcLow;
                }
                break;

            case DecoderState.CrcLow:
                _receivedCrcLow = value;
                _state = DecoderState.CrcHigh;
                break;

            case DecoderState.CrcHigh:
                CompleteFrame(value, frames, errors);
                break;

            case DecoderState.Resync:
                if (value == ProtocolConstants.Magic0)
                {
                    _state = DecoderState.ResyncMagic1;
                }
                break;

            case DecoderState.ResyncMagic1:
                if (value == ProtocolConstants.Magic1)
                {
                    _state = DecoderState.Header;
                    _headerIndex = 0;
                    _payloadIndex = 0;
                    _payloadLength = 0;
                }
                else if (value != ProtocolConstants.Magic0)
                {
                    _state = DecoderState.Resync;
                }
                break;

            default:
                Reset();
                break;
        }
    }

    private void CompleteFrame(
        byte crcHigh,
        List<BinaryFrame> frames,
        List<FrameDecodeError> errors)
    {
        Span<byte> crcInput = stackalloc byte[
            ProtocolConstants.HeaderBytesAfterMagic + ProtocolConstants.MaxPayloadBytes];

        _header.AsSpan().CopyTo(crcInput);
        if (_payloadLength != 0)
        {
            _payload.AsSpan(0, _payloadLength).CopyTo(
                crcInput[ProtocolConstants.HeaderBytesAfterMagic..]);
        }

        var expected = Crc16CcittFalse.Compute(
            crcInput[..(ProtocolConstants.HeaderBytesAfterMagic + _payloadLength)]);
        var received = (ushort)(_receivedCrcLow | (crcHigh << 8));

        if (received != expected)
        {
            errors.Add(new FrameDecodeError(
                FrameDecodeErrorKind.Crc,
                $"CRC mismatch received=0x{received:X4} expected=0x{expected:X4}"));
            Reset();
            return;
        }

        var payload = new byte[_payloadLength];
        if (_payloadLength != 0)
        {
            _payload.AsSpan(0, _payloadLength).CopyTo(payload);
        }

        frames.Add(new BinaryFrame(
            _header[0],
            (FrameType)_header[1],
            _header[2],
            _header[3],
            BinaryPrimitives.ReadUInt16LittleEndian(_header.AsSpan(4, 2)),
            payload));

        Reset();
    }
}
