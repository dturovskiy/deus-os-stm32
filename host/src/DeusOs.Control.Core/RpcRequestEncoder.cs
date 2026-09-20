using System.Buffers.Binary;
using System.Text;

namespace DeusOs.Control.Core;

public static class RpcRequestEncoder
{
    public static byte[] EncodePayload(ushort rpcId, IReadOnlyList<string> arguments)
    {
        ArgumentNullException.ThrowIfNull(arguments);

        if (arguments.Count > ProtocolConstants.MaxArgs)
        {
            throw new ArgumentOutOfRangeException(
                nameof(arguments),
                $"at most {ProtocolConstants.MaxArgs} arguments are supported");
        }

        var encoded = new List<byte[]>(arguments.Count);
        var total = 4;

        foreach (var argument in arguments)
        {
            ArgumentNullException.ThrowIfNull(argument);
            var bytes = Encoding.UTF8.GetBytes(argument);

            if (bytes.Length > ProtocolConstants.MaxArgBytes)
            {
                throw new ArgumentOutOfRangeException(
                    nameof(arguments),
                    $"argument exceeds {ProtocolConstants.MaxArgBytes} bytes");
            }

            if (bytes.Contains((byte)0))
            {
                throw new ArgumentException(
                    "arguments may not contain NUL",
                    nameof(arguments));
            }

            encoded.Add(bytes);
            total += 1 + bytes.Length;
        }

        if (total > ProtocolConstants.MaxPayloadBytes)
        {
            throw new ArgumentOutOfRangeException(
                nameof(arguments),
                "RPC request payload exceeds protocol maximum");
        }

        var payload = new byte[total];
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(0, 2), rpcId);
        payload[2] = checked((byte)arguments.Count);
        payload[3] = 0;

        var offset = 4;
        foreach (var bytes in encoded)
        {
            payload[offset++] = checked((byte)bytes.Length);
            bytes.CopyTo(payload, offset);
            offset += bytes.Length;
        }

        return payload;
    }
}
