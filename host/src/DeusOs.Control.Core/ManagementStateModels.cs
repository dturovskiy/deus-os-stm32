namespace DeusOs.Control.Core;

public sealed record PingStatus(ushort RequestId);

public sealed record HealthSnapshot(
    uint Tick,
    bool Pc13High,
    bool WatchdogActive,
    uint WatchdogReloadCount,
    uint ResetFlags,
    bool IwdgReset);

public sealed record ApplicationStartResult(
    ushort RequestId,
    ushort ApplicationId,
    ushort ActiveId);

public sealed record ApplicationStopResult(
    ushort RequestId,
    ushort ActiveId);

public static class ManagementStateParser
{
    private const int PingOutputMax = 16;
    private const int HealthOutputMax = 512;
    private const int ApplicationControlOutputMax = 128;

    public static PingStatus ParsePing(RpcResult result)
    {
        ValidateRpcResult(result, ProtocolConstants.RpcPing, "ping");
        var line = RequireSingleLine(result.OutputText, PingOutputMax, "ping");

        if (!string.Equals(line, "PONG", StringComparison.Ordinal))
        {
            throw ProtocolError($"unexpected ping output '{line}'");
        }

        return new PingStatus(result.RequestId);
    }

    public static HealthSnapshot ParseHealth(RpcResult result)
    {
        ValidateRpcResult(result, ProtocolConstants.RpcHealth, "health");
        var line = RequireSingleLine(result.OutputText, HealthOutputMax, "health");
        var values = ParsePrefixedValues(line, "HEALTH", "health");

        return new HealthSnapshot(
            KeyValueTokens.RequireHex32(values, "TICK", HostErrorKind.Protocol),
            RequireBooleanHex32(values, "PC13", "health"),
            RequireBooleanHex32(values, "WDOG_ACTIVE", "health"),
            KeyValueTokens.RequireHex32(
                values,
                "WDOG_RELOAD_COUNT",
                HostErrorKind.Protocol),
            KeyValueTokens.RequireHex32(
                values,
                "RESET_FLAGS",
                HostErrorKind.Protocol),
            RequireBooleanHex32(values, "IWDG_RESET", "health"));
    }

    public static ApplicationStartResult ParseApplicationStart(RpcResult result)
    {
        ValidateRpcResult(result, ProtocolConstants.RpcAppStart, "appstart");
        var line = RequireSingleLine(
            result.OutputText,
            ApplicationControlOutputMax,
            "appstart");
        var values = ParsePrefixedValues(line, "APP_START_OK", "appstart");

        var applicationId = RequireApplicationId(values, "ID", allowZero: false);
        var activeId = RequireApplicationId(values, "ACTIVE_ID", allowZero: true);

        return new ApplicationStartResult(
            result.RequestId,
            applicationId,
            activeId);
    }

    public static ApplicationStopResult ParseApplicationStop(RpcResult result)
    {
        ValidateRpcResult(result, ProtocolConstants.RpcAppStop, "appstop");
        var line = RequireSingleLine(
            result.OutputText,
            ApplicationControlOutputMax,
            "appstop");
        var values = ParsePrefixedValues(line, "APP_STOP_OK", "appstop");

        return new ApplicationStopResult(
            result.RequestId,
            RequireApplicationId(values, "ACTIVE_ID", allowZero: true));
    }

    private static void ValidateRpcResult(
        RpcResult result,
        ushort expectedRpcId,
        string operation)
    {
        ArgumentNullException.ThrowIfNull(result);

        if (result.RpcId != expectedRpcId)
        {
            throw ProtocolError(
                $"{operation} result RPC ID 0x{result.RpcId:X4} != 0x{expectedRpcId:X4}");
        }

        if (result.RequestId == 0)
        {
            throw ProtocolError($"{operation} result has zero request ID");
        }

        if (!result.IsSuccess)
        {
            throw new DeusHostException(
                HostErrorKind.RpcStatus,
                $"{operation} failed with status {result.StatusDomain}/{result.StatusCode}");
        }
    }

    private static string RequireSingleLine(
        string text,
        int maxLength,
        string operation)
    {
        ArgumentNullException.ThrowIfNull(text);

        if (text.Length > maxLength)
        {
            throw ProtocolError(
                $"{operation} output length {text.Length} exceeds {maxLength}");
        }

        var lines = text.Split(
            new[] { "\r\n", "\n" },
            StringSplitOptions.RemoveEmptyEntries);

        if (lines.Length != 1)
        {
            throw ProtocolError(
                $"{operation} requires exactly one non-empty output line");
        }

        return lines[0];
    }

    private static IReadOnlyDictionary<string, string> ParsePrefixedValues(
        string line,
        string prefix,
        string operation)
    {
        if (!line.StartsWith(prefix + " ", StringComparison.Ordinal))
        {
            throw ProtocolError(
                $"{operation} output does not begin with '{prefix}'");
        }

        var remainder = line[(prefix.Length + 1)..];
        if (remainder.Length == 0)
        {
            throw ProtocolError($"{operation} output has no key/value fields");
        }

        return KeyValueTokens.ParseLine(remainder, HostErrorKind.Protocol);
    }

    private static bool RequireBooleanHex32(
        IReadOnlyDictionary<string, string> values,
        string key,
        string operation)
    {
        var value = KeyValueTokens.RequireHex32(
            values,
            key,
            HostErrorKind.Protocol);

        if (value > 1)
        {
            throw ProtocolError(
                $"{operation} field '{key}' must be 0 or 1, got 0x{value:X8}");
        }

        return value == 1;
    }

    private static ushort RequireApplicationId(
        IReadOnlyDictionary<string, string> values,
        string key,
        bool allowZero)
    {
        var value = KeyValueTokens.RequireHex32(
            values,
            key,
            HostErrorKind.Protocol);

        if (value > ushort.MaxValue || (!allowZero && value == 0))
        {
            throw ProtocolError(
                $"application field '{key}' is outside the accepted 16-bit range: 0x{value:X8}");
        }

        return checked((ushort)value);
    }

    private static DeusHostException ProtocolError(string message) =>
        new(HostErrorKind.Protocol, message);
}
