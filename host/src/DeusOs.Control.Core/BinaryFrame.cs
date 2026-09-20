namespace DeusOs.Control.Core;

public sealed record BinaryFrame(
    byte Version,
    FrameType Type,
    byte Flags,
    byte Reserved,
    ushort RequestId,
    byte[] Payload);

public enum FrameDecodeErrorKind
{
    Sync,
    Crc,
    Length,
}

public sealed record FrameDecodeError(FrameDecodeErrorKind Kind, string Message);

public sealed record FrameDecodeBatch(
    IReadOnlyList<BinaryFrame> Frames,
    IReadOnlyList<FrameDecodeError> Errors);
