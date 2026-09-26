using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;

namespace DeusOs.Control.Cli;

internal static class Program
{
    public static async Task<int> Main(string[] args)
    {
        try
        {
            var parsed = CliArguments.Parse(args);
            var discovery = CreateDiscovery();

            if (parsed.Command == "devices")
            {
                var devices = await discovery.DiscoverAsync(CancellationToken.None);
                if (devices.Count == 0)
                {
                    Console.WriteLine("No Deus OS management devices found.");
                    return 0;
                }

                for (var index = 0; index < devices.Count; ++index)
                {
                    var device = devices[index];
                    Console.WriteLine(
                        $"{index}: {device.DisplayName} [{device.Platform}] {device.Locator}");
                }

                return 0;
            }

            var candidate = await SelectDeviceAsync(
                discovery,
                parsed.DeviceLocator,
                CancellationToken.None);

            await using var session = new DeusDeviceSession(discovery);
            var negotiation = await session.ConnectAsync(
                candidate,
                CancellationToken.None);
            return await ExecuteCommandAsync(session, negotiation, parsed);
        }
        catch (DeusHostException exception)
        {
            Console.Error.WriteLine(
                $"ERROR[{exception.Kind}]: {exception.Message}");
            return 2;
        }
        catch (OperationCanceledException)
        {
            Console.Error.WriteLine("ERROR[Cancelled]: operation cancelled");
            return 3;
        }
        catch (Exception exception)
        {
            Console.Error.WriteLine($"ERROR: {exception.Message}");
            return 1;
        }
    }

    private static IDeviceDiscovery CreateDiscovery()
    {
        if (OperatingSystem.IsWindows())
        {
            return new WindowsWinUsbDiscovery();
        }

        if (OperatingSystem.IsLinux())
        {
            return new LinuxLibUsbDiscovery();
        }

        throw new DeusHostException(
            HostErrorKind.Discovery,
            "Deus OS CP currently supports Windows and Linux hosts");
    }

    private static async Task<DeviceCandidate> SelectDeviceAsync(
        IDeviceDiscovery discovery,
        string? requestedLocator,
        CancellationToken cancellationToken)
    {
        var devices = await discovery.DiscoverAsync(cancellationToken);

        if (requestedLocator is not null)
        {
            var match = devices.FirstOrDefault(
                device => string.Equals(
                    device.Locator,
                    requestedLocator,
                    StringComparison.Ordinal));

            return match ?? throw new DeusHostException(
                HostErrorKind.Discovery,
                $"requested device '{requestedLocator}' was not found");
        }

        return devices.Count switch
        {
            0 => throw new DeusHostException(
                HostErrorKind.Discovery,
                "no Deus OS management device found"),
            1 => devices[0],
            _ => throw new DeusHostException(
                HostErrorKind.Discovery,
                "multiple devices found; specify --device <locator>"),
        };
    }

    private static async Task<int> ExecuteCommandAsync(
        DeusDeviceSession session,
        NegotiationResult negotiation,
        CliArguments parsed)
    {
        switch (parsed.Command)
        {
            case "info":
                PrintInfo(negotiation);
                return 0;

            case "ping":
            {
                var count = ParsePingCount(parsed.CommandArgument);
                var requestIds = new HashSet<ushort>();
                ushort? previous = null;
                ushort first = 0;
                ushort last = 0;

                for (var index = 0; index < count; ++index)
                {
                    var result = await session.ExecuteAsync(
                        (client, token) => client.PingAsync(token),
                        CancellationToken.None);
                    EnsureSuccess(result, "ping");

                    if (result.RequestId == 0 || !requestIds.Add(result.RequestId))
                    {
                        throw new DeusHostException(
                            HostErrorKind.RequestCorrelation,
                            $"ping request id 0x{result.RequestId:X4} is zero or duplicated");
                    }

                    if (previous.HasValue)
                    {
                        var expected = previous.Value == ushort.MaxValue
                            ? (ushort)1
                            : checked((ushort)(previous.Value + 1));
                        if (result.RequestId != expected)
                        {
                            throw new DeusHostException(
                                HostErrorKind.RequestCorrelation,
                                $"ping request id 0x{result.RequestId:X4} is not sequential after 0x{previous.Value:X4}");
                        }
                    }

                    if (index == 0)
                    {
                        first = result.RequestId;
                    }

                    last = result.RequestId;
                    previous = result.RequestId;

                    if (count == 1)
                    {
                        Console.Write(result.OutputText);
                    }
                    else
                    {
                        Console.WriteLine(
                            $"PING_INDEX={index + 1} REQUEST_ID=0x{result.RequestId:X4} OUTPUT={result.OutputText.Trim()}");
                    }
                }

                if (count > 1)
                {
                    Console.WriteLine(
                        $"PING_COUNT={count} UNIQUE_REQUEST_IDS={requestIds.Count} FIRST=0x{first:X4} LAST=0x{last:X4}");
                }

                return 0;
            }

            case "health":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.HealthAsync(token),
                    CancellationToken.None);
                EnsureSuccess(result, "health");
                Console.Write(result.OutputText);
                return 0;
            }

