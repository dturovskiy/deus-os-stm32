using System.Buffers.Binary;

namespace DeusOs.Control.Core;

public static class HelloParser
{
    public static HelloInfo Parse(BinaryFrame frame)
    {
        if (frame.Type != FrameType.HelloResponse)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"expected HELLO response, got {frame.Type}");
        }

        if (frame.Version != ProtocolConstants.ProtocolVersion ||
            frame.Flags != 0 ||
            frame.Reserved != 0 ||
            frame.Payload.Length != 16)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "malformed HELLO response");
        }

        var payload = frame.Payload.AsSpan();
        if (payload[3] != 0)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "HELLO reserved byte is nonzero");
        }

        return new HelloInfo(
            payload[0],
            payload[1],
            payload[2],
            BinaryPrimitives.ReadUInt16LittleEndian(payload[4..6]),
            BinaryPrimitives.ReadUInt16LittleEndian(payload[6..8]),
            BinaryPrimitives.ReadUInt16LittleEndian(payload[8..10]),
            BinaryPrimitives.ReadUInt16LittleEndian(payload[10..12]),
            BinaryPrimitives.ReadUInt32LittleEndian(payload[12..16]));
    }
}
