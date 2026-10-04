using System.Buffers.Binary;
using System.Security.Cryptography;

namespace DeusOs.Control.Core;

public enum FirmwareUpdateAccessPolicy
{
    PublishedOnly = 0,
    PrepublicationAcceptance = 1,
}

public enum FirmwareUpdateState : byte
{
    RecoveryIdle = 0x00,
    HeaderStaged = 0x01,
    Authorized = 0x02,
    Receiving = 0x03,
    Verifying = 0x04,
    Committed = 0x05,
    Resetting = 0x80,
}

public enum FirmwareUpdateStatus : byte
{
    Ok = 0x00,
    InvalidState = 0x01,
    BadLength = 0x02,
    BadFlags = 0x03,
    BadHeader = 0x04,
    AuthFailed = 0x05,
    TargetMismatch = 0x06,
    VersionRejected = 0x07,
    OutOfSequence = 0x08,
    FlashFailed = 0x09,
    DigestFailed = 0x0A,
    VectorInvalid = 0x0B,
    InternalError = 0x0C,
}

public sealed record FirmwareUpdateResponse(
    byte Opcode,
    FirmwareUpdateStatus Status,
    FirmwareUpdateState State,
    ushort ExpectedOffset,
    uint VersionFloor,
    uint CommittedVersion);

public sealed record FirmwareUpdatePackage(
    byte[] Header,
    byte[] HeaderAuthTag,
    byte[] Image,
    uint FirmwareVersion);

public sealed class FirmwareUpdateClient : IAsyncDisposable
{
    public const ushort BootloaderVendorId = 0x1209;
    public const ushort BootloaderProductId = 0x000D;
    public const uint ProductId = 0x534F4544;
    public const ushort TargetDeviceId = 0x0410;
    public const uint ApplicationOrigin = 0x08002000;
    public const int HeaderBytes = 48;
    public const int HeaderAuthTagBytes = 32;
    public const int MaxImageBytes = 53248;
    public const int MaxDataBytes = 48;
    public const int ApplicationPageBytes = 1024;

    private const byte DestructiveFlag = 0x01;
    private const byte OpcodeEnterBootloader = 0x01;
    private const byte OpcodeInfo = 0x02;
    private const byte OpcodeBegin = 0x03;
    private const byte OpcodeAuthorizeHeader = 0x04;
    private const byte OpcodeData = 0x05;
    private const byte OpcodeEnd = 0x06;
    private const int ResponseBytes = 16;

    private readonly DeviceProtocolChannel _channel;
    private readonly bool _ownsChannel;
    private bool _disposed;

    public FirmwareUpdateClient(IDeviceTransport transport)
    {
        _channel = new DeviceProtocolChannel(
            transport ?? throw new ArgumentNullException(nameof(transport)));
        _ownsChannel = true;
    }

    internal FirmwareUpdateClient(DeviceProtocolChannel channel)
    {
        _channel = channel ?? throw new ArgumentNullException(nameof(channel));
        _ownsChannel = false;
    }

