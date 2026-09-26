using System.Buffers.Binary;

namespace DeusOs.Control.Core;

public enum AssetTransferOpcode : byte
{
    Begin = 0x01,
    WriteChunk = 0x02,
    Commit = 0x03,
    Abort = 0x04,
    Status = 0x05,
    ReadChunk = 0x06,
}

public enum AssetTransferStatus : byte
{
    Ok = 0,
    Malformed = 1,
    BadFlags = 2,
    BadOpcode = 3,
    BadObjectType = 4,
    BadSchema = 5,
    BadLength = 6,
    BadTransferId = 7,
    Busy = 8,
    BadState = 9,
    BadOffset = 10,
    DataConflict = 11,
    CrcMismatch = 12,
    ValidationFailed = 13,
    NotFound = 14,
    GenerationMismatch = 15,
    StorageError = 16,
    WearBudgetExhausted = 17,
    InternalError = 18,
}

public enum AssetTransferSessionState : byte
{
    None = 0,
    Receiving = 1,
    CompleteUncommitted = 2,
}

public sealed record AssetTransferResponse(
    AssetTransferOpcode Opcode,
    AssetTransferStatus Status,
    AssetTransferSessionState SessionState,
    ushort ObjectType,
    ushort TransferId,
    uint CommittedGeneration,
    ushort TotalLength,
    ushort NextOffset,
    byte[] Data)
{
    public bool IsSuccess => Status == AssetTransferStatus.Ok;

    public AssetCommittedMetadata? CommittedMetadata =>
        Opcode == AssetTransferOpcode.Status && Data.Length == 8
            ? new AssetCommittedMetadata(
                BinaryPrimitives.ReadUInt16LittleEndian(Data.AsSpan(0, 2)),
                BinaryPrimitives.ReadUInt32LittleEndian(Data.AsSpan(4, 4)))
            : null;
}

public sealed record AssetCommittedMetadata(
    ushort PayloadLength,
    uint PayloadCrc32);

public enum AssetAccessPolicy
{
    PublishedOnly,
    PrepublicationAcceptance,
}

public sealed record AssetStatusSnapshot(
    AssetTransferStatus Status,
    AssetTransferSessionState SessionState,
    ushort ObjectType,
    ushort TransferId,
    uint CommittedGeneration,
    ushort TransferTotalLength,
    ushort NextOffset,
    ushort CommittedPayloadLength,
    uint CommittedPayloadCrc32)
{
    public bool HasCommittedRecord => CommittedGeneration != 0;
}

public sealed record AssetCommitResult(
    uint Generation,
    ushort PayloadLength,
    uint PayloadCrc32,
    bool Changed);

public sealed record AssetReadResult(
    uint Generation,
    byte[] Payload,
    uint PayloadCrc32);

public static class AssetTransferProtocol
{
    public const byte Version = 1;
    public const byte AllowDestructive = 0x01;
    public const int ChunkMax = 32;
    public const int ObjectMax = 960;
    public const int CommonResponseBytes = 18;
    public const int OledUiLayoutBytes = 8;
    public const ushort OledUiLayoutObjectType = 0x0001;

    public static byte[] EncodeBegin(
        ushort objectType,
        ushort transferId,
        ushort totalLength,
        byte schemaVersion,
        uint payloadCrc32)
    {
        if (transferId == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(transferId));
        }

        if (totalLength is 0 or > ObjectMax)
        {
            throw new ArgumentOutOfRangeException(nameof(totalLength));
        }

