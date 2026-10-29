using System.Globalization;

namespace DeusOs.Control.Core;

/// <summary>
/// A single read-only INFO probe after an externally observed mode transition.
/// This code cannot enter the bootloader or perform firmware-update transactions.
/// Linux physical-port observations are untrusted until independently attested.
/// </summary>
public sealed class ReadOnlyRecoveryInfoProbe
{
    private readonly IDeviceDiscovery _bootloaderDiscovery;

    public ReadOnlyRecoveryInfoProbe(IDeviceDiscovery bootloaderDiscovery) =>
        _bootloaderDiscovery = bootloaderDiscovery ??
            throw new ArgumentNullException(nameof(bootloaderDiscovery));

    public async Task<RecoveryInfoProbeResult> ProbeAsync(
        RecoveryHandoffSnapshot snapshot,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(snapshot);
        cancellationToken.ThrowIfCancellationRequested();

        var preflight = ValidateSnapshot(snapshot);
        if (preflight != RecoveryInfoStop.None)
        {
            return Stop(preflight);
        }

        // Exactly one inventory pass; never poll until a desirable device appears.
        IReadOnlyList<DeviceCandidate> candidates;
        try
        {
            candidates = await _bootloaderDiscovery.DiscoverAsync(cancellationToken);
        }
        catch (OperationCanceledException)
        {
            throw;
        }
        catch (Exception)
        {
            return Stop(RecoveryInfoStop.DiscoveryFailure);
        }

        if (candidates is null || candidates.Count == 0)
        {
            return Stop(RecoveryInfoStop.NoBootloader);
        }

        if (candidates.Count != 1)
        {
            return Stop(RecoveryInfoStop.AmbiguousBootloader);
        }

        var candidate = candidates[0];
        if (candidate is null ||
            !StringComparer.Ordinal.Equals(candidate.Platform, "Linux") ||
            !StringComparer.Ordinal.Equals(candidate.Locator, snapshot.PhysicalPortLocator))
        {
            return Stop(RecoveryInfoStop.WrongPortOrHost);
        }

        // OpenAsync in LinuxBootloaderLibUsbDiscovery checks VID/PID + FF/00/00
        // interface 0 and two exact bulk endpoints before opening/claiming.
        // The generic IDeviceDiscovery contract itself does not attest those
        // descriptors; tests can only establish the orchestration call order.
        IDeviceTransport transport;
        try
        {
            transport = await _bootloaderDiscovery.OpenAsync(
                candidate, cancellationToken);
        }
        catch (OperationCanceledException)
        {
            throw;
        }
        catch (Exception)
        {
            return Stop(RecoveryInfoStop.TransportOpenFailure);
        }

        if (transport is null)
        {
            return Stop(RecoveryInfoStop.TransportOpenFailure);
        }

        // A new transport object is opened even when the physical locator is
        // identical before and after reset (normal Linux libusb behavior).
        await using var client = new FirmwareUpdateClient(transport);
        if (!StringComparer.Ordinal.Equals(transport.Locator, candidate.Locator))
        {
            return Stop(RecoveryInfoStop.TransportLocatorChanged);
        }

        FirmwareUpdateResponse info;
        try
        {
            // FirmwareUpdateClient.InfoAsync emits opcode 02 with flags=0.
            // No BEGIN/AUTHORIZE_HEADER/DATA/END or ENTER_BOOTLOADER reachable.
            info = await client.InfoAsync(cancellationToken);
        }
        catch (OperationCanceledException)
        {
            throw;
        }
        catch (Exception)
        {
            return Stop(RecoveryInfoStop.ReadOnlyInfoFailure);
        }

        if (info.Opcode != 0x02 ||
            info.Status != FirmwareUpdateStatus.Ok ||
            info.State != FirmwareUpdateState.RecoveryIdle ||
            info.ExpectedOffset != 0)
        {
            return Stop(RecoveryInfoStop.InvalidInfo);
        }

        if (snapshot.EntryMode == RecoveryEntryMode.ValidApplication)
        {
            if (info.CommittedVersion is not (5u or 6u))
            {
                return Stop(RecoveryInfoStop.UnexpectedCommittedVersion);
            }

            if (info.VersionFloor < info.CommittedVersion ||
                info.VersionFloor >= 7u)
            {
                return Stop(RecoveryInfoStop.VersionPolicyBlocked);
            }
        }
        else
        {
            if (info.CommittedVersion != 0)
            {
                return Stop(RecoveryInfoStop.InvalidNoAppState);
            }

            if (info.VersionFloor > 7u)
            {
                return Stop(RecoveryInfoStop.VersionPolicyBlocked);
            }
        }

        // Read-only INFO obtained from the selected transport is not
        // cryptographic identity or authorization to flash. Always STOP.
        return new RecoveryInfoProbeResult(
            RecoveryInfoStop.InfoObservedGateClosed,
            true,
            info.VersionFloor,
            info.CommittedVersion,
            false,
            false);
    }

