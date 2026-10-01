using DeusOs.Control.Core;

namespace DeusOs.Control.Transport.Linux;

public sealed class LinuxBootloaderLibUsbDiscovery : IDeviceDiscovery
{
    public const ushort VendorId = 0x1209;
    public const ushort ProductId = 0x000D;
    public const int InterfaceNumber = 0;
    public const byte OutEndpoint = 0x01;
    public const byte InEndpoint = 0x81;

    internal static readonly LinuxUsbProfile BootloaderProfile = new(
        VendorId,
        ProductId,
        InterfaceNumber,
        OutEndpoint,
        InEndpoint,
        "Deus OS Bootloader");

    public ValueTask<IReadOnlyList<DeviceCandidate>> DiscoverAsync(
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();

        if (!OperatingSystem.IsLinux())
        {
            return ValueTask.FromResult<IReadOnlyList<DeviceCandidate>>(
                Array.Empty<DeviceCandidate>());
        }

        try
        {
            var candidates = LinuxLibUsbNative.EnumerateCandidates(
                BootloaderProfile);
            return ValueTask.FromResult<IReadOnlyList<DeviceCandidate>>(
                candidates);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Discovery,
                "Linux bootloader libusb discovery failed",
                exception);
        }
    }

    public ValueTask<IDeviceTransport> OpenAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(candidate);
        cancellationToken.ThrowIfCancellationRequested();

        if (!OperatingSystem.IsLinux())
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "Linux bootloader libusb transport is unavailable on this platform");
        }

        IDeviceTransport transport = LinuxLibUsbTransport.Open(
            candidate.Locator,
            BootloaderProfile);
        return ValueTask.FromResult(transport);
    }
}
