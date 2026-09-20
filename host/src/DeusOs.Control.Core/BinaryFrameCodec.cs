using System.Buffers.Binary;

namespace DeusOs.Control.Core;

public static class BinaryFrameCodec
{
    public static byte[] Encode(
        FrameType type,
        byte flags,
        ushort requestId,
        ReadOnlySpan<byte> payload)
    {
        if (payload.Length > ProtocolConstants.MaxPayloadBytes)
        {
            throw new ArgumentOutOfRangeException(nameof(payload));
        }

        var wire = new byte[
            ProtocolConstants.FixedPrefixBytes +
            payload.Length +
            ProtocolConstants.CrcBytes];

        wire[0] = ProtocolConstants.Magic0;
        wire[1] = ProtocolConstants.Magic1;
        wire[2] = ProtocolConstants.ProtocolVersion;
        wire[3] = (byte)type;
        wire[4] = flags;
        wire[5] = 0;
        BinaryPrimitives.WriteUInt16LittleEndian(wire.AsSpan(6, 2), requestId);
        BinaryPrimitives.WriteUInt16LittleEndian(wire.AsSpan(8, 2), checked((ushort)payload.Length));
        payload.CopyTo(wire.AsSpan(ProtocolConstants.FixedPrefixBytes));

        var crc = Crc16CcittFalse.Compute(
            wire.AsSpan(2, ProtocolConstants.HeaderBytesAfterMagic + payload.Length));
        BinaryPrimitives.WriteUInt16LittleEndian(
            wire.AsSpan(ProtocolConstants.FixedPrefixBytes + payload.Length, 2),
            crc);

        return wire;
    }
}
