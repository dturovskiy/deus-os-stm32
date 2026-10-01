using System.Buffers.Binary;

namespace DeusOs.Control.Core;

internal sealed class DeusRpcClient
{
    private const int MaxAggregatedOutputBytes = 64 * 1024;

    private readonly DeviceProtocolChannel _channel;

    internal DeusRpcClient(DeviceProtocolChannel channel)
    {
        _channel = channel ?? throw new ArgumentNullException(nameof(channel));
    }

    internal async Task<RpcResult> RpcAsync(
        ushort rpcId,
        IReadOnlyList<string>? arguments = null,
        byte flags = 0,
        TimeSpan? timeout = null,
        CancellationToken cancellationToken = default)
    {
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

    internal async Task<RpcResult> RpcCoreAsync(
        ushort rpcId,
        IReadOnlyList<string> arguments,
        byte flags,
        CancellationToken cancellationToken)
    {
        await _channel.EnterAsync(cancellationToken);
        try
        {
            var requestId = _channel.NextRequestId();
            var payload = RpcRequestEncoder.EncodePayload(rpcId, arguments);
            var wire = BinaryFrameCodec.Encode(
                FrameType.RpcRequest,
                flags,
                requestId,
                payload);

            await _channel.WriteAsync(wire, cancellationToken);

            var output = new MemoryStream();
            ushort expectedSequence = 0;
            ushort receivedChunks = 0;

            while (true)
            {
                var frame = await _channel.ReadNextFrameAsync(cancellationToken);

                _channel.ValidateCommonResponseFrame(frame);

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
                        _channel.FormatProtocolError(frame));
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
        finally
        {
            _channel.Exit();
        }
    }

}
