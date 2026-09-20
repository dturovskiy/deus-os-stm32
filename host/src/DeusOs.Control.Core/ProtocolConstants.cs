namespace DeusOs.Control.Core;

public static class ProtocolConstants
{
    public const byte Magic0 = 0xA5;
    public const byte Magic1 = 0x5A;
    public const byte ProtocolVersion = 1;
    public const int HeaderBytesAfterMagic = 8;
    public const int FixedPrefixBytes = 10;
    public const int CrcBytes = 2;
    public const int MaxPayloadBytes = 132;
    public const int MaxWireBytes = 144;
    public const int RpcDataChunkMax = 48;
    public const int MaxArgs = 4;
    public const int MaxArgBytes = 31;

    public const byte RpcFlagAllowDestructive = 0x01;

    public const ushort RpcPing = 0x0001;
    public const ushort RpcHealth = 0x0003;
    public const ushort RpcRpcInfo = 0x0020;
    public const ushort RpcAppList = 0x0021;
    public const ushort RpcAppStart = 0x0022;
    public const ushort RpcAppStop = 0x0023;
    public const ushort RpcSysInfo = 0x0024;

    public const byte ExpectedServiceVersion = 3;
    public const ushort ExpectedRegistryCount = 36;
}

public enum FrameType : byte
{
    HelloRequest = 0x01,
    RpcRequest = 0x02,
    HelloResponse = 0x81,
    RpcData = 0x82,
    RpcEnd = 0x83,
    ProtocolError = 0x84,
}

public enum HostErrorKind
{
    Discovery,
    Open,
    TransportDisconnected,
    Timeout,
    Protocol,
    Crc,
    RequestCorrelation,
    RpcStatus,
    IncompatibleProtocol,
    IncompatibleService,
    InvalidSystemInfo,
    Cancelled,
}

public enum ConnectionState
{
    Disconnected,
    Discovered,
    Opening,
    Negotiating,
    Ready,
    Recovering,
    Faulted,
}

[Flags]
public enum SystemCapability : uint
{
    None = 0,
    SystemHealth = 1u << 0,
    ApplicationRuntime = 1u << 1,
    ApplicationControl = 1u << 2,
    Diagnostics = 1u << 3,
    LocalUi = 1u << 4,
    AssetConfigurationTransfer = 1u << 5,
    FirmwareUpdate = 1u << 6,
    NetworkServices = 1u << 7,
}
