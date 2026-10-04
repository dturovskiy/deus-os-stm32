using System.Runtime.InteropServices;
using DeusOs.Control.Core;

namespace DeusOs.Control.Transport.Linux;

internal sealed class LinuxNativeIoLifetime
{
    private readonly object _sync = new();
    private readonly SemaphoreSlim _operationGate = new(1, 1);
    private bool _closing;
    private Task? _disposeTask;

    internal async Task<T> RunAsync<T>(
        Func<T> nativeOperation,
        CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(nativeOperation);
        cancellationToken.ThrowIfCancellationRequested();
        ThrowIfClosing();

        await _operationGate.WaitAsync(cancellationToken);
        try
        {
            ThrowIfClosing();
            cancellationToken.ThrowIfCancellationRequested();

            T result = default!;
            Exception? nativeException = null;
            try
            {
                result = await Task.Run(
                    () =>
                    {
                        cancellationToken.ThrowIfCancellationRequested();
                        return nativeOperation();
                    });
            }
            catch (Exception exception)
            {
                nativeException = exception;
            }

            cancellationToken.ThrowIfCancellationRequested();

            if (nativeException is not null)
            {
                System.Runtime.ExceptionServices.ExceptionDispatchInfo
                    .Capture(nativeException)
                    .Throw();
            }

            return result;
        }
        finally
        {
            _operationGate.Release();
        }
    }

    internal ValueTask DisposeAsync(Action closeNativeResources)
    {
        ArgumentNullException.ThrowIfNull(closeNativeResources);

        Task disposeTask;
        lock (_sync)
        {
            if (_disposeTask is null)
            {
                _closing = true;
                _disposeTask = DisposeCoreAsync(closeNativeResources);
            }

            disposeTask = _disposeTask;
        }

        return new ValueTask(disposeTask);
    }

    private async Task DisposeCoreAsync(Action closeNativeResources)
    {
        await _operationGate.WaitAsync();
        try
        {
            closeNativeResources();
        }
        finally
        {
            _operationGate.Release();
        }
    }

    private void ThrowIfClosing()
    {
        lock (_sync)
        {
            ObjectDisposedException.ThrowIf(_closing, this);
        }
    }
}

internal sealed class LinuxLibUsbTransport : IDeviceTransport
{
    private const uint IoTimeoutMilliseconds = 2000;

    private readonly IntPtr _context;
    private readonly IntPtr _handle;
    private readonly LinuxUsbProfile _profile;
    private readonly LinuxNativeIoLifetime _lifetime = new();
    private bool _claimed;

    private LinuxLibUsbTransport(
        string locator,
        IntPtr context,
        IntPtr handle,
        LinuxUsbProfile profile)
    {
        Locator = locator;
        _context = context;
        _handle = handle;
        _profile = profile;
        _claimed = true;
    }

    public string Locator { get; }

    public static LinuxLibUsbTransport Open(string locator) =>
        Open(locator, LinuxLibUsbDiscovery.RuntimeProfile);

    internal static LinuxLibUsbTransport Open(
        string locator,
        LinuxUsbProfile profile)
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
                    descriptor.VendorId != profile.VendorId ||
                    descriptor.ProductId != profile.ProductId)
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

            LinuxLibUsbNative.ValidateTopology(matchedDevice, profile);

            LinuxLibUsbNative.Check(
                LinuxLibUsbNative.libusb_open(matchedDevice, out handle),
                "libusb_open",
                HostErrorKind.Open);

            var kernelDriver = LinuxLibUsbNative.libusb_kernel_driver_active(
                handle,
                profile.InterfaceNumber);

            if (kernelDriver == 1)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    $"kernel driver is bound to IF{profile.InterfaceNumber}; automatic detach is forbidden");
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
                    profile.InterfaceNumber),
                $"libusb_claim_interface(IF{profile.InterfaceNumber})",
                HostErrorKind.Open);
            claimed = true;

            return new LinuxLibUsbTransport(locator, context, handle, profile);
        }
        catch
        {
            if (claimed && handle != IntPtr.Zero)
            {
                _ = LinuxLibUsbNative.libusb_release_interface(
                    handle,
                    profile.InterfaceNumber);
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
        var bytes = data.ToArray();
        var transferred = await _lifetime.RunAsync(
            () =>
            {
                var result = LinuxLibUsbNative.libusb_bulk_transfer(
                    _handle,
                    _profile.OutEndpoint,
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
        var bytes = new byte[buffer.Length];

        var transferred = await _lifetime.RunAsync(
            () =>
            {
                var result = LinuxLibUsbNative.libusb_bulk_transfer(
                    _handle,
                    _profile.InEndpoint,
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

    public ValueTask DisposeAsync() =>
        _lifetime.DisposeAsync(
            () =>
            {
                if (_claimed)
                {
                    _ = LinuxLibUsbNative.libusb_release_interface(
                        _handle,
                        _profile.InterfaceNumber);
                    _claimed = false;
                }

                LinuxLibUsbNative.libusb_close(_handle);
                LinuxLibUsbNative.libusb_exit(_context);
            });
}
