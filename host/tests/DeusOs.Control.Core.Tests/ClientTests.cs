using System.Buffers.Binary;
using System.Text;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class ClientTests
{
    [Fact]
    public async Task NegotiationSurvivesArbitrarySmallTransportReads()
    {
        var sourceTree = "0123456789abcdef0123456789abcdef01234567";
        var transport = new ScriptedTransport(readChunkLimit: 7);

        transport.EnqueueResponse(CreateHelloResponse(1));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
                $"SOURCE_TREE={sourceTree}\r\n" +
                "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
                "CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000\r\n"));

        await using var client = new DeusDeviceClient(transport);
        var result = await client.NegotiateAsync(TestContext.Current.CancellationToken);

        Assert.Equal(ConnectionState.Ready, client.State);
        Assert.Equal(sourceTree, result.SystemInfo.SourceTree);
        Assert.Equal((byte)3, result.Hello.ServiceVersion);
        Assert.Equal((ushort)36, result.Hello.RegistryCount);
        Assert.Equal(2, transport.Writes.Count);
    }

    [Fact]
    public async Task RpcRejectsWrongRequestId()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        transport.EnqueueResponse(CreateHelloResponse(1));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo()));

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcPing,
                0x9999,
                "PONG\r\n"));

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.RequestCorrelation, exception.Kind);
    }

    [Fact]
    public async Task RpcMapsProtocolError()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.EnqueueResponse(
            BinaryFrameCodec.Encode(
                FrameType.ProtocolError,
                0,
                3,
                new byte[] { 0x03, 0xA5 }));

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Protocol, exception.Kind);
        Assert.Contains("protocol error code=3", exception.Message);
    }

    [Fact]
    public async Task ActiveRpcTimeoutMapsToTimeout()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.RpcAsync(
                ProtocolConstants.RpcPing,
                timeout: TimeSpan.FromMilliseconds(150),
                cancellationToken: TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Timeout, exception.Kind);
        Assert.Equal(ConnectionState.Ready, client.State);
    }

    [Fact]
    public async Task ActiveRpcCancellationMapsToCancelled()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;
        using var cancellation = CancellationTokenSource.CreateLinkedTokenSource(
            TestContext.Current.CancellationToken);
        cancellation.CancelAfter(TimeSpan.FromMilliseconds(150));

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(cancellation.Token));

        Assert.Equal(HostErrorKind.Cancelled, exception.Kind);
        Assert.Equal(ConnectionState.Ready, client.State);
    }

    [Fact]
    public async Task DisconnectClearsPartialDecoderAndSessionState()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        var response = CreateRpcResponse(
            ProtocolConstants.RpcPing,
            3,
            "PONG\r\n");
        transport.EnqueueResponse(response.AsSpan(0, 7).ToArray());

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.TransportDisconnected, exception.Kind);
        Assert.Equal(ConnectionState.Recovering, client.State);
        Assert.Null(client.Hello);
        Assert.Null(client.SystemInfo);

        EnqueueNegotiation(transport, DefaultSourceTree);
        var result = await client.NegotiateAsync(TestContext.Current.CancellationToken);

        Assert.Equal(ConnectionState.Ready, client.State);
        Assert.Equal(DefaultSourceTree, result.SystemInfo.SourceTree);
    }

    [Fact]
    public async Task ReconnectRequiresFreshNegotiationAndIdentity()
    {
        const string replacementSourceTree =
            "abcdef0123456789abcdef0123456789abcdef01";

        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        var original = await client.NegotiateAsync(TestContext.Current.CancellationToken);
        Assert.Equal(DefaultSourceTree, original.SystemInfo.SourceTree);

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.TransportDisconnected, exception.Kind);
        Assert.Equal(ConnectionState.Recovering, client.State);
        Assert.Null(client.Hello);
        Assert.Null(client.SystemInfo);

        EnqueueNegotiation(transport, replacementSourceTree);
        var reconnected = await client.NegotiateAsync(
            TestContext.Current.CancellationToken);

        Assert.Equal(ConnectionState.Ready, client.State);
        Assert.Equal(replacementSourceTree, reconnected.SystemInfo.SourceTree);
        Assert.NotEqual(original.SystemInfo.SourceTree, reconnected.SystemInfo.SourceTree);
    }


    [Fact]
    public async Task DeviceSessionAutomaticallyRecoversSameLocatorWithFreshNegotiation()
    {
        const string replacementSourceTree =
            "abcdef0123456789abcdef0123456789abcdef01";

        var first = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(first, DefaultSourceTree);

        var second = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(second, replacementSourceTree);
        second.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcPing,
                3,
                "PONG\r\n"));

        var discovery = new ScriptedDiscovery(first, second);
        await using var session = new DeusDeviceSession(
            discovery,
            recoveryTimeout: TimeSpan.FromSeconds(1),
            recoveryPollInterval: TimeSpan.FromMilliseconds(1));

        var states = new List<ConnectionState>();
        session.StateChanged += states.Add;

        var initial = await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);
        Assert.Equal(DefaultSourceTree, initial.SystemInfo.SourceTree);
        Assert.Equal(ConnectionState.Ready, session.State);

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => session.ExecuteAsync(
                (client, token) => client.PingAsync(token),
                TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.TransportDisconnected, exception.Kind);
        Assert.Equal(ConnectionState.Ready, session.State);
        Assert.Equal(
            replacementSourceTree,
            session.LastNegotiation?.SystemInfo.SourceTree);
        Assert.Equal(2, discovery.OpenCount);
        Assert.Contains(ConnectionState.Recovering, states);
        Assert.Contains(ConnectionState.Opening, states);
        Assert.Contains(ConnectionState.Negotiating, states);

        var ping = await session.ExecuteAsync(
            (client, token) => client.PingAsync(token),
            TestContext.Current.CancellationToken);

        Assert.Equal((ushort)3, ping.RequestId);
        Assert.Equal("PONG\r\n", ping.OutputText);
    }

    private const string DefaultSourceTree =
        "0123456789abcdef0123456789abcdef01234567";

    private static void EnqueueNegotiation(
        ScriptedTransport transport,
        string sourceTree)
    {
        transport.EnqueueResponse(CreateHelloResponse(1));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(sourceTree)));
    }

    private static string ValidSysInfo(
        string sourceTree = DefaultSourceTree) =>
        "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
        $"SOURCE_TREE={sourceTree}\r\n" +
        "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
        "CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000\r\n";

    private static byte[] CreateHelloResponse(ushort requestId)
    {
        var payload = new byte[16];
        payload[0] = 1;
        payload[1] = 3;
        payload[2] = 4;
        payload[3] = 0;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4, 2), 32);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(6, 2), 132);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(8, 2), 48);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(10, 2), 36);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(12, 4), 0x3F);

        return BinaryFrameCodec.Encode(
            FrameType.HelloResponse,
            0,
            requestId,
            payload);
    }

    private static byte[] CreateRpcResponse(
        ushort rpcId,
        ushort requestId,
        string output)
    {
        var bytes = Encoding.UTF8.GetBytes(output);
        var wire = new List<byte>();
        ushort sequence = 0;
        ushort chunks = 0;
        var offset = 0;

        while (offset < bytes.Length)
        {
            var length = Math.Min(
                ProtocolConstants.RpcDataChunkMax,
                bytes.Length - offset);
            var payload = new byte[4 + length];
            BinaryPrimitives.WriteUInt16LittleEndian(
                payload.AsSpan(0, 2),
                rpcId);
            BinaryPrimitives.WriteUInt16LittleEndian(
                payload.AsSpan(2, 2),
                sequence++);
            bytes.AsSpan(offset, length).CopyTo(payload.AsSpan(4));

            wire.AddRange(BinaryFrameCodec.Encode(
                FrameType.RpcData,
                0,
                requestId,
                payload));

            ++chunks;
            offset += length;
        }

        var end = new byte[10];
        BinaryPrimitives.WriteUInt16LittleEndian(end.AsSpan(0, 2), rpcId);
        BinaryPrimitives.WriteUInt16LittleEndian(end.AsSpan(2, 2), chunks);
        BinaryPrimitives.WriteUInt32LittleEndian(
            end.AsSpan(4, 4),
            checked((uint)bytes.Length));
        end[8] = 0;
        end[9] = 0;

        wire.AddRange(BinaryFrameCodec.Encode(
            FrameType.RpcEnd,
            0,
            requestId,
            end));

        return wire.ToArray();
    }


    private sealed class ScriptedDiscovery : IDeviceDiscovery
    {
        private readonly Queue<IDeviceTransport> _transports;

        public ScriptedDiscovery(params IDeviceTransport[] transports)
        {
            _transports = new Queue<IDeviceTransport>(transports);
        }

        public DeviceCandidate Candidate { get; } = new(
            "mock:device",
            "Mock Deus OS Device",
            "Mock");

        public int OpenCount { get; private set; }

        public ValueTask<IReadOnlyList<DeviceCandidate>> DiscoverAsync(
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            IReadOnlyList<DeviceCandidate> candidates = new[] { Candidate };
            return ValueTask.FromResult(candidates);
        }

        public ValueTask<IDeviceTransport> OpenAsync(
            DeviceCandidate candidate,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();

            if (!string.Equals(
                    candidate.Locator,
                    Candidate.Locator,
                    StringComparison.Ordinal))
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    $"unexpected mock locator '{candidate.Locator}'");
            }

            if (_transports.Count == 0)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    "no scripted transport remains");
            }

            ++OpenCount;
            return ValueTask.FromResult(_transports.Dequeue());
        }
    }

    private sealed class ScriptedTransport : IDeviceTransport
    {
        private readonly Queue<byte> _reads = new();
        private readonly int _readChunkLimit;

        public ScriptedTransport(int readChunkLimit)
        {
            _readChunkLimit = readChunkLimit;
        }

        public string Locator => "mock:device";

        public List<byte[]> Writes { get; } = new();

        public bool BlockWhenEmpty { get; set; }

        public void EnqueueResponse(byte[] wire)
        {
            foreach (var value in wire)
            {
                _reads.Enqueue(value);
            }
        }

        public ValueTask WriteAsync(
            ReadOnlyMemory<byte> data,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            Writes.Add(data.ToArray());
            return ValueTask.CompletedTask;
        }

        public async ValueTask<int> ReadAsync(
            Memory<byte> buffer,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();

            if (_reads.Count == 0)
            {
                if (!BlockWhenEmpty)
                {
                    return 0;
                }

                await Task.Delay(
                    Timeout.InfiniteTimeSpan,
                    cancellationToken);
                return 0;
            }

            var count = Math.Min(
                Math.Min(buffer.Length, _readChunkLimit),
                _reads.Count);

            for (var index = 0; index < count; ++index)
            {
                buffer.Span[index] = _reads.Dequeue();
            }

            return count;
        }

        public ValueTask DisposeAsync() =>
            ValueTask.CompletedTask;
    }
}
