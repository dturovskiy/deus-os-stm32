using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// Minimal developer-bench adapter for the already existing firmware-update
/// client. Not a separate updater, signing service, Gate or recovery policy.
/// Device I/O happens only on an explicit owner-run enter/info/update command.
/// </summary>
internal static class BenchFirmwareCommand
{
    internal const string NormalV6Sha256 =
        "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED";

    // Owner-signed original Windows V5 application promoted as NORMAL V7.
    // The SHA pins the ENTIRE authenticated .pkg; no arbitrary packages accepted.
    internal const string NormalV7Sha256 =
        "E109ACCB1743173C6082E5A1172813900A4200E0BB4FCF39EEED6ED1BE53A7A5";
    internal const string InstalledV6SourceTree =
        "081aac70c31909bf681254393e2288917cb9b8b2";

    // Exact previously signed normal v8, native ARM15.3 compact PA3/RXNEIE gate.
    // SOURCE CANDIDATE ONLY: not proof of target v8 installation or Host qualification.
    internal const string NormalV8Sha256 =
        "DDFBB8AB5B829852B9D0245CE779F26B2B274354AD5E8FD7BAA1474DF4B4281D";
    internal const string InstalledV7SourceTree =
        "66ef68d9c6e72d23233a7d7369631a9c2b9f3b14";

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length == 0 || args[0] is "help" or "--help")
        {
            PrintHelp();
            return 0;
        }

        var action = args[0];
        if (action is not ("check" or "check-v7" or "check-v8" or "devices" or "enter" or "enter-v7" or "enter-v8" or "info" or "update" or "update-v7" or "update-v8"))
        {
            throw new ArgumentException($"unknown bench-fw command '{action}'");
        }

        if (action is "check" or "check-v7" or "check-v8")
        {
            if (args.Length != 2)
                throw new ArgumentException("bench-fw check requires exactly one package path");
            var package = action switch
            {
                "check" => ReadPinnedNormalV6(args[1]),
                "check-v7" => ReadPinnedNormalV7(args[1]),
                _ => ReadPinnedNormalV8(args[1])
            };
            var verifiedSha = action switch
            {
                "check" => NormalV6Sha256,
                "check-v7" => NormalV7Sha256,
                _ => NormalV8Sha256
            };
            Console.WriteLine(
                $"PACKAGE=PASS VERSION={package.FirmwareVersion} SHA256={verifiedSha} " +
                $"IMAGE_BYTES={package.Image.Length} DEVICE_IO=NONE");
            return 0;
        }

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException(
                "bench-fw hardware commands must run on the STM32 Linux USB host");

        if (action == "devices")
        {
            if (args.Length != 1)
                throw new ArgumentException("bench-fw devices has no options");
            await PrintDevicesAsync("runtime 000C/000E", new LinuxLibUsbDiscovery());
            await PrintDevicesAsync("bootloader 000D", new LinuxBootloaderLibUsbDiscovery());
            return 0;
        }

        var fields = ParseOptions(args.Skip(1).ToArray());
        if (!fields.TryGetValue("--device", out var locator) ||
            string.IsNullOrWhiteSpace(locator) ||
            !ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator))
        {
            throw new ArgumentException("provide the actual physical --device usb:BBB:PORT locator");
        }

        if (action is "enter" or "enter-v7" or "enter-v8")
        {
            RequireExactKeys(fields, "--device", "--confirm-enter");
            if (fields["--confirm-enter"] != "yes")
                throw new ArgumentException("enter requires --confirm-enter yes");
            var discovery = new LinuxLibUsbDiscovery();
            var candidate = await RequireOneAsync(discovery, locator);
            var transport = await discovery.OpenAsync(candidate, CancellationToken.None);
            await using var client = new DeusDeviceClient(transport);
            var negotiated = await client.NegotiateAsync();
            if (action == "enter-v7" &&
                (negotiated.SystemInfo.SourceTree != InstalledV6SourceTree ||
                 negotiated.Hello.ServiceVersion != 3 ||
                 negotiated.Hello.RegistryCount != 36))
                throw new InvalidOperationException(
                    "normal-v7 enter denied: installed signed-v6 SourceTree/service/registry identity mismatch");

            if (action == "enter-v8" &&
                (negotiated.SystemInfo.SourceTree != InstalledV7SourceTree ||
                 negotiated.Hello.ServiceVersion != 3 ||
                 negotiated.Hello.RegistryCount != 39))
                throw new InvalidOperationException(
                    "normal-v8 enter denied: installed signed-v7 SourceTree/service/registry identity mismatch; no reset requested");

            Console.WriteLine($"RUNTIME=CONNECTED PORT={locator} SOURCE={negotiated.SystemInfo.SourceTree}");
            var reply = await client.EnterBootloaderAsync(
                FirmwareUpdateAccessPolicy.PrepublicationAcceptance);
            Console.WriteLine(
                $"ENTER={reply.Status}/{reply.State} PORT={locator} " +
                "STOP=CHECK_BOOTLOADER_INFO_NEXT DO_NOT_AUTORETRY=YES");
            return 0;
        }

        if (action == "info")
            RequireExactKeys(fields, "--device");
        else
        {
            RequireExactKeys(fields, "--device", "--package", "--confirm-update");
            if (fields["--confirm-update"] != "yes")
                throw new ArgumentException("update requires --confirm-update yes");
            if (!fields.TryGetValue("--package", out var file) || string.IsNullOrWhiteSpace(file))
                throw new ArgumentException("update requires --package <exact-signed.pkg>");
        }

        // Validate and SHA-bind the package BEFORE opening a USB transport.
        byte[]? packageBytes = null;
        if (action is "update" or "update-v7" or "update-v8")
        {
            packageBytes = File.ReadAllBytes(fields["--package"]);
            _ = action switch
            {
                "update" => ValidatePinnedNormalV6(packageBytes),
                "update-v7" => ValidatePinnedNormalV7(packageBytes),
                _ => ValidatePinnedNormalV8(packageBytes)
            };
        }

        var bootDiscovery = new LinuxBootloaderLibUsbDiscovery();
        var bootCandidate = await RequireOneAsync(bootDiscovery, locator);
        var bootTransport = await bootDiscovery.OpenAsync(bootCandidate, CancellationToken.None);
        await using var updater = new FirmwareUpdateClient(bootTransport);
        var current = await updater.InfoAsync();
        EnsureIdleInfo(current);
        Console.WriteLine(
            $"BOOT_INFO=PASS PORT={locator} FLOOR={current.VersionFloor} " +
            $"COMMITTED={current.CommittedVersion} STATE={current.State}");

        if (action == "info")
        {
            Console.WriteLine("FLASH=NOT_STARTED");
            return 0;
        }

        // This specific bench starts from a confirmed rev5/000C application.
        // Do not guess a new image/recovery action for a different floor.
        if (action == "update-v8")
        {
            // Exact fresh bootloader INFO required; match only the previously
            // attested installed signed-v7 transition. No speculative update
            // when the version floor or committed application has drifted.
            if (current.CommittedVersion != 7u || current.VersionFloor != 7u)
                throw new InvalidOperationException(
                    "signed normal v8 not eligible: expected committed7/floor7; no Flash update requested");
        }
        else if (action == "update-v7")
        {
            // The actual bootloader INFO must attest an installed v6 and a
            // version floor strictly below the signed target v7.
            if (current.CommittedVersion != 6u ||
                current.VersionFloor < 5u || current.VersionFloor >= 7u)
                throw new InvalidOperationException(
                    "signed normal v7 not eligible: bootloader committed version/floor mismatch; no update requested");
        }
        else if (current.CommittedVersion != 5u ||
                 current.VersionFloor < 5u || current.VersionFloor >= 6u)
        {
            throw new InvalidOperationException(
                "signed v6 is not eligible for current bootloader INFO; " +
                "no update request was sent");
        }

        var selectedSha = action switch
        {
            "update-v8" => NormalV8Sha256,
            "update-v7" => NormalV7Sha256,
            _ => NormalV6Sha256
        };
        Console.WriteLine($"UPDATE_BEGIN=YES PACKAGE_SHA256={selectedSha}");
        // Exactly one attempt. On timeout/disconnect after BEGIN the state is
        // ambiguous: operator must inspect the actual bootloader, NOT replay.
        var completed = await updater.UpdateAsync(packageBytes!);
        if (action == "update-v8" &&
            (completed.Status != FirmwareUpdateStatus.Ok ||
             completed.State != FirmwareUpdateState.Committed ||
             completed.CommittedVersion != 8u))
            throw new InvalidOperationException(
                "signed normal v8 result ambiguous: expected Ok/Committed version8; do not retry");
        Console.WriteLine(
            $"UPDATE_COMMITTED={completed.Status}/{completed.State} " +
            $"VERSION={completed.CommittedVersion} " +
            "REBOOT_AND_SMOKE_TEST=REQUIRED");
        return 0;
    }

    internal static FirmwareUpdatePackage ReadPinnedNormalV6(string path)
    {
        return ValidatePinnedNormalV6(File.ReadAllBytes(path));
    }

    internal static FirmwareUpdatePackage ValidatePinnedNormalV6(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        if (data.Length != 49072 ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(data)), NormalV6Sha256))
        {
            throw new ArgumentException("signed normal v6 package whole SHA/size mismatch");
        }

        var parsed = FirmwareUpdateClient.ParsePackage(data);
        if (parsed.FirmwareVersion != 6u || parsed.Image.Length != 48992)
            throw new ArgumentException("signed normal v6 header/image length mismatch");
        return parsed;
    }

    internal static FirmwareUpdatePackage ReadPinnedNormalV7(string path)
    {
        return ValidatePinnedNormalV7(File.ReadAllBytes(path));
    }

    internal static FirmwareUpdatePackage ValidatePinnedNormalV7(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        if (data.Length != 53124 ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(data)), NormalV7Sha256))
        {
            throw new ArgumentException("signed normal v7 package whole SHA/size mismatch");
        }

        var parsed = FirmwareUpdateClient.ParsePackage(data);
        if (parsed.FirmwareVersion != 7u || parsed.Image.Length != 53044)
            throw new ArgumentException("signed normal v7 header/image length mismatch");
        return parsed;
    }

    internal static FirmwareUpdatePackage ReadPinnedNormalV8(string path)
    {
        return ValidatePinnedNormalV8(File.ReadAllBytes(path));
    }

    internal static FirmwareUpdatePackage ValidatePinnedNormalV8(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        if (data.Length != 53276 ||
            !StringComparer.Ordinal.Equals(
                Convert.ToHexString(SHA256.HashData(data)), NormalV8Sha256))
        {
            throw new ArgumentException("signed normal v8 package whole SHA/size mismatch");
        }

        var parsed = FirmwareUpdateClient.ParsePackage(data);
        if (parsed.FirmwareVersion != 8u || parsed.Image.Length != 53196)
            throw new ArgumentException("signed normal v8 header/image length mismatch");
        return parsed;
    }

    internal static void EnsureIdleInfo(FirmwareUpdateResponse info)
    {
        if (info.Opcode != 0x02 ||
            info.Status != FirmwareUpdateStatus.Ok ||
            info.State != FirmwareUpdateState.RecoveryIdle ||
            info.ExpectedOffset != 0)
        {
            throw new InvalidOperationException(
                $"unexpected bootloader INFO {info.Status}/{info.State}; STOP");
        }
    }

    private static Dictionary<string, string> ParseOptions(string[] args)
    {
        var options = new Dictionary<string, string>(StringComparer.Ordinal);
        if (args.Length % 2 != 0)
            throw new ArgumentException("bench-fw options must be --key value pairs");
        for (var i = 0; i < args.Length; i += 2)
        {
            if (!args[i].StartsWith("--", StringComparison.Ordinal) ||
                !options.TryAdd(args[i], args[i + 1]))
                throw new ArgumentException("unknown or duplicate option");
        }
        return options;
    }

    private static void RequireExactKeys(
        IReadOnlyDictionary<string, string> options, params string[] expected)
    {
        if (options.Count != expected.Length ||
            expected.Any(k => !options.ContainsKey(k)))
            throw new ArgumentException(
                "unexpected bench-fw options; see bench-fw --help");
    }

    private static async Task<DeviceCandidate> RequireOneAsync(
        IDeviceDiscovery discovery, string locator)
    {
        var list = await discovery.DiscoverAsync(CancellationToken.None);
        var matches = list.Where(d => StringComparer.Ordinal.Equals(d.Locator, locator)).ToArray();
        if (list.Count != 1 || matches.Length != 1)
            throw new InvalidOperationException(
                $"expected exactly one device at {locator}; found {list.Count}, STOP");
        return matches[0];
    }

    private static async Task PrintDevicesAsync(string label, IDeviceDiscovery discovery)
    {
        var list = await discovery.DiscoverAsync(CancellationToken.None);
        Console.WriteLine($"DEVICES={label} COUNT={list.Count}");
        foreach (var device in list)
            Console.WriteLine($"DEVICE={label} PORT={device.Locator} {device.DisplayName}");
    }

    private static void PrintHelp()
    {
        Console.WriteLine(
            """
            Developer bench firmware operations (Linux USB host; no auto-retry):
              deus-cp bench-fw check <signed-v6.pkg>                    (offline; legacy)
              deus-cp bench-fw check-v7 <signed-normal-v7.pkg>          (offline; exact SHA)
              deus-cp bench-fw check-v8 <signed-normal-v8.pkg>          (offline; exact SHA, no USB)
              deus-cp bench-fw devices                                  (USB inventory)
              deus-cp bench-fw enter --device usb:BBB:PORT --confirm-enter yes
              deus-cp bench-fw enter-v7 --device usb:BBB:PORT --confirm-enter yes  (v6 source locked)
              deus-cp bench-fw enter-v8 --device usb:BBB:PORT --confirm-enter yes  (v7 source locked, RESET)
              deus-cp bench-fw info --device usb:BBB:PORT               (bootloader read-only)
              deus-cp bench-fw update --device usb:BBB:PORT --package <signed-v6.pkg> --confirm-update yes
              deus-cp bench-fw update-v7 --device usb:BBB:PORT --package <signed-normal-v7.pkg> --confirm-update yes
              deus-cp bench-fw update-v8 --device usb:BBB:PORT --package <signed-normal-v8.pkg> --confirm-update yes

            Enter, INFO and update are separate commands by design.
            No production signer/RA release infrastructure is used.
            Never rerun ENTER or UPDATE automatically after an ambiguous timeout.
            """);
    }
}
