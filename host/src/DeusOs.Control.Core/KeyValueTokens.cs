using System.Globalization;

namespace DeusOs.Control.Core;

internal static class KeyValueTokens
{
    public static IReadOnlyDictionary<string, string> ParseLine(
        string line,
        HostErrorKind errorKind)
    {
        var values = new Dictionary<string, string>(StringComparer.Ordinal);

        foreach (var token in line.Split(
                     ' ',
                     StringSplitOptions.RemoveEmptyEntries |
                     StringSplitOptions.TrimEntries))
        {
            var separator = token.IndexOf('=');
            if (separator <= 0 || separator == token.Length - 1)
            {
                throw new DeusHostException(
                    errorKind,
                    $"malformed key/value token '{token}'");
            }

            var key = token[..separator];
            var value = token[(separator + 1)..];

            if (!values.TryAdd(key, value))
            {
                throw new DeusHostException(
                    errorKind,
                    $"duplicate key '{key}'");
            }
        }

        return values;
    }

    public static string Require(
        IReadOnlyDictionary<string, string> values,
        string key,
        HostErrorKind errorKind)
    {
        if (!values.TryGetValue(key, out var value))
        {
            throw new DeusHostException(errorKind, $"missing key '{key}'");
        }

        return value;
    }

    public static uint RequireHex32(
        IReadOnlyDictionary<string, string> values,
        string key,
        HostErrorKind errorKind)
    {
        var text = Require(values, key, errorKind);
        if (text.Length != 10 ||
            !text.StartsWith("0x", StringComparison.Ordinal) ||
            !uint.TryParse(
                text.AsSpan(2),
                NumberStyles.AllowHexSpecifier,
                CultureInfo.InvariantCulture,
                out var value))
        {
            throw new DeusHostException(
                errorKind,
                $"invalid 32-bit hex value for '{key}': '{text}'");
        }

        return value;
    }
}
