using System.Buffers.Binary;
using DeusOs.Control.Core;

static void Check(bool condition, string label)
{
    if (!condition) throw new Exception("BRIDGE_OFFLINE_FAIL_" + label);
    Console.WriteLine("PASS " + label);
}
static void Reject<T>(Action body, string label) where T:Exception
{
    try { body(); }
    catch(T) { Console.WriteLine("PASS REJECT_"+label); return; }
    throw new Exception("BRIDGE_OFFLINE_WRONGLY_ACCEPTED_"+label);
}

Check((byte)FrameType.EspBridgeRequest==0x05 &&
      (byte)FrameType.EspBridgeResponse==0x87,"USB_FRAME_TYPES_05_87");
var sid=0x2143A1C5u;
byte[] framePayload=EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Tx,sid,1,[0x01,0xC0,0xDB,0x02]);
Check(framePayload.Length==12 &&
      framePayload[0]==2 &&
      BinaryPrimitives.ReadUInt32LittleEndian(framePayload.AsSpan(1,4))==sid &&
      framePayload[7]==4,"USB_BRIDGE_REQUEST_LAYOUT_LE");
Reject<ArgumentException>(()=>EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Open,0,0,[]),"ZERO_SESSION");
Reject<ArgumentException>(()=>EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Open,sid,1,[]),"OPEN_NONZERO_SEQ");
Reject<ArgumentException>(()=>EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Tx,sid,2,[]),"EMPTY_TX");
Reject<ArgumentException>(()=>EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Tx,sid,2,new byte[41]),"OVERSIZED_TX");
Reject<ArgumentException>(()=>EspBridgeWireV0.EncodeRequest(
    EspBridgeOperation.Rx,sid,2,[1]),"RX_WITH_DATA");

byte[] goodReply=new byte[9+3];
goodReply[0]=3;
BinaryPrimitives.WriteUInt32LittleEndian(goodReply.AsSpan(1,4),sid);
BinaryPrimitives.WriteUInt16LittleEndian(goodReply.AsSpan(5,2),2);
goodReply[7]=0;goodReply[8]=3;
goodReply[9]=1;goodReply[10]=0xc0;goodReply[11]=0xdb;
Check(EspBridgeWireV0.DecodeResponse(goodReply,EspBridgeOperation.Rx,sid,2).Data
      .AsSpan().SequenceEqual(new byte[]{1,0xc0,0xdb}),"RX_REPLY_THREE_BYTES");
Reject<DeusHostException>(()=>EspBridgeWireV0.DecodeResponse(
    goodReply,EspBridgeOperation.Rx,sid,3),"REPLAY_SEQUENCE");
Reject<DeusHostException>(()=>EspBridgeWireV0.DecodeResponse(
    goodReply,EspBridgeOperation.Rx,sid+1,2),"WRONG_SESSION");
Reject<DeusHostException>(()=>EspBridgeWireV0.DecodeResponse(
    goodReply.AsSpan(0,11),EspBridgeOperation.Rx,sid,2),"TRUNCATED_RX");
var malformed=(byte[])goodReply.Clone();malformed[7]=0x80;
Reject<DeusHostException>(()=>EspBridgeWireV0.DecodeResponse(
    malformed,EspBridgeOperation.Rx,sid,2),"INVALID_STATUS");

byte[] escInput=[0x01,0xc0,0xdb,0x02];
var escWire=EspRomSlip.Encode(escInput);
Check(escWire.AsSpan().SequenceEqual(new byte[]{0xc0,0x01,0xdb,0xdc,0xdb,0xdd,0x02,0xc0}),
      "SLIP_ESCAPE_C0_DB");
Check(EspRomSlip.DecodeOne(escWire).AsSpan().SequenceEqual(escInput),"SLIP_ROUNDTRIP");
Reject<FormatException>(()=>EspRomSlip.DecodeOne([0xc0,0xdb,0x00,0xc0]),"BAD_SLIP_ESCAPE");
Reject<FormatException>(()=>EspRomSlip.DecodeOne([0xc0,0x01,0xdb,0xc0]),"DANGLING_SLIP_ESCAPE");
var splitter=new EspRomSlipStream();
Check(splitter.Feed([0xaa,0xbb,0xc0,0x01,0xdb]).Count==0,"SLIP_FRAGMENT_PREFIX");
var outFrames=splitter.Feed([0xdc,0xdb,0xdd,0x02,0xc0]);
Check(outFrames.Count==1 && outFrames[0].AsSpan().SequenceEqual(escInput),
      "SLIP_FRAGMENT_REASSEMBLY");
