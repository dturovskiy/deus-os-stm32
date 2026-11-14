using System.Buffers.Binary;

namespace DeusOs.Control.Core;

/// <summary>
/// Host-side v0 STM32 USB Management ↔ USART2 bridge protocol.
/// Does NOT provision ESP GPIO0/EN, select ROM boot, or authorize Flash.
/// A real target must independently admit OPEN; current normal10 denies it.
/// </summary>
public enum EspBridgeOperation : byte
{
    Open = 1,
    Tx = 2,
    Rx = 3,
    Close = 4
}

public enum EspBridgeStatus : byte
{
    Ok = 0,
    Denied = 1,
    BadSession = 2,
    BadSequence = 3,
    BadLength = 4,
    UartUncertain = 5
}

public sealed record EspBridgeReply(
    EspBridgeOperation Operation,
    uint Session,
    ushort Sequence,
    EspBridgeStatus Status,
    byte[] Data);

public static class EspBridgeWireV0
{
    public const int MaxData = 40;
    public const int RequestPrefixBytes = 8;
    public const int ReplyPrefixBytes = 9;
    public const int MaxResponsePayloadBytes = ReplyPrefixBytes + MaxData;

    public static byte[] EncodeRequest(
        EspBridgeOperation operation,
        uint session,
        ushort sequence,
        ReadOnlySpan<byte> data)
    {
        if (session == 0 || operation is < EspBridgeOperation.Open or > EspBridgeOperation.Close ||
            data.Length > MaxData ||
            (operation == EspBridgeOperation.Tx && data.IsEmpty) ||
            (operation != EspBridgeOperation.Tx && !data.IsEmpty) ||
            (operation == EspBridgeOperation.Open && sequence != 0) ||
            (operation != EspBridgeOperation.Open && sequence == 0))
        {
            throw new ArgumentException("invalid ESP bridge operation/session/sequence/data shape");
        }

        var payload = new byte[RequestPrefixBytes + data.Length];
        payload[0] = (byte)operation;
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(1, 4), session);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(5, 2), sequence);
        payload[7] = checked((byte)data.Length);
        data.CopyTo(payload.AsSpan(RequestPrefixBytes));
        return payload;
    }

    public static EspBridgeReply DecodeResponse(
        ReadOnlySpan<byte> payload,
        EspBridgeOperation expectedOperation,
        uint expectedSession,
        ushort expectedSequence)
    {
        if (payload.Length < ReplyPrefixBytes || payload.Length > MaxResponsePayloadBytes ||
            payload[0] != (byte)expectedOperation ||
            BinaryPrimitives.ReadUInt32LittleEndian(payload.Slice(1, 4)) != expectedSession ||
            BinaryPrimitives.ReadUInt16LittleEndian(payload.Slice(5, 2)) != expectedSequence ||
            payload[7] > (byte)EspBridgeStatus.UartUncertain ||
            payload[8] > MaxData ||
            payload.Length != ReplyPrefixBytes + payload[8] ||
            (expectedOperation != EspBridgeOperation.Rx && payload[8] != 0))
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "ESP bridge reply operation/session/sequence/status/length mismatch");
        }

        return new EspBridgeReply(
            expectedOperation, expectedSession, expectedSequence,
            (EspBridgeStatus)payload[7], payload.Slice(ReplyPrefixBytes).ToArray());
    }
}

/// <summary>
/// One-shot, no-retry host session against STM32 frame types 0x05/0x87.
/// It cannot bypass the target's source-locked can_enter=DENIED.
/// Keep a single client per *exclusive* USB transport; don't run a management
/// RPC/Node session concurrently on the same USB interface.
/// </summary>
public sealed class EspBridgeClient : IAsyncDisposable
{
    private readonly DeviceProtocolChannel _channel;
    // Every USB UART0 request must own the sequence slot through its ACK.
    // The esptool PTY proxy runs RX and TX pumps concurrently.
    private readonly SemaphoreSlim _sequenceLock = new(1, 1);
    private uint _session;
    private ushort _sequence;
    private bool _open;
    private bool _faulted;
    private bool _disposed;

    public EspBridgeClient(IDeviceTransport transport)
    {
        _channel = new DeviceProtocolChannel(
            transport ?? throw new ArgumentNullException(nameof(transport)));
    }

    public bool IsOpen => _open && !_faulted;
    public bool IsFaulted => _faulted;

