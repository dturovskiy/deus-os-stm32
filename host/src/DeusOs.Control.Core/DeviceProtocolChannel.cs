namespace DeusOs.Control.Core;

internal sealed class DeviceProtocolChannel : IAsyncDisposable
{
    private const int ReadBufferBytes = 512;

    private enum AbandonedResponseShape
    {
        SingleResponse,
        RpcUntilTerminal,
    }

    private readonly IDeviceTransport _transport;
    private readonly BinaryFrameDecoder _decoder = new();
    private readonly RequestIdAllocator _requestIds = new();
    private readonly Queue<BinaryFrame> _queuedFrames = new();
    private readonly Dictionary<ushort, AbandonedResponseShape> _abandonedRequests = new();
    private readonly SemaphoreSlim _operationGate = new(1, 1);
    private bool _requestIdEpochStarted;
    private bool _freshSessionRequired;
    private bool _disposed;

    internal DeviceProtocolChannel(IDeviceTransport transport)
    {
        _transport = transport ?? throw new ArgumentNullException(nameof(transport));
    }

    internal string Locator => _transport.Locator;

    internal ushort NextRequestId()
    {
        ThrowIfDisposed();

        if (_freshSessionRequired)
        {
            throw new DeusHostException(
                HostErrorKind.RequestCorrelation,
                "a fresh client/transport session is required before request ids may be reused");
        }

        for (var attempt = 0; attempt < ushort.MaxValue; ++attempt)
        {
            var requestId = _requestIds.Next();

            if (_requestIdEpochStarted &&
                requestId == 1 &&
                _abandonedRequests.Count != 0)
            {
                _freshSessionRequired = true;
                throw new DeusHostException(
                    HostErrorKind.RequestCorrelation,
                    "request id space wrapped while abandoned responses remain outstanding; a fresh client/transport session is required");
            }

            _requestIdEpochStarted = true;

            if (!_abandonedRequests.ContainsKey(requestId))
            {
                return requestId;
            }
        }

        _freshSessionRequired = true;
        throw new DeusHostException(
            HostErrorKind.RequestCorrelation,
            "no request id is available while abandoned responses remain outstanding; a fresh client/transport session is required");
    }

    internal void AbandonSingleResponse(ushort requestId)
    {
        RegisterAbandonedRequest(requestId, AbandonedResponseShape.SingleResponse);
    }

    internal void AbandonRpcStream(ushort requestId)
    {
        RegisterAbandonedRequest(requestId, AbandonedResponseShape.RpcUntilTerminal);
    }

    internal Task EnterAsync(CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        return _operationGate.WaitAsync(cancellationToken);
    }

    internal void Exit()
    {
        _operationGate.Release();
    }

    internal ValueTask WriteAsync(
        ReadOnlyMemory<byte> data,
        CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        return _transport.WriteAsync(data, cancellationToken);
    }

    internal async Task<BinaryFrame> ReadMatchingFrameAsync(
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

    internal async Task<BinaryFrame> ReadNextFrameAsync(
        CancellationToken cancellationToken)
    {
        var buffer = new byte[ReadBufferBytes];

        while (true)
        {
            while (_queuedFrames.Count != 0)
            {
                var queued = _queuedFrames.Dequeue();
                if (DiscardIfAbandoned(queued))
                {
                    continue;
                }

                return queued;
            }

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
        }
    }

    internal void ValidateCommonResponseFrame(BinaryFrame frame)
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

    internal string FormatProtocolError(BinaryFrame frame)
    {
        if (frame.Payload.Length != 2)
        {
            return "malformed PROTOCOL_ERROR response";
        }

        return $"protocol error code={frame.Payload[0]} observed=0x{frame.Payload[1]:X2}";
    }

    internal void ResetProtocolState()
    {
        _decoder.Reset();
        _queuedFrames.Clear();
        _requestIds.Reset();
        _requestIdEpochStarted = false;

        if (_abandonedRequests.Count != 0)
        {
            _freshSessionRequired = true;
        }
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        _operationGate.Dispose();
        await _transport.DisposeAsync();
    }

    private void RegisterAbandonedRequest(
        ushort requestId,
        AbandonedResponseShape responseShape)
    {
        ThrowIfDisposed();

        if (requestId == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(requestId));
        }

        if (_abandonedRequests.TryGetValue(requestId, out var existing))
        {
            if (existing != responseShape)
            {
                throw new InvalidOperationException(
                    $"request id 0x{requestId:X4} is already abandoned with response shape {existing}");
            }

            return;
        }

        _abandonedRequests.Add(requestId, responseShape);
    }

    private bool DiscardIfAbandoned(BinaryFrame frame)
    {
        if (!_abandonedRequests.TryGetValue(frame.RequestId, out var responseShape))
        {
            return false;
        }

        if (responseShape == AbandonedResponseShape.SingleResponse ||
            frame.Type == FrameType.RpcEnd ||
            frame.Type == FrameType.ProtocolError)
        {
            _abandonedRequests.Remove(frame.RequestId);
        }

        return true;
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
