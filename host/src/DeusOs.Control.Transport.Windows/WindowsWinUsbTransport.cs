using System.ComponentModel;
using System.Runtime.InteropServices;
using DeusOs.Control.Core;
using Microsoft.Win32.SafeHandles;

namespace DeusOs.Control.Transport.Windows;

internal sealed class WindowsWinUsbTransport : IDeviceTransport
{
    private const byte OutPipe = 0x04;
    private const byte InPipe = 0x84;
    private const uint PipeTransferTimeout = 0x03;
    private const uint IoTimeoutMilliseconds = 2000;

    private readonly SafeFileHandle _fileHandle;
    private readonly SafeWinUsbHandle _winUsbHandle;
    private bool _disposed;

    private WindowsWinUsbTransport(
        string locator,
        SafeFileHandle fileHandle,
        SafeWinUsbHandle winUsbHandle)
    {
        Locator = locator;
        _fileHandle = fileHandle;
        _winUsbHandle = winUsbHandle;
    }

    public string Locator { get; }

    public static WindowsWinUsbTransport Open(string locator)
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
            ValidateInterface(winUsb);
            SetTimeout(winUsb, OutPipe);
            SetTimeout(winUsb, InPipe);
            return new WindowsWinUsbTransport(locator, file, winUsb);
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
        ThrowIfDisposed();
        cancellationToken.ThrowIfCancellationRequested();

        var bytes = data.ToArray();

        try
        {
            var transferred = await Task.Run(
                () =>
                {
                    if (!NativeMethods.WinUsb_WritePipe(
                            _winUsbHandle,
                            OutPipe,
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
        catch (OperationCanceledException)
        {
            throw;
        }
    }

    public async ValueTask<int> ReadAsync(
        Memory<byte> buffer,
        CancellationToken cancellationToken)
    {
        ThrowIfDisposed();
        cancellationToken.ThrowIfCancellationRequested();

        var bytes = new byte[buffer.Length];

        try
        {
            var transferred = await Task.Run(
                () =>
                {
                    if (!NativeMethods.WinUsb_ReadPipe(
                            _winUsbHandle,
                            InPipe,
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
        catch (OperationCanceledException)
        {
            throw;
        }
    }

    public ValueTask DisposeAsync()
    {
        if (!_disposed)
        {
            _disposed = true;
            _winUsbHandle.Dispose();
            _fileHandle.Dispose();
        }

        return ValueTask.CompletedTask;
    }

    private static void ValidateInterface(SafeWinUsbHandle handle)
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

        if (descriptor.InterfaceNumber != 2 ||
            descriptor.InterfaceClass != 0xFF ||
            descriptor.InterfaceSubClass != 0 ||
            descriptor.InterfaceProtocol != 0 ||
            descriptor.NumEndpoints != 2)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "WinUSB interface does not match IF2 FF/00/00 with two endpoints");
        }

        var sawOut = false;
        var sawIn = false;

        for (byte index = 0; index < descriptor.NumEndpoints; ++index)
        {
            if (!NativeMethods.WinUsb_QueryPipe(
                    handle,
                    0,
                    index,
                    out var pipe))
            {
                throw CreateIoException(
                    HostErrorKind.Open,
                    "WinUsb_QueryPipe");
            }

            if (pipe.PipeType != UsbdPipeType.Bulk ||
                pipe.MaximumPacketSize != 64)
            {
                throw new DeusHostException(
                    HostErrorKind.Open,
                    $"unexpected pipe 0x{pipe.PipeId:X2} type/packet");
            }

            sawOut |= pipe.PipeId == OutPipe;
            sawIn |= pipe.PipeId == InPipe;
        }

        if (!sawOut || !sawIn)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                "required EP4 OUT/IN pipes were not found");
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

    private static DeusHostException CreateIoException(
        HostErrorKind kind,
        string operation)
    {
        var error = Marshal.GetLastWin32Error();
        return new DeusHostException(
            kind,
            $"{operation} failed ({error}): {new Win32Exception(error).Message}");
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
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
