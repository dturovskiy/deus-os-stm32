using DeusOs.Control.Core;
using DeusOs.Control.Gate3Offline;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;

namespace DeusOs.Control.Cli;

internal static class Program
{
    public static async Task<int> Main(string[] args)
    {
        try
        {
            // Direct, minimal adapter to the existing firmware-update client.
            // Normal CLI commands and formal Gate acceptance stay unchanged.
            if (args.Length > 0 && args[0] == "bench-fw")
                return await BenchFirmwareCommand.RunAsync(args[1..]);

            var parsed = CliArguments.Parse(args);

            // Purely offline contract inspection. Must precede any discovery,
            // session creation, USB open, reset or firmware update operation.
            if (parsed.Command == "recovery-contract")
            {
                PrintRecoveryContract();
                return 0;
            }

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

    private static void PrintRecoveryContract()
    {
        // Expected identities only: no package file is opened or authenticated.
        // Real owner evidence and physical target identity remain separate.
        Console.WriteLine("RECOVERY_CONTRACT=SOURCE_PINS_ONLY");
        Console.WriteLine($"COMBINED_TREE={RecoveryRouteReadiness.CombinedTree}");
        Console.WriteLine($"NORMAL_V6_BYTES={RecoveryRouteReadiness.NormalPackageBytes} SHA256={RecoveryRouteReadiness.NormalPackageSha256}");
        Console.WriteLine($"RESCUE_V7_BYTES={RecoveryRouteReadiness.RescuePackageBytes} SHA256={RecoveryRouteReadiness.RescuePackageSha256}");
        Console.WriteLine("LIVE_USB=NOT_QUERIED PHYSICAL_IDENTITY_AUTHENTICATED=NO");
        Console.WriteLine("DESTRUCTIVE_OPERATION_AUTHORIZED=NO");
        Console.WriteLine("GATE3_AUTO_ACCEPTANCE=NO");
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
                        (client, token) => client.PingStatusAsync(token),
                        CancellationToken.None);

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
                        Console.WriteLine("PONG");
                    }
                    else
                    {
                        Console.WriteLine(
                            $"PING_INDEX={index + 1} REQUEST_ID=0x{result.RequestId:X4} OUTPUT=PONG");
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
                    (client, token) => client.HealthSnapshotAsync(token),
                    CancellationToken.None);
                PrintHealth(result);
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

            case "stacks":
            {
                // Both RPCs have been SAFE/read-only since registry 36;
                // usable with the installed v6 or future registry 39.
                // Avoid "schedstack": it is a DIAGNOSTIC test, blocked
                // while the production scheduler is active.
                const ushort rpcMspStat = 0x0006;
                const ushort rpcSchedProd = 0x0007;
                var msp = await session.ExecuteAsync(
                    (client, token) => client.RpcAsync(
                        rpcMspStat, cancellationToken: token),
                    CancellationToken.None);
                EnsureSuccess(msp, "mspstat");
                Console.Write(msp.OutputText);

                var psp = await session.ExecuteAsync(
                    (client, token) => client.RpcAsync(
                        rpcSchedProd, cancellationToken: token),
                    CancellationToken.None);
                EnsureSuccess(psp, "schedprod");
                Console.Write(psp.OutputText);

                // An RPC can succeed even when the firmware detects a
                // stack-canary or margin failure. Do not return exit 0.
                if (!msp.OutputText.Contains(
                        "MSP_STACK_OK\r\n", StringComparison.Ordinal) ||
                    !psp.OutputText.Contains(
                        "SCHED_PROD_OK\r\n", StringComparison.Ordinal))
                {
                    Console.Error.WriteLine("STACKS_STATUS=FAIL");
                    return 2;
                }

                Console.WriteLine("STACKS_STATUS=PASS");
                return 0;
            }

            case "node-start":
            {
                // Caller supplies a fresh nonzero 32-bit session ID.  No
                // cross-reboot uniqueness or authentication is implied.
                var nonce = ParseNodeNonce(parsed.CommandArgument);
                var result = await session.ExecuteAsync(
                    (client, token) => client.StartNodeAsync(nonce, token),
                    CancellationToken.None);
                EnsureSuccess(result, "node start");
                Console.Write(result.OutputText);
                return 0;
            }

            case "node-crc":
            {
                var payload = parsed.CommandArgument ??
                    throw new ArgumentException("node crc requires ASCII text");
                var result = await session.ExecuteAsync(
                    (client, token) => client.NodeCrcAsync(payload, token),
                    CancellationToken.None);
                EnsureSuccess(result, "node crc");
                Console.Write(result.OutputText);
                return 0;
            }

            case "node-status":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.NodeStatAsync(token),
                    CancellationToken.None);
                EnsureSuccess(result, "node status");
                Console.Write(result.OutputText);
                return 0;
            }

            case "wifi-scan":
                await RunWifiScanAsync(session);
                return 0;
            case "wifi-caps":
            case "wifi-status":
            case "wifi-result":
            {
                var command = parsed.Command switch {
                    "wifi-caps" => "caps",
                    "wifi-status" => "status",
                    _ => "result"
                };
                var result = await session.ExecuteAsync(
                    (client, token) => client.NodeWifiAsync(
                        command, cancellationToken: token),
                    CancellationToken.None);
                EnsureSuccess(result, "wifi " + command);
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
                    (client, token) => client.StartApplicationControlAsync(id, token),
                    CancellationToken.None);
                PrintApplicationStart(result);
                PrintApplications(await session.ExecuteAsync(
                    (client, token) => client.ApplicationsAsync(token),
                    CancellationToken.None));
                return 0;
            }

            case "app-stop":
            {
                var result = await session.ExecuteAsync(
                    (client, token) => client.StopApplicationControlAsync(token),
                    CancellationToken.None);
                PrintApplicationStop(result);
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

    private static void PrintHealth(HealthSnapshot health)
    {
        Console.WriteLine(
            $"HEALTH TICK=0x{health.Tick:X8} " +
            $"PC13=0x{(health.Pc13High ? 1u : 0u):X8} " +
            $"WDOG_ACTIVE=0x{(health.WatchdogActive ? 1u : 0u):X8} " +
            $"WDOG_RELOAD_COUNT=0x{health.WatchdogReloadCount:X8} " +
            $"RESET_FLAGS=0x{health.ResetFlags:X8} " +
            $"IWDG_RESET=0x{(health.IwdgReset ? 1u : 0u):X8}");
    }

    private static void PrintApplicationStart(ApplicationStartResult result)
    {
        Console.WriteLine(
            $"APP_START_OK ID=0x{(uint)result.ApplicationId:X8} " +
            $"ACTIVE_ID=0x{(uint)result.ActiveId:X8}");
    }

    private static void PrintApplicationStop(ApplicationStopResult result)
    {
        Console.WriteLine(
            $"APP_STOP_OK ACTIVE_ID=0x{(uint)result.ActiveId:X8}");
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

    /* Wi-Fi commands require a future registry-40 STM32 and an already
     * admitted Node HELLO. Legacy normal9 (registry-39) is refused by
     * DeusDeviceClient BEFORE any Wi-Fi request leaves the host.
     * No automatic HELLO, reset, retry of SCAN_START, or credentials.
     */
    private static Dictionary<string, string> ParseWifiReply(string text)
    {
        var fields = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var part in text.Split(
                     new[] { ' ', '\r', '\n', '\t' },
                     StringSplitOptions.RemoveEmptyEntries))
        {
            var separator = part.IndexOf('=');
            if (separator <= 0 || separator == part.Length - 1)
            {
                // The firmware prints WIFI_SSID_HEX= for hidden AP names.
                if (part == "WIFI_SSID_HEX=")
                    fields.TryAdd("WIFI_SSID_HEX", "");
                continue;
            }
            if (part.StartsWith("WIFI_", StringComparison.Ordinal) &&
                !fields.TryAdd(part[..separator], part[(separator + 1)..]))
                throw new InvalidOperationException(
                    "duplicate Wi-Fi response field: " + part[..separator]);
        }
        return fields;
    }
    private static uint WifiHex(
        IReadOnlyDictionary<string, string> fields, string key)
    {
        if (!fields.TryGetValue(key, out var value) ||
            value.Length != 10 ||
            !value.StartsWith("0x", StringComparison.Ordinal) ||
            !uint.TryParse(value[2..],
                System.Globalization.NumberStyles.AllowHexSpecifier,
                System.Globalization.CultureInfo.InvariantCulture,
                out var parsed))
            throw new InvalidOperationException(
                "malformed or missing Wi-Fi reply field " + key);
        return parsed;
    }
    private static async Task<Dictionary<string,string>> WifiQueueAndWaitAsync(
        DeusDeviceSession session, string action,
        uint generation = 0, byte index = 0, byte offset = 0)
    {
        var command = await session.ExecuteAsync(
            (client, token) => client.NodeWifiAsync(
                action, generation, index, offset, token),
            CancellationToken.None);
        EnsureSuccess(command, "wifi " + action);
        var expected = action switch
        {
            "caps" => 0x85u, "scan" => 0x86u,
            "status" => 0x87u, "page" => 0x88u,
            _ => throw new ArgumentException("unknown Wi-Fi command")
        };
        var queued = action switch
        {
            "caps" => 0x05u, "scan" => 0x06u,
            "status" => 0x07u, _ => 0x08u
        };
        if (!command.OutputText.Contains(
                $"NODE_WIFI_QUEUED=0x{queued:X8}",
                StringComparison.Ordinal))
            throw new InvalidOperationException(
                "Wi-Fi request was not confirmed queued: " + action);
        // Only read the MCU's snapshot. Polling never sends a second
        // SCAN_START. 5s bounds the 3s radio-start deadline plus USB.
        for (var attempt = 0; attempt < 50; ++attempt)
        {
            var result = await session.ExecuteAsync(
                (client, token) => client.NodeWifiAsync(
                    "result", cancellationToken: token),
                CancellationToken.None);
            EnsureSuccess(result, "wifi result");
            var fields = ParseWifiReply(result.OutputText);
            if (WifiHex(fields, "WIFI_VALID") == 1u)
            {
                var remoteError = WifiHex(fields, "WIFI_ERROR");
                if (remoteError != 0u)
                    throw new InvalidOperationException(
                        $"ESP NodeLink returned Wi-Fi error {remoteError} for {action}; " +
                        "no automatic re-HELLO or RF retry.");
                if (WifiHex(fields, "WIFI_REPLY_TYPE") != expected)
                    throw new InvalidOperationException(
                        "unexpected Wi-Fi reply opcode (stale/corrupted result)");
                return fields;
            }
            if (WifiHex(fields, "WIFI_READY") == 0u)
                throw new InvalidOperationException(
                    "Node lost READY while waiting for Wi-Fi reply; no automatic retry.");
            await Task.Delay(100);
        }
        throw new TimeoutException(
            "Wi-Fi request has no accepted reply; no automatic retry.");
    }
    private static async Task RunWifiScanAsync(DeusDeviceSession session)
    {
        // The host refuses to send SCAN_START without an existing HELLO
        // session and explicit registry-40 capability.
        var node = await session.ExecuteAsync(
            (client, token) => client.NodeStatAsync(token),
            CancellationToken.None);
        EnsureSuccess(node, "node status");
        if (!node.OutputText.Contains("NODE_READY=0x00000001",StringComparison.Ordinal) ||
            !node.OutputText.Contains("NODE_PENDING=0x00000000",StringComparison.Ordinal))
            throw new InvalidOperationException(
                "Wi-Fi scan requires already READY, idle Node Link; " +
                "run no implicit node start.");

        var caps = await WifiQueueAndWaitAsync(session, "caps");
        if (WifiHex(caps, "WIFI_MAX_APS") != 4u ||
            WifiHex(caps, "WIFI_MAX_SSID") != 32u ||
            WifiHex(caps, "WIFI_PAGE_MAX") != 14u)
            throw new InvalidOperationException(
                "ESP Wi-Fi capability geometry incompatible; scan NOT started.");

        var started = await WifiQueueAndWaitAsync(session, "scan");
        var generation = WifiHex(started, "WIFI_GENERATION");
        if (generation == 0u || WifiHex(started, "WIFI_STATE") != 2u)
            throw new InvalidOperationException(
                "ESP did not acknowledge new nonblocking Wi-Fi scan.");

        // SDK V0 scanning times out after 8s. Accept only current
        // generation, never stale pre-HELLO/status data.
        var deadline = DateTimeOffset.UtcNow.AddSeconds(11);
        Dictionary<string,string> state;
        do
        {
            await Task.Delay(250);
            state = await WifiQueueAndWaitAsync(session, "status");
            if (WifiHex(state, "WIFI_GENERATION") != generation)
                throw new InvalidOperationException(
                    "Wi-Fi scan generation drift; refuse stale AP results.");
            var s = WifiHex(state, "WIFI_STATE");
            if (s == 4u)
                throw new InvalidOperationException(
                    $"ESP Wi-Fi SDK scan failed: 0x{WifiHex(state,"WIFI_LAST_ERROR"):X8}");
            if (s == 3u)
                break;
            if (s != 2u)
                throw new InvalidOperationException(
                    "ESP scan state changed unexpectedly.");
        } while (DateTimeOffset.UtcNow < deadline);
        if (WifiHex(state, "WIFI_STATE") != 3u)
            throw new TimeoutException(
                "Wi-Fi scan exceeded local deadline, no repeat issued.");

        var count = WifiHex(state, "WIFI_COUNT");
        var truncated = WifiHex(state, "WIFI_TRUNCATED");
        if (count > 4u || truncated > 1u)
            throw new InvalidOperationException("ESP AP count invalid.");
        Console.WriteLine(
            $"WIFI_SCAN=RESULTS GENERATION=0x{generation:X8} AP_COUNT={count} TRUNCATED={truncated}");
        for (byte apIndex = 0; apIndex < count; ++apIndex)
        {
            var name = new List<byte>(32);
            uint? channel = null;
            uint? rawRssi = null;
            uint? authmode = null;
            uint? expectedLength = null;
            do
            {
                var page = await WifiQueueAndWaitAsync(
                    session, "page", generation, apIndex,
                    checked((byte)name.Count));
                if (WifiHex(page, "WIFI_GENERATION") != generation ||
                    WifiHex(page, "WIFI_AP_INDEX") != apIndex ||
                    WifiHex(page, "WIFI_SSID_OFFSET") != name.Count)
                    throw new InvalidOperationException(
                        "Wi-Fi AP page correlation failed.");
                var length = WifiHex(page, "WIFI_SSID_LENGTH");
                var pageLength = WifiHex(page, "WIFI_PAGE_LENGTH");
                var ch = WifiHex(page, "WIFI_CHANNEL");
                var rssi = WifiHex(page, "WIFI_RSSI_U8");
                var auth = WifiHex(page, "WIFI_AUTHMODE");
                if (length > 32u || pageLength > 14u ||
                    rssi > 255u || ch > 255u || auth > 255u ||
                    (expectedLength.HasValue && expectedLength.Value != length) ||
                    (channel.HasValue && channel.Value != ch) ||
                    (rawRssi.HasValue && rawRssi.Value != rssi) ||
                    (authmode.HasValue && authmode.Value != auth))
                    throw new InvalidOperationException(
                        "inconsistent or oversized AP page data.");
                expectedLength = length;
                channel = ch;
                rawRssi = rssi;
                authmode = auth;
                if (!page.TryGetValue("WIFI_SSID_HEX", out var hex) ||
                    hex.Length != pageLength * 2u ||
                    hex.Any(c => !Uri.IsHexDigit(c)))
                    throw new InvalidOperationException(
                        "Wi-Fi SSID contains invalid hex data.");
                var bytes = Convert.FromHexString(hex);
                if (bytes.Length != pageLength ||
                    name.Count + bytes.Length > length)
                    throw new InvalidOperationException(
                        "Wi-Fi SSID page overrun.");
                name.AddRange(bytes);
                if (name.Count < length && pageLength == 0u)
                    throw new InvalidOperationException(
                        "zero-progress AP pagination.");
            } while (name.Count < expectedLength!.Value);
            var signedRssi = unchecked((sbyte)(byte)rawRssi!.Value);
            Console.WriteLine(
                $"WIFI_AP_INDEX={apIndex} CHANNEL={channel} RSSI_DBM={signedRssi} " +
                $"AUTHMODE={authmode} SSID_HEX={Convert.ToHexString(name.ToArray())}");
        }
    }

    private static uint ParseNodeNonce(string? text)
    {
        if (text is null)
        {
            throw new ArgumentException(
                "node start requires a fresh nonzero 8-digit hex session id");
        }

        var digits = text.StartsWith("0x", StringComparison.OrdinalIgnoreCase)
            ? text[2..]
            : text;
        if (digits.Length != 8 ||
            !uint.TryParse(
                digits,
                System.Globalization.NumberStyles.AllowHexSpecifier,
                System.Globalization.CultureInfo.InvariantCulture,
                out var nonce) ||
            nonce == 0u)
        {
            throw new ArgumentException(
                "node start requires exactly 8 hex digits (nonzero); use a new ID after reboot");
        }

        return nonce;
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

            if (remaining[0] == "recovery")
            {
                if (remaining.Count != 2 ||
                    !StringComparer.Ordinal.Equals(remaining[1], "contract") ||
                    device is not null)
                {
                    throw new ArgumentException(
                        "recovery accepts only: recovery contract (offline; no --device)");
                }

                return new CliArguments("recovery-contract", null, null);
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

            if (remaining[0] == "wifi")
            {
                if (remaining.Count != 2)
                    throw new ArgumentException(
                        "wifi accepts scan, caps, status, or result");
                var wifiCommand = remaining[1] switch
                {
                    "scan" => "wifi-scan",
                    "caps" => "wifi-caps",
                    "status" => "wifi-status",
                    "result" => "wifi-result",
                    _ => throw new ArgumentException(
                        "wifi accepts scan, caps, status, or result")
                };
                return new CliArguments(wifiCommand,device,null);
            }
            if (remaining[0] == "node")
            {
                if (remaining.Count < 2)
                {
                    throw new ArgumentException(
                        "node requires start, crc, or status");
                }

                return remaining[1] switch
                {
                    "start" when remaining.Count == 3 =>
                        new CliArguments("node-start", device, remaining[2]),
                    "crc" when remaining.Count == 3 =>
                        new CliArguments("node-crc", device, remaining[2]),
                    "status" when remaining.Count == 2 =>
                        new CliArguments("node-status", device, null),
                    _ => throw new ArgumentException(
                        "node accepts: start <fresh_hex8>, crc <ASCII1..16>, status"),
                };
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
                    "stacks" or
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
                  deus-cp recovery contract    (offline pins; never USB or Flash)
                  deus-cp [--device <locator>] info
                  deus-cp [--device <locator>] ping [--count <1..1024>]
                  deus-cp [--device <locator>] health
                  deus-cp [--device <locator>] rpcinfo
                  deus-cp [--device <locator>] stacks   (read-only MSP + PSP watermarks)
                  deus-cp [--device <locator>] node start <fresh_hex8>
                  deus-cp [--device <locator>] node crc <ASCII1..16>
                  deus-cp [--device <locator>] node status
                  deus-cp [--device <locator>] wifi caps     (future registry-40 only)
                  deus-cp [--device <locator>] wifi scan     (explicit on-demand RF, future registry-40 only)
                  deus-cp [--device <locator>] wifi status   (single NodeLink status request)
                  deus-cp [--device <locator>] wifi result   (MCU cached result only)
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
