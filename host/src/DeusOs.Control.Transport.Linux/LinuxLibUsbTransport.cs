using System.Runtime.InteropServices;
using DeusOs.Control.Core;

namespace DeusOs.Control.Transport.Linux;

internal sealed class LinuxLibUsbTransport : IDeviceTransport
{
    private const uint IoTimeoutMilliseconds = 2000;

    private readonly IntPtr _context;
    private readonly IntPtr _handle;
    private bool _claimed;
    private bool _disposed;

    private LinuxLibUsbTransport(
        string locator,
        IntPtr context,
        IntPtr handle)
    {
        Locator = locator;
        _context = context;
        _handle = handle;
        _claimed = true;
    }

    public string Locator { get; }

    public static LinuxLibUsbTransport Open(string locator)
    {
        LinuxLibUsbNative.Check(
            LinuxLibUsbNative.libusb_init(out var context),
            "libusb_init",
            HostErrorKind.Open);

        IntPtr handle = IntPtr.Zero;
        IntPtr list = IntPtr.Zero;
        var claimed = false;

        try
        {
            var count = LinuxLibUsbNative.libusb_get_device_list(
                context,
                out list);
            if (count < 0)
            {
                throw LinuxLibUsbNative.CreateException(
                    checked((int)count),
                    "libusb_get_device_list",
                    HostErrorKind.Open);
            }

            IntPtr matchedDevice = IntPtr.Zero;

            for (nint index = 0; index < count; ++index)
            {
                var device = Marshal.ReadIntPtr(
                    list,
                    checked((int)(index * IntPtr.Size)));

                if (LinuxLibUsbNative.libusb_get_device_descriptor(
                        device,
                        out var descriptor) != 0 ||
                    descriptor.VendorId != LinuxLibUsbNative.ExpectedVendorId ||
                    descriptor.ProductId != LinuxLibUsbNative.ExpectedProductId)
                {
                    continue;
                }

                if (LinuxLibUsbNative.FormatLocator(device) == locator)
                {
                    matchedDevice = device;
                    break;
                }
            }

            if (matchedDevice == IntPtr.Zero)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    $"Linux device locator '{locator}' is no longer present");
            }

            LinuxLibUsbNative.ValidateManagementTopology(matchedDevice);

            LinuxLibUsbNative.Check(
                LinuxLibUsbNative.libusb_open(matchedDevice, out handle),
                "libusb_open",
                HostErrorKind.Open);

            var kernelDriver = LinuxLibUsbNative.libusb_kernel_driver_active(
                handle,
                LinuxLibUsbNative.ManagementInterface);

            if (kernelDriver == 1)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    "kernel driver is bound to management IF2; automatic detach is forbidden");
            }

            if (kernelDriver < 0 &&
                kernelDriver != LinuxLibUsbNative.ErrorNoDevice)
            {
                throw LinuxLibUsbNative.CreateException(
                    kernelDriver,
                    "libusb_kernel_driver_active",
                    HostErrorKind.Open);
            }

            LinuxLibUsbNative.Check(
                LinuxLibUsbNative.libusb_claim_interface(
                    handle,
                    LinuxLibUsbNative.ManagementInterface),
                "libusb_claim_interface(IF2)",
                HostErrorKind.Open);
            claimed = true;

            return new LinuxLibUsbTransport(locator, context, handle);
        }
        catch
        {
            if (claimed && handle != IntPtr.Zero)
            {
                _ = LinuxLibUsbNative.libusb_release_interface(
                    handle,
                    LinuxLibUsbNative.ManagementInterface);
            }

            if (handle != IntPtr.Zero)
            {
                LinuxLibUsbNative.libusb_close(handle);
            }

            LinuxLibUsbNative.libusb_exit(context);
            throw;
        }
        finally
        {
            if (list != IntPtr.Zero)
            {
                LinuxLibUsbNative.libusb_free_device_list(list, 1);
            }
        }
    }

    public async ValueTask WriteAsync(
        ReadOnlyMemory<byte> data,
        CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        cancellationToken.ThrowIfCancellationRequested();

        var bytes = data.ToArray();
        var transferred = await Task.Run(
            () =>
            {
                var result = LinuxLibUsbNative.libusb_bulk_transfer(
                    _handle,
                    LinuxLibUsbNative.OutEndpoint,
                    bytes,
                    bytes.Length,
                    out var count,
                    IoTimeoutMilliseconds);

                LinuxLibUsbNative.Check(
                    result,
                    "libusb_bulk_transfer(OUT)",
                    HostErrorKind.TransportDisconnected);
                return count;
            },
            cancellationToken);

        if (transferred != bytes.Length)
        {
            throw new DeusHostException(
                HostErrorKind.TransportDisconnected,
                $"short libusb write {transferred}/{bytes.Length}");
        }
    }

    public async ValueTask<int> ReadAsync(
        Memory<byte> buffer,
        CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        cancellationToken.ThrowIfCancellationRequested();

        var bytes = new byte[buffer.Length];

        var transferred = await Task.Run(
            () =>
            {
                var result = LinuxLibUsbNative.libusb_bulk_transfer(
                    _handle,
                    LinuxLibUsbNative.InEndpoint,
                    bytes,
                    bytes.Length,
                    out var count,
                    IoTimeoutMilliseconds);

                LinuxLibUsbNative.Check(
                    result,
                    "libusb_bulk_transfer(IN)",
                    HostErrorKind.TransportDisconnected);
                return count;
            },
            cancellationToken);

        bytes.AsSpan(0, transferred).CopyTo(buffer.Span);
        return transferred;
    }

    public ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return ValueTask.CompletedTask;
        }

        _disposed = true;

        if (_claimed)
        {
            _ = LinuxLibUsbNative.libusb_release_interface(
                _handle,
                LinuxLibUsbNative.ManagementInterface);
            _claimed = false;
        }

        LinuxLibUsbNative.libusb_close(_handle);
        LinuxLibUsbNative.libusb_exit(_context);

        return ValueTask.CompletedTask;
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
