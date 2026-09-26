using System.Buffers.Binary;
using System.Text;

namespace DeusOs.Control.Core;

public sealed class DeusDeviceClient : IAsyncDisposable
{
    private const int ReadBufferBytes = 512;
    private const int MaxAggregatedOutputBytes = 64 * 1024;

    private readonly IDeviceTransport _transport;
    private readonly BinaryFrameDecoder _decoder = new();
    private readonly RequestIdAllocator _requestIds = new();
    private readonly RequestIdAllocator _transferIds = new();
    private readonly Queue<BinaryFrame> _queuedFrames = new();
    private readonly SemaphoreSlim _rpcGate = new(1, 1);
    private bool _disposed;

    public DeusDeviceClient(IDeviceTransport transport)
    {
        _transport = transport ?? throw new ArgumentNullException(nameof(transport));
        State = ConnectionState.Discovered;
    }

    public ConnectionState State { get; private set; }

    public string Locator => _transport.Locator;

    public HelloInfo? Hello { get; private set; }

    public SystemInfo? SystemInfo { get; private set; }

    public async Task<NegotiationResult> NegotiateAsync(
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        State = ConnectionState.Negotiating;
        ResetSessionState();

        try
        {
            using var timeout = CancellationTokenSource.CreateLinkedTokenSource(
                cancellationToken);
            timeout.CancelAfter(TimeSpan.FromSeconds(2));

            var requestId = _requestIds.Next();
            var helloRequest = BinaryFrameCodec.Encode(
                FrameType.HelloRequest,
                0,
                requestId,
                ReadOnlySpan<byte>.Empty);

            await _transport.WriteAsync(helloRequest, timeout.Token);
            var helloFrame = await ReadMatchingFrameAsync(
                requestId,
                FrameType.HelloResponse,
                timeout.Token);

            var hello = HelloParser.Parse(helloFrame);
            ValidateHello(hello);

            var sysinfoResult = await RpcCoreAsync(
                ProtocolConstants.RpcSysInfo,
                Array.Empty<string>(),
                0,
                timeout.Token);

            EnsureRpcSuccess(sysinfoResult, "sysinfo");
            var systemInfo = SystemInfoParser.Parse(sysinfoResult.OutputText);
            ValidateSystemInfo(hello, systemInfo);

            Hello = hello;
            SystemInfo = systemInfo;
            State = ConnectionState.Ready;
            return new NegotiationResult(hello, systemInfo);
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            State = ConnectionState.Faulted;
            throw new DeusHostException(
                HostErrorKind.Timeout,
                "device negotiation timed out",
                exception);
        }
        catch (OperationCanceledException exception)
        {
            State = ConnectionState.Faulted;
            throw new DeusHostException(
                HostErrorKind.Cancelled,
                "device negotiation cancelled",
                exception);
        }
        catch
        {
            State = ConnectionState.Faulted;
            throw;
        }
    }

    public async Task<RpcResult> RpcAsync(
        ushort rpcId,
        IReadOnlyList<string>? arguments = null,
        byte flags = 0,
        TimeSpan? timeout = null,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();

        if (State != ConnectionState.Ready)
        {
            throw new InvalidOperationException(
                $"device is not ready (state={State})");
        }

        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(timeout ?? TimeSpan.FromSeconds(2));

        try
        {
            return await RpcCoreAsync(
                rpcId,
                arguments ?? Array.Empty<string>(),
                flags,
                linked.Token);
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            throw new DeusHostException(
                HostErrorKind.Timeout,
                $"RPC 0x{rpcId:X4} timed out",
                exception);
        }
        catch (OperationCanceledException exception)
        {
            throw new DeusHostException(
                HostErrorKind.Cancelled,
                $"RPC 0x{rpcId:X4} cancelled",
                exception);
        }
    }

    public Task<RpcResult> PingAsync(
        CancellationToken cancellationToken = default) =>
        RpcAsync(ProtocolConstants.RpcPing, cancellationToken: cancellationToken);

    public Task<RpcResult> HealthAsync(
        CancellationToken cancellationToken = default) =>
        RpcAsync(ProtocolConstants.RpcHealth, cancellationToken: cancellationToken);

    public Task<RpcResult> RpcInfoAsync(
        CancellationToken cancellationToken = default) =>
        RpcAsync(ProtocolConstants.RpcRpcInfo, cancellationToken: cancellationToken);

