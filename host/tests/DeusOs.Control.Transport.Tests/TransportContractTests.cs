using DeusOs.Control.Core;
using DeusOs.Control.Transport.Linux;
using DeusOs.Control.Transport.Windows;
using Xunit;

namespace DeusOs.Control.Transport.Tests;

public sealed class TransportContractTests
{
    [Fact]
    public void WindowsManagementGuidMatchesFirmwareContract()
    {
        Assert.Equal(
            new Guid("C8B05EDE-1683-5002-81F0-95636B89CEC6"),
            WindowsWinUsbDiscovery.ManagementInterfaceGuid);
    }

    [Fact]
    public void LinuxTopologyConstantsMatchFirmwareContract()
    {
        Assert.Equal((ushort)0x1209, LinuxLibUsbDiscovery.VendorId);
        Assert.Equal((ushort)0x000C, LinuxLibUsbDiscovery.ProductId);
        Assert.Equal(2, LinuxLibUsbDiscovery.InterfaceNumber);
        Assert.Equal((byte)0x04, LinuxLibUsbDiscovery.OutEndpoint);
        Assert.Equal((byte)0x84, LinuxLibUsbDiscovery.InEndpoint);
    }

    [Fact]
    public void LinuxLocatorUsesBusAndPhysicalPortPathOnly()
    {
        Assert.Equal(
            "usb:001:8",
            LinuxLibUsbNative.FormatPortLocator(1, new byte[] { 8 }));
        Assert.Equal(
            "usb:002:3.5.1",
            LinuxLibUsbNative.FormatPortLocator(2, new byte[] { 3, 5, 1 }));
        Assert.Equal(
            "usb:001:root",
            LinuxLibUsbNative.FormatPortLocator(1, Array.Empty<byte>()));
    }

    [Fact]
    public void BootloaderTransportConstantsMatchFirmwareContract()
    {
        Assert.Equal(
            new Guid("F08907B7-BEC4-5FCF-BC4C-B446ED345D87"),
            WindowsBootloaderWinUsbDiscovery.BootloaderInterfaceGuid);
        Assert.Equal((ushort)0x1209, LinuxBootloaderLibUsbDiscovery.VendorId);
        Assert.Equal((ushort)0x000D, LinuxBootloaderLibUsbDiscovery.ProductId);
        Assert.Equal(0, LinuxBootloaderLibUsbDiscovery.InterfaceNumber);
        Assert.Equal((byte)0x01, LinuxBootloaderLibUsbDiscovery.OutEndpoint);
        Assert.Equal((byte)0x81, LinuxBootloaderLibUsbDiscovery.InEndpoint);
    }

    [Theory]
    [InlineData(121)]
    [InlineData(1460)]
    public void WindowsIoTimeoutErrorsMapToTimeout(int error)
    {
        Assert.Equal(
            HostErrorKind.Timeout,
            InvokeWindowsIoErrorMapping(
                error,
                HostErrorKind.TransportDisconnected));
    }

    [Fact]
    public void WindowsTimeoutCodeDoesNotRewriteOpenFailure()
    {
        Assert.Equal(
            HostErrorKind.Open,
            InvokeWindowsIoErrorMapping(
                121,
                HostErrorKind.Open));
    }

    [Fact]
    public void WindowsNonTimeoutIoErrorPreservesFallbackKind()
    {
        Assert.Equal(
            HostErrorKind.Open,
            InvokeWindowsIoErrorMapping(
                5,
                HostErrorKind.Open));
    }

    [Fact]
    public async Task NonWindowsBootloaderDiscoveryIsInert()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var devices = await new WindowsBootloaderWinUsbDiscovery()
            .DiscoverAsync(TestContext.Current.CancellationToken);

