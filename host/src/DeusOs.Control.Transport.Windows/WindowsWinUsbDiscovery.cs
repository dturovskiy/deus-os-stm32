using System.ComponentModel;
using System.Runtime.InteropServices;
using DeusOs.Control.Core;
using Microsoft.Win32.SafeHandles;

namespace DeusOs.Control.Transport.Windows;

public sealed class WindowsWinUsbDiscovery : IDeviceDiscovery
{
    public static readonly Guid ManagementInterfaceGuid =
        new("C8B05EDE-1683-5002-81F0-95636B89CEC6");

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
            var paths = EnumerateInterfacePaths(
                ManagementInterfaceGuid);
            IReadOnlyList<DeviceCandidate> candidates = paths
                .Where(path => TrySelectRuntimeInterface(path, out _))
                .Select(path => new DeviceCandidate(
                    path,
                    "Deus OS Device",
                    "Windows"))
                .ToArray();
            return ValueTask.FromResult(candidates);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Discovery,
                "Windows WinUSB discovery failed",
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
                "Windows WinUSB transport is unavailable on this platform");
        }

        try
        {
            if (!StringComparer.Ordinal.Equals(candidate.Platform, "Windows") ||
                !TrySelectRuntimeInterface(candidate.Locator, out var interfaceNumber))
            {
                throw new DeusHostException(
                    HostErrorKind.Open, "unrecognized Windows runtime USB identity");
            }
            IDeviceTransport transport =
                WindowsWinUsbTransport.Open(candidate.Locator, interfaceNumber, 0x04, 0x84);
            return ValueTask.FromResult(transport);
        }
        catch (Exception exception)
            when (exception is not DeusHostException)
        {
            throw new DeusHostException(
                HostErrorKind.Open,
                $"failed to open WinUSB device '{candidate.Locator}'",
                exception);
        }
    }

    // Verify the USB device-interface path, not just the management GUID.
    // The bootloader has a distinct VID/PID and cannot be opened as runtime.
    internal static bool TrySelectRuntimeInterface(string? locator, out byte interfaceNumber)
    {
        interfaceNumber = 0;
        if (string.IsNullOrWhiteSpace(locator))
        {
            return false;
        }

        var segments = locator.Split('#');
        if (segments.Length != 4 ||
            !string.Equals(segments[0], @"\\?\usb", StringComparison.OrdinalIgnoreCase) ||
            !string.Equals(segments[3], ManagementInterfaceGuid.ToString("B"),
                StringComparison.OrdinalIgnoreCase) ||
            segments[2].Length == 0)
        {
            return false;
        }

        if (string.Equals(segments[1], "vid_1209&pid_000c&mi_02",
                StringComparison.OrdinalIgnoreCase))
        {
            interfaceNumber = 2;
            return true;
        }
        if (string.Equals(segments[1], "vid_1209&pid_000e",
                StringComparison.OrdinalIgnoreCase) ||
            string.Equals(segments[1], "vid_1209&pid_000e&mi_00",
                StringComparison.OrdinalIgnoreCase))
        {
            interfaceNumber = 0;
            return true;
        }
        return false;
    }

    internal static IReadOnlyList<string> EnumerateInterfacePaths(Guid guid) =>
        NativeMethods.EnumerateInterfacePaths(guid);

    private static class NativeMethods
    {
        private const uint CrSuccess = 0;
        private const uint PresentOnly = 0;

        [DllImport(
            "cfgmgr32.dll",
            CharSet = CharSet.Unicode,
            SetLastError = true)]
        private static extern uint CM_Get_Device_Interface_List_SizeW(
            out uint length,
            ref Guid interfaceClassGuid,
            string? deviceId,
            uint flags);

        [DllImport(
            "cfgmgr32.dll",
            CharSet = CharSet.Unicode,
            SetLastError = true)]
        private static extern uint CM_Get_Device_Interface_ListW(
            ref Guid interfaceClassGuid,
            string? deviceId,
            [Out] char[] buffer,
            uint bufferLength,
            uint flags);

        public static IReadOnlyList<string> EnumerateInterfacePaths(Guid guid)
        {
            var mutableGuid = guid;
            var result = CM_Get_Device_Interface_List_SizeW(
                out var length,
                ref mutableGuid,
                null,
                PresentOnly);

            if (result != CrSuccess)
            {
                throw new Win32Exception(
                    unchecked((int)result),
                    "CM_Get_Device_Interface_List_SizeW failed");
            }

            if (length <= 1)
            {
                return Array.Empty<string>();
            }

            var buffer = new char[length];
            result = CM_Get_Device_Interface_ListW(
                ref mutableGuid,
                null,
                buffer,
                length,
                PresentOnly);

            if (result != CrSuccess)
            {
                throw new Win32Exception(
                    unchecked((int)result),
                    "CM_Get_Device_Interface_ListW failed");
            }

            var values = new List<string>();
            var start = 0;
            for (var index = 0; index < buffer.Length; ++index)
            {
                if (buffer[index] != '\0')
                {
                    continue;
                }

                if (index == start)
                {
                    break;
                }

                values.Add(new string(buffer, start, index - start));
                start = index + 1;
            }

            return values;
        }
    }
}
