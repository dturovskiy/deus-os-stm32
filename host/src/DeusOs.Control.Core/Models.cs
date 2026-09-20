namespace DeusOs.Control.Core;

public sealed record HelloInfo(
    byte ProtocolVersion,
    byte ServiceVersion,
    byte MaxArgs,
    ushort LineCapacity,
    ushort RequestPayloadMax,
    ushort DataChunkMax,
    ushort RegistryCount,
    uint CapabilityFlags);

public sealed record RpcResult(
    ushort RpcId,
    ushort RequestId,
    ushort ChunkCount,
    uint TotalOutputBytes,
    byte StatusDomain,
    byte StatusCode,
    byte[] Output)
{
    public string OutputText => System.Text.Encoding.UTF8.GetString(Output);

    public bool IsSuccess => StatusDomain == 0 && StatusCode == 0;
}

public sealed record SystemInfo(
    uint AbiVersion,
    string OsId,
    string PlatformId,
    string ArchId,
    string SourceTree,
    byte ProtocolVersion,
    byte ServiceVersion,
    uint ApplicationRuntimeAbi,
    SystemCapability Capabilities,
    uint UnitIdKind)
{
    public bool SourceTreeBound => SourceTree != "UNBOUND";
}

public sealed record NegotiationResult(
    HelloInfo Hello,
    SystemInfo SystemInfo);

public sealed record ApplicationInfo(
    ushort Id,
    string Name,
    uint AbiVersion,
    uint Flags,
    string State,
    uint StateId,
    bool Active);

public sealed record ApplicationSnapshot(
    uint RuntimeAbi,
    uint RegistryCount,
    ushort ActiveId,
    uint FaultCount,
    IReadOnlyList<ApplicationInfo> Applications);
