using System.Buffers.Binary;
using System.Reflection;
using System.Text;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class ManagementServiceOperationTests
{
    private const string SourceTree =
        "0123456789abcdef0123456789abcdef01234567";

    [Fact]
    public void CatalogMatchesFrozenServiceV1AllowlistExactly()
    {
        var operations = ManagementServiceOperationCatalog.Operations;

        Assert.Equal(8, operations.Count);
        Assert.Equal(
            new[]
            {
                ManagementServiceOperation.Ping,
                ManagementServiceOperation.Health,
                ManagementServiceOperation.Applications,
                ManagementServiceOperation.AssetStatus,
                ManagementServiceOperation.ReadOledUiLayout,
                ManagementServiceOperation.StartApplication,
                ManagementServiceOperation.StopApplication,
                ManagementServiceOperation.WriteOledUiLayout,
            },
            operations.Select(descriptor => descriptor.Operation));
        Assert.Equal(
            5,
            operations.Count(
                descriptor =>
                    descriptor.Access == ManagementServiceOperationAccess.ReadOnly));
        Assert.Equal(
            3,
            operations.Count(
                descriptor =>
                    descriptor.Access == ManagementServiceOperationAccess.Control));
        Assert.DoesNotContain(
            operations,
            descriptor =>
                descriptor.Access == ManagementServiceOperationAccess.Destructive);
        Assert.Equal(
            operations.Count,
            operations.Select(descriptor => descriptor.Operation).Distinct().Count());

        Assert.Equal(
            SystemCapability.None,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.Ping).RequiredCapability);
        Assert.Equal(
            SystemCapability.SystemHealth,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.Health).RequiredCapability);
        Assert.Equal(
            SystemCapability.ApplicationRuntime,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.Applications).RequiredCapability);
        Assert.Equal(
            SystemCapability.ApplicationControl,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.StartApplication).RequiredCapability);
        Assert.Equal(
            SystemCapability.ApplicationControl,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.StopApplication).RequiredCapability);
        Assert.Equal(
            SystemCapability.AssetConfigurationTransfer,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.AssetStatus).RequiredCapability);
        Assert.Equal(
            SystemCapability.AssetConfigurationTransfer,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.ReadOledUiLayout).RequiredCapability);
        Assert.Equal(
            SystemCapability.AssetConfigurationTransfer,
            ManagementServiceOperationCatalog.Get(
                ManagementServiceOperation.WriteOledUiLayout).RequiredCapability);
    }

    [Fact]
    public void CatalogRejectsUndefinedOperation()
    {
        var undefined = (ManagementServiceOperation)ushort.MaxValue;

        Assert.False(
            ManagementServiceOperationCatalog.TryGet(
                undefined,
                out var descriptor));
        Assert.Null(descriptor);
        Assert.Throws<ArgumentOutOfRangeException>(
            () => ManagementServiceOperationCatalog.Get(undefined));
    }

    [Fact]
    public void FacadePublicSurfaceHasNoRawOrDestructiveRoute()
    {
        var methods = typeof(ManagementServiceOperations)
            .GetMethods(
                BindingFlags.Instance |
                BindingFlags.Public |
                BindingFlags.DeclaredOnly)
            .Where(method => !method.IsSpecialName)
            .ToArray();

        Assert.Equal(
            new[]
            {
                nameof(ManagementServiceOperations.ApplicationsAsync),
                nameof(ManagementServiceOperations.AssetStatusAsync),
                nameof(ManagementServiceOperations.HealthAsync),
                nameof(ManagementServiceOperations.PingAsync),
                nameof(ManagementServiceOperations.ReadOledUiLayoutAsync),
                nameof(ManagementServiceOperations.StartApplicationAsync),
                nameof(ManagementServiceOperations.StopApplicationAsync),
                nameof(ManagementServiceOperations.WriteOledUiLayoutAsync),
            },
            methods.Select(method => method.Name).Order(StringComparer.Ordinal));

        foreach (var method in methods)
        {
            Assert.DoesNotContain("Rpc", method.Name, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Bootloader", method.Name, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Firmware", method.Name, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Watchdog", method.Name, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Diagnostic", method.Name, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Stress", method.Name, StringComparison.OrdinalIgnoreCase);

            foreach (var parameter in method.GetParameters())
            {
                Assert.DoesNotContain(
                    parameter.Name ?? string.Empty,
                    new[] { "rpcId", "flags", "arguments", "command" },
                    StringComparer.OrdinalIgnoreCase);
                Assert.NotEqual(typeof(IReadOnlyList<string>), parameter.ParameterType);
            }
        }

        var start = Assert.Single(
            methods,
            method =>
                method.Name == nameof(ManagementServiceOperations.StartApplicationAsync));
        var applicationId = Assert.Single(
            start.GetParameters(),
            parameter => parameter.ParameterType == typeof(ushort));
        Assert.Equal("applicationId", applicationId.Name);
    }

    [Fact]
    public async Task FacadeRoutesTypedRpcOperationsThroughSession()
    {
        var transport = new ScriptedTransport();
        EnqueueNegotiation(transport, SystemCapability.SystemHealth |
            SystemCapability.ApplicationRuntime |
            SystemCapability.ApplicationControl |
            SystemCapability.Diagnostics |
            SystemCapability.LocalUi);
        transport.EnqueueResponse(
            CreateRpcResponse(ProtocolConstants.RpcPing, 3, "PONG\r\n"));
        transport.EnqueueResponse(
            CreateRpcResponse(ProtocolConstants.RpcHealth, 4, "HEALTH=OK\r\n"));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcAppList,
                5,
                ValidApplicationList()));
        transport.EnqueueResponse(
            CreateRpcResponse(ProtocolConstants.RpcAppStart, 6, "APP_START=OK\r\n"));
        transport.EnqueueResponse(
            CreateRpcResponse(ProtocolConstants.RpcAppStop, 7, "APP_STOP=OK\r\n"));

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);
        var operations = new ManagementServiceOperations(session);

        var negotiation = await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        Assert.Same(negotiation, operations.LastNegotiation);
        Assert.Equal(ConnectionState.Ready, operations.State);

        var ping = await operations.PingAsync(TestContext.Current.CancellationToken);
        var health = await operations.HealthAsync(TestContext.Current.CancellationToken);
        var applications = await operations.ApplicationsAsync(
            TestContext.Current.CancellationToken);
        var start = await operations.StartApplicationAsync(
            2,
            TestContext.Current.CancellationToken);
        var stop = await operations.StopApplicationAsync(
            TestContext.Current.CancellationToken);

        Assert.Equal("PONG\r\n", ping.OutputText);
        Assert.Equal("HEALTH=OK\r\n", health.OutputText);
        Assert.Equal((uint)2, applications.RegistryCount);
        Assert.Equal((ushort)1, applications.ActiveId);
        Assert.Equal("APP_START=OK\r\n", start.OutputText);
        Assert.Equal("APP_STOP=OK\r\n", stop.OutputText);

        Assert.Equal(7, transport.Writes.Count);
        Assert.Equal(
            new ushort[]
            {
                ProtocolConstants.RpcPing,
                ProtocolConstants.RpcHealth,
                ProtocolConstants.RpcAppList,
                ProtocolConstants.RpcAppStart,
                ProtocolConstants.RpcAppStop,
            },
            transport.Writes
                .Skip(2)
                .Select(ReadRpcId));
    }

    [Fact]
    public async Task StartApplicationPreservesTypedApplicationIdValidation()
    {
        var transport = new ScriptedTransport();
        EnqueueNegotiation(
            transport,
            SystemCapability.SystemHealth |
            SystemCapability.ApplicationRuntime |
            SystemCapability.ApplicationControl |
            SystemCapability.Diagnostics |
            SystemCapability.LocalUi);

        var discovery = new ScriptedDiscovery(transport);
        await using var session = new DeusDeviceSession(discovery);
        var operations = new ManagementServiceOperations(session);
        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        await Assert.ThrowsAsync<ArgumentOutOfRangeException>(
            () => operations.StartApplicationAsync(
                0,
                TestContext.Current.CancellationToken));

        Assert.Equal(2, transport.Writes.Count);
    }

    [Fact]
    public async Task AssetStatusIsRestrictedToPublishedCapability()
    {
        var (session, operations, transport) =
            await CreateUnpublishedAssetSessionAsync();
        await using (session)
        {
            var exception = await Assert.ThrowsAsync<DeusHostException>(
                () => operations.AssetStatusAsync(
                    TestContext.Current.CancellationToken));

            Assert.Equal(HostErrorKind.InvalidSystemInfo, exception.Kind);
            Assert.Equal(2, transport.Writes.Count);
        }
    }

    [Fact]
    public async Task AssetReadIsRestrictedToPublishedCapability()
    {
        var (session, operations, transport) =
            await CreateUnpublishedAssetSessionAsync();
        await using (session)
        {
            var exception = await Assert.ThrowsAsync<DeusHostException>(
                () => operations.ReadOledUiLayoutAsync(
                    TestContext.Current.CancellationToken));

            Assert.Equal(HostErrorKind.InvalidSystemInfo, exception.Kind);
            Assert.Equal(2, transport.Writes.Count);
        }
    }

    [Fact]
    public async Task AssetWriteIsRestrictedToPublishedCapability()
    {
        var (session, operations, transport) =
            await CreateUnpublishedAssetSessionAsync();
        await using (session)
        {
            var exception = await Assert.ThrowsAsync<DeusHostException>(
                () => operations.WriteOledUiLayoutAsync(
                    OledUiLayoutConfigV1.Default,
                    TestContext.Current.CancellationToken));

            Assert.Equal(HostErrorKind.InvalidSystemInfo, exception.Kind);
            Assert.Equal(2, transport.Writes.Count);
        }
    }

    private static async Task<(
        DeusDeviceSession Session,
        ManagementServiceOperations Operations,
        ScriptedTransport Transport)> CreateUnpublishedAssetSessionAsync()
    {
        var transport = new ScriptedTransport();
        EnqueueNegotiation(
            transport,
            SystemCapability.SystemHealth |
            SystemCapability.ApplicationRuntime |
            SystemCapability.ApplicationControl |
            SystemCapability.Diagnostics |
            SystemCapability.LocalUi,
            helloCapabilities: 0x7F);

        var discovery = new ScriptedDiscovery(transport);
        var session = new DeusDeviceSession(discovery);
        await session.ConnectAsync(
            discovery.Candidate,
            TestContext.Current.CancellationToken);

        return (
            session,
            new ManagementServiceOperations(session),
            transport);
    }

    private static void EnqueueNegotiation(
        ScriptedTransport transport,
        SystemCapability capabilities,
        uint helloCapabilities = 0x3F)
    {
        transport.EnqueueResponse(CreateHelloResponse(1, helloCapabilities));
        transport.EnqueueResponse(
            CreateRpcResponse(
                ProtocolConstants.RpcSysInfo,
                2,
                ValidSysInfo(capabilities)));
    }

    private static byte[] CreateHelloResponse(
        ushort requestId,
        uint capabilityFlags)
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

    private static string ValidSysInfo(SystemCapability capabilities) =>
        "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
        $"SOURCE_TREE={SourceTree}\r\n" +
        "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
        $"CAPABILITIES=0x{(uint)capabilities:X8} UNIT_ID_KIND=0x00000000\r\n";

    private static string ValidApplicationList() =>
        "APP_RUNTIME_ABI=0x00000001 APP_REGISTRY_COUNT=0x00000002 APP_ACTIVE_ID=0x00000001 " +
        "APP_FAULT_COUNT=0x00000000 APP_EVENT_COUNT=0x00000000 APP_VIEW_REVISION=0x00000001 " +
        "APP_LAST_EVENT_TYPE=0x00000000 APP_LAST_EVENT_SOURCE=0x00000000\r\n" +
        "APP_ID=0x00000001 NAME=system.home ABI=0x00000001 FLAGS=0x00000001 STATE=RUNNING " +
        "STATE_ID=0x00000003 ACTIVE=0x00000001\r\n" +
        "APP_ID=0x00000002 NAME=device.info ABI=0x00000001 FLAGS=0x00000000 STATE=STOPPED " +
        "STATE_ID=0x00000001 ACTIVE=0x00000000\r\n";

    private static ushort ReadRpcId(byte[] wire)
    {
        Assert.Equal((byte)FrameType.RpcRequest, wire[3]);
        return BinaryPrimitives.ReadUInt16LittleEndian(
            wire.AsSpan(ProtocolConstants.FixedPrefixBytes, 2));
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
        private readonly IDeviceTransport _transport;

        public ScriptedDiscovery(IDeviceTransport transport)
        {
            _transport = transport;
        }

        public DeviceCandidate Candidate { get; } = new(
            "mock:service-policy",
            "Mock Service Policy Device",
            "Mock");

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

            return ValueTask.FromResult(_transport);
        }
    }

    private sealed class ScriptedTransport : IDeviceTransport
    {
        private readonly Queue<byte> _reads = new();

        public string Locator => "mock:service-policy";

        public List<byte[]> Writes { get; } = new();

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

        public ValueTask<int> ReadAsync(
            Memory<byte> buffer,
            CancellationToken cancellationToken)
        {
            cancellationToken.ThrowIfCancellationRequested();
            if (_reads.Count == 0)
            {
                return ValueTask.FromResult(0);
            }

            var count = Math.Min(buffer.Length, _reads.Count);
            for (var index = 0; index < count; ++index)
            {
                buffer.Span[index] = _reads.Dequeue();
            }

            return ValueTask.FromResult(count);
        }

        public ValueTask DisposeAsync() => ValueTask.CompletedTask;
    }
}