var huge=new EspRomSlipStream();
Reject<FormatException>(()=>huge.Feed(
    Enumerable.Repeat((byte)0x01,EspRomSlip.MaxPacketBytes+1)
      .Prepend((byte)0xC0).ToArray()),"SLIP_OVERFLOW");
Check(huge.IsFaulted,"SLIP_OVERFLOW_STICKY_FAULT");
var sync=EspRomSlip.BuildReadOnlySyncRequest();
var syncPacket=EspRomSlip.DecodeOne(sync);
Check(syncPacket.Length==44 && syncPacket[0]==0 && syncPacket[1]==8 &&
      syncPacket[2]==36 && syncPacket[3]==0 &&
      syncPacket[8]==7 && syncPacket[9]==7 && syncPacket[10]==0x12 &&
      syncPacket[11]==0x20 &&
      syncPacket.AsSpan(12,32).ToArray().All(b=>b==0x55),
      "READ_ONLY_ESP_ROM_SYNC_V0_ENVELOPE");

var denying=new FakeTransport((req) => (EspBridgeStatus.Denied, Array.Empty<byte>()));
await using (var client=new EspBridgeClient(denying))
{
    await RejectAsync<DeusHostException>(
        ()=>client.OpenAsync(sid),"TARGET_OPEN_HARD_DENIED");
    Check(client.IsFaulted && !client.IsOpen &&
          denying.Operations.SequenceEqual([EspBridgeOperation.Open]),
          "TARGET_DENIED_ZERO_UART_TX_NO_RETRY");
    Reject<InvalidOperationException>(()=>client.ReadAsync().GetAwaiter().GetResult(),
                                    "READ_AFTER_DENIED");
}

var okTransport=new FakeTransport(req=>{
    var op=(EspBridgeOperation)req[0];
    return op==EspBridgeOperation.Rx ?
        (EspBridgeStatus.Ok, new byte[]{0xc0,0x01,0x02,0xc0}) :
        (EspBridgeStatus.Ok,Array.Empty<byte>());
});
await using (var client=new EspBridgeClient(okTransport))
{
    await client.OpenAsync(sid);
    Check(client.IsOpen,"EXCLUSIVE_SESSION_OPEN");
    await client.WriteAsync(escWire);
    var returned=await client.ReadAsync();
    Check(returned.AsSpan().SequenceEqual(new byte[]{0xc0,1,2,0xc0}),"UART_RX_FRAGMENT_TRANSFER");
    await client.CloseAsync();
    Check(!client.IsOpen &&
          okTransport.Operations.SequenceEqual([
              EspBridgeOperation.Open,EspBridgeOperation.Tx,
              EspBridgeOperation.Rx,EspBridgeOperation.Close]) &&
          okTransport.Sequences.SequenceEqual(new ushort[]{0,1,2,3}),
          "OPEN_TX_RX_CLOSE_SEQUENCE_EXACT_ONCE");
}
// One ROM SYNC on a fully mocked, pre-admitted bridge: 46 SLIP bytes
// produce two max40 USART2-TX chunks and one matching ROM response.
var fakeRom=new FakeTransport(req=>{
    var op=(EspBridgeOperation)req[0];
    return op==EspBridgeOperation.Rx
        ? (EspBridgeStatus.Ok,
           EspRomSlip.Encode(new byte[]{1,8,2,0,0,0,0,0,0,0}))
        : (EspBridgeStatus.Ok,Array.Empty<byte>());
});
await using(var romBridge=new EspBridgeClient(fakeRom))
{
    await romBridge.OpenAsync(sid);
    var romReply=await EspRomBridgeSync.ProbeOnceAsync(romBridge);
    Check(EspRomSlip.IsSyncResponse(romReply),"SINGLE_READONLY_ROM_SYNC_FRAME");
    await romBridge.CloseAsync();
    Check(fakeRom.Operations.SequenceEqual(new[]{
        EspBridgeOperation.Open,EspBridgeOperation.Tx,
        EspBridgeOperation.Tx,EspBridgeOperation.Rx,
        EspBridgeOperation.Close
    }) && fakeRom.Sequences.SequenceEqual(new ushort[]{0,1,2,3,4}),
    "ONE_ROM_SYNC_SPLIT_TX40_6_AND_RX_NO_RETRY");
}
await using(var deniedSync=new EspBridgeClient(
    new FakeTransport(_=>(EspBridgeStatus.Denied,Array.Empty<byte>()))))
{
    await RejectAsync<InvalidOperationException>(
        ()=>EspRomBridgeSync.ProbeOnceAsync(deniedSync),"ROM_SYNC_WITHOUT_ADMITTED_BRIDGE");
}

