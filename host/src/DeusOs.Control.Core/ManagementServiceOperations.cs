namespace DeusOs.Control.Core;

public enum ManagementServiceOperationAccess
{
    ReadOnly,
    Control,
    Destructive,
}

public enum ManagementServiceOperation
{
    Ping,
    Health,
    Applications,
    AssetStatus,
    ReadOledUiLayout,
    StartApplication,
    StopApplication,
    WriteOledUiLayout,
}

public sealed record ManagementServiceOperationDescriptor(
    ManagementServiceOperation Operation,
    ManagementServiceOperationAccess Access,
    SystemCapability RequiredCapability);

public static class ManagementServiceOperationCatalog
{
    private static readonly IReadOnlyList<ManagementServiceOperationDescriptor>
        OperationsValue = Array.AsReadOnly(
            new[]
            {
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.Ping,
                    ManagementServiceOperationAccess.ReadOnly,
                    SystemCapability.None),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.Health,
                    ManagementServiceOperationAccess.ReadOnly,
                    SystemCapability.SystemHealth),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.Applications,
                    ManagementServiceOperationAccess.ReadOnly,
                    SystemCapability.ApplicationRuntime),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.AssetStatus,
                    ManagementServiceOperationAccess.ReadOnly,
                    SystemCapability.AssetConfigurationTransfer),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.ReadOledUiLayout,
                    ManagementServiceOperationAccess.ReadOnly,
                    SystemCapability.AssetConfigurationTransfer),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.StartApplication,
                    ManagementServiceOperationAccess.Control,
                    SystemCapability.ApplicationControl),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.StopApplication,
                    ManagementServiceOperationAccess.Control,
                    SystemCapability.ApplicationControl),
                new ManagementServiceOperationDescriptor(
                    ManagementServiceOperation.WriteOledUiLayout,
                    ManagementServiceOperationAccess.Control,
                    SystemCapability.AssetConfigurationTransfer),
            });

    public static IReadOnlyList<ManagementServiceOperationDescriptor> Operations =>
        OperationsValue;

    public static bool TryGet(
        ManagementServiceOperation operation,
        out ManagementServiceOperationDescriptor? descriptor)
    {
        foreach (var candidate in OperationsValue)
        {
            if (candidate.Operation == operation)
            {
                descriptor = candidate;
                return true;
            }
        }

        descriptor = null;
        return false;
    }

    public static ManagementServiceOperationDescriptor Get(
        ManagementServiceOperation operation)
    {
        if (TryGet(operation, out var descriptor) && descriptor is not null)
        {
            return descriptor;
        }

        throw new ArgumentOutOfRangeException(
            nameof(operation),
            operation,
            "operation is not part of the service-v1 allowlist");
    }
}

public sealed class ManagementServiceOperations
{
    private readonly DeusDeviceSession _session;

    public ManagementServiceOperations(DeusDeviceSession session)
    {
        _session = session ?? throw new ArgumentNullException(nameof(session));
    }

    public ConnectionState State => _session.State;

    public NegotiationResult? LastNegotiation => _session.LastNegotiation;

    public Task<RpcResult> PingAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.PingAsync(token),
            cancellationToken);

    public Task<RpcResult> HealthAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.HealthAsync(token),
            cancellationToken);

    public Task<ApplicationSnapshot> ApplicationsAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.ApplicationsAsync(token),
            cancellationToken);

    public Task<AssetStatusSnapshot> AssetStatusAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.AssetStatusAsync(
                AssetAccessPolicy.PublishedOnly,
                token),
            cancellationToken);

    public Task<AssetReadResult?> ReadOledUiLayoutAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.ReadOledUiLayoutAsync(
                AssetAccessPolicy.PublishedOnly,
                token),
            cancellationToken);

    public Task<RpcResult> StartApplicationAsync(
        ushort applicationId,
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.StartApplicationAsync(
                applicationId,
                token),
            cancellationToken);

    public Task<RpcResult> StopApplicationAsync(
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.StopApplicationAsync(token),
            cancellationToken);

    public Task<AssetCommitResult> WriteOledUiLayoutAsync(
        OledUiLayoutConfigV1 configuration,
        CancellationToken cancellationToken = default) =>
        _session.ExecuteAsync(
            (client, token) => client.WriteOledUiLayoutAsync(
                configuration,
                AssetAccessPolicy.PublishedOnly,
                token),
            cancellationToken);
}
