namespace DeusOs.Control.Core;

public sealed class DeusDeviceClient : IAsyncDisposable
{
    private readonly DeviceProtocolChannel _channel;
    private readonly DeusRpcClient _rpc;
    private readonly AssetTransferClient _asset;
    private readonly FirmwareUpdateClient _firmwareUpdate;
    private bool _disposed;

    public DeusDeviceClient(IDeviceTransport transport)
    {
        _channel = new DeviceProtocolChannel(
            transport ?? throw new ArgumentNullException(nameof(transport)));
        _rpc = new DeusRpcClient(_channel);
        _asset = new AssetTransferClient(_channel);
        _firmwareUpdate = new FirmwareUpdateClient(_channel);
        State = ConnectionState.Discovered;
    }

    public ConnectionState State { get; private set; }

    public string Locator => _channel.Locator;

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

            var requestId = _channel.NextRequestId();
            var helloRequest = BinaryFrameCodec.Encode(
                FrameType.HelloRequest,
                0,
                requestId,
                ReadOnlySpan<byte>.Empty);

            await _channel.WriteAsync(helloRequest, timeout.Token);
            var helloFrame = await _channel.ReadMatchingFrameAsync(
                requestId,
                FrameType.HelloResponse,
                timeout.Token);

            var hello = HelloParser.Parse(helloFrame);
            ValidateHello(hello);

            var sysinfoResult = await _rpc.RpcCoreAsync(
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

        return await ExecuteProtocolOperationAsync(
            () => _rpc.RpcAsync(
                rpcId,
                arguments,
                flags,
                timeout,
                cancellationToken));
    }

    public async Task<FirmwareUpdateResponse> EnterBootloaderAsync(
        FirmwareUpdateAccessPolicy policy = FirmwareUpdateAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();

        if (State != ConnectionState.Ready || SystemInfo is null)
        {
            throw new InvalidOperationException(
                $"device is not ready for firmware update entry (state={State})");
        }

        if (policy == FirmwareUpdateAccessPolicy.PublishedOnly &&
            (SystemInfo.Capabilities & SystemCapability.FirmwareUpdate) == 0)
        {
            throw new DeusHostException(
                HostErrorKind.IncompatibleProtocol,
                "firmware update is not a published system capability on this device");
        }

        var response = await ExecuteProtocolOperationAsync(
            () => _firmwareUpdate.EnterBootloaderAsync(cancellationToken));

        if (response.Status != FirmwareUpdateStatus.Ok ||
            response.State != FirmwareUpdateState.Resetting)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"runtime update entry returned {response.Status}/{response.State}");
        }

        State = ConnectionState.Recovering;
        return response;
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
        return await ExecuteProtocolOperationAsync(
            () => _asset.AssetStatusAsync(policy, cancellationToken));
    }

    public async Task<AssetReadResult?> ReadOledUiLayoutAsync(
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        EnsureAssetTransferAvailable(policy);
        return await ExecuteProtocolOperationAsync(
            () => _asset.ReadOledUiLayoutAsync(policy, cancellationToken));
    }

    public async Task<AssetCommitResult> WriteOledUiLayoutAsync(
        OledUiLayoutConfigV1 configuration,
        AssetAccessPolicy policy = AssetAccessPolicy.PublishedOnly,
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();
        EnsureAssetTransferAvailable(policy);
        return await ExecuteProtocolOperationAsync(
            () => _asset.WriteOledUiLayoutAsync(
                configuration,
                policy,
                cancellationToken));
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        State = ConnectionState.Disconnected;
        await _channel.DisposeAsync();
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

        var forbidden = SystemCapability.NetworkServices;

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
        _channel.ResetProtocolState();
        _asset.ResetSessionState();
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

    private async Task<T> ExecuteProtocolOperationAsync<T>(
        Func<Task<T>> operation)
    {
        try
        {
            return await operation();
        }
        catch (DeusHostException exception)
            when (exception.Kind == HostErrorKind.TransportDisconnected)
        {
            State = ConnectionState.Recovering;
            ResetSessionState();
            throw;
        }
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
