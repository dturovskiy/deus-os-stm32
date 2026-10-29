using DeusOs.Control.Core;

namespace DeusOs.Control.Gate3Offline;

/// <summary>
/// Non-destructive Gate3 source candidate. Calls the exact accepted V2 probe
/// once after independent claim consistency checks. Neither matching metadata
/// nor an INFO reply authenticates the physical USB target to the host.
/// This coordinator does not contain reset, update, signing or Flash APIs.
/// </summary>
public sealed class RecoveryRouteCoordinator
{
    private readonly ReadOnlyRecoveryInfoProbe _probe;

    public RecoveryRouteCoordinator(ReadOnlyRecoveryInfoProbe probe) =>
        _probe = probe ?? throw new ArgumentNullException(nameof(probe));

    public async Task<RecoveryCoordinatorDecision> ObserveOnceAsync(
        RecoverySourcePins? sources, RecoverySessionClaims? claimedSession,
        RecoveryHandoffSnapshot? observedSnapshot,
        CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();

        // All source/package pins and the claimed mode transition are checked
        // BEFORE USB discovery. These are user-supplied claims, not attestations.
        var preflight = RecoveryRouteReadiness.Preflight(sources, claimedSession);
        if (preflight != RecoveryStop.PreviewCompatibleOnly)
            return Fail(RecoveryCoordinatorStop.SourceOrSessionRejected, preflight);

        if (claimedSession is null || observedSnapshot is null ||
            !SameSession(claimedSession, observedSnapshot))
            return Fail(RecoveryCoordinatorStop.SnapshotMismatch);

        // Validate the frozen V2 snapshot contract before touching IDeviceDiscovery.
        var snapshotStop = ReadOnlyRecoveryInfoProbe.ValidateSnapshot(observedSnapshot);
        if (snapshotStop != RecoveryInfoStop.None)
            return Fail(RecoveryCoordinatorStop.SnapshotRejected, v2Stop: snapshotStop);

        // Exactly one discovery/open/read-only INFO attempt, owned by accepted
        // shadow V2. No retry and no direct FirmwareUpdateClient use here.
        var v2 = await _probe.ProbeAsync(observedSnapshot, cancellationToken);
        if (v2.StopReason != RecoveryInfoStop.InfoObservedGateClosed ||
            !v2.ReadOnlyInfoObserved)
            return Fail(RecoveryCoordinatorStop.V2ProbeStopped, v2Stop: v2.StopReason);

        if (v2.PhysicalIdentityAuthenticated || v2.DestructiveOperationAuthorized)
            return Fail(RecoveryCoordinatorStop.V2SafetyContractViolated,
                v2Stop: v2.StopReason);

        // The frozen V2 code has already validated opcode 0x02, flags=0 on
        // outbound request, Ok/RecoveryIdle response, and zero offset.
        // Only floor/version are copied from that single read-only observation.
        // This reconstructed struct is NOT a second INFO or separate evidence.
        var policy = RecoveryRouteReadiness.Evaluate(
            sources, claimedSession,
            new RecoveryInfoClaims(0x02, 0, FirmwareUpdateStatus.Ok,
                FirmwareUpdateState.RecoveryIdle, 0,
                v2.VersionFloor, v2.CommittedVersion));

        if (policy.Stop != RecoveryStop.PreviewCompatibleOnly ||
            !policy.OfflineClaimsConsistent ||
            policy.PhysicalIdentityAuthenticated ||
            policy.DestructiveOperationAuthorized ||
            policy.FullGate3Accepted || policy.Gate4Authorized)
            return Fail(RecoveryCoordinatorStop.PolicyRejected, policy.Stop, v2.StopReason);

        return new RecoveryCoordinatorDecision(
            RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop,
            RecoveryStop.PreviewCompatibleOnly, v2.StopReason,
            ReadOnlyInfoObserved: true, VersionFloor: v2.VersionFloor,
            CommittedVersion: v2.CommittedVersion,
            PhysicalIdentityAuthenticated: false,
            DestructiveOperationAuthorized: false,
            FullGate3Accepted: false, Gate4Authorized: false);
    }

    private static bool SameSession(RecoverySessionClaims a, RecoveryHandoffSnapshot b)
    {
        var route = a.Route switch
        {
            RecoveryRoute.ValidApplication => RecoveryEntryMode.ValidApplication,
            RecoveryRoute.NoValidApplication => RecoveryEntryMode.NoValidApplication,
            _ => (RecoveryEntryMode)(-1),
        };
        return b.EntryMode == route
            && StringComparer.Ordinal.Equals(a.UsbOwner, b.UsbOwner)
            && StringComparer.Ordinal.Equals(a.OpeningHost, b.SelectedHost)
            && StringComparer.Ordinal.Equals(a.PhysicalPort, b.PhysicalPortLocator)
            && a.RuntimeProductId == b.RuntimeProductId
            && StringComparer.Ordinal.Equals(a.RuntimePhysicalPort, b.RuntimePhysicalPortLocator)
            && a.RuntimeDetachObserved == b.RuntimeDetachObserved
            && a.RuntimeDetachOrder == b.RuntimeDetachOrdinal
            && StringComparer.Ordinal.Equals(a.RuntimeSession, b.RuntimeSessionMarker)
            && a.BootloaderAttachObserved == b.BootloaderAttachObserved
            && a.BootloaderAttachOrder == b.BootloaderAttachOrdinal
            && StringComparer.Ordinal.Equals(a.BootloaderSession, b.BootloaderSessionMarker);
    }

    private static RecoveryCoordinatorDecision Fail(
        RecoveryCoordinatorStop stop, RecoveryStop? policyStop = null,
        RecoveryInfoStop? v2Stop = null) =>
        new(stop, policyStop, v2Stop, false, 0, 0, false, false, false, false);
}

public enum RecoveryCoordinatorStop
{
    SourceOrSessionRejected,
    SnapshotMismatch,
    SnapshotRejected,
    V2ProbeStopped,
    V2SafetyContractViolated,
    PolicyRejected,
    InfoObservedPolicyCompatibleStop,
}

public sealed record RecoveryCoordinatorDecision(
    RecoveryCoordinatorStop Stop, RecoveryStop? PolicyStop,
    RecoveryInfoStop? ProbeStop, bool ReadOnlyInfoObserved,
    uint VersionFloor, uint CommittedVersion,
    bool PhysicalIdentityAuthenticated,
    bool DestructiveOperationAuthorized,
    bool FullGate3Accepted, bool Gate4Authorized);
