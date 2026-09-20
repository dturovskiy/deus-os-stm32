using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Threading;
using DeusOs.Control.Core;

namespace DeusOs.Control.Desktop;

public sealed class MainWindow : Window
{
    private readonly DesktopController _controller = new();
    private readonly ComboBox _devices = new();
    private readonly TextBlock _status = new();
    private readonly TextBlock _identity = new();
    private readonly TextBlock _health = new();
    private readonly ListBox _applications = new();
    private readonly Button _refreshButton;
    private readonly Button _connectButton;
    private readonly Button _healthButton;
    private readonly Button _startButton;
    private readonly Button _stopButton;
    private readonly CancellationTokenSource _livenessCancellation = new();
    private Task? _livenessTask;
    private bool _recoveryRefreshPending;
    private IReadOnlyList<DeviceCandidate> _deviceCandidates =
        Array.Empty<DeviceCandidate>();

    public MainWindow()
    {
        Title = "Deus OS Control Panel";
        Width = 760;
        Height = 540;
        MinWidth = 600;
        MinHeight = 420;

        _refreshButton = new Button { Content = "Refresh devices" };
        _connectButton = new Button { Content = "Connect" };
        _healthButton = new Button { Content = "Refresh health" };
        _startButton = new Button { Content = "Start selected" };
        _stopButton = new Button { Content = "Stop active" };

        _refreshButton.Click += async (_, _) => await RefreshDevicesAsync();
        _connectButton.Click += async (_, _) => await ConnectAsync();
        _healthButton.Click += async (_, _) => await RefreshHealthAsync();
        _startButton.Click += async (_, _) => await StartSelectedAsync();
        _stopButton.Click += async (_, _) => await StopAsync();

        _controller.StateChanged += OnConnectionStateChanged;
        Closed += async (_, _) =>
        {
            _controller.StateChanged -= OnConnectionStateChanged;
            _livenessCancellation.Cancel();

            if (_livenessTask is not null)
            {
                try
                {
                    await _livenessTask;
                }
                catch (OperationCanceledException)
                {
                }
            }

            _livenessCancellation.Dispose();
            await _controller.DisposeAsync();
        };

        Content = BuildContent();
        ApplyConnectionState(_controller.State);
        _ = RefreshDevicesAsync();
        _livenessTask = MonitorConnectionAsync(_livenessCancellation.Token);
    }

    private Avalonia.Controls.Control BuildContent()
    {
        var deviceRow = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 8,
        };
        deviceRow.Children.Add(_devices);
        deviceRow.Children.Add(_refreshButton);
        deviceRow.Children.Add(_connectButton);