    internal static RecoveryInfoStop ValidateSnapshot(RecoveryHandoffSnapshot s)
    {
        if (!StringComparer.Ordinal.Equals(s.UsbOwner, "Ubuntu") ||
            !StringComparer.Ordinal.Equals(s.SelectedHost, "Ubuntu"))
        {
            return RecoveryInfoStop.OwnerNotLinux;
        }

        if (!IsExactLinuxPortLocator(s.PhysicalPortLocator))
        {
            return RecoveryInfoStop.InvalidPhysicalPort;
        }

        if (!s.BootloaderAttachObserved ||
            s.BootloaderAttachOrdinal <= 0 ||
            string.IsNullOrWhiteSpace(s.BootloaderSessionMarker))
        {
            return RecoveryInfoStop.NoBootloaderAttachEvidence;
        }

        if (s.EntryMode == RecoveryEntryMode.ValidApplication)
        {
            if (s.RuntimeProductId is not (0x000C or 0x000E) ||
                !StringComparer.Ordinal.Equals(
                    s.RuntimePhysicalPortLocator, s.PhysicalPortLocator) ||
                !s.RuntimeDetachObserved ||
                s.RuntimeDetachOrdinal <= 0 ||
                s.BootloaderAttachOrdinal <= s.RuntimeDetachOrdinal ||
                string.IsNullOrWhiteSpace(s.RuntimeSessionMarker) ||
                StringComparer.Ordinal.Equals(
                    s.RuntimeSessionMarker, s.BootloaderSessionMarker))
            {
                return RecoveryInfoStop.HandoffNotEstablished;
            }
        }
        else if (s.EntryMode == RecoveryEntryMode.NoValidApplication)
        {
            if (s.RuntimeProductId != 0 ||
                s.RuntimePhysicalPortLocator is not null ||
                s.RuntimeDetachObserved ||
                s.RuntimeDetachOrdinal != 0 ||
                s.RuntimeSessionMarker is not null)
            {
                return RecoveryInfoStop.NoAppEntryInvalid;
            }
        }
        else
        {
            return RecoveryInfoStop.InvalidEntryMode;
        }

        return RecoveryInfoStop.None;
    }

    public static bool IsExactLinuxPortLocator(string? locator)
    {
        if (string.IsNullOrWhiteSpace(locator))
        {
            return false;
        }

        var segments = locator.Split(':');
        if (segments.Length != 3 || segments[0] != "usb" ||
            segments[1].Length != 3 ||
            !byte.TryParse(segments[1], NumberStyles.None,
                CultureInfo.InvariantCulture, out var bus) ||
            bus == 0 || bus.ToString("D3", CultureInfo.InvariantCulture) != segments[1])
        {
            return false;
        }

        var ports = segments[2].Split('.');
        if (ports.Length is < 1 or > 8)
        {
            return false;
        }

        foreach (var port in ports)
        {
            if (port.Length is < 1 or > 3 ||
                !byte.TryParse(port, NumberStyles.None,
                    CultureInfo.InvariantCulture, out var value) ||
                value == 0 || value.ToString(CultureInfo.InvariantCulture) != port)
            {
                return false;
            }
        }

        // usb:001:root is explicitly NOT a physically distinctive port.
        return true;
    }

    private static RecoveryInfoProbeResult Stop(RecoveryInfoStop reason) =>
        new(reason, false, 0, 0, false, false);
}

public enum RecoveryEntryMode
{
    ValidApplication = 0,
    NoValidApplication = 1,
}

public enum RecoveryInfoStop
{
    None,
    OwnerNotLinux,
    InvalidPhysicalPort,
    NoBootloaderAttachEvidence,
    HandoffNotEstablished,
    NoAppEntryInvalid,
    InvalidEntryMode,
    DiscoveryFailure,
    NoBootloader,
    AmbiguousBootloader,
    WrongPortOrHost,
    TransportOpenFailure,
    TransportLocatorChanged,
    ReadOnlyInfoFailure,
    InvalidInfo,
    UnexpectedCommittedVersion,
    InvalidNoAppState,
    VersionPolicyBlocked,
    InfoObservedGateClosed,
}

public sealed record RecoveryHandoffSnapshot(
    RecoveryEntryMode EntryMode,
    string UsbOwner,
    string SelectedHost,
    string PhysicalPortLocator,
    ushort RuntimeProductId,
    string? RuntimePhysicalPortLocator,
    bool RuntimeDetachObserved,
    long RuntimeDetachOrdinal,
    string? RuntimeSessionMarker,
    bool BootloaderAttachObserved,
    long BootloaderAttachOrdinal,
    string? BootloaderSessionMarker);

public sealed record RecoveryInfoProbeResult(
    RecoveryInfoStop StopReason,
    bool ReadOnlyInfoObserved,
    uint VersionFloor,
    uint CommittedVersion,
    bool PhysicalIdentityAuthenticated,
    bool DestructiveOperationAuthorized);
