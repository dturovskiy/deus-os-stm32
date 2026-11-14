using System.Buffers.Binary;
using System.Security.Cryptography;
using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Control.Cli;

/// <summary>
/// Binary STDIO ↔ STM32 USB 0x05/0x87 ↔ USART2 UART0 ESP8266 bridge.
/// CH340 continues to own BOOT/EN on the current split-control bench.
/// No CH340 UART data transport or automatic ESP reset/Flash is performed.
/// Intended for an external bounded PTY/esptool supervisor after an EXACT
/// new signed STM32 source lock is physically running.
/// </summary>
internal static class BenchEspRawUartCommand
{
    private const string OldNormal10Source = "d892610a4b842fe84b17c68da2546bd29c5698b6";

    internal static async Task<int> RunAsync(string[] args)
    {
        if (args.Length != 6 || args[0] != "--device" ||
            args[2] != "--expected-source-tree" ||
            args[4] != "--confirm-rom-uart" || args[5] != "yes")
            throw new ArgumentException(
                "raw-uart-once --device usb:BBB:PORT --expected-source-tree " +
                "<40 lowercase hex> --confirm-rom-uart yes");

        var locator = args[1];
        var expected = args[3];
        if (!ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator) ||
            expected.Length != 40 ||
            expected.Any(c => !((c is >= '0' and <= '9') || (c is >= 'a' and <= 'f'))) ||
            expected == OldNormal10Source)
            throw new InvalidOperationException("wrong USB locator / source identity / normal10 hard deny");

        if (!OperatingSystem.IsLinux())
            throw new InvalidOperationException("native Linux USB owner required");

        var discovery = new LinuxLibUsbDiscovery();
        var found = await discovery.DiscoverAsync(CancellationToken.None);
        if (found.Count != 1 || found[0].Locator != locator)
            throw new InvalidOperationException("not exactly one expected STM32 USB device");

        await using (var management = new DeusDeviceClient(
            await discovery.OpenAsync(found[0], CancellationToken.None)))
        {
            var n = await management.NegotiateAsync();
            if (n.Hello.ServiceVersion != 3 ||
                n.Hello.RegistryCount < 40 ||
                n.SystemInfo.SourceTree != expected)
                throw new InvalidOperationException("target source lock/management identity mismatch");
        }

        uint sid = BinaryPrimitives.ReadUInt32LittleEndian(
            RandomNumberGenerator.GetBytes(4));
        if (sid == 0) sid = 1;

        await using var bridge = new EspBridgeClient(
            await discovery.OpenAsync(found[0], CancellationToken.None));
        await bridge.OpenAsync(sid);

        // Never mix protocol markers with raw UART stdout consumed by esptool.
        Console.Error.WriteLine("STM32_ROM_BRIDGE_EXCLUSIVE_OPEN_WAIT_EXTERNAL_BOOT");
        Console.Error.Flush();
        var stdin = Console.OpenStandardInput();
        var stdout = Console.OpenStandardOutput();
        // The STM32 target expires/aborts its bridge lease after 15s.
        // A missing CH340 token must be rejected and CLOSEd before that.
        using var ownerTimeout = new CancellationTokenSource(TimeSpan.FromSeconds(8));
        try
        {
            await RequireBootHandoverTokenAsync(stdin, ownerTimeout.Token);
        }
        catch (Exception)
        {
            // No UART bytes have been sent. Clean CLOSE prevents the target
            // from latching a permanent abort fault until an MCU reset.
            using var closeTimeout = new CancellationTokenSource(TimeSpan.FromSeconds(2));
            if (bridge.IsOpen)
                await bridge.CloseAsync(closeTimeout.Token);
            Console.Error.WriteLine("STM32_ROM_BRIDGE_CLOSE=PASS_NO_ADAPTER_TOKEN");
            throw;
        }
        Console.Error.WriteLine("STM32_ROM_BRIDGE_RAW_UART_READY");
        Console.Error.Flush();

        using var sessionTimeout = new CancellationTokenSource(TimeSpan.FromMinutes(8));
        var rxTask = PumpRxAsync(bridge, stdout, sessionTimeout.Token);
        var txTask = PumpTxAsync(bridge, stdin, sessionTimeout.Token);
        var completed = await Task.WhenAny(rxTask, txTask);
        Exception? primaryFault = null;
        try
        {
            await completed;
        }
        catch (Exception ex)
        {
            primaryFault = ex;
        }
        finally
        {
            sessionTimeout.Cancel();
            // Do not dispose a live libusb channel while its RX task owns it.
            // A stuck/failed pipe is a hard-stop, not a UART replay.
            try
            {
                await Task.WhenAll(rxTask, txTask)
                    .WaitAsync(TimeSpan.FromSeconds(4));
            }
            catch (OperationCanceledException) { }
        }
        if (primaryFault is not null)
            throw new InvalidOperationException(
                "raw ESP UART bridge fault, no retry", primaryFault);
        if (bridge.IsOpen)
        {
            using var closeTimeout = new CancellationTokenSource(TimeSpan.FromSeconds(2));
            await bridge.CloseAsync(closeTimeout.Token);
        }
        return 0;
    }

    private static async Task RequireBootHandoverTokenAsync(Stream stream, CancellationToken ct)
    {
        byte[] expected = "ADAPTER_ROM_BOOT_ASSERTED_ONCE\n"u8.ToArray();
        var one = new byte[1];
        for (var i = 0; i < expected.Length; i++)
        {
            var got = await stream.ReadAsync(one, ct);
            if (got != 1 || one[0] != expected[i])
                throw new InvalidOperationException("CH340 BOOT/EN token absent: no UART bytes sent");
        }
    }

    private static async Task PumpTxAsync(
        EspBridgeClient bridge, Stream input, CancellationToken ct)
    {
        byte[] buffer = new byte[EspBridgeWireV0.MaxData];
        while (!ct.IsCancellationRequested)
        {
            var read = await input.ReadAsync(buffer, ct);
            if (read <= 0) return;
            await bridge.WriteAsync(buffer.AsMemory(0, read), ct);
        }
    }

    private static async Task PumpRxAsync(
        EspBridgeClient bridge, Stream output, CancellationToken ct)
    {
        while (!ct.IsCancellationRequested)
        {
            var chunk = await bridge.ReadAsync(ct);
            if (chunk.Length > 0)
            {
                await output.WriteAsync(chunk, ct);
                await output.FlushAsync(ct);
            }
            else
            {
                await Task.Delay(5, ct);
            }
        }
    }
}
