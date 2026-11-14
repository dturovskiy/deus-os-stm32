using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// Exact normal13-to-normal14 bench update path, reusing FirmwareUpdateClient.
/// ENTER, INFO, UPDATE are separate operator actions; no automatic retry.
/// </summary>
internal static class BenchFirmwareV14Command
{
    internal const string SignedNormal14Sha256 =
        "7B3ED8FEA09BF6BD8078B3F73E0888C696B32758771F52C0DFAAA1406BB10935";
    internal const string NativeNormal14ImageSha256 =
        "42581E58C1B568706094BBBE163D2EC1F88432635EE7F5A2B4CD4CBFFD7E5D35";
    internal const string InstalledNormal13SourceTree =
        "288b25818a250c79bd81b678dfebd61142bd139c";
    internal const string ExpectedSignedPackageFileName =
        "deus_os_000e_esp_split_wifi_rom_bridge_deferred_rx_normal_v14_20261114.pkg";

    private const int SignedPackageBytes = 50868;
    private const int ImageBytes = 50788;

    internal static FirmwareUpdatePackage ValidatePinnedPackage(byte[] bytes)
    {
        ArgumentNullException.ThrowIfNull(bytes);
        if (bytes.Length != SignedPackageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(bytes)), SignedNormal14Sha256))
            throw new ArgumentException(
                "normal14 whole signed package SHA-256/length mismatch; USB not accessed");

        var package = FirmwareUpdateClient.ParsePackage(bytes);
        if (package.FirmwareVersion != 14u ||
            package.Image.Length != ImageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(package.Image)),
                NativeNormal14ImageSha256))
            throw new ArgumentException(
                "normal14 image version/length/SHA-256 mismatch; USB not accessed");

        return package;
    }

    internal static void RequireNormal13BootInfo(FirmwareUpdateResponse info)
    {
        BenchFirmwareCommand.EnsureIdleInfo(info);
        if (info.VersionFloor != 13u || info.CommittedVersion != 13u)
            throw new InvalidOperationException(
                "normal14 update DENIED: expected bootloader FLOOR=13 COMMITTED=13 RecoveryIdle; no Flash attempted");
    }

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length == 0 || args[0] is "help" or "--help")
        {
            PrintHelp();
            return 0;
        }

        var action = args[0];
        if (action == "check-normal-v14")
        {
            if (args.Length != 2)
                throw new ArgumentException(
                    "bench-fw-v14 check-normal-v14 requires one signed package");
            var package = ValidatePinnedPackage(File.ReadAllBytes(args[1]));
            Console.WriteLine(
                $"SIGNED_NORMAL14=PASS VERSION={package.FirmwareVersion} " +
                $"PKG_BYTES={SignedPackageBytes} IMAGE_BYTES={ImageBytes} " +
                $"PKG_SHA256={SignedNormal14Sha256} DEVICE_IO=NONE");
            return 0;
        }

        if (action is not ("devices" or "enter-v14" or "info" or "update-normal-v14"))
            throw new ArgumentException(
                $"unknown bench-fw-v14 action '{action}'; no device opened");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException(
                "bench-fw-v14 physical commands require the owner's Linux USB host");

        if (action == "devices")
        {
            if (args.Length != 1)
                throw new ArgumentException("bench-fw-v14 devices takes no options");
            await PrintDevicesAsync("runtime", new LinuxLibUsbDiscovery());
            await PrintDevicesAsync("bootloader", new LinuxBootloaderLibUsbDiscovery());
            return 0;
        }

        var options = ParseOptions(args[1..]);
        if (!options.TryGetValue("--device", out var locator) ||
            !ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator))
            throw new ArgumentException("provide exact --device usb:BBB:PORT locator");

        if (action == "enter-v14")
        {
            RequireExactKeys(options, "--device", "--confirm-enter");
            if (options["--confirm-enter"] != "yes")
                throw new ArgumentException("ENTER requires --confirm-enter yes");

            var discovery = new LinuxLibUsbDiscovery();
            var device = await RequireExactlyOneAsync(discovery, locator);
            var transport = await discovery.OpenAsync(device, CancellationToken.None);
            await using var client = new DeusDeviceClient(transport);
            var negotiated = await client.NegotiateAsync();

            if (!StringComparer.Ordinal.Equals(
                    negotiated.SystemInfo.SourceTree, InstalledNormal13SourceTree) ||
                negotiated.Hello.ServiceVersion != 3 ||
                negotiated.Hello.RegistryCount != 40)
                throw new InvalidOperationException(
                    "ENTER denied: signed normal13 source/service/registry mismatch; no reset attempted");

            Console.WriteLine(
                $"NORMAL13_IDENTITY=PASS SOURCE={InstalledNormal13SourceTree} SERVICE=3 REGISTRY=40");
            var reply = await client.EnterBootloaderAsync(
                FirmwareUpdateAccessPolicy.PrepublicationAcceptance);
            if (reply.Status != FirmwareUpdateStatus.Ok ||
                reply.State != FirmwareUpdateState.Resetting)
                throw new InvalidOperationException(
                    $"ENTER state ambiguous ({reply.Status}/{reply.State}); STOP, no retry");

            Console.WriteLine(
                $"ENTER={reply.Status}/{reply.State} PORT={locator} " +
                "STOP=READ_BOOTLOADER_INFO_NEXT DO_NOT_REPEAT_ENTER=YES");
            return 0;
        }

        if (action == "info")
            RequireExactKeys(options, "--device");
        else
        {
            RequireExactKeys(options, "--device", "--package", "--confirm-update");
            if (options["--confirm-update"] != "yes")
                throw new ArgumentException("UPDATE requires --confirm-update yes");
            if (string.IsNullOrWhiteSpace(options["--package"]))
                throw new ArgumentException("UPDATE requires --package");
            if (!StringComparer.Ordinal.Equals(
                    Path.GetFileName(options["--package"]), ExpectedSignedPackageFileName))
                throw new ArgumentException(
                    "UPDATE requires the exact accepted normal14 signed filename");
        }

        // Parse exact signed package BEFORE opening the bootloader device.
        byte[]? packageBytes = null;
        if (action == "update-normal-v14")
            _ = ValidatePinnedPackage(
                packageBytes = File.ReadAllBytes(options["--package"]));

        var bootDiscovery = new LinuxBootloaderLibUsbDiscovery();
        var bootDevice = await RequireExactlyOneAsync(bootDiscovery, locator);
        var bootTransport = await bootDiscovery.OpenAsync(bootDevice, CancellationToken.None);
        await using var updater = new FirmwareUpdateClient(bootTransport);
        var current = await updater.InfoAsync();
        BenchFirmwareCommand.EnsureIdleInfo(current);
        Console.WriteLine(
            $"BOOT_INFO=PASS FLOOR={current.VersionFloor} " +
            $"COMMITTED={current.CommittedVersion} STATE={current.State}");

        if (action == "info")
        {
            Console.WriteLine("FLASH=NOT_STARTED");
            return 0;
        }

        RequireNormal13BootInfo(current);
        Console.WriteLine(
            $"UPDATE_BEGIN=YES PACKAGE_SHA256={SignedNormal14Sha256} " +
            "FROM_FLOOR13_COMMITTED13=YES ATTEMPTS=1");
        // A partial request or timeout is ambiguous; caller must never reissue.
        var done = await updater.UpdateAsync(packageBytes!);
        if (done.Status != FirmwareUpdateStatus.Ok ||
            done.State != FirmwareUpdateState.Committed ||
            done.CommittedVersion != 14u)
            throw new InvalidOperationException(
                "normal14 UPDATE ambiguous: expected Ok/Committed/14; do not retry");

        Console.WriteLine(
            $"UPDATE_COMMITTED={done.Status}/{done.State} VERSION={done.CommittedVersion} " +
            "REBOOT_AND_POSTINSTALL_SMOKE_REQUIRED=YES");
        return 0;
    }

    private static Dictionary<string, string> ParseOptions(string[] args)
    {
        if ((args.Length & 1) != 0)
            throw new ArgumentException("bench-fw-v14 options must be --key value pairs");
        var values = new Dictionary<string, string>(StringComparer.Ordinal);
        for (var i = 0; i < args.Length; i += 2)
        {
            if (!args[i].StartsWith("--", StringComparison.Ordinal) ||
                !values.TryAdd(args[i], args[i + 1]))
                throw new ArgumentException("duplicate or malformed bench-fw-v14 option");
        }
        return values;
    }

    private static void RequireExactKeys(
        IReadOnlyDictionary<string, string> options, params string[] keys)
    {
        if (options.Count != keys.Length || keys.Any(k => !options.ContainsKey(k)))
            throw new ArgumentException("unexpected bench-fw-v14 options");
    }

    private static async Task<DeviceCandidate> RequireExactlyOneAsync(
        IDeviceDiscovery discovery, string port)
    {
        var devices = await discovery.DiscoverAsync(CancellationToken.None);
        var matching = devices.Where(d =>
            StringComparer.Ordinal.Equals(d.Locator, port)).ToArray();
        if (devices.Count != 1 || matching.Length != 1)
            throw new InvalidOperationException(
                $"expected exactly one STM32 at {port}; found {devices.Count}, STOP");
        return matching[0];
    }

    private static async Task PrintDevicesAsync(
        string kind, IDeviceDiscovery discovery)
    {
        var devices = await discovery.DiscoverAsync(CancellationToken.None);
        Console.WriteLine($"DEVICES={kind} COUNT={devices.Count}");
        foreach (var device in devices)
            Console.WriteLine($"DEVICE={kind} PORT={device.Locator} {device.DisplayName}");
    }

    private static void PrintHelp()
    {
        Console.WriteLine(
            """
            Normal13 -> normal14 bench firmware, owner Linux USB host:
              bench-fw-v14 check-normal-v14 <signed-normal14.pkg> (OFFLINE)
              bench-fw-v14 devices                                (READONLY)
              bench-fw-v14 enter-v14 --device usb:BBB:PORT --confirm-enter yes
              bench-fw-v14 info --device usb:BBB:PORT              (READONLY)
              bench-fw-v14 update-normal-v14 --device usb:BBB:PORT --package <signed-normal14.pkg> --confirm-update yes

            ENTER and UPDATE require distinct authorized one-shot markers.
            Runtime ENTER: normal13 source tree, service3, registry40.
            UPDATE: exact signed SHA and bootloader floor13/committed13/idle.
            No queued device command or automatic retry.
            """);
    }
}
