namespace DeusOs.Control.Core;

public sealed class DeusHostException : Exception
{
    public DeusHostException(HostErrorKind kind, string message)
        : base(message)
    {
        Kind = kind;
    }

    public DeusHostException(HostErrorKind kind, string message, Exception innerException)
        : base(message, innerException)
    {
        Kind = kind;
    }

    public HostErrorKind Kind { get; }
}
