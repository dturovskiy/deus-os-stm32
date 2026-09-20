using System.Buffers.Binary;
using System.Text;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class ProtocolTests
{
    [Fact]
    public void CrcKnownVectorMatchesCcittFalse()
    {
        var bytes = Encoding.ASCII.GetBytes("123456789");
        Assert.Equal((ushort)0x29B1, Crc16CcittFalse.Compute(bytes));
    }

    [Fact]
    public void RpcPingRequestWireMatchesGoldenVector()
    {
        var payload = RpcRequestEncoder.EncodePayload(
            ProtocolConstants.RpcPing,
            Array.Empty<string>());
        var wire = BinaryFrameCodec.Encode(
            FrameType.RpcRequest,
            0,
            0x1234,
            payload);

        Assert.Equal(
            Convert.FromHexString("A55A0102000034120400010000000349"),
            wire);
    }

    [Fact]
    public void DecoderAcceptsCompleteFrame()
    {
        var wire = CreateFrame(0x1001, new byte[] { 1, 2, 3 });
        var decoder = new BinaryFrameDecoder();

        var batch = decoder.Feed(wire);

        Assert.Empty(batch.Errors);
        var frame = Assert.Single(batch.Frames);
        Assert.Equal((ushort)0x1001, frame.RequestId);
        Assert.Equal(new byte[] { 1, 2, 3 }, frame.Payload);
    }

    [Fact]
    public void DecoderAcceptsByteAtATime()
    {
        var wire = CreateFrame(0x1002, Enumerable.Range(0, 32).Select(x => (byte)x).ToArray());
        var decoder = new BinaryFrameDecoder();
        var frames = new List<BinaryFrame>();

        foreach (var value in wire)
        {
            var batch = decoder.Feed(new[] { value });
            Assert.Empty(batch.Errors);
            frames.AddRange(batch.Frames);
        }

        var frame = Assert.Single(frames);
        Assert.Equal((ushort)0x1002, frame.RequestId);
    }

    [Fact]
    public void DecoderAcceptsEveryTwoChunkSplit()
    {
        var wire = CreateFrame(
            0x1003,
            Enumerable.Range(0, 48).Select(x => (byte)(255 - x)).ToArray());

        for (var split = 1; split < wire.Length; ++split)
        {
            var decoder = new BinaryFrameDecoder();
            var first = decoder.Feed(wire.AsSpan(0, split));
            var second = decoder.Feed(wire.AsSpan(split));

            Assert.Empty(first.Errors);
            Assert.Empty(second.Errors);
            Assert.Empty(first.Frames);
            Assert.Single(second.Frames);
        }
    }

    [Fact]
    public void DecoderAcceptsMultipleFramesInOneChunk()
    {
        var first = CreateFrame(0x2001, new byte[] { 0x11 });
        var second = CreateFrame(0x2002, new byte[] { 0x22, 0x33 });
        var combined = first.Concat(second).ToArray();

        var batch = new BinaryFrameDecoder().Feed(combined);

        Assert.Empty(batch.Errors);
        Assert.Equal(2, batch.Frames.Count);
        Assert.Equal((ushort)0x2001, batch.Frames[0].RequestId);
        Assert.Equal((ushort)0x2002, batch.Frames[1].RequestId);
    }

    [Fact]
    public void DecoderRejectsBadCrc()
    {
        var wire = CreateFrame(0x3001, new byte[] { 1, 2, 3 });
        wire[^1] ^= 0x80;

        var batch = new BinaryFrameDecoder().Feed(wire);

        Assert.Empty(batch.Frames);
        var error = Assert.Single(batch.Errors);
        Assert.Equal(FrameDecodeErrorKind.Crc, error.Kind);
    }

    [Fact]
    public void DecoderResynchronizesAfterOversizeLength()
    {
        var malformed = new byte[]
        {
            0xA5, 0x5A,
            0x01, 0x82, 0x00, 0x00,
            0x01, 0x00,
            0x85, 0x00,
        };
        var valid = CreateFrame(0x3002, new byte[] { 0x44 });
        var stream = malformed.Concat(new byte[] { 0x10, 0x20 }).Concat(valid).ToArray();
        var decoder = new BinaryFrameDecoder();

        var batch = decoder.Feed(stream);

        Assert.Contains(
            batch.Errors,
            error => error.Kind == FrameDecodeErrorKind.Length);
        var frame = Assert.Single(batch.Frames);
        Assert.Equal((ushort)0x3002, frame.RequestId);
    }

    [Fact]
    public void RequestIdWrapSkipsZero()
    {
        var allocator = new RequestIdAllocator(ushort.MaxValue);

        Assert.Equal(ushort.MaxValue, allocator.Next());
        Assert.Equal((ushort)1, allocator.Next());
        Assert.Equal((ushort)2, allocator.Next());
    }

    [Fact]
    public void SysInfoParserAcceptsExactContractAndUnknownFutureKey()
    {
        var text =
            "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
            "SOURCE_TREE=0123456789abcdef0123456789abcdef01234567\r\n" +
            "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
            "CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000 FUTURE_KEY=value\r\n";

        var info = SystemInfoParser.Parse(text);

        Assert.Equal("DEUS_OS", info.OsId);
        Assert.Equal("STM32F103C8", info.PlatformId);
        Assert.Equal("ARMV7M", info.ArchId);
        Assert.True(info.SourceTreeBound);
        Assert.Equal(SystemCapability.LocalUi, info.Capabilities & SystemCapability.LocalUi);
    }

    [Fact]
    public void SysInfoParserRejectsDuplicateMandatoryKey()
    {
        var text =
            "SYSINFO_ABI=0x00000001 OS_ID=DEUS_OS OS_ID=OTHER PLATFORM_ID=STM32F103C8 ARCH_ID=ARMV7M\r\n" +
            "SOURCE_TREE=UNBOUND\r\n" +
            "PROTOCOL_VERSION=0x00000001 SERVICE_VERSION=0x00000003 APP_RUNTIME_ABI=0x00000001\r\n" +
            "CAPABILITIES=0x0000001F UNIT_ID_KIND=0x00000000\r\n";

        var exception = Assert.Throws<DeusHostException>(
            () => SystemInfoParser.Parse(text));

        Assert.Equal(HostErrorKind.InvalidSystemInfo, exception.Kind);
    }

    [Fact]
    public void ApplicationListParserMatchesFirmwareShape()
    {
        var text =
            "APP_RUNTIME_ABI=0x00000001 APP_REGISTRY_COUNT=0x00000002 APP_ACTIVE_ID=0x00000001 " +
            "APP_FAULT_COUNT=0x00000000 APP_EVENT_COUNT=0x00000000 APP_VIEW_REVISION=0x00000001 " +
            "APP_LAST_EVENT_TYPE=0x00000000 APP_LAST_EVENT_SOURCE=0x00000000\r\n" +
            "APP_ID=0x00000001 NAME=system.home ABI=0x00000001 FLAGS=0x00000001 STATE=RUNNING " +
            "STATE_ID=0x00000003 ACTIVE=0x00000001\r\n" +
            "APP_ID=0x00000002 NAME=device.info ABI=0x00000001 FLAGS=0x00000000 STATE=STOPPED " +
            "STATE_ID=0x00000001 ACTIVE=0x00000000\r\n";

        var snapshot = ApplicationListParser.Parse(text);

        Assert.Equal((uint)2, snapshot.RegistryCount);
        Assert.Equal((ushort)1, snapshot.ActiveId);
        Assert.Equal(2, snapshot.Applications.Count);
        Assert.True(snapshot.Applications[0].Active);
        Assert.False(snapshot.Applications[1].Active);
    }

    [Fact]
    public void SystemCapabilityBitsMatchFrozenAbiExactly()
    {
        Assert.Equal(0x00000001u, (uint)SystemCapability.SystemHealth);
        Assert.Equal(0x00000002u, (uint)SystemCapability.ApplicationRuntime);
        Assert.Equal(0x00000004u, (uint)SystemCapability.ApplicationControl);
        Assert.Equal(0x00000008u, (uint)SystemCapability.Diagnostics);
        Assert.Equal(0x00000010u, (uint)SystemCapability.LocalUi);
        Assert.Equal(0x00000020u, (uint)SystemCapability.AssetConfigurationTransfer);
        Assert.Equal(0x00000040u, (uint)SystemCapability.FirmwareUpdate);
        Assert.Equal(0x00000080u, (uint)SystemCapability.NetworkServices);

        const SystemCapability allKnown =
            SystemCapability.SystemHealth |
            SystemCapability.ApplicationRuntime |
            SystemCapability.ApplicationControl |
            SystemCapability.Diagnostics |
            SystemCapability.LocalUi |
            SystemCapability.AssetConfigurationTransfer |
            SystemCapability.FirmwareUpdate |
            SystemCapability.NetworkServices;

        Assert.Equal(0x000000FFu, (uint)allKnown);
    }

    private static byte[] CreateFrame(ushort requestId, byte[] payload) =>
        BinaryFrameCodec.Encode(
            FrameType.RpcData,
            0,
            requestId,
            payload);
}
