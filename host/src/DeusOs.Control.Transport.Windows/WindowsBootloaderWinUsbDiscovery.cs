using DeusOs.Control.Core;

namespace DeusOs.Control.Transport.Windows;

public sealed class WindowsBootloaderWinUsbDiscovery : IDeviceDiscovery
{
    public static readonly Guid BootloaderInterfaceGuid =
        new("F08907B7-BEC4-5FCF-BC4C-B446ED345D87");

    public const byte InterfaceNumber = 0;
    public const byte OutEndpoint = 0x01;
    public const byte InEndpoint = 0x81;

    public ValueTask<IReadOnlyList<DeviceCandidate>> DiscoverAsync(
        CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();

        if (!OperatingSystem.IsWindows())
        {
            return ValueTask.FromResult<IReadOnlyList<DeviceCandidate>>(
                Array.Empty<DeviceCandidate>());
        }

        try
        {
            IReadOnlyList<DeviceCandidate> candidates =
                WindowsWinUsbDiscovery
                    .EnumerateInterfacePaths(BootloaderInterfaceGuid)
                    .Select(path => new DeviceCandidate(
                        path,
                        "Deus OS Bootloader",
                        "Windows"))
                    .ToArray();
            return ValueTask.FromResult(candidates);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Discovery,
                "Windows bootloader WinUSB discovery failed",
                exception);
        }
    }

    public ValueTask<IDeviceTransport> OpenAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(candidate);
        cancellationToken.ThrowIfCancellationRequested();

        if (!OperatingSystem.IsWindows())
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "Windows bootloader WinUSB transport is unavailable on this platform");
        }

        try
        {
            IDeviceTransport transport = WindowsWinUsbTransport.Open(
                candidate.Locator,
                InterfaceNumber,
                OutEndpoint,
                InEndpoint);
            return ValueTask.FromResult(transport);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                $"failed to open bootloader WinUSB device '{candidate.Locator}'",
                exception);
        }
    }
}
