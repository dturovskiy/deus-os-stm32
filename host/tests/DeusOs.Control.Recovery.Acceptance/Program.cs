using DeusOs.Control.Core;
using DeusOs.Control.Gate3Offline;
var p = new RecoverySourcePins(RecoveryRouteReadiness.CombinedTree,
    new(RecoveryRouteReadiness.NormalPackageSha256, 49072, 6),
    new(RecoveryRouteReadiness.RescuePackageSha256, 52092, 7));
var h = new RecoverySessionClaims(RecoveryRoute.ValidApplication,
    "Ubuntu", "Ubuntu", "usb:001:8", "usb:001:8",
    0x1209, 0x000D, 0, 0x01, 0x81, 1, true, 3, "new-session",
    0x000E, "usb:001:8", true, 2, "old-session");
var i = new RecoveryInfoClaims(0x02, 0, FirmwareUpdateStatus.Ok,
    FirmwareUpdateState.RecoveryIdle, 0, 6, 6);
var count=0;
void Test(string name, RecoveryStop expected, RecoverySourcePins? source=null,
    RecoverySessionClaims? handoff=null, RecoveryInfoClaims? info=null)
{
    var decision = RecoveryRouteReadiness.Evaluate(source??p,handoff??h,info??i);
    if(decision.Stop!=expected || decision.PhysicalIdentityAuthenticated
       ||decision.DestructiveOperationAuthorized||decision.FullGate3Accepted
       ||decision.Gate4Authorized
       ||decision.OfflineClaimsConsistent!=(expected==RecoveryStop.PreviewCompatibleOnly))
        throw new Exception("FAIL "+name+" "+decision);
    ++count;
    Console.WriteLine("PASS "+name);
}
Test("valid-app-same-port-new-session",RecoveryStop.PreviewCompatibleOnly);
Test("digest-case",RecoveryStop.PreviewCompatibleOnly,
    source:p with { Rescue=p.Rescue with {Sha256=p.Rescue.Sha256.ToLowerInvariant()}});
Test("source-tree-mismatch",RecoveryStop.SourceOrPackageNotPinned,
    source:p with {SourceTree="0000000000000000000000000000000000000000"});
Test("normal-sha-mismatch",RecoveryStop.SourceOrPackageNotPinned,
    source:p with {Normal=p.Normal with {Sha256=new string('0',64)}});
Test("rescue-sha-mismatch",RecoveryStop.SourceOrPackageNotPinned,
    source:p with {Rescue=p.Rescue with {Sha256=new string('A',64)}});
Test("normal-size-mismatch",RecoveryStop.SourceOrPackageNotPinned,
    source:p with {Normal=p.Normal with {Bytes=49071}});
Test("rescue-version-mismatch",RecoveryStop.SourceOrPackageNotPinned,
    source:p with {Rescue=p.Rescue with {Version=6}});
Test("wrong-owner",RecoveryStop.WrongUsbOwner,handoff:h with {UsbOwner="Windows"});
Test("wrong-opening-host",RecoveryStop.WrongUsbOwner,handoff:h with {OpeningHost="Windows"});
Test("wrong-port",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {BootloaderPhysicalPort="usb:001:9"});
Test("noncanonical-port",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {PhysicalPort="usb:001:08"});
Test("wrong-bootloader-pid",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {BootloaderProductId=0x000C});
Test("wrong-bootloader-interface",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {BootloaderInterface=2});
Test("wrong-bootloader-ep",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {BootloaderInEndpoint=0x84});
Test("zero-candidates",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {CandidateCount=0});
Test("multiple-candidates",RecoveryStop.IdentityOrTopologyUncorrelated,
    handoff:h with {CandidateCount=2});
Test("missing-bootloader-attach",RecoveryStop.NoBootloaderAttach,
    handoff:h with {BootloaderAttachObserved=false});
Test("stale-attach-ordinal",RecoveryStop.HandoffNotCorrelated,
    handoff:h with {BootloaderAttachOrder=2});
Test("stale-session-id",RecoveryStop.HandoffNotCorrelated,
    handoff:h with {BootloaderSession="old-session"});
Test("no-runtime-detach",RecoveryStop.HandoffNotCorrelated,
    handoff:h with {RuntimeDetachObserved=false});
Test("wrong-runtime-pid",RecoveryStop.HandoffNotCorrelated,
    handoff:h with {RuntimeProductId=0x000D});
Test("legacy-v5-entry",RecoveryStop.PreviewCompatibleOnly,
    handoff:h with {RuntimeProductId=0x000C},
    info:i with {CommittedVersion=5,VersionFloor=5});
Test("noapp-with-runtime",RecoveryStop.NoAppRouteContradiction,
    handoff:h with {Route=RecoveryRoute.NoValidApplication});
Test("info-wrong-flags",RecoveryStop.InfoNotValid,info:i with {Flags=1});
Test("info-wrong-opcode",RecoveryStop.InfoNotValid,info:i with {Opcode=3});
Test("info-bad-status",RecoveryStop.InfoNotValid,
    info:i with {Status=FirmwareUpdateStatus.VersionRejected});
Test("info-state",RecoveryStop.InfoNotValid,
    info:i with {State=FirmwareUpdateState.Authorized});
Test("info-offset",RecoveryStop.InfoNotValid,info:i with {ExpectedOffset=48});
Test("valid-app-commit7",RecoveryStop.UnexpectedCommittedApplication,
    info:i with {CommittedVersion=7});
Test("valid-app-floor7",RecoveryStop.RescueVersionNotAdmissible,
    info:i with {VersionFloor=7});
Test("valid-app-floor-lt-commit",RecoveryStop.RescueVersionNotAdmissible,
    info:i with {VersionFloor=5});
var noapp=h with {Route=RecoveryRoute.NoValidApplication, RuntimeProductId=0,
    RuntimePhysicalPort=null, RuntimeSession=null,
    RuntimeDetachObserved=false,RuntimeDetachOrder=0};
Test("no-app-floor7-equality",RecoveryStop.PreviewCompatibleOnly,
    handoff:noapp,info:i with {CommittedVersion=0,VersionFloor=7});
Test("no-app-floor8",RecoveryStop.RescueVersionNotAdmissible,
    handoff:noapp,info:i with {CommittedVersion=0,VersionFloor=8});
Test("no-app-with-committed-app",RecoveryStop.InvalidNoAppInfo,
    handoff:noapp,info:i with {CommittedVersion=6});
Console.WriteLine($"GATE3_OFFLINE_POLICY_TESTS=PASS COUNT={count} PHYSICAL_000D=NOT_ATTESTED FLASH_AUTHORIZED=NO");
await CoordinatorTests.RunAsync();
