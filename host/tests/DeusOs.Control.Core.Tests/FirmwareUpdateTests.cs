using System.Buffers.Binary;
using System.Security.Cryptography;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class FirmwareUpdateTests
{
    [Fact]
    public void PackageParserAcceptsFrozenHeaderAndPayloadDigest()
    {
        var package = CreatePackage(8, 7);
        var parsed = FirmwareUpdateClient.ParsePackage(package);

        Assert.Equal((uint)7, parsed.FirmwareVersion);
        Assert.Equal(48, parsed.Header.Length);
        Assert.Equal(32, parsed.HeaderAuthTag.Length);
        Assert.Equal(8, parsed.Image.Length);
        Assert.Equal(FirmwareUpdateClient.ProductId,
            BinaryPrimitives.ReadUInt32LittleEndian(parsed.Header.AsSpan(0, 4)));
        Assert.Equal(FirmwareUpdateClient.TargetDeviceId,
            BinaryPrimitives.ReadUInt16LittleEndian(parsed.Header.AsSpan(6, 2)));
    }

    [Fact]
    public void PackageParserRejectsPayloadDigestMismatch()
    {
        var package = CreatePackage(8, 1);
        package[^1] ^= 0x80;

        Assert.Throws<ArgumentException>(
            () => FirmwareUpdateClient.ParsePackage(package));
    }

    [Fact]
    public async Task BootloaderUpdateUsesFrozenOpcodeAndChunkSequence()
    {
        var transport = new ScriptedTransport();

        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            3, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            4, 0x06, FirmwareUpdateState.Committed, 8, 0, 7));

        await using var client = new FirmwareUpdateClient(transport);
        var result = await client.UpdateAsync(
            CreatePackage(8, 7),
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateState.Committed, result.State);
        Assert.Equal((uint)7, result.CommittedVersion);
        Assert.Equal(4, transport.Writes.Count);

        AssertRequest(transport.Writes[0], 1, 0x03, 49);
        AssertRequest(transport.Writes[1], 2, 0x04, 33);
        AssertRequest(transport.Writes[2], 3, 0x05, 11);
        AssertRequest(transport.Writes[3], 4, 0x06, 1);

        Assert.Equal((ushort)0,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[2].AsSpan(11, 2)));
    }

    [Fact]
    public async Task BootloaderUpdateRetriesDataExactlyOnceAfterTimeout()
    {
        var transport = new ScriptedTransport();
        transport.TimeoutOnWriteNumbers.Add(3);
        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            4, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            5, 0x06, FirmwareUpdateState.Committed, 8, 0, 7));

        await using var client = new FirmwareUpdateClient(transport);
        var result = await client.UpdateAsync(
            CreatePackage(8, 7),
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateState.Committed, result.State);
        Assert.Equal(5, transport.Writes.Count);
        AssertRequest(transport.Writes[2], 3, 0x05, 11);
        AssertRequest(transport.Writes[3], 4, 0x05, 11);
        Assert.Equal(
            transport.Writes[2].AsSpan(10, 11).ToArray(),
            transport.Writes[3].AsSpan(10, 11).ToArray());
        AssertRequest(transport.Writes[4], 5, 0x06, 1);
    }

    [Fact]
    public async Task BootloaderUpdateDoesNotRetryDataMoreThanOnce()
    {
        var transport = new ScriptedTransport();
        transport.TimeoutOnWriteNumbers.Add(3);
        transport.TimeoutOnWriteNumbers.Add(4);
        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));

        await using var client = new FirmwareUpdateClient(transport);
        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.UpdateAsync(
                CreatePackage(8, 7),
                TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Timeout, exception.Kind);
        Assert.Equal(4, transport.Writes.Count);
        AssertRequest(transport.Writes[2], 3, 0x05, 11);
        AssertRequest(transport.Writes[3], 4, 0x05, 11);
        Assert.Equal(
            transport.Writes[2].AsSpan(10, 11).ToArray(),
            transport.Writes[3].AsSpan(10, 11).ToArray());
    }

    [Fact]
    public async Task BootloaderUpdateIgnoresDelayedTimedOutDataResponseBeforeRetryResponse()
    {
        var transport = new ScriptedTransport();
        transport.TimeoutOnWriteNumbers.Add(3);
        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            3, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            4, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            5, 0x06, FirmwareUpdateState.Committed, 8, 0, 7));

        await using var client = new FirmwareUpdateClient(transport);
        var result = await client.UpdateAsync(
            CreatePackage(8, 7),
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateState.Committed, result.State);
        Assert.Equal((uint)7, result.CommittedVersion);
        Assert.Equal(5, transport.Writes.Count);
        AssertRequest(transport.Writes[2], 3, 0x05, 11);
        AssertRequest(transport.Writes[3], 4, 0x05, 11);
        AssertRequest(transport.Writes[4], 5, 0x06, 1);
    }

    [Fact]
    public async Task BootloaderUpdateMarksNativeTransportTimeoutResponseAsStale()
    {
        var transport = new ScriptedTransport();
        transport.HostTimeoutOnWriteNumbers.Add(3);
        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            3, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            4, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            5, 0x06, FirmwareUpdateState.Committed, 8, 0, 7));

        await using var client = new FirmwareUpdateClient(transport);
        var result = await client.UpdateAsync(
            CreatePackage(8, 7),
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateState.Committed, result.State);
        Assert.Equal((uint)7, result.CommittedVersion);
        Assert.Equal(5, transport.Writes.Count);
    }

    [Fact]
    public async Task BootloaderUpdateDiscardsDelayedTimedOutDataResponseAfterRetrySuccess()
    {
        var transport = new ScriptedTransport();
        transport.TimeoutOnWriteNumbers.Add(3);
        transport.EnqueueResponse(CreateResponse(
            1, 0x03, FirmwareUpdateState.HeaderStaged, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            2, 0x04, FirmwareUpdateState.Authorized, 0, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            4, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            3, 0x05, FirmwareUpdateState.Receiving, 8, 0, 0));
        transport.EnqueueResponse(CreateResponse(
            5, 0x06, FirmwareUpdateState.Committed, 8, 0, 7));

        await using var client = new FirmwareUpdateClient(transport);
        var result = await client.UpdateAsync(
            CreatePackage(8, 7),
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateState.Committed, result.State);
        Assert.Equal((uint)7, result.CommittedVersion);
        Assert.Equal(5, transport.Writes.Count);
        AssertRequest(transport.Writes[2], 3, 0x05, 11);
        AssertRequest(transport.Writes[3], 4, 0x05, 11);
        AssertRequest(transport.Writes[4], 5, 0x06, 1);
    }

    [Fact]
    public async Task BootloaderInfoStillRejectsUnknownRequestIdMismatch()
    {
        var transport = new ScriptedTransport();
        transport.EnqueueResponse(CreateResponse(
            2, 0x02, FirmwareUpdateState.RecoveryIdle, 0, 3, 0));

        await using var client = new FirmwareUpdateClient(transport);
        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.InfoAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.RequestCorrelation, exception.Kind);
        Assert.Single(transport.Writes);
        Assert.Equal((byte)FrameType.FirmwareUpdateRequest, transport.Writes[0][3]);
        Assert.Equal((byte)0, transport.Writes[0][4]);
        Assert.Equal(
            (ushort)1,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[0].AsSpan(6, 2)));
        Assert.Equal((byte)0x02, transport.Writes[0][10]);
    }

    [Fact]
    public async Task BootloaderInfoUsesNonDestructiveFlag()
    {
        var transport = new ScriptedTransport();
        transport.EnqueueResponse(CreateResponse(
            1, 0x02, FirmwareUpdateState.RecoveryIdle, 0, 3, 0));

        await using var client = new FirmwareUpdateClient(transport);
        var response = await client.InfoAsync(
            TestContext.Current.CancellationToken);

        Assert.Equal((uint)3, response.VersionFloor);
        Assert.Single(transport.Writes);
        Assert.Equal((byte)FrameType.FirmwareUpdateRequest, transport.Writes[0][3]);
        Assert.Equal((byte)0, transport.Writes[0][4]);
        Assert.Equal((byte)0x02, transport.Writes[0][10]);
    }

    [Fact]
    public void FrameTypesMatchFrozenFirmwareUpdateAbi()
    {
        Assert.Equal(0x04, (byte)FrameType.FirmwareUpdateRequest);
        Assert.Equal(0x86, (byte)FrameType.FirmwareUpdateResponse);
        Assert.Equal((ushort)0x1209, FirmwareUpdateClient.BootloaderVendorId);
        Assert.Equal((ushort)0x000D, FirmwareUpdateClient.BootloaderProductId);
        Assert.Equal((uint)0x08002000, FirmwareUpdateClient.ApplicationOrigin);
    }

    private static byte[] CreatePackage(int imageLength, uint version)
    {
        var image = new byte[imageLength];
        for (var index = 0; index < image.Length; ++index)
        {
            image[index] = checked((byte)(0x20 + index));
        }

        var header = new byte[48];
        BinaryPrimitives.WriteUInt32LittleEndian(
            header.AsSpan(0, 4),
            FirmwareUpdateClient.ProductId);
        header[4] = 1;
        header[5] = 0;
        BinaryPrimitives.WriteUInt16LittleEndian(
            header.AsSpan(6, 2),
            FirmwareUpdateClient.TargetDeviceId);
        BinaryPrimitives.WriteUInt32LittleEndian(
            header.AsSpan(8, 4),
            checked((uint)image.Length));
        BinaryPrimitives.WriteUInt32LittleEndian(
            header.AsSpan(12, 4),
            version);
        SHA256.HashData(image).CopyTo(header, 16);

        var package = new byte[48 + 32 + image.Length];
        header.CopyTo(package, 0);
        image.CopyTo(package, 80);
        return package;
    }

    private static byte[] CreateResponse(
        ushort requestId,
        byte opcode,
        FirmwareUpdateState state,
        ushort expectedOffset,
        uint versionFloor,
        uint committedVersion)
    {
        var payload = new byte[16];
        payload[0] = opcode;
        payload[1] = (byte)FirmwareUpdateStatus.Ok;
        payload[2] = (byte)state;
        BinaryPrimitives.WriteUInt16LittleEndian(
            payload.AsSpan(4, 2),
            expectedOffset);
        BinaryPrimitives.WriteUInt32LittleEndian(
            payload.AsSpan(8, 4),
            versionFloor);
        BinaryPrimitives.WriteUInt32LittleEndian(
            payload.AsSpan(12, 4),
            committedVersion);

        return BinaryFrameCodec.Encode(
            FrameType.FirmwareUpdateResponse,
            0,
            requestId,
            payload);
    }

    private static void AssertRequest(
        byte[] wire,
        ushort requestId,
        byte opcode,
        ushort payloadLength)
    {
        Assert.Equal((byte)FrameType.FirmwareUpdateRequest, wire[3]);
        Assert.Equal((byte)1, wire[4]);
        Assert.Equal(requestId,
            BinaryPrimitives.ReadUInt16LittleEndian(wire.AsSpan(6, 2)));
        Assert.Equal(payloadLength,
            BinaryPrimitives.ReadUInt16LittleEndian(wire.AsSpan(8, 2)));
        Assert.Equal(opcode, wire[10]);
    }

    private sealed class ScriptedTransport : IDeviceTransport
    {
        private readonly Queue<byte[]> _responses = new();
        private bool _timeoutNextRead;
        private bool _hostTimeoutNextRead;

        public string Locator => "mock:bootloader";

        public List<byte[]> Writes { get; } = new();
        public HashSet<int> TimeoutOnWriteNumbers { get; } = new();
        public HashSet<int> HostTimeoutOnWriteNumbers { get; } = new();

        public void EnqueueResponse(byte[] wire)
        {
            _responses.Enqueue(wire.ToArray());
        }

        public ValueTask WriteAsync(
            ReadOnlyMemory<byte> data,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            Writes.Add(data.ToArray());
            _timeoutNextRead = TimeoutOnWriteNumbers.Contains(Writes.Count);
            _hostTimeoutNextRead =
                HostTimeoutOnWriteNumbers.Contains(Writes.Count);
            return ValueTask.CompletedTask;
        }

        public ValueTask<int> ReadAsync(
            Memory<byte> buffer,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (_timeoutNextRead)
            {
                _timeoutNextRead = false;
                throw new OperationCanceledException(cancellationToken);
            }

            if (_hostTimeoutNextRead)
            {
                _hostTimeoutNextRead = false;
                throw new DeusHostException(
                    HostErrorKind.Timeout,
                    "scripted native transport timeout");
            }

            if (_responses.Count == 0)
            {
                return ValueTask.FromResult(0);
            }

            var response = _responses.Dequeue();
            if (response.Length > buffer.Length)
            {
                throw new InvalidOperationException(
                    $"scripted response length {response.Length} exceeds read buffer {buffer.Length}");
            }

            response.AsMemory().CopyTo(buffer);
            return ValueTask.FromResult(response.Length);
        }

        public ValueTask DisposeAsync() => ValueTask.CompletedTask;
    }
}
