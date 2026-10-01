using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;
using Xunit;

namespace DeusOs.Control.Transport.Tests;

public sealed class TransportContractTests
{
    [Fact]
    public void WindowsManagementGuidMatchesFirmwareContract()
    {
        Assert.Equal(
            new Guid("C8B05EDE-1683-5002-81F0-95636B89CEC6"),
            WindowsWinUsbDiscovery.ManagementInterfaceGuid);
    }

    [Fact]
    public void LinuxTopologyConstantsMatchFirmwareContract()
    {
        Assert.Equal((ushort)0x1209, LinuxLibUsbDiscovery.VendorId);
        Assert.Equal((ushort)0x000C, LinuxLibUsbDiscovery.ProductId);
        Assert.Equal(2, LinuxLibUsbDiscovery.InterfaceNumber);
        Assert.Equal((byte)0x04, LinuxLibUsbDiscovery.OutEndpoint);
        Assert.Equal((byte)0x84, LinuxLibUsbDiscovery.InEndpoint);
    }

    [Fact]
    public void LinuxLocatorUsesBusAndPhysicalPortPathOnly()
    {
        Assert.Equal(
            "usb:001:8",
            LinuxLibUsbNative.FormatPortLocator(1, new byte[] { 8 }));
        Assert.Equal(
            "usb:002:3.5.1",
            LinuxLibUsbNative.FormatPortLocator(2, new byte[] { 3, 5, 1 }));
        Assert.Equal(
            "usb:001:root",
            LinuxLibUsbNative.FormatPortLocator(1, Array.Empty<byte>()));
    }

    [Fact]
    public void BootloaderTransportConstantsMatchFirmwareContract()
    {
        Assert.Equal(
            new Guid("F08907B7-BEC4-5FCF-BC4C-B446ED345D87"),
            WindowsBootloaderWinUsbDiscovery.BootloaderInterfaceGuid);
        Assert.Equal((ushort)0x1209, LinuxBootloaderLibUsbDiscovery.VendorId);
        Assert.Equal((ushort)0x000D, LinuxBootloaderLibUsbDiscovery.ProductId);
        Assert.Equal(0, LinuxBootloaderLibUsbDiscovery.InterfaceNumber);
        Assert.Equal((byte)0x01, LinuxBootloaderLibUsbDiscovery.OutEndpoint);
        Assert.Equal((byte)0x81, LinuxBootloaderLibUsbDiscovery.InEndpoint);
    }

    [Theory]
    [InlineData(121)]
    [InlineData(1460)]
    public void WindowsIoTimeoutErrorsMapToTimeout(int error)
    {
        Assert.Equal(
            HostErrorKind.Timeout,
            InvokeWindowsIoErrorMapping(
                error,
                HostErrorKind.TransportDisconnected));
    }

    [Fact]
    public void WindowsTimeoutCodeDoesNotRewriteOpenFailure()
    {
        Assert.Equal(
            HostErrorKind.Open,
            InvokeWindowsIoErrorMapping(
                121,
                HostErrorKind.Open));
    }

    [Fact]
    public void WindowsNonTimeoutIoErrorPreservesFallbackKind()
    {
        Assert.Equal(
            HostErrorKind.Open,
            InvokeWindowsIoErrorMapping(
                5,
                HostErrorKind.Open));
    }

    [Fact]
    public async Task NonWindowsBootloaderDiscoveryIsInert()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var devices = await new WindowsBootloaderWinUsbDiscovery()
            .DiscoverAsync(CancellationToken.None);

        Assert.Empty(devices);
    }

    [Fact]
    public async Task NonWindowsDiscoveryIsInert()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var devices = await new WindowsWinUsbDiscovery()
            .DiscoverAsync(CancellationToken.None);

        Assert.Empty(devices);
    }

    [Fact]
    public async Task NonLinuxBootloaderDiscoveryIsInert()
    {
        if (OperatingSystem.IsLinux())
        {
            return;
        }

        var devices = await new LinuxBootloaderLibUsbDiscovery()
            .DiscoverAsync(CancellationToken.None);

        Assert.Empty(devices);
    }

    private static HostErrorKind InvokeWindowsIoErrorMapping(
        int error,
        HostErrorKind fallbackKind)
    {
        var transportType = typeof(WindowsBootloaderWinUsbDiscovery)
            .Assembly
            .GetType(
                "DeusOs.Control.Transport.Windows.WindowsWinUsbTransport",
                throwOnError: true)!;
        var method = transportType.GetMethod(
            "MapIoErrorKind",
            System.Reflection.BindingFlags.Static |
            System.Reflection.BindingFlags.NonPublic)!;

        return Assert.IsType<HostErrorKind>(
            method.Invoke(
                null,
                new object[] { error, fallbackKind }));
    }

    [Fact]
    public async Task NonLinuxDiscoveryIsInert()
    {
        if (OperatingSystem.IsLinux())
        {
            return;
        }

        var devices = await new LinuxLibUsbDiscovery()
            .DiscoverAsync(CancellationToken.None);

        Assert.Empty(devices);
    }
}