        var payload = new byte[16];
        payload[0] = Version;
        payload[1] = (byte)AssetTransferOpcode.Begin;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), objectType);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4, 2), transferId);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(6, 2), totalLength);
        payload[8] = schemaVersion;
        payload[9] = 0;
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(10, 4), payloadCrc32);
        payload[14] = 0;
        payload[15] = 0;
        return payload;
    }

    public static byte[] EncodeWriteChunk(
        ushort objectType,
        ushort transferId,
        ushort offset,
        ReadOnlySpan<byte> data)
    {
        if (transferId == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(transferId));
        }

        if (data.Length is < 1 or > ChunkMax)
        {
            throw new ArgumentOutOfRangeException(nameof(data));
        }

        var payload = new byte[10 + data.Length];
        payload[0] = Version;
        payload[1] = (byte)AssetTransferOpcode.WriteChunk;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), objectType);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4, 2), transferId);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(6, 2), offset);
        payload[8] = checked((byte)data.Length);
        payload[9] = 0;
        data.CopyTo(payload.AsSpan(10));
        return payload;
    }

    public static byte[] EncodeCommit(ushort objectType, ushort transferId) =>
        EncodeShort(AssetTransferOpcode.Commit, objectType, transferId);

    public static byte[] EncodeAbort(ushort objectType, ushort transferId) =>
        EncodeShort(AssetTransferOpcode.Abort, objectType, transferId);

    public static byte[] EncodeStatus(ushort objectType, ushort transferId = 0) =>
        EncodeShort(AssetTransferOpcode.Status, objectType, transferId);

    public static byte[] EncodeReadChunk(
        ushort objectType,
        uint expectedGeneration,
        ushort offset,
        byte requestedLength)
    {
        if (requestedLength is 0 or > ChunkMax)
        {
            throw new ArgumentOutOfRangeException(nameof(requestedLength));
        }

        var payload = new byte[12];
        payload[0] = Version;
        payload[1] = (byte)AssetTransferOpcode.ReadChunk;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), objectType);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4, 4), expectedGeneration);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(8, 2), offset);
        payload[10] = requestedLength;
        payload[11] = 0;
        return payload;
    }

    public static AssetTransferResponse ParseResponse(
        BinaryFrame frame,
        AssetTransferOpcode expectedOpcode)
    {
        if (frame.Type != FrameType.AssetTransferResponse)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"expected AssetTransferResponse, got {frame.Type}");
        }

        var payload = frame.Payload;
        if (payload.Length < CommonResponseBytes)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"asset response payload {payload.Length} is shorter than {CommonResponseBytes}");
        }

        if (payload[0] != Version)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                $"asset protocol version {payload[0]} is unsupported");
        }

        var opcode = (AssetTransferOpcode)payload[1];
        if (opcode != expectedOpcode)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"asset response opcode {opcode} does not match {expectedOpcode}");
        }

        if (!Enum.IsDefined(typeof(AssetTransferStatus), payload[2]) ||
            !Enum.IsDefined(typeof(AssetTransferSessionState), payload[3]))
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "asset response contains unknown status/session state");
        }

        var dataLength = payload[16];
        if (payload[17] != 0 || dataLength > ChunkMax ||
            payload.Length != CommonResponseBytes + dataLength)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "asset response data length/reserved field is malformed");
        }

        if (opcode == AssetTransferOpcode.Status && dataLength != 8)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "asset STATUS response must carry exactly 8 metadata bytes");
        }

        if ((opcode is AssetTransferOpcode.Begin or
            AssetTransferOpcode.WriteChunk or
            AssetTransferOpcode.Commit or
            AssetTransferOpcode.Abort) &&
            dataLength != 0)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "asset non-read response unexpectedly carries data");
        }

        var data = payload.AsSpan(CommonResponseBytes, dataLength).ToArray();

        return new AssetTransferResponse(
            opcode,
            (AssetTransferStatus)payload[2],
            (AssetTransferSessionState)payload[3],
            BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(4, 2)),
            BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(6, 2)),
            BinaryPrimitives.ReadUInt32LittleEndian(payload.AsSpan(8, 4)),
            BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(12, 2)),
            BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(14, 2)),
            data);
    }

    public static AssetStatusSnapshot ToStatusSnapshot(
        AssetTransferResponse response)
    {
        var metadata = response.CommittedMetadata ??
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "Asset STATUS response metadata is missing");

        return new AssetStatusSnapshot(
            response.Status,
            response.SessionState,
            response.ObjectType,
            response.TransferId,
            response.CommittedGeneration,
            response.TotalLength,
            response.NextOffset,
            metadata.PayloadLength,
            metadata.PayloadCrc32);
    }

    private static byte[] EncodeShort(
        AssetTransferOpcode opcode,
        ushort objectType,
        ushort transferId)
    {
        if (opcode != AssetTransferOpcode.Status && transferId == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(transferId));
        }

        var payload = new byte[6];
        payload[0] = Version;
        payload[1] = (byte)opcode;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), objectType);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4, 2), transferId);
        return payload;
    }
}

public sealed record OledUiLayoutConfigV1(
    byte ConsoleX,
    byte ConsoleY,
    byte ConsoleWidth,
    byte ConsoleHeight)
{
    public const byte SchemaVersion = 1;

    public byte[] Serialize()
    {
        Validate();
        return new byte[]
        {
            SchemaVersion,
            ConsoleX,
            ConsoleY,
            ConsoleWidth,
            ConsoleHeight,
            0,
            0,
            0,
        };
    }

    public static OledUiLayoutConfigV1 Default { get; } =
        new(1, 10, 126, 22);

    public static OledUiLayoutConfigV1 Deserialize(ReadOnlySpan<byte> payload)
    {
        if (payload.Length != 8 ||
            payload[0] != SchemaVersion ||
            payload[5] != 0 ||
            payload[6] != 0 ||
            payload[7] != 0)
        {
            throw new ArgumentException(
                "invalid OLED_UI_LAYOUT_CONFIG_V1 payload",
                nameof(payload));
        }

        var result = new OledUiLayoutConfigV1(
            payload[1],
            payload[2],
            payload[3],
            payload[4]);
        result.Validate();
        return result;
    }

    private void Validate()
    {
        if (ConsoleY < 10 ||
            ConsoleWidth < 5 ||
            ConsoleHeight < 6 ||
            (int)ConsoleX + ConsoleWidth > 128 ||
            (int)ConsoleY + ConsoleHeight > 32)
        {
            throw new ArgumentOutOfRangeException(
                nameof(OledUiLayoutConfigV1),
                "console rectangle violates OLED_UI_LAYOUT_CONFIG_V1 bounds");
        }
    }
}
