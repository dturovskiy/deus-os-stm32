using DeusOs.Control.Core;
using System.Globalization;

namespace DeusOs.Control.Gate3Offline;

/// <summary>
/// Review-only composition of a signed-pair receipt, handoff markers and one INFO result.
/// All inputs are untrusted claims until original native evidence is independently reviewed.
/// No USB transport, runtime reset, firmware update, signer or Flash API is reachable here.
/// A compatible preview MUST NOT be treated as physical identity or destructive approval.
/// </summary>
public static class RecoveryRouteReadiness
{
    public const string CombinedTree = "081aac70c31909bf681254393e2288917cb9b8b2";
    public const string NormalPackageSha256 = "2D3191E7D3E9918AAC02BF6C3DA83C61F27472DB80F872E7CBAD63811C9949ED";
    public const string RescuePackageSha256 = "C2B026C5944E794781B77E6220C40CE42CC91D9ADD1A3974835D4E13681CAE7A";
    public const int NormalPackageBytes = 49072;
    public const int RescuePackageBytes = 52092;

    public static RecoveryPreview Evaluate(RecoverySourcePins? sources,
        RecoverySessionClaims? handoff, RecoveryInfoClaims? info)
    {
        var preflight = Preflight(sources, handoff);
        if (preflight != RecoveryStop.PreviewCompatibleOnly)
            return Stop(preflight);
        if (handoff is null)
            return Stop(RecoveryStop.WrongUsbOwner);

        if (info is null || info.Opcode != 0x02 || info.Flags != 0
            || info.Status != FirmwareUpdateStatus.Ok
            || info.State != FirmwareUpdateState.RecoveryIdle
            || info.ExpectedOffset != 0)
            return Stop(RecoveryStop.InfoNotValid);

        if (handoff.Route == RecoveryRoute.ValidApplication)
        {
            if (info.CommittedVersion is not (5u or 6u))
                return Stop(RecoveryStop.UnexpectedCommittedApplication);
            if (info.VersionFloor < info.CommittedVersion || info.VersionFloor >= 7)
                return Stop(RecoveryStop.RescueVersionNotAdmissible);
        }
        else
        {
            if (info.CommittedVersion != 0)
                return Stop(RecoveryStop.InvalidNoAppInfo);
            if (info.VersionFloor > 7)
                return Stop(RecoveryStop.RescueVersionNotAdmissible);
        }

        // Deliberate terminal STOP even when the injected policy claims all match.
        // These values must never be interpreted as a trusted live 000D session.
        return new RecoveryPreview(RecoveryStop.PreviewCompatibleOnly,
            OfflineClaimsConsistent: true, PhysicalIdentityAuthenticated: false,
            DestructiveOperationAuthorized: false, FullGate3Accepted: false,
            Gate4Authorized: false);
    }

    // Claim-only preflight. Must complete before ANY discovery/open/INFO operation.
    public static RecoveryStop Preflight(RecoverySourcePins? sources,
        RecoverySessionClaims? handoff)
    {
        if (sources is null || !StringComparer.Ordinal.Equals(sources.SourceTree, CombinedTree)
            || !Matches(sources.Normal, NormalPackageSha256, NormalPackageBytes, 6)
            || !Matches(sources.Rescue, RescuePackageSha256, RescuePackageBytes, 7))
            return RecoveryStop.SourceOrPackageNotPinned;

        if (handoff is null || !StringComparer.Ordinal.Equals(handoff.UsbOwner, "Ubuntu")
            || !StringComparer.Ordinal.Equals(handoff.OpeningHost, "Ubuntu"))
            return RecoveryStop.WrongUsbOwner;

        if (!ExactPort(handoff.PhysicalPort)
            || !StringComparer.Ordinal.Equals(handoff.BootloaderPhysicalPort, handoff.PhysicalPort)
            || handoff.BootloaderVendorId != 0x1209
            || handoff.BootloaderProductId != 0x000D
            || handoff.BootloaderInterface != 0
            || handoff.BootloaderOutEndpoint != 0x01
            || handoff.BootloaderInEndpoint != 0x81
            || handoff.CandidateCount != 1)
            return RecoveryStop.IdentityOrTopologyUncorrelated;

        if (!handoff.BootloaderAttachObserved || handoff.BootloaderAttachOrder <= 0
            || string.IsNullOrWhiteSpace(handoff.BootloaderSession))
            return RecoveryStop.NoBootloaderAttach;

        if (handoff.Route == RecoveryRoute.ValidApplication)
        {
            if (handoff.RuntimeProductId is not (0x000C or 0x000E)
                || !StringComparer.Ordinal.Equals(handoff.RuntimePhysicalPort, handoff.PhysicalPort)
                || !handoff.RuntimeDetachObserved || handoff.RuntimeDetachOrder <= 0
                || handoff.BootloaderAttachOrder <= handoff.RuntimeDetachOrder
                || string.IsNullOrWhiteSpace(handoff.RuntimeSession)
                || StringComparer.Ordinal.Equals(handoff.RuntimeSession, handoff.BootloaderSession))
                return RecoveryStop.HandoffNotCorrelated;
        }
        else if (handoff.Route == RecoveryRoute.NoValidApplication)
        {
            if (handoff.RuntimeProductId != 0 || handoff.RuntimePhysicalPort is not null
                || handoff.RuntimeSession is not null || handoff.RuntimeDetachObserved
                || handoff.RuntimeDetachOrder != 0)
                return RecoveryStop.NoAppRouteContradiction;
        }
        else return RecoveryStop.UnknownRoute;

        return RecoveryStop.PreviewCompatibleOnly;
    }

