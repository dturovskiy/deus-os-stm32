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
