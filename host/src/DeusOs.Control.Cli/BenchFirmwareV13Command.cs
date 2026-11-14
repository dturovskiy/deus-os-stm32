using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// Exact normal12-to-normal13 bench update path, reusing FirmwareUpdateClient.
/// ENTER, INFO, UPDATE are separate operator actions; no automatic retry.
/// </summary>
internal static class BenchFirmwareV13Command
{
    internal const string SignedNormal13Sha256 =
        "ED789BD9C926DC4D93A3FF7EEBE40802E79596A7FD7368E920A8D2E4E58086F1";
    internal const string NativeNormal13ImageSha256 =
        "DD50855C9631FABA8FDA806693BBC31E35BF4EF433FE42DEDA724BB01475F0F3";
    internal const string InstalledNormal12SourceTree =
        "a41fcbe3798fbc1a66007c0b526e94e7e77354c8";
    internal const string ExpectedSignedPackageFileName =
        "deus_os_000e_esp_split_wifi_rom_bridge_admission_normal_v13_20261114.pkg";

    private const int SignedPackageBytes = 50812;
    private const int ImageBytes = 50732;

    internal static FirmwareUpdatePackage ValidatePinnedPackage(byte[] bytes)
    {
        ArgumentNullException.ThrowIfNull(bytes);
        if (bytes.Length != SignedPackageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(bytes)), SignedNormal13Sha256))
            throw new ArgumentException(
                "normal13 whole signed package SHA-256/length mismatch; USB not accessed");

        var package = FirmwareUpdateClient.ParsePackage(bytes);
        if (package.FirmwareVersion != 13u ||
            package.Image.Length != ImageBytes ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(package.Image)),
                NativeNormal13ImageSha256))
            throw new ArgumentException(
                "normal13 image version/length/SHA-256 mismatch; USB not accessed");

        return package;
    }

    internal static void RequireNormal12BootInfo(FirmwareUpdateResponse info)
    {
        BenchFirmwareCommand.EnsureIdleInfo(info);
        if (info.VersionFloor != 12u || info.CommittedVersion != 12u)
            throw new InvalidOperationException(
                "normal13 update DENIED: expected bootloader FLOOR=12 COMMITTED=12 RecoveryIdle; no Flash attempted");
    }

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length == 0 || args[0] is "help" or "--help")
        {
            PrintHelp();
            return 0;
        }

        var action = args[0];
        if (action == "check-normal-v13")
        {
            if (args.Length != 2)
                throw new ArgumentException(
                    "bench-fw-v13 check-normal-v13 requires one signed package");
            var package = ValidatePinnedPackage(File.ReadAllBytes(args[1]));
            Console.WriteLine(
                $"SIGNED_NORMAL13=PASS VERSION={package.FirmwareVersion} " +
                $"PKG_BYTES={SignedPackageBytes} IMAGE_BYTES={ImageBytes} " +
                $"PKG_SHA256={SignedNormal13Sha256} DEVICE_IO=NONE");
            return 0;
        }

        if (action is not ("devices" or "enter-v13" or "info" or "update-normal-v13"))
            throw new ArgumentException(
                $"unknown bench-fw-v13 action '{action}'; no device opened");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException(
                "bench-fw-v13 physical commands require the owner's Linux USB host");

        if (action == "devices")
        {
            if (args.Length != 1)
                throw new ArgumentException("bench-fw-v13 devices takes no options");
            await PrintDevicesAsync("runtime", new LinuxLibUsbDiscovery());
            await PrintDevicesAsync("bootloader", new LinuxBootloaderLibUsbDiscovery());
            return 0;
        }

        var options = ParseOptions(args[1..]);
        if (!options.TryGetValue("--device", out var locator) ||
            !ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator))
            throw new ArgumentException("provide exact --device usb:BBB:PORT locator");

        if (action == "enter-v13")
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
                    negotiated.SystemInfo.SourceTree, InstalledNormal12SourceTree) ||
                negotiated.Hello.ServiceVersion != 3 ||
                negotiated.Hello.RegistryCount != 40)
                throw new InvalidOperationException(
                    "ENTER denied: signed normal12 source/service/registry mismatch; no reset attempted");

            Console.WriteLine(
                $"NORMAL12_IDENTITY=PASS SOURCE={InstalledNormal12SourceTree} SERVICE=3 REGISTRY=40");
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
                    "UPDATE requires the exact accepted normal13 signed filename");
        }

        // Parse exact signed package BEFORE opening the bootloader device.
        byte[]? packageBytes = null;
        if (action == "update-normal-v13")
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

        RequireNormal12BootInfo(current);
        Console.WriteLine(
            $"UPDATE_BEGIN=YES PACKAGE_SHA256={SignedNormal13Sha256} " +
            "FROM_FLOOR12_COMMITTED12=YES ATTEMPTS=1");
        // A partial request or timeout is ambiguous; caller must never reissue.
        var done = await updater.UpdateAsync(packageBytes!);
        if (done.Status != FirmwareUpdateStatus.Ok ||
            done.State != FirmwareUpdateState.Committed ||
            done.CommittedVersion != 13u)
            throw new InvalidOperationException(
                "normal13 UPDATE ambiguous: expected Ok/Committed/13; do not retry");

        Console.WriteLine(
            $"UPDATE_COMMITTED={done.Status}/{done.State} VERSION={done.CommittedVersion} " +
            "REBOOT_AND_POSTINSTALL_SMOKE_REQUIRED=YES");
        return 0;
    }

    private static Dictionary<string, string> ParseOptions(string[] args)
    {
        if ((args.Length & 1) != 0)
            throw new ArgumentException("bench-fw-v13 options must be --key value pairs");
        var values = new Dictionary<string, string>(StringComparer.Ordinal);
        for (var i = 0; i < args.Length; i += 2)
        {
            if (!args[i].StartsWith("--", StringComparison.Ordinal) ||
                !values.TryAdd(args[i], args[i + 1]))
                throw new ArgumentException("duplicate or malformed bench-fw-v13 option");
        }
        return values;
    }

    private static void RequireExactKeys(
        IReadOnlyDictionary<string, string> options, params string[] keys)
    {
        if (options.Count != keys.Length || keys.Any(k => !options.ContainsKey(k)))
            throw new ArgumentException("unexpected bench-fw-v13 options");
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
            Normal12 -> normal13 bench firmware, owner Linux USB host:
              bench-fw-v13 check-normal-v13 <signed-normal13.pkg> (OFFLINE)
              bench-fw-v13 devices                                (READONLY)
              bench-fw-v13 enter-v13 --device usb:BBB:PORT --confirm-enter yes
              bench-fw-v13 info --device usb:BBB:PORT              (READONLY)
              bench-fw-v13 update-normal-v13 --device usb:BBB:PORT --package <signed-normal13.pkg> --confirm-update yes

            ENTER and UPDATE require distinct authorized one-shot markers.
            Runtime ENTER: normal12 source tree, service3, registry40.
            UPDATE: exact signed SHA and bootloader floor12/committed12/idle.
            No queued device command or automatic retry.
            """);
    }
}
