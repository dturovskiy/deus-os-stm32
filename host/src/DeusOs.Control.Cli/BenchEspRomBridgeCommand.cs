using System.Buffers.Binary;
using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// STM32-managed ESP ROM read-only SYNC, never a CH340 serial route.
/// Offline plan does not open any device. Physical use requires a separately
/// hardware-qualified, installed STM32 bridge that admits OPEN locally.
/// No GPIO0/EN/Flash action can be authorized by host flags.
/// </summary>
internal static class BenchEspRomBridgeCommand
{
    private const string KnownInstalledNormal10SourceTree =
        "d892610a4b842fe84b17c68da2546bd29c5698b6";

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length > 0 && args[0] == "raw-uart-once")
            return await BenchEspRawUartCommand.RunAsync(args[1..]);

        if (args.Length == 1 && args[0] == "plan")
        {
            Console.WriteLine("ESP_BRIDGE_ROUTE=MAC_USB_MANAGEMENT_TO_STM32_USART2_TO_ESP_ROM");
            Console.WriteLine("USB_FRAMES=0x05_REQUEST_0x87_RESPONSE");
            Console.WriteLine("MAX_UART_CHUNK_BYTES=40");
            Console.WriteLine("ROM_ACTION=ONE_READONLY_SYNC_ONLY");
            Console.WriteLine("CH340_UART_DATA=NOT_CONNECTED");
            Console.WriteLine("CURRENT_NORMAL10_CAN_ENTER=HARD_DENY");
            Console.WriteLine("TARGET=STM32_FULL_OWNERSHIP_GPIO0_EN_PLUS_USART2");
            Console.WriteLine("CURRENT_HARDWARE_CH340_BOOT_EN=NOT_REWIRED");
            Console.WriteLine("AUTHORIZED_VARIANT=ONLY_AFTER_SIGNED_STM32_BOARD_QUALIFIED_IMAGE");
            Console.WriteLine("NO_GPIO_NO_RF_NO_FLASH_NO_RESET_NO_USB_OPEN=YES");
            return 0;
        }

        if (args.Length != 7 ||
            (args[0] != "rom-sync-staged" && args[0] != "rom-sync-stm32-once"))
            throw new ArgumentException(
                "Usage: bench-esp-rom plan | rom-sync-stm32-once | rom-sync-staged " +
                "--device usb:BBB:PORT --expected-source-tree <40 lower hex> " +
                "--confirm-readonly-sync yes");

        var fields = new Dictionary<string,string>(StringComparer.Ordinal);
        for (var i = 1; i < args.Length; i += 2)
        {
            if (!args[i].StartsWith("--",StringComparison.Ordinal) ||
                !fields.TryAdd(args[i],args[i+1]))
                throw new ArgumentException("invalid or duplicated ROM bridge argument");
        }
        bool stm32Owned = args[0] == "rom-sync-stm32-once";
        if (fields.Count != 3 ||
            !fields.TryGetValue("--device",out var locator) ||
            !fields.TryGetValue("--expected-source-tree",out var expected) ||
            !fields.TryGetValue(stm32Owned ? "--confirm-rom-reset" :
                "--confirm-readonly-sync",out var confirm) ||
            confirm != "yes")
            throw new ArgumentException("exact device/source-tree/read-only confirmation required");

        if (!ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator))
            throw new ArgumentException("exact USB topology locator usb:BBB:PORT required");

        if (expected.Length != 40 ||
            expected.Any(c=>!((c>='0'&&c<='9')||(c>='a'&&c<='f'))))
            throw new ArgumentException("expected source-tree must be 40 lowercase hex characters");

        if (StringComparer.Ordinal.Equals(expected,KnownInstalledNormal10SourceTree))
            throw new InvalidOperationException(
                "installed normal10 permanently denies ESP bridge OPEN: no USB attempted");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException("ESP bridge ROM probe requires the Mac mini Linux USB owner");

        var discovery = new LinuxLibUsbDiscovery();
        var found = await discovery.DiscoverAsync(CancellationToken.None);
        if (found.Count != 1 ||
            !StringComparer.Ordinal.Equals(found[0].Locator,locator))
            throw new InvalidOperationException(
                "not exactly one physical STM32 runtime management device on expected USB port");

        // Non-mutating target identity read before opening a dedicated bridge
        // transport. Never simultaneously open two writers to USB interface0.
        await using (var client = new DeusDeviceClient(
            await discovery.OpenAsync(found[0],CancellationToken.None)))
        {
            var n = await client.NegotiateAsync();
            if (n.Hello.ServiceVersion != 3 ||
                n.Hello.RegistryCount < 40 ||
                !StringComparer.Ordinal.Equals(n.SystemInfo.SourceTree,expected))
                throw new InvalidOperationException(
                    "STM32 bridge firmware identity does not match exact trusted source lock");
        }

        var raw = RandomNumberGenerator.GetBytes(4);
        var session = BinaryPrimitives.ReadUInt32LittleEndian(raw);
        if (session == 0) session = 1; // still random-backed nonzero

        await using var bridge = new EspBridgeClient(
            await discovery.OpenAsync(found[0],CancellationToken.None));
        await bridge.OpenAsync(session);
        if (stm32Owned)
        {
            // OPEN itself requests the reset/BOOT transition on the exact
            // signed and physically qualified STM32-owned board variant.
            // Task0 can wait 250ms per iteration before progressing the
            // EN-low and ROM-settling FSM. It needs two service passes,
            // not merely 20ms+100ms wall time; 1000ms is bounded below
            // the target's 15s inactive-bridge abort lease. If not ready,
            // the target will still reject TX fail-closed without retry.
            Console.WriteLine("STM32_ESP_BOOT_OWNER=STM32_EXCLUSIVE");
            await Task.Delay(1000);
        }
        else
        {
            // Historical split-control adapter, never the new default board.
            Console.WriteLine("STM32_ESP_BRIDGE_OPEN=PASS_AWAITING_ADAPTER_BOOT");
            Console.Out.Flush();
            // Target maintenance lease is 15s after OPEN. The adapter
            // proof must arrive early enough to send a clean CLOSE on denial.
            using var adapterDeadline =
                new CancellationTokenSource(TimeSpan.FromSeconds(8));
            string? adapterProof;
            try
            {
                adapterProof = await Console.In.ReadLineAsync(adapterDeadline.Token);
            }
            catch (OperationCanceledException)
            {
                // No ROM TX was sent: close the admitted target session
                // cleanly instead of poisoning the target's 15s abort latch.
                await bridge.CloseAsync();
                Console.WriteLine("STM32_ESP_BRIDGE_CLOSE=PASS_NO_ADAPTER_TOKEN");
                throw new InvalidOperationException(
                    "adapter boot handover timed out; no UART SYNC sent");
            }
            if (adapterProof != "ADAPTER_ROM_BOOT_ASSERTED_ONCE")
            {
                // EOF / wrong token is a pre-TX denial, not uncertain UART.
                // A bounded normal CLOSE preserves future one-shot admission.
                await bridge.CloseAsync();
                Console.WriteLine("STM32_ESP_BRIDGE_CLOSE=PASS_NO_ADAPTER_TOKEN");
                throw new InvalidOperationException(
                    "adapter boot handover not accepted; no UART SYNC sent");
            }
        }
        var reply = await EspRomBridgeSync.ProbeOnceAsync(bridge);
        await bridge.CloseAsync();

        if (!EspRomSlip.IsSyncResponse(reply))
            throw new InvalidOperationException("ROM SYNC reply not accepted");

        Console.WriteLine("ESP_ROM_SYNC=PASS_READONLY_ONCE");
        Console.WriteLine("ROUTE=STM32_USB_05_87_USART2_SLIP");
        Console.WriteLine(stm32Owned ?
            "ESP_BOOT_RESET=STM32_BOARD_OPEN_AND_NORMAL_CLOSE" :
            "ESP_BOOT_RESET=EXTERNAL_ADAPTER_OWNER");
        Console.WriteLine("ESP_FLASH_ERASE_RF=NOT_ATTEMPTED");
        return 0;
    }
}
