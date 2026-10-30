using System.ComponentModel;
using System.Runtime.InteropServices;
using DeusOs.Control.Core;
using Microsoft.Win32.SafeHandles;

namespace DeusOs.Control.Transport.Windows;

internal sealed class WindowsNativeIoLifetime
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

internal sealed class WindowsWinUsbTransport : IDeviceTransport
{
    private const byte OutPipe = 0x04;
    private const byte InPipe = 0x84;
    private const uint PipeTransferTimeout = 0x03;
    private const uint IoTimeoutMilliseconds = 2000;
    private const int ErrorSemTimeout = 121;
    private const int ErrorTimeout = 1460;

    private readonly SafeFileHandle _fileHandle;
    private readonly SafeWinUsbHandle _winUsbHandle;
    private readonly byte _outPipe;
    private readonly byte _inPipe;
    private readonly WindowsNativeIoLifetime _lifetime = new();

    private WindowsWinUsbTransport(
        string locator,
        SafeFileHandle fileHandle,
        SafeWinUsbHandle winUsbHandle,
        byte outPipe,
        byte inPipe)
    {
        Locator = locator;
        _fileHandle = fileHandle;
        _winUsbHandle = winUsbHandle;
        _outPipe = outPipe;
        _inPipe = inPipe;
    }

    public string Locator { get; }

    public static WindowsWinUsbTransport Open(string locator) =>
        Open(locator, null, OutPipe, InPipe);

    internal static WindowsWinUsbTransport Open(
        string locator,
        byte interfaceNumber,
        byte outPipe,
        byte inPipe) =>
        Open(locator, (byte?)interfaceNumber, outPipe, inPipe);

    private static WindowsWinUsbTransport Open(
        string locator,
        byte? interfaceNumber,
        byte outPipe,
        byte inPipe)
    {
        var file = NativeMethods.CreateFileW(
            locator,
            NativeMethods.GenericRead | NativeMethods.GenericWrite,
            NativeMethods.FileShareRead | NativeMethods.FileShareWrite,
            IntPtr.Zero,
            NativeMethods.OpenExisting,
            NativeMethods.FileAttributeNormal | NativeMethods.FileFlagOverlapped,
            IntPtr.Zero);

        if (file.IsInvalid)
        {
            var error = Marshal.GetLastWin32Error();
            file.Dispose();
            throw new DeusHostException(
                HostErrorKind.Open,
                $"CreateFile failed ({error}): {new Win32Exception(error).Message}");
        }

        if (!NativeMethods.WinUsb_Initialize(file, out var rawWinUsb))
        {
            var error = Marshal.GetLastWin32Error();
            file.Dispose();
            throw new DeusHostException(
                HostErrorKind.Open,
                $"WinUsb_Initialize failed ({error}): {new Win32Exception(error).Message}");
        }

        var winUsb = new SafeWinUsbHandle(rawWinUsb);
        try
        {
            ValidateInterface(winUsb, interfaceNumber, outPipe, inPipe);
            SetTimeout(winUsb, outPipe);
            SetTimeout(winUsb, inPipe);
            return new WindowsWinUsbTransport(
                locator,
                file,
                winUsb,
                outPipe,
                inPipe);
        }
        catch
        {
            winUsb.Dispose();
            file.Dispose();
            throw;
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
                if (!NativeMethods.WinUsb_WritePipe(
                        _winUsbHandle,
                        _outPipe,
                        bytes,
                        checked((uint)bytes.Length),
                        out var count,
                        IntPtr.Zero))
                {
                    throw CreateIoException(
                        HostErrorKind.TransportDisconnected,
                        "WinUsb_WritePipe");
                }

                return count;
            },
            cancellationToken);

