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
    public async Task RpcTimeoutDiscardsDelayedStreamBeforeFreshRpc()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;

        var timeout = await Assert.ThrowsAsync<DeusHostException>(
            () => client.RpcAsync(
                ProtocolConstants.RpcPing,
                timeout: TimeSpan.FromMilliseconds(50),
                cancellationToken: TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Timeout, timeout.Kind);

        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            3,
            "OLD\r\n"));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            4,
            "PONG\r\n"));

        var fresh = await client.PingAsync(TestContext.Current.CancellationToken);

        Assert.Equal((ushort)4, fresh.RequestId);
        Assert.Equal("PONG\r\n", fresh.OutputText);
        Assert.Equal(ConnectionState.Ready, client.State);
    }

    [Fact]
    public async Task RpcCancellationDiscardsDelayedStreamBeforeFreshRpc()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;
        using var cancellation = CancellationTokenSource.CreateLinkedTokenSource(
            TestContext.Current.CancellationToken);
        cancellation.CancelAfter(TimeSpan.FromMilliseconds(50));

        var cancelled = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(cancellation.Token));

        Assert.Equal(HostErrorKind.Cancelled, cancelled.Kind);

        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            3,
            "OLD\r\n"));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            4,
            "PONG\r\n"));

        var fresh = await client.PingAsync(TestContext.Current.CancellationToken);

        Assert.Equal((ushort)4, fresh.RequestId);
        Assert.Equal("PONG\r\n", fresh.OutputText);
    }

    [Fact]
    public async Task RpcTimeoutAfterPartialDataDiscardsRemainingDelayedStream()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;
        transport.EnqueueResponse(CreateRpcDataFrame(
            ProtocolConstants.RpcPing,
            3,
            0,
            "PART"));

        var timeout = await Assert.ThrowsAsync<DeusHostException>(
            () => client.RpcAsync(
                ProtocolConstants.RpcPing,
                timeout: TimeSpan.FromMilliseconds(50),
                cancellationToken: TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Timeout, timeout.Kind);

        transport.EnqueueResponse(CreateRpcDataFrame(
            ProtocolConstants.RpcPing,
            3,
            1,
            "LATE"));
        transport.EnqueueResponse(CreateRpcEndFrame(
            ProtocolConstants.RpcPing,
            3,
            2,
            8));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            4,
            "PONG\r\n"));

        var fresh = await client.PingAsync(TestContext.Current.CancellationToken);

        Assert.Equal((ushort)4, fresh.RequestId);
        Assert.Equal("PONG\r\n", fresh.OutputText);
    }

    [Fact]
    public async Task RpcAbandonedProtocolErrorIsTerminalAndDoesNotPoisonFreshRpc()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;

        await Assert.ThrowsAsync<DeusHostException>(
            () => client.RpcAsync(
                ProtocolConstants.RpcPing,
                timeout: TimeSpan.FromMilliseconds(50),
                cancellationToken: TestContext.Current.CancellationToken));

        transport.EnqueueResponse(BinaryFrameCodec.Encode(
            FrameType.ProtocolError,
            0,
            3,
            new byte[] { 0x03, (byte)FrameType.RpcRequest }));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            4,
            "PONG\r\n"));

        var fresh = await client.PingAsync(TestContext.Current.CancellationToken);

        Assert.Equal((ushort)4, fresh.RequestId);
        Assert.Equal("PONG\r\n", fresh.OutputText);
    }

    [Fact]
    public async Task MultipleAbandonedRpcStreamsRemainIsolatedUntilTheirTerminals()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;

        for (var index = 0; index < 2; ++index)
        {
            var timeout = await Assert.ThrowsAsync<DeusHostException>(
                () => client.RpcAsync(
                    ProtocolConstants.RpcPing,
                    timeout: TimeSpan.FromMilliseconds(50),
                    cancellationToken: TestContext.Current.CancellationToken));
            Assert.Equal(HostErrorKind.Timeout, timeout.Kind);
        }

        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            3,
            "OLD3\r\n"));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            4,
            "OLD4\r\n"));
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            5,
            "PONG\r\n"));

        var fresh = await client.PingAsync(TestContext.Current.CancellationToken);

        Assert.Equal((ushort)5, fresh.RequestId);
        Assert.Equal("PONG\r\n", fresh.OutputText);
    }

    [Fact]
    public async Task UnresolvedAbandonedRpcFailsClosedAtRequestIdWrap()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;

        var timeout = await Assert.ThrowsAsync<DeusHostException>(
            () => client.RpcAsync(
                ProtocolConstants.RpcPing,
                timeout: TimeSpan.FromMilliseconds(50),
                cancellationToken: TestContext.Current.CancellationToken));
        Assert.Equal(HostErrorKind.Timeout, timeout.Kind);

        transport.ResponseFactory = CreateAutomaticRpcResponse;

        for (var requestId = 4; requestId <= ushort.MaxValue; ++requestId)
        {
            var result = await client.PingAsync(TestContext.Current.CancellationToken);
            if (requestId == 4 || requestId == ushort.MaxValue)
            {
                Assert.Equal((ushort)requestId, result.RequestId);
            }
        }

        var wrap = await Assert.ThrowsAsync<DeusHostException>(
            () => client.PingAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.RequestCorrelation, wrap.Kind);
        Assert.Contains("fresh client/transport session", wrap.Message);

        var renegotiate = await Assert.ThrowsAsync<DeusHostException>(
            () => client.NegotiateAsync(TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.RequestCorrelation, renegotiate.Kind);
        Assert.Equal(ConnectionState.Faulted, client.State);
    }

    [Fact]
    public async Task AssetCancellationDiscardsDelayedSingleResponseBeforeFreshAssetRequest()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        transport.EnqueueResponse(CreateHelloResponse(1, 0x7F));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(DefaultSourceTree, 0x3F)));

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.BlockWhenEmpty = true;
        using var cancellation = CancellationTokenSource.CreateLinkedTokenSource(
            TestContext.Current.CancellationToken);
        cancellation.CancelAfter(TimeSpan.FromMilliseconds(50));

        await Assert.ThrowsAnyAsync<OperationCanceledException>(
            () => client.AssetStatusAsync(
                AssetAccessPolicy.PublishedOnly,
                cancellation.Token));

        Assert.Equal(
            (ushort)3,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[2].AsSpan(6, 2)));

        transport.EnqueueResponse(CreateAssetStatusResponse(3));
        transport.EnqueueResponse(CreateAssetStatusResponse(4));

        var fresh = await client.AssetStatusAsync(
            AssetAccessPolicy.PublishedOnly,
            TestContext.Current.CancellationToken);

        Assert.Equal(AssetTransferStatus.Ok, fresh.Status);
        Assert.Equal(AssetTransferSessionState.None, fresh.SessionState);
        Assert.Equal(
            (ushort)4,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[3].AsSpan(6, 2)));
        Assert.Equal(ConnectionState.Ready, client.State);
    }

    [Fact]
    public async Task AssetNativeTimeoutDiscardsDelayedSingleResponseBeforeFreshAssetRequest()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        transport.EnqueueResponse(CreateHelloResponse(1, 0x7F));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(DefaultSourceTree, 0x3F)));

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        transport.ResponseFactory = _ =>
            throw new DeusHostException(
                HostErrorKind.Timeout,
                "synthetic Asset native timeout");

        var timeout = await Assert.ThrowsAsync<DeusHostException>(
            () => client.AssetStatusAsync(
                AssetAccessPolicy.PublishedOnly,
                TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.Timeout, timeout.Kind);
        Assert.Equal(
            (ushort)3,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[2].AsSpan(6, 2)));

        transport.ResponseFactory = null;
        transport.EnqueueResponse(CreateAssetStatusResponse(3));
        transport.EnqueueResponse(CreateAssetStatusResponse(4));

        var fresh = await client.AssetStatusAsync(
            AssetAccessPolicy.PublishedOnly,
            TestContext.Current.CancellationToken);

        Assert.Equal(AssetTransferStatus.Ok, fresh.Status);
        Assert.Equal(
            (ushort)4,
            BinaryPrimitives.ReadUInt16LittleEndian(
                transport.Writes[3].AsSpan(6, 2)));
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
    public async Task DeviceSessionStateNotificationsPreserveConnectOrder()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);

        var states = new List<ConnectionState>();
        var ready = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);

        session.StateChanged += state =>
        {
            lock (states)
            {
                states.Add(state);
            }

            if (state == ConnectionState.Ready)
            {
                ready.TrySetResult(true);
            }
        };

        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        await ready.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        ConnectionState[] observedStates;
        lock (states)
        {
            observedStates = states.ToArray();
        }

        Assert.Equal(
            new[]
            {
                ConnectionState.Discovered,
                ConnectionState.Opening,
                ConnectionState.Negotiating,
                ConnectionState.Ready,
            },
            observedStates);
    }

    [Fact]
    public async Task StateChangedSubscriberCanSynchronouslyExecuteWithoutDeadlock()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);
        transport.EnqueueResponse(CreateRpcResponse(
            ProtocolConstants.RpcPing,
            3,
            "PONG\r\n"));

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);
        using var reentrantTimeout = CancellationTokenSource.CreateLinkedTokenSource(
            TestContext.Current.CancellationToken);
        reentrantTimeout.CancelAfter(TimeSpan.FromSeconds(1));

        var nestedResult = new TaskCompletionSource<RpcResult>(
            TaskCreationOptions.RunContinuationsAsynchronously);

        session.StateChanged += state =>
        {
            if (state != ConnectionState.Ready)
            {
                return;
            }

            try
            {
                var result = session.ExecuteAsync(
                    (client, token) => client.PingAsync(token),
                    reentrantTimeout.Token).GetAwaiter().GetResult();
                nestedResult.TrySetResult(result);
            }
            catch (Exception exception)
            {
                nestedResult.TrySetException(exception);
            }
        };

        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        var ping = await nestedResult.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        Assert.Equal((ushort)3, ping.RequestId);
        Assert.Equal("PONG\r\n", ping.OutputText);
        Assert.Equal(ConnectionState.Ready, session.State);
    }

    [Fact]
    public async Task StateChangedSubscriberCanSynchronouslyDisconnectWithoutDeadlock()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);
        using var reentrantTimeout = CancellationTokenSource.CreateLinkedTokenSource(
            TestContext.Current.CancellationToken);
        reentrantTimeout.CancelAfter(TimeSpan.FromSeconds(1));

        var disconnected = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);

        session.StateChanged += state =>
        {
            if (state != ConnectionState.Ready)
            {
                return;
            }

            try
            {
                session.DisconnectAsync(reentrantTimeout.Token)
                    .GetAwaiter()
                    .GetResult();
                disconnected.TrySetResult(true);
            }
            catch (Exception exception)
            {
                disconnected.TrySetException(exception);
            }
        };

        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        await disconnected.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        Assert.Equal(ConnectionState.Disconnected, session.State);
        Assert.Null(session.Candidate);
        Assert.Null(session.LastNegotiation);
    }

    [Fact]
    public async Task ThrowingStateChangedSubscriberIsIsolatedAcrossRecoveryAndDisconnect()
    {
        const string replacementSourceTree =
            "abcdef0123456789abcdef0123456789abcdef01";

        var first = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(first, DefaultSourceTree);

        var second = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(second, replacementSourceTree);

        var discovery = new ScriptedDiscovery(first, second);
        await using var session = new DeusDeviceSession(
            discovery,
            recoveryTimeout: TimeSpan.FromSeconds(1),
            recoveryPollInterval: TimeSpan.FromMilliseconds(1));

        var secondReady = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        var disconnected = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        var readyCount = 0;

        session.StateChanged += _ =>
            throw new InvalidOperationException("observer failure");
        session.StateChanged += state =>
        {
            if (state == ConnectionState.Ready &&
                Interlocked.Increment(ref readyCount) == 2)
            {
                secondReady.TrySetResult(true);
            }
            else if (state == ConnectionState.Disconnected)
            {
                disconnected.TrySetResult(true);
            }
        };

        var negotiation = await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);
        Assert.Equal(DefaultSourceTree, negotiation.SystemInfo.SourceTree);

        var disconnect = await Assert.ThrowsAsync<DeusHostException>(
            () => session.ExecuteAsync(
                (client, token) => client.PingAsync(token),
                TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.TransportDisconnected, disconnect.Kind);
        Assert.Equal(ConnectionState.Ready, session.State);
        Assert.Equal(
            replacementSourceTree,
            session.LastNegotiation?.SystemInfo.SourceTree);

        await secondReady.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        await session.DisconnectAsync(TestContext.Current.CancellationToken);
        await disconnected.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        Assert.Equal(ConnectionState.Disconnected, session.State);
        Assert.Equal(2, discovery.OpenCount);
    }

    [Fact]
    public async Task StateChangedUnsubscribePreventsLaterNotifications()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);

        var observed = new List<ConnectionState>();
        var ready = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        var disconnected = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);

        Action<ConnectionState> removable = state =>
        {
            lock (observed)
            {
                observed.Add(state);
            }
        };

        session.StateChanged += removable;
        session.StateChanged += state =>
        {
            if (state == ConnectionState.Ready)
            {
                ready.TrySetResult(true);
            }
            else if (state == ConnectionState.Disconnected)
            {
                disconnected.TrySetResult(true);
            }
        };

        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);
        await ready.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        session.StateChanged -= removable;
        await session.DisconnectAsync(TestContext.Current.CancellationToken);
        await disconnected.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        ConnectionState[] removableStates;
        lock (observed)
        {
            removableStates = observed.ToArray();
        }

        Assert.Equal(
            new[]
            {
                ConnectionState.Discovered,
                ConnectionState.Opening,
                ConnectionState.Negotiating,
                ConnectionState.Ready,
            },
            removableStates);
    }

    [Fact]
    public async Task DisposeDoesNotWaitForBlockedStateChangedSubscriber()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        var discovery = new ScriptedDiscovery(transport);
        var session = new DeusDeviceSession(discovery);
        var callbackStarted = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        var releaseCallback = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);

        session.StateChanged += state =>
        {
            if (state == ConnectionState.Ready)
            {
                callbackStarted.TrySetResult(true);
                releaseCallback.Task.GetAwaiter().GetResult();
            }
        };

        try
        {
            await session.ConnectAsync(
                discovery.Candidate,
                TestContext.Current.CancellationToken);
            await callbackStarted.Task.WaitAsync(
                TimeSpan.FromSeconds(2),
                TestContext.Current.CancellationToken);

            await session.DisposeAsync().AsTask().WaitAsync(
                TimeSpan.FromSeconds(1),
                TestContext.Current.CancellationToken);

            Assert.Equal(ConnectionState.Disconnected, session.State);

            await session.DisposeAsync();
        }
        finally
        {
            releaseCallback.TrySetResult(true);
            await session.DisposeAsync();
        }
    }

    [Fact]
    public async Task DisconnectDisposeRaceIsBoundedAndIdempotent()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        EnqueueNegotiation(transport, DefaultSourceTree);

        var discovery = new ScriptedDiscovery(transport);
        var session = new DeusDeviceSession(discovery);

        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        var disconnectTask = Task.Run(async () =>
        {
            try
            {
                await session.DisconnectAsync(TestContext.Current.CancellationToken);
                return (Exception?)null;
            }
            catch (Exception exception)
            {
                return exception;
            }
        }, TestContext.Current.CancellationToken);
        var disposeTask = session.DisposeAsync().AsTask();

        var disconnectException = await disconnectTask.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);
        await disposeTask.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        Assert.True(
            disconnectException is null or ObjectDisposedException,
            disconnectException?.ToString());
        Assert.Equal(ConnectionState.Disconnected, session.State);

        await session.DisposeAsync();
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
        var secondReady = new TaskCompletionSource<bool>(
            TaskCreationOptions.RunContinuationsAsynchronously);
        var readyCount = 0;
        session.StateChanged += state =>
        {
            lock (states)
            {
                states.Add(state);
            }

            if (state == ConnectionState.Ready &&
                Interlocked.Increment(ref readyCount) == 2)
            {
                secondReady.TrySetResult(true);
            }
        };

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

        await secondReady.Task.WaitAsync(
            TimeSpan.FromSeconds(2),
            TestContext.Current.CancellationToken);

        ConnectionState[] observedStates;
        lock (states)
        {
            observedStates = states.ToArray();
        }

        Assert.Equal(
            new[]
            {
                ConnectionState.Discovered,
                ConnectionState.Opening,
                ConnectionState.Negotiating,
                ConnectionState.Ready,
                ConnectionState.Recovering,
                ConnectionState.Discovered,
                ConnectionState.Opening,
                ConnectionState.Negotiating,
                ConnectionState.Ready,
            },
            observedStates);

        var ping = await session.ExecuteAsync(
            (client, token) => client.PingAsync(token),
            TestContext.Current.CancellationToken);

        Assert.Equal((ushort)3, ping.RequestId);
        Assert.Equal("PONG\r\n", ping.OutputText);
    }

    [Fact]
    public async Task PublishedFirmwareCapabilityEnablesPublishedBootloaderEntry()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        transport.EnqueueResponse(CreateHelloResponse(1, 0x7F));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(DefaultSourceTree, 0x7F)));
        transport.EnqueueResponse(CreateFirmwareUpdateResponse(
            3,
            0x01,
            FirmwareUpdateStatus.Ok,
            FirmwareUpdateState.Resetting,
            0,
            1,
            1));

        await using var client = new DeusDeviceClient(transport);
        var negotiation = await client.NegotiateAsync(
            TestContext.Current.CancellationToken);

        Assert.True(
            (negotiation.SystemInfo.Capabilities &
             SystemCapability.FirmwareUpdate) != 0);

        var response = await client.EnterBootloaderAsync(
            FirmwareUpdateAccessPolicy.PublishedOnly,
            TestContext.Current.CancellationToken);

        Assert.Equal(FirmwareUpdateStatus.Ok, response.Status);
        Assert.Equal(FirmwareUpdateState.Resetting, response.State);
        Assert.Equal(ConnectionState.Recovering, client.State);
        Assert.Equal(3, transport.Writes.Count);
        Assert.Equal(
            (byte)FrameType.FirmwareUpdateRequest,
            transport.Writes[2][3]);
        Assert.Equal((byte)0x01, transport.Writes[2][10]);
    }

    [Fact]
    public async Task PublishedBootloaderEntryStillRejectsDeviceWithoutFirmwareCapability()
    {
        var transport = new ScriptedTransport(readChunkLimit: 512);
        transport.EnqueueResponse(CreateHelloResponse(1, 0x7F));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(DefaultSourceTree, 0x3F)));

        await using var client = new DeusDeviceClient(transport);
        await client.NegotiateAsync(TestContext.Current.CancellationToken);

        var exception = await Assert.ThrowsAsync<DeusHostException>(
            () => client.EnterBootloaderAsync(
                FirmwareUpdateAccessPolicy.PublishedOnly,
                TestContext.Current.CancellationToken));

        Assert.Equal(HostErrorKind.IncompatibleProtocol, exception.Kind);
        Assert.Equal(ConnectionState.Ready, client.State);
        Assert.Equal(2, transport.Writes.Count);
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
        string sourceTree = DefaultSourceTree,
        uint capabilities = 0x0000001F) =>
        "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
        $"SOURCE_TREE={sourceTree}\r\n" +
        "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
        $"CAPABILITIES=0x{capabilities:X8} UNIT_ID_KIND=0x00000000\r\n";

    private static byte[] CreateHelloResponse(
        ushort requestId,
        uint capabilityFlags = 0x0000003F)
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
        BinaryPrimitives.WriteUInt32LittleEndian(
            payload.AsSpan(12, 4),
            capabilityFlags);

        return BinaryFrameCodec.Encode(
            FrameType.HelloResponse,
            0,
            requestId,
            payload);
    }

    private static byte[] CreateAssetStatusResponse(ushort requestId)
    {
        var payload = new byte[
            AssetTransferProtocol.CommonResponseBytes + 8];
        payload[0] = AssetTransferProtocol.Version;
        payload[1] = (byte)AssetTransferOpcode.Status;
        payload[2] = (byte)AssetTransferStatus.Ok;
        payload[3] = (byte)AssetTransferSessionState.None;
        BinaryPrimitives.WriteUInt16LittleEndian(
            payload.AsSpan(4, 2),
            AssetTransferProtocol.OledUiLayoutObjectType);
        payload[16] = 8;

        return BinaryFrameCodec.Encode(
            FrameType.AssetTransferResponse,
            0,
            requestId,
            payload);
    }

    private static byte[] CreateFirmwareUpdateResponse(
        ushort requestId,
        byte opcode,
        FirmwareUpdateStatus status,
        FirmwareUpdateState state,
        ushort expectedOffset,
        uint versionFloor,
        uint committedVersion)
    {
        var payload = new byte[16];
        payload[0] = opcode;
        payload[1] = (byte)status;
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

    private static byte[] CreateAutomaticRpcResponse(byte[] wire)
    {
        Assert.Equal((byte)FrameType.RpcRequest, wire[3]);
        var requestId = BinaryPrimitives.ReadUInt16LittleEndian(wire.AsSpan(6, 2));
        var rpcId = BinaryPrimitives.ReadUInt16LittleEndian(
            wire.AsSpan(ProtocolConstants.FixedPrefixBytes, 2));
        return CreateRpcResponse(rpcId, requestId, "PONG\r\n");
    }

    private static byte[] CreateRpcDataFrame(
        ushort rpcId,
        ushort requestId,
        ushort sequence,
        string output)
    {
        var bytes = Encoding.UTF8.GetBytes(output);
        var payload = new byte[4 + bytes.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(0, 2), rpcId);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), sequence);
        bytes.AsSpan().CopyTo(payload.AsSpan(4));
        return BinaryFrameCodec.Encode(
            FrameType.RpcData,
            0,
            requestId,
            payload);
    }

    private static byte[] CreateRpcEndFrame(
        ushort rpcId,
        ushort requestId,
        ushort chunkCount,
        uint totalBytes)
    {
        var payload = new byte[10];
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(0, 2), rpcId);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2, 2), chunkCount);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4, 4), totalBytes);
        payload[8] = 0;
        payload[9] = 0;
        return BinaryFrameCodec.Encode(
            FrameType.RpcEnd,
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

        public Func<byte[], byte[]?>? ResponseFactory { get; set; }

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
            var wire = data.ToArray();
            Writes.Add(wire);

            var response = ResponseFactory?.Invoke(wire);
            if (response is not null)
            {
                EnqueueResponse(response);
            }

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
