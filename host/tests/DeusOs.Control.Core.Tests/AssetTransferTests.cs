using System.Buffers.Binary;
using System.Text;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class AssetTransferTests
{
    [Fact]
    public void Crc32KnownVectorMatchesIsoHdlc()
    {
        Assert.Equal(
            0xCBF43926u,
            Crc32IsoHdlc.Compute(Encoding.ASCII.GetBytes("123456789")));
    }

    [Fact]
    public void DefaultOledLayoutSerializesToFrozenEightBytes()
    {
        Assert.Equal(
            Convert.FromHexString("01010A7E16000000"),
            OledUiLayoutConfigV1.Default.Serialize());
    }

    [Fact]
    public void OledLayoutDeserializerRejectsReservedByte()
    {
        var payload = Convert.FromHexString("01010A7E16010000");

        Assert.Throws<ArgumentException>(
            () => OledUiLayoutConfigV1.Deserialize(payload));
    }

    [Fact]
    public void BeginPayloadMatchesFrozenLayout()
    {
        var payload = AssetTransferProtocol.EncodeBegin(
            AssetTransferProtocol.OledUiLayoutObjectType,
            0x1234,
            8,
            OledUiLayoutConfigV1.SchemaVersion,
            0xA1B2C3D4u);

        Assert.Equal(16, payload.Length);
        Assert.Equal((byte)1, payload[0]);
        Assert.Equal((byte)AssetTransferOpcode.Begin, payload[1]);
        Assert.Equal((ushort)0x0001, BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(2, 2)));
        Assert.Equal((ushort)0x1234, BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(4, 2)));
        Assert.Equal((ushort)8, BinaryPrimitives.ReadUInt16LittleEndian(payload.AsSpan(6, 2)));
        Assert.Equal((byte)1, payload[8]);
        Assert.Equal((byte)0, payload[9]);
        Assert.Equal(0xA1B2C3D4u, BinaryPrimitives.ReadUInt32LittleEndian(payload.AsSpan(10, 4)));
        Assert.Equal((byte)0, payload[14]);
        Assert.Equal((byte)0, payload[15]);
    }

    [Fact]
    public void StatusResponseExposesEightByteCommittedMetadata()
    {
        var payload = new byte[26];
        payload[0] = AssetTransferProtocol.Version;
        payload[1] = (byte)AssetTransferOpcode.Status;
        payload[2] = (byte)AssetTransferStatus.Ok;
        payload[3] = (byte)AssetTransferSessionState.None;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4, 2), 0x0001);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(6, 2), 0);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(8, 4), 7);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(12, 2), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(14, 2), 0);
        payload[16] = 8;
        payload[17] = 0;
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(18, 2), 8);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(20, 2), 0);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(22, 4), 0x11223344u);

        var response = AssetTransferProtocol.ParseResponse(
            new BinaryFrame(
                ProtocolConstants.ProtocolVersion,
                FrameType.AssetTransferResponse,
                0,
                0,
                0x2211,
                payload),
            AssetTransferOpcode.Status);
        var status = AssetTransferProtocol.ToStatusSnapshot(response);

        Assert.Equal((uint)7, status.CommittedGeneration);
        Assert.Equal((ushort)8, status.CommittedPayloadLength);
        Assert.Equal(0x11223344u, status.CommittedPayloadCrc32);
    }

    [Fact]
    public void ReadChunkPayloadHonorsThirtyTwoByteCeiling()
    {
        Assert.Throws<ArgumentOutOfRangeException>(
            () => AssetTransferProtocol.EncodeReadChunk(
                AssetTransferProtocol.OledUiLayoutObjectType,
                1,
                0,
                33));
    }

    [Fact]
    public void FrameTypesMatchFrozenAssetAbi()
    {
        Assert.Equal(0x03, (byte)FrameType.AssetTransferRequest);
        Assert.Equal(0x85, (byte)FrameType.AssetTransferResponse);
        Assert.Equal(
            0x00000040u,
            ProtocolConstants.HelloCapabilityAssetConfigurationTransfer);
    }
}