    public async Task OpenAsync(uint freshSession, CancellationToken cancellationToken = default)
    {
        if (_disposed || _open || _faulted || freshSession == 0)
            throw new InvalidOperationException("ESP bridge not closed/clean or session invalid");

        try
        {
            var reply = await ExchangeAsync(
                EspBridgeOperation.Open, freshSession, 0, ReadOnlyMemory<byte>.Empty, cancellationToken);
            RequireOk(reply);
            _session = freshSession;
            _sequence = 0;
            _open = true;
        }
        catch
        {
            _faulted = true;
            throw;
        }
    }

    public async Task WriteAsync(ReadOnlyMemory<byte> bytes, CancellationToken cancellationToken = default)
    {
        RequireOpen();
        if (bytes.Length is < 1 or > EspBridgeWireV0.MaxData)
            throw new ArgumentOutOfRangeException(nameof(bytes),
                "bridge TX accepts exactly 1..40 bytes per request");
        await ExchangeNextAsync(EspBridgeOperation.Tx, bytes, cancellationToken);
    }

    public async Task<byte[]> ReadAsync(CancellationToken cancellationToken = default)
    {
        RequireOpen();
        var reply = await ExchangeNextAsync(
            EspBridgeOperation.Rx, ReadOnlyMemory<byte>.Empty, cancellationToken);
        return reply.Data;
    }

    public async Task CloseAsync(CancellationToken cancellationToken = default)
    {
        RequireOpen();
        await ExchangeNextAsync(
            EspBridgeOperation.Close, ReadOnlyMemory<byte>.Empty, cancellationToken);
        _open = false;
        _session = 0;
        _sequence = 0;
    }

    private async Task<EspBridgeReply> ExchangeNextAsync(
        EspBridgeOperation operation, ReadOnlyMemory<byte> data,
        CancellationToken cancellationToken)
    {
        // Lock BEFORE reading the sequence. DeviceProtocolChannel's internal
        // mutex alone is insufficient because the next seq was previously
        // calculated by two concurrent tasks outside that mutex.
        await _sequenceLock.WaitAsync(cancellationToken);
        try
        {
            RequireOpen();
            if (_sequence == ushort.MaxValue)
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "ESP bridge sequence exhausted: fresh physical session required, no replay");

            var next = checked((ushort)(_sequence + 1));
            var reply = await ExchangeAsync(
                operation, _session, next, data, cancellationToken);
            RequireOk(reply);
            _sequence = next;
            return reply;
        }
        catch
        {
            _faulted = true;
            _open = false;
            throw;
        }
        finally
        {
            _sequenceLock.Release();
        }
    }

    private async Task<EspBridgeReply> ExchangeAsync(
        EspBridgeOperation operation,
        uint session,
        ushort sequence,
        ReadOnlyMemory<byte> data,
        CancellationToken cancellationToken)
    {
        var payload = EspBridgeWireV0.EncodeRequest(
            operation, session, sequence, data.Span);
        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        timeout.CancelAfter(TimeSpan.FromSeconds(2));

        await _channel.EnterAsync(timeout.Token);
        try
        {
            var id = _channel.NextRequestId();
            var wire = BinaryFrameCodec.Encode(
                FrameType.EspBridgeRequest, 0, id, payload);
            await _channel.WriteAsync(wire, timeout.Token);
            var response = await _channel.ReadMatchingFrameAsync(
                id, FrameType.EspBridgeResponse, timeout.Token);
            return EspBridgeWireV0.DecodeResponse(
                response.Payload, operation, session, sequence);
        }
        finally
        {
            _channel.Exit();
        }
    }

    private static void RequireOk(EspBridgeReply reply)
    {
        if (reply.Status != EspBridgeStatus.Ok)
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "ESP bridge target refused operation: " + reply.Status +
                "; no automatic retry or GPIO/Flash fallback");
    }

    private void RequireOpen()
    {
        if (_disposed || !_open || _faulted)
            throw new InvalidOperationException(
                "ESP bridge session not admitted or faulted: no automatic resume");
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed) return;
        _disposed = true;
        // Closing the USB transport invalidates the device's maintenance owner.
        // Never silently synthesize an ESP ROM/app/BOOT recovery claim.
        await _channel.DisposeAsync();
    }
}

/// <summary>
/// ESP8266 ROM SLIP framing and READ-ONLY ROM SYNC request generator.
/// Pure byte transforms: no UART, reset, Flash, GPIO or RF side effects.
/// </summary>
public static class EspRomSlip
{
    private const byte End = 0xC0;
    private const byte Esc = 0xDB;
    private const byte EscEnd = 0xDC;
    private const byte EscEsc = 0xDD;
    public const int MaxPacketBytes = 4096;