        var appButtons = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 8,
        };
        appButtons.Children.Add(_startButton);
        appButtons.Children.Add(_stopButton);

        var root = new StackPanel
        {
            Margin = new Thickness(16),
            Spacing = 10,
        };

        root.Children.Add(new TextBlock { Text = "Device" });
        root.Children.Add(deviceRow);
        root.Children.Add(_status);
        root.Children.Add(new TextBlock { Text = "System identity" });
        root.Children.Add(_identity);
        root.Children.Add(_healthButton);
        root.Children.Add(_health);
        root.Children.Add(new TextBlock { Text = "Applications" });
        root.Children.Add(_applications);
        root.Children.Add(appButtons);

        return new ScrollViewer { Content = root };
    }

    private void OnConnectionStateChanged(ConnectionState state)
    {
        Dispatcher.UIThread.Post(() => ApplyConnectionState(state));
    }

    private void ApplyConnectionState(ConnectionState state)
    {
        switch (state)
        {
            case ConnectionState.Ready:
                if (_controller.LastNegotiation is { } negotiation)
                {
                    ApplyNegotiation(negotiation);
                }

                if (_recoveryRefreshPending)
                {
                    _status.Text = "READY (recovered; waiting for runtime)";
                    UpdateReadyControls(false);
                }
                else
                {
                    _status.Text = "READY";
                    UpdateReadyControls(true);
                }
                break;

            case ConnectionState.Recovering:
                _recoveryRefreshPending = true;
                ClearLiveState(clearIdentity: true);
                _status.Text = "RECOVERING";
                UpdateReadyControls(false);
                break;

            case ConnectionState.Discovered:
                _status.Text = "DISCOVERED";
                UpdateReadyControls(false);
                break;

            case ConnectionState.Opening:
                _status.Text = "OPENING";
                UpdateReadyControls(false);
                break;

            case ConnectionState.Negotiating:
                _status.Text = "NEGOTIATING";
                UpdateReadyControls(false);
                break;

            case ConnectionState.Faulted:
                _recoveryRefreshPending = false;
                ClearLiveState(clearIdentity: true);
                _status.Text = "FAULTED";
                UpdateReadyControls(false);
                break;

            case ConnectionState.Disconnected:
            default:
                _recoveryRefreshPending = false;
                ClearLiveState(clearIdentity: true);
                _status.Text = "DISCONNECTED";
                UpdateReadyControls(false);
                break;
        }
    }

    private async Task RefreshDevicesAsync()
    {
        await RunUiOperationAsync(
            async () =>
            {
                await _controller.DisconnectAsync();
                ClearLiveState(clearIdentity: true);

                _deviceCandidates = await _controller.DiscoverAsync(
                    CancellationToken.None);
                _devices.ItemsSource = _deviceCandidates
                    .Select(candidate =>
                        $"{candidate.DisplayName} [{candidate.Platform}] {candidate.Locator}")
                    .ToArray();

                if (_deviceCandidates.Count != 0)
                {
                    _devices.SelectedIndex = 0;
                }

                _status.Text = _deviceCandidates.Count == 0
                    ? "No management device found"
                    : $"{_deviceCandidates.Count} device(s) discovered";
            });
    }

    private async Task ConnectAsync()
    {
        if (_devices.SelectedIndex < 0 ||
            _devices.SelectedIndex >= _deviceCandidates.Count)
        {
            _status.Text = "Select a device first";
            return;
        }

        await RunUiOperationAsync(
            async () =>
            {
                UpdateReadyControls(false);
                _status.Text = "NEGOTIATING";

                var negotiation = await _controller.ConnectAsync(
                    _deviceCandidates[_devices.SelectedIndex],
                    CancellationToken.None);

                ApplyNegotiation(negotiation);
                _status.Text = "READY (loading live state)";
                UpdateReadyControls(false);
                await RefreshSupportedSurfacesAsync();
                _status.Text = "READY";
                UpdateReadyControls(true);
            });
    }

    private async Task RefreshHealthAsync()
    {
        await RunUiOperationAsync(RefreshHealthCoreAsync);
    }

    private async Task RefreshHealthCoreAsync()
    {
        var result = await _controller.HealthAsync(CancellationToken.None);
        EnsureSuccess(result, "health");
        _health.Text = result.OutputText.Trim();
    }

    private async Task StartSelectedAsync()
    {
        if (!HasCapability(SystemCapability.ApplicationControl))
        {
            _status.Text = "Application control unavailable";
            return;
        }

        if (_applications.SelectedItem is not ApplicationInfo application)
        {
            _status.Text = "Select an application first";
            return;
        }

        await RunUiOperationAsync(
            async () =>
            {
                await _controller.StartApplicationAsync(
                    application.Id,
                    CancellationToken.None);
                await RefreshApplicationsCoreAsync();
            });
    }

    private async Task StopAsync()
    {
        if (!HasCapability(SystemCapability.ApplicationControl))
        {
            _status.Text = "Application control unavailable";
            return;
        }

        await RunUiOperationAsync(
            async () =>
            {
                await _controller.StopApplicationAsync(CancellationToken.None);
                await RefreshApplicationsCoreAsync();
            });
    }

    private async Task RefreshApplicationsCoreAsync()
    {
        var snapshot = await _controller.ApplicationsAsync(
            CancellationToken.None);
        _applications.ItemsSource = snapshot.Applications;
    }

    private async Task RefreshSupportedSurfacesAsync()
    {
        if (HasCapability(SystemCapability.ApplicationRuntime))
        {
            await RefreshApplicationsUntilReadyAsync();
        }
        else
        {
            _applications.ItemsSource = null;
        }

        if (HasCapability(SystemCapability.SystemHealth))
        {
            await RefreshHealthCoreAsync();
        }
        else
        {
            _health.Text = string.Empty;
        }
    }

    private async Task RefreshApplicationsUntilReadyAsync()
    {
        var deadline =
            DateTime.UtcNow + TimeSpan.FromSeconds(5);

        while (true)
        {
            var snapshot = await _controller.ApplicationsAsync(
                CancellationToken.None);
            _applications.ItemsSource = snapshot.Applications;

            if (snapshot.ActiveId != 0)
            {
                return;
            }

            if (DateTime.UtcNow >= deadline)
            {
                throw new DeusHostException(
                    HostErrorKind.Timeout,
                    "application runtime did not become ready after reconnect");
            }

            await Task.Delay(TimeSpan.FromMilliseconds(100));
        }
    }

    private bool HasCapability(SystemCapability capability)
    {
        var capabilities =
            _controller.LastNegotiation?.SystemInfo.Capabilities ??
            SystemCapability.None;
        return (capabilities & capability) != 0;
    }

    private void ApplyNegotiation(NegotiationResult negotiation)
    {
        var info = negotiation.SystemInfo;
        _identity.Text =
            $"{info.OsId} | {info.PlatformId} | {info.ArchId}\n" +
            $"source {info.SourceTree}\n" +
            $"protocol {info.ProtocolVersion}, service {info.ServiceVersion}, " +
            $"capabilities 0x{(uint)info.Capabilities:X8}";
    }

    private void ClearLiveState(bool clearIdentity)
    {
        if (clearIdentity)
        {
            _identity.Text = string.Empty;
        }

        _health.Text = string.Empty;
        _applications.ItemsSource = null;
    }

    private async Task MonitorConnectionAsync(
        CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                await Task.Delay(
                    TimeSpan.FromMilliseconds(500),
                    cancellationToken);
            }
            catch (OperationCanceledException)
            {
                return;
            }

            if (_controller.State != ConnectionState.Ready ||
                _recoveryRefreshPending)
            {
                continue;
            }

            try
            {
                var result = await _controller.PingAsync(cancellationToken);
                EnsureSuccess(result, "liveness ping");
            }
            catch (DeusHostException exception)
                when (exception.Kind == HostErrorKind.Cancelled &&
                      cancellationToken.IsCancellationRequested)
            {
                return;
            }
            catch (OperationCanceledException)
                when (cancellationToken.IsCancellationRequested)
            {
                return;
            }
            catch (DeusHostException exception)
            {
                await HandleHostExceptionAsync(exception);
            }
        }
    }

    private async Task RunUiOperationAsync(Func<Task> operation)
    {
        try
        {
            await operation();
        }
        catch (DeusHostException exception)
        {
            await HandleHostExceptionAsync(exception);
        }
        catch (Exception exception)
        {
            _status.Text = exception.Message;
            UpdateReadyControls(
                _controller.State == ConnectionState.Ready);
        }
    }

    private async Task HandleHostExceptionAsync(DeusHostException exception)
    {
        if (exception.Kind == HostErrorKind.TransportDisconnected &&
            _controller.State == ConnectionState.Ready &&
            _controller.LastNegotiation is { } recovered)
        {
            ApplyNegotiation(recovered);
            _status.Text = "READY (recovered; waiting for runtime)";
            UpdateReadyControls(false);

            try
            {
                await RefreshSupportedSurfacesAsync();
                _recoveryRefreshPending = false;
                _status.Text = "READY (recovered)";
                UpdateReadyControls(true);
            }
            catch (DeusHostException refreshException)
            {
                _recoveryRefreshPending = false;
                _status.Text =
                    $"{refreshException.Kind}: {refreshException.Message}";
                UpdateReadyControls(
                    _controller.State == ConnectionState.Ready);
            }

            return;
        }

        _status.Text = $"{exception.Kind}: {exception.Message}";
        UpdateReadyControls(
            _controller.State == ConnectionState.Ready);
    }

    private void UpdateReadyControls(bool ready)
    {
        var capabilities =
            _controller.LastNegotiation?.SystemInfo.Capabilities ??
            SystemCapability.None;

        var health =
            (capabilities & SystemCapability.SystemHealth) != 0;
        var applications =
            (capabilities & SystemCapability.ApplicationRuntime) != 0;
        var applicationControl =
            (capabilities & SystemCapability.ApplicationControl) != 0;

        _healthButton.IsEnabled = ready && health;
        _applications.IsEnabled = ready && applications;
        _startButton.IsEnabled =
            ready && applications && applicationControl;
        _stopButton.IsEnabled =
            ready && applications && applicationControl;
    }

    private static void EnsureSuccess(RpcResult result, string operation)
    {
        if (!result.IsSuccess)
        {
            throw new DeusHostException(
                HostErrorKind.RpcStatus,
                $"{operation} failed with status {result.StatusDomain}/{result.StatusCode}");
        }
    }
}
