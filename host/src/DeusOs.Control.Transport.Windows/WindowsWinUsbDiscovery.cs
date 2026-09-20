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
            var paths = NativeMethods.EnumerateInterfacePaths(
                ManagementInterfaceGuid);
            IReadOnlyList<DeviceCandidate> candidates = paths
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
            IDeviceTransport transport =
                WindowsWinUsbTransport.Open(candidate.Locator);
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
