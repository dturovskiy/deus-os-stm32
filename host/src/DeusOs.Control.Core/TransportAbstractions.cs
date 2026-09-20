namespace DeusOs.Control.Core;

public sealed record DeviceCandidate(
    string Locator,
    string DisplayName,
    string Platform);

public interface IDeviceTransport : IAsyncDisposable
{
    string Locator { get; }

    ValueTask WriteAsync(
        ReadOnlyMemory<byte> data,
        CancellationToken cancellationToken);

    ValueTask<int> ReadAsync(
        Memory<byte> buffer,
        CancellationToken cancellationToken);
}

public interface IDeviceDiscovery
{
    ValueTask<IReadOnlyList<DeviceCandidate>> DiscoverAsync(
        CancellationToken cancellationToken);

    ValueTask<IDeviceTransport> OpenAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken);
}
