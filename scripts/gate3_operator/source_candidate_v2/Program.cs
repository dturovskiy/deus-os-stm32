using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;

namespace DeusOs.Gate3HardwareHandoff;

/// <summary>
/// Deliberately bounded hardware helper: "enter" sends exactly one reset-entry
/// request through the production facade; "info" uses the frozen accepted V2
/// INFO probe; "selftest" performs NO native USB calls at all.
/// No update/erase/flash method is imported or invoked by this helper.
/// </summary>
internal static class Program
{
    // Frozen installed rev5 identity. Never confuse the future v6 tree with
    // the currently installed runtime which alone may be sent ENTER.
    // Only a separately reviewed and re-signed future release may enable
    // this. The current candidate compiles but cannot send a USB request.
    private static readonly bool HardwareExecutionAdmitted = false;
    private const string ExpectedInstalledRuntimeTree = "ccbd50200ec7f0ec53da6b1048d7377271dfb7bd";
    private static async Task<int> Main(string[] args)
    {
        if (args.Length == 1 && args[0] == "selftest")
        {
            Console.WriteLine("GATE3_HELPER_SELFTEST=PASS");
            Console.WriteLine("ENTRIES=enter,info;FIRMWARE_UPDATE_WRITE_METHODS=NONE");
            return 0;
        }

        if (!HardwareExecutionAdmitted)
            return Fail("SOURCE_CANDIDATE_HARDWARE_NOT_RELEASED");

        if (!OperatingSystem.IsLinux())
        {
            Console.WriteLine("FAIL=LINUX_OWNER_REQUIRED");
            return 21;
        }

        if (args.Length == 2 && args[0] == "enter")
            return await EnterOnceAsync(args[1]);

        if (args.Length == 6 && args[0] == "info")
            return await InfoOnceAsync(args[1], args[2], args[3], args[4], args[5]);

        Console.WriteLine("FAIL=EXACT_ARGUMENTS_REQUIRED");
        return 22;
    }

    private static bool ValidLocator(string locator) =>
        ReadOnlyRecoveryInfoProbe.IsExactLinuxPortLocator(locator);

    private static async Task<int> EnterOnceAsync(string locator)
    {
        if (!ValidLocator(locator)) return Fail("LOCATOR_UNVERIFIED");

        var discovery = new LinuxLibUsbDiscovery();
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(12));
        try
        {
            // No retries, no fallbacks, and no auto-select from multiple devices.
            var devices = await discovery.DiscoverAsync(timeout.Token);
            if (devices.Count != 1 || devices[0].Platform != "Linux" ||
                !StringComparer.Ordinal.Equals(devices[0].Locator, locator))
                return Fail("RUNTIME_DISCOVERY_NOT_UNIQUE_OR_WRONG_PORT");

            var transport = await discovery.OpenAsync(devices[0], timeout.Token);
            await using var client = new DeusDeviceClient(transport);
            if (client.Locator != locator) return Fail("RUNTIME_TRANSPORT_PORT_MISMATCH");

            var negotiation = await client.NegotiateAsync(timeout.Token);
            var info = negotiation.SystemInfo;
            if (!info.SourceTreeBound)
                return Fail("RUNTIME_SOURCE_TREE_NOT_BOUND");
            if (!StringComparer.Ordinal.Equals(info.SourceTree, ExpectedInstalledRuntimeTree))
                return Fail("INSTALLED_RUNTIME_TREE_PIN_MISMATCH");
            if ((info.Capabilities & SystemCapability.FirmwareUpdate) == 0)
                return Fail("PUBLISHED_FIRMWARE_UPDATE_CAPABILITY_ABSENT");
            // This is a source-only review candidate. Before the eventful
            // send in a newly signed release, the transport/descriptor
            // ownership must also be independently qualified on the bench.

            Console.WriteLine("RUNTIME_NEGOTIATED=PASS");
            Console.WriteLine("RUNTIME_TREE=" + info.SourceTree);
            Console.WriteLine("RUNTIME_CAPABILITIES=0x" + ((uint)info.Capabilities).ToString("X8"));
            // Persist in remote evidence BEFORE transmitting opcode 01.
            Console.WriteLine("ENTER_BOOTLOADER_REQUEST_BOUNDARY=ARMED_SINGLE_SEND");
            Console.Out.Flush();

            // This is the ONLY reset-authorized operation. It has a one-shot
            // protocol request internally and MUST NOT be retried on ambiguity.
            var response = await client.EnterBootloaderAsync(
                FirmwareUpdateAccessPolicy.PublishedOnly, timeout.Token);
            if (response.Opcode != 0x01 ||
                response.Status != FirmwareUpdateStatus.Ok ||
                response.State != FirmwareUpdateState.Resetting)
                return Fail("ENTER_RESPONSE_NOT_OK_RESETTING");

            Console.WriteLine("ENTER_RESPONSE=OK_RESETTING");
            return 0;
        }
        catch (Exception ex)
        {
            // Do not classify any post-send exception as safe-to-retry.
            Console.WriteLine("FAIL=ENTER_ONCE_EXCEPTION_" + ex.GetType().Name);
            return 23;
        }
    }

    private static async Task<int> InfoOnceAsync(string locator,
        string oldSession, string newSession, string detachOrdinalText,
        string attachOrdinalText)
    {
        if (!ValidLocator(locator) || oldSession.Length is < 2 or > 96 ||
            newSession.Length is < 2 or > 96 ||
            StringComparer.Ordinal.Equals(oldSession, newSession) ||
            !long.TryParse(detachOrdinalText, out var detachOrdinal) ||
            !long.TryParse(attachOrdinalText, out var attachOrdinal) ||
            detachOrdinal < 1 || attachOrdinal <= detachOrdinal)
            return Fail("HANDOFF_SESSION_CLAIMS_UNSAFE");

        var snapshot = new RecoveryHandoffSnapshot(
            RecoveryEntryMode.ValidApplication, "Ubuntu", "Ubuntu",
            locator, 0x000C, locator, true, detachOrdinal, oldSession,
            true, attachOrdinal, newSession);

        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(8));
        try
        {
            var probe = new ReadOnlyRecoveryInfoProbe(
                new LinuxBootloaderLibUsbDiscovery());
            var response = await probe.ProbeAsync(snapshot, timeout.Token);
            Console.WriteLine("INFO_STOP=" + response.StopReason);
            Console.WriteLine("INFO_OBSERVED=" + response.ReadOnlyInfoObserved);
            Console.WriteLine("INFO_VERSION_FLOOR=" + response.VersionFloor);
            Console.WriteLine("INFO_COMMITTED=" + response.CommittedVersion);
            Console.WriteLine("PHYSICAL_IDENTITY_AUTHENTICATED=" +
                response.PhysicalIdentityAuthenticated);
            Console.WriteLine("DESTRUCTIVE_OPERATION_AUTHORIZED=" +
                response.DestructiveOperationAuthorized);
            return response.StopReason == RecoveryInfoStop.InfoObservedGateClosed &&
                response.ReadOnlyInfoObserved &&
                !response.PhysicalIdentityAuthenticated &&
                !response.DestructiveOperationAuthorized ? 0 : 24;
        }
        catch (Exception ex)
        {
            Console.WriteLine("FAIL=READONLY_INFO_EXCEPTION_" + ex.GetType().Name);
            return 25;
        }
    }

    private static int Fail(string reason)
    {
        Console.WriteLine("FAIL=" + reason);
        return 20;
    }
}
