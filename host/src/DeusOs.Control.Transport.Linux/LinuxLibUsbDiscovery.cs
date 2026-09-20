using System.Runtime.InteropServices;
using DeusOs.Control.Core;

namespace DeusOs.Control.Transport.Linux;

public sealed class LinuxLibUsbDiscovery : IDeviceDiscovery
{
    public const ushort VendorId = 0x1209;
    public const ushort ProductId = 0x000C;
    public const int InterfaceNumber = 2;
    public const byte OutEndpoint = 0x04;
    public const byte InEndpoint = 0x84;

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
            var candidates = LinuxLibUsbNative.EnumerateCandidates();
            return ValueTask.FromResult<IReadOnlyList<DeviceCandidate>>(candidates);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Discovery,
                "Linux libusb discovery failed",
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
                "Linux libusb transport is unavailable on this platform");
        }

        IDeviceTransport transport = LinuxLibUsbTransport.Open(candidate.Locator);
        return ValueTask.FromResult(transport);
    }
}

internal static class LinuxLibUsbNative
{
    internal const ushort ExpectedVendorId = LinuxLibUsbDiscovery.VendorId;
    internal const ushort ExpectedProductId = LinuxLibUsbDiscovery.ProductId;
    internal const int ManagementInterface = LinuxLibUsbDiscovery.InterfaceNumber;
    internal const byte OutEndpoint = LinuxLibUsbDiscovery.OutEndpoint;
    internal const byte InEndpoint = LinuxLibUsbDiscovery.InEndpoint;
    internal const int ErrorNoDevice = -4;
    internal const int ErrorTimeout = -7;

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct DeviceDescriptor
    {
        public byte Length;
        public byte DescriptorType;
        public ushort BcdUsb;
        public byte DeviceClass;
        public byte DeviceSubClass;
        public byte DeviceProtocol;
        public byte MaxPacketSize0;
        public ushort VendorId;
        public ushort ProductId;
        public ushort BcdDevice;
        public byte ManufacturerIndex;
        public byte ProductIndex;
        public byte SerialNumberIndex;
        public byte NumConfigurations;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct ConfigDescriptor
    {
        public byte Length;
        public byte DescriptorType;
        public ushort TotalLength;
        public byte NumInterfaces;
        public byte ConfigurationValue;
        public byte ConfigurationIndex;
        public byte Attributes;
        public byte MaxPower;
        public IntPtr Interfaces;
        public IntPtr Extra;
        public int ExtraLength;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct UsbInterface
    {
        public IntPtr AlternateSettings;
        public int NumAlternateSettings;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct InterfaceDescriptor
    {
        public byte Length;
        public byte DescriptorType;
        public byte InterfaceNumber;
        public byte AlternateSetting;
        public byte NumEndpoints;
        public byte InterfaceClass;
        public byte InterfaceSubClass;
        public byte InterfaceProtocol;
        public byte InterfaceIndex;
        public IntPtr Endpoints;
        public IntPtr Extra;
        public int ExtraLength;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct EndpointDescriptor
    {
        public byte Length;
        public byte DescriptorType;
        public byte EndpointAddress;
        public byte Attributes;
        public ushort MaxPacketSize;
        public byte Interval;
        public byte Refresh;
        public byte SynchAddress;
        public IntPtr Extra;
        public int ExtraLength;
    }

    internal static IReadOnlyList<DeviceCandidate> EnumerateCandidates()
    {
        Check(libusb_init(out var context), "libusb_init", HostErrorKind.Discovery);
        try
        {
            var count = libusb_get_device_list(context, out var list);
            if (count < 0)
            {
                throw CreateException(
                    checked((int)count),
                    "libusb_get_device_list",
                    HostErrorKind.Discovery);
            }

            try
            {
                var candidates = new List<DeviceCandidate>();

                for (nint index = 0; index < count; ++index)
                {
                    var device = Marshal.ReadIntPtr(
                        list,
                        checked((int)(index * IntPtr.Size)));

                    if (libusb_get_device_descriptor(
                            device,
                            out var descriptor) != 0 ||
                        descriptor.VendorId != ExpectedVendorId ||
                        descriptor.ProductId != ExpectedProductId)
                    {
                        continue;
                    }

                    var locator = FormatLocator(device);
                    candidates.Add(new DeviceCandidate(
                        locator,
                        "Deus OS Device",
                        "Linux"));
                }

                return candidates;
            }
            finally
            {
                libusb_free_device_list(list, 1);
            }
        }
        finally
        {
            libusb_exit(context);
        }
    }

    internal static string FormatLocator(IntPtr device)
    {
        var bus = libusb_get_bus_number(device);
        Span<byte> ports = stackalloc byte[8];
        var managedPorts = ports.ToArray();
        var portCount = libusb_get_port_numbers(
            device,
            managedPorts,
            managedPorts.Length);

        var locatorPorts = portCount > 0
            ? managedPorts.Take(portCount).ToArray()
            : Array.Empty<byte>();

        return FormatPortLocator(bus, locatorPorts);
    }

    internal static string FormatPortLocator(
        byte bus,
        IReadOnlyList<byte> ports)
    {
        var path = ports.Count > 0
            ? string.Join(".", ports)
            : "root";

        return $"usb:{bus:D3}:{path}";
    }

    internal static void ValidateManagementTopology(IntPtr device)
    {
        Check(
            libusb_get_active_config_descriptor(device, out var configPointer),
            "libusb_get_active_config_descriptor",
            HostErrorKind.Open);

        try
        {
            var config = Marshal.PtrToStructure<ConfigDescriptor>(configPointer);
            var interfaceSize = Marshal.SizeOf<UsbInterface>();
            var interfaceDescriptorSize = Marshal.SizeOf<InterfaceDescriptor>();
            var endpointSize = Marshal.SizeOf<EndpointDescriptor>();

            var found = false;

            for (var interfaceIndex = 0;
                 interfaceIndex < config.NumInterfaces;
                 ++interfaceIndex)
            {
                var usbInterface = Marshal.PtrToStructure<UsbInterface>(
                    IntPtr.Add(
                        config.Interfaces,
                        interfaceIndex * interfaceSize));

                for (var altIndex = 0;
                     altIndex < usbInterface.NumAlternateSettings;
                     ++altIndex)
                {
                    var descriptor = Marshal.PtrToStructure<InterfaceDescriptor>(
                        IntPtr.Add(
                            usbInterface.AlternateSettings,
                            altIndex * interfaceDescriptorSize));

                    if (descriptor.InterfaceNumber != ManagementInterface ||
                        descriptor.AlternateSetting != 0)
                    {
                        continue;
                    }

                    if (descriptor.InterfaceClass != 0xFF ||
                        descriptor.InterfaceSubClass != 0 ||
                        descriptor.InterfaceProtocol != 0 ||
                        descriptor.NumEndpoints != 2)
                    {
                        throw new DeusHostException(
                            HostErrorKind.Open,
                            "Linux management interface is not IF2 FF/00/00 with two endpoints");
                    }

                    var sawOut = false;
                    var sawIn = false;

                    for (var endpointIndex = 0;
                         endpointIndex < descriptor.NumEndpoints;
                         ++endpointIndex)
                    {
                        var endpoint = Marshal.PtrToStructure<EndpointDescriptor>(
                            IntPtr.Add(
                                descriptor.Endpoints,
                                endpointIndex * endpointSize));

                        var transferType = endpoint.Attributes & 0x03;
                        if (transferType != 0x02 ||
                            endpoint.MaxPacketSize != 64)
                        {
                            throw new DeusHostException(
                                HostErrorKind.Open,
                                $"unexpected endpoint 0x{endpoint.EndpointAddress:X2} type/packet");
                        }

                        sawOut |= endpoint.EndpointAddress == OutEndpoint;
                        sawIn |= endpoint.EndpointAddress == InEndpoint;
                    }

                    if (!sawOut || !sawIn)
                    {
                        throw new DeusHostException(
                            HostErrorKind.Open,
                            "required EP4 OUT/IN endpoints were not found");
                    }

                    found = true;
                }
            }

            if (!found)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    "management interface 2 was not found");
            }
        }
        finally
        {
            libusb_free_config_descriptor(configPointer);
        }
    }

    internal static DeusHostException CreateException(
        int code,
        string operation,
        HostErrorKind fallbackKind)
    {
        var kind = code == ErrorNoDevice
            ? HostErrorKind.TransportDisconnected
            : code == ErrorTimeout
                ? HostErrorKind.Timeout
                : fallbackKind;

        var namePointer = libusb_error_name(code);
        var name = namePointer == IntPtr.Zero
            ? $"libusb error {code}"
            : Marshal.PtrToStringAnsi(namePointer) ?? $"libusb error {code}";

        return new DeusHostException(kind, $"{operation} failed: {name} ({code})");
    }

    internal static void Check(
        int result,
        string operation,
        HostErrorKind fallbackKind)
    {
        if (result < 0)
        {
            throw CreateException(result, operation, fallbackKind);
        }
    }

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_init(out IntPtr context);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void libusb_exit(IntPtr context);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern nint libusb_get_device_list(
        IntPtr context,
        out IntPtr list);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void libusb_free_device_list(
        IntPtr list,
        int unrefDevices);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_get_device_descriptor(
        IntPtr device,
        out DeviceDescriptor descriptor);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern byte libusb_get_bus_number(IntPtr device);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern byte libusb_get_device_address(IntPtr device);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_get_port_numbers(
        IntPtr device,
        [Out] byte[] portNumbers,
        int portNumbersLength);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_get_active_config_descriptor(
        IntPtr device,
        out IntPtr configDescriptor);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void libusb_free_config_descriptor(
        IntPtr configDescriptor);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_open(
        IntPtr device,
        out IntPtr deviceHandle);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void libusb_close(IntPtr deviceHandle);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_kernel_driver_active(
        IntPtr deviceHandle,
        int interfaceNumber);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_claim_interface(
        IntPtr deviceHandle,
        int interfaceNumber);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_release_interface(
        IntPtr deviceHandle,
        int interfaceNumber);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int libusb_bulk_transfer(
        IntPtr deviceHandle,
        byte endpoint,
        byte[] data,
        int length,
        out int transferred,
        uint timeout);

    [DllImport("libusb-1.0.so.0", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr libusb_error_name(int errorCode);
}
