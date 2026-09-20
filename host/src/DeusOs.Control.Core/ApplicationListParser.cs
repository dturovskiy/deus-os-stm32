namespace DeusOs.Control.Core;

public static class ApplicationListParser
{
    public static ApplicationSnapshot Parse(string text)
    {
        ArgumentNullException.ThrowIfNull(text);

        var lines = text.Split(
            new[] { "\r\n", "\n" },
            StringSplitOptions.RemoveEmptyEntries);

        if (lines.Length < 1)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "applist output is empty");
        }

        var summary = KeyValueTokens.ParseLine(
            lines[0],
            HostErrorKind.Protocol);

        var runtimeAbi = KeyValueTokens.RequireHex32(
            summary,
            "APP_RUNTIME_ABI",
            HostErrorKind.Protocol);
        var registryCount = KeyValueTokens.RequireHex32(
            summary,
            "APP_REGISTRY_COUNT",
            HostErrorKind.Protocol);
        var activeIdRaw = KeyValueTokens.RequireHex32(
            summary,
            "APP_ACTIVE_ID",
            HostErrorKind.Protocol);
        var faultCount = KeyValueTokens.RequireHex32(
            summary,
            "APP_FAULT_COUNT",
            HostErrorKind.Protocol);

        if (activeIdRaw > ushort.MaxValue)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                "APP_ACTIVE_ID exceeds 16-bit range");
        }

        var applications = new List<ApplicationInfo>();
        foreach (var line in lines.Skip(1))
        {
            var values = KeyValueTokens.ParseLine(
                line,
                HostErrorKind.Protocol);

            var idRaw = KeyValueTokens.RequireHex32(
                values,
                "APP_ID",
                HostErrorKind.Protocol);
            if (idRaw == 0 || idRaw > ushort.MaxValue)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    $"invalid APP_ID 0x{idRaw:X8}");
            }

            var activeRaw = KeyValueTokens.RequireHex32(
                values,
                "ACTIVE",
                HostErrorKind.Protocol);
            if (activeRaw > 1)
            {
                throw new DeusHostException(
                    HostErrorKind.Protocol,
                    $"invalid ACTIVE value {activeRaw}");
            }

            applications.Add(new ApplicationInfo(
                checked((ushort)idRaw),
                KeyValueTokens.Require(
                    values,
                    "NAME",
                    HostErrorKind.Protocol),
                KeyValueTokens.RequireHex32(
                    values,
                    "ABI",
                    HostErrorKind.Protocol),
                KeyValueTokens.RequireHex32(
                    values,
                    "FLAGS",
                    HostErrorKind.Protocol),
                KeyValueTokens.Require(
                    values,
                    "STATE",
                    HostErrorKind.Protocol),
                KeyValueTokens.RequireHex32(
                    values,
                    "STATE_ID",
                    HostErrorKind.Protocol),
                activeRaw == 1));
        }

        if (applications.Count != registryCount)
        {
            throw new DeusHostException(
                HostErrorKind.Protocol,
                $"APP_REGISTRY_COUNT={registryCount} but {applications.Count} rows were returned");
        }

        return new ApplicationSnapshot(
            runtimeAbi,
            registryCount,
            checked((ushort)activeIdRaw),
            faultCount,
            applications);
    }
}