    public static FirmwareUpdatePackage ParsePackage(ReadOnlySpan<byte> package)
    {
        if (package.Length < HeaderBytes + HeaderAuthTagBytes + 8)
        {
            throw new ArgumentException("firmware package is too short", nameof(package));
        }

        var header = package[..HeaderBytes];
        var imageLength = checked((int)BinaryPrimitives.ReadUInt32LittleEndian(
            header.Slice(8, 4)));
        var firmwareVersion = BinaryPrimitives.ReadUInt32LittleEndian(
            header.Slice(12, 4));

        if (BinaryPrimitives.ReadUInt32LittleEndian(header[..4]) != ProductId ||
            header[4] != 1 ||
            header[5] != 0 ||
            BinaryPrimitives.ReadUInt16LittleEndian(header.Slice(6, 2)) !=
                TargetDeviceId)
        {
            throw new ArgumentException(
                "firmware package target/header tuple is invalid",
                nameof(package));
        }

        if (imageLength < 8 ||
            imageLength > MaxImageBytes ||
            (imageLength & 3) != 0 ||
            firmwareVersion == 0 ||
            package.Length != HeaderBytes + HeaderAuthTagBytes + imageLength)
        {
            throw new ArgumentException(
                "firmware package length/version contract is invalid",
                nameof(package));
        }

        var image = package.Slice(HeaderBytes + HeaderAuthTagBytes, imageLength);
        Span<byte> digest = stackalloc byte[32];
        SHA256.HashData(image, digest);

        if (!CryptographicOperations.FixedTimeEquals(
                digest,
                header.Slice(16, 32)))
        {
            throw new ArgumentException(
                "firmware package payload SHA-256 does not match its header",
                nameof(package));
        }

        return new FirmwareUpdatePackage(
            header.ToArray(),
            package.Slice(HeaderBytes, HeaderAuthTagBytes).ToArray(),
            image.ToArray(),
            firmwareVersion);
    }

    internal async Task<FirmwareUpdateResponse> EnterBootloaderAsync(
        CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        return await ExecuteSingleRequestAsync(
            OpcodeEnterBootloader,
            DestructiveFlag,
            new byte[] { OpcodeEnterBootloader },
            cancellationToken);
    }

    public async Task<FirmwareUpdateResponse> InfoAsync(
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        return await ExecuteSingleRequestAsync(
            OpcodeInfo,
            0,
            new byte[] { OpcodeInfo },
            cancellationToken);
    }

