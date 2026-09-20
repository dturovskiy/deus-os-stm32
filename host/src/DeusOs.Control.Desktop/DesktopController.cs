using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;

namespace DeusOs.Control.Desktop;

internal sealed class DesktopController : IAsyncDisposable
{
    private readonly IDeviceDiscovery _discovery;
    private readonly DeusDeviceSession _session;

    public DesktopController()
    {
        _discovery = OperatingSystem.IsWindows()
            ? new WindowsWinUsbDiscovery()
            : OperatingSystem.IsLinux()
                ? new LinuxLibUsbDiscovery()
                : throw new PlatformNotSupportedException(
                    "Deus OS CP supports Windows and Linux");

        _session = new DeusDeviceSession(_discovery);
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

    public Task<RpcResult> PingAsync(CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.PingAsync(token),
            cancellationToken);

    public Task<RpcResult> HealthAsync(CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.HealthAsync(token),
            cancellationToken);

    public Task<ApplicationSnapshot> ApplicationsAsync(
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.ApplicationsAsync(token),
            cancellationToken);

    public Task<RpcResult> StartApplicationAsync(
        ushort applicationId,
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) =>
                client.StartApplicationAsync(applicationId, token),
            cancellationToken);

    public Task<RpcResult> StopApplicationAsync(
        CancellationToken cancellationToken) =>
        _session.ExecuteAsync(
            (client, token) => client.StopApplicationAsync(token),
            cancellationToken);

    public Task DisconnectAsync() =>
        _session.DisconnectAsync();

    public ValueTask DisposeAsync() =>
        _session.DisposeAsync();
}
