using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// Exact normal10-to-normal11 bench update path, reusing FirmwareUpdateClient.
/// ENTER, INFO, UPDATE are separate operator actions; no automatic retry.
/// </summary>
internal static class BenchFirmwareV11Command
{
    internal const string SignedNormal11Sha256 =
        "986AF02EB6DF09B94A6C0542FFACCB3A6DAA65D94DD4F57C4A639A512FFB4AC8";
    internal const string NativeNormal11ImageSha256 =
        "EC6149B62B7E17E76C90009AEEA1E14A32F8B6E8EFF456C28EB246571659C2BD";
    internal const string InstalledNormal10SourceTree =
        "d892610a4b842fe84b17c68da2546bd29c5698b6";
    internal const string ExpectedSignedPackageFileName =
        "deus_os_000e_esp_split_wifi_rom_normal_v11_20261112.pkg";

    private const int SignedPackageBytes = 50456;
    private const int ImageBytes = 50376;

    internal static FirmwareUpdatePackage ValidatePinnedPackage(byte[] bytes)
    {
        ArgumentNullException.ThrowIfNull(bytes);
        if (bytes.Length != SignedPackageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(bytes)), SignedNormal11Sha256))
            throw new ArgumentException(
                "normal11 whole signed package SHA-256/length mismatch; USB not accessed");

        var package = FirmwareUpdateClient.ParsePackage(bytes);
        if (package.FirmwareVersion != 11u ||
            package.Image.Length != ImageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(package.Image)),
                NativeNormal11ImageSha256))
            throw new ArgumentException(
                "normal11 image version/length/SHA-256 mismatch; USB not accessed");

        return package;
    }

    internal static void RequireNormal10BootInfo(FirmwareUpdateResponse info)
    {
        BenchFirmwareCommand.EnsureIdleInfo(info);
        if (info.VersionFloor != 10u || info.CommittedVersion != 10u)
            throw new InvalidOperationException(
                "normal11 update DENIED: expected bootloader FLOOR=10 COMMITTED=10 RecoveryIdle; no Flash attempted");
    }

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length == 0 || args[0] is "help" or "--help")
        {
            PrintHelp();
            return 0;
        }

        var action = args[0];
        if (action == "check-normal-v11")
        {
            if (args.Length != 2)
                throw new ArgumentException(
                    "bench-fw-v11 check-normal-v11 requires one signed package");
            var package = ValidatePinnedPackage(File.ReadAllBytes(args[1]));
            Console.WriteLine(
                $"SIGNED_NORMAL11=PASS VERSION={package.FirmwareVersion} " +
                $"PKG_BYTES={SignedPackageBytes} IMAGE_BYTES={ImageBytes} " +
                $"PKG_SHA256={SignedNormal11Sha256} DEVICE_IO=NONE");
            return 0;
        }

        if (action is not ("devices" or "enter-v11" or "info" or "update-normal-v11"))
            throw new ArgumentException(
                $"unknown bench-fw-v11 action '{action}'; no device opened");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException(
                "bench-fw-v11 physical commands require the owner's Linux USB host");

        if (action == "devices")
        {
            if (args.Length != 1)
                throw new ArgumentException("bench-fw-v11 devices takes no options");
            await PrintDevicesAsync("runtime", new LinuxLibUsbDiscovery());
            await PrintDevicesAsync("bootloader", new LinuxBootloaderLibUsbDiscovery());
            return 0;
        }

        var options = ParseOptions(args[1..]);
        if (!options.TryGetValue("--device", out var locator) ||
            !ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator))
            throw new ArgumentException("provide exact --device usb:BBB:PORT locator");

        if (action == "enter-v11")
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
                    negotiated.SystemInfo.SourceTree, InstalledNormal10SourceTree) ||
                negotiated.Hello.ServiceVersion != 3 ||
                negotiated.Hello.RegistryCount != 40)
                throw new InvalidOperationException(
                    "ENTER denied: signed normal10 source/service/registry mismatch; no reset attempted");

            Console.WriteLine(
                $"NORMAL10_IDENTITY=PASS SOURCE={InstalledNormal10SourceTree} SERVICE=3 REGISTRY=40");
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
                    "UPDATE requires the exact accepted normal11 signed filename");
        }

        // Parse exact signed package BEFORE opening the bootloader device.
        byte[]? packageBytes = null;
        if (action == "update-normal-v11")
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

        RequireNormal10BootInfo(current);
        Console.WriteLine(
            $"UPDATE_BEGIN=YES PACKAGE_SHA256={SignedNormal11Sha256} " +
            "FROM_FLOOR10_COMMITTED10=YES ATTEMPTS=1");
        // A partial request or timeout is ambiguous; caller must never reissue.
        var done = await updater.UpdateAsync(packageBytes!);
        if (done.Status != FirmwareUpdateStatus.Ok ||
            done.State != FirmwareUpdateState.Committed ||
            done.CommittedVersion != 11u)
            throw new InvalidOperationException(
                "normal11 UPDATE ambiguous: expected Ok/Committed/11; do not retry");

        Console.WriteLine(
            $"UPDATE_COMMITTED={done.Status}/{done.State} VERSION={done.CommittedVersion} " +
            "REBOOT_AND_POSTINSTALL_SMOKE_REQUIRED=YES");
        return 0;
    }

    private static Dictionary<string, string> ParseOptions(string[] args)
    {
        if ((args.Length & 1) != 0)
            throw new ArgumentException("bench-fw-v11 options must be --key value pairs");
        var values = new Dictionary<string, string>(StringComparer.Ordinal);
        for (var i = 0; i < args.Length; i += 2)
        {
            if (!args[i].StartsWith("--", StringComparison.Ordinal) ||
                !values.TryAdd(args[i], args[i + 1]))
                throw new ArgumentException("duplicate or malformed bench-fw-v11 option");
        }
        return values;
    }

    private static void RequireExactKeys(
        IReadOnlyDictionary<string, string> options, params string[] keys)
    {
        if (options.Count != keys.Length || keys.Any(k => !options.ContainsKey(k)))
            throw new ArgumentException("unexpected bench-fw-v11 options");
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
            Normal10 -> normal11 bench firmware, owner Linux USB host:
              bench-fw-v11 check-normal-v11 <signed-normal11.pkg> (OFFLINE)
              bench-fw-v11 devices                                (READONLY)
              bench-fw-v11 enter-v11 --device usb:BBB:PORT --confirm-enter yes
              bench-fw-v11 info --device usb:BBB:PORT              (READONLY)
              bench-fw-v11 update-normal-v11 --device usb:BBB:PORT --package <signed-normal11.pkg> --confirm-update yes

            ENTER and UPDATE require distinct authorized one-shot markers.
            Runtime ENTER: normal10 source tree, service3, registry40.
            UPDATE: exact signed SHA and bootloader floor10/committed10/idle.
            No queued device command or automatic retry.
            """);
    }
}