    public async Task<FirmwareUpdateResponse> UpdateAsync(
        ReadOnlyMemory<byte> packageBytes,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        var package = ParsePackage(packageBytes.Span);

        await _channel.EnterAsync(cancellationToken);
        try
        {
            var beginPayload = new byte[1 + HeaderBytes];
            beginPayload[0] = OpcodeBegin;
            package.Header.CopyTo(beginPayload, 1);

            var begin = await SendRequestCoreAsync(
                OpcodeBegin,
                DestructiveFlag,
                beginPayload,
                cancellationToken);
            EnsureOk(begin, FirmwareUpdateState.HeaderStaged);

            var authorizePayload = new byte[1 + HeaderAuthTagBytes];
            authorizePayload[0] = OpcodeAuthorizeHeader;
            package.HeaderAuthTag.CopyTo(authorizePayload, 1);

            var authorize = await SendRequestCoreAsync(
                OpcodeAuthorizeHeader,
                DestructiveFlag,
                authorizePayload,
                cancellationToken);
            EnsureOk(authorize, FirmwareUpdateState.Authorized);

            var offset = 0;
            while (offset < package.Image.Length)
            {
                var pageRemaining =
                    ApplicationPageBytes - (offset & (ApplicationPageBytes - 1));
                var count = Math.Min(
                    MaxDataBytes,
                    Math.Min(package.Image.Length - offset, pageRemaining));

                if ((count & 1) != 0 || count < 2)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"invalid DATA chunk length {count} at offset {offset}");
                }

                var dataPayload = new byte[3 + count];
                dataPayload[0] = OpcodeData;
                BinaryPrimitives.WriteUInt16LittleEndian(
                    dataPayload.AsSpan(1, 2),
                    checked((ushort)offset));
                package.Image.AsSpan(offset, count).CopyTo(dataPayload.AsSpan(3));

                var data = await SendDataWithSingleTimeoutRetryAsync(
                    dataPayload,
                    cancellationToken);
                EnsureOk(data, FirmwareUpdateState.Receiving);

                var expectedOffset = checked((ushort)(offset + count));
                if (data.ExpectedOffset != expectedOffset)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"bootloader expected offset {data.ExpectedOffset}, host expected {expectedOffset}");
                }

                offset += count;
            }

            var end = await SendRequestCoreAsync(
                OpcodeEnd,
                DestructiveFlag,
                new byte[] { OpcodeEnd },
                cancellationToken);
            EnsureOk(end, FirmwareUpdateState.Committed);
            return end;
        }
        finally
        {
            _channel.Exit();
        }
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        if (_ownsChannel)
        {
            await _channel.DisposeAsync();
        }
    }

    private async Task<FirmwareUpdateResponse> ExecuteSingleRequestAsync(
        byte opcode,
        byte flags,
        ReadOnlyMemory<byte> payload,
        CancellationToken cancellationToken)
    {
        await _channel.EnterAsync(cancellationToken);
        try
        {
            return await SendRequestCoreAsync(
                opcode,
                flags,
                payload,
                cancellationToken);
        }
        finally
        {
            _channel.Exit();
        }
    }

    private async Task<FirmwareUpdateResponse> SendRequestCoreAsync(
        byte opcode,
        byte flags,
        ReadOnlyMemory<byte> payload,
        CancellationToken cancellationToken)
    {
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(2));

        ushort requestId = 0;

        try
        {
            requestId = _channel.NextRequestId();
            var wire = BinaryFrameCodec.Encode(
                FrameType.FirmwareUpdateRequest,
                flags,
                requestId,
                payload.Span);

            await _channel.WriteAsync(wire, timeout.Token);
            var responseFrame = await _channel.ReadMatchingFrameAsync(
                requestId,
                FrameType.FirmwareUpdateResponse,
                timeout.Token);

            return ParseResponse(responseFrame, opcode);
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.Timeout)
        {
            AbandonSingleResponse(requestId);
            throw;
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            AbandonSingleResponse(requestId);
            throw new DeusHostException(
                HostErrorKind.Timeout,
                $"firmware update opcode 0x{opcode:X2} timed out",
                exception);
        }
        catch (OperationCanceledException exception)
        {
            AbandonSingleResponse(requestId);
            throw new DeusHostException(
                HostErrorKind.Cancelled,
                $"firmware update opcode 0x{opcode:X2} cancelled",
                exception);
        }
    }

    private void AbandonSingleResponse(ushort requestId)
    {
        if (requestId != 0)
        {
            _channel.AbandonSingleResponse(requestId);
        }
    }

    private async Task<FirmwareUpdateResponse> SendDataWithSingleTimeoutRetryAsync(
        ReadOnlyMemory<byte> payload,
        CancellationToken cancellationToken)
    {
        try
        {
            return await SendRequestCoreAsync(
                OpcodeData,
                DestructiveFlag,
                payload,
                cancellationToken);
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.Timeout)
        {
            return await SendRequestCoreAsync(
                OpcodeData,
                DestructiveFlag,
                payload,
                cancellationToken);
        }
    }

    private static FirmwareUpdateResponse ParseResponse(
        BinaryFrame frame,
        byte expectedOpcode)
    {
        if (frame.Payload.Length != ResponseBytes)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"firmware update response length {frame.Payload.Length}, expected {ResponseBytes}");
        }

        var payload = frame.Payload.AsSpan();
        if (payload[0] != expectedOpcode ||
            payload[3] != 0 ||
            payload[6] != 0 ||
            payload[7] != 0)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "firmware update response echo/reserved fields are invalid");
        }

        return new FirmwareUpdateResponse(
            payload[0],
            (FirmwareUpdateStatus)payload[1],
            (FirmwareUpdateState)payload[2],
            BinaryPrimitives.ReadUInt16LittleEndian(payload.Slice(4, 2)),
            BinaryPrimitives.ReadUInt32LittleEndian(payload.Slice(8, 4)),
            BinaryPrimitives.ReadUInt32LittleEndian(payload.Slice(12, 4)));
    }

    private static void EnsureOk(
        FirmwareUpdateResponse response,
        FirmwareUpdateState expectedState)
    {
        if (response.Status != FirmwareUpdateStatus.Ok)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"firmware update failed status={response.Status} state={response.State}");
        }

        if (response.State != expectedState)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"firmware update state {response.State}, expected {expectedState}");
        }
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