            case "rpcinfo":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.RpcInfoAsync(token),
                    CancellationToken.None);
                EnsureSuccess(result, "rpcinfo");
                Console.Write(result.OutputText);
                return 0;
            }

            case "apps":
                PrintApplications(await session.ExecuteAsync(
                    (client, token) => client.ApplicationsAsync(token),
                    CancellationToken.None));
                return 0;

            case "app-start":
            {
                var id = ParseApplicationId(parsed.CommandArgument);
                var result = await session.ExecuteAsync(
                    (client, token) => client.StartApplicationAsync(id, token),
                    CancellationToken.None);
                Console.Write(result.OutputText);
                PrintApplications(await session.ExecuteAsync(
                    (client, token) => client.ApplicationsAsync(token),
                    CancellationToken.None));
                return 0;
            }

            case "app-stop":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.StopApplicationAsync(token),
                    CancellationToken.None);
                Console.Write(result.OutputText);
                PrintApplications(await session.ExecuteAsync(
                    (client, token) => client.ApplicationsAsync(token),
                    CancellationToken.None));
                return 0;
            }

            case "config-status":
            {
                var status = await session.ExecuteAsync(
                    (client, token) => client.AssetStatusAsync(
                        AssetAccessPolicy.PublishedOnly,
                        token),
                    CancellationToken.None);
                PrintAssetStatus(status);
                return 0;
            }

            case "config-read":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.ReadOledUiLayoutAsync(
                        AssetAccessPolicy.PublishedOnly,
                        token),
                    CancellationToken.None);

                if (result is null)
                {
                    Console.WriteLine("CONFIG_COMMITTED=NO FALLBACK=COMPILED_DEFAULT");
                    return 0;
                }

                var layout = OledUiLayoutConfigV1.Deserialize(result.Payload);
                Console.WriteLine(
                    $"CONFIG_COMMITTED=YES GENERATION={result.Generation} CRC32=0x{result.PayloadCrc32:X8}");
                Console.WriteLine(
                    $"OLED_CONSOLE_X={layout.ConsoleX} OLED_CONSOLE_Y={layout.ConsoleY} " +
                    $"OLED_CONSOLE_WIDTH={layout.ConsoleWidth} OLED_CONSOLE_HEIGHT={layout.ConsoleHeight}");
                return 0;
            }

            case "config-set-layout":
            {
                var layout = ParseOledLayout(parsed.CommandArgument);
                var result = await session.ExecuteAsync(
                    (client, token) => client.WriteOledUiLayoutAsync(
                        layout,
                        AssetAccessPolicy.PublishedOnly,
                        token),
                    CancellationToken.None);

                Console.WriteLine(
                    $"CONFIG_COMMIT=PASS GENERATION={result.Generation} CHANGED={result.Changed} " +
                    $"LENGTH={result.PayloadLength} CRC32=0x{result.PayloadCrc32:X8}");
                return 0;
            }

            default:
                throw new ArgumentException(
                    $"unknown command '{parsed.Command}'");
        }
    }

    private static void PrintInfo(NegotiationResult negotiation)
    {
        var hello = negotiation.Hello;
        var info = negotiation.SystemInfo;

        Console.WriteLine(
            $"Protocol={hello.ProtocolVersion} Service={hello.ServiceVersion} Registry={hello.RegistryCount}");
        Console.WriteLine(
            $"OS={info.OsId} Platform={info.PlatformId} Arch={info.ArchId}");
        Console.WriteLine($"SourceTree={info.SourceTree}");
        Console.WriteLine(
            $"Capabilities=0x{(uint)info.Capabilities:X8} UnitIdKind={info.UnitIdKind}");
    }

    private static void PrintApplications(ApplicationSnapshot snapshot)
    {
        Console.WriteLine(
            $"RuntimeAbi={snapshot.RuntimeAbi} Registry={snapshot.RegistryCount} " +
            $"Active=0x{snapshot.ActiveId:X4} Faults={snapshot.FaultCount}");

        foreach (var application in snapshot.Applications)
        {
            Console.WriteLine(
                $"0x{application.Id:X4} {application.Name} " +
                $"{application.State} active={application.Active}");
        }
    }

    private static void PrintAssetStatus(AssetStatusSnapshot status)
    {
        Console.WriteLine(
            $"ASSET_STATUS={status.Status} SESSION={status.SessionState} " +
            $"OBJECT_TYPE=0x{status.ObjectType:X4} TRANSFER_ID=0x{status.TransferId:X4}");
        Console.WriteLine(
            $"TRANSFER_LENGTH={status.TransferTotalLength} NEXT_OFFSET={status.NextOffset} " +
            $"COMMITTED_GENERATION={status.CommittedGeneration} " +
            $"COMMITTED_LENGTH={status.CommittedPayloadLength} " +
            $"COMMITTED_CRC32=0x{status.CommittedPayloadCrc32:X8}");
    }

    private static OledUiLayoutConfigV1 ParseOledLayout(string? text)
    {
        if (string.IsNullOrWhiteSpace(text))
        {
            throw new ArgumentException(
                "config set-layout requires x,y,width,height");
        }

        var fields = text.Split(',', StringSplitOptions.None);
        if (fields.Length != 4 ||
            !byte.TryParse(fields[0], out var x) ||
            !byte.TryParse(fields[1], out var y) ||
            !byte.TryParse(fields[2], out var width) ||
            !byte.TryParse(fields[3], out var height))
        {
            throw new ArgumentException(
                "config set-layout values must be decimal bytes: x y width height");
        }

        var layout = new OledUiLayoutConfigV1(x, y, width, height);
        _ = layout.Serialize();
        return layout;
    }

    private static int ParsePingCount(string? text)
    {
        if (text is null)
        {
            return 1;
        }

        if (!int.TryParse(text, out var count) || count < 1 || count > 1024)
        {
            throw new ArgumentOutOfRangeException(
                nameof(text),
                "ping --count must be in range 1..1024");
        }

        return count;
    }

    private static ushort ParseApplicationId(string? text)
    {
        if (string.IsNullOrWhiteSpace(text))
        {
            throw new ArgumentException("app start requires an application id");
        }

        var value = text.StartsWith("0x", StringComparison.OrdinalIgnoreCase)
            ? Convert.ToUInt32(text[2..], 16)
            : Convert.ToUInt32(text, 10);

        if (value == 0 || value > ushort.MaxValue)
        {
            throw new ArgumentOutOfRangeException(nameof(text));
        }

        return checked((ushort)value);
    }

    private static void EnsureSuccess(RpcResult result, string operation)
    {
        if (!result.IsSuccess)
        {
            throw new DeusHostException(
                HostErrorKind.RpcStatus,
                $"{operation} failed with status {result.StatusDomain}/{result.StatusCode}");
        }
    }

    private sealed record CliArguments(
        string Command,
        string? DeviceLocator,
        string? CommandArgument)
    {
        public static CliArguments Parse(string[] args)
        {
            string? device = null;
            var remaining = new List<string>();

            for (var index = 0; index < args.Length; ++index)
            {
                if (args[index] == "--device")
                {
                    if (++index >= args.Length)
                    {
                        throw new ArgumentException(
                            "--device requires a locator");
                    }

                    device = args[index];
                    continue;
                }

                remaining.Add(args[index]);
            }

            if (remaining.Count == 0 ||
                remaining[0] is "-h" or "--help" or "help")
            {
                PrintUsage();
                Environment.Exit(0);
            }

            if (remaining[0] == "ping")
            {
                if (remaining.Count == 1)
                {
                    return new CliArguments("ping", device, null);
                }

                if (remaining.Count != 3 || remaining[1] != "--count")
                {
                    throw new ArgumentException(
                        "ping accepts only --count <1..1024>");
                }

                return new CliArguments("ping", device, remaining[2]);
            }

            if (remaining[0] == "app")
            {
                if (remaining.Count < 2)
                {
                    throw new ArgumentException(
                        "app requires start or stop");
                }

                return remaining[1] switch
                {
                    "start" => new CliArguments(
                        "app-start",
                        device,
                        remaining.Count >= 3 ? remaining[2] : null),
                    "stop" => new CliArguments(
                        "app-stop",
                        device,
                        null),
                    _ => throw new ArgumentException(
                        "app requires start or stop"),
                };
            }

            if (remaining[0] == "config")
            {
                if (remaining.Count < 2)
                {
                    throw new ArgumentException(
                        "config requires status, read, or set-layout");
                }

                return remaining[1] switch
                {
                    "status" when remaining.Count == 2 =>
                        new CliArguments("config-status", device, null),
                    "read" when remaining.Count == 2 =>
                        new CliArguments("config-read", device, null),
                    "set-layout" when remaining.Count == 6 =>
                        new CliArguments(
                            "config-set-layout",
                            device,
                            string.Join(",", remaining.Skip(2))),
                    "set-layout" => throw new ArgumentException(
                        "config set-layout requires: <x> <y> <width> <height>"),
                    _ => throw new ArgumentException(
                        "config requires status, read, or set-layout"),
                };
            }

            var command = remaining[0];
            if (command is not (
                    "devices" or
                    "info" or
                    "ping" or
                    "health" or
                    "rpcinfo" or
                    "apps"))
            {
                throw new ArgumentException(
                    $"unknown command '{command}'");
            }

            return new CliArguments(command, device, null);
        }

        private static void PrintUsage()
        {
            Console.WriteLine(
                """
                Deus OS CP CLI
                  deus-cp devices
                  deus-cp [--device <locator>] info
                  deus-cp [--device <locator>] ping [--count <1..1024>]
                  deus-cp [--device <locator>] health
                  deus-cp [--device <locator>] rpcinfo
                  deus-cp [--device <locator>] apps
                  deus-cp [--device <locator>] app start <id>
                  deus-cp [--device <locator>] app stop
                  deus-cp [--device <locator>] config status
                  deus-cp [--device <locator>] config read
                  deus-cp [--device <locator>] config set-layout <x> <y> <width> <height>
                """);
        }
    }
}