        if (transferred != bytes.Length)
        {
            throw new DeusHostException(
                HostErrorKind.TransportDisconnected,
                $"short WinUSB write {transferred}/{bytes.Length}");
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
                if (!NativeMethods.WinUsb_ReadPipe(
                        _winUsbHandle,
                        _inPipe,
                        bytes,
                        checked((uint)bytes.Length),
                        out var count,
                        IntPtr.Zero))
                {
                    throw CreateIoException(
                        HostErrorKind.TransportDisconnected,
                        "WinUsb_ReadPipe");
                }

                return count;
            },
            cancellationToken);

        bytes.AsSpan(0, checked((int)transferred)).CopyTo(buffer.Span);
        return checked((int)transferred);
    }

    public ValueTask DisposeAsync() =>
        _lifetime.DisposeAsync(
            () =>
            {
                _winUsbHandle.Dispose();
                _fileHandle.Dispose();
            });

    internal static bool IsSupportedRuntimeInterface(byte interfaceNumber) =>
        interfaceNumber is 0 or 2;

    private static void ValidateInterface(
        SafeWinUsbHandle handle,
        byte? interfaceNumber,
        byte outPipe,
        byte inPipe)
    {
        if (!NativeMethods.WinUsb_QueryInterfaceSettings(
                handle,
                0,
                out var descriptor))
        {
            throw CreateIoException(
                HostErrorKind.Open,
                "WinUsb_QueryInterfaceSettings");
        }

        if (descriptor.NumEndpoints != 2)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "WinUSB management interface must expose exactly two endpoints");
        }

        var pipes = new (byte Address, int Type, ushort MaxPacketSize)[2];
        for (byte index = 0; index < 2; ++index)
        {
            if (!NativeMethods.WinUsb_QueryPipe(handle, 0, index, out var pipe))
            {
                throw CreateIoException(HostErrorKind.Open, "WinUsb_QueryPipe");
            }

            pipes[index] = (
                pipe.PipeId,
                (int)pipe.PipeType,
                pipe.MaximumPacketSize);
        }

        ValidateTopologyDescriptor(
            interfaceNumber,
            descriptor.Length,
            descriptor.DescriptorType,
            descriptor.InterfaceNumber,
            descriptor.AlternateSetting,
            descriptor.InterfaceClass,
            descriptor.InterfaceSubClass,
            descriptor.InterfaceProtocol,
            pipes,
            outPipe,
            inPipe);
    }

    // Real WinUSB open and negative tests use the very same validator.
    // No native handle is opened when called from an offline test.
    internal static void ValidateTopologyDescriptor(
        byte? expectedInterface,
        byte descriptorLength,
        byte descriptorType,
        byte interfaceNumber,
        byte alternateSetting,
        byte interfaceClass,
        byte interfaceSubClass,
        byte interfaceProtocol,
        IReadOnlyList<(byte Address, int Type, ushort MaxPacketSize)> pipes,
        byte outPipe,
        byte inPipe)
    {
        if ((expectedInterface.HasValue
                ? interfaceNumber != expectedInterface.Value
                : !IsSupportedRuntimeInterface(interfaceNumber)) ||
            descriptorLength != 9 ||
            descriptorType != 4 ||
            alternateSetting != 0 ||
            interfaceClass != 0xFF ||
            interfaceSubClass != 0 ||
            interfaceProtocol != 0 ||
            pipes is null || pipes.Count != 2)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "WinUSB management descriptor must be IF0/2 FF/00/00 with two endpoints");
        }

        var sawOut = false;
        var sawIn = false;
        foreach (var pipe in pipes)
        {
            if (pipe.Type != (int)UsbdPipeType.Bulk ||
                pipe.MaxPacketSize != 64)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    $"unexpected WinUSB management pipe 0x{pipe.Address:X2} type/packet");
            }

            sawOut |= pipe.Address == outPipe;
            sawIn |= pipe.Address == inPipe;
        }

        if (!sawOut || !sawIn)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                $"required OUT 0x{outPipe:X2} / IN 0x{inPipe:X2} pipes were not found");
        }
    }

    private static void SetTimeout(
        SafeWinUsbHandle handle,
        byte pipeId)
    {
        var timeout = IoTimeoutMilliseconds;
        if (!NativeMethods.WinUsb_SetPipePolicy(
                handle,
                pipeId,
                PipeTransferTimeout,
                sizeof(uint),
                ref timeout))
        {
            throw CreateIoException(
                HostErrorKind.Open,
                $"WinUsb_SetPipePolicy(0x{pipeId:X2})");
        }
    }

    internal static HostErrorKind MapIoErrorKind(
        int error,
        HostErrorKind fallbackKind)
    {
        return fallbackKind == HostErrorKind.TransportDisconnected &&
            (error == ErrorSemTimeout || error == ErrorTimeout)
            ? HostErrorKind.Timeout
            : fallbackKind;
    }

    private static DeusHostException CreateIoException(
        HostErrorKind kind,
        string operation)
    {
        var error = Marshal.GetLastWin32Error();
        return new DeusHostException(
            MapIoErrorKind(error, kind),
            $"{operation} failed ({error}): {new Win32Exception(error).Message}");
    }

    private enum UsbdPipeType : int
    {
        Control = 0,
        Isochronous = 1,
        Bulk = 2,
        Interrupt = 3,
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    private struct UsbInterfaceDescriptor
    {
        public byte Length;
        public byte DescriptorType;
        public byte InterfaceNumber;
        public byte AlternateSetting;
        public byte NumEndpoints;
        public byte InterfaceClass;
        public byte InterfaceSubClass;
        public byte InterfaceProtocol;
        public byte Interface;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct WinUsbPipeInformation
    {
        public UsbdPipeType PipeType;
        public byte PipeId;
        public ushort MaximumPacketSize;
        public byte Interval;
    }

    private sealed class SafeWinUsbHandle : SafeHandle
    {
        public SafeWinUsbHandle(IntPtr handle)
            : base(IntPtr.Zero, true)
        {
            SetHandle(handle);
        }

        public override bool IsInvalid =>
            handle == IntPtr.Zero || handle == new IntPtr(-1);

        protected override bool ReleaseHandle() =>
            NativeMethods.WinUsb_Free(handle);
    }

    private static class NativeMethods
    {
        public const uint GenericRead = 0x80000000;
        public const uint GenericWrite = 0x40000000;
        public const uint FileShareRead = 0x00000001;
        public const uint FileShareWrite = 0x00000002;
        public const uint OpenExisting = 3;
        public const uint FileAttributeNormal = 0x00000080;
        public const uint FileFlagOverlapped = 0x40000000;

        [DllImport(
            "kernel32.dll",
            CharSet = CharSet.Unicode,
            SetLastError = true)]
        public static extern SafeFileHandle CreateFileW(
            string fileName,
            uint desiredAccess,
            uint shareMode,
            IntPtr securityAttributes,
            uint creationDisposition,
            uint flagsAndAttributes,
            IntPtr templateFile);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_Initialize(
            SafeFileHandle deviceHandle,
            out IntPtr interfaceHandle);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_Free(IntPtr interfaceHandle);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_QueryInterfaceSettings(
            SafeWinUsbHandle interfaceHandle,
            byte alternateInterfaceNumber,
            out UsbInterfaceDescriptor usbAltInterfaceDescriptor);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_QueryPipe(
            SafeWinUsbHandle interfaceHandle,
            byte alternateInterfaceNumber,
            byte pipeIndex,
            out WinUsbPipeInformation pipeInformation);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_SetPipePolicy(
            SafeWinUsbHandle interfaceHandle,
            byte pipeId,
            uint policyType,
            uint valueLength,
            ref uint value);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_ReadPipe(
            SafeWinUsbHandle interfaceHandle,
            byte pipeId,
            [Out] byte[] buffer,
            uint bufferLength,
            out uint lengthTransferred,
            IntPtr overlapped);

        [DllImport("winusb.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WinUsb_WritePipe(
            SafeWinUsbHandle interfaceHandle,
            byte pipeId,
            byte[] buffer,
            uint bufferLength,
            out uint lengthTransferred,
            IntPtr overlapped);
    }
}