    public Task<RpcResult> SysInfoAsync(
        CancellationToken cancellationToken = default) =>
        RpcAsync(ProtocolConstants.RpcSysInfo, cancellationToken: cancellationToken);

    public async Task<ApplicationSnapshot> ApplicationsAsync(
        CancellationToken cancellationToken = default)
    {
        var result = await RpcAsync(
            ProtocolConstants.RpcAppList,
            cancellationToken: cancellationToken);
        EnsureRpcSuccess(result, "applist");
        return ApplicationListParser.Parse(result.OutputText);
    }

    public async Task<RpcResult> StartApplicationAsync(
        ushort applicationId,
        CancellationToken cancellationToken = default)
    {
        if (applicationId == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(applicationId));
        }

        var result = await RpcAsync(
            ProtocolConstants.RpcAppStart,
            new[] { $"0x{applicationId:X4}" },
            cancellationToken: cancellationToken);
        EnsureRpcSuccess(result, "appstart");
        return result;
    }

    public async Task<RpcResult> StopApplicationAsync(
        CancellationToken cancellationToken = default)
    {
        var result = await RpcAsync(
            ProtocolConstants.RpcAppStop,
            cancellationToken: cancellationToken);
        EnsureRpcSuccess(result, "appstop");
        return result;
    }


    public async Task<AssetStatusSnapshot> AssetStatusAsync(
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        EnsureAssetTransferAvailable(policy);

        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(2));

        await _rpcGate.WaitAsync(linked.Token);
        try
        {
            var response = await AssetRequestCoreAsync(
                AssetTransferOpcode.Status,
                0,
                AssetTransferProtocol.EncodeStatus(
                    AssetTransferProtocol.OledUiLayoutObjectType),
                linked.Token);

            EnsureAssetSuccess(response, "asset status");
            return AssetTransferProtocol.ToStatusSnapshot(response);
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            throw new DeusHostException(
                HostErrorKind.Timeout,
                "Asset STATUS timed out",
                exception);
        }
        finally
        {
            _rpcGate.Release();
        }
    }

    public async Task<AssetReadResult?> ReadOledUiLayoutAsync(
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        EnsureAssetTransferAvailable(policy);

        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(5));

        await _rpcGate.WaitAsync(linked.Token);
        try
        {
            var statusResponse = await AssetRequestCoreAsync(
                AssetTransferOpcode.Status,
                0,
                AssetTransferProtocol.EncodeStatus(
                    AssetTransferProtocol.OledUiLayoutObjectType),
                linked.Token);
            EnsureAssetSuccess(statusResponse, "asset status");

            var status = AssetTransferProtocol.ToStatusSnapshot(statusResponse);
            if (!status.HasCommittedRecord)
            {
                return null;
            }

            if (status.CommittedPayloadLength !=
                AssetTransferProtocol.OledUiLayoutBytes)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    $"committed OLED payload length {status.CommittedPayloadLength} is invalid");
            }

            var payload = await ReadAssetPayloadCoreAsync(
                AssetTransferProtocol.OledUiLayoutObjectType,
                status.CommittedGeneration,
                status.CommittedPayloadLength,
                linked.Token);

            var crc = Crc32IsoHdlc.Compute(payload);
            if (crc != status.CommittedPayloadCrc32)
            {
                throw new DeusHostException(
                    HostErrorKind.Crc,
                    $"Asset readback CRC 0x{crc:X8} != STATUS 0x{status.CommittedPayloadCrc32:X8}");
            }

            _ = OledUiLayoutConfigV1.Deserialize(payload);
            return new AssetReadResult(
                status.CommittedGeneration,
                payload,
                crc);
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            throw new DeusHostException(
                HostErrorKind.Timeout,
                "Asset readback timed out",
                exception);
        }
        finally
        {
            _rpcGate.Release();
        }
    }

    public async Task<AssetCommitResult> WriteOledUiLayoutAsync(
        OledUiLayoutConfigV1 configuration,
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        EnsureAssetTransferAvailable(policy);

        var payload = configuration.Serialize();
        var payloadCrc = Crc32IsoHdlc.Compute(payload);

        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(5));

        await _rpcGate.WaitAsync(linked.Token);
        try
        {
            var beforeResponse = await AssetRequestCoreAsync(
                AssetTransferOpcode.Status,
                0,
                AssetTransferProtocol.EncodeStatus(
                    AssetTransferProtocol.OledUiLayoutObjectType),
                linked.Token);
            EnsureAssetSuccess(beforeResponse, "asset pre-write status");
            var before = AssetTransferProtocol.ToStatusSnapshot(beforeResponse);

            var transferId = _transferIds.Next();
            var begin = await AssetRequestCoreAsync(
                AssetTransferOpcode.Begin,
                AssetTransferProtocol.AllowDestructive,
                AssetTransferProtocol.EncodeBegin(
                    AssetTransferProtocol.OledUiLayoutObjectType,
                    transferId,
                    checked((ushort)payload.Length),
                    OledUiLayoutConfigV1.SchemaVersion,
                    payloadCrc),
                linked.Token);
            EnsureAssetSuccess(begin, "asset begin");

            if (begin.TransferId != transferId ||
                begin.ObjectType != AssetTransferProtocol.OledUiLayoutObjectType ||
                begin.TotalLength != payload.Length ||
                begin.NextOffset != 0)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "Asset BEGIN response state mismatch");
            }

            ushort offset = 0;
            while (offset < payload.Length)
            {
                var count = Math.Min(
                    AssetTransferProtocol.ChunkMax,
                    payload.Length - offset);
                var chunk = payload.AsSpan(offset, count);

                var write = await AssetRequestCoreAsync(
                    AssetTransferOpcode.WriteChunk,
                    AssetTransferProtocol.AllowDestructive,
                    AssetTransferProtocol.EncodeWriteChunk(
                        AssetTransferProtocol.OledUiLayoutObjectType,
                        transferId,
                        offset,
                        chunk),
                    linked.Token);
                EnsureAssetSuccess(write, "asset write");

                offset = checked((ushort)(offset + count));
                if (write.TransferId != transferId ||
                    write.NextOffset != offset)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        "Asset WRITE_CHUNK next-offset mismatch");
                }
            }

            var commit = await AssetRequestCoreAsync(
                AssetTransferOpcode.Commit,
                AssetTransferProtocol.AllowDestructive,
                AssetTransferProtocol.EncodeCommit(
                    AssetTransferProtocol.OledUiLayoutObjectType,
                    transferId),
                linked.Token);
            EnsureAssetSuccess(commit, "asset commit");

            if (commit.TransferId != transferId ||
                commit.CommittedGeneration == 0 ||
                commit.TotalLength != payload.Length ||
                commit.NextOffset != payload.Length)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "Asset COMMIT response state mismatch");
            }

            var afterResponse = await AssetRequestCoreAsync(
                AssetTransferOpcode.Status,
                0,
                AssetTransferProtocol.EncodeStatus(
                    AssetTransferProtocol.OledUiLayoutObjectType),
                linked.Token);
            EnsureAssetSuccess(afterResponse, "asset post-write status");
            var after = AssetTransferProtocol.ToStatusSnapshot(afterResponse);

            if (after.CommittedGeneration != commit.CommittedGeneration ||
                after.CommittedPayloadLength != payload.Length ||
                after.CommittedPayloadCrc32 != payloadCrc)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "Asset post-COMMIT STATUS does not match committed identity");
            }

            var readback = await ReadAssetPayloadCoreAsync(
                AssetTransferProtocol.OledUiLayoutObjectType,
                after.CommittedGeneration,
                after.CommittedPayloadLength,
                linked.Token);

            if (!payload.AsSpan().SequenceEqual(readback) ||
                Crc32IsoHdlc.Compute(readback) != payloadCrc)
            {
                throw new DeusHostException(
                    HostErrorKind.Crc,
                    "Asset post-COMMIT readback does not match requested payload");
            }

            return new AssetCommitResult(
                after.CommittedGeneration,
                after.CommittedPayloadLength,
                after.CommittedPayloadCrc32,
                before.CommittedGeneration != after.CommittedGeneration);
        }
        catch (OperationCanceledException exception)
            when (!cancellationToken.IsCancellationRequested)
        {
            throw new DeusHostException(
                HostErrorKind.Timeout,
                "Asset write timed out",
                exception);
        }
        finally
        {
            _rpcGate.Release();
        }
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        State = ConnectionState.Disconnected;
        _rpcGate.Dispose();
        await _transport.DisposeAsync();
    }


    private void EnsureAssetTransferAvailable(AssetAccessPolicy policy)
    {
        if (State != ConnectionState.Ready ||
            Hello is null ||
            SystemInfo is null)
        {
            throw new InvalidOperationException(
                $"device is not ready for Asset transfer (state={State})");
        }

        if ((Hello.CapabilityFlags &
             ProtocolConstants.HelloCapabilityAssetConfigurationTransfer) == 0)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                "management HELLO does not advertise Asset/Configuration transfer v1");
        }

        if (policy == AssetAccessPolicy.PublishedOnly &&
            (SystemInfo.Capabilities &
             SystemCapability.AssetConfigurationTransfer) == 0)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                "Asset/Configuration is not yet a published system capability");
        }
    }

    private async Task<AssetTransferResponse> AssetRequestCoreAsync(
        AssetTransferOpcode opcode,
        byte flags,
        ReadOnlyMemory<byte> payload,
        CancellationToken cancellationToken)
    {
        try
        {
            var requestId = _requestIds.Next();
            var wire = BinaryFrameCodec.Encode(
                FrameType.AssetTransferRequest,
                flags,
                requestId,
                payload.Span);

            await _transport.WriteAsync(wire, cancellationToken);

            var frame = await ReadMatchingFrameAsync(
                requestId,
                FrameType.AssetTransferResponse,
                cancellationToken);

            return AssetTransferProtocol.ParseResponse(frame, opcode);
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.TransportDisconnected)
        {
            State = ConnectionState.Recovering;
            ResetSessionState();
            throw;
        }
    }

    private async Task<byte[]> ReadAssetPayloadCoreAsync(
        ushort objectType,
        uint generation,
        ushort totalLength,
        CancellationToken cancellationToken)
    {
        if (generation == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(generation));
        }

        if (totalLength == 0 ||
            totalLength > AssetTransferProtocol.ObjectMax)
        {
            throw new ArgumentOutOfRangeException(nameof(totalLength));
        }

        var payload = new byte[totalLength];
        ushort offset = 0;

        while (offset < totalLength)
        {
            var count = checked((byte)Math.Min(
                AssetTransferProtocol.ChunkMax,
                totalLength - offset));

            var response = await AssetRequestCoreAsync(
                AssetTransferOpcode.ReadChunk,
                0,
                AssetTransferProtocol.EncodeReadChunk(
                    objectType,
                    generation,
                    offset,
                    count),
                cancellationToken);

            EnsureAssetSuccess(response, "asset read");

            if (response.ObjectType != objectType ||
                response.CommittedGeneration != generation ||
                response.TotalLength != totalLength ||
                response.Data.Length != count ||
                response.NextOffset != offset + count)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "Asset READ_CHUNK response state mismatch");
            }

            response.Data.CopyTo(payload, offset);
            offset = checked((ushort)(offset + count));
        }

        return payload;
    }

    private static void EnsureAssetSuccess(
        AssetTransferResponse response,
        string operation)
    {
        if (response.Status != AssetTransferStatus.Ok)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"{operation} failed with Asset status {response.Status}");
        }
    }

    private async Task<RpcResult> RpcCoreAsync(
        ushort rpcId,
        IReadOnlyList<string> arguments,
        byte flags,
        CancellationToken cancellationToken)
    {
        await _rpcGate.WaitAsync(cancellationToken);
        try
        {
            var requestId = _requestIds.Next();
            var payload = RpcRequestEncoder.EncodePayload(rpcId, arguments);
            var wire = BinaryFrameCodec.Encode(
                FrameType.RpcRequest,
                flags,
                requestId,
                payload);

            await _transport.WriteAsync(wire, cancellationToken);

            var output = new MemoryStream();
            ushort expectedSequence = 0;
            ushort receivedChunks = 0;

            while (true)
            {
                var frame = await ReadNextFrameAsync(cancellationToken);

                ValidateCommonResponseFrame(frame);

                if (frame.RequestId != requestId)
                {
                    throw new DeusHostException(
                        HostErrorKind.RequestCorrelation,
                        $"received request id 0x{frame.RequestId:X4} while waiting for 0x{requestId:X4}");
                }

                if (frame.Type == FrameType.ProtocolError)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        FormatProtocolError(frame));
                }

                if (frame.Type == FrameType.RpcData)
                {
                    if (frame.Payload.Length < 4)
                    {
                        throw new DeusHostException(
                            HostErrorKind.Protocol,
                            "RPC_DATA payload shorter than four bytes");
                    }

                    var dataRpcId = BinaryPrimitives.ReadUInt16LittleEndian(
                        frame.Payload.AsSpan(0, 2));
                    var sequence = BinaryPrimitives.ReadUInt16LittleEndian(
                        frame.Payload.AsSpan(2, 2));

                    if (dataRpcId != rpcId)
                    {
                        throw new DeusHostException(
                            HostErrorKind.RequestCorrelation,
                            $"RPC_DATA rpc id 0x{dataRpcId:X4} does not match 0x{rpcId:X4}");
                    }

                    if (sequence != expectedSequence)
                    {
                        throw new DeusHostException(
                            HostErrorKind.RequestCorrelation,
                            $"RPC_DATA sequence {sequence} does not match expected {expectedSequence}");
                    }

                    var data = frame.Payload.AsSpan(4);
                    if (data.Length > ProtocolConstants.RpcDataChunkMax)
                    {
                        throw new DeusHostException(
                            HostErrorKind.Protocol,
                            $"RPC_DATA chunk {data.Length} exceeds {ProtocolConstants.RpcDataChunkMax}");
                    }

                    if ((output.Length + data.Length) > MaxAggregatedOutputBytes)
                    {
                        throw new DeusHostException(
                            HostErrorKind.Protocol,
                            "RPC output exceeds host safety bound");
                    }

                    output.Write(data);
                    ++expectedSequence;
                    ++receivedChunks;
                    continue;
                }

                if (frame.Type != FrameType.RpcEnd)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"unexpected frame type {frame.Type} during RPC");
                }

                if (frame.Payload.Length != 10)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"RPC_END payload length {frame.Payload.Length} is not 10");
                }

                var endRpcId = BinaryPrimitives.ReadUInt16LittleEndian(
                    frame.Payload.AsSpan(0, 2));
                var chunkCount = BinaryPrimitives.ReadUInt16LittleEndian(
                    frame.Payload.AsSpan(2, 2));
                var totalBytes = BinaryPrimitives.ReadUInt32LittleEndian(
                    frame.Payload.AsSpan(4, 4));
                var statusDomain = frame.Payload[8];
                var statusCode = frame.Payload[9];

                if (endRpcId != rpcId)
                {
                    throw new DeusHostException(
                        HostErrorKind.RequestCorrelation,
                        $"RPC_END rpc id 0x{endRpcId:X4} does not match 0x{rpcId:X4}");
                }

                if (chunkCount != receivedChunks)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"RPC_END chunk count {chunkCount} does not match received {receivedChunks}");
                }

                if (totalBytes != output.Length)
                {
                    throw new DeusHostException(
                        HostErrorKind.Protocol,
                        $"RPC_END output length {totalBytes} does not match received {output.Length}");
                }

                return new RpcResult(
                    rpcId,
                    requestId,
                    chunkCount,
                    totalBytes,
                    statusDomain,
                    statusCode,
                    output.ToArray());
            }
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.TransportDisconnected)
        {
            State = ConnectionState.Recovering;
            ResetSessionState();
            throw;
        }
        finally
        {
            _rpcGate.Release();
        }
    }

    private async Task<BinaryFrame> ReadMatchingFrameAsync(
        ushort requestId,
        FrameType expectedType,
        CancellationToken cancellationToken)
    {
        while (true)
        {
            var frame = await ReadNextFrameAsync(cancellationToken);
            ValidateCommonResponseFrame(frame);

            if (frame.RequestId != requestId)
            {
                throw new DeusHostException(
                    HostErrorKind.RequestCorrelation,
                    $"received request id 0x{frame.RequestId:X4} while waiting for 0x{requestId:X4}");
            }

            if (frame.Type == FrameType.ProtocolError)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    FormatProtocolError(frame));
            }

            if (frame.Type != expectedType)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    $"expected {expectedType}, got {frame.Type}");
            }

            return frame;
        }
    }

    private async Task<BinaryFrame> ReadNextFrameAsync(
        CancellationToken cancellationToken)
    {
        if (_queuedFrames.Count != 0)
        {
            return _queuedFrames.Dequeue();
        }

        var buffer = new byte[ReadBufferBytes];

        while (true)
        {
            int count;
            try
            {
                count = await _transport.ReadAsync(buffer, cancellationToken);
            }
            catch (DeusHostException)
            {
                throw;
            }
            catch (OperationCanceledException)
            {
                throw;
            }
            catch (Exception exception)
            {
                throw new DeusHostException(
                    HostErrorKind.TransportDisconnected,
                    "transport read failed",
                    exception);
            }

            if (count <= 0)
            {
                throw new DeusHostException(
                    HostErrorKind.TransportDisconnected,
                    "transport closed");
            }

            var batch = _decoder.Feed(buffer.AsSpan(0, count));
            foreach (var error in batch.Errors)
            {
                if (error.Kind == FrameDecodeErrorKind.Crc)
                {
                    throw new DeusHostException(
                        HostErrorKind.Crc,
                        error.Message);
                }

                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    error.Message);
            }

            foreach (var frame in batch.Frames)
            {
                _queuedFrames.Enqueue(frame);
            }

            if (_queuedFrames.Count != 0)
            {
                return _queuedFrames.Dequeue();
            }
        }
    }

    private static void ValidateCommonResponseFrame(BinaryFrame frame)
    {
        if (frame.Version != ProtocolConstants.ProtocolVersion)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                $"frame protocol version {frame.Version} is unsupported");
        }

        if (frame.Flags != 0 || frame.Reserved != 0)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "response flags/reserved byte must be zero");
        }
    }

    private static string FormatProtocolError(BinaryFrame frame)
    {
        if (frame.Payload.Length != 2)
        {
            return "malformed PROTOCOL_ERROR response";
        }

        return $"protocol error code={frame.Payload[0]} observed=0x{frame.Payload[1]:X2}";
    }

    private static void ValidateHello(HelloInfo hello)
    {
        if (hello.ProtocolVersion != ProtocolConstants.ProtocolVersion)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                $"protocol {hello.ProtocolVersion} != {ProtocolConstants.ProtocolVersion}");
        }

        if (hello.ServiceVersion != ProtocolConstants.ExpectedServiceVersion ||
            hello.RegistryCount != ProtocolConstants.ExpectedRegistryCount)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleService,
                $"service/registry {hello.ServiceVersion}/{hello.RegistryCount} != " +
                $"{ProtocolConstants.ExpectedServiceVersion}/{ProtocolConstants.ExpectedRegistryCount}");
        }

        if (hello.MaxArgs != ProtocolConstants.MaxArgs ||
            hello.RequestPayloadMax != ProtocolConstants.MaxPayloadBytes ||
            hello.DataChunkMax != ProtocolConstants.RpcDataChunkMax)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                "HELLO protocol bounds do not match the host v1 contract");
        }

        const uint requiredBaseCapabilities = 0x0000003Fu;
        const uint knownCapabilities =
            requiredBaseCapabilities |
            ProtocolConstants.HelloCapabilityAssetConfigurationTransfer;

        if ((hello.CapabilityFlags & requiredBaseCapabilities) !=
                requiredBaseCapabilities ||
            (hello.CapabilityFlags & ~knownCapabilities) != 0)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                $"unexpected HELLO capability mask 0x{hello.CapabilityFlags:X8}");
        }
    }

    private static void ValidateSystemInfo(
        HelloInfo hello,
        SystemInfo info)
    {
        if (info.AbiVersion != 1 ||
            info.OsId != "DEUS_OS" ||
            info.PlatformId != "STM32F103C8" ||
            info.ArchId != "ARMV7M" ||
            info.ProtocolVersion != hello.ProtocolVersion ||
            info.ServiceVersion != hello.ServiceVersion ||
            info.ApplicationRuntimeAbi != 1 ||
            info.UnitIdKind != 0)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                "sysinfo identity/ABI values do not match the v1 host contract");
        }

        const SystemCapability expected =
            SystemCapability.SystemHealth |
            SystemCapability.ApplicationRuntime |
            SystemCapability.ApplicationControl |
            SystemCapability.Diagnostics |
            SystemCapability.LocalUi;

        var forbidden =
            SystemCapability.FirmwareUpdate |
            SystemCapability.NetworkServices;

        if ((info.Capabilities & expected) != expected ||
            (info.Capabilities & forbidden) != 0)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                $"unexpected capability mask 0x{(uint)info.Capabilities:X8}");
        }

        if ((info.Capabilities &
             SystemCapability.AssetConfigurationTransfer) != 0 &&
            (hello.CapabilityFlags &
             ProtocolConstants.HelloCapabilityAssetConfigurationTransfer) == 0)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                "system Asset capability is set without management HELLO Asset protocol capability");
        }
    }

    private void ResetSessionState()
    {
        _decoder.Reset();
        _queuedFrames.Clear();
        _requestIds.Reset();
        _transferIds.Reset();
        Hello = null;
        SystemInfo = null;
    }

    private static void EnsureRpcSuccess(
        RpcResult result,
        string operation)
    {
        if (!result.IsSuccess)
        {
            throw new DeusHostException(
                HostErrorKind.RpcStatus,
                $"{operation} failed with status {result.StatusDomain}/{result.StatusCode}");
        }
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
