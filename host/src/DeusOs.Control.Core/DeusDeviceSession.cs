namespace DeusOs.Control.Core;

public sealed class DeusDeviceSession : IAsyncDisposable
{
    private readonly IDeviceDiscovery _discovery;
    private readonly TimeSpan _recoveryTimeout;
    private readonly TimeSpan _recoveryPollInterval;
    private readonly SemaphoreSlim _operationGate = new(1, 1);

    private DeusDeviceClient? _client;
    private DeviceCandidate? _candidate;
    private bool _disposed;

    public DeusDeviceSession(
        IDeviceDiscovery discovery,
        TimeSpan? recoveryTimeout = null,
        TimeSpan? recoveryPollInterval = null)
    {
        _discovery = discovery ?? throw new ArgumentNullException(nameof(discovery));
        _recoveryTimeout = recoveryTimeout ?? TimeSpan.FromSeconds(30);
        _recoveryPollInterval = recoveryPollInterval ?? TimeSpan.FromMilliseconds(250);

        if (_recoveryTimeout <= TimeSpan.Zero)
        {
            throw new ArgumentOutOfRangeException(nameof(recoveryTimeout));
        }

        if (_recoveryPollInterval <= TimeSpan.Zero)
        {
            throw new ArgumentOutOfRangeException(nameof(recoveryPollInterval));
        }
    }

    public ConnectionState State { get; private set; } =
        ConnectionState.Disconnected;

    public DeviceCandidate? Candidate => _candidate;

    public NegotiationResult? LastNegotiation { get; private set; }

    public event Action<ConnectionState>? StateChanged;

    public async Task<NegotiationResult> ConnectAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(candidate);
        ThrowIfDisposed();

        await _operationGate.WaitAsync(cancellationToken);
        try
        {
            await DisposeCurrentClientAsync();
            _candidate = candidate;
            LastNegotiation = null;
            SetState(ConnectionState.Discovered);

            try
            {
                return await OpenAndNegotiateAsync(candidate, cancellationToken);
            }
            catch
            {
                LastNegotiation = null;
                SetState(ConnectionState.Faulted);
                throw;
            }
        }
        finally
        {
            _operationGate.Release();
        }
    }

    public async Task<T> ExecuteAsync<T>(
        Func<DeusDeviceClient, CancellationToken, Task<T>> operation,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(operation);
        ThrowIfDisposed();

        await _operationGate.WaitAsync(cancellationToken);
        try
        {
            var client = _client;
            if (State != ConnectionState.Ready || client is null)
            {
                throw new InvalidOperationException(
                    $"device session is not READY (state={State})");
            }

            try
            {
                return await operation(client, cancellationToken);
            }
            catch (DeusHostException exception)
                when (exception.Kind == HostErrorKind.TransportDisconnected)
            {
                LastNegotiation = null;
                SetState(ConnectionState.Recovering);
                await DisposeCurrentClientAsync();

                try
                {
                    await RecoverAsync(cancellationToken);
                }
                catch (Exception recoveryException)
                {
                    LastNegotiation = null;
                    SetState(ConnectionState.Faulted);
                    throw new DeusHostException(
                        HostErrorKind.TransportDisconnected,
                        "transport disconnected and automatic session recovery failed",
                        new AggregateException(exception, recoveryException));
                }

                throw;
            }
        }
        finally
        {
            _operationGate.Release();
        }
    }

    public async Task DisconnectAsync(
        CancellationToken cancellationToken = default)
    {
        ThrowIfDisposed();

        await _operationGate.WaitAsync(cancellationToken);
        try
        {
            await DisposeCurrentClientAsync();
            _candidate = null;
            LastNegotiation = null;
            SetState(ConnectionState.Disconnected);
        }
        finally
        {
            _operationGate.Release();
        }
    }

    public async ValueTask DisposeAsync()
    {
        if (_disposed)
        {
            return;
        }

        await _operationGate.WaitAsync();
        try
        {
            if (_disposed)
            {
                return;
            }

            _disposed = true;
            await DisposeCurrentClientAsync();
            _candidate = null;
            LastNegotiation = null;
            SetState(ConnectionState.Disconnected);
        }
        finally
        {
            _operationGate.Release();
        }
    }

    private async Task<NegotiationResult> OpenAndNegotiateAsync(
        DeviceCandidate candidate,
        CancellationToken cancellationToken)
    {
        SetState(ConnectionState.Opening);
        var transport = await _discovery.OpenAsync(
            candidate,
            cancellationToken);
        var client = new DeusDeviceClient(transport);

        try
        {
            SetState(ConnectionState.Negotiating);
            var negotiation = await client.NegotiateAsync(cancellationToken);

            _client = client;
            _candidate = candidate;
            LastNegotiation = negotiation;
            SetState(ConnectionState.Ready);
            return negotiation;
        }
        catch
        {
            await client.DisposeAsync();
            throw;
        }
    }

    private async Task RecoverAsync(CancellationToken cancellationToken)
    {
        var previous = _candidate ?? throw new InvalidOperationException(
            "cannot recover a session without a previous session locator");

        using var timeout = CancellationTokenSource.CreateLinkedTokenSource(
            cancellationToken);
        timeout.CancelAfter(_recoveryTimeout);

        while (true)
        {
            timeout.Token.ThrowIfCancellationRequested();

            try
            {
                var candidates = await _discovery.DiscoverAsync(timeout.Token);
                var match = candidates.FirstOrDefault(
                    candidate => string.Equals(
                        candidate.Locator,
                        previous.Locator,
                        StringComparison.Ordinal));

                if (match is not null)
                {
                    SetState(ConnectionState.Discovered);
                    await OpenAndNegotiateAsync(match, timeout.Token);
                    return;
                }
            }
            catch (DeusHostException exception)
                when (IsTransientRecoveryError(exception.Kind))
            {
                SetState(ConnectionState.Recovering);
            }

            await Task.Delay(_recoveryPollInterval, timeout.Token);
        }
    }

    private static bool IsTransientRecoveryError(HostErrorKind kind) =>
        kind is
            HostErrorKind.Discovery or
            HostErrorKind.Open or
            HostErrorKind.TransportDisconnected or
            HostErrorKind.Timeout;

    private async ValueTask DisposeCurrentClientAsync()
    {
        if (_client is null)
        {
            return;
        }

        var client = _client;
        _client = null;
        await client.DisposeAsync();
    }

    private void SetState(ConnectionState state)
    {
        if (State == state)
        {
            return;
        }

        State = state;
        StateChanged?.Invoke(state);
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
