namespace DeusOs.Control.Core;

public static class SystemInfoParser
{
    public static SystemInfo Parse(string text)
    {
        ArgumentNullException.ThrowIfNull(text);

        var lines = text.Split(
            new[] { "\r\n", "\n" },
            StringSplitOptions.RemoveEmptyEntries);

        if (lines.Length != 4)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                $"sysinfo requires exactly four non-empty lines, got {lines.Length}");
        }

        var all = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var line in lines)
        {
            var lineValues = KeyValueTokens.ParseLine(
                line,
                HostErrorKind.InvalidSystemInfo);

            foreach (var pair in lineValues)
            {
                if (!all.TryAdd(pair.Key, pair.Value))
                {
                    throw new DeusHostException(
                        HostErrorKind.InvalidSystemInfo,
                        $"duplicate sysinfo key '{pair.Key}'");
                }
            }
        }

        var sourceTree = KeyValueTokens.Require(
            all,
            "SOURCE_TREE",
            HostErrorKind.InvalidSystemInfo);

        if (sourceTree != "UNBOUND" && !IsLowerHexTree(sourceTree))
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                $"invalid SOURCE_TREE '{sourceTree}'");
        }

        var protocol = KeyValueTokens.RequireHex32(
            all,
            "PROTOCOL_VERSION",
            HostErrorKind.InvalidSystemInfo);
        var service = KeyValueTokens.RequireHex32(
            all,
            "SERVICE_VERSION",
            HostErrorKind.InvalidSystemInfo);

        if (protocol > byte.MaxValue || service > byte.MaxValue)
        {
            throw new DeusHostException(
                HostErrorKind.InvalidSystemInfo,
                "protocol/service version exceeds byte range");
        }

        return new SystemInfo(
            KeyValueTokens.RequireHex32(
                all,
                "SYSINFO_ABI",
                HostErrorKind.InvalidSystemInfo),
            KeyValueTokens.Require(
                all,
                "OS_ID",
                HostErrorKind.InvalidSystemInfo),
            KeyValueTokens.Require(
                all,
                "PLATFORM_ID",
                HostErrorKind.InvalidSystemInfo),
            KeyValueTokens.Require(
                all,
                "ARCH_ID",
                HostErrorKind.InvalidSystemInfo),
            sourceTree,
            checked((byte)protocol),
            checked((byte)service),
            KeyValueTokens.RequireHex32(
                all,
                "APP_RUNTIME_ABI",
                HostErrorKind.InvalidSystemInfo),
            (SystemCapability)KeyValueTokens.RequireHex32(
                all,
                "CAPABILITIES",
                HostErrorKind.InvalidSystemInfo),
            KeyValueTokens.RequireHex32(
                all,
                "UNIT_ID_KIND",
                HostErrorKind.InvalidSystemInfo));
    }

    private static bool IsLowerHexTree(string value)
    {
        if (value.Length != 40)
        {
            return false;
        }

        foreach (var character in value)
        {
            if (!((character >= '0' && character <= '9') ||
                  (character >= 'a' && character <= 'f')))
            {
                return false;
            }
        }

        return true;
    }
}