    public static byte[] Encode(ReadOnlySpan<byte> packet)
    {
        if (packet.IsEmpty || packet.Length > MaxPacketBytes)
            throw new ArgumentOutOfRangeException(nameof(packet));
        var output = new List<byte>(packet.Length + 2) { End };
        foreach (var b in packet)
        {
            if (b is End or Esc)
            {
                output.Add(Esc);
                output.Add(b == End ? EscEnd : EscEsc);
            }
            else output.Add(b);
        }
        output.Add(End);
        return output.ToArray();
    }

    public static byte[] DecodeOne(ReadOnlySpan<byte> wire)
    {
        if (wire.Length < 3 || wire[0] != End || wire[^1] != End)
            throw new FormatException("SLIP frame must have start/end delimiters");
        var data = new List<byte>(wire.Length - 2);
        for (var i = 1; i < wire.Length - 1; ++i)
        {
            var value = wire[i];
            if (value == End)
                throw new FormatException("SLIP contains an extra frame delimiter");
            if (value == Esc)
            {
                if (++i >= wire.Length - 1)
                    throw new FormatException("SLIP escape truncated");
                value = wire[i] switch
                {
                    EscEnd => End,
                    EscEsc => Esc,
                    _ => throw new FormatException("SLIP escape invalid")
                };
            }
            data.Add(value);
            if (data.Count > MaxPacketBytes)
                throw new FormatException("SLIP exceeds bounded ROM packet");
        }
        if (data.Count == 0)
            throw new FormatException("SLIP empty ROM packet");
        return data.ToArray();
    }

    public static byte[] BuildReadOnlySyncRequest()
    {
        // ESP8266 ROM: 0x00=host→ROM, 0x08=SYNC, LE length=36, checksum=0.
        // Payload 0x07 0x07 0x12 0x20 + 32 x 0x55.
        var packet = new byte[8 + 36];
        packet[1] = 0x08;
        packet[2] = 36;
        packet[8] = 0x07;
        packet[9] = 0x07;
        packet[10] = 0x12;
        packet[11] = 0x20;
        packet.AsSpan(12, 32).Fill(0x55);
        return Encode(packet);
    }

    public static bool IsSyncResponse(ReadOnlySpan<byte> packet)
    {
        if (packet.Length < 10 || packet[0] != 0x01 || packet[1] != 0x08)
            return false;
        var declared = BinaryPrimitives.ReadUInt16LittleEndian(packet.Slice(2, 2));
        return declared >= 2 && declared <= MaxPacketBytes - 8 &&
               packet.Length == 8 + declared &&
               packet[^2] == 0 && packet[^1] == 0;
    }
}

/// <summary>
/// Incremental bounded ROM SLIP reply decoder: RX from bridge may deliver
/// arbitrarily split chunks. Malformed escape/overflow faults permanently.
/// </summary>
public sealed class EspRomSlipStream
{
    private readonly List<byte> _current = new();
    private bool _started;
    private bool _escape;
    private bool _faulted;

    public bool IsFaulted => _faulted;

    public IReadOnlyList<byte[]> Feed(ReadOnlySpan<byte> bytes)
    {
        if (_faulted) throw new InvalidOperationException("faulted SLIP stream");
        var frames = new List<byte[]>();
        foreach (var b in bytes)
        {
            if (b == 0xC0)
            {
                if (_escape)
                {
                    _faulted = true;
                    throw new FormatException("SLIP delimiter after dangling escape");
                }
                if (_started && _current.Count > 0)
                {
                    frames.Add(_current.ToArray());
                    _current.Clear();
                }
                _started = true;
                continue;
            }

            if (!_started) continue; // discard UART ROM startup chatter before SLIP
            if (_escape)
            {
                if (b != 0xDC && b != 0xDD)
                {
                    _faulted = true;
                    throw new FormatException("invalid SLIP escape in ROM RX stream");
                }
                _current.Add(b == 0xDC ? (byte)0xC0 : (byte)0xDB);
                _escape = false;
            }
            else if (b == 0xDB) _escape = true;
            else _current.Add(b);

            if (_current.Count > EspRomSlip.MaxPacketBytes)
            {
                _faulted = true;
                throw new FormatException("ROM SLIP packet limit exceeded");
            }
        }
        return frames;
    }
}


