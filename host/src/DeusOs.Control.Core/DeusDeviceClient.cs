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

        var future =
            SystemCapability.AssetConfigurationTransfer |
            SystemCapability.FirmwareUpdate |
            SystemCapability.NetworkServices;

        if ((info.Capabilities & expected) != expected ||
            (info.Capabilities & future) != 0)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                $"unexpected capability mask 0x{(uint)info.Capabilities:X8}");
        }
    }

    private void ResetSessionState()
    {
        _decoder.Reset();
        _queuedFrames.Clear();
        _requestIds.Reset();
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