// Real ESP flash PTY has simultaneous RX and TX; every bridge exchange
// must use a fresh sequence and preserve STM32 strict request correlation.
var concurrentTransport=new FakeTransport(_=>(EspBridgeStatus.Ok,Array.Empty<byte>()));
await using (var client=new EspBridgeClient(concurrentTransport))
{
    await client.OpenAsync(sid);
    Task[] concurrentOps=Enumerable.Range(0,32)
        .Select(i=>i%2==0
            ? (Task)client.WriteAsync(new byte[]{checked((byte)i)})
            : client.ReadAsync())
        .ToArray();
    await Task.WhenAll(concurrentOps);
    await client.CloseAsync();
    Check(concurrentTransport.Sequences.SequenceEqual(
        Enumerable.Range(0,34).Select(i=>checked((ushort)i))) &&
          !client.IsFaulted && !client.IsOpen,
          "CONCURRENT_PTY_RX_TX_EXACT_MONOTONIC_SEQUENCE_34");
}
var forged=new FakeTransport(_=>(EspBridgeStatus.Ok,Array.Empty<byte>()),wrongSession:true);
await using (var client=new EspBridgeClient(forged))
{
    await RejectAsync<DeusHostException>(
        ()=>client.OpenAsync(sid),"FORGED_SESSION_RESPONSE");
    Check(client.IsFaulted,"FORGED_RESPONSE_HARD_STOP");
}
Console.WriteLine("NATIVE_MAC_ESP_ROM_BRIDGE_CLIENT_SLIP_OFFLINE=PASS");
Console.WriteLine("ALL_IO=FAKE_NO_USB_NO_ESP_FLASH_NO_GPIO_NO_RF=YES");

static async Task RejectAsync<T>(Func<Task> action,string label) where T:Exception
{
    try {await action();}
    catch(T) {Console.WriteLine("PASS REJECT_"+label);return;}
    throw new Exception("BRIDGE_OFFLINE_WRONGLY_ACCEPTED_"+label);
}

sealed class FakeTransport(
    Func<byte[],(EspBridgeStatus Status,byte[] Data)> responder,
    bool wrongSession=false): IDeviceTransport
{
    private readonly Queue<byte> _pending=new();
    public List<EspBridgeOperation> Operations {get;}=new();
    public List<ushort> Sequences {get;}=new();
    public string Locator=>"FAKE_USB_NO_DEVICE";
    public async ValueTask WriteAsync(ReadOnlyMemory<byte> bytes,CancellationToken ct)
    {
        // Simulate an asynchronous libusb write so RX and TX overlap.
        await Task.Yield();
        var decoder=new BinaryFrameDecoder();
        var parsed=decoder.Feed(bytes.Span);
        if(parsed.Errors.Count!=0||parsed.Frames.Count!=1)
            throw new Exception("FAKE_BAD_INPUT_ENVELOPE");
        var req=parsed.Frames[0];
        if(req.Type!=FrameType.EspBridgeRequest||req.Flags!=0)
            throw new Exception("FAKE_UNEXPECTED_REQUEST");
        var p=req.Payload;
        var op=(EspBridgeOperation)p[0];
        var sid=BinaryPrimitives.ReadUInt32LittleEndian(p.AsSpan(1,4));
        var seq=BinaryPrimitives.ReadUInt16LittleEndian(p.AsSpan(5,2));
        Operations.Add(op);
        Sequences.Add(seq);
        var (status,data)=responder(p);
        var reply=new byte[9+data.Length];
        reply[0]=(byte)op;
        BinaryPrimitives.WriteUInt32LittleEndian(reply.AsSpan(1,4),
                                                  wrongSession? sid+1:sid);
        BinaryPrimitives.WriteUInt16LittleEndian(reply.AsSpan(5,2),seq);
        reply[7]=(byte)status;
        reply[8]=checked((byte)data.Length);
        data.CopyTo(reply,9);
        var wire=BinaryFrameCodec.Encode(
            FrameType.EspBridgeResponse,0,req.RequestId,reply);
        foreach(var b in wire)_pending.Enqueue(b);
    }
    public ValueTask<int> ReadAsync(Memory<byte> buffer,CancellationToken ct)
    {
        var i=0;
        while(i<buffer.Length&&_pending.TryDequeue(out var b))
            buffer.Span[i++]=b;
        return ValueTask.FromResult(i);
    }
    public ValueTask DisposeAsync()=>ValueTask.CompletedTask;
}
