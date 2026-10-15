using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;

namespace DeusOs.Control.Desktop;

internal sealed class DesktopController : IAsyncDisposable
{
    private readonly IDeviceDiscovery _discovery;
    private readonly DeusDeviceSession _session;
    private readonly ManagementServiceOperations _management;

    public DesktopController()
    {
        _discovery = OperatingSystem.IsWindows()
            ? new WindowsWinUsbDiscovery()
            : OperatingSystem.IsLinux()
                ? new LinuxLibUsbDiscovery()
                : throw new PlatformNotSupportedException(
                    "Deus OS CP supports Windows and Linux");

        _session = new DeusDeviceSession(_discovery);
        _management = new ManagementServiceOperations(_session);
    }

    public ConnectionState State => _session.State;

    public NegotiationResult? LastNegotiation => _session.LastNegotiation;

    public event Action<ConnectionState>? StateChanged
    {
        add => _session.StateChanged += value;
        remove => _session.StateChanged -= value;
    }

    public async Task<IReadOnlyList<DeviceCandidate>> DiscoverAsync(
        CancellationToken cancellationToken) =>
        await _discovery.DiscoverAsync(cancellationToken);

    public Task<NegotiationResult> ConnectAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken) =>
        _session.ConnectAsync(candidate, cancellationToken);

    public Task<PingStatus> PingAsync(CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.PingStatusAsync(token),
            cancellationToken);

    public Task<HealthSnapshot> HealthAsync(CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.HealthSnapshotAsync(token),
            cancellationToken);

    public Task<ApplicationSnapshot> ApplicationsAsync(
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.ApplicationsAsync(token),
            cancellationToken);

    public Task<AssetStatusSnapshot> AssetStatusAsync(
        CancellationToken cancellationToken) =>
        _management.AssetStatusAsync(cancellationToken);

    public Task<AssetReadResult?> ReadOledUiLayoutAsync(
        CancellationToken cancellationToken) =>
        _management.ReadOledUiLayoutAsync(cancellationToken);

    public Task<ApplicationStartResult> StartApplicationAsync(
        ushort applicationId,
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) =>
                client.StartApplicationControlAsync(applicationId, token),
            cancellationToken);

    public Task<ApplicationStopResult> StopApplicationAsync(
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.StopApplicationControlAsync(token),
            cancellationToken);

    public Task DisconnectAsync() =>
        _session.DisconnectAsync();

    public ValueTask DisposeAsync() =>
        _session.DisposeAsync();
}
