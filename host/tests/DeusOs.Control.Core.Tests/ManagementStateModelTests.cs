using System.Text;
using DeusOs.Control.Core;
using Xunit;

namespace DeusOs.Control.Core.Tests;

public sealed class ManagementStateModelTests
{
    [Fact]
    public void PingParserAcceptsExactPongAndPreservesRequestId()
    {
        var parsed = ManagementStateParser.ParsePing(
            Result(ProtocolConstants.RpcPing, 0x1234, "PONG\r\n"));

        Assert.Equal((ushort)0x1234, parsed.RequestId);
    }

    [Fact]
    public void PingParserRejectsMalformedAndMultilineOutput()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParsePing(
                Result(ProtocolConstants.RpcPing, 1, "PONG EXTRA\r\n")));
        AssertProtocolFailure(
            () => ManagementStateParser.ParsePing(
                Result(ProtocolConstants.RpcPing, 1, "PONG\r\nSECOND\r\n")));
    }

    [Fact]
    public void ParserRejectsWrongRpcIdZeroRequestAndFailedStatus()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParsePing(
                Result(ProtocolConstants.RpcHealth, 1, "PONG\r\n")));
        AssertProtocolFailure(
            () => ManagementStateParser.ParsePing(
                Result(ProtocolConstants.RpcPing, 0, "PONG\r\n")));

        var status = Assert.Throws<DeusHostException>(
            () => ManagementStateParser.ParsePing(
                Result(
                    ProtocolConstants.RpcPing,
                    1,
                    "PONG\r\n",
                    statusDomain: 1,
                    statusCode: 2)));
        Assert.Equal(HostErrorKind.RpcStatus, status.Kind);
    }

    [Fact]
    public void HealthParserMapsFrozenFieldsExactly()
    {
        var health = ManagementStateParser.ParseHealth(
            Result(
                ProtocolConstants.RpcHealth,
                7,
                "HEALTH TICK=0x12345678 PC13=0x00000001 WDOG_ACTIVE=0x00000001 " +
                "WDOG_RELOAD_COUNT=0x0000002A RESET_FLAGS=0xA5A50004 IWDG_RESET=0x00000000\r\n"));

        Assert.Equal(0x12345678u, health.Tick);
        Assert.True(health.Pc13High);
        Assert.True(health.WatchdogActive);
        Assert.Equal(0x2Au, health.WatchdogReloadCount);
        Assert.Equal(0xA5A50004u, health.ResetFlags);
        Assert.False(health.IwdgReset);
    }

    [Fact]
    public void HealthParserAllowsUnknownForwardCompatibleField()
    {
        var health = ManagementStateParser.ParseHealth(
            Result(
                ProtocolConstants.RpcHealth,
                8,
                "HEALTH TICK=0x00000001 PC13=0x00000000 WDOG_ACTIVE=0x00000001 " +
                "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000001 " +
                "FUTURE_FIELD=0xDEADBEEF\r\n"));

        Assert.Equal(1u, health.Tick);
        Assert.False(health.Pc13High);
        Assert.True(health.IwdgReset);
    }

    [Fact]
    public void HealthParserRejectsMissingDuplicateAndBadHexFields()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH PC13=0x00000000 WDOG_ACTIVE=0x00000001 " +
                    "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n")));

        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH TICK=0x00000001 TICK=0x00000002 PC13=0x00000000 " +
                    "WDOG_ACTIVE=0x00000001 WDOG_RELOAD_COUNT=0x00000002 " +
                    "RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n")));

        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH TICK=1 PC13=0x00000000 WDOG_ACTIVE=0x00000001 " +
                    "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n")));
    }

    [Fact]
    public void HealthParserRejectsNonBooleanBitFields()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH TICK=0x00000001 PC13=0x00000002 WDOG_ACTIVE=0x00000001 " +
                    "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n")));

        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH TICK=0x00000001 PC13=0x00000000 WDOG_ACTIVE=0x00000002 " +
                    "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n")));
    }

    [Fact]
    public void HealthParserEnforcesBoundedSingleLineInput()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH TICK=0x00000001 PC13=0x00000000 WDOG_ACTIVE=0x00000001 " +
                    "WDOG_RELOAD_COUNT=0x00000002 RESET_FLAGS=0x00000003 IWDG_RESET=0x00000000\r\n" +
                    "SECOND\r\n")));

        AssertProtocolFailure(
            () => ManagementStateParser.ParseHealth(
                Result(
                    ProtocolConstants.RpcHealth,
                    1,
                    "HEALTH " + new string('X', 600))));
    }

    [Fact]
    public void ApplicationStartParserMapsAcknowledgedIds()
    {
        var result = ManagementStateParser.ParseApplicationStart(
            Result(
                ProtocolConstants.RpcAppStart,
                9,
                "APP_START_OK ID=0x00000002 ACTIVE_ID=0x00000002\r\n"));

        Assert.Equal((ushort)9, result.RequestId);
        Assert.Equal((ushort)2, result.ApplicationId);
        Assert.Equal((ushort)2, result.ActiveId);
    }

    [Fact]
    public void ApplicationStopParserAcceptsZeroActiveId()
    {
        var result = ManagementStateParser.ParseApplicationStop(
            Result(
                ProtocolConstants.RpcAppStop,
                10,
                "APP_STOP_OK ACTIVE_ID=0x00000000\r\n"));

        Assert.Equal((ushort)10, result.RequestId);
        Assert.Equal((ushort)0, result.ActiveId);
    }

    [Fact]
    public void ApplicationControlParsersRejectMalformedAndOutOfRangeIds()
    {
        AssertProtocolFailure(
            () => ManagementStateParser.ParseApplicationStart(
                Result(
                    ProtocolConstants.RpcAppStart,
                    1,
                    "APP_START_OK ID=0x00000000 ACTIVE_ID=0x00000001\r\n")));
        AssertProtocolFailure(
            () => ManagementStateParser.ParseApplicationStart(
                Result(
                    ProtocolConstants.RpcAppStart,
                    1,
                    "APP_START_OK ID=0x00010000 ACTIVE_ID=0x00000001\r\n")));
        AssertProtocolFailure(
            () => ManagementStateParser.ParseApplicationStop(
                Result(
                    ProtocolConstants.RpcAppStop,
                    1,
                    "APP_STOP_BAD ACTIVE_ID=0x00000000\r\n")));
    }

    [Fact]
    public void ApplicationControlParsersAllowUnknownForwardCompatibleFields()
    {
        var start = ManagementStateParser.ParseApplicationStart(
            Result(
                ProtocolConstants.RpcAppStart,
                11,
                "APP_START_OK ID=0x00000002 ACTIVE_ID=0x00000002 REV=0x00000003\r\n"));
        var stop = ManagementStateParser.ParseApplicationStop(
            Result(
                ProtocolConstants.RpcAppStop,
                12,
                "APP_STOP_OK ACTIVE_ID=0x00000000 REV=0x00000004\r\n"));

        Assert.Equal((ushort)2, start.ApplicationId);
        Assert.Equal((ushort)0, stop.ActiveId);
    }

    private static RpcResult Result(
        ushort rpcId,
        ushort requestId,
        string output,
        byte statusDomain = 0,
        byte statusCode = 0) =>
        new(
            rpcId,
            requestId,
            1,
            checked((uint)Encoding.UTF8.GetByteCount(output)),
            statusDomain,
            statusCode,
            Encoding.UTF8.GetBytes(output));

    private static void AssertProtocolFailure(Action action)
    {
        var exception = Assert.Throws<DeusHostException>(action);
        Assert.Equal(HostErrorKind.Protocol, exception.Kind);
    }
}
