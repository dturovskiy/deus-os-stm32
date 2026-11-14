using System.Buffers.Binary;
using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// ESP01-016 normal13 ONE target OPEN then immediate CLOSE, or one Denied.
/// No UART TX/RX, no CH340, no ROM SYNC, no GPIO or Flash operation.
/// The owner one-shot marker is owned by the foreground host wrapper.
/// </summary>
internal static class BenchEspBridgeAdmissionCommand
{
    internal const string InstalledNormal13SourceTree =
        "288b25818a250c79bd81b678dfebd61142bd139c";

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length == 1 && args[0] == "help")
        {
            Console.WriteLine("bench-esp-admission once --device usb:001:8 --confirm-one-open yes");
            Console.WriteLine("ONE_OPEN_THEN_CLOSE_ONLY NO_UART_TX_RX_NO_ROM_NO_CH340_NO_FLASH");
            return 0;
        }

        if (args.Length != 5 ||
            args[0] != "once" ||
            args[1] != "--device" ||
            args[2] != "usb:001:8" ||
            args[3] != "--confirm-one-open" ||
            args[4] != "yes")
            throw new ArgumentException(
                "exact one-shot: bench-esp-admission once --device usb:001:8 --confirm-one-open yes");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException("Mac mini Linux owner host required");

        var discovery = new LinuxLibUsbDiscovery();
        var devices = await discovery.DiscoverAsync(CancellationToken.None);
        if (devices.Count != 1 ||
            !StringComparer.Ordinal.Equals(devices[0].Locator, "usb:001:8"))
            throw new InvalidOperationException(
                "expected one STM32 management USB on usb:001:8; no bridge OPEN sent");

        // Complete management negotiation; RELEASE this transport before
        // acquiring the exclusive bridge interface. Never open simultaneously.
        await using (var client = new DeusDeviceClient(
            await discovery.OpenAsync(devices[0], CancellationToken.None)))
        {
            var n = await client.NegotiateAsync();
            if (n.Hello.ServiceVersion != 3 ||
                n.Hello.RegistryCount != 40 ||
                !StringComparer.Ordinal.Equals(
                    n.SystemInfo.SourceTree, InstalledNormal13SourceTree))
                throw new InvalidOperationException(
                    "normal13 source, Service3 or Registry40 mismatch; no OPEN sent");
        }

        var random = RandomNumberGenerator.GetBytes(4);
        uint session = BinaryPrimitives.ReadUInt32LittleEndian(random);
        if (session == 0u) session = 1u;

        await using var bridge = new EspBridgeClient(
            await discovery.OpenAsync(devices[0], CancellationToken.None));
        try
        {
            await bridge.OpenAsync(session);
        }
        catch (DeusHostException ex) when (
            ex.Kind == HostErrorKind.Protocol &&
            ex.Message.Contains(
                "ESP bridge target refused operation: Denied",
                StringComparison.Ordinal))
        {
            // The target returned a fully decoded Denied; no CLOSE of a
            // session that was never admitted, and never retry OPEN.
            Console.WriteLine("ESP_BRIDGE_OPEN=Denied TARGET_RESPONSE=VALID");
            Console.WriteLine("BRIDGE_CLOSE=NOT_APPLICABLE_NO_SESSION");
            Console.WriteLine("NO_UART_TX_RX_NO_ESP_ROM_CH340_FLASH=YES");
            return 10;
        }

        Console.WriteLine("ESP_BRIDGE_OPEN=Ok TARGET_RESPONSE=VALID");
        // Must be immediate: normal13 lease is 15 seconds; UART untouched.
        await bridge.CloseAsync();
        Console.WriteLine("ESP_BRIDGE_CLOSE=Ok");
        Console.WriteLine("NO_UART_TX_RX_NO_ESP_ROM_CH340_FLASH=YES");
        return 0;
    }
}