/// <summary>
/// The only implemented ROM operation is one read-only SYNC. Before this call,
/// the STM32 must already have admitted an explicitly owned bridge session
/// and separately placed ESP GPIO0/EN into ROM mode. No auto-boot, retry,
/// erase, Flash write, stub download or baud change is performed here.
/// </summary>
public static class EspRomBridgeSync
{
    public static async Task<byte[]> ProbeOnceAsync(
        EspBridgeClient bridge, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(bridge);
        if (!bridge.IsOpen)
            throw new InvalidOperationException(
                "ESP ROM SYNC requires a previously admitted physical bridge");

        var packet = EspRomSlip.BuildReadOnlySyncRequest();
        for (var offset = 0; offset < packet.Length; offset += EspBridgeWireV0.MaxData)
        {
            var count = Math.Min(EspBridgeWireV0.MaxData, packet.Length - offset);
            try
            {
                await bridge.WriteAsync(packet.AsMemory(offset, count), cancellationToken);
            }
            catch (DeusHostException ex)
            {
                throw new DeusHostException(
                    ex.Kind,
                    $"ESP_ROM_SYNC_TX_FRAGMENT_OFFSET_{offset}_BYTES_{count}: {ex.Message}", ex);
            }
        }

        var slip = new EspRomSlipStream();
        var rxBytes = 0;
        var rxNonemptyPolls = 0;
        var slipDelimiters = 0;
        for (var attempt = 0; attempt < 40; ++attempt)
        {
            byte[] chunk;
            try
            {
                chunk = await bridge.ReadAsync(cancellationToken);
            }
            catch (DeusHostException ex)
            {
                throw new DeusHostException(
                    ex.Kind,
                    $"ESP_ROM_SYNC_RX_POLL_{attempt}: {ex.Message}", ex);
            }
            rxBytes += chunk.Length;
            if (chunk.Length != 0) ++rxNonemptyPolls;
            foreach (var value in chunk)
                if (value == 0xC0) ++slipDelimiters;
            foreach (var frame in slip.Feed(chunk))
            {
                if (EspRomSlip.IsSyncResponse(frame)) return frame;
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    "unexpected ESP ROM response to one read-only SYNC; no retry");
            }

            await Task.Delay(15, cancellationToken);
        }

        throw new DeusHostException(
            HostErrorKind.Timeout,
            $"no ESP ROM SYNC response during 40 bounded RX polls; " +
            $"RX_BYTES={rxBytes} RX_NONEMPTY_POLLS={rxNonemptyPolls} " +
            $"SLIP_DELIMITERS={slipDelimiters}; no retry");
    }

    // Preserve the primary ROM failure while explicitly closing an admitted
    // STM32 bridge on timeout/unrecognized ROM response. Transport-level
    // failures may already have faulted the session: never fabricate a CLOSE.
    // This helper performs no extra ROM TX, GPIO, reset, or firmware action.
    public static async Task<byte[]> ProbeOnceAndCloseAsync(
        EspBridgeClient bridge,
        Action<string>? reportCleanup = null,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(bridge);
        Exception? primary = null;
        Exception? closeError = null;
        byte[]? reply = null;
        try
        {
            reply = await ProbeOnceAsync(bridge, cancellationToken);
        }
        catch (Exception error)
        {
            primary = error;
        }

        static void Report(Action<string>? report, string message)
        {
            // Diagnostics must not override either the ROM or CLOSE error.
            try { report?.Invoke(message); }
            catch (Exception) { }
        }

        if (bridge.IsOpen)
        {
            try
            {
                // Closing is an explicit USB bridge protocol operation,
                // not a retry of ESP ROM SYNC.
                await bridge.CloseAsync(CancellationToken.None);
                Report(reportCleanup, primary is null
                    ? "STM32_ESP_BRIDGE_CLOSE=PASS_AFTER_ROM_RESPONSE"
                    : "STM32_ESP_BRIDGE_CLOSE=PASS_AFTER_ROM_PROBE_FAILURE");
            }
            catch (Exception error)
            {
                closeError = error;
                Report(reportCleanup, "STM32_ESP_BRIDGE_CLOSE=FAILED_NO_RETRY " +
                    error.GetType().Name + ":" + error.Message);
            }
        }
        else
        {
            Report(reportCleanup, "STM32_ESP_BRIDGE_CLOSE=NOT_AVAILABLE_SESSION_FAULTED");
        }

        if (primary is not null)
        {
            if (closeError is not null)
                primary.Data["ESP_BRIDGE_CLOSE_ERROR"] =
                    closeError.GetType().Name + ":" + closeError.Message;
            System.Runtime.ExceptionServices.ExceptionDispatchInfo.Capture(primary).Throw();
        }
        if (closeError is not null)
            System.Runtime.ExceptionServices.ExceptionDispatchInfo.Capture(closeError).Throw();
        return reply ?? throw new InvalidOperationException("ROM result absent after admitted bridge");
    }
}