    private static RecoveryPreview Stop(RecoveryStop reason) =>
        new(reason, false, false, false, false, false);

    private static bool Matches(PackageReceipt? candidate, string digest, int length, uint version) =>
        candidate is not null && candidate.Bytes == length && candidate.Version == version
        && candidate.Sha256.Length == 64 && candidate.Sha256.All(Uri.IsHexDigit)
        && StringComparer.OrdinalIgnoreCase.Equals(candidate.Sha256, digest);

    private static bool ExactPort(string? locator)
    {
        if (string.IsNullOrWhiteSpace(locator)) return false;
        var fields = locator.Split(':');
        if (fields.Length != 3 || fields[0] != "usb" || fields[1].Length != 3
            || !byte.TryParse(fields[1], NumberStyles.None, CultureInfo.InvariantCulture,
                out var bus) || bus == 0
            || bus.ToString("D3", CultureInfo.InvariantCulture) != fields[1])
            return false;
        var ports = fields[2].Split('.');
        return ports.Length is >= 1 and <= 8 && ports.All(s => s.Length is >= 1 and <= 3
            && byte.TryParse(s, NumberStyles.None, CultureInfo.InvariantCulture, out var port)
            && port != 0 && port.ToString(CultureInfo.InvariantCulture) == s);
    }
}

public enum RecoveryRoute { ValidApplication, NoValidApplication }
public enum RecoveryStop
{
    SourceOrPackageNotPinned, WrongUsbOwner, IdentityOrTopologyUncorrelated,
    NoBootloaderAttach, HandoffNotCorrelated, NoAppRouteContradiction, UnknownRoute,
    InfoNotValid, UnexpectedCommittedApplication, InvalidNoAppInfo,
    RescueVersionNotAdmissible, PreviewCompatibleOnly
}
public sealed record PackageReceipt(string Sha256, int Bytes, uint Version);
public sealed record RecoverySourcePins(string SourceTree, PackageReceipt Normal, PackageReceipt Rescue);
public sealed record RecoverySessionClaims(
    RecoveryRoute Route, string UsbOwner, string OpeningHost, string PhysicalPort,
    string BootloaderPhysicalPort, ushort BootloaderVendorId, ushort BootloaderProductId,
    int BootloaderInterface, byte BootloaderOutEndpoint, byte BootloaderInEndpoint,
    int CandidateCount, bool BootloaderAttachObserved, long BootloaderAttachOrder,
    string? BootloaderSession, ushort RuntimeProductId, string? RuntimePhysicalPort,
    bool RuntimeDetachObserved, long RuntimeDetachOrder, string? RuntimeSession);
public sealed record RecoveryInfoClaims(
    byte Opcode, byte Flags, FirmwareUpdateStatus Status, FirmwareUpdateState State,
    ushort ExpectedOffset, uint VersionFloor, uint CommittedVersion);
public sealed record RecoveryPreview(
    RecoveryStop Stop, bool OfflineClaimsConsistent, bool PhysicalIdentityAuthenticated,
    bool DestructiveOperationAuthorized, bool FullGate3Accepted, bool Gate4Authorized);
