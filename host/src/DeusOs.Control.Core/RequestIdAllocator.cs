namespace DeusOs.Control.Core;

public sealed class RequestIdAllocator
{
    private ushort _next;

    public RequestIdAllocator(ushort initial = 1)
    {
        if (initial == 0)
        {
            throw new ArgumentOutOfRangeException(nameof(initial));
        }

        _next = initial;
    }

    public ushort Next()
    {
        var current = _next;
        _next = current == ushort.MaxValue
            ? (ushort)1
            : (ushort)(current + 1);
        return current;
    }

    public void Reset()
    {
        _next = 1;
    }
}
