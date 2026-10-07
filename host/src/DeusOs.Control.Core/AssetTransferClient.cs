namespace DeusOs.Control.Core;

internal sealed class AssetTransferClient
{
    private readonly DeviceProtocolChannel _channel;
    private readonly RequestIdAllocator _transferIds = new();

    internal AssetTransferClient(DeviceProtocolChannel channel)
    {
        _channel = channel ?? throw new ArgumentNullException(nameof(channel));
    }

    internal void ResetSessionState()
    {
        _transferIds.Reset();
    }

    public async Task<AssetStatusSnapshot> AssetStatusAsync(
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(2));

        await _channel.EnterAsync(linked.Token);
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
            _channel.Exit();
        }
    }

    public async Task<AssetReadResult?> ReadOledUiLayoutAsync(
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(5));

        await _channel.EnterAsync(linked.Token);
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
            _channel.Exit();
        }
    }

    public async Task<AssetCommitResult> WriteOledUiLayoutAsync(
        OledUiLayoutConfigV1 configuration,
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        var payload = configuration.Serialize();
        var payloadCrc = Crc32IsoHdlc.Compute(payload);

        using var linked = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        linked.CancelAfter(TimeSpan.FromSeconds(5));

        await _channel.EnterAsync(linked.Token);
        ushort transferId = 0;
        var sessionMayBeActive = false;

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

            transferId = _transferIds.Next();
            sessionMayBeActive = true;
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
            sessionMayBeActive = false;

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
        {
            await TryAbortWriteSessionAsync(
                transferId,
                sessionMayBeActive);

            if (!cancellationToken.IsCancellationRequested)
            {
                throw new DeusHostException(
                    HostErrorKind.Timeout,
                    "Asset write timed out",
                    exception);
            }

            throw;
        }
        catch (DeusHostException)
        {
            await TryAbortWriteSessionAsync(
                transferId,
                sessionMayBeActive);
            throw;
        }
        finally
        {
            _channel.Exit();
        }
    }

    private async Task TryAbortWriteSessionAsync(
        ushort transferId,
        bool sessionMayBeActive)
    {
        if (!sessionMayBeActive || transferId == 0)
        {
            return;
        }

        using var cleanup =
            new CancellationTokenSource(TimeSpan.FromSeconds(2));

        try
        {
            var abort = await AssetRequestCoreAsync(
                AssetTransferOpcode.Abort,
                0,
                AssetTransferProtocol.EncodeAbort(
                    AssetTransferProtocol.OledUiLayoutObjectType,
                    transferId),
                cleanup.Token);
            EnsureAssetSuccess(abort, "asset abort cleanup");
        }
        catch (DeusHostException)
        {
        }
        catch (OperationCanceledException)
        {
        }
    }

    private async Task<AssetTransferResponse> AssetRequestCoreAsync(
        AssetTransferOpcode opcode,
        byte flags,
        ReadOnlyMemory<byte> payload,
        CancellationToken cancellationToken)
    {
        ushort requestId = 0;

        try
        {
            requestId = _channel.NextRequestId();
            var wire = BinaryFrameCodec.Encode(
                FrameType.AssetTransferRequest,
                flags,
                requestId,
                payload.Span);

            await _channel.WriteAsync(wire, cancellationToken);

            var frame = await _channel.ReadMatchingFrameAsync(
                requestId,
                FrameType.AssetTransferResponse,
                cancellationToken);

            return AssetTransferProtocol.ParseResponse(frame, opcode);
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.Timeout)
        {
            AbandonSingleResponse(requestId);
            throw;
        }
        catch (OperationCanceledException)
        {
            AbandonSingleResponse(requestId);
            throw;
        }
    }

    private void AbandonSingleResponse(ushort requestId)
    {
        if (requestId != 0)
        {
            _channel.AbandonSingleResponse(requestId);
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

}