        Assert.Empty(devices);
    }

    [Fact]
    public async Task NonWindowsDiscoveryIsInert()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        var devices = await new WindowsWinUsbDiscovery()
            .DiscoverAsync(TestContext.Current.CancellationToken);

        Assert.Empty(devices);
    }

    [Fact]
    public async Task NonLinuxBootloaderDiscoveryIsInert()
    {
        if (OperatingSystem.IsLinux())
        {
            return;
        }

        var devices = await new LinuxBootloaderLibUsbDiscovery()
            .DiscoverAsync(TestContext.Current.CancellationToken);

        Assert.Empty(devices);
    }

    [Fact]
    public async Task NativeLifetimePreCancelledOperationDoesNotInvokeDelegate()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var cancellation = new CancellationTokenSource();
            cancellation.Cancel();
            var calls = 0;

            await Assert.ThrowsAnyAsync<OperationCanceledException>(
                () => lifetime.RunAsync(
                    () =>
                    {
                        Interlocked.Increment(ref calls);
                        return 1;
                    },
                    cancellation.Token));

            Assert.Equal(0, calls);
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimeCancellationWhileQueuedNeverInvokesDelegate()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var firstStarted = new ManualResetEventSlim();
            using var firstRelease = new ManualResetEventSlim();
            using var cancellation = new CancellationTokenSource();
            var queuedCalls = 0;

            var first = lifetime.RunAsync(
                () =>
                {
                    firstStarted.Set();
                    firstRelease.Wait();
                    return 1;
                },
                TestContext.Current.CancellationToken);

            Assert.True(firstStarted.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            var queued = lifetime.RunAsync(
                () =>
                {
                    Interlocked.Increment(ref queuedCalls);
                    return 2;
                },
                cancellation.Token);

            cancellation.Cancel();
            await Assert.ThrowsAnyAsync<OperationCanceledException>(() => queued);
            Assert.Equal(0, queuedCalls);

            firstRelease.Set();
            _ = await first;
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimeCancellationWaitsForActiveNativeDrain()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var started = new ManualResetEventSlim();
            using var release = new ManualResetEventSlim();
            using var cancellation = new CancellationTokenSource();

            var operation = lifetime.RunAsync(
                () =>
                {
                    started.Set();
                    release.Wait();
                    return 7;
                },
                cancellation.Token);

            Assert.True(started.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            cancellation.Cancel();
            Assert.False(operation.IsCompleted);

            release.Set();
            await Assert.ThrowsAnyAsync<OperationCanceledException>(
                () => operation);
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimeCancellationWinsAfterNativeFailureDrains()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var started = new ManualResetEventSlim();
            using var release = new ManualResetEventSlim();
            using var cancellation = new CancellationTokenSource();

            var operation = lifetime.RunAsync(
                () =>
                {
                    started.Set();
                    release.Wait();
                    throw new InvalidOperationException("native failure after cancel");
                },
                cancellation.Token);

            Assert.True(started.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            cancellation.Cancel();
            Assert.False(operation.IsCompleted);

            release.Set();
            await Assert.ThrowsAnyAsync<OperationCanceledException>(() => operation);
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimeDisposeWaitsForActiveWorkBeforeClose()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var started = new ManualResetEventSlim();
            using var release = new ManualResetEventSlim();
            var closeCount = 0;

            var operation = lifetime.RunAsync(
                () =>
                {
                    started.Set();
                    release.Wait();
                    return 9;
                },
                TestContext.Current.CancellationToken);

            Assert.True(started.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            var dispose = lifetime.DisposeAsync(
                () => Interlocked.Increment(ref closeCount)).AsTask();

            Assert.False(dispose.IsCompleted);
            Assert.Equal(0, closeCount);

            release.Set();
            Assert.Equal(9, await operation);
            await dispose;
            Assert.Equal(1, closeCount);
        }
    }

    [Fact]
    public async Task NativeLifetimeRejectsNewOperationAfterDisposeStarts()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var started = new ManualResetEventSlim();
            using var release = new ManualResetEventSlim();
            var secondCalls = 0;

            var active = lifetime.RunAsync(
                () =>
                {
                    started.Set();
                    release.Wait();
                    return 1;
                },
                TestContext.Current.CancellationToken);

            Assert.True(started.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            var dispose = lifetime.DisposeAsync(() => { }).AsTask();

            await Assert.ThrowsAsync<ObjectDisposedException>(
                () => lifetime.RunAsync(
                    () =>
                    {
                        Interlocked.Increment(ref secondCalls);
                        return 2;
                    },
                    TestContext.Current.CancellationToken));
            Assert.Equal(0, secondCalls);

            release.Set();
            _ = await active;
            await dispose;
        }
    }

    [Fact]
    public async Task NativeLifetimeRepeatedDisposeClosesExactlyOnce()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            var closeCount = 0;
            var first = lifetime.DisposeAsync(
                () => Interlocked.Increment(ref closeCount)).AsTask();
            var second = lifetime.DisposeAsync(
                () => Interlocked.Increment(ref closeCount)).AsTask();

            await Task.WhenAll(first, second);
            Assert.Equal(1, closeCount);
        }
    }

    [Fact]
    public async Task NativeLifetimeSerializesSameInstanceOperations()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            using var firstStarted = new ManualResetEventSlim();
            using var firstRelease = new ManualResetEventSlim();
            using var secondStarted = new ManualResetEventSlim();

            var first = lifetime.RunAsync(
                () =>
                {
                    firstStarted.Set();
                    firstRelease.Wait();
                    return 1;
                },
                TestContext.Current.CancellationToken);

            Assert.True(firstStarted.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            var second = lifetime.RunAsync(
                () =>
                {
                    secondStarted.Set();
                    return 2;
                },
                TestContext.Current.CancellationToken);

            Assert.False(secondStarted.IsSet);
            firstRelease.Set();

            Assert.Equal(1, await first);
            Assert.Equal(2, await second);
            Assert.True(secondStarted.IsSet);
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimeInstancesRemainIndependent()
    {
        foreach (var pair in CreateLifetimeHarnessPairs())
        {
            using var firstStarted = new ManualResetEventSlim();
            using var firstRelease = new ManualResetEventSlim();

            var first = pair.First.RunAsync(
                () =>
                {
                    firstStarted.Set();
                    firstRelease.Wait();
                    return 1;
                },
                TestContext.Current.CancellationToken);

            Assert.True(firstStarted.Wait(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken));
            var second = await pair.Second.RunAsync(
                    () => 2,
                    TestContext.Current.CancellationToken)
                .WaitAsync(TimeSpan.FromSeconds(2), TestContext.Current.CancellationToken);
            Assert.Equal(2, second);

            firstRelease.Set();
            Assert.Equal(1, await first);
            await pair.First.DisposeAsync(() => { });
            await pair.Second.DisposeAsync(() => { });
        }
    }

    [Fact]
    public async Task NativeLifetimePreservesNativeFailureWithoutCancellation()
    {
        foreach (var lifetime in CreateLifetimeHarnesses())
        {
            var exception = await Assert.ThrowsAsync<InvalidOperationException>(
                () => lifetime.RunAsync(
                    () => throw new InvalidOperationException("native failure"),
                    TestContext.Current.CancellationToken));

            Assert.Equal("native failure", exception.Message);
            await lifetime.DisposeAsync(() => { });
        }
    }

    [Theory]
    [InlineData(-7, HostErrorKind.Timeout)]
    [InlineData(-4, HostErrorKind.TransportDisconnected)]
    public void LinuxIoErrorsMapToFrozenKinds(
        int error,
        HostErrorKind expected)
    {
        if (!OperatingSystem.IsLinux())
        {
            return;
        }

        var exception = LinuxLibUsbNative.CreateException(
            error,
            "test",
            HostErrorKind.Open);

        Assert.Equal(expected, exception.Kind);
    }

    private static IReadOnlyList<LifetimeHarness> CreateLifetimeHarnesses() =>
        new[]
        {
            CreateWindowsLifetimeHarness(),
            CreateLinuxLifetimeHarness(),
        };

    private static IReadOnlyList<LifetimeHarnessPair> CreateLifetimeHarnessPairs() =>
        new[]
        {
            new LifetimeHarnessPair(
                CreateWindowsLifetimeHarness(),
                CreateWindowsLifetimeHarness()),
            new LifetimeHarnessPair(
                CreateLinuxLifetimeHarness(),
                CreateLinuxLifetimeHarness()),
        };

    private static LifetimeHarness CreateWindowsLifetimeHarness()
    {
        var lifetime = new WindowsNativeIoLifetime();
        return new LifetimeHarness(
            (operation, token) => lifetime.RunAsync(operation, token),
            close => lifetime.DisposeAsync(close));
    }

    private static LifetimeHarness CreateLinuxLifetimeHarness()
    {
        var lifetime = new LinuxNativeIoLifetime();
        return new LifetimeHarness(
            (operation, token) => lifetime.RunAsync(operation, token),
            close => lifetime.DisposeAsync(close));
    }

    private sealed record LifetimeHarness(
        Func<Func<int>, CancellationToken, Task<int>> RunAsync,
        Func<Action, ValueTask> DisposeAsync);

    private sealed record LifetimeHarnessPair(
        LifetimeHarness First,
        LifetimeHarness Second);

    private static HostErrorKind InvokeWindowsIoErrorMapping(
        int error,
        HostErrorKind fallbackKind)
    {
        var transportType = typeof(WindowsBootloaderWinUsbDiscovery)
            .Assembly
            .GetType(
                "DeusOs.Control.Transport.Windows.WindowsWinUsbTransport",
                throwOnError: true)!;
        var method = transportType.GetMethod(
            "MapIoErrorKind",
            System.Reflection.BindingFlags.Static |
            System.Reflection.BindingFlags.NonPublic)!;

        return Assert.IsType<HostErrorKind>(
            method.Invoke(
                null,
                new object[] { error, fallbackKind }));
    }

    [Fact]
    public async Task NonLinuxDiscoveryIsInert()
    {
        if (OperatingSystem.IsLinux())
        {
            return;
        }

        var devices = await new LinuxLibUsbDiscovery()
            .DiscoverAsync(TestContext.Current.CancellationToken);

        Assert.Empty(devices);
    }
}
