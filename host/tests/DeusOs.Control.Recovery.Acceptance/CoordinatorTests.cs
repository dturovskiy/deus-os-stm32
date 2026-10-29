using System.Buffers.Binary;
using DeusOs.Control.Core;
using DeusOs.Control.Gate3Offline;

internal static class CoordinatorTests
{
    private const string Port = "usb:003:1.4";
    private static readonly RecoverySourcePins Sources = new(
        RecoveryRouteReadiness.CombinedTree,
        new(RecoveryRouteReadiness.NormalPackageSha256, 49072, 6),
        new(RecoveryRouteReadiness.RescuePackageSha256, 52092, 7));

    private static readonly RecoverySessionClaims Session = new(
        RecoveryRoute.ValidApplication, "Ubuntu", "Ubuntu",
        Port, Port, 0x1209, 0x000D, 0, 0x01, 0x81,
        1, true, 4, "000d-new",
        0x000E, Port, true, 3, "runtime-old");

    private static readonly RecoveryHandoffSnapshot Snapshot = new(
        RecoveryEntryMode.ValidApplication, "Ubuntu", "Ubuntu", Port,
        0x000E, Port, true, 3, "runtime-old", true, 4, "000d-new");

    public static async Task RunAsync()
    {
        int count = 0;

        async Task Check(string name, RecoveryCoordinatorStop expected,
            RecoverySourcePins? source = null,
            RecoverySessionClaims? session = null,
            RecoveryHandoffSnapshot? snapshot = null,
            uint floor = 6, uint committed = 6,
            FirmwareUpdateStatus status = FirmwareUpdateStatus.Ok,
            FirmwareUpdateState state = FirmwareUpdateState.RecoveryIdle,
            ushort offset = 0, byte opcode = 0x02, bool badCrc = false,
            int actualCandidates = 1, bool timeout = false,
            string? locator = null, bool cancel = false,
            int discover = 1, int open = 1, int writes = 1, int reads = 1)
        {
            var transport = new MockTransport(Frame(floor, committed, status,
                state, offset, opcode, badCrc))
            { Timeout = timeout, LocatorOverride = locator };
            var discovery = new MockDiscovery(transport)
            { CandidateCount = actualCandidates };
            var coordinator = new RecoveryRouteCoordinator(
                new ReadOnlyRecoveryInfoProbe(discovery));
            var selectedSession = session ?? Session;
            var selectedSnapshot = snapshot ?? Snapshot;

            if (cancel)
            {
                using var cancelled = new CancellationTokenSource();
                cancelled.Cancel();
                bool threw = false;
                try
                {
                    _ = await coordinator.ObserveOnceAsync(source ?? Sources,
                        selectedSession, selectedSnapshot, cancelled.Token);
                }
                catch (OperationCanceledException) { threw = true; }
                if (!threw) throw new Exception(name + ": cancellation not propagated");
            }
            else
            {
                var result = await coordinator.ObserveOnceAsync(source ?? Sources,
                    selectedSession, selectedSnapshot);
                if (result.Stop != expected
                    || result.PhysicalIdentityAuthenticated
                    || result.DestructiveOperationAuthorized
                    || result.FullGate3Accepted || result.Gate4Authorized
                    || result.ReadOnlyInfoObserved !=
                        (expected == RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop))
                    throw new Exception(name + " outcome=" + result);

                if (expected == RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop
                    && (result.VersionFloor != floor || result.CommittedVersion != committed
                        || result.ProbeStop != RecoveryInfoStop.InfoObservedGateClosed))
                    throw new Exception(name + ": read-only INFO was not forwarded intact");
            }

            if (discovery.DiscoverCount != discover || discovery.OpenCount != open
                || transport.Writes.Count != writes || transport.ReadCount != reads)
                throw new Exception(name + " counts=" +
                    $"{discovery.DiscoverCount}/{discovery.OpenCount}/" +
                    $"{transport.Writes.Count}/{transport.ReadCount} wanted " +
                    $"{discover}/{open}/{writes}/{reads}");

            // Each write MUST be the single valid INFO request frame; never a
            // reset (01), BEGIN (03), AUTHORIZE (04), DATA (05) or END (06).
            foreach (var w in transport.Writes)
                if (w.Length != 13 || w[0] != 0xA5 || w[1] != 0x5A ||
                    w[3] != 0x04 || w[4] != 0 || w[10] != 0x02)
                    throw new Exception(name + ": non-INFO write observed");
            ++count;
            Console.WriteLine("PASS integrated " + name);
        }

        await Check("valid-app-single-info", RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop);
        await Check("noapp-floor7", RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop,
            session: NoAppSession(), snapshot: NoAppSnapshot(), floor: 7,
            committed: 0);
        await Check("legacy-v5", RecoveryCoordinatorStop.InfoObservedPolicyCompatibleStop,
            session: Session with { RuntimeProductId = 0x000C },
            snapshot: Snapshot with { RuntimeProductId = 0x000C }, floor: 5, committed: 5);
        await Check("sources-invalid-stop-before-discovery",
            RecoveryCoordinatorStop.SourceOrSessionRejected,
            source: Sources with { Rescue = Sources.Rescue with { Bytes = 111 } },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("wrong-owner-stop-before-discovery",
            RecoveryCoordinatorStop.SourceOrSessionRejected,
            session: Session with { UsbOwner = "Windows" },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("multiple-claimed-stop-before-discovery",
            RecoveryCoordinatorStop.SourceOrSessionRejected,
            session: Session with { CandidateCount = 2 },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("session-port-mismatch",
            RecoveryCoordinatorStop.SnapshotMismatch,
            snapshot: Snapshot with { PhysicalPortLocator = "usb:003:1.5" },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("session-ordinal-mismatch",
            RecoveryCoordinatorStop.SnapshotMismatch,
            snapshot: Snapshot with { BootloaderAttachOrdinal = 5 },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("session-marker-mismatch",
            RecoveryCoordinatorStop.SnapshotMismatch,
            snapshot: Snapshot with { BootloaderSessionMarker = "other" },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("session-entry-mode-mismatch",
            RecoveryCoordinatorStop.SnapshotMismatch,
            snapshot: Snapshot with { EntryMode = RecoveryEntryMode.NoValidApplication },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("preflight-blocks-invalid-mirrored-runtime-before-v2",
            RecoveryCoordinatorStop.SourceOrSessionRejected,
            snapshot: Snapshot with { RuntimeProductId = 0x000D },
            session: Session with { RuntimeProductId = 0x000D },
            discover: 0, open: 0, writes: 0, reads: 0);
        await Check("no-real-device",
            RecoveryCoordinatorStop.V2ProbeStopped,
            actualCandidates: 0, discover: 1, open: 0, writes: 0, reads: 0);
        await Check("ambiguous-real-devices",
            RecoveryCoordinatorStop.V2ProbeStopped,
            actualCandidates: 2, discover: 1, open: 0, writes: 0, reads: 0);
        await Check("stale-opened-transport-locator",
            RecoveryCoordinatorStop.V2ProbeStopped,
            locator: "usb:003:1.5", discover: 1, open: 1, writes: 0, reads: 0);
        await Check("timeout-no-retry",
            RecoveryCoordinatorStop.V2ProbeStopped, timeout: true);
        await Check("invalid-info-crc",
            RecoveryCoordinatorStop.V2ProbeStopped, badCrc: true);
        await Check("invalid-info-opcode",
            RecoveryCoordinatorStop.V2ProbeStopped, opcode: 0x03);
        await Check("invalid-info-state",
            RecoveryCoordinatorStop.V2ProbeStopped, state: FirmwareUpdateState.Authorized);
        await Check("invalid-info-status",
            RecoveryCoordinatorStop.V2ProbeStopped, status: FirmwareUpdateStatus.AuthFailed);
        await Check("invalid-info-offset",
            RecoveryCoordinatorStop.V2ProbeStopped, offset: 1);
        await Check("valid-app-floor7",
            RecoveryCoordinatorStop.V2ProbeStopped, floor: 7, committed: 6);
        await Check("noapp-floor8",
            RecoveryCoordinatorStop.V2ProbeStopped,
            session: NoAppSession(), snapshot: NoAppSnapshot(), floor: 8,
            committed: 0);
        await Check("cancelled-no-discovery",
            RecoveryCoordinatorStop.SourceOrSessionRejected, cancel: true,
            discover: 0, open: 0, writes: 0, reads: 0);

        Console.WriteLine($"GATE3_INTEGRATED_V2_OFFLINE_TESTS=PASS COUNT={count} " +
            "USB_LIVE=NO PHYSICAL_IDENTITY_ATTESTED=NO FLASH_AUTHORIZED=NO");
    }

    private static RecoverySessionClaims NoAppSession() => Session with
    {
        Route = RecoveryRoute.NoValidApplication, RuntimeProductId = 0,
        RuntimePhysicalPort = null, RuntimeSession = null,
        RuntimeDetachObserved = false, RuntimeDetachOrder = 0
    };

    private static RecoveryHandoffSnapshot NoAppSnapshot() => Snapshot with
    {
        EntryMode = RecoveryEntryMode.NoValidApplication,
        RuntimeProductId = 0, RuntimePhysicalPortLocator = null,
        RuntimeSessionMarker = null, RuntimeDetachObserved = false,
        RuntimeDetachOrdinal = 0
    };

    private static byte[] Frame(uint floor, uint committed,
        FirmwareUpdateStatus status, FirmwareUpdateState state,
        ushort offset, byte opcode, bool badCrc)
    {
        byte[] wire = new byte[28];
        wire[0] = 0xA5; wire[1] = 0x5A; wire[2] = 1;
        wire[3] = 0x86; wire[6] = 1; wire[8] = 16;
        wire[10] = opcode; wire[11] = (byte)status;
        wire[12] = (byte)state;
        BinaryPrimitives.WriteUInt16LittleEndian(wire.AsSpan(14,2), offset);
        BinaryPrimitives.WriteUInt32LittleEndian(wire.AsSpan(18,4), floor);
        BinaryPrimitives.WriteUInt32LittleEndian(wire.AsSpan(22,4), committed);
        ushort crc = 0xFFFF;
        foreach (byte b in wire.AsSpan(2, 24))
        {
            crc ^= (ushort)(b << 8);
            for (int bit = 0; bit < 8; bit++)
                crc = (ushort)((crc & 0x8000) != 0 ? (crc << 1) ^ 0x1021 : crc << 1);
        }
        BinaryPrimitives.WriteUInt16LittleEndian(wire.AsSpan(26,2),
            badCrc ? (ushort)(crc ^ 1) : crc);
        return wire;
    }

    private sealed class MockDiscovery(MockTransport transport) : IDeviceDiscovery
    {
        public int CandidateCount { get; set; } = 1;
        public int DiscoverCount { get; private set; }
        public int OpenCount { get; private set; }

        public ValueTask<IReadOnlyList<DeviceCandidate>> DiscoverAsync(CancellationToken ct)
        {
            ct.ThrowIfCancellationRequested();
            ++DiscoverCount;
            return ValueTask.FromResult<IReadOnlyList<DeviceCandidate>>(
                Enumerable.Range(0, CandidateCount)
                .Select(_ => new DeviceCandidate(Port, "Deus OS Bootloader", "Linux"))
                .ToArray());
        }

        public ValueTask<IDeviceTransport> OpenAsync(DeviceCandidate device,
            CancellationToken ct)
        {
            ct.ThrowIfCancellationRequested();
            ++OpenCount;
            return ValueTask.FromResult<IDeviceTransport>(transport);
        }
    }

    private sealed class MockTransport(byte[] wire) : IDeviceTransport
    {
        private bool _served;
        public string? LocatorOverride { get; init; }
        public string Locator => LocatorOverride ?? Port;
        public bool Timeout { get; init; }
        public List<byte[]> Writes { get; } = new();
        public int ReadCount { get; private set; }

        public ValueTask WriteAsync(ReadOnlyMemory<byte> data, CancellationToken ct)
        {
            ct.ThrowIfCancellationRequested();
            Writes.Add(data.ToArray());
            return ValueTask.CompletedTask;
        }

        public ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken ct)
        {
            ct.ThrowIfCancellationRequested();
            ++ReadCount;
            if (Timeout) throw new DeusHostException(
                HostErrorKind.Timeout, "synthetic INFO timeout");
            if (_served) return ValueTask.FromResult(0);
            _served = true;
            wire.AsMemory().CopyTo(buffer);
            return ValueTask.FromResult(wire.Length);
        }

        public ValueTask DisposeAsync() => ValueTask.CompletedTask;
    }
}
